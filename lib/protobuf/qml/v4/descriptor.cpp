#include "protobuf/qml/v4/descriptor.h"
#include "protobuf/qml/v4/v4util.h"

#include <QByteArray>
#include <QQmlInfo>
#include <QThreadPool>
#include <private/qv4arraybuffer_p.h>
#include <private/qv4typedarray_p.h>
#include <private/qv4variantobject_p.h>

using namespace QV4;

namespace protobuf {
namespace qml {

typedef uint32_t OneofIndex;

void Descriptor::serialize(QQmlV4Function* args) {
  if (!descriptor_) {
    qmlInfo(this) << __func__ << " Descriptor unavailable.";
    return;
  }
  auto v4 = args->v4engine();
  Scope scope(v4);
  ScopedArrayObject arg_msg(scope, (*args)[0]);
  if (!arg_msg) {
    qmlInfo(this) << __func__ << " Nothing to serialize";
    return;
  }
  auto msg = jsValueToMessage(v4, *arg_msg);
  if (!msg) {
    qmlInfo(this) << __func__ << " Failed to serialize";
    return;
  }

  ScopedValue arg_cb(scope, (*args)[1]);
  if (arg_cb->isNullOrUndefined()) {
    auto size = msg->ByteSize();
    Scoped<ArrayBuffer> buf(scope,
                            v4->memoryManager->alloc<ArrayBuffer>(v4, size));
    Q_ASSERT(buf);
    if (size <= 0) {
      qmlInfo(this) << __func__ << " Nothing to serialize";
    } else {
      if (!msg->SerializeToArray(buf->data(), buf->byteLength())) {
        qmlInfo(this) << __func__ << " Failed to serialize message object";
      }
    }
    args->setReturnValue(buf->asReturnedValue());
  } else {
    QThreadPool::globalInstance()->start(
        new SerializeTask(std::move(msg), v4, arg_cb));
  }
}

google::protobuf::Message* Descriptor::parseToNewMessage(const char* data,
                                                         int size) {
  auto msg = message_factory_.GetPrototype(descriptor_)->New();
  if (!msg->ParseFromArray(data, size)) {
    qmlInfo(this) << "Failed to parse message";
    return nullptr;
  }
  return msg;
}

void Descriptor::parse(QQmlV4Function* args) {
  if (!descriptor_) {
    qmlInfo(this) << __func__ << " Descriptor unavailable.";
    return;
  }
  auto v4 = args->v4engine();
  Scope scope(v4);
  Scoped<ArrayBuffer> arg_buf(scope, (*args)[0]);
  ScopedValue arg_cb(scope, (*args)[1]);

  if (arg_cb->isNullOrUndefined()) {
    // No callback, do it synchronously
    const char* data = nullptr;
    int size = 0;
    if (arg_buf) {
      data = arg_buf->data();
      size = arg_buf->byteLength();
      // qmlInfo(this) << __func__ << " Got ArrayBuffer " << size << " : "
      // << reinterpret_cast<intptr_t>(data);
    } else {
      ScopedValue o2(scope, (*args)[0]);
      auto var = v4->toVariant(o2, 0, false);
      if (static_cast<QMetaType::Type>(var.type()) == QMetaType::QByteArray) {
        auto ba = var.value<QByteArray>();
        qmlInfo(this) << __func__ << " Got QByteArray";
        data = ba.data();
        size = ba.size();
      } else {
        qmlInfo(this) << __func__ << " Not a serializable type";
      }
    }
    if (size <= 0 || !data) {
      qmlInfo(this) << __func__ << " Nothing to parse";
      return;
    }
    auto msg = defaultMessage();
    Q_ASSERT(msg);
    if (!msg->ParseFromArray(data, size)) {
      qDebug() << "Failed to parse";
      qmlInfo(this) << __func__ << " Failed to parse";
      return;
    }
    args->setReturnValue(messageToJsValue(v4, *msg));
  } else {
    // The caller supplied callback so we do it asynchronously
    if (arg_buf) {
      QThreadPool::globalInstance()->start(new ParseTask(
          this, newMessage(), arg_buf->asByteArray(), v4, arg_cb));
    } else {
      ScopedValue o2(scope, (*args)[0]);
      auto var = v4->toVariant(o2, 0, false);
      if (static_cast<QMetaType::Type>(var.type()) == QMetaType::QByteArray) {
        auto ba = var.value<QByteArray>();
        QThreadPool::globalInstance()->start(
            new ParseTask(this, newMessage(), ba, v4, arg_cb));
      } else {
        qmlInfo(this) << __func__ << " Not a serializable type";
      }
    }
  }
}

using namespace ::google::protobuf;

std::unique_ptr<google::protobuf::Message> Descriptor::jsValueToMessage(
    ExecutionEngine* v4, ArrayObject& value) {
  std::unique_ptr<google::protobuf::Message> msg(
      message_factory_.GetPrototype(descriptor_)->New());
  if (!msg) {
    qmlInfo(this) << __func__ << " Failed to create message protobuf object.";
    return nullptr;
  }
  if (!jsValueToMessage(v4, value, *msg)) {
    return nullptr;
  }
  return std::move(msg);
}

template <typename T>
bool typed_get_indexed(TypedArray* array, int index, T* value) {
  if (array->length() <= index) {
    return false;
  }
  *value = *reinterpret_cast<T*>(array->d()->buffer->data->data() +
                                 index * sizeof(T));
  return true;
}

template <typename T>
bool typed_put_indexed(TypedArray* array, int index, T value) {
  if (array->length() <= index) {
    return false;
  }
  *reinterpret_cast<T*>(array->d()->buffer->data->data() + index * sizeof(T)) =
      value;
  return true;
}

template <typename T>
bool typed_get_indexed(ArrayBuffer* buffer, int index, T* value) {
  if (buffer->byteLength() < (index + 1) * sizeof(T)) {
    return false;
  }
  *value = *reinterpret_cast<T*>(buffer->data() + index * sizeof(T));
  return true;
}

template <typename T>
bool typed_put_indexed(ArrayBuffer* buffer, int index, T value) {
  if (buffer->byteLength() < (index + 1) * sizeof(T)) {
    return false;
  }
  *reinterpret_cast<T*>(buffer->data() + index * sizeof(T)) = value;
  return true;
}

bool Descriptor::jsValueToMessage(ExecutionEngine* v4,
                                  ArrayObject& root,
                                  google::protobuf::Message& msg) {
  Scope scope(v4);
  ScopedArrayObject field_values(scope, root.getIndexed(0));
  Scoped<ArrayBuffer> oneof_cases(scope, root.getIndexed(1));
  if (!field_values || !oneof_cases) {
    qmlInfo(this) << __func__ << " Invalid arguments";
    return false;
  }
  auto reflection = msg.GetReflection();
  auto descriptor = msg.GetDescriptor();
  size_t field_count = descriptor->field_count();
  for (size_t i = 0; i < field_count && i < field_values->getLength(); ++i) {
    auto field = descriptor->field(i);
    auto oneof = field->containing_oneof();
    if (oneof) {
      OneofIndex oneof_case;
      if (!typed_get_indexed(oneof_cases, oneof->index(), &oneof_case) ||
          oneof_case != field->number()) {
        continue;
      }
    }
    if (field->is_repeated()) {
      ScopedValue field_value(scope, field_values->getIndexed(i));
      if (!field_value) {
        qmlInfo(this) << "Invalid type for repeated field: "
                      << QString::fromStdString(field->name());
        continue;
      }
      setRepeatedFieldValue(v4, *reflection, msg, field,
                            field_values->getIndexed(i));
    } else {
      setFieldValue(v4, *reflection, msg, field, field_values->getIndexed(i));
    }
  }
  return true;
}

ReturnedValue Descriptor::messageToJsValue(
    ExecutionEngine* v4, const google::protobuf::Message& msg) {
  Scope scope(v4);
  auto message = msg.GetDescriptor();
  ScopedArrayObject field_values(scope,
                                 v4->newArrayObject(message->field_count()));
  // Cannot construct TypedArray because Heap::TypedArray symbol is not exposed.
  // Scoped<TypedArray> oneof_cases(scope,
  //                                v4->memoryManager->alloc<TypedArray>(
  //                                    v4, QV4::Heap::TypedArray::Int32Array));
  // Scoped<ArrayBuffer> buffer(
  //     scope, v4->memoryManager->alloc<ArrayBuffer>(
  //                v4, message->oneof_decl_count() * sizeof(OneofIndex)));
  // oneof_cases->d()->buffer = buffer->d();
  // oneof_cases->d()->byteLength = buffer->byteLength();
  // oneof_cases->d()->byteOffset = 0;

  Scoped<ArrayBuffer> oneof_cases(
      scope, v4->memoryManager->alloc<ArrayBuffer>(
                 v4, message->oneof_decl_count() * sizeof(OneofIndex)));

  bool parsed = false;
  auto reflection = msg.GetReflection();

  // Storing selected oneof number for each oneof declarations
  QVector<int> oneof_numbers(message->oneof_decl_count(), -1);

  ScopedValue v(scope);
  for (int i = 0; i < message->field_count(); ++i) {
    auto field = message->field(i);
    auto oneof = field->containing_oneof();
    if (oneof) {
      auto oneof_number = &oneof_numbers[oneof->index()];
      if (*oneof_number < 0) {
        // Store actual oneof number when looking up for the first time.
        auto oneof_field = reflection->GetOneofFieldDescriptor(msg, oneof);
        *oneof_number = oneof_field ? oneof_field->number() : 0;
        Q_ASSERT(*oneof_number <= std::numeric_limits<OneofIndex>::max());
        if (*oneof_number > 0) {
          parsed = true;
          oneof_numbers[oneof->index()] = *oneof_number;
          auto res = typed_put_indexed(oneof_cases, oneof->index(),
                                       static_cast<OneofIndex>(*oneof_number));
          Q_ASSERT(res);
        }
      }
      if (*oneof_number != field->number()) {
        // Ignore any value for not-current oneof fields.
        field_values->putIndexed(i, Primitive::undefinedValue());
        continue;
      }
    }
    if (field->is_repeated()) {
      auto size = reflection->FieldSize(msg, field);
      if (size > 0) {
        parsed = true;
        v = getRepeatedFieldValue(v4, *reflection, msg, field, size);
        field_values->putIndexed(i, v);
      } else {
        field_values->putIndexed(i, Primitive::undefinedValue());
      }
    } else if (reflection->HasField(msg, field)) {
      parsed = true;
      v = getFieldValue(v4, *reflection, msg, field);
      field_values->putIndexed(i, v);
    } else {
      field_values->putIndexed(i, Primitive::undefinedValue());
    }
  }

  if (parsed) {
    ScopedArrayObject ret(scope, v4->newArrayObject(2));
    ret->putIndexed(0, field_values);
    ret->putIndexed(1, oneof_cases);
    return ret->asReturnedValue();
  }
  return Encode::undefined();
}

ReturnedValue Descriptor::getFieldValue(
    ExecutionEngine* v4,
    const google::protobuf::Reflection& ref,
    const google::protobuf::Message& msg,
    const google::protobuf::FieldDescriptor* field) {
  Scope scope(v4);
  ScopedValue v(scope);
  if (field->cpp_type() == FieldDescriptor::CPPTYPE_INT32) {
    v = Primitive::fromInt32(ref.GetInt32(msg, field));
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_INT64) {
    auto value = ref.GetInt64(msg, field);
    if (value <= std::numeric_limits<int32_t>::max() &&
        value >= std::numeric_limits<int32_t>::lowest()) {
      v = Primitive::fromInt32(value);
    } else {
      v = v4->fromVariant(qint64(value));
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_UINT32) {
    v = Primitive::fromUInt32(ref.GetUInt32(msg, field));
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_UINT64) {
    auto value = ref.GetUInt64(msg, field);
    if (value <= std::numeric_limits<uint32_t>::max()) {
      v = Primitive::fromUInt32(value);
    } else {
      v = v4->fromVariant(quint64(ref.GetUInt64(msg, field)));
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE) {
    v = Primitive::fromDouble(ref.GetDouble(msg, field));
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_FLOAT) {
    v = Primitive::fromDouble(ref.GetFloat(msg, field));
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_BOOL) {
    v = Primitive::fromBoolean(ref.GetBool(msg, field));
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING &&
             field->type() == FieldDescriptor::TYPE_BYTES) {
    v = v4->newArrayBuffer(
        QByteArray::fromStdString(ref.GetString(msg, field)));
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
    v = v4->newString(QString::fromStdString(ref.GetString(msg, field)));
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
    v = Primitive::fromInt32(ref.GetEnum(msg, field)->number());
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
    return messageToJsValue(v4, ref.GetMessage(msg, field));
  } else {
    Q_ASSERT(false);
  }
  return v->asReturnedValue();
}

ReturnedValue Descriptor::getRepeatedFieldValue(
    ExecutionEngine* v4,
    const google::protobuf::Reflection& ref,
    const google::protobuf::Message& msg,
    const google::protobuf::FieldDescriptor* field,
    int size) {
  Scope scope(v4);
  ScopedValue v(scope);
  if (field->cpp_type() == FieldDescriptor::CPPTYPE_INT32) {
    // Cannot construct TypedArray because Heap::TypedArray symbol is not
    // exposed.
    // Scoped<TypedArray> vs(scope, v4->memoryManager->alloc<TypedArray>(
    //                                  v4, QV4::Heap::TypedArray::Int32Array));
    // if (!vs) {
    //   return Encode::null();
    // }
    // Scoped<ArrayBuffer> buffer(
    //     scope, v4->memoryManager->alloc<ArrayBuffer>(
    //                v4, ((size - 1) / 8 + 1) * 8 * sizeof(int32_t)));
    // if (!buffer) {
    //   return Encode::null();
    // }
    // vs->d()->buffer = buffer->d();
    // vs->d()->byteLength = size * sizeof(int32_t);
    // vs->d()->byteOffset = 0;

    // We set buffer.byteLength to what is exactly needed so that JS side code
    // knows actual typed array length to construct.
    Scoped<ArrayBuffer> vs(scope, v4->memoryManager->alloc<ArrayBuffer>(
                                      v4, size * sizeof(int32_t)));
    if (!vs) {
      return Encode::null();
    }
    for (int i = 0; i < size; i++) {
      auto res = typed_put_indexed(vs, i, ref.GetRepeatedInt32(msg, field, i));
      Q_ASSERT(res);
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_INT64) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      auto value = ref.GetRepeatedInt64(msg, field, i);
      if (value <= std::numeric_limits<int32_t>::max() &&
          value >= std::numeric_limits<int32_t>::lowest()) {
        v = Primitive::fromInt32(value);
        vs->putIndexed(i, v);
      } else {
        v = v4->fromVariant(qint64());
        vs->putIndexed(i, v);
      }
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_UINT32) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      v = Primitive::fromUInt32(ref.GetRepeatedUInt32(msg, field, i));
      vs->putIndexed(i, v);
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_UINT64) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      auto value = ref.GetRepeatedUInt64(msg, field, i);
      if (value <= std::numeric_limits<uint32_t>::max()) {
        v = Primitive::fromUInt32(value);
        vs->putIndexed(i, v);
      } else {
        v = v4->fromVariant(quint64(ref.GetRepeatedUInt64(msg, field, i)));
        vs->putIndexed(i, v);
      }
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      v = Primitive::fromDouble(ref.GetRepeatedDouble(msg, field, i));
      vs->putIndexed(i, v);
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_FLOAT) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      v = Primitive::fromDouble(ref.GetRepeatedFloat(msg, field, i));
      vs->putIndexed(i, v);
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_BOOL) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      v = Primitive::fromBoolean(ref.GetRepeatedBool(msg, field, i));
      vs->putIndexed(i, v);
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING &&
             field->type() == FieldDescriptor::TYPE_BYTES) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      v = v4->newArrayBuffer(
          QByteArray::fromStdString(ref.GetRepeatedString(msg, field, i)));
      vs->putIndexed(i, v);
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      v = v4->newString(
          QString::fromStdString(ref.GetRepeatedString(msg, field, i)));
      vs->putIndexed(i, v);
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      v = Primitive::fromInt32(ref.GetRepeatedEnum(msg, field, i)->number());
      vs->putIndexed(i, v);
    }
    return vs->asReturnedValue();
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
    ScopedArrayObject vs(scope, v4->newArrayObject(size));
    for (int i = 0; i < size; i++) {
      v = messageToJsValue(v4, ref.GetRepeatedMessage(msg, field, i));
      vs->putIndexed(i, v);
    }
    return vs->asReturnedValue();
  } else {
    Q_ASSERT(false);
  }
}

