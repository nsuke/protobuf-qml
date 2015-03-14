#include "protobuf/qml/descriptor_database.h"
#include "protobuf/qml/util.h"
#include <google/protobuf/descriptor.pb.h>

#include <QVariantList>
#include <sstream>
#include <vector>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

DescriptorWrapper::~DescriptorWrapper() {
  clearSharedMessage();
}

bool packFieldsToMessage(const QVariantList& value, Message& msg);
QVariantList unpackFieldsFromMessage(const Message& msg);

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
        ref.AddString(&msg, field, list[i].value<QString>().toStdString());
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      for (int i = 0; i < size; i++)
        ref.AddEnum(
            &msg,
            field,
            field->enum_type()->FindValueByNumber(list[i].value<int>()));
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      for (int i = 0; i < size; i++)
        packFieldsToMessage(list[i].value<QVariantList>(),
                            *ref.AddMessage(&msg, field));
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
    packFieldsToMessage(value.value<QVariantList>(), *sub_msg);
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
      ref.SetString(&msg, field, value.value<QString>().toStdString());
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      ref.SetEnum(&msg,
                  field,
                  field->enum_type()->FindValueByNumber(value.value<int>()));
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      setMessage(ref, msg, field, value);
      break;
  }
}

QVariantList getReflectionRepeatedValue(const Reflection& ref,
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
      for (int i = 0; i < size; i++)
        result.append(
            QString::fromStdString(ref.GetRepeatedString(msg, field, i)));
      break;

    case FieldDescriptor::CPPTYPE_ENUM:
      for (int i = 0; i < size; i++)
        result.append(ref.GetRepeatedEnum(msg, field, i)->number());
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      for (int i = 0; i < size; i++)
        result.append(
            unpackFieldsFromMessage(ref.GetRepeatedMessage(msg, field, i)));
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
      return unpackFieldsFromMessage(ref.GetMessage(msg, field));
  }
  return QVariant();
}

QVariantList unpackFieldsFromMessage(const Message& msg) {
  QVariantList result;
  auto reflection = msg.GetReflection();
  auto descriptor = msg.GetDescriptor();
  auto field_count = descriptor->field_count();
  if (field_count > 0) {
    for (int i = 0; i < field_count; i++) {
      auto field_descriptor = descriptor->field(i);
      QVariantList field;
      if (field_descriptor->is_repeated()) {
        auto size = reflection->FieldSize(msg, field_descriptor);
        if (size > 0) {
          result.append(QVariant(getReflectionRepeatedValue(
              *reflection, msg, field_descriptor, size)));
        } else {
          result.append(QVariant());
        }
      } else if (reflection->HasField(msg, field_descriptor)) {
        result.append(getReflectionValue(*reflection, msg, field_descriptor));
      } else {
        result.append(QVariant());
      }
      qDebug() << "result : " << result;
      Q_ASSERT(result.size() == i + 1);
    }
  }
  // TODO: restore oneof
  // auto oneof_count = descriptor->oneof_decl_count();
  // if (oneof_count > 0) {
  //  for (int i = 0; i < oneof_count; i++) {
  //    auto oneof_descriptor = descriptor->oneof_decl(i);
  //    if (reflection->HasOneof(msg, oneof_descriptor)) {
  //      auto field_descriptor =
  //          reflection->GetOneofFieldDescriptor(msg, oneof_descriptor);
  //      auto value = getReflectionValue(*reflection, msg, field_descriptor);
  //      if (value.isValid()) {
  //        auto field_name = field_descriptor->camelcase_name();
  //        auto oneof_name = camelize(oneof_descriptor->name());
  //        QVariantMap oneof;
  //        oneof.insert(QString::fromStdString(field_name), std::move(value));
  //        result.insert(QString::fromStdString(oneof_name), std::move(oneof));
  //        break;
  //      }
  //    }
  //  }
  //}
  return std::move(result);
}

QVariantList DescriptorWrapper::parse(InputDevice* input) {
  if (!input) return QVariantList();
  auto msg = sharedMessage();
  msg->Clear();
  auto session = input->createSession();
  if (!session) return QVariantList();
  msg->ParseFromZeroCopyStream(session.stream());
  return unpackFieldsFromMessage(*msg);
}

bool packFieldsToMessage(const QVariantList& fields, Message& msg) {
  auto reflection = msg.GetReflection();
  auto descriptor = msg.GetDescriptor();
  auto field_count = descriptor->field_count();
  for (int i = 0; i < field_count && i < fields.size(); ++i) {
    auto field = fields[i];
    if (field.isValid()) {
      auto desc = descriptor->field(i);
      if (desc->is_repeated()) {
        if (!field.canConvert(QMetaType::QVariantList)) {
          qWarning() << "Invalid type for repeated field: "
                     << QString::fromStdString(desc->name());
        } else {
          auto list = field.value<QVariantList>();
          auto size = list.size();
          if (size > 0) {
            setReflectionRepeatedValue(*reflection, msg, desc, list, size);
          }
        }
      } else {
        qDebug() << "field : " << field;
        setReflectionValue(*reflection, msg, desc, field);
      }
    }
  }
  // TODO: restore oneof
  // for (int i = 0; i < descriptor->oneof_decl_count(); i++) {
  //   auto oneof_descriptor = descriptor->oneof_decl(i);
  //   auto oneof_name = camelize(oneof_descriptor->name());
  //   auto it = value.find(QString::fromStdString(oneof_name));
  //   if (it != value.end()) {
  //     if (!it.value().canConvert(QMetaType::QVariantMap)) {
  //       qWarning() << "Invalid type for oneof field: "
  //                  << QString::fromStdString(oneof_name);
  //     } else {
  //       auto oneof = it.value().value<QVariantMap>();
  //       for (int j = 0; j < oneof_descriptor->field_count(); j++) {
  //         auto field_descriptor = oneof_descriptor->field(j);
  //         auto field_name = field_descriptor->camelcase_name();
  //         auto it2 = oneof.constFind(QString::fromStdString(field_name));
  //         if (it2 != oneof.constEnd() && it2.value().isValid()) {
  //           setReflectionValue(*reflection, msg, field_descriptor,
  //           it2.value());
  //           break;
  //         }
  //       }
  //     }
  //     value.erase(it);
  //   }
  // }
  // for (auto it = value.constBegin(); it != value.constEnd(); it++) {
  //   qWarning() << "Unexpected field: " << it.key();
  // }
  return true;
}

bool DescriptorWrapper::serialize(OutputDevice* output,
                                   const QVariantList& value) {
  try {
    if (!output || value.isEmpty()) return false;
    auto msg = sharedMessage();
    msg->Clear();
    if (packFieldsToMessage(value, *msg)) {
      auto session = output->createSession();
      return msg->SerializeToZeroCopyStream(session.stream());
    }
    return false;
  } catch (google::protobuf::FatalException& ex) {
    qWarning() << "Serialize failed : " << ex.what();
    return false;
  } catch (std::exception& ex) {
    qWarning() << "Serialize failed : " << ex.what();
    return false;
  }
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
