#ifndef EMATH_GLOBAL_H
#define EMATH_GLOBAL_H

#include <QtCore/qglobal.h>

#define Q2_BEGIN_NAMESPACE namespace CuteCube{
#define Q2_END_NAMESPACE } \
	using namespace CuteCube;

#if defined(QT2IMAP_LIBRARY)
#  define QT2IMAPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QT2IMAPHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // EMATH_GLOBAL_H
