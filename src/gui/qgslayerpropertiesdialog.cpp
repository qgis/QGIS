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
#include "moc_qgslayerpropertiesdialog.cpp"
#include "qgsmaplayerloadstyledialog.h"
#include "qgsmaplayersavestyledialog.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsnative.h"
#include "qgssettings.h"
#include "qgsmaplayer.h"
#include "qgsmetadatawidget.h"
#include "qgsproviderregistry.h"
#include "qgsfileutils.h"
#include "qgssldexportcontext.h"
#include "qstackedwidget.h"
#include "qgsmapcanvas.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

QgsLayerPropertiesDialog::QgsLayerPropertiesDialog( QgsMapLayer *layer, QgsMapCanvas *canvas, const QString &settingsKey, QWidget *parent, Qt::WindowFlags fl, QgsSettings *settings )
  : QgsOptionsDialogBase( settingsKey, parent, fl, settings )
  , mCanvas( canvas )
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

  QgsSettings settings; // where we keep last used filter in persistent state
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Layer Metadata" ), lastUsedDir, tr( "QGIS Layer Metadata File" ) + " (*.qmd)" );
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

  QgsSettings settings; // where we keep last used filter in persistent state
  const QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString outputFileName = QFileDialog::getSaveFileName( this, tr( "Save Layer Metadata as QMD" ), lastUsedDir, tr( "QMD File" ) + " (*.qmd)" );
  // return dialog focus on Mac
  activateWindow();
  raise();
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
  const QString infoWindowTitle = QObject::tr( "Save Default Metadata" );
  const QString errorMsg = mLayer->saveDefaultMetadata( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( this, infoWindowTitle, errorMsg );
    refocusDialog();
  }
  else
  {
    QMessageBox::information( this, infoWindowTitle, tr( "Metadata saved." ) );
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
    tr( "QGIS Layer Style File" ) + " (*.qml)"
  );
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
    tr( "QGIS Layer Style File" ) + " (*.qml)"
  );
  // return dialog focus on Mac
  activateWindow();
  raise();
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
    QMessageBox::information( this, tr( "Default Style" ), message );
    refocusDialog();
  }
}

void QgsLayerPropertiesDialog::initialize()
{
  restoreOptionsBaseUi( generateDialogTitle() );
}

void QgsLayerPropertiesDialog::refocusDialog()
{
  activateWindow(); // set focus back to properties dialog
}

void QgsLayerPropertiesDialog::addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory )
{
  if ( !factory->supportsLayer( mLayer ) || !factory->supportLayerPropertiesDialog() )
  {
    return;
  }

  QgsMapLayerConfigWidget *page = factory->createWidget( mLayer, mCanvas, false, this );
  mConfigWidgets << page;

  const QString beforePage = factory->layerPropertiesPagePositionHint();
  if ( beforePage.isEmpty() )
    addPage( factory->title(), factory->title(), factory->icon(), page );
  else
    insertPage( factory->title(), factory->title(), factory->icon(), page, beforePage );

  page->syncToLayer( mLayer );
}

void QgsLayerPropertiesDialog::loadDefaultStyle()
{
  QString msg;
  bool defaultLoadedFlag = false;

  const QgsDataProvider *provider = mLayer->dataProvider();
  if ( !provider )
    return;
  if ( provider->styleStorageCapabilities().testFlag( Qgis::ProviderStyleStorageCapability::LoadFromDatabase ) )
  {
    QMessageBox askToUser;
    askToUser.setText( tr( "Load default style from: " ) );
    askToUser.setIcon( QMessageBox::Question );
    askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
    askToUser.addButton( tr( "Local Database" ), QMessageBox::NoRole );
    askToUser.addButton( tr( "Datasource Database" ), QMessageBox::YesRole );

    switch ( askToUser.exec() )
    {
      case 0:
        return;
      case 2:
        msg = mLayer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag, false );
        if ( !defaultLoadedFlag )
        {
          //something went wrong - let them know why
          QMessageBox::information( this, tr( "Default Style" ), msg );
        }
        if ( msg.compare( tr( "Loaded from Provider" ) ) )
        {
          QMessageBox::information( this, tr( "Default Style" ), tr( "No default style was found for this layer." ) );
        }
        else
        {
          syncToLayer();
          apply();
        }

        return;
      default:
        break;
    }
  }

  QString myMessage = mLayer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag, true );
  //  QString myMessage = layer->loadDefaultStyle( defaultLoadedFlag );
  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    // all worked OK so no need to inform user
    syncToLayer();
    apply();
  }
  else
  {
    //something went wrong - let them know why
    QMessageBox::information( this, tr( "Default Style" ), myMessage );
  }
}

