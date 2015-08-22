#ifndef PROTOBUF_QML_COMMON_H
#define PROTOBUF_QML_COMMON_H

#include <QtGlobal>
#include <QDebug>

#ifdef GRPC_QML_EXPORT
#define GRPC_QML_DLLEXPORT Q_DECL_EXPORT
#else  // GRPC_QML_EXPORT
#define GRPC_QML_DLLEXPORT Q_DECL_IMPORT
#endif  // GRPC_QML_EXPORT

#ifdef PROTOBUF_QML_EXPORT
#define PROTOBUF_QML_DLLEXPORT Q_DECL_EXPORT
#else  // PROTOBUF_QML_EXPORT
#define PROTOBUF_QML_DLLEXPORT Q_DECL_IMPORT
#endif  // PROTOBUF_QML_EXPORT

#endif  // PROTOBUF_QML_COMMON_H
