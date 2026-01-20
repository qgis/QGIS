/***************************************************************************
                          qgsprojecttrustdialog.cpp
                             -------------------
    begin                : October 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojecttrustdialog.h"

#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgssettings.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"

#include <QFileInfo>
#include <QPushButton>
#include <QSvgRenderer>

#include "moc_qgsprojecttrustdialog.cpp"

QgsProjectTrustDialog::QgsProjectTrustDialog( QgsProject *project, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mButtonBox->button( QDialogButtonBox::StandardButton::YesToAll )->setText( tr( "Always Trust" ) );
  mButtonBox->button( QDialogButtonBox::StandardButton::Yes )->setText( tr( "Trust" ) );
  mButtonBox->button( QDialogButtonBox::StandardButton::No )->setText( tr( "Deny" ) );
  mButtonBox->button( QDialogButtonBox::StandardButton::NoToAll )->setText( tr( "Always Deny" ) );

  connect( mButtonBox, &QDialogButtonBox::clicked, this, &QgsProjectTrustDialog::buttonBoxClicked );

  connect( mScriptPreviewList, &QListWidget::itemDoubleClicked, this, [this]( QListWidgetItem *item ) {
    const int row = mScriptPreviewList->row( item );
    if ( row >= 0 && row < mEmbeddedScriptsVisitor.embeddedScripts().size() )
    {
      mScriptPreviewEditor->setText( mEmbeddedScriptsVisitor.embeddedScripts()[row].script() );
      mScriptPreviewStackedWidget->setCurrentIndex( 1 );
    }
  } );

  connect( mScriptPreviewBackButton, &QAbstractButton::clicked, this, [this] {
    mScriptPreviewStackedWidget->setCurrentIndex( 0 );
  } );

  mScriptPreviewEditor->setReadOnly( true );
  mScriptPreviewEditor->setLineNumbersVisible( false );

  QSvgRenderer svg( u":/images/themes/default/mIconPythonFile.svg"_s );
  if ( svg.isValid() )
  {
    const double maxLength = 64.0;
    QSizeF size( maxLength, maxLength );
    const QRectF viewBox = svg.viewBoxF();
    if ( viewBox.height() > viewBox.width() )
    {
      size.setWidth( maxLength * viewBox.width() / viewBox.height() );
    }
    else
    {
      size.setHeight( maxLength * viewBox.height() / viewBox.width() );
    }

    QPixmap pixmap( static_cast<int>( maxLength ), static_cast<int>( maxLength ) );
    pixmap.fill( Qt::transparent );

    QPainter painter;
    painter.begin( &pixmap );
    painter.setRenderHint( QPainter::SmoothPixmapTransform );
    painter.translate( ( maxLength - size.width() ) / 2, ( maxLength - size.height() ) / 2 );
    svg.render( &painter, QRectF( 0, 0, size.width(), size.height() ) );
    painter.end();

    mIconLabel->setPixmap( pixmap );
  }

  if ( project )
  {
    QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromUri( project->fileName() );
    if ( storage )
    {
      if ( !storage->filePath( project->fileName() ).isEmpty() )
      {
        QFileInfo projectFileInfo( storage->filePath( project->fileName() ) );
        mProjectAbsoluteFilePath = projectFileInfo.absoluteFilePath();
        mProjectAbsolutePath = projectFileInfo.absolutePath();
        mProjectIsFile = true;
      }
      else
      {
        mProjectAbsolutePath = project->fileName();
        mProjectIsFile = false;
      }
    }
    else
    {
      QFileInfo projectFileInfo( project->fileName() );
      mProjectAbsoluteFilePath = projectFileInfo.absoluteFilePath();
      mProjectAbsolutePath = projectFileInfo.absolutePath();
      mProjectIsFile = true;
    }

    if ( mProjectIsFile )
    {
      mProjectDetailsLabel->setText( tr( "The current project file path is ’%1’." ).arg( u"<b>%1</b>"_s.arg( mProjectAbsoluteFilePath ) ) );
      QDir dir( mProjectAbsolutePath );
      mTrustProjectFolderCheckBox->setText( tr( "Apply decision to all projects in folder ’%1’" ).arg( u"%1"_s.arg( dir.dirName() ) ) );
    }
    else
    {
      mProjectDetailsLabel->setText( tr( "The current project URI is ’%1’." ).arg( u"<b>%1</b>"_s.arg( mProjectAbsoluteFilePath ) ) );
      mTrustProjectFolderCheckBox->setVisible( false );
    }

    project->accept( &mEmbeddedScriptsVisitor, QgsObjectVisitorContext() );
  }

  for ( const QgsEmbeddedScriptEntity &scriptDetails : mEmbeddedScriptsVisitor.embeddedScripts() )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mScriptPreviewList );
    switch ( scriptDetails.type() )
    {
      case Qgis::EmbeddedScriptType::Macro:
        newItem->setIcon( QgsApplication::getThemeIcon( u"/mIconPythonFile.svg"_s ) );
        break;

      case Qgis::EmbeddedScriptType::ExpressionFunction:
        newItem->setIcon( QgsApplication::getThemeIcon( u"/mIconExpression.svg"_s ) );
        break;

      case Qgis::EmbeddedScriptType::Action:
        newItem->setIcon( QgsApplication::getThemeIcon( u"/mAction.svg"_s ) );
        break;

      case Qgis::EmbeddedScriptType::FormInitCode:
        newItem->setIcon( QgsApplication::getThemeIcon( u"/mActionFormView.svg"_s ) );
        break;
    }
    newItem->setText( scriptDetails.name() );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mScriptPreviewList->addItem( newItem );
  }
}

void QgsProjectTrustDialog::buttonBoxClicked( QAbstractButton *button )
{
  QDialogButtonBox::StandardButton buttonType = mButtonBox->standardButton( button );
  if ( buttonType == QDialogButtonBox::StandardButton::Help )
  {
    showHelp();
    return;
  }

  bool accepted = false;
  QString path = !mProjectIsFile || mTrustProjectFolderCheckBox->isChecked() ? mProjectAbsolutePath : mProjectAbsoluteFilePath;
  if ( !path.isEmpty() )
  {
    QStringList trustedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders->value();
    trustedProjectsFolders.removeAll( path );
    QStringList untrustedProjectsFolders = QgsSettingsRegistryCore::settingsCodeExecutionUntrustedProjectsFolders->value();
    untrustedProjectsFolders.removeAll( path );

    QStringList temporarilyTrustedProjectsFolders = QgsApplication::temporarilyTrustedProjectsFolders();
    temporarilyTrustedProjectsFolders.removeAll( path );
    QStringList temporarilyUntrustedProjectsFolders = QgsApplication::temporarilyUntrustedProjectsFolders();
    temporarilyUntrustedProjectsFolders.removeAll( path );

    if ( buttonType == QDialogButtonBox::StandardButton::YesToAll )
    {
      trustedProjectsFolders << path;
      accepted = true;
    }
    else if ( buttonType == QDialogButtonBox::StandardButton::Yes )
    {
      temporarilyTrustedProjectsFolders << path;
      accepted = true;
    }
    else if ( buttonType == QDialogButtonBox::StandardButton::NoToAll )
    {
      untrustedProjectsFolders << path;
      accepted = false;
    }
    else if ( buttonType == QDialogButtonBox::StandardButton::No )
    {
      temporarilyUntrustedProjectsFolders << path;
      accepted = false;
    }

    trustedProjectsFolders.sort();
    untrustedProjectsFolders.sort();
    temporarilyTrustedProjectsFolders.sort();
    temporarilyUntrustedProjectsFolders.sort();

    QgsSettingsRegistryCore::settingsCodeExecutionTrustedProjectsFolders->setValue( trustedProjectsFolders );
    QgsSettingsRegistryCore::settingsCodeExecutionUntrustedProjectsFolders->setValue( untrustedProjectsFolders );

    QgsApplication::setTemporarilyTrustedProjectsFolders( temporarilyTrustedProjectsFolders );
    QgsApplication::setTemporarilyUntrustedProjectsFolders( temporarilyUntrustedProjectsFolders );
  }

  done( accepted ? QDialog::Accepted : QDialog::Rejected );
}

void QgsProjectTrustDialog::showHelp()
{
  QgsHelp::openHelp( u"introduction/getting_started.html"_s );
}