void QgsLayerPropertiesDialog::saveDefaultStyle()
{
  QString errorMsg;
  const QgsDataProvider *provider = mLayer->dataProvider();
  if ( !provider )
    return;
  if ( provider->styleStorageCapabilities().testFlag( Qgis::ProviderStyleStorageCapability::SaveToDatabase ) )
  {
    QMessageBox askToUser;
    askToUser.setText( tr( "Save default style to: " ) );
    askToUser.setIcon( QMessageBox::Question );
    askToUser.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
    askToUser.addButton( tr( "Local Database" ), QMessageBox::NoRole );
    askToUser.addButton( tr( "Datasource Database" ), QMessageBox::YesRole );

    switch ( askToUser.exec() )
    {
      case 0:
        return;
      case 2:
      {
        apply();
        QString errorMessage;
        if ( QgsProviderRegistry::instance()->styleExists( mLayer->providerType(), mLayer->source(), QString(), errorMessage ) )
        {
          if ( QMessageBox::question( nullptr, QObject::tr( "Save style in database" ), QObject::tr( "A matching style already exists in the database for this layer. Do you want to overwrite it?" ), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
          {
            return;
          }
        }
        else if ( !errorMessage.isEmpty() )
        {
          QMessageBox::warning( nullptr, QObject::tr( "Save style in database" ), errorMessage );
          return;
        }

        mLayer->saveStyleToDatabase( QString(), QString(), true, QString(), errorMsg );
        if ( errorMsg.isNull() )
        {
          return;
        }
        break;
      }
      default:
        break;
    }
  }

  QgsLayerPropertiesDialog::saveStyleAsDefault();
}

void QgsLayerPropertiesDialog::saveStyleAs()
{
  if ( !mLayer->dataProvider() )
    return;
  QgsMapLayerSaveStyleDialog dlg( mLayer );

  if ( dlg.exec() )
  {
    apply();

    bool defaultLoadedFlag = false;
    QString errorMessage;

    StyleType type = dlg.currentStyleType();
    switch ( type )
    {
      case QML:
      case SLD:
      {
        QString filePath = dlg.outputFilePath();
        if ( type == QML )
          errorMessage = mLayer->saveNamedStyle( filePath, defaultLoadedFlag, dlg.styleCategories() );
        else
        {
          const QgsSldExportContext sldContext { dlg.sldExportOptions(), Qgis::SldExportVendorExtension::NoVendorExtension, filePath };
          errorMessage = mLayer->saveSldStyleV2( defaultLoadedFlag, sldContext );
        }

        //reset if the default style was loaded OK only
        if ( defaultLoadedFlag )
        {
          syncToLayer();
        }
        else
        {
          //let the user know what went wrong
          QMessageBox::information( this, tr( "Save Style" ), errorMessage );
        }

        break;
      }
      case DatasourceDatabase:
      {
        QString infoWindowTitle = QObject::tr( "Save style to DB (%1)" ).arg( mLayer->providerType() );

        QgsMapLayerSaveStyleDialog::SaveToDbSettings dbSettings = dlg.saveToDbSettings();

        if ( QgsProviderRegistry::instance()->styleExists( mLayer->providerType(), mLayer->source(), dbSettings.name, errorMessage ) )
        {
          if ( QMessageBox::question( nullptr, QObject::tr( "Save style in database" ), QObject::tr( "A matching style already exists in the database for this layer. Do you want to overwrite it?" ), QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
          {
            return;
          }
        }
        else if ( !errorMessage.isEmpty() )
        {
          QMessageBox::warning( this, infoWindowTitle, errorMessage );
          return;
        }

        mLayer->saveStyleToDatabase( dbSettings.name, dbSettings.description, dbSettings.isDefault, dbSettings.uiFileContent, errorMessage, dlg.styleCategories() );

        if ( !errorMessage.isNull() )
        {
          QMessageBox::warning( this, infoWindowTitle, errorMessage );
        }
        else
        {
          QMessageBox::information( this, infoWindowTitle, tr( "Style saved" ) );
        }
        break;
      }
      case UserDatabase:
      {
        QString infoWindowTitle = tr( "Save default style to local database" );
        errorMessage = mLayer->saveDefaultStyle( defaultLoadedFlag, dlg.styleCategories() );
        if ( !defaultLoadedFlag )
        {
          QMessageBox::warning( this, infoWindowTitle, errorMessage );
        }
        else
        {
          QMessageBox::information( this, infoWindowTitle, tr( "Style saved" ) );
        }
        break;
      }
    }
  }
}

void QgsLayerPropertiesDialog::loadStyle()
{
  QString errorMsg;
  QStringList ids, names, descriptions;

  //get the list of styles in the db
  int sectionLimit = mLayer->listStylesInDatabase( ids, names, descriptions, errorMsg );
  QgsMapLayerLoadStyleDialog dlg( mLayer, this );
  dlg.initializeLists( ids, names, descriptions, sectionLimit );

  if ( dlg.exec() )
  {
    mOldStyle = mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() );
    QgsMapLayer::StyleCategories categories = dlg.styleCategories();
    StyleType type = dlg.currentStyleType();
    bool defaultLoadedFlag = false;
    switch ( type )
    {
      case QML:
      case SLD:
      {
        QString filePath = dlg.filePath();
        if ( type == SLD )
        {
          errorMsg = mLayer->loadSldStyle( filePath, defaultLoadedFlag );
        }
        else
        {
          errorMsg = mLayer->loadNamedStyle( filePath, defaultLoadedFlag, true, categories );
        }
        //reset if the default style was loaded OK only
        if ( defaultLoadedFlag )
        {
          syncToLayer();
          apply();
        }
        else
        {
          //let the user know what went wrong
          QMessageBox::warning( this, tr( "Load Style" ), errorMsg );
        }
        break;
      }
      case DatasourceDatabase:
      {
        QString selectedStyleId = dlg.selectedStyleId();

        QString qmlStyle = mLayer->getStyleFromDatabase( selectedStyleId, errorMsg );
        if ( !errorMsg.isNull() )
        {
          QMessageBox::warning( this, tr( "Load Styles from Database" ), errorMsg );
          return;
        }

        QDomDocument myDocument( QStringLiteral( "qgis" ) );
        myDocument.setContent( qmlStyle );

        if ( mLayer->importNamedStyle( myDocument, errorMsg, categories ) )
        {
          syncToLayer();
          apply();
        }
        else
        {
          QMessageBox::warning( this, tr( "Load Styles from Database" ), tr( "The retrieved style is not a valid named style. Error message: %1" ).arg( errorMsg ) );
        }
        break;
      }
      case UserDatabase:
      {
        errorMsg = mLayer->loadNamedStyle( mLayer->styleURI(), defaultLoadedFlag, true, categories );
        //reset if the default style was loaded OK only
        if ( defaultLoadedFlag )
        {
          syncToLayer();
          apply();
        }
        else
        {
          QMessageBox::warning( this, tr( "Load Default Style" ), errorMsg );
        }
        break;
      }
    }
    activateWindow(); // set focus back to properties dialog
  }
}

void QgsLayerPropertiesDialog::storeCurrentStyleForUndo()
{
  if ( !mLayer )
    return;

  mOldStyle = mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() );
}

QString QgsLayerPropertiesDialog::generateDialogTitle() const
{
  QString title = tr( "Layer Properties - %1" ).arg( mLayer->name() );

  if ( !mLayer->styleManager()->isDefault( mLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mLayer->styleManager()->currentStyle() );

  return title;
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
    mBtnStyle->setVisible( !isMetadataPanel );
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
