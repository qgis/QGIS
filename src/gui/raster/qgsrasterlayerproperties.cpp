/***************************************************************************
  qgsrasterlayerproperties.cpp  -  description
  -------------------
      begin                : 1/1/2004
      copyright            : (C) 2004 Tim Sutton
      email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlayerproperties.h"

#include <limits>
#include <typeinfo>

#include "qgsapplication.h"
#include "qgsbrightnesscontrastfilter.h"
#include "qgscreaterasterattributetabledialog.h"
#include "qgsdatumtransformdialog.h"
#include "qgsdoublevalidator.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionfinder.h"
#include "qgsfileutils.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgshillshaderendererwidget.h"
#include "qgshuesaturationfilter.h"
#include "qgsloadrasterattributetabledialog.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaptip.h"
#include "qgsmaptoolemitpoint.h"
#include "qgsmetadataurlitemdelegate.h"
#include "qgsmetadatawidget.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsmultibandcolorrendererwidget.h"
#include "qgspalettedrendererwidget.h"
#include "qgsproject.h"
#include "qgsprojectutils.h"
#include "qgsprovidersourcewidget.h"
#include "qgsprovidersourcewidgetproviderregistry.h"
#include "qgsrasterattributetablewidget.h"
#include "qgsrastercontourrendererwidget.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterhistogramwidget.h"
#include "qgsrasterlabelingwidget.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayertemporalpropertieswidget.h"
#include "qgsrasterpyramid.h"
#include "qgsrasterrange.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrastersinglecolorrendererwidget.h"
#include "qgsrastertransparency.h"
#include "qgsrastertransparencywidget.h"
#include "qgssettings.h"
#include "qgssinglebandgrayrendererwidget.h"
#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgsvectorlayer.h"
#include "qgswebframe.h"
#include "qgswebview.h"

#include <QColorDialog>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QLinearGradient>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScreen>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QUrl>
#include <QVector>

#include "moc_qgsrasterlayerproperties.cpp"

QgsRasterLayerProperties::QgsRasterLayerProperties( QgsMapLayer *lyr, QgsMapCanvas *canvas, QWidget *parent, Qt::WindowFlags fl )
  : QgsLayerPropertiesDialog( lyr, canvas, u"RasterLayerProperties"_s, parent, fl )
  // Constant that signals property not used.
  , TRSTRING_NOT_SET( tr( "Not Set" ) )
  , mRasterLayer( qobject_cast<QgsRasterLayer *>( lyr ) )
{
  mGrayMinimumMaximumEstimated = true;
  mRGBMinimumMaximumEstimated = true;

  setupUi( this );

  mMetadataViewer = new QgsWebView( this );
  mOptsPage_Information->layout()->addWidget( mMetadataViewer );

  mRasterTransparencyWidget = new QgsRasterTransparencyWidget( mRasterLayer, canvas, this );

  transparencyScrollArea->setWidget( mRasterTransparencyWidget );

  mLabelingWidget = new QgsRasterLabelingWidget( mRasterLayer, canvas, this );
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  vl->addWidget( mLabelingWidget );
  mOptsPage_Labeling->setLayout( vl );

  connect( buttonBuildPyramids, &QPushButton::clicked, this, &QgsRasterLayerProperties::buttonBuildPyramids_clicked );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsRasterLayerProperties::mCrsSelector_crsChanged );
  connect( mRenderTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterLayerProperties::mRenderTypeComboBox_currentIndexChanged );
  connect( mResetColorRenderingBtn, &QToolButton::clicked, this, &QgsRasterLayerProperties::mResetColorRenderingBtn_clicked );
  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsRasterLayerProperties::showHelp );

  mSourceGroupBox->hide();

  mBtnStyle = new QPushButton( tr( "Style" ) );
  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  connect( lyr->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsRasterLayerProperties::syncToLayer );

  connect( this, &QDialog::accepted, this, &QgsRasterLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsRasterLayerProperties::rollback );

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsRasterLayerProperties::apply );

  cbxPyramidsFormat->addItem( tr( "External" ), QVariant::fromValue( Qgis::RasterPyramidFormat::GeoTiff ) );
  cbxPyramidsFormat->addItem( tr( "Internal (if possible)" ), QVariant::fromValue( Qgis::RasterPyramidFormat::Internal ) );
  cbxPyramidsFormat->addItem( tr( "External (Erdas Imagine)" ), QVariant::fromValue( Qgis::RasterPyramidFormat::Erdas ) );

  // brightness/contrast controls
  connect( mSliderBrightness, &QAbstractSlider::valueChanged, mBrightnessSpinBox, &QSpinBox::setValue );
  connect( mBrightnessSpinBox, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), mSliderBrightness, &QAbstractSlider::setValue );
  mBrightnessSpinBox->setClearValue( 0 );

  connect( mSliderContrast, &QAbstractSlider::valueChanged, mContrastSpinBox, &QSpinBox::setValue );
  connect( mContrastSpinBox, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), mSliderContrast, &QAbstractSlider::setValue );
  mContrastSpinBox->setClearValue( 0 );

  // gamma correction controls
  connect( mSliderGamma, &QAbstractSlider::valueChanged, this, &QgsRasterLayerProperties::updateGammaSpinBox );
  connect( mGammaSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsRasterLayerProperties::updateGammaSlider );
  mGammaSpinBox->setClearValue( 1.0 );

  // Connect saturation slider and spin box
  connect( sliderSaturation, &QAbstractSlider::valueChanged, spinBoxSaturation, &QSpinBox::setValue );
  connect( spinBoxSaturation, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), sliderSaturation, &QAbstractSlider::setValue );
  spinBoxSaturation->setClearValue( 0 );

  // Connect colorize strength slider and spin box
  connect( sliderColorizeStrength, &QAbstractSlider::valueChanged, spinColorizeStrength, &QSpinBox::setValue );
  connect( spinColorizeStrength, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), sliderColorizeStrength, &QAbstractSlider::setValue );
  spinColorizeStrength->setClearValue( 100 );

  // enable or disable saturation slider and spin box depending on grayscale combo choice
  connect( comboGrayscale, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterLayerProperties::toggleSaturationControls );

  // enable or disable colorize colorbutton with colorize checkbox
  connect( mColorizeCheck, &QAbstractButton::toggled, this, &QgsRasterLayerProperties::toggleColorizeControls );

  // enable or disable Build Pyramids button depending on selection in pyramid list
  connect( lbxPyramidResolutions, &QListWidget::itemSelectionChanged, this, &QgsRasterLayerProperties::toggleBuildPyramidsButton );

  mRefreshSettingsWidget->setLayer( mRasterLayer );
  mMapLayerServerPropertiesWidget->setHasWfsTitle( false );
  mMapLayerServerPropertiesWidget->setServerProperties( mRasterLayer->serverProperties() );

  // set up the scale based layer visibility stuff....
  mScaleRangeWidget->setMapCanvas( mCanvas );
  chkUseScaleDependentRendering->setChecked( lyr->hasScaleBasedVisibility() );
  mScaleRangeWidget->setScaleRange( lyr->minimumScale(), lyr->maximumScale() );

  // build GUI components
  QIcon myPyramidPixmap( QgsApplication::getThemeIcon( "/mIconPyramid.svg" ) );
  QIcon myNoPyramidPixmap( QgsApplication::getThemeIcon( "/mIconNoPyramid.svg" ) );

  initMapTipPreview();

  if ( !mRasterLayer )
  {
    return;
  }

  mEnableMapTips->setChecked( mRasterLayer->mapTipsEnabled() );

  updateRasterAttributeTableOptionsPage();

  connect( mRasterLayer, &QgsRasterLayer::rendererChanged, this, &QgsRasterLayerProperties::updateRasterAttributeTableOptionsPage );

  connect( mCreateRasterAttributeTableButton, &QPushButton::clicked, this, [this] {
    if ( mRasterLayer->canCreateRasterAttributeTable() )
    {
      // Create the attribute table from the renderer
      QgsCreateRasterAttributeTableDialog dlg { mRasterLayer };
      dlg.setOpenWhenDoneVisible( false );
      if ( dlg.exec() == QDialog::Accepted )
      {
        updateRasterAttributeTableOptionsPage();
      }
    }
  } );

  connect( mLoadRasterAttributeTableFromFileButton, &QPushButton::clicked, this, [this] {
    // Load the attribute table from a VAT.DBF file
    QgsLoadRasterAttributeTableDialog dlg { mRasterLayer };
    dlg.setOpenWhenDoneVisible( false );
    if ( dlg.exec() == QDialog::Accepted )
    {
      updateRasterAttributeTableOptionsPage();
    }
  } );

  mBackupCrs = mRasterLayer->crs();

  // Handles window modality raising canvas
  if ( mCanvas && mRasterTransparencyWidget->pixelSelectorTool() )
  {
    connect( mRasterTransparencyWidget->pixelSelectorTool(), &QgsMapToolEmitPoint::deactivated, this, [this] {
      hide();
      setModal( true );
      show();
      raise();
      activateWindow();
    } );

    connect( mRasterTransparencyWidget->pbnAddValuesFromDisplay, &QPushButton::clicked, this, [this] {
      hide();
      setModal( false );

      // Transfer focus to the canvas to use the selector tool
      mCanvas->window()->raise();
      mCanvas->window()->activateWindow();
      mCanvas->window()->setFocus();
    } );
  }

  if ( mCanvas )
  {
    mContext = mCanvas->createExpressionContext();
  }
  else
  {
    mContext << QgsExpressionContextUtils::globalScope()
             << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
             << QgsExpressionContextUtils::atlasScope( nullptr )
             << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }

  // Initialize with layer center
  mContext << QgsExpressionContextUtils::mapLayerPositionScope( mRasterLayer->extent().center() );
  mContext << QgsExpressionContextUtils::layerScope( mRasterLayer );

  connect( mInsertExpressionButton, &QAbstractButton::clicked, this, [this] {
    // Get the linear indexes if the start and end of the selection
    int selectionStart = mMapTipWidget->selectionStart();
    int selectionEnd = mMapTipWidget->selectionEnd();
    QString expression = QgsExpressionFinder::findAndSelectActiveExpression( mMapTipWidget );
    QgsExpressionBuilderDialog exprDlg( nullptr, expression, this, u"generic"_s, mContext );

    exprDlg.setWindowTitle( tr( "Insert Expression" ) );
    if ( exprDlg.exec() == QDialog::Accepted && !exprDlg.expressionText().trimmed().isEmpty() )
      mMapTipWidget->insertText( "[%" + exprDlg.expressionText().trimmed() + "%]" );
    else // Restore the selection
      mMapTipWidget->setLinearSelection( selectionStart, selectionEnd );
  } );

  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();

  // Only do pyramids if dealing directly with GDAL.
  if ( provider && ( provider->capabilities() & Qgis::RasterInterfaceCapability::BuildPyramids || provider->providerCapabilities() & Qgis::RasterProviderCapability::BuildPyramids ) )
  {
    // initialize resampling methods
    cboResamplingMethod->clear();

    const auto constProviderType = QgsRasterDataProvider::pyramidResamplingMethods( mRasterLayer->providerType() );
    for ( const QPair<QString, QString> &method : std::as_const( constProviderType ) )
    {
      cboResamplingMethod->addItem( method.second, method.first );
    }

    // keep it in sync with qgsrasterpyramidsoptionwidget.cpp
    QString prefix = provider->name() + "/driverOptions/_pyramids/";
    QgsSettings mySettings;
    QString defaultMethod = mySettings.value( prefix + "resampling", "AVERAGE" ).toString();
    int idx = cboResamplingMethod->findData( defaultMethod );
    if ( idx >= 0 )
      cboResamplingMethod->setCurrentIndex( idx );


    // build pyramid list
    const QList<QgsRasterPyramid> myPyramidList = provider->buildPyramidList();

    for ( const QgsRasterPyramid &pyramid : myPyramidList )
    {
      if ( pyramid.getExists() )
      {
        lbxPyramidResolutions->addItem( new QListWidgetItem( myPyramidPixmap, QString::number( pyramid.getXDim() ) + u" x "_s + QString::number( pyramid.getYDim() ) ) );
      }
      else
      {
        lbxPyramidResolutions->addItem( new QListWidgetItem( myNoPyramidPixmap, QString::number( pyramid.getXDim() ) + u" x "_s + QString::number( pyramid.getYDim() ) ) );
      }
    }
  }
  else
  {
    // disable Pyramids tab completely
    mOptsPage_Pyramids->setEnabled( false );
  }

  // We can calculate histogram for all data sources but estimated only if
  // size is unknown - could also be enabled if well supported (estimated histogram
  // and let user know that it is estimated)
  if ( !provider || !( provider->capabilities() & Qgis::RasterInterfaceCapability::Size ) )
  {
    // disable Histogram tab completely
    mOptsPage_Histogram->setEnabled( false );
  }

  QVBoxLayout *layout = new QVBoxLayout( metadataFrame );
  layout->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mRasterLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mCanvas );
  layout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( layout );

  QVBoxLayout *temporalLayout = new QVBoxLayout( temporalFrame );
  temporalLayout->setContentsMargins( 0, 0, 0, 0 );
  mTemporalWidget = new QgsRasterLayerTemporalPropertiesWidget( this, mRasterLayer );
  temporalLayout->addWidget( mTemporalWidget );

  QgsDebugMsgLevel( "Setting crs to " + mRasterLayer->crs().toWkt( Qgis::CrsWktVariant::Preferred ), 2 );
  QgsDebugMsgLevel( "Setting crs to " + mRasterLayer->crs().userFriendlyIdentifier(), 2 );
  mCrsSelector->setCrs( mRasterLayer->crs() );

  // Set text for pyramid info box
  QString pyramidFormat( u"<h2>%1</h2><p>%2 %3 %4</p><b><font color='red'><p>%5</p><p>%6</p>"_s );
  QString pyramidHeader = tr( "Description" );
  QString pyramidSentence1 = tr( "Large resolution raster layers can slow navigation in QGIS." );
  QString pyramidSentence2 = tr( "By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom." );
  QString pyramidSentence3 = tr( "You must have write access in the directory where the original data is stored to build pyramids." );
  QString pyramidSentence4 = tr( "Please note that building internal pyramids may alter the original data file and once created they cannot be removed!" );
  QString pyramidSentence5 = tr( "Please note that building internal pyramids could corrupt your image - always make a backup of your data first!" );

  tePyramidDescription->setHtml( pyramidFormat.arg( pyramidHeader, pyramidSentence1, pyramidSentence2, pyramidSentence3, pyramidSentence4, pyramidSentence5 ) );

  //resampling
  mResamplingGroupBox->setSaveCheckedState( true );
  mResamplingUtils.initWidgets( mRasterLayer, mZoomedInResamplingComboBox, mZoomedOutResamplingComboBox, mMaximumOversamplingSpinBox, mCbEarlyResampling );
  mResamplingUtils.refreshWidgetsFromLayer();

  const QgsRasterRenderer *renderer = mRasterLayer->renderer();

  btnColorizeColor->setColorDialogTitle( tr( "Select Color" ) );
  btnColorizeColor->setContext( u"symbology"_s );

  // Hue and saturation color control
  const QgsHueSaturationFilter *hueSaturationFilter = mRasterLayer->hueSaturationFilter();
  //set hue and saturation controls to current values
  if ( hueSaturationFilter )
  {
    sliderSaturation->setValue( hueSaturationFilter->saturation() );
    comboGrayscale->setCurrentIndex( ( int ) hueSaturationFilter->grayscaleMode() );

    // Set initial state of saturation controls based on grayscale mode choice
    toggleSaturationControls( static_cast<int>( hueSaturationFilter->grayscaleMode() ) );

    // Set initial state of colorize controls
    mColorizeCheck->setChecked( hueSaturationFilter->colorizeOn() );
    btnColorizeColor->setColor( hueSaturationFilter->colorizeColor() );
    toggleColorizeControls( hueSaturationFilter->colorizeOn() );
    sliderColorizeStrength->setValue( hueSaturationFilter->colorizeStrength() );
    mInvertColorsCheck->setChecked( hueSaturationFilter->invertColors() );
  }

  //blend mode
  mBlendModeComboBox->setShowClippingModes( QgsProjectUtils::layerIsContainedInGroupLayer( QgsProject::instance(), mRasterLayer ) );
  mBlendModeComboBox->setBlendMode( mRasterLayer->blendMode() );

  //transparency band
  if ( provider )
  {
    mRasterTransparencyWidget->cboxTransparencyBand->setShowNotSetOption( true, tr( "None" ) );
    mRasterTransparencyWidget->cboxTransparencyBand->setLayer( mRasterLayer );

// Alpha band is set in sync()
#if 0
    if ( renderer )
    {
      QgsDebugMsgLevel( u"alphaBand = %1"_s.arg( renderer->alphaBand() ), 2 );
      if ( renderer->alphaBand() > 0 )
      {
        cboxTransparencyBand->setCurrentIndex( cboxTransparencyBand->findData( renderer->alphaBand() ) );
      }
    }
#endif
  }

  // create histogram widget
  mHistogramWidget = nullptr;
  if ( mOptsPage_Histogram->isEnabled() )
  {
    mHistogramWidget = new QgsRasterHistogramWidget( mRasterLayer, mOptsPage_Histogram );
    mHistogramStackedWidget->addWidget( mHistogramWidget );
  }

  //insert renderer widgets into registry
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( u"paletted"_s, QgsPalettedRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( u"multibandcolor"_s, QgsMultiBandColorRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( u"singlebandpseudocolor"_s, QgsSingleBandPseudoColorRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( u"singlebandgray"_s, QgsSingleBandGrayRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( u"hillshade"_s, QgsHillshadeRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( u"contour"_s, QgsRasterContourRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( u"singlecolor"_s, QgsRasterSingleColorRendererWidget::create );

  //fill available renderers into combo box
  QgsRasterRendererRegistryEntry entry;
  mDisableRenderTypeComboBoxCurrentIndexChanged = true;
  const auto constRenderersList = QgsApplication::rasterRendererRegistry()->renderersList();
  for ( const QString &name : constRenderersList )
  {
    if ( QgsApplication::rasterRendererRegistry()->rendererData( name, entry ) )
    {
      if ( ( mRasterLayer->rasterType() != Qgis::RasterLayerType::SingleBandColorData && entry.name != "singlebandcolordata"_L1 ) || ( mRasterLayer->rasterType() == Qgis::RasterLayerType::SingleBandColorData && entry.name == "singlebandcolordata"_L1 ) )
      {
        mRenderTypeComboBox->addItem( entry.visibleName, entry.name );
      }
    }
  }
  mDisableRenderTypeComboBoxCurrentIndexChanged = false;

  int widgetIndex = 0;
  if ( renderer )
  {
    QString rendererType = renderer->type();
    widgetIndex = mRenderTypeComboBox->findData( rendererType );
    if ( widgetIndex != -1 )
    {
      mDisableRenderTypeComboBoxCurrentIndexChanged = true;
      mRenderTypeComboBox->setCurrentIndex( widgetIndex );
      mDisableRenderTypeComboBoxCurrentIndexChanged = false;
    }

    if ( rendererType == "singlebandcolordata"_L1 && mRenderTypeComboBox->count() == 1 )
    {
      // no band rendering options for singlebandcolordata, so minimize group box
      QSizePolicy sizep = mBandRenderingGrpBx->sizePolicy();
      sizep.setVerticalStretch( 0 );
      sizep.setVerticalPolicy( QSizePolicy::Maximum );
      mBandRenderingGrpBx->setSizePolicy( sizep );
      mBandRenderingGrpBx->updateGeometry();
    }

    if ( mRasterLayer->providerType() != "wms"_L1 )
    {
      mWMSPrintGroupBox->hide();
      mPublishDataSourceUrlCheckBox->hide();
      mBackgroundLayerCheckBox->hide();
    }
  }

  initializeDataDefinedButton( mRasterTransparencyWidget->mOpacityDDBtn, QgsRasterPipe::Property::RendererOpacity );

  mRenderTypeComboBox_currentIndexChanged( widgetIndex );

  setMetadataWidget( mMetadataWidget, mOptsPage_Metadata );

  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsRasterLayerProperties::loadStyle );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsRasterLayerProperties::saveStyleAs );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsRasterLayerProperties::saveStyleAsDefault );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsRasterLayerProperties::loadDefaultStyle );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsRasterLayerProperties::aboutToShowStyleMenu );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsRasterLayerProperties::loadMetadataFromFile );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsRasterLayerProperties::saveMetadataToFile );
  menuMetadata->addSeparator();
  menuMetadata->addAction( tr( "Save as Default" ), this, &QgsRasterLayerProperties::saveMetadataAsDefault );
  menuMetadata->addAction( tr( "Restore Default" ), this, &QgsRasterLayerProperties::loadDefaultMetadata );
  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  // update based on lyr's current state
  sync();

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( u"/Windows/RasterLayerProperties/tab"_s ) )
  {
    settings.setValue( u"Windows/RasterLayerProperties/tab"_s, mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  mResetColorRenderingBtn->setIcon( QgsApplication::getThemeIcon( u"/mActionUndo.svg"_s ) );

  optionsStackedWidget_CurrentChanged( mOptionsStackedWidget->currentIndex() );

  //Add help page references
  mOptsPage_Information->setProperty( "helpPage", u"working_with_raster/raster_properties.html#information-properties"_s );
  mOptsPage_Source->setProperty( "helpPage", u"working_with_raster/raster_properties.html#source-properties"_s );
  mOptsPage_Style->setProperty( "helpPage", u"working_with_raster/raster_properties.html#symbology-properties"_s );
  mOptsPage_Transparency->setProperty( "helpPage", u"working_with_raster/raster_properties.html#transparency-properties"_s );

  if ( mOptsPage_Histogram )
    mOptsPage_Histogram->setProperty( "helpPage", u"working_with_raster/raster_properties.html#histogram-properties"_s );

  mOptsPage_Rendering->setProperty( "helpPage", u"working_with_raster/raster_properties.html#rendering-properties"_s );
  mOptsPage_Temporal->setProperty( "helpPage", u"working_with_raster/raster_properties.html#temporal-properties"_s );

  if ( mOptsPage_Pyramids )
    mOptsPage_Pyramids->setProperty( "helpPage", u"working_with_raster/raster_properties.html#pyramids-properties"_s );

  if ( mOptsPage_Display )
    mOptsPage_Display->setProperty( "helpPage", u"working_with_raster/raster_properties.html#display-properties"_s );

  mOptsPage_Metadata->setProperty( "helpPage", u"working_with_raster/raster_properties.html#metadata-properties"_s );
  mOptsPage_Legend->setProperty( "helpPage", u"working_with_raster/raster_properties.html#legend-properties"_s );
  mOptsPage_Server->setProperty( "helpPage", u"working_with_raster/raster_properties.html#server-properties"_s );

  initialize();
}

void QgsRasterLayerProperties::addPropertiesPageFactory( const QgsMapLayerConfigWidgetFactory *factory )
{
  if ( !factory->supportsLayer( mRasterLayer ) || !factory->supportLayerPropertiesDialog() )
  {
    return;
  }

  QgsMapLayerConfigWidget *page = factory->createWidget( mRasterLayer, nullptr, false, this );
  switch ( factory->parentPage() )
  {
    case QgsMapLayerConfigWidgetFactory::ParentPage::NoParent:
    {
      mConfigWidgets << page;

      const QString beforePage = factory->layerPropertiesPagePositionHint();
      if ( beforePage.isEmpty() )
        addPage( factory->title(), factory->title(), factory->icon(), page );
      else
        insertPage( factory->title(), factory->title(), factory->icon(), page, beforePage );
      break;
    }

    case QgsMapLayerConfigWidgetFactory::ParentPage::Temporal:
      mTemporalWidget->addWidget( page );
      break;
  }
}

QgsExpressionContext QgsRasterLayerProperties::createExpressionContext() const
{
  return mContext;
}

void QgsRasterLayerProperties::updateRasterAttributeTableOptionsPage()
{
  if ( mRasterAttributeTableWidget )
  {
    mOptsPage_RasterAttributeTable->layout()->removeWidget( mRasterAttributeTableWidget );
    mRasterAttributeTableWidget = nullptr;
  }

  // Setup raster attribute table
  if ( mRasterLayer->attributeTableCount() > 0 )
  {
    mRasterAttributeTableWidget = new QgsRasterAttributeTableWidget( this, mRasterLayer );
    mOptsPage_RasterAttributeTable->layout()->addWidget( mRasterAttributeTableWidget );
    // When the renderer changes we need to sync the style options page
    connect( mRasterAttributeTableWidget, &QgsRasterAttributeTableWidget::rendererChanged, this, &QgsRasterLayerProperties::syncToLayer );
    mNoRasterAttributeTableWidget->hide();
  }
  else
  {
    mNoRasterAttributeTableWidget->show();
    mCreateRasterAttributeTableButton->setEnabled( mRasterLayer->canCreateRasterAttributeTable() );
  }
}

void QgsRasterLayerProperties::setRendererWidget( const QString &rendererName )
{
  QgsDebugMsgLevel( "rendererName = " + rendererName, 3 );
  QgsRasterRendererWidget *oldWidget = mRendererWidget;
  QgsRasterRenderer *oldRenderer = mRasterLayer->renderer();

  int alphaBand = -1;
  double opacity = 1;
  QColor nodataColor;
  const QList<int> oldBands = oldRenderer ? oldRenderer->usesBands() : QList<int>();
  if ( oldRenderer )
  {
    // Retain alpha band and opacity when switching renderer
    alphaBand = oldRenderer->alphaBand();
    opacity = oldRenderer->opacity();
    nodataColor = oldRenderer->nodataColor();
  }

  QgsRasterRendererRegistryEntry rendererEntry;
  if ( QgsApplication::rasterRendererRegistry()->rendererData( rendererName, rendererEntry ) )
  {
    if ( rendererEntry.widgetCreateFunction ) //single band color data renderer e.g. has no widget
    {
      QgsDebugMsgLevel( u"renderer has widgetCreateFunction"_s, 3 );
      // Current canvas extent (used to calc min/max) in layer CRS
      QgsRectangle myExtent = mCanvas->mapSettings().outputExtentToLayerExtent( mRasterLayer, mCanvas->extent() );
      if ( oldWidget && ( !oldRenderer || rendererName != oldRenderer->type() ) )
      {
        if ( rendererName == "singlebandgray"_L1 )
        {
          whileBlocking( mRasterLayer )->setRenderer( QgsApplication::rasterRendererRegistry()->defaultRendererForDrawingStyle( Qgis::RasterDrawingStyle::SingleBandGray, mRasterLayer->dataProvider() ) );
          whileBlocking( mRasterLayer )->setDefaultContrastEnhancement();
        }
        else if ( rendererName == "multibandcolor"_L1 )
        {
          whileBlocking( mRasterLayer )->setRenderer( QgsApplication::rasterRendererRegistry()->defaultRendererForDrawingStyle( Qgis::RasterDrawingStyle::MultiBandColor, mRasterLayer->dataProvider() ) );
          whileBlocking( mRasterLayer )->setDefaultContrastEnhancement();
        }
      }
      mRasterLayer->renderer()->setAlphaBand( alphaBand );
      mRasterLayer->renderer()->setOpacity( opacity );
      mRasterLayer->renderer()->setNodataColor( nodataColor );
      mRendererWidget = rendererEntry.widgetCreateFunction( mRasterLayer, myExtent );
      mRendererWidget->setMapCanvas( mCanvas );
      mRendererStackedWidget->addWidget( mRendererWidget );
      if ( oldWidget )
      {
        //compare used bands in new and old renderer and reset transparency dialog if different
        std::unique_ptr<QgsRasterRenderer> newRenderer;
        newRenderer.reset( mRendererWidget->renderer() );
        const QList<int> newBands = newRenderer->usesBands();
        if ( oldBands != newBands )
        {
          mRasterTransparencyWidget->syncToLayer();
        }
      }
    }
  }

  const int widgetIndex = mRenderTypeComboBox->findData( rendererName );
  if ( widgetIndex != -1 )
  {
    mDisableRenderTypeComboBoxCurrentIndexChanged = true;
    mRenderTypeComboBox->setCurrentIndex( widgetIndex );
    mDisableRenderTypeComboBoxCurrentIndexChanged = false;
  }

  if ( mRendererWidget != oldWidget )
    delete oldWidget;

  if ( mHistogramWidget )
  {
    mHistogramWidget->setRendererWidget( rendererName, mRendererWidget );
  }
}

void QgsRasterLayerProperties::sync()
{
  QgsSettings myQSettings;

  if ( !mSourceWidget )
  {
    mSourceWidget = QgsGui::sourceWidgetProviderRegistry()->createWidget( mRasterLayer );
    if ( mSourceWidget )
    {
      QHBoxLayout *layout = new QHBoxLayout();
      layout->addWidget( mSourceWidget );
      mSourceGroupBox->setLayout( layout );
      if ( !mSourceWidget->groupTitle().isEmpty() )
        mSourceGroupBox->setTitle( mSourceWidget->groupTitle() );
      mSourceGroupBox->show();

      connect( mSourceWidget, &QgsProviderSourceWidget::validChanged, this, [this]( bool isValid ) {
        buttonBox->button( QDialogButtonBox::Apply )->setEnabled( isValid );
        buttonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid );
      } );
    }
  }

  if ( mSourceWidget )
  {
    mSourceWidget->setMapCanvas( mCanvas );
    mSourceWidget->setSourceUri( mRasterLayer->source() );
  }

  const QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
    return;

  mRasterTransparencyWidget->syncToLayer();
  mLabelingWidget->syncToLayer( mRasterLayer );

  if ( provider->dataType( 1 ) == Qgis::DataType::ARGB32
       || provider->dataType( 1 ) == Qgis::DataType::ARGB32_Premultiplied )
  {
    mRasterTransparencyWidget->gboxNoDataValue->setEnabled( false );
    mRasterTransparencyWidget->gboxCustomTransparency->setEnabled( false );
    mOptionsStackedWidget->setCurrentWidget( mOptsPage_Server );
  }

  // TODO: Wouldn't it be better to just removeWidget() the tabs than delete them? [LS]
  if ( !( provider->capabilities() & Qgis::RasterInterfaceCapability::BuildPyramids
          || provider->providerCapabilities() & Qgis::RasterProviderCapability::BuildPyramids ) )
  {
    if ( mOptsPage_Pyramids )
    {
      delete mOptsPage_Pyramids;
      mOptsPage_Pyramids = nullptr;
    }
  }

  if ( !( provider->capabilities() & Qgis::RasterInterfaceCapability::Size ) )
  {
    if ( mOptsPage_Histogram )
    {
      delete mOptsPage_Histogram;
      mOptsPage_Histogram = nullptr;
      delete mHistogramWidget;
      mHistogramWidget = nullptr;
    }
  }

  QgsDebugMsgLevel( u"populate transparency tab"_s, 3 );

  /*
   * Style tab
   */

  //set brightness, contrast and gamma
  QgsBrightnessContrastFilter *brightnessFilter = mRasterLayer->brightnessFilter();
  if ( brightnessFilter )
  {
    mSliderBrightness->setValue( brightnessFilter->brightness() );
    mSliderContrast->setValue( brightnessFilter->contrast() );
    mGammaSpinBox->setValue( brightnessFilter->gamma() );
  }

  // Hue and saturation color control
  const QgsHueSaturationFilter *hueSaturationFilter = mRasterLayer->hueSaturationFilter();
  //set hue and saturation controls to current values
  if ( hueSaturationFilter )
  {
    sliderSaturation->setValue( hueSaturationFilter->saturation() );
    comboGrayscale->setCurrentIndex( ( int ) hueSaturationFilter->grayscaleMode() );

    // Set state of saturation controls based on grayscale mode choice
    toggleSaturationControls( static_cast<int>( hueSaturationFilter->grayscaleMode() ) );

    // Set state of colorize controls
    mColorizeCheck->setChecked( hueSaturationFilter->colorizeOn() );
    btnColorizeColor->setColor( hueSaturationFilter->colorizeColor() );
    toggleColorizeControls( hueSaturationFilter->colorizeOn() );
    sliderColorizeStrength->setValue( hueSaturationFilter->colorizeStrength() );
    mInvertColorsCheck->setChecked( hueSaturationFilter->invertColors() );
  }

  // Resampling
  mResamplingUtils.refreshWidgetsFromLayer();

  mRefreshSettingsWidget->syncToLayer();
  mMapLayerServerPropertiesWidget->sync();

  QgsDebugMsgLevel( u"populate general tab"_s, 3 );
  /*
   * General Tab
   */

  mLayerOrigNameLineEd->setText( mRasterLayer->name() );

  QgsDebugMsgLevel( u"populate metadata tab"_s, 2 );
  /*
   * Metadata Tab
   */
  //populate the metadata tab's text browser widget with gdal metadata info
  updateInformationContent();

  mEnableMapTips->setChecked( mRasterLayer->mapTipsEnabled() );
  mMapTipWidget->setText( mRasterLayer->mapTipTemplate() );

  //WMS print layer
  QVariant wmsPrintLayer = mRasterLayer->customProperty( u"WMSPrintLayer"_s );
  if ( wmsPrintLayer.isValid() )
  {
    mWMSPrintLayerLineEdit->setText( wmsPrintLayer.toString() );
  }

  QVariant wmsPublishDataSourceUrl = mRasterLayer->customProperty( u"WMSPublishDataSourceUrl"_s, false );
  mPublishDataSourceUrlCheckBox->setChecked( wmsPublishDataSourceUrl.toBool() );

  QVariant wmsBackgroundLayer = mRasterLayer->customProperty( u"WMSBackgroundLayer"_s, false );
  mBackgroundLayerCheckBox->setChecked( wmsBackgroundLayer.toBool() );

  mLegendPlaceholderWidget->setLastPathSettingsKey( u"lastLegendPlaceholderDir"_s );
  mLegendPlaceholderWidget->setSource( mRasterLayer->legendPlaceholderImage() );
  mLegendConfigEmbeddedWidget->setLayer( mRasterLayer );
  mIncludeByDefaultInLayoutLegendsCheck->setChecked( mRasterLayer->legend() && !mRasterLayer->legend()->flags().testFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault ) );

  mTemporalWidget->syncToLayer();

  mPropertyCollection = mRasterLayer->pipe()->dataDefinedProperties();
  updateDataDefinedButtons();

  for ( QgsMapLayerConfigWidget *page : std::as_const( mConfigWidgets ) )
  {
    page->syncToLayer( mRasterLayer );
  }
}

