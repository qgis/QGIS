/***************************************************************************
    qgswebviewwidgetfactory.cpp
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

#include "qgswebviewwidgetfactory.h"

#include "qgswebviewwidgetwrapper.h"
#include "qgswebviewconfigdlg.h"

QgsWebViewWidgetFactory::QgsWebViewWidgetFactory( const QString& name )
    :  QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper* QgsWebViewWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsWebViewWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsWebViewWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsWebViewWidgetConfigDlg( vl, fieldIdx, parent );
}
