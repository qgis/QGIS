/***************************************************************************
  qgslayerpropertiesdialog.cpp
  --------------------------------------
  Date                 : June 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayerpropertiesdialog.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsnative.h"
#include "qgssettings.h"
#include "qgsmaplayer.h"
#include "qgsmetadatawidget.h"
#include "qgsfileutils.h"
#include "qstackedwidget.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

QgsLayerPropertiesDialog::QgsLayerPropertiesDialog( QgsMapLayer *layer, const QString &settingsKey, QWidget *parent, Qt::WindowFlags fl, QgsSettings *settings )
  : QgsOptionsDialogBase( settingsKey, parent, fl, settings )
  , mLayer( layer )
{

}

void QgsLayerPropertiesDialog::setMetadataWidget( QgsMetadataWidget *widget, QWidget *page )
{
  mMetadataWidget = widget;
  mMetadataPage = page;
}

void QgsLayerPropertiesDialog::loadMetadataFromFile()
{
  if ( !mLayer || !mMetadataWidget )
    return;

  QgsSettings settings;  // where we keep last used filter in persistent state
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Layer Metadata" ), lastUsedDir,
                           tr( "QGIS Layer Metadata File" ) + " (*.qmd)" );
  if ( fileName.isNull() )
  {
    return;
  }

  bool defaultLoadedFlag = false;
  const QString message = mLayer->loadNamedMetadata( fileName, defaultLoadedFlag );

  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mLayer->metadata() );
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::warning( this, tr( "Load Metadata" ), message );
  }

  settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( fileName ).path() );

  refocusDialog();
}

void QgsLayerPropertiesDialog::saveMetadataToFile()
{
  if ( !mLayer || !mMetadataWidget )
    return;

  QgsSettings settings;  // where we keep last used filter in persistent state
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString outputFileName = QFileDialog::getSaveFileName( this, tr( "Save Layer Metadata as QMD" ),
                           lastUsedDir, tr( "QMD File" ) + " (*.qmd)" );
  if ( outputFileName.isEmpty() )
  {
    return;
  }

  mMetadataWidget->acceptMetadata();

  //ensure the user never omitted the extension from the file name
  if ( !outputFileName.endsWith( QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata ), Qt::CaseInsensitive ) )
  {
    outputFileName += QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata );
  }

  bool defaultLoadedFlag = false;
  const QString message = mLayer->saveNamedMetadata( outputFileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( outputFileName ).absolutePath() );
  else
    QMessageBox::information( this, tr( "Save Metadata" ), message );

  refocusDialog();
}

void QgsLayerPropertiesDialog::saveMetadataAsDefault()
{
  if ( !mLayer || !mMetadataWidget )
    return;

  mMetadataWidget->acceptMetadata();

  bool defaultSavedFlag = false;
  const QString errorMsg = mLayer->saveDefaultMetadata( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( this, tr( "Default Metadata" ), errorMsg );
    refocusDialog();
  }
}

void QgsLayerPropertiesDialog::loadDefaultMetadata()
{
  if ( !mLayer || !mMetadataWidget )
    return;

  bool defaultLoadedFlag = false;
  const QString message = mLayer->loadNamedMetadata( mLayer->metadataUri(), defaultLoadedFlag );
  //reset if the default metadata was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mLayer->metadata() );
  }
  else
  {
    QMessageBox::information( this, tr( "Default Metadata" ), message );
    refocusDialog();
  }
}

void QgsLayerPropertiesDialog::loadStyleFromFile()
{
  if ( !mLayer )
    return;

  QgsSettings settings;
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString fileName = QFileDialog::getOpenFileName(
                       this,
                       tr( "Load layer properties from style file" ),
                       lastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( fileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  if ( !fileName.endsWith( QLatin1String( ".qml" ), Qt::CaseInsensitive ) )
    fileName += QLatin1String( ".qml" );

  storeCurrentStyleForUndo();

  bool defaultLoadedFlag = false;
  const QString message = mLayer->loadNamedStyle( fileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( fileName ).absolutePath() );
    syncToLayer();
  }
  else
  {
    QMessageBox::information( this, tr( "Load Style" ), message );
    refocusDialog();
  }
}

void QgsLayerPropertiesDialog::saveStyleToFile()
{
  if ( !mLayer )
    return;

  QgsSettings settings;
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString outputFileName = QFileDialog::getSaveFileName(
                             this,
                             tr( "Save layer properties as style file" ),
                             lastUsedDir,
                             tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( outputFileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  outputFileName = QgsFileUtils::ensureFileNameHasExtension( outputFileName, QStringList() << QStringLiteral( "qml" ) );

  apply(); // make sure the style to save is up-to-date

  // then export style
  bool defaultLoadedFlag = false;
  const QString message = mLayer->saveNamedStyle( outputFileName, defaultLoadedFlag );

  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( outputFileName ).absolutePath() );
  }
  else
  {
    QMessageBox::information( this, tr( "Save Style" ), message );
    refocusDialog();
  }
}

void QgsLayerPropertiesDialog::saveStyleAsDefault()
{
  if ( !mLayer )
    return;

  apply(); // make sure the style to save is up-to-date

  // a flag passed by reference
  bool defaultSavedFlag = false;
  // TODO Once the deprecated `saveDefaultStyle()` method is gone, just
  // remove the NOWARN_DEPRECATED tags
  Q_NOWARN_DEPRECATED_PUSH
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  const QString message = mLayer->saveDefaultStyle( defaultSavedFlag );
  Q_NOWARN_DEPRECATED_POP
  if ( !defaultSavedFlag )
  {
    // let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              message
                            );
    refocusDialog();
  }
}

void QgsLayerPropertiesDialog::loadDefaultStyle()
{
  if ( !mLayer )
    return;

  bool defaultLoadedFlag = false;
  const QString message = mLayer->loadDefaultStyle( defaultLoadedFlag );
  // reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    syncToLayer();
  }
  else
  {
    // otherwise let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              message
                            );
    refocusDialog();
  }
}

void QgsLayerPropertiesDialog::refocusDialog()
{
  activateWindow(); // set focus back to properties dialog
}

void QgsLayerPropertiesDialog::storeCurrentStyleForUndo()
{
  if ( !mLayer )
    return;

  mOldStyle = mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() );
}

void QgsLayerPropertiesDialog::rollback()
{
  if ( mOldStyle.xmlData() != mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() ).xmlData() )
  {
    // need to reset style to previous - style applied directly to the layer (not in apply())
    QString message;
    QDomDocument doc( QStringLiteral( "qgis" ) );
    int errorLine, errorColumn;
    doc.setContent( mOldStyle.xmlData(), false, &message, &errorLine, &errorColumn );
    mLayer->importNamedStyle( doc, message );
    syncToLayer();
  }
}

void QgsLayerPropertiesDialog::optionsStackedWidget_CurrentChanged( int index )
{
  QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( index );

  if ( mMetadataPage && mBtnStyle && mBtnMetadata )
  {
    const bool isMetadataPanel = ( index == mOptStackedWidget->indexOf( mMetadataPage ) );
    mBtnStyle->setVisible( ! isMetadataPanel );
    mBtnMetadata->setVisible( isMetadataPanel );
  }
}

void QgsLayerPropertiesDialog::openUrl( const QUrl &url )
{
  QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}
