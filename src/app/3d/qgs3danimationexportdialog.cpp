/***************************************************************************
  qgs3danimationexportdialog.cpp
  ------------------------------
  Date                 : February 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3danimationexportdialog.h"
#include "qgsproject.h"
#include "qgsgui.h"
#include "qgssettings.h"
#include "qgsoffscreen3dengine.h"
#include "qgs3danimationsettings.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"

#include <QRegExp>
#include <QValidator>
#include <QDir>

const QString FRAME_PLACEHOLDER = QStringLiteral("#####");

Qgs3DAnimationExportDialog::Qgs3DAnimationExportDialog(): QDialog( nullptr )
{
  setupUi( this );
  QgsSettings settings;

  const QString templateText = QStringLiteral("%1%2.jpg").arg( QgsProject::instance()->baseName() ).arg(FRAME_PLACEHOLDER);
  mTemplateLineEdit->setText( templateText );
  QRegExp rx(QStringLiteral("\\w+%1\\.{1}\\w+").arg(FRAME_PLACEHOLDER)); //e.g. anyprefix#####.png
  QValidator *validator = new QRegExpValidator(rx, this);
  mTemplateLineEdit->setValidator(validator);

  mOutputDirFileWidget->setStorageMode(QgsFileWidget::GetDirectory);
  mOutputDirFileWidget->setDialogTitle( tr( "Select directory for 3D animation frames" ) );
  mOutputDirFileWidget->lineEdit()->setShowClearButton( false );
  mOutputDirFileWidget->setDefaultRoot( settings.value( QStringLiteral( "Export3DAnimation/lastDir" ), QString(), QgsSettings::App ).toString() );
  mOutputDirFileWidget->setFilePath( settings.value( QStringLiteral( "Export3DAnimation/lastDir" ), QString(), QgsSettings::App ).toString() );

  connect( mOutputDirFileWidget, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Export3DAnimation/lastDir" ), mOutputDirFileWidget->filePath(), QgsSettings::App );
  } );

  mFpsSpinBox->setValue(settings.value( QStringLiteral( "Export3DAnimation/fps" ), 30).toInt());
  mWidthSpinBox->setValue(settings.value( QStringLiteral( "Export3DAnimation/width" ), 800).toInt());
  mHeightSpinBox->setValue(settings.value( QStringLiteral( "Export3DAnimation/height" ), 600).toInt());

  QgsGui::enableAutoGeometryRestore( this );
}

Qgs3DAnimationExportDialog::~Qgs3DAnimationExportDialog() = default;

QString Qgs3DAnimationExportDialog::fileName(int frameNo) const
{
  const QString dir = mOutputDirFileWidget->filePath();
  QString name = mTemplateLineEdit->text();
  Q_ASSERT( name.contains(FRAME_PLACEHOLDER) ); // asserted by validator
  name = name.replace(FRAME_PLACEHOLDER, QString::number(frameNo));
  return QDir(dir).absoluteFilePath(name);
}

int Qgs3DAnimationExportDialog::fps() const
{
  const int fps = mFpsSpinBox->value();
  return fps;
}

QSize Qgs3DAnimationExportDialog::frameSize() const
{
  const int width = mWidthSpinBox->value();
  const int height = mHeightSpinBox->value();
  return QSize(width, height);
}
