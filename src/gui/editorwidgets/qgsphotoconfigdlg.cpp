/***************************************************************************
    qgsphotoconfigdlg.cpp
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

#include "qgsphotoconfigdlg.h"

QgsPhotoConfigDlg::QgsPhotoConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget *parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
}

QgsEditorWidgetConfig QgsPhotoConfigDlg::config()
{
  QgsEditorWidgetConfig cfg;

  cfg.insert( "Height", sbWidgetHeight->value() );
  cfg.insert( "Width", sbWidgetWidth->value() );

  return cfg;
}

void QgsPhotoConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  sbWidgetHeight->setValue( config.value( "Height", 0 ).toInt() );
  sbWidgetWidth->setValue( config.value( "Width", 0 ).toInt() );
}
