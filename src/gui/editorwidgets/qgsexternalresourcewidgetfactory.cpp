/***************************************************************************
   qgsexternalresourcewidgetfactory.cpp

 ---------------------
 begin                : 16.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexternalresourcewidgetfactory.h"
#include "qgsfilewidget.h"
#include "qgsexternalresourcewidgetwrapper.h"
#include "qgsexternalresourceconfigdlg.h"

QgsExternalResourceWidgetFactory::QgsExternalResourceWidgetFactory( const QString &name, QgsMessageBar *messageBar )
  : QgsEditorWidgetFactory( name )
  , mMessageBar( messageBar )
{
}

QgsEditorWidgetWrapper *QgsExternalResourceWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsExternalResourceWidgetWrapper( vl, fieldIdx, editor, mMessageBar, parent );
}

QgsEditorConfigWidget *QgsExternalResourceWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsExternalResourceConfigDlg( vl, fieldIdx, parent );
}

unsigned int QgsExternalResourceWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  if ( vl->fields().at( fieldIdx ).type() == QVariant::String )
    return 5;

  return 0;
}