void Descriptor::setFieldValue(ExecutionEngine* v4,
                               const google::protobuf::Reflection& ref,
                               google::protobuf::Message& msg,
                               const google::protobuf::FieldDescriptor* field,
                               ReturnedValue value) {
  Scope scope(v4);
  ScopedValue v(scope, value);
  if (v->isNullOrUndefined()) {
    return;
  }
  if (field->cpp_type() == FieldDescriptor::CPPTYPE_INT32) {
    ref.SetInt32(&msg, field, v->toInt32());
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_INT64) {
    if (v->isInteger()) {
      ref.SetInt64(&msg, field, v->toInt32());
    } else {
      auto var = v4->toVariant(v, 0, false);
      Q_ASSERT(var.isValid());
      ref.SetInt64(&msg, field, static_cast<int64>(var.value<qint64>()));
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_UINT32) {
    ref.SetUInt32(&msg, field, v->toUInt32());
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_UINT64) {
    if (v->isInteger()) {
      ref.SetUInt64(&msg, field, v->toUInt32());
    } else {
      auto var = v4->toVariant(v, 0, false);
      Q_ASSERT(var.isValid());
      ref.SetUInt64(&msg, field, static_cast<uint64>(var.value<quint64>()));
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE) {
    ref.SetDouble(&msg, field, v->toNumber());
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_FLOAT) {
    ref.SetFloat(&msg, field, v->toNumber());
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_BOOL) {
    ref.SetBool(&msg, field, v->toBoolean());
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING &&
             field->type() == FieldDescriptor::TYPE_BYTES) {
    Scoped<ArrayBuffer> array(scope, value);
    if (array) {
      ref.SetString(&msg, field, array->asByteArray().toStdString());
    } else {
      auto var = v4->toVariant(v, 0, false);
      Q_ASSERT(var.canConvert(QMetaType::QByteArray));
      ref.SetString(&msg, field, var.value<QByteArray>().toStdString());
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
    ScopedString v(scope, value);
    Q_ASSERT(v);
    ref.SetString(&msg, field, v->toQString().toStdString());
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
    ref.SetEnum(&msg, field,
                field->enum_type()->FindValueByNumber(v->toInt32()));
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
    auto sub_msg = ref.MutableMessage(&msg, field);
    Q_ASSERT(sub_msg);
    ScopedArrayObject v(scope, value);
    jsValueToMessage(v4, *v, *sub_msg);
  } else {
    Q_ASSERT(false);
  }
}

void Descriptor::setRepeatedFieldValue(
    ExecutionEngine* v4,
    const google::protobuf::Reflection& ref,
    google::protobuf::Message& msg,
    const google::protobuf::FieldDescriptor* field,
    QV4::ReturnedValue value) {
  Scope scope(v4);
  if (field->cpp_type() == FieldDescriptor::CPPTYPE_INT32) {
    Scoped<TypedArray> list(scope, value);
    if (!list) return;
    auto size = list->length();
    ScopedValue v(scope);
    for (int i = 0; i < size; i++) {
      int32_t v;
      if (typed_get_indexed(list, i, &v)) {
        ref.AddInt32(&msg, field, v);
      }
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_INT64) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedValue v(scope);
    for (int i = 0; i < size; i++) {
      v = list->getIndexed(i);
      if (!v->isNullOrUndefined()) {
        if (v->isInteger()) {
          ref.AddInt64(&msg, field, v->toInt32());
        } else {
          auto var = v4->toVariant(v, 0, false);
          ref.AddInt64(&msg, field, static_cast<int64>(var.value<qint64>()));
        }
      }
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_UINT32) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedValue v(scope);
    for (int i = 0; i < size; i++) {
      v = list->getIndexed(i);
      if (!v->isNullOrUndefined()) ref.AddUInt32(&msg, field, v->toUInt32());
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_UINT64) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedValue v(scope);
    for (int i = 0; i < size; i++) {
      v = list->getIndexed(i);
      if (!v->isNullOrUndefined()) {
        if (v->isInteger()) {
          ref.AddUInt64(&msg, field, v->toUInt32());
        } else {
          auto var = v4->toVariant(v, 0, false);
          ref.AddUInt64(&msg, field, static_cast<uint64>(var.value<quint64>()));
        }
      }
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedValue v(scope);
    for (int i = 0; i < size; i++) {
      v = list->getIndexed(i);
      if (!v->isNullOrUndefined()) ref.AddDouble(&msg, field, v->toNumber());
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_FLOAT) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedValue v(scope);
    for (int i = 0; i < size; i++) {
      v = list->getIndexed(i);
      if (!v->isNullOrUndefined()) ref.AddFloat(&msg, field, v->toNumber());
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_BOOL) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedValue v(scope);
    for (int i = 0; i < size; i++) {
      v = list->getIndexed(i);
      if (!v->isNullOrUndefined()) ref.AddBool(&msg, field, v->toBoolean());
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING &&
             field->type() == FieldDescriptor::TYPE_BYTES) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedValue v(scope);
    Scoped<ArrayBuffer> array(scope);
    for (int i = 0; i < size; i++) {
      array = list->getIndexed(i);
      if (array) {
        ref.AddString(&msg, field, array->asByteArray().toStdString());
      } else {
        v = list->getIndexed(i);
        auto var = v4->toVariant(v, 0, false);
        Q_ASSERT(var.canConvert(QMetaType::QByteArray));
        ref.SetString(&msg, field, var.value<QByteArray>().toStdString());
      }
      v = list->getIndexed(i);
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedString v(scope);
    for (int i = 0; i < size; i++) {
      v = list->getIndexed(i);
      if (!v->isNullOrUndefined())
        ref.AddString(&msg, field, v->toQString().toStdString());
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedValue v(scope);
    for (int i = 0; i < size; i++) {
      v = list->getIndexed(i);
      if (!v->isNullOrUndefined())
        ref.AddEnum(&msg, field,
                    field->enum_type()->FindValueByNumber(v->toInt32()));
    }
  } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
    ScopedArrayObject list(scope, value);
    if (!list) return;
    auto size = list->getLength();
    ScopedArrayObject v(scope);
    for (int i = 0; i < size; i++) {
      v = list->getIndexed(i);
      if (!v->isNullOrUndefined())
        jsValueToMessage(v4, *v, *ref.AddMessage(&msg, field));
    }
  } else {
    Q_ASSERT(false);
  }
}

SerializeTask::SerializeTask(std::unique_ptr<google::protobuf::Message> msg,
                             QV4::ExecutionEngine* v4,
                             const QV4::Value& callback)
    : msg_(std::move(msg)), v4_(v4) {
  setAutoDelete(false);
  connect(this, &SerializeTask::done, this, &SerializeTask::onDone);
  callback_.set(v4_, packCallbackObject(v4_, callback));
}

void SerializeTask::run() {
  QByteArray ba(msg_->ByteSize(), 0);
  if (!msg_->SerializeToArray(ba.data(), ba.size())) {
    qmlInfo(this) << __func__ << " Failed to serialize message object";
  }
  done(ba);
}

void SerializeTask::onDone(const QByteArray& ba) {
  Scope scope(v4_);

  auto unpack = unpackCallbackObject(v4_, callback_.value());
  ScopedObject cb(scope, unpack.second);
  if (!cb) {
    qDebug() << "No callback";
    return;
  }
  // Force copying
  Scoped<ArrayBuffer> buf(
      scope, v4_->newArrayBuffer(QByteArray(ba.data(), ba.size())));
  ScopedCallData cdata(scope, 2);
  cdata->thisObject = unpack.first;
  cdata->args[0] = QV4::Primitive::nullValue();
  cdata->args[1] = buf;
  cb->call(cdata);
  delete this;
}

ParseTask::ParseTask(Descriptor* p,
                     google::protobuf::Message* msg,
                     QByteArray buf,
                     QV4::ExecutionEngine* v4,
                     const QV4::Value& callback)
    : p_(p), msg_(msg), buf_(std::move(buf)), v4_(v4) {
  setAutoDelete(false);
  connect(this, &ParseTask::done, this, &ParseTask::onDone);
  Scope scope(v4_);
  callback_.set(v4_, packCallbackObject(v4_, callback));
}

void ParseTask::run() {
  if (!msg_->ParseFromArray(buf_.data(), buf_.size())) {
    qmlInfo(this) << __func__ << " Failed to parse message object";
  }
  done();
}

void ParseTask::onDone() {
  Scope scope(v4_);
  auto unpack = unpackCallbackObject(v4_, callback_.value());
  ScopedObject cb(scope, unpack.second);
  if (!cb) {
    qDebug() << "No callback";
    return;
  }
  ScopedCallData cdata(scope, 2);
  cdata->thisObject = unpack.first;
  cdata->args[0] = QV4::Primitive::nullValue();
  cdata->args[1] = p_->messageToJsValue(v4_, *msg_);
  cb->call(cdata);

  delete this;
}
}
}
