/***************************************************************************
    qgsclassificationwidgetwrapperfactory.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsclassificationwidgetwrapperfactory.h"

#include "qgsclassificationwidgetwrapper.h"
#include "qgsdummyconfigdlg.h"

QgsClassificationWidgetWrapperFactory::QgsClassificationWidgetWrapperFactory( const QString& name )
    :  QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper*QgsClassificationWidgetWrapperFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsClassificationWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget*QgsClassificationWidgetWrapperFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "Displays a combo box containing values of attributes used for classification." ) );
}
