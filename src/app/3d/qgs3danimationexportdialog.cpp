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

#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsspinbox.h"

#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QtGlobal>

#include "moc_qgs3danimationexportdialog.cpp"

Qgs3DAnimationExportDialog::Qgs3DAnimationExportDialog()
  : QDialog( nullptr )
{
  setupUi( this );
  mFpsSpinBox->setClearValue( 30 );
  mWidthSpinBox->setClearValue( 800 );
  mHeightSpinBox->setClearValue( 600 );
  const QgsSettings settings;

  const QString templateText = settings.value( u"Export3DAnimation/fileNameTemplate"_s, u"%1####.jpg"_s.arg( QgsProject::instance()->baseName() ), QgsSettings::App ).toString();
  mTemplateLineEdit->setText( templateText );
  const thread_local QRegularExpression rx( u"^\\w+#+\\.{1}\\w+$"_s ); //e.g. anyprefix#####.png
  QValidator *validator = new QRegularExpressionValidator( rx, this );
  mTemplateLineEdit->setValidator( validator );

  connect( mTemplateLineEdit, &QLineEdit::textChanged, this, [this] {
    QgsSettings settings;
    settings.setValue( u"Export3DAnimation/fileNameTemplate"_s, mTemplateLineEdit->text() );
  } );

  mOutputDirFileWidget->setStorageMode( QgsFileWidget::GetDirectory );
  mOutputDirFileWidget->setDialogTitle( tr( "Select directory for 3D animation frames" ) );
  mOutputDirFileWidget->lineEdit()->setShowClearButton( false );
  mOutputDirFileWidget->setDefaultRoot( settings.value( u"Export3DAnimation/lastDir"_s, QString(), QgsSettings::App ).toString() );
  mOutputDirFileWidget->setFilePath( settings.value( u"Export3DAnimation/lastDir"_s, QString(), QgsSettings::App ).toString() );

  connect( mOutputDirFileWidget, &QgsFileWidget::fileChanged, this, [this] {
    QgsSettings settings;
    settings.setValue( u"Export3DAnimation/lastDir"_s, mOutputDirFileWidget->filePath(), QgsSettings::App );
  } );

  mFpsSpinBox->setValue( settings.value( u"Export3DAnimation/fps"_s, 30 ).toInt() );
  connect( mFpsSpinBox, static_cast<void ( QSpinBox::* )( int )>( &QgsSpinBox::valueChanged ), this, [this] {
    QgsSettings settings;
    settings.setValue( u"Export3DAnimation/fps"_s, mFpsSpinBox->value() );
  } );

  mWidthSpinBox->setValue( settings.value( u"Export3DAnimation/width"_s, 800 ).toInt() );
  connect( mWidthSpinBox, static_cast<void ( QSpinBox::* )( int )>( &QgsSpinBox::valueChanged ), this, [this] {
    QgsSettings settings;
    settings.setValue( u"Export3DAnimation/width"_s, mWidthSpinBox->value() );
  } );

  mHeightSpinBox->setValue( settings.value( u"Export3DAnimation/height"_s, 600 ).toInt() );
  connect( mHeightSpinBox, static_cast<void ( QSpinBox::* )( int )>( &QgsSpinBox::valueChanged ), this, [this] {
    QgsSettings settings;
    settings.setValue( u"Export3DAnimation/height"_s, mHeightSpinBox->value() );
  } );

  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [] {
    QgsHelp::openHelp( u"map_views/3d_map_view.html#create-animation"_s );
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