void QgsRasterLayerProperties::apply()
{
  if ( mSourceWidget )
  {
    const QString newSource = mSourceWidget->sourceUri();
    if ( newSource != mRasterLayer->source() )
    {
      mRasterLayer->setDataSource( newSource, mRasterLayer->name(), mRasterLayer->providerType(), QgsDataProvider::ProviderOptions() );
    }
  }

  // Do nothing on "bad" layers
  if ( !mRasterLayer->isValid() )
    return;

  // apply all plugin dialogs
  for ( QgsMapLayerConfigWidget *page : std::as_const( mConfigWidgets ) )
  {
    page->apply();
  }


  /*
   * Legend Tab
   */
  mRasterLayer->setLegendPlaceholderImage( mLegendPlaceholderWidget->source() );
  mLegendConfigEmbeddedWidget->applyToLayer();

  QgsDebugMsgLevel( u"apply processing symbology tab"_s, 3 );
  /*
   * Symbology Tab
   */

  //set whether the layer histogram should be inverted
  //mRasterLayer->setInvertHistogram( cboxInvertColorMap->isChecked() );

  mRasterLayer->brightnessFilter()->setBrightness( mSliderBrightness->value() );
  mRasterLayer->brightnessFilter()->setContrast( mSliderContrast->value() );
  mRasterLayer->brightnessFilter()->setGamma( mGammaSpinBox->value() );

  QgsDebugMsgLevel( u"processing transparency tab"_s, 3 );
  /*
   * Transparent Pixel Tab
   */

  mRasterTransparencyWidget->applyToRasterProvider( mRasterLayer->dataProvider() );

  //set renderer from widget
  QgsRasterRendererWidget *rendererWidget = dynamic_cast<QgsRasterRendererWidget *>( mRendererStackedWidget->currentWidget() );
  if ( rendererWidget )
  {
    rendererWidget->doComputations();

    mRasterLayer->setRenderer( rendererWidget->renderer() );
  }

  mBackupCrs = mRasterLayer->crs();
  mMetadataWidget->acceptMetadata();
  mMetadataFilled = false;

  //transparency settings
  QgsRasterRenderer *rasterRenderer = mRasterLayer->renderer();
  mRasterTransparencyWidget->applyToRasterRenderer( rasterRenderer );

  mLabelingWidget->apply();

  if ( rasterRenderer )
  {
    // Sync the layer styling widget
    mRasterLayer->emitStyleChanged();
  }

  QgsDebugMsgLevel( u"processing general tab"_s, 3 );
  /*
   * General Tab
   */
  mRasterLayer->setName( mLayerOrigNameLineEd->text() );

  // set up the scale based layer visibility stuff....
  mRasterLayer->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mRasterLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );
  mRasterLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );

  mRefreshSettingsWidget->saveToLayer();
  if ( mMapLayerServerPropertiesWidget->save() )
    mMetadataFilled = true;

  //update the legend pixmap
  // pixmapLegend->setPixmap( mRasterLayer->legendAsPixmap() );
  // pixmapLegend->setScaledContents( true );
  // pixmapLegend->repaint();

  mResamplingUtils.refreshLayerFromWidgets();

  // Hue and saturation controls
  QgsHueSaturationFilter *hueSaturationFilter = mRasterLayer->hueSaturationFilter();
  if ( hueSaturationFilter )
  {
    hueSaturationFilter->setSaturation( sliderSaturation->value() );
    hueSaturationFilter->setGrayscaleMode( ( QgsHueSaturationFilter::GrayscaleMode ) comboGrayscale->currentIndex() );
    hueSaturationFilter->setColorizeOn( mColorizeCheck->checkState() );
    hueSaturationFilter->setColorizeColor( btnColorizeColor->color() );
    hueSaturationFilter->setColorizeStrength( sliderColorizeStrength->value() );
    hueSaturationFilter->setInvertColors( mInvertColorsCheck->isChecked() );
  }

  //set the blend mode for the layer
  mRasterLayer->setBlendMode( mBlendModeComboBox->blendMode() );

  // Update temporal properties
  mTemporalWidget->saveTemporalProperties();

  mRasterLayer->setCrs( mCrsSelector->crs() );

  if ( !mWMSPrintLayerLineEdit->text().isEmpty() )
  {
    mRasterLayer->setCustomProperty( u"WMSPrintLayer"_s, mWMSPrintLayerLineEdit->text() );
  }

  mRasterLayer->setCustomProperty( "WMSPublishDataSourceUrl", mPublishDataSourceUrlCheckBox->isChecked() );
  mRasterLayer->setCustomProperty( "WMSBackgroundLayer", mBackgroundLayerCheckBox->isChecked() );

  mRasterLayer->pipe()->setDataDefinedProperties( mPropertyCollection );

  mRasterLayer->setMapTipsEnabled( mEnableMapTips->isChecked() );
  mRasterLayer->setMapTipTemplate( mMapTipWidget->text() );

  // Force a redraw of the legend
  mRasterLayer->setLegend( QgsMapLayerLegend::defaultRasterLegend( mRasterLayer ) );
  if ( QgsMapLayerLegend *legend = mRasterLayer->legend() )
  {
    legend->setFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault, !mIncludeByDefaultInLayoutLegendsCheck->isChecked() );
  }

  //make sure the layer is redrawn
  mRasterLayer->triggerRepaint();

  // notify the project we've made a change
  QgsProject::instance()->setDirty( true );
}

