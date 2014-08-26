/***************************************************************************
    qgscolorwidgetfactory.cpp
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

#include "qgscolorwidgetfactory.h"

#include "qgscolorwidgetwrapper.h"
#include "qgsdummyconfigdlg.h"

QgsColorWidgetFactory::QgsColorWidgetFactory( const QString& name )
    : QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper* QgsColorWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsColorWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsColorWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "Field contains a color." ) );
}
