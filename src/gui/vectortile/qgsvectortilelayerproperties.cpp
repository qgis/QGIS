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
#include "moc_qgsvectortilelayerproperties.cpp"
#include "qgshelp.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsprovidersourcewidgetproviderregistry.h"
#include "qgsvectortilebasicrendererwidget.h"
#include "qgsvectortilebasiclabelingwidget.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortileutils.h"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsjsonutils.h"
#include "qgsmetadatawidget.h"
#include "qgsmaplayerloadstyledialog.h"
#include "qgsmapboxglstyleconverter.h"
#include "qgsprovidersourcewidget.h"
#include "qgsdatumtransformdialog.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTextStream>

QgsVectorTileLayerProperties::QgsVectorTileLayerProperties( QgsVectorTileLayer *lyr, QgsMapCanvas *canvas, QgsMessageBar *messageBar, QWidget *parent, Qt::WindowFlags flags )
  : QgsLayerPropertiesDialog( lyr, canvas, QStringLiteral( "VectorTileLayerProperties" ), parent, flags )
  , mLayer( lyr )
{
  setupUi( this );

  mRendererWidget = new QgsVectorTileBasicRendererWidget( nullptr, canvas, messageBar, this );
  mOptsPage_Style->layout()->addWidget( mRendererWidget );
  mOptsPage_Style->layout()->setContentsMargins( 0, 0, 0, 0 );

  mLabelingWidget = new QgsVectorTileBasicLabelingWidget( nullptr, canvas, messageBar, this );
  mOptsPage_Labeling->layout()->addWidget( mLabelingWidget );
  mOptsPage_Labeling->layout()->setContentsMargins( 0, 0, 0, 0 );

  connect( this, &QDialog::accepted, this, &QgsVectorTileLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsVectorTileLayerProperties::rollback );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsVectorTileLayerProperties::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorTileLayerProperties::showHelp );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsVectorTileLayerProperties::crsChanged );

  // scale based layer visibility related widgets
  mScaleRangeWidget->setMapCanvas( mCanvas );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mSourceGroupBox->hide();

#ifdef WITH_QTWEBKIT
  // Setup information tab

  const int horizontalDpi = logicalDpiX();

  // Adjust zoom: text is ok, but HTML seems rather big at least on Linux/KDE
  if ( horizontalDpi > 96 )
  {
    mMetadataViewer->setZoomFactor( mMetadataViewer->zoomFactor() * 0.9 );
  }
  mMetadataViewer->page()->setLinkDelegationPolicy( QWebPage::LinkDelegationPolicy::DelegateAllLinks );
  connect( mMetadataViewer->page(), &QWebPage::linkClicked, this, &QgsVectorTileLayerProperties::openUrl );
  mMetadataViewer->page()->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
  mMetadataViewer->page()->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );

#endif
  mOptsPage_Information->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( metadataFrame );
  layout->setContentsMargins( 0, 0, 0, 0 );
  metadataFrame->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mCanvas );
  layout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( layout );
  mOptsPage_Metadata->setContentsMargins( 0, 0, 0, 0 );

  setMetadataWidget( mMetadataWidget, mOptsPage_Metadata );

  // update based on lyr's current state
  syncToLayer();

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( QStringLiteral( "/Windows/VectorTileLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/VectorTileLayerProperties/tab" ), mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsVectorTileLayerProperties::loadStyle );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsVectorTileLayerProperties::saveStyleToFile );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsVectorTileLayerProperties::saveStyleAsDefault );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsVectorTileLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsVectorTileLayerProperties::aboutToShowStyleMenu );

  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsVectorTileLayerProperties::loadMetadataFromFile );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsVectorTileLayerProperties::saveMetadataToFile );
  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  initialize();
}