void QgsRasterLayerProperties::buttonBuildPyramids_clicked()
{
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();

  auto feedback = std::make_unique<QgsRasterBlockFeedback>();

  connect( feedback.get(), &QgsRasterBlockFeedback::progressChanged, mPyramidProgress, &QProgressBar::setValue );
  //
  // Go through the list marking any files that are selected in the listview
  // as true so that we can generate pyramids for them.
  //
  QList<QgsRasterPyramid> myPyramidList = provider->buildPyramidList();
  for ( int myCounterInt = 0; myCounterInt < lbxPyramidResolutions->count(); myCounterInt++ )
  {
    QListWidgetItem *myItem = lbxPyramidResolutions->item( myCounterInt );
    //mark to be pyramided
    myPyramidList[myCounterInt].setBuild( myItem->isSelected() || myPyramidList[myCounterInt].getExists() );
  }

  // keep it in sync with qgsrasterpyramidsoptionwidget.cpp
  QString prefix = provider->name() + "/driverOptions/_pyramids/";
  QgsSettings mySettings;
  QString resamplingMethod( cboResamplingMethod->currentData().toString() );
  mySettings.setValue( prefix + "resampling", resamplingMethod );

  //
  // Ask raster layer to build the pyramids
  //

  // let the user know we're going to possibly be taking a while
  QApplication::setOverrideCursor( Qt::WaitCursor );
  QString res = provider->buildPyramids(
    myPyramidList,
    resamplingMethod,
    cbxPyramidsFormat->currentData().value<Qgis::RasterPyramidFormat>(),
    QStringList(),
    feedback.get()
  );
  QApplication::restoreOverrideCursor();
  mPyramidProgress->setValue( 0 );
  buttonBuildPyramids->setEnabled( false );
  if ( !res.isNull() )
  {
    if ( res == "CANCELED"_L1 )
    {
      // user canceled
    }
    else if ( res == "ERROR_WRITE_ACCESS"_L1 )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ), tr( "Write access denied. Adjust the file permissions and try again." ) );
    }
    else if ( res == "ERROR_WRITE_FORMAT"_L1 )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ), tr( "The file was not writable. Some formats do not "
                                                                 "support pyramid overviews. Consult the GDAL documentation if in doubt." ) );
    }
    else if ( res == "FAILED_NOT_SUPPORTED"_L1 )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ), tr( "Building pyramid overviews is not supported on this type of raster." ) );
    }
    else if ( res == "ERROR_JPEG_COMPRESSION"_L1 )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ), tr( "Building internal pyramid overviews is not supported on raster layers with JPEG compression and your current libtiff library." ) );
    }
    else if ( res == "ERROR_VIRTUAL"_L1 )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ), tr( "Building pyramid overviews is not supported on this type of raster." ) );
    }
  }

  //
  // repopulate the pyramids list
  //
  lbxPyramidResolutions->clear();
  // Need to rebuild list as some or all pyramids may have failed to build
  myPyramidList = provider->buildPyramidList();
  QIcon myPyramidPixmap( QgsApplication::getThemeIcon( "/mIconPyramid.svg" ) );
  QIcon myNoPyramidPixmap( QgsApplication::getThemeIcon( "/mIconNoPyramid.svg" ) );

  for ( const QgsRasterPyramid &pyramid : std::as_const( myPyramidList ) )
  {
    if ( pyramid.getExists() )
    {
      lbxPyramidResolutions->addItem( new QListWidgetItem( myPyramidPixmap, QString::number( pyramid.getXDim() ) + u" x "_s + QString::number( pyramid.getYDim() ) ) );
    }
    else
    {
      lbxPyramidResolutions->addItem( new QListWidgetItem( myNoPyramidPixmap, QString::number( pyramid.getXDim() ) + u" x "_s + QString::number( pyramid.getYDim() ) ) );
    }
  }
  //update the legend pixmap
  // pixmapLegend->setPixmap( mRasterLayer->legendAsPixmap() );
  // pixmapLegend->setScaledContents( true );
  // pixmapLegend->repaint();

  //populate the metadata tab's text browser widget with gdal metadata info
  updateInformationContent();
}

