#include "qpb/descriptor_database.h"
#include <google/protobuf/descriptor.pb.h>

#include <QVariantList>
#include <sstream>
#include <vector>

namespace qpb {

using namespace google::protobuf;

bool packToMessage(const QVariantMap& value, Message& msg);
QVariant unpackFromMessage(const Message& msg);

void AsyncProcessor::doParse(int key, InputDevice* input) {
  Q_ASSERT(!has_task_);
  has_task_ = true;
  parent_->parseCompleted(key, parent_->parse(input));
  has_task_ = false;
}

void AsyncProcessor::doSerialize(int key,
                                 OutputDevice* output,
                                 QVariantMap value) {
  Q_ASSERT(!has_task_);
  has_task_ = true;
  parent_->serialize(output, value);
  parent_->serializeCompleted(key);
  has_task_ = false;
}

DescriptorWrapper::~DescriptorWrapper() {
  for (auto& a : async_) {
    a->quit();
    a->wait();
  }
  message_.setLocalData(nullptr);
}

void setReflectionRepeatedValue(const Reflection& ref,
                                Message& msg,
                                const FieldDescriptor* field,
                                const QVariantList& list,
                                int size) {
  // TODO: Handle invalid QVariant that cannot be converted
#define QPB_ADD_REPEATED(TYPE_ENUM, TYPE, CPP_TYPE)                    \
  case FieldDescriptor::CPPTYPE_##TYPE_ENUM:                           \
    for (int i = 0; i < size; i++)                                     \
      ref.Add##TYPE(&msg, field, CPP_TYPE(list[i].value<CPP_TYPE>())); \
    break;

  switch (field->cpp_type()) {
    QPB_ADD_REPEATED(INT32, Int32, int32);
    QPB_ADD_REPEATED(INT64, Int64, int64);
    QPB_ADD_REPEATED(UINT32, UInt32, uint32);
    QPB_ADD_REPEATED(UINT64, UInt64, uint64);
    QPB_ADD_REPEATED(DOUBLE, Double, double);
    QPB_ADD_REPEATED(FLOAT, Float, float);
    QPB_ADD_REPEATED(BOOL, Bool, bool);
    case FieldDescriptor::CPPTYPE_STRING:
      for (int i = 0; i < size; i++)
        ref.AddString(&msg, field, list[i].value<QString>().toStdString());
      break;

    case FieldDescriptor::CPPTYPE_ENUM:
    // TODO: add enum support
    case FieldDescriptor::CPPTYPE_MESSAGE:
      for (int i = 0; i < size; i++)
        packToMessage(list[i].value<QVariantMap>(),
                      *ref.AddMessage(&msg, field));
      break;
  }
#undef QPB_SET_REPEATED
}

void setMessage(const Reflection& ref,
                Message& msg,
                const FieldDescriptor* field,
                const QVariant& value) {
  auto sub_msg = ref.MutableMessage(&msg, field);
  Q_ASSERT(sub_msg);
  if (value.canConvert(QMetaType::QVariantMap)) {
    packToMessage(value.value<QVariantMap>(), *sub_msg);
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
      // TODO: add enum support
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
#define QPB_GET_REPEATED(TYPE_ENUM, TYPE, QT_TYPE)                     \
  case FieldDescriptor::CPPTYPE_##TYPE_ENUM:                           \
    for (int i = 0; i < size; i++)                                     \
      result.append(                                                   \
          static_cast<QT_TYPE>(ref.GetRepeated##TYPE(msg, field, i))); \
    break;

  QVariantList result;
  switch (field->cpp_type()) {
    QPB_GET_REPEATED(INT32, Int32, qint32);
    QPB_GET_REPEATED(INT64, Int64, qint64);
    QPB_GET_REPEATED(UINT32, UInt32, quint32);
    QPB_GET_REPEATED(UINT64, UInt64, quint64);
    QPB_GET_REPEATED(DOUBLE, Double, double);
    QPB_GET_REPEATED(FLOAT, Float, float);
    QPB_GET_REPEATED(BOOL, Bool, bool);
    case FieldDescriptor::CPPTYPE_STRING:
      for (int i = 0; i < size; i++)
        result.append(
            QString::fromStdString(ref.GetRepeatedString(msg, field, i)));
      break;

    case FieldDescriptor::CPPTYPE_ENUM:
    // TODO: add enum support
    case FieldDescriptor::CPPTYPE_MESSAGE:
      for (int i = 0; i < size; i++)
        result.append(unpackFromMessage(ref.GetRepeatedMessage(msg, field, i)));
      break;
  }
  return result;
#undef QPB_GET_REPEATED
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
      // TODO: add enum field support
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      return unpackFromMessage(ref.GetMessage(msg, field));
  }
  return QVariant();
}

constexpr auto kCapitalizeOffset = 'a' - 'A';

inline bool is_small_char(char c) {
  return c >= 'a' && c <= 'z';
}

inline char capitalize(char c) {
  return c - kCapitalizeOffset;
}

std::string camelize(const std::string& name) {
  std::ostringstream ss;
  bool capitalizing = false;
  for (auto& c : name) {
    if (capitalizing) {
      capitalizing = false;
      if (is_small_char(c)) {
        ss << capitalize(c);
      } else {
        ss << '_' << c;
      }
    } else {
      if (c == '_') {
        capitalizing = true;
      } else {
        ss << c;
      }
    }
  }
  return ss.str();
}

QVariant unpackFromMessage(const Message& msg) {
  QVariantMap result;
  auto reflection = msg.GetReflection();
  auto descriptor = msg.GetDescriptor();
  if (descriptor->field_count() > 0) {
    for (int i = 0; i < descriptor->field_count(); i++) {
      auto field_descriptor = descriptor->field(i);
      QVariantMap field;
      auto field_name = camelize(field_descriptor->name());
      if (field_descriptor->is_repeated()) {
        auto size = reflection->FieldSize(msg, field_descriptor);
        if (size > 0) {
          result.insert(QString::fromStdString(field_name),
                        getReflectionRepeatedValue(
                            *reflection, msg, field_descriptor, size));
        }
      } else if (reflection->HasField(msg, field_descriptor)) {
        auto value = getReflectionValue(*reflection, msg, field_descriptor);
        if (value.isValid())
          result.insert(QString::fromStdString(field_name), std::move(value));
      }
    }
  }
  return result.isEmpty() ? QVariant() : std::move(result);
}

QVariant DescriptorWrapper::parse(InputDevice* input) {
  if (!input) return QVariant();
  auto msg = sharedMessage();
  msg->Clear();
  auto session = input->createSession();
  if (!session) return QVariant();
  msg->ParseFromZeroCopyStream(session.stream());
  return unpackFromMessage(*msg);
}

bool packToMessage(const QVariantMap& value, Message& msg) {
  auto reflection = msg.GetReflection();
  auto descriptor = msg.GetDescriptor();
  if (descriptor->field_count() <= 0) return false;
  for (int i = 0; i < descriptor->field_count(); i++) {
    auto field_name = camelize(descriptor->field(i)->name());
    auto field_descriptor = descriptor->field(i);
    auto it = value.constFind(QString::fromStdString(field_name));
    if (it != value.constEnd()) {
      if (field_descriptor->is_repeated()) {
        if (!it.value().canConvert(QMetaType::QVariantList)) {
          qDebug() << "Invalid type for repeated field: "
                   << QString::fromStdString(field_name);
        } else {
          auto list = it.value().value<QVariantList>();
          auto size = list.size();
          if (size > 0) {
            setReflectionRepeatedValue(
                *reflection, msg, field_descriptor, list, size);
          }
        }
      } else {
        setReflectionValue(*reflection, msg, field_descriptor, it.value());
      }
    }
  }
  return true;
}

bool DescriptorWrapper::serialize(OutputDevice* output, QVariantMap value) {
  try {
    if (!output || value.isEmpty()) return false;
    auto msg = sharedMessage();
    msg->Clear();
    if (packToMessage(value, *msg)) {
      auto session = output->createSession();
      return msg->SerializeToZeroCopyStream(session.stream());
    }
    return false;
  } catch (google::protobuf::FatalException& ex) {
    qDebug() << "Serialize failed : " << ex.what();
    return false;
  } catch (std::exception& ex) {
    qDebug() << "Serialize failed : " << ex.what();
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
      return new FileDescriptorWrapper(desc, this);
    }
  }
  return nullptr;
}
}
