/***************************************************************************
  qgsvectortilelayerproperties.cpp
  --------------------------------------
  Date                 : May 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilelayerproperties.h"

#include "qgsfileutils.h"
#include "qgshelp.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsvectortilebasicrendererwidget.h"
#include "qgsvectortilebasiclabelingwidget.h"
#include "qgsvectortilelayer.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgsapplication.h"
#include "qgsmetadatawidget.h"
#include "qgsmaplayerloadstyledialog.h"
#include "qgsmapboxglstyleconverter.h"
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTextStream>

QgsVectorTileLayerProperties::QgsVectorTileLayerProperties( QgsVectorTileLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent, Qt::WindowFlags flags )
  : QgsOptionsDialogBase( QStringLiteral( "VectorTileLayerProperties" ), parent, flags )
  , mLayer( lyr )
  , mMapCanvas( canvas )
{
  setupUi( this );

  mRendererWidget = new QgsVectorTileBasicRendererWidget( nullptr, canvas, messageBar, this );
  mOptsPage_Style->layout()->addWidget( mRendererWidget );
  mOptsPage_Style->layout()->setContentsMargins( 0, 0, 0, 0 );

  mLabelingWidget = new QgsVectorTileBasicLabelingWidget( nullptr, canvas, messageBar, this );
  mOptsPage_Labeling->layout()->addWidget( mLabelingWidget );
  mOptsPage_Labeling->layout()->setContentsMargins( 0, 0, 0, 0 );

  connect( this, &QDialog::accepted, this, &QgsVectorTileLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsVectorTileLayerProperties::onCancel );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsVectorTileLayerProperties::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorTileLayerProperties::showHelp );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

#ifdef WITH_QTWEBKIT
  // Setup information tab

  const int horizontalDpi = logicalDpiX();

  // Adjust zoom: text is ok, but HTML seems rather big at least on Linux/KDE
  if ( horizontalDpi > 96 )
  {
    mMetadataViewer->setZoomFactor( mMetadataViewer->zoomFactor() * 0.9 );
  }
  mMetadataViewer->page()->setLinkDelegationPolicy( QWebPage::LinkDelegationPolicy::DelegateAllLinks );
  connect( mMetadataViewer->page(), &QWebPage::linkClicked, this, &QgsVectorTileLayerProperties::urlClicked );
  mMetadataViewer->page()->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
  mMetadataViewer->page()->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );

#endif
  mOptsPage_Information->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( metadataFrame );
  layout->setContentsMargins( 0, 0, 0, 0 );
  metadataFrame->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mMapCanvas );
  layout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( layout );
  mOptsPage_Metadata->setContentsMargins( 0, 0, 0, 0 );

  // update based on lyr's current state
  syncToLayer();

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( QStringLiteral( "/Windows/VectorTileLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/VectorTileLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  QString title = tr( "Layer Properties - %1" ).arg( mLayer->name() );

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsVectorTileLayerProperties::loadStyle );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsVectorTileLayerProperties::saveStyleAs );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsVectorTileLayerProperties::saveDefaultStyle );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsVectorTileLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsVectorTileLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsVectorTileLayerProperties::loadMetadata );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsVectorTileLayerProperties::saveMetadataAs );
  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  if ( !mLayer->styleManager()->isDefault( mLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mLayer->styleManager()->currentStyle() );
  restoreOptionsBaseUi( title );
}

void QgsVectorTileLayerProperties::apply()
{
  mRendererWidget->apply();
  mLabelingWidget->apply();
  mMetadataWidget->acceptMetadata();
}

void QgsVectorTileLayerProperties::onCancel()
{
  if ( mOldStyle.xmlData() != mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() ).xmlData() )
  {
    // need to reset style to previous - style applied directly to the layer (not in apply())
    QString myMessage;
    QDomDocument doc( QStringLiteral( "qgis" ) );
    int errorLine, errorColumn;
    doc.setContent( mOldStyle.xmlData(), false, &myMessage, &errorLine, &errorColumn );
    mLayer->importNamedStyle( doc, myMessage );
    syncToLayer();
  }
}

void QgsVectorTileLayerProperties::syncToLayer()
{
  /*
   * Information Tab
   */
  const QString myStyle = QgsApplication::reportStyleSheet( QgsApplication::StyleSheetType::WebBrowser );
  // Inject the stylesheet
  const QString html { mLayer->htmlMetadata().replace( QLatin1String( "<head>" ), QStringLiteral( R"raw(<head><style type="text/css">%1</style>)raw" ) ).arg( myStyle ) };
  mMetadataViewer->setHtml( html );

  /*
   * Symbology Tab
   */
  mRendererWidget->setLayer( mLayer );

  /*
   * Labels Tab
   */
  mLabelingWidget->setLayer( mLayer );
}


void QgsVectorTileLayerProperties::loadDefaultStyle()
{
  bool defaultLoadedFlag = false;
  const QString myMessage = mLayer->loadDefaultStyle( defaultLoadedFlag );
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
                              myMessage
                            );
  }
}

void QgsVectorTileLayerProperties::saveDefaultStyle()
{
  apply(); // make sure the style to save is up-to-date

  // a flag passed by reference
  bool defaultSavedFlag = false;
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  const QString myMessage = mLayer->saveDefaultStyle( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    // let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              myMessage
                            );
  }
}

