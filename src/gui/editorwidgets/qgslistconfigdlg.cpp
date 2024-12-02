/***************************************************************************
    qgslistconfigdlg.cpp
     --------------------------------------
    Date                 : 9.2020
    Copyright            : (C) 2020 Stephen Knox
    Email                : stephenknox73 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslistconfigdlg.h"
#include "moc_qgslistconfigdlg.cpp"

QgsListConfigDlg::QgsListConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  connect( mNullBehavior, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
}


QVariantMap QgsListConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "EmptyIsNull" ), mEmptyisNull->isChecked() );
  cfg.insert( QStringLiteral( "EmptyIsEmptyArray" ), mEmptyisEmptyArray->isChecked() );

  return cfg;
}

void QgsListConfigDlg::setConfig( const QVariantMap &config )
{
  mEmptyisNull->setChecked( config.value( QStringLiteral( "EmptyIsNull" ) ).toBool() );
  mEmptyisEmptyArray->setChecked( config.value( QStringLiteral( "EmptyIsEmptyArray" ) ).toBool() );
}
