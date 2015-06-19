#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/util.h"
#include <google/protobuf/descriptor.pb.h>

#include <QDebug>
#include <QVariantList>
#include <sstream>
#include <vector>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

DescriptorWrapper::~DescriptorWrapper() {
  clearSharedMessage();
}

bool packToMessage(const QVariantList& fields,
                   const QList<int>& oneofs,
                   Message& msg);
bool packToMessage(QVariant value, Message& msg) {
  if (!value.canConvert(QMetaType::QVariantList)) {
    return false;
  }
  auto vlist = value.value<QVariantList>();
  if (vlist.size() < 2 || !vlist[0].canConvert(QMetaType::QVariantList)) {
    return false;
  }

  QList<int> oneof_numbers;
  if (vlist[1].canConvert(QMetaType::QVariantList)) {
    for (auto& v : vlist[1].value<QVariantList>()) {
      if (!v.canConvert(QMetaType::Int)) {
        qWarning() << "Invalid type found in oneof numbers.";
        return false;
      }
      oneof_numbers << v.value<int>();
    }
  }

  return packToMessage(vlist[0].value<QVariantList>(), oneof_numbers, msg);
}

QVariant unpackFromMessage(const Message& msg);

Message* DescriptorWrapper::dataToMessage(const QVariant& msgData) {
  auto msg = newMessage();
  packToMessage(msgData, *msg);
  return msg;
}

QVariant DescriptorWrapper::dataFromMessage(const Message& msg) {
  return unpackFromMessage(msg);
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
inline std::string QByteArrayToStdString(const QByteArray& ba) {
  return ba.toStdString();
}
inline QByteArray QByteArrayFromStdString(const std::string& str) {
  return QByteArray::fromStdString(str);
}
#else
inline std::string QByteArrayToStdString(const QByteArray& ba) {
  return std::string(ba.data(), ba.size());
}
inline QByteArray QByteArrayFromStdString(const std::string& str) {
  return QByteArray(str.data(), str.size());
}
#endif

// TODO: Handle invalid QVariant that cannot be converted
void setReflectionRepeatedValue(const Reflection& ref,
                                Message& msg,
                                const FieldDescriptor* field,
                                const QVariantList& list,
                                int size) {
#define PROTOBUF_QML_ADD_REPEATED(TYPE_ENUM, TYPE, CPP_TYPE)           \
  case FieldDescriptor::CPPTYPE_##TYPE_ENUM:                           \
    for (int i = 0; i < size; i++)                                     \
      ref.Add##TYPE(&msg, field, CPP_TYPE(list[i].value<CPP_TYPE>())); \
    break;

  switch (field->cpp_type()) {
    PROTOBUF_QML_ADD_REPEATED(INT32, Int32, int32);
    PROTOBUF_QML_ADD_REPEATED(INT64, Int64, int64);
    PROTOBUF_QML_ADD_REPEATED(UINT32, UInt32, uint32);
    PROTOBUF_QML_ADD_REPEATED(UINT64, UInt64, uint64);
    PROTOBUF_QML_ADD_REPEATED(DOUBLE, Double, double);
    PROTOBUF_QML_ADD_REPEATED(FLOAT, Float, float);
    PROTOBUF_QML_ADD_REPEATED(BOOL, Bool, bool);
    case FieldDescriptor::CPPTYPE_STRING:
      for (int i = 0; i < size; i++)
        if (field->type() == FieldDescriptor::TYPE_BYTES) {
          ref.AddString(&msg, field,
                        QByteArrayToStdString(list[i].value<QByteArray>()));
        } else {
          ref.AddString(&msg, field, list[i].value<QString>().toStdString());
        }
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      for (int i = 0; i < size; i++)
        ref.AddEnum(&msg, field, field->enum_type()->FindValueByNumber(
                                     list[i].value<int>()));
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      for (int i = 0; i < size; i++) {
        packToMessage(list[i].value<QVariantList>(),
                      *ref.AddMessage(&msg, field));
      }
      break;
  }
#undef PROTOBUF_QML_SET_REPEATED
}

void setMessage(const Reflection& ref,
                Message& msg,
                const FieldDescriptor* field,
                const QVariant& value) {
  auto sub_msg = ref.MutableMessage(&msg, field);
  Q_ASSERT(sub_msg);
  if (value.canConvert(QMetaType::QVariantList)) {
    packToMessage(value.value<QVariantList>(), *sub_msg);
  }
}

void setReflectionValue(const Reflection& ref,
                        Message& msg,
                        const FieldDescriptor* field,
                        const QVariant& value) {
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      ref.SetInt32(&msg, field, value.value<int>());
      break;
    case FieldDescriptor::CPPTYPE_INT64:
      ref.SetInt64(&msg, field, static_cast<int64>(value.value<qint64>()));
      break;
    case FieldDescriptor::CPPTYPE_UINT32:
      ref.SetUInt32(&msg, field, value.value<uint32_t>());
      break;
    case FieldDescriptor::CPPTYPE_UINT64:
      ref.SetUInt64(&msg, field, static_cast<uint64>(value.value<quint64>()));
      break;
    case FieldDescriptor::CPPTYPE_DOUBLE:
      ref.SetDouble(&msg, field, value.value<double>());
      break;
    case FieldDescriptor::CPPTYPE_FLOAT:
      ref.SetFloat(&msg, field, value.value<float>());
      break;
    case FieldDescriptor::CPPTYPE_BOOL:
      ref.SetBool(&msg, field, value.value<bool>());
      break;
    case FieldDescriptor::CPPTYPE_STRING:
      if (field->type() == FieldDescriptor::TYPE_BYTES) {
        ref.SetString(&msg, field,
                      QByteArrayToStdString(value.value<QByteArray>()));
      } else {
        ref.SetString(&msg, field, value.value<QString>().toStdString());
      }
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      ref.SetEnum(&msg, field,
                  field->enum_type()->FindValueByNumber(value.value<int>()));
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      setMessage(ref, msg, field, value);
      break;
  }
}