void QgsRasterLayerProperties::mRenderTypeComboBox_currentIndexChanged( int index )
{
  if ( index < 0 || mDisableRenderTypeComboBoxCurrentIndexChanged || !mRasterLayer->renderer() )
  {
    return;
  }

  QString rendererName = mRenderTypeComboBox->itemData( index ).toString();
  setRendererWidget( rendererName );
}

void QgsRasterLayerProperties::mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mCanvas, tr( "Select Transformation" ) );
  mRasterLayer->setCrs( crs );
  mMetadataWidget->crsChanged();
}

void QgsRasterLayerProperties::aboutToShowStyleMenu()
{
  // this should be unified with QgsVectorLayerProperties::aboutToShowStyleMenu()

  QMenu *m = qobject_cast<QMenu *>( sender() );

  QgsMapLayerStyleGuiUtils::instance()->removesExtraMenuSeparators( m );
  // re-add style manager actions!
  m->addSeparator();
  QgsMapLayerStyleGuiUtils::instance()->addStyleManagerActions( m, mRasterLayer );
}

void QgsRasterLayerProperties::syncToLayer()
{
  QgsRasterRenderer *renderer = mRasterLayer->renderer();
  if ( renderer )
  {
    setRendererWidget( renderer->type() );
  }
  sync();
  mRasterLayer->triggerRepaint();
}

