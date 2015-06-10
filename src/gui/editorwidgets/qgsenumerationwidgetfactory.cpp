/***************************************************************************
    qgsenumerationwidgetfactory.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsenumerationwidgetfactory.h"

#include "qgsenumerationwidgetwrapper.h"
#include "qgsdummyconfigdlg.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

QgsEnumerationWidgetFactory::QgsEnumerationWidgetFactory( const QString& name )
    :  QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper* QgsEnumerationWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsEnumerationWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsEnumerationWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "Combo box with values that can be used within the column's type. Must be supported by the provider." ) );
}


bool QgsEnumerationWidgetFactory::isFieldSupported( QgsVectorLayer* vl, int fieldIdx )
{
  QStringList list;
  vl->dataProvider()->enumValues( fieldIdx, list );
  if ( list.size() > 0 )
    return true;
  else
    return false;
}
