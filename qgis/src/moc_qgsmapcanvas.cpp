/****************************************************************************
** QgsMapCanvas meta object code from reading C++ file 'qgsmapcanvas.h'
**
** Created: Fri Jul 5 17:08:23 2002
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "qgsmapcanvas.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QgsMapCanvas::className() const
{
    return "QgsMapCanvas";
}

QMetaObject *QgsMapCanvas::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QgsMapCanvas;

#ifndef QT_NO_TRANSLATION
QString QgsMapCanvas::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QgsMapCanvas", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QgsMapCanvas::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QgsMapCanvas", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QgsMapCanvas::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"QgsMapCanvas", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QgsMapCanvas.setMetaObject( metaObj );
    return metaObj;
}

void* QgsMapCanvas::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QgsMapCanvas" ) ) return (QgsMapCanvas*)this;
    return QWidget::qt_cast( clname );
}

bool QgsMapCanvas::qt_invoke( int _id, QUObject* _o )
{
    return QWidget::qt_invoke(_id,_o);
}

bool QgsMapCanvas::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QgsMapCanvas::qt_property( int _id, int _f, QVariant* _v)
{
    return QWidget::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES
