/****************************************************************************
** QgisAppBase meta object code from reading C++ file 'QgisAppBase.h'
**
** Created: Fri Jul 5 08:50:38 2002
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QgisAppBase.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 19)
#error "This file was generated using the moc from 3.0.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QgisAppBase::className() const
{
    return "QgisAppBase";
}

QMetaObject *QgisAppBase::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QgisAppBase;

#ifndef QT_NO_TRANSLATION
QString QgisAppBase::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QgisAppBase", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QgisAppBase::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QgisAppBase", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QgisAppBase::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QMainWindow::staticMetaObject();
    static const QUMethod slot_0 = {"fileExit", 0, 0 };
    static const QUMethod slot_1 = {"fileOpen", 0, 0 };
    static const QUMethod slot_2 = {"addLayer", 0, 0 };
    static const QUMethod slot_3 = {"zoomIn", 0, 0 };
    static const QUMethod slot_4 = {"zoomOut", 0, 0 };
    static const QUMethod slot_5 = {"init", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "fileExit()", &slot_0, QMetaData::Public },
	{ "fileOpen()", &slot_1, QMetaData::Public },
	{ "addLayer()", &slot_2, QMetaData::Public },
	{ "zoomIn()", &slot_3, QMetaData::Public },
	{ "zoomOut()", &slot_4, QMetaData::Public },
	{ "init()", &slot_5, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"QgisAppBase", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QgisAppBase.setMetaObject( metaObj );
    return metaObj;
}

void* QgisAppBase::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QgisAppBase" ) ) return (QgisAppBase*)this;
    return QMainWindow::qt_cast( clname );
}

bool QgisAppBase::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: fileExit(); break;
    case 1: fileOpen(); break;
    case 2: addLayer(); break;
    case 3: zoomIn(); break;
    case 4: zoomOut(); break;
    case 5: init(); break;
    default:
	return QMainWindow::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QgisAppBase::qt_emit( int _id, QUObject* _o )
{
    return QMainWindow::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QgisAppBase::qt_property( int _id, int _f, QVariant* _v)
{
    return QMainWindow::qt_property( _id, _f, _v);
}
#endif // QT_NO_PROPERTIES
