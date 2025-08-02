/***************************************************************************
    qgsclassificationconfigdlg.cpp
     --------------------------------------
    Date                 : 7.8.2025
    Copyright            : (C) 2014 Moritz Hackenberg
    Email                : hackenberg at dev-gis dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsclassificationconfigdlg.h"
#include "moc_qgsclassificationconfigdlg.cpp"

#include "qgsattributetypeloaddialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QClipboard>
#include <QKeyEvent>
#include <QMimeData>
#include <QRegularExpression>

QgsClassificationConfigDlg::QgsClassificationConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  connect( mCbxAllowNull, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
}

QVariantMap QgsClassificationConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "AllowNull" ), mCbxAllowNull->isChecked() );

  return cfg;
}

void QgsClassificationConfigDlg::setConfig( const QVariantMap &config )
{
  mCbxAllowNull->setChecked( config.value( QStringLiteral( "AllowNull" ) ).toBool() );
}
