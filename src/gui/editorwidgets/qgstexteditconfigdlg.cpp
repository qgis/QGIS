/***************************************************************************
    qgstexteditconfigdlg.cpp
     --------------------------------------
    Date                 : 8.5.2014
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

#include "qgstexteditconfigdlg.h"

QgsTextEditConfigDlg::QgsTextEditConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
}


QgsEditorWidgetConfig QgsTextEditConfigDlg::config()
{
  QgsEditorWidgetConfig cfg;

  cfg.insert( "IsMultiline", mIsMultiline->isChecked() );
  cfg.insert( "UseHtml", mUseHtml->isChecked() );

  return cfg;
}

void QgsTextEditConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  mIsMultiline->setChecked( config.value( "IsMultiline" ).toBool() );
  mUseHtml->setChecked( config.value( "UseHtml" ).toBool() );
}
