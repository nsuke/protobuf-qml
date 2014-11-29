#include "qpb/qml_generator.h"

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>
#include <QByteArray>

#include <memory>
#include <stdexcept>

namespace qpb {

using namespace google::protobuf;

bool QmlGenerator::Generate(const FileDescriptor* file,
                            const std::string& parameter,
                            compiler::GeneratorContext* generator_context,
                            std::string* error) const {
  try {
    auto path = compiler::cpp::StripProto(file->name());
    path.append(".pb.js");
    std::unique_ptr<io::ZeroCopyOutputStream> out(
        generator_context->Open(path));
    io::Printer p(out.get(), '$');
    FileGenerator g(file);
    g.generateJsFile(p);
    return true;
  } catch (std::exception& ex) {
    *error = ex.what();
    return false;
  }
}

FileGenerator::FileGenerator(const google::protobuf::FileDescriptor* file)
    : file_(file) {
  if (!file) {
    throw std::invalid_argument("File descriptor is null");
  }
  for (int i = 0; i < file_->message_type_count(); i++) {
    message_generators_.emplace_back(
        new MessageGenerator(file_->message_type(i)));
  }
  for (int i = 0; i < file_->enum_type_count(); i++) {
    enum_generators_.emplace_back(new EnumGenerator(file_->enum_type(i)));
  }
}

void FileGenerator::generateJsFile(io::Printer& p) {
  p.Print(
      ".pragma library\n"
      ".import Protobuf $qpb_version$ as Qpb\n",
      "qpb_version",
      "1.0");
  std::vector<std::string> deps_;
  for (int i = 0; i < file_->dependency_count(); i++) {
    auto d = file_->dependency(i)->name();
    auto path = compiler::cpp::StripProto(d);
    std::string import_name("Q__");
    import_name.append(path).append("_pb_js");
    path.append(".pb.js");
    p.Print(".import \"$path$\" as $import_name$\n",
            "path",
            path,
            "import_name",
            import_name);
    deps_.push_back(std::move(import_name));
  }
  p.Print("\n");
  p.Print(
      "var init = {\n"
      "  once: function() {\n"
      "    if(!this.done) {\n");
  for (auto& dep : deps_) {
    p.Print(
        "      if(!$import_name$.init.once()) {\n"
        "        console.info('Failed to initialize dependency:  "
        "$import_name$');\n"
        "        return false;\n"
        "      }\n",
        "import_name",
        dep);
  }
  std::string file_desc_str;
  auto file_desc_size = serializedFileDescriptor(file_desc_str);
  p.Print(
      "      this.file = Qpb.DescriptorPool.addFileDescriptor("
      "\"$file_descriptor$\", $file_descriptor_size$);\n"
      "      if(this.file) {\n",
      "file_descriptor_size",
      SimpleItoa(file_desc_size),
      "file_descriptor",
      file_desc_str);
  for (auto& g : message_generators_) {
    g->generateMessageInit(p);
  }

  p.Print(
      "        this.done = true;\n"
      "      } else {\n"
      "        console.info('Failed to initialize: $file_name$');\n"
      "      }\n"
      "    }\n"
      "    return this.done;\n"
      "  },\n"
      "};\n",
      "file_name",
      file_->name());

  for (auto& g : enum_generators_) {
    g->generateEnum(p);
  }
  for (auto& g : message_generators_) {
    g->generateMessage(p);
  }
}

int FileGenerator::serializedFileDescriptor(std::string& out) {
  FileDescriptorProto file_pb;
  file_->CopyTo(&file_pb);
  auto size = file_pb.ByteSize();
  QByteArray byte_array(size, ' ');
  if (!file_pb.SerializeToArray(byte_array.data(), byte_array.size())) {
    throw std::runtime_error("Failed to serialize file descriptor");
  }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
  out = byte_array.toBase64().toStdString();
#else
  auto tmp_ba = byte_array.toBase64();
  out = std::string(tmp_ba.data(), tmp_ba.size());
#endif
  return size;
}
}
