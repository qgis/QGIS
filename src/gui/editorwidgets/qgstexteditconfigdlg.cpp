/***************************************************************************
    qgstexteditconfigdlg.cpp
     --------------------------------------
    Date                 : 8.5.2014
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

#include "qgstexteditconfigdlg.h"
#include "moc_qgstexteditconfigdlg.cpp"

QgsTextEditConfigDlg::QgsTextEditConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  connect( mIsMultiline, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mUseHtml, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
}


QVariantMap QgsTextEditConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "IsMultiline" ), mIsMultiline->isChecked() );
  cfg.insert( QStringLiteral( "UseHtml" ), mUseHtml->isChecked() );

  return cfg;
}

void QgsTextEditConfigDlg::setConfig( const QVariantMap &config )
{
  mIsMultiline->setChecked( config.value( QStringLiteral( "IsMultiline" ) ).toBool() );
  mUseHtml->setChecked( config.value( QStringLiteral( "UseHtml" ) ).toBool() );
}
