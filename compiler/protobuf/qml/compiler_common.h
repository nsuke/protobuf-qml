#ifndef PROTOBUF_QML_COMPILER_H
#define PROTOBUF_QML_COMPILER_H

#define PBQML_USE_INDENT                                        \
  void indentUp() { variables_["i"] = variables_["i"] + "  "; } \
  void indentDown() {                                           \
    auto& i = variables_["i"];                                  \
    PBQML_ASSERT(i.size() >= 2);                                \
    variables_["i"] = i.substr(0, i.size() - 2);                \
  }                                                             \
  const std::string& indent() { return variables_["i"]; }       \
  void indent(const std::string& v) { variables_["i"] = v; }

#if PBQML_COMPILER_HAS_QT

#include <QtGlobal>

#define PBQML_ASSERT_X Q_ASSERT_X
#define PBQML_ASSERT Q_ASSERT

#else  // PBQML_COMPILER_HAS_QT

#ifdef _DEBUG

#include <exception>
#include <iostream>

#define PBQML_ASSERT(cond)                      \
  do {                                          \
    if (!cond) {                                \
      std::cerr << "ASSERT failure: " << #cond; \
    }                                           \
  } while (false)

#define PBQML_ASSERT_X(cond, where, what)                                \
  do {                                                                   \
    if (!cond) {                                                         \
      std::cerr << "ASSERT failure in " << where << ": " << what << " (" \
                << __FILE__ << ":" << __LINE__ << ")" << std::endl;      \
      std::terminate();                                                  \
    }                                                                    \
  } while (false)

#else  // _DEBUG

#define PBQML_ASSERT(cond) \
  do {                     \
  } while (false)

#define PBQML_ASSERT_X(cond, where, msg) PBQML_ASSERT(cond)

#endif  // _DEBUG

#endif  // PBQML_COMPILER_HAS_QT

#endif  // PROTOBUF_QML_COMPILER_H
