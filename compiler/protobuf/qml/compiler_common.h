#ifndef PROTOBUF_QML_COMPILER_H
#define PROTOBUF_QML_COMPILER_H

#if PBQML_COMPILER_HAS_QT

#include <QtGlobal>

#define PBQML_ASSERT Q_ASSERT_X

#else  // PBQML_COMPILER_HAS_QT

#ifdef _DEBUG

#include <exception>
#include <iostream>

#define PBQML_ASSERT(cond, where, what)                                  \
  do {                                                                   \
    if (!cond) {                                                         \
      std::cerr << "ASSERT failure in " << where << ": " << what << " (" \
                << __FILE__ << ":" << __LINE__ << ")" << std::endl;      \
      std::terminate();                                                  \
    }                                                                    \
  } while (false)

#else  // _DEBUG

#define PBQML_ASSERT(cond, where, msg) \
  do {                                 \
  } while (false)

#endif  // _DEBUG

#endif  // PBQML_COMPILER_HAS_QT

#endif  // PROTOBUF_QML_COMPILER_H
