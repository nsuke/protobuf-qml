#ifndef QPB_COMMON_H
#define QPB_COMMON_H

#include <QtGlobal>
#include <QDebug>

#ifdef QPB_EXPORT
#define QPB_DLLEXPORT Q_DECL_EXPORT
#else  // QPB_EXPORT
#define QPB_DLLEXPORT Q_DECL_IMPORT
#endif  // QPB_EXPORT

#define QPB_LOG_F qDebug() << __PRETTY_FUNCTION__

#endif  // QPB_COMMON_H
