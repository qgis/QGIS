/***************************************************************************
    qgsenumerationwidgetfactory.cpp
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

#include "qgsenumerationwidgetfactory.h"

#include "qgsenumerationwidgetwrapper.h"
#include "qgsdummyconfigdlg.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

QgsEnumerationWidgetFactory::QgsEnumerationWidgetFactory( const QString &name )
  :  QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsEnumerationWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsEnumerationWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsEnumerationWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "Combo box with values that can be used within the column's type. Must be supported by the provider." ) );
}


unsigned int QgsEnumerationWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  QStringList list;
  vl->dataProvider()->enumValues( fieldIdx, list );
  if ( !list.isEmpty() )
    return 20;
  else
    return 0;
}
