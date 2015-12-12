#include "protobuf/qml/file.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <sstream>

#ifdef _MSC_VER
#include <io.h>
#else
#define O_BINARY 0
#include <unistd.h>
#include <linux/stat.h>
#endif

namespace protobuf {
namespace qml {

using namespace ::google::protobuf;

bool FileChannel::exists() {
  if (!path_.isEmpty()) {
    return false;
  }
  std::ifstream f(cPath());
  return f.good();
}

void FileChannel::clear() {
  if (exists()) {
    std::remove(cPath());
  }
}

io::ZeroCopyInputStream* FileChannel::openInput(int tag) {
  if (path_.isEmpty()) {
    // TODO: restore error reporting
    // parseError(tag, "Path is empty");
    qWarning() << "Path is empty";
    return nullptr;
  }
  file_ = open(cPath(), O_BINARY | O_RDONLY);
  if (file_ <= 0) {
    file_ = 0;
    // std::ostringstream ss;
    // ss <<  "Failed to open file to read : " << strerror(errno);
    // parseError(tag, QString::fromStdString(ss.str()));
    qWarning() << "Failed to open file to read : " << strerror(errno);
    return nullptr;
  }
  return new io::FileInputStream(file_);
}

void FileChannel::closeInput(int tag, io::ZeroCopyInputStream* stream) {
  if (stream) {
    if (!reinterpret_cast<io::FileInputStream*>(stream)->Close()) {
      qWarning() << "Failed to close input stream : " << strerror(errno);
    }
    delete stream;
  }
  if (file_ > 0) {
    close(file_);
    file_ = 0;
  }
}

io::ZeroCopyOutputStream* FileChannel::openOutput(int tag, int hint) {
  if (path_.isEmpty()) {
    // serializeError(tag, "Path is empty");
    qWarning() << "Path is empty";
    return nullptr;
  }
#ifdef _MSC_VER
  constexpr auto mode = _S_IREAD | _S_IWRITE;
#else
  constexpr auto mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
#endif
  file_ = open(cPath(), O_BINARY | O_WRONLY | O_CREAT | O_TRUNC, mode);
  if (file_ <= 0) {
    file_ = 0;
    // std::ostringstream ss;
    // ss << "Failed to open file to write : " << strerror(errno);
    // serializeError(tag, QString::fromStdString(ss.str()));
    qWarning() << "Failed to open file ( " << path_ << ") to write : " << strerror(errno);
    return nullptr;
  }
  return new io::FileOutputStream(file_);
}

void FileChannel::closeOutput(int tag, io::ZeroCopyOutputStream* stream) {
  if (stream) {
    if (!reinterpret_cast<io::FileOutputStream*>(stream)->Close()) {
      qWarning() << "Failed to close output stream : " << strerror(errno);
    }
    delete stream;
  }
  if (file_ > 0) {
    close(file_);
    file_ = 0;
  }
}
}
}
