/***************************************************************************
    qgsuuidwidgetfactory.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsuuidwidgetfactory.h"

#include "qgsuuidwidgetwrapper.h"
#include "qgsdummyconfigdlg.h"

QgsUuidWidgetFactory::QgsUuidWidgetFactory( const QString &name )
  :  QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper *QgsUuidWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsUuidWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsUuidWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "Read-only field that generates a UUID if empty." ) );
}

unsigned int QgsUuidWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QVariant::Type type = vl->fields().field( fieldIdx ).type();
  return type == QVariant::String ? 5 : 0;
}