void QgsVectorTileLayerProperties::apply()
{
  if ( mSourceWidget )
  {
    const QString newSource = mSourceWidget->sourceUri();
    if ( newSource != mLayer->source() )
    {
      mLayer->setDataSource( newSource, mLayer->name(), mLayer->providerType(), QgsDataProvider::ProviderOptions() );
    }
  }

  mLayer->setName( mLayerOrigNameLineEd->text() );
  mLayer->setCrs( mCrsSelector->crs() );

  mRendererWidget->apply();
  mLabelingWidget->apply();
  mMetadataWidget->acceptMetadata();

  mLayer->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );
  mLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );

  //layer title and abstract
  mLayer->serverProperties()->setShortName( mLayerShortNameLineEdit->text() );
  mLayer->serverProperties()->setTitle( mLayerTitleLineEdit->text() );
  mLayer->serverProperties()->setAbstract( mLayerAbstractTextEdit->toPlainText() );
  mLayer->serverProperties()->setKeywordList( mLayerKeywordListLineEdit->text() );
  mLayer->serverProperties()->setDataUrl( mLayerDataUrlLineEdit->text() );
  mLayer->serverProperties()->setDataUrlFormat( mLayerDataUrlFormatComboBox->currentText() );

  //layer attribution
  mLayer->serverProperties()->setAttribution( mLayerAttributionLineEdit->text() );
  mLayer->serverProperties()->setAttributionUrl( mLayerAttributionUrlLineEdit->text() );

  // LegendURL
  mLayer->setLegendUrl( mLayerLegendUrlLineEdit->text() );
  mLayer->setLegendUrlFormat( mLayerLegendUrlFormatComboBox->currentText() );
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
   * Source
   */

  mLayerOrigNameLineEd->setText( mLayer->name() );
  mCrsSelector->setCrs( mLayer->crs() );

  if ( !mSourceWidget )
  {
    mSourceWidget = QgsGui::sourceWidgetProviderRegistry()->createWidget( mLayer );
    if ( mSourceWidget )
    {
      QHBoxLayout *layout = new QHBoxLayout();
      layout->addWidget( mSourceWidget );
      mSourceGroupBox->setLayout( layout );
      if ( !mSourceWidget->groupTitle().isEmpty() )
        mSourceGroupBox->setTitle( mSourceWidget->groupTitle() );
      mSourceGroupBox->show();

      connect( mSourceWidget, &QgsProviderSourceWidget::validChanged, this, [=]( bool isValid ) {
        buttonBox->button( QDialogButtonBox::Apply )->setEnabled( isValid );
        buttonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid );
      } );
    }
  }

  if ( mSourceWidget )
  {
    mSourceWidget->setMapCanvas( mCanvas );
    mSourceWidget->setSourceUri( mLayer->source() );
  }

  /*
   * Symbology Tab
   */
  mRendererWidget->syncToLayer( mLayer );

  /*
   * Labels Tab
   */
  mLabelingWidget->setLayer( mLayer );

  /*
   * Rendering
   */
  chkUseScaleDependentRendering->setChecked( mLayer->hasScaleBasedVisibility() );
  mScaleRangeWidget->setScaleRange( mLayer->minimumScale(), mLayer->maximumScale() );

  /*
   * Server
   */
  //layer title and abstract
  mLayerShortNameLineEdit->setText( mLayer->serverProperties()->shortName() );
  mLayerTitleLineEdit->setText( mLayer->serverProperties()->title() );
  mLayerAbstractTextEdit->setPlainText( mLayer->serverProperties()->abstract() );
  mLayerKeywordListLineEdit->setText( mLayer->serverProperties()->keywordList() );
  mLayerDataUrlLineEdit->setText( mLayer->serverProperties()->dataUrl() );
  mLayerDataUrlFormatComboBox->setCurrentIndex(
    mLayerDataUrlFormatComboBox->findText(
      mLayer->serverProperties()->dataUrlFormat()
    )
  );
  //layer attribution
  mLayerAttributionLineEdit->setText( mLayer->serverProperties()->attribution() );
  mLayerAttributionUrlLineEdit->setText( mLayer->serverProperties()->attributionUrl() );

  // layer legend url
  mLayerLegendUrlLineEdit->setText( mLayer->legendUrl() );
  mLayerLegendUrlFormatComboBox->setCurrentIndex(
    mLayerLegendUrlFormatComboBox->findText(
      mLayer->legendUrlFormat()
    )
  );
}

void QgsVectorTileLayerProperties::saveDefaultStyle()
{
  saveStyleAsDefault();
}

void QgsVectorTileLayerProperties::loadStyle()
{
  const QgsSettings settings; // where we keep last used filter in persistent state

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
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
        in.setCodec( "UTF-8" );
#endif
        const QString content = in.readAll();

        QgsMapBoxGlStyleConversionContext context;
        // convert automatically from pixel sizes to millimeters, because pixel sizes
        // are a VERY edge case in QGIS and don't play nice with hidpi map renders or print layouts
        context.setTargetUnit( Qgis::RenderUnit::Millimeters );
        //assume source uses 96 dpi
        context.setPixelSizeConversionFactor( 25.4 / 96.0 );

        //load sprites
        QVariantMap styleDefinition = QgsJsonUtils::parseJson( content ).toMap();

        QFileInfo fi( dlg.filePath() );
        QgsVectorTileUtils::loadSprites( styleDefinition, context, QStringLiteral( "file://" ) + fi.absolutePath() );

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
  saveStyleToFile();
}

void QgsVectorTileLayerProperties::aboutToShowStyleMenu()
{
  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mLayer );
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

void QgsVectorTileLayerProperties::crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mCanvas, tr( "Select Transformation" ) );
  mLayer->setCrs( crs );
  mMetadataWidget->crsChanged();
}
