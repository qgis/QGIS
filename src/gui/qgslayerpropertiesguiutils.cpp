/***************************************************************************
  qgslayerpropertiesguiutils.cpp
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

#include "qgslayerpropertiesguiutils.h"
#include "qgssettings.h"
#include "qgsmaplayer.h"
#include "qgsmetadatawidget.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

QgsLayerPropertiesGuiUtils::QgsLayerPropertiesGuiUtils( QWidget *parent, QgsMapLayer *layer, QgsMetadataWidget *metadataWidget )
  : QObject( parent )
  , mParentWidget( parent )
  , mLayer( layer )
  , mMetadataWidget( metadataWidget )
{

}

void QgsLayerPropertiesGuiUtils::loadMetadataFromFile()
{
  if ( !mLayer || !mMetadataWidget )
    return;

  QgsSettings settings;  // where we keep last used filter in persistent state
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  const QString fileName = QFileDialog::getOpenFileName( mParentWidget, tr( "Load Layer Metadata" ), lastUsedDir,
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
    QMessageBox::warning( mParentWidget, tr( "Load Metadata" ), message );
  }

  settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( fileName ).path() );

  refocusParent();
}

void QgsLayerPropertiesGuiUtils::saveMetadataToFile()
{
  if ( !mLayer || !mMetadataWidget )
    return;

  QgsSettings settings;  // where we keep last used filter in persistent state
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString outputFileName = QFileDialog::getSaveFileName( mParentWidget, tr( "Save Layer Metadata as QMD" ),
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
    QMessageBox::information( mParentWidget, tr( "Save Metadata" ), message );

  refocusParent();
}

void QgsLayerPropertiesGuiUtils::saveMetadataAsDefault()
{
  if ( !mLayer || !mMetadataWidget )
    return;

  mMetadataWidget->acceptMetadata();

  bool defaultSavedFlag = false;
  const QString errorMsg = mLayer->saveDefaultMetadata( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( mParentWidget, tr( "Default Metadata" ), errorMsg );
    refocusParent();
  }
}

void QgsLayerPropertiesGuiUtils::loadDefaultMetadata()
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
    QMessageBox::information( mParentWidget, tr( "Default Metadata" ), message );
    refocusParent();
  }
}

void QgsLayerPropertiesGuiUtils::loadStyleFromFile()
{
  if ( !mLayer )
    return;

  QgsSettings settings;
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString fileName = QFileDialog::getOpenFileName(
                       mParentWidget,
                       tr( "Load layer properties from style file" ),
                       lastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( fileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  if ( !fileName.endsWith( QLatin1String( ".qml" ), Qt::CaseInsensitive ) )
    fileName += QLatin1String( ".qml" );

  emit storeCurrentStyleForUndo();

  bool defaultLoadedFlag = false;
  const QString message = mLayer->loadNamedStyle( fileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( fileName ).absolutePath() );
    emit syncDialogToLayer();
  }
  else
  {
    QMessageBox::information( mParentWidget, tr( "Load Style" ), message );
    refocusParent();
  }
}

void QgsLayerPropertiesGuiUtils::saveStyleAsDefault()
{
  if ( !mLayer )
    return;

  emit applyDialogToLayer(); // make sure the style to save is up-to-date

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
    QMessageBox::information( mParentWidget,
                              tr( "Default Style" ),
                              message
                            );
    refocusParent();
  }
}

void QgsLayerPropertiesGuiUtils::loadDefaultStyle()
{
  if ( !mLayer )
    return;

  bool defaultLoadedFlag = false;
  const QString message = mLayer->loadDefaultStyle( defaultLoadedFlag );
  // reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    emit syncDialogToLayer();
  }
  else
  {
    // otherwise let the user know what went wrong
    QMessageBox::information( mParentWidget,
                              tr( "Default Style" ),
                              message
                            );
    refocusParent();
  }
}

void QgsLayerPropertiesGuiUtils::refocusParent()
{
  mParentWidget->activateWindow(); // set focus back to properties dialog
}
