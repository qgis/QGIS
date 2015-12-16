/***************************************************************************
    qgsfilenamewidgetfactory.cpp
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

#include "qgsfilenamewidgetfactory.h"

#include "qgsfilenamewidgetwrapper.h"
#include "qgsdummyconfigdlg.h"

QgsFileNameWidgetFactory::QgsFileNameWidgetFactory( const QString& name ) :
    QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper* QgsFileNameWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsFileNameWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsFileNameWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "Simplifies file selection by adding a file chooser dialog." ) );
}
