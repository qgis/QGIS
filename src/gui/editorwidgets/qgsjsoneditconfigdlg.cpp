/***************************************************************************
    qgsjsoneditconfigdlg.cpp
     --------------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsjsoneditconfigdlg.h"
#include "moc_qgsjsoneditconfigdlg.cpp"

QgsJsonEditConfigDlg::QgsJsonEditConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  connect( mDefaultViewComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mFormatJsonComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
}


QVariantMap QgsJsonEditConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "DefaultView" ), mDefaultViewComboBox->currentIndex() );
  cfg.insert( QStringLiteral( "FormatJson" ), mFormatJsonComboBox->currentIndex() );

  return cfg;
}

void QgsJsonEditConfigDlg::setConfig( const QVariantMap &config )
{
  mDefaultViewComboBox->setCurrentIndex( config.value( QStringLiteral( "DefaultView" ) ).toInt() );
  mFormatJsonComboBox->setCurrentIndex( config.value( QStringLiteral( "FormatJson" ) ).toInt() );
}
