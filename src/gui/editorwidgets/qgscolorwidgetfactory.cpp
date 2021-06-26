/***************************************************************************
    qgscolorwidgetfactory.cpp
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

#include "qgscolorwidgetfactory.h"

#include "qgscolorwidgetwrapper.h"
#include "qgsdummyconfigdlg.h"

QgsColorWidgetFactory::QgsColorWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper *QgsColorWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsColorWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsColorWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "Field contains a color." ) );
}

unsigned int QgsColorWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  const QVariant::Type type = field.type();
  if ( type == QVariant::Color )
  {
    return 20;
  }
  else
  {
    return 5;
  }
}
