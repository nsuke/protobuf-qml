#include "qpb/qml_generator.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/compiler/cpp/cpp_helpers.h>
#include <google/protobuf/stubs/strutil.h>
#include <QByteArray>
#include <memory>

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
      "      var file = Qpb.DescriptorPool.addFileDescriptor("
      "\"$file_descriptor$\", $file_descriptor_size$);\n"
      "      if(file) {\n",
      "file_descriptor_size",
      SimpleItoa(file_desc_size),
      "file_descriptor",
      file_desc_str);
  for (int i = 0; i < file_->message_type_count(); i++) {
    auto msg = file_->message_type(i);
    p.Print(
        "        this.$message_name$ = file.messageType($message_index$);\n",
        "message_name",
        msg->name(),
        "message_index",
        SimpleItoa(i));
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

  for (int i = 0; i < file_->message_type_count(); i++) {
    generateMessage(p, file_->message_type(i));
    // generateMessagePrototype(p, file_->message_type(i));
  }
}

void FileGenerator::generateMessage(io::Printer& p, const Descriptor* t) {
  p.Print(
      "var $message_name$ = {\n"
      "  parse: function(input, callback) {\n"
      "    if(!init.once()) return;\n"
      "    if(!callback) {\n"
      "      return init.$message_name$.parse(input);\n"
      "    } else {\n"
      "      console.error('not implemented yet');\n"
      "    }\n"
      "  },\n"
      "  serialize: function(output, value, callback) {\n"
      "    if(!init.once()) return;\n"
      "    if(!callback) {\n"
      "      return init.$message_name$.serialize(output, value);\n"
      "    } else {\n"
      "      console.error('not implemented yet');\n"
      "    }\n"
      "  },\n"
      "};\n",
      "message_name",
      t->name());
  //p.Print("$message_name$.prototype.serializeTo = function(output) {\n", );
}

void FileGenerator::generateMessagePrototype(io::Printer& p,
                                             const Descriptor* t) {
  //  p.Print(
  //      "$message_name$.prototype.parse = function(input) {\n"
  //      "};\n";
  //      "message_name", t->name());
}

// void FileGenerator::generateMessagePrototype(io::Printer& p,
//                                              const Descriptor* t) {
//   // TODO: handle cutoff when reading tag
//   p.Print("$message_name$.prototype.serializeTo = function(output) {\n",
//           "message_name",
//           t->name());
//   for (int i = 0; i < t->field_count(); i++) {
//     auto fp = t->field(i);
//     p.Print(
//         "  Qpb.WireFormatLite.writeInt32(output, $number$, "
//         "this.$field_name$);\n"
//         "  if(Qpb.WireFormatLite.error) return false;\n",
//         "field_name",
//         fp->name(),
//         "number",
//         SimpleItoa(fp->number()));
//   }
//   p.Print(
//       "  return true;\n"
//       "};\n"
//       "$message_name$.prototype.parseFrom = function(input) {\n",
//       "message_name",
//       t->name());
//   for (int i = 0; i < t->field_count(); i++) {
//     auto fp = t->field(i);
//     p.Print(
//         "  var tag = Qpb.WireFormatLite.readTag(input);\n"
//         "  if(Qpb.WireFormatLite.error) return false;\n"
//         "  this.$field_name$ = Qpb.WireFormatLite.readInt32(input);\n"
//         "  if(Qpb.WireFormatLite.error) return false;\n",
//         "field_name",
//         fp->name(),
//         "number",
//         SimpleItoa(fp->number()));
//   }
//   p.Print(
//       "  return true;\n"
//       "};\n");
// }
//
int FileGenerator::serializedFileDescriptor(std::string& out) {
  FileDescriptorProto file_pb;
  file_->CopyTo(&file_pb);
  auto size = file_pb.ByteSize();
  QByteArray byte_array(size, ' ');
  if (!file_pb.SerializeToArray(byte_array.data(), byte_array.size())) {
    throw std::runtime_error("Failed to serialize file descriptor");
  }
  out = byte_array.toBase64().toStdString();
  return size;
}
}