void QgsRasterLayerProperties::optionsStackedWidget_CurrentChanged( int index )
{
  QgsLayerPropertiesDialog::optionsStackedWidget_CurrentChanged( index );

  if ( !mHistogramWidget )
    return;

  if ( index == mOptStackedWidget->indexOf( mOptsPage_Histogram ) )
  {
    mHistogramWidget->setActive( true );
  }
  else
  {
    mHistogramWidget->setActive( false );
  }

  if ( index == mOptStackedWidget->indexOf( mOptsPage_Information ) || !mMetadataFilled )
  {
    //set the metadata contents (which can be expensive)
    updateInformationContent();
  }
}

void QgsRasterLayerProperties::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsRasterPipe::Property key )
{
  button->blockSignals( true );
  button->init( static_cast<int>( key ), mPropertyCollection, QgsRasterPipe::propertyDefinitions(), nullptr );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsRasterLayerProperties::updateProperty );
  button->registerExpressionContextGenerator( this );
  button->blockSignals( false );
}

void QgsRasterLayerProperties::updateDataDefinedButtons()
{
  const QList<QgsPropertyOverrideButton *> propertyOverrideButtons { findChildren<QgsPropertyOverrideButton *>() };
  for ( QgsPropertyOverrideButton *button : propertyOverrideButtons )
  {
    updateDataDefinedButton( button );
  }
}