QVariant getReflectionRepeatedValue(const Reflection& ref,
                                    const Message& msg,
                                    const FieldDescriptor* field,
                                    int size) {
  qDebug() << "getReflectionRepeatedValue";
  QVariantList result;
  switch (field->cpp_type()) {
#define PROTOBUF_QML_GET_REPEATED(TYPE_ENUM, TYPE, QT_TYPE)            \
  case FieldDescriptor::CPPTYPE_##TYPE_ENUM:                           \
    for (int i = 0; i < size; i++)                                     \
      result.append(                                                   \
          static_cast<QT_TYPE>(ref.GetRepeated##TYPE(msg, field, i))); \
    break;

    PROTOBUF_QML_GET_REPEATED(INT32, Int32, qint32);
    PROTOBUF_QML_GET_REPEATED(INT64, Int64, qint64);
    PROTOBUF_QML_GET_REPEATED(UINT32, UInt32, quint32);
    PROTOBUF_QML_GET_REPEATED(UINT64, UInt64, quint64);
    PROTOBUF_QML_GET_REPEATED(DOUBLE, Double, double);
    PROTOBUF_QML_GET_REPEATED(FLOAT, Float, float);
    PROTOBUF_QML_GET_REPEATED(BOOL, Bool, bool);
#undef PROTOBUF_QML_GET_REPEATED

    case FieldDescriptor::CPPTYPE_STRING:
      if (field->type() == FieldDescriptor::TYPE_BYTES) {
        for (int i = 0; i < size; i++)
          result.append(
              QByteArrayFromStdString(ref.GetRepeatedString(msg, field, i)));
      } else {
        for (int i = 0; i < size; i++)
          result.append(
              QString::fromStdString(ref.GetRepeatedString(msg, field, i)));
      }
      break;

    case FieldDescriptor::CPPTYPE_ENUM:
      for (int i = 0; i < size; i++)
        result.append(ref.GetRepeatedEnum(msg, field, i)->number());
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      for (int i = 0; i < size; i++)
        result.append(unpackFromMessage(ref.GetRepeatedMessage(msg, field, i)));
      break;
  }
  return result;
}

QVariant getReflectionValue(const Reflection& ref,
                            const Message& msg,
                            const FieldDescriptor* field) {
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      return ref.GetInt32(msg, field);
    case FieldDescriptor::CPPTYPE_INT64:
      return static_cast<qint64>(ref.GetInt64(msg, field));
    case FieldDescriptor::CPPTYPE_UINT32:
      return ref.GetUInt32(msg, field);
    case FieldDescriptor::CPPTYPE_UINT64:
      return static_cast<quint64>(ref.GetUInt64(msg, field));
    case FieldDescriptor::CPPTYPE_DOUBLE:
      return ref.GetDouble(msg, field);
    case FieldDescriptor::CPPTYPE_FLOAT:
      return ref.GetFloat(msg, field);
    case FieldDescriptor::CPPTYPE_BOOL:
      return ref.GetBool(msg, field);
    case FieldDescriptor::CPPTYPE_STRING:
      return QString::fromStdString(ref.GetString(msg, field));
    case FieldDescriptor::CPPTYPE_ENUM:
      return ref.GetEnum(msg, field)->number();
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return unpackFromMessage(ref.GetMessage(msg, field));
  }
  return QVariant();
}

QVariant unpackFromMessage(const Message& msg) {
  QVariantList result;
  bool parsed = false;
  auto reflection = msg.GetReflection();
  auto message = msg.GetDescriptor();
  // Storing selected oneof number for each oneof declarations
  QVector<int> oneof_numbers(message->oneof_decl_count(), -1);

  for (int i = 0; i < message->field_count(); ++i) {
    auto field = message->field(i);
    auto oneof = field->containing_oneof();
    if (oneof) {
      auto oneof_number = &oneof_numbers[oneof->index()];
      if (*oneof_number < 0) {
        // Store actual oneof number when looking up for the first time.
        auto oneof_field = reflection->GetOneofFieldDescriptor(msg, oneof);
        *oneof_number = oneof_field ? oneof_field->number() : 0;
        if (*oneof_number > 0) {
          parsed = true;
        }
      }
      if (*oneof_number != field->number()) {
        // Ignore any value for not-current oneof fields.
        result.append(QVariant());
        continue;
      }
    }
    if (field->is_repeated()) {
      auto size = reflection->FieldSize(msg, field);
      if (size > 0) {
        parsed = true;
        result.append(QVariant(
            getReflectionRepeatedValue(*reflection, msg, field, size)));
      } else {
        // Repeated field with no element.
        result.append(QVariant());
      }
    } else if (reflection->HasField(msg, field)) {
      parsed = true;
      result.append(getReflectionValue(*reflection, msg, field));
    } else {
      result.append(QVariant());
    }
    // qDebug() << "result : " << result;
    Q_ASSERT(result.size() == i + 1);
  }

  if (parsed) {
    QVariantList ret;
    ret.append(QVariant::fromValue(result));
    ret.append(QVariant::fromValue(oneof_numbers.toList()));
    return std::move(ret);
  }
  return QVariant();
}

bool packToMessage(const QVariantList& raw_fields,
                   const QList<int>& oneof_numbers,
                   Message& msg) {
  auto reflection = msg.GetReflection();
  auto descriptor = msg.GetDescriptor();
  auto field_count = descriptor->field_count();
  for (int i = 0; i < field_count && i < raw_fields.size(); ++i) {
    auto raw_field = raw_fields[i];
    auto field = descriptor->field(i);
    auto oneof = field->containing_oneof();
    if (oneof) {
    }
    if ((!field->containing_oneof() && raw_field.isValid()) ||
        (oneof && oneof_numbers.size() > oneof->index() &&
         oneof_numbers[oneof->index()] == field->number())) {
      if (field->is_repeated()) {
        if (!raw_field.canConvert(QMetaType::QVariantList)) {
          qWarning() << "Invalid type for repeated field: "
                     << QString::fromStdString(field->name());
        } else {
          auto list = raw_field.value<QVariantList>();
          auto size = list.size();
          if (size > 0) {
            setReflectionRepeatedValue(*reflection, msg, field, list, size);
          }
        }
      } else {
        setReflectionValue(*reflection, msg, field, raw_field);
      }
    }
  }
  return true;
}

FileDescriptorWrapper* DescriptorPoolWrapper::addFileDescriptor(
    QVariant encoded) {
  if (!encoded.canConvert<QByteArray>()) {
    return nullptr;
  }
  auto ba = QByteArray::fromBase64(encoded.value<QByteArray>());

  FileDescriptorProto pb;
  if (pb.ParseFromArray(ba.data(), ba.size())) {
    if (auto desc = pool_.BuildFile(pb)) {
      auto file = new FileDescriptorWrapper(desc);
      children_.emplace_back(file);
      return file;
    }
  }
  return nullptr;
}
}
}
