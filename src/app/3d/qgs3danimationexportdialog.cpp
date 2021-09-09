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
#include "qgsspinbox.h"
#include "qgshelp.h"

#include <QtGlobal>

Qgs3DAnimationExportDialog::Qgs3DAnimationExportDialog(): QDialog( nullptr )
{
  setupUi( this );
  mFpsSpinBox->setClearValue( 30 );
  mWidthSpinBox->setClearValue( 800 );
  mHeightSpinBox->setClearValue( 600 );
  const QgsSettings settings;

  const QString templateText = settings.value( QStringLiteral( "Export3DAnimation/fileNameTemplate" ),
                               QStringLiteral( "%1####.jpg" ).arg( QgsProject::instance()->baseName() )
                               , QgsSettings::App ).toString();
  mTemplateLineEdit->setText( templateText );
  const QRegExp rx( QStringLiteral( "\\w+#+\\.{1}\\w+" ) ); //e.g. anyprefix#####.png
  QValidator *validator = new QRegExpValidator( rx, this );
  mTemplateLineEdit->setValidator( validator );

  connect( mTemplateLineEdit, &QLineEdit::textChanged, this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Export3DAnimation/fileNameTemplate" ), mTemplateLineEdit->text() );
  } );

  mOutputDirFileWidget->setStorageMode( QgsFileWidget::GetDirectory );
  mOutputDirFileWidget->setDialogTitle( tr( "Select directory for 3D animation frames" ) );
  mOutputDirFileWidget->lineEdit()->setShowClearButton( false );
  mOutputDirFileWidget->setDefaultRoot( settings.value( QStringLiteral( "Export3DAnimation/lastDir" ), QString(), QgsSettings::App ).toString() );
  mOutputDirFileWidget->setFilePath( settings.value( QStringLiteral( "Export3DAnimation/lastDir" ), QString(), QgsSettings::App ).toString() );

  connect( mOutputDirFileWidget, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Export3DAnimation/lastDir" ), mOutputDirFileWidget->filePath(), QgsSettings::App );
  } );

  mFpsSpinBox->setValue( settings.value( QStringLiteral( "Export3DAnimation/fps" ), 30 ).toInt() );
  connect( mFpsSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QgsSpinBox::valueChanged ), this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Export3DAnimation/fps" ), mFpsSpinBox->value() );
  } );

  mWidthSpinBox->setValue( settings.value( QStringLiteral( "Export3DAnimation/width" ), 800 ).toInt() );
  connect( mWidthSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QgsSpinBox::valueChanged ), this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Export3DAnimation/width" ), mWidthSpinBox->value() );
  } );

  mHeightSpinBox->setValue( settings.value( QStringLiteral( "Export3DAnimation/height" ), 600 ).toInt() );
  connect( mHeightSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QgsSpinBox::valueChanged ), this, [ = ]
  {
    QgsSettings settings;
    settings.setValue( QStringLiteral( "Export3DAnimation/height" ), mHeightSpinBox->value() );
  } );

  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "introduction/qgis_gui.html#creating-an-animation" ) );
  } );

  QgsGui::enableAutoGeometryRestore( this );
}

QString Qgs3DAnimationExportDialog::outputDirectory() const
{
  const QString dir = mOutputDirFileWidget->filePath();
  return dir;
}

QString Qgs3DAnimationExportDialog::fileNameExpression() const
{
  const QString name = mTemplateLineEdit->text();
  return name;
}

Qgs3DAnimationExportDialog::~Qgs3DAnimationExportDialog() = default;

int Qgs3DAnimationExportDialog::fps() const
{
  const int fps = mFpsSpinBox->value();
  return fps;
}

QSize Qgs3DAnimationExportDialog::frameSize() const
{
  const int width = mWidthSpinBox->value();
  const int height = mHeightSpinBox->value();
  return QSize( width, height );
}