void QgsRasterLayerProperties::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  if ( !button )
    return;

  if ( button->propertyKey() < 0 )
    return;

  QgsRasterPipe::Property key = static_cast<QgsRasterPipe::Property>( button->propertyKey() );
  whileBlocking( button )->setToProperty( mPropertyCollection.property( key ) );
}

void QgsRasterLayerProperties::updateProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  QgsRasterPipe::Property key = static_cast<QgsRasterPipe::Property>( button->propertyKey() );
  mPropertyCollection.setProperty( key, button->toProperty() );
}

void QgsRasterLayerProperties::toggleSaturationControls( int grayscaleMode )
{
  // Enable or disable saturation controls based on choice of grayscale mode
  if ( grayscaleMode == 0 )
  {
    sliderSaturation->setEnabled( true );
    spinBoxSaturation->setEnabled( true );
  }
  else
  {
    sliderSaturation->setEnabled( false );
    spinBoxSaturation->setEnabled( false );
  }
}

void QgsRasterLayerProperties::toggleColorizeControls( bool colorizeEnabled )
{
  // Enable or disable colorize controls based on checkbox
  btnColorizeColor->setEnabled( colorizeEnabled );
  sliderColorizeStrength->setEnabled( colorizeEnabled );
  spinColorizeStrength->setEnabled( colorizeEnabled );
}


