#include "protobuf/qml/qml_generator.h"
#include "protobuf/qml/base64.h"
#include "protobuf/qml/compiler_util.h"

#include <google/protobuf/io/zero_copy_stream.h>

#include <memory>
#include <stdexcept>

namespace protobuf {
namespace qml {

using namespace google::protobuf;

bool QmlGenerator::Generate(const FileDescriptor* file,
                            const std::string& parameter,
                            compiler::GeneratorContext* generator_context,
                            std::string* error) const {
  try {
    {
      auto path = generateFilePath(file);
      std::unique_ptr<io::ZeroCopyOutputStream> out(
          generator_context->Open(path));
      io::Printer p(out.get(), '$');
      FileGenerator g(file);
      g.generateJsFile(p);
    }

    for (int i = 0; i < file->service_count(); ++i) {
      std::unique_ptr<io::ZeroCopyOutputStream> out(
          generator_context->Open(file->service(i)->name() + "Client.qml"));
      io::Printer p(out.get(), '$');
      ServiceGenerator g(file->service(i));
      g.generateQmlFile(p);
    }

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
    message_generators_.emplace_back(file_->message_type(i));
  }
  for (int i = 0; i < file_->enum_type_count(); i++) {
    enum_generators_.emplace_back(file_->enum_type(i));
  }
}

void FileGenerator::generateJsFile(io::Printer& p) {
  p.Print(
      ".pragma library\n"
      ".import Protobuf $protobuf_qml_version$ as Protobuf\n",
      "protobuf_qml_version", "1.0");
  std::vector<std::string> deps_;
  for (int i = 0; i < file_->dependency_count(); i++) {
    auto d = file_->dependency(i);
    auto path = generateFilePath(d);
    auto import_name = generateImportName(d);
    p.Print(".import '$path$' as $import_name$\n", "path", path, "import_name",
            import_name);
    deps_.emplace_back(std::move(import_name));
  }
  p.Print("'use strict';\n\n");
  p.Print(
      "var _file = {\n"
      "  get descriptor() {\n"
      "    if(!this._desc");
  for (auto& dep : deps_) {
    p.Print(" && $import_name$._file.descriptor", "import_name", dep);
  }
  std::string file_desc_str;
  auto file_desc_size = serializedFileDescriptor(file_desc_str);
  p.Print(
      ") {\n"
      "      this._desc = Protobuf.DescriptorPool.addFileDescriptor("
      "'$file_descriptor$', $file_descriptor_size$);\n"
      "    }\n"
      "    if (!this._desc)\n"
      "      console.warn('Failed to initialize: $file_name$');\n"
      "    return this._desc;\n"
      "  },\n"
      "};\n"
      "\n",
      "file_descriptor_size", std::to_string(file_desc_size), "file_descriptor",
      file_desc_str, "file_name", file_->name());

  for (auto& g : enum_generators_) {
    g.generateEnum(p);
  }
  for (auto& g : message_generators_) {
    g.generateMessage(p);
  }
}

int FileGenerator::serializedFileDescriptor(std::string& out) {
  FileDescriptorProto file_pb;
  file_->CopyTo(&file_pb);
  auto size = file_pb.ByteSize();
  auto buf = base64Buffer(size);
  if (!file_pb.SerializeToArray(buf.data(), buf.size())) {
    throw std::runtime_error("Failed to serialize file descriptor");
  }
  out = toBase64(buf);
  return size;
}
}
}
