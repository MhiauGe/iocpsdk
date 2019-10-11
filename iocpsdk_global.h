#ifndef IOCPSDK_GLOBAL_H
#define IOCPSDK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(IOCPSDK_LIBRARY)
#  define IOCPSDKSHARED_EXPORT Q_DECL_EXPORT
#else
#  define IOCPSDKSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // IOCPSDK_GLOBAL_H