QLinearGradient QgsRasterLayerProperties::redGradient()
{
  //define a gradient
  // TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient( mGradientWidth, 0, mGradientWidth, mGradientHeight );
  myGradient.setColorAt( 0.0, QColor( 242, 14, 25, 190 ) );
  myGradient.setColorAt( 0.5, QColor( 175, 29, 37, 190 ) );
  myGradient.setColorAt( 1.0, QColor( 114, 17, 22, 190 ) );
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::greenGradient()
{
  //define a gradient
  // TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient( mGradientWidth, 0, mGradientWidth, mGradientHeight );
  myGradient.setColorAt( 0.0, QColor( 48, 168, 5, 190 ) );
  myGradient.setColorAt( 0.8, QColor( 36, 122, 4, 190 ) );
  myGradient.setColorAt( 1.0, QColor( 21, 71, 2, 190 ) );
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::blueGradient()
{
  //define a gradient
  // TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient( mGradientWidth, 0, mGradientWidth, mGradientHeight );
  myGradient.setColorAt( 0.0, QColor( 30, 0, 106, 190 ) );
  myGradient.setColorAt( 0.2, QColor( 30, 72, 128, 190 ) );
  myGradient.setColorAt( 1.0, QColor( 30, 223, 196, 190 ) );
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::grayGradient()
{
  //define a gradient
  // TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient( mGradientWidth, 0, mGradientWidth, mGradientHeight );
  myGradient.setColorAt( 0.0, QColor( 5, 5, 5, 190 ) );
  myGradient.setColorAt( 0.8, QColor( 122, 122, 122, 190 ) );
  myGradient.setColorAt( 1.0, QColor( 220, 220, 220, 190 ) );
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::highlightGradient()
{
  //define another gradient for the highlight
  // TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient( mGradientWidth, 0, mGradientWidth, mGradientHeight );
  myGradient.setColorAt( 1.0, QColor( 255, 255, 255, 50 ) );
  myGradient.setColorAt( 0.5, QColor( 255, 255, 255, 100 ) );
  myGradient.setColorAt( 0.0, QColor( 255, 255, 255, 150 ) );
  return myGradient;
}

//
//
// Next four methods for saving and restoring qml style state
//
//

void QgsRasterLayerProperties::saveDefaultStyle()
{
  saveStyleAsDefault();
}

void QgsRasterLayerProperties::restoreWindowModality()
{
  hide();
  setModal( true );
  show();
  raise();
  activateWindow();
}

void QgsRasterLayerProperties::toggleBuildPyramidsButton()
{
  if ( lbxPyramidResolutions->selectedItems().empty() )
  {
    buttonBuildPyramids->setEnabled( false );
  }
  else
  {
    buttonBuildPyramids->setEnabled( true );
  }
}

void QgsRasterLayerProperties::mResetColorRenderingBtn_clicked()
{
  mBlendModeComboBox->setBlendMode( QPainter::CompositionMode_SourceOver );
  mSliderBrightness->setValue( 0 );
  mSliderContrast->setValue( 0 );
  mGammaSpinBox->setValue( 1.0 );
  sliderSaturation->setValue( 0 );
  comboGrayscale->setCurrentIndex( ( int ) QgsHueSaturationFilter::GrayscaleOff );
  mColorizeCheck->setChecked( false );
  sliderColorizeStrength->setValue( 100 );
  mInvertColorsCheck->setChecked( false );
}

bool QgsRasterLayerProperties::rasterIsMultiBandColor()
{
  return mRasterLayer && nullptr != dynamic_cast<QgsMultiBandColorRenderer *>( mRasterLayer->renderer() );
}

void QgsRasterLayerProperties::updateInformationContent()
{
  const QString myStyle = QgsApplication::reportStyleSheet( QgsApplication::StyleSheetType::WebBrowser );
  // Inject the stylesheet
  const QString html { mRasterLayer->htmlMetadata().replace( "<head>"_L1, QStringLiteral( R"raw(<head><style type="text/css">%1</style>)raw" ) ).arg( myStyle ) };
  mMetadataViewer->setHtml( html );
  mMetadataFilled = true;
}

void QgsRasterLayerProperties::rollback()
{
  // Give the user a chance to save the raster attribute table edits.
  if ( mRasterAttributeTableWidget && mRasterAttributeTableWidget->isDirty() )
  {
    mRasterAttributeTableWidget->setEditable( false, false );
  }
  QgsLayerPropertiesDialog::rollback();

  if ( mBackupCrs != mRasterLayer->crs() )
    mRasterLayer->setCrs( mBackupCrs );
}

void QgsRasterLayerProperties::showHelp()
{
  const QVariant helpPage = mOptionsStackedWidget->currentWidget()->property( "helpPage" );

  if ( helpPage.isValid() )
  {
    QgsHelp::openHelp( helpPage.toString() );
  }
  else
  {
    QgsHelp::openHelp( u"working_with_raster/raster_properties.html"_s );
  }
}

void QgsRasterLayerProperties::updateGammaSpinBox( int value )
{
  whileBlocking( mGammaSpinBox )->setValue( value / 100.0 );
}

void QgsRasterLayerProperties::updateGammaSlider( double value )
{
  whileBlocking( mSliderGamma )->setValue( value * 100 );
}


bool QgsRasterLayerProperties::eventFilter( QObject *obj, QEvent *ev )
{
  // If the map tip preview container is resized, resize the map tip
  if ( obj == mMapTipPreviewContainer && ev->type() == QEvent::Resize )
  {
    resizeMapTip();
  }
  return QgsOptionsDialogBase::eventFilter( obj, ev );
}

void QgsRasterLayerProperties::initMapTipPreview()
{
  // HTML editor and preview are in a splitter. By default, the editor takes 2/3 of the space
  mMapTipSplitter->setSizes( { 400, 200 } );
  // Event filter is used to resize the map tip when the container is resized
  mMapTipPreviewContainer->installEventFilter( this );

  // Note: there's quite a bit of overlap between this and the code in QgsMapTip::showMapTip
  // Create the WebView
  mMapTipPreview = new QgsWebView( mMapTipPreviewContainer );
  mMapTipPreviewLayout->addWidget( mMapTipPreview );

  mMapTipPreview->page()->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
  mMapTipPreview->page()->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );
  mMapTipPreview->page()->settings()->setAttribute( QWebSettings::LocalStorageEnabled, true );

  // Disable scrollbars, avoid random resizing issues
  mMapTipPreview->page()->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
  mMapTipPreview->page()->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );


  // Update the map tip preview when the expression or the map tip template changes
  connect( mMapTipWidget, &QgsCodeEditorHTML::textChanged, this, &QgsRasterLayerProperties::updateMapTipPreview );
}

void QgsRasterLayerProperties::updateMapTipPreview()
{
  mMapTipPreview->setMaximumSize( mMapTipPreviewContainer->width(), mMapTipPreviewContainer->height() );
  const QString htmlContent = QgsMapTip::rasterMapTipPreviewText( mRasterLayer, mCanvas, mMapTipWidget->text() );
  mMapTipPreview->setHtml( htmlContent );
}

void QgsRasterLayerProperties::resizeMapTip()
{
  // Ensure the map tip is not bigger than the container
  mMapTipPreview->setMaximumSize( mMapTipPreviewContainer->width(), mMapTipPreviewContainer->height() );
  mMapTipPreview->adjustSize();
}