void QgsVectorTileLayerProperties::loadStyle()
{
  const QgsSettings settings;  // where we keep last used filter in persistent state

  QStringList ids, names, descriptions;

  QgsMapLayerLoadStyleDialog dlg( mLayer );

  if ( dlg.exec() )
  {
    mOldStyle = mLayer->styleManager()->style( mLayer->styleManager()->currentStyle() );
    const QgsMapLayer::StyleCategories categories = dlg.styleCategories();
    const QString type = dlg.fileExtension();
    if ( type.compare( QLatin1String( "qml" ), Qt::CaseInsensitive ) == 0 )
    {
      QString message;
      bool defaultLoadedFlag = false;
      const QString filePath = dlg.filePath();
      message = mLayer->loadNamedStyle( filePath, defaultLoadedFlag, categories );

      //reset if the default style was loaded OK only
      if ( defaultLoadedFlag )
      {
        syncToLayer();
      }
      else
      {
        //let the user know what went wrong
        QMessageBox::warning( this, tr( "Load Style" ), message );
      }
    }
    else if ( type.compare( QLatin1String( "json" ), Qt::CaseInsensitive ) == 0 )
    {
      QFile file( dlg.filePath() );
      if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        QMessageBox::warning( this, tr( "Load Style" ), tr( "Could not read %1" ).arg( QDir::toNativeSeparators( dlg.filePath() ) ) );
      }
      else
      {
        QTextStream in( &file );
        const QString content = in.readAll();

        QgsMapBoxGlStyleConversionContext context;
        // convert automatically from pixel sizes to millimeters, because pixel sizes
        // are a VERY edge case in QGIS and don't play nice with hidpi map renders or print layouts
        context.setTargetUnit( QgsUnitTypes::RenderMillimeters );
        //assume source uses 96 dpi
        context.setPixelSizeConversionFactor( 25.4 / 96.0 );

        QgsMapBoxGlStyleConverter converter;

        if ( converter.convert( content, &context ) != QgsMapBoxGlStyleConverter::Success )
        {
          QMessageBox::warning( this, tr( "Load Style" ), converter.errorMessage() );
        }
        else
        {
          if ( dlg.styleCategories().testFlag( QgsMapLayer::StyleCategory::Symbology ) )
          {
            mLayer->setRenderer( converter.renderer() );
          }
          if ( dlg.styleCategories().testFlag( QgsMapLayer::StyleCategory::Labeling ) )
          {
            mLayer->setLabeling( converter.labeling() );
          }
          syncToLayer();
        }
      }
    }
    activateWindow(); // set focus back to properties dialog
  }
}

void QgsVectorTileLayerProperties::saveStyleAs()
{
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
  QString message;
  message = mLayer->saveNamedStyle( outputFileName, defaultLoadedFlag );

  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( outputFileName ).absolutePath() );
  }
  else
    QMessageBox::information( this, tr( "Save Style" ), message );
}

void QgsVectorTileLayerProperties::aboutToShowStyleMenu()
{
  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
}

void QgsVectorTileLayerProperties::loadMetadata()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  const QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  const QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer metadata from metadata file" ), myLastUsedDir,
                             tr( "QGIS Layer Metadata File" ) + " (*.qmd)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  QString myMessage;
  bool defaultLoadedFlag = false;
  myMessage = mLayer->loadNamedMetadata( myFileName, defaultLoadedFlag );

  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mLayer->metadata() );
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::warning( this, tr( "Load Metadata" ), myMessage );
  }

  const QFileInfo myFI( myFileName );
  const QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

  activateWindow(); // set focus back to properties dialog
}

void QgsVectorTileLayerProperties::saveMetadataAs()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  const QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString myOutputFileName = QFileDialog::getSaveFileName( this, tr( "Save Layer Metadata as QMD" ),
                             myLastUsedDir, tr( "QMD File" ) + " (*.qmd)" );
  if ( myOutputFileName.isNull() ) //dialog canceled
  {
    return;
  }

  mMetadataWidget->acceptMetadata();

  //ensure the user never omitted the extension from the file name
  if ( !myOutputFileName.endsWith( QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata ), Qt::CaseInsensitive ) )
  {
    myOutputFileName += QgsMapLayer::extensionPropertyType( QgsMapLayer::Metadata );
  }

  bool defaultLoadedFlag = false;
  const QString message = mLayer->saveNamedMetadata( myOutputFileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
    myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( myOutputFileName ).absolutePath() );
  else
    QMessageBox::information( this, tr( "Save Metadata" ), message );
}

void QgsVectorTileLayerProperties::showHelp()
{
  const QVariant helpPage = mOptionsStackedWidget->currentWidget()->property( "helpPage" );

  if ( helpPage.isValid() )
  {
    QgsHelp::openHelp( helpPage.toString() );
  }
  else
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector_tiles/vector_tiles_properties.html" ) );
  }
}

void QgsVectorTileLayerProperties::urlClicked( const QUrl &url )
{
  const QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}

void QgsVectorTileLayerProperties::optionsStackedWidget_CurrentChanged( int index )
{
  QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( index );

  const bool isMetadataPanel = ( index == mOptStackedWidget->indexOf( mOptsPage_Metadata ) );
  mBtnStyle->setVisible( ! isMetadataPanel );
  mBtnMetadata->setVisible( isMetadataPanel );
}
