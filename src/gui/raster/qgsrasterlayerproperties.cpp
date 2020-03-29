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

#include <limits>
#include <typeinfo>

#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsbilinearrasterresampler.h"
#include "qgsbrightnesscontrastfilter.h"
#include "qgscontrastenhancement.h"
#include "qgscoordinatetransform.h"
#include "qgscubicrasterresampler.h"
#include "qgsprojectionselectiondialog.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsmaptoolemitpoint.h"
#include "qgsmaptopixel.h"
#include "qgsmetadatawidget.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsmultibandcolorrendererwidget.h"
#include "qgsnative.h"
#include "qgspalettedrendererwidget.h"
#include "qgsproject.h"
#include "qgsrasterbandstats.h"
#include "qgsrastercontourrendererwidget.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterhistogramwidget.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsrasterpyramid.h"
#include "qgsrasterrange.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrasterresamplefilter.h"
#include "qgsrastertransparency.h"
#include "qgssinglebandgrayrendererwidget.h"
#include "qgssinglebandpseudocolorrendererwidget.h"
#include "qgshuesaturationfilter.h"
#include "qgshillshaderendererwidget.h"
#include "qgssettings.h"
#include "qgsdatumtransformdialog.h"
#include "qgsmaplayerlegend.h"
#include "qgsfileutils.h"
#include "qgswebview.h"

#include "qgsrasterlayertemporalpropertieswidget.h"

#include <QDesktopServices>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QLinearGradient>
#include <QPainterPath>
#include <QPolygonF>
#include <QColorDialog>
#include <QList>
#include <QMouseEvent>
#include <QVector>
#include <QUrl>
#include <QMenu>
#include <QScreen>

QgsRasterLayerProperties::QgsRasterLayerProperties( QgsMapLayer *lyr, QgsMapCanvas *canvas, QWidget *parent, Qt::WindowFlags fl )
  : QgsOptionsDialogBase( QStringLiteral( "RasterLayerProperties" ), parent, fl )
    // Constant that signals property not used.
  , TRSTRING_NOT_SET( tr( "Not Set" ) )
  , mDefaultStandardDeviation( 0 )
  , mDefaultRedBand( 0 )
  , mDefaultGreenBand( 0 )
  , mDefaultBlueBand( 0 )
  , mRasterLayer( qobject_cast<QgsRasterLayer *>( lyr ) )
  , mGradientHeight( 0.0 )
  , mGradientWidth( 0.0 )
  , mMapCanvas( canvas )
  , mMetadataFilled( false )
{
  mGrayMinimumMaximumEstimated = true;
  mRGBMinimumMaximumEstimated = true;

  setupUi( this );
  connect( mLayerOrigNameLineEd, &QLineEdit::textEdited, this, &QgsRasterLayerProperties::mLayerOrigNameLineEd_textEdited );
  connect( buttonBuildPyramids, &QPushButton::clicked, this, &QgsRasterLayerProperties::buttonBuildPyramids_clicked );
  connect( pbnAddValuesFromDisplay, &QToolButton::clicked, this, &QgsRasterLayerProperties::pbnAddValuesFromDisplay_clicked );
  connect( pbnAddValuesManually, &QToolButton::clicked, this, &QgsRasterLayerProperties::pbnAddValuesManually_clicked );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsRasterLayerProperties::mCrsSelector_crsChanged );
  connect( pbnDefaultValues, &QToolButton::clicked, this, &QgsRasterLayerProperties::pbnDefaultValues_clicked );
  connect( pbnExportTransparentPixelValues, &QToolButton::clicked, this, &QgsRasterLayerProperties::pbnExportTransparentPixelValues_clicked );
  connect( pbnImportTransparentPixelValues, &QToolButton::clicked, this, &QgsRasterLayerProperties::pbnImportTransparentPixelValues_clicked );
  connect( pbnRemoveSelectedRow, &QToolButton::clicked, this, &QgsRasterLayerProperties::pbnRemoveSelectedRow_clicked );
  connect( mRenderTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterLayerProperties::mRenderTypeComboBox_currentIndexChanged );
  connect( mResetColorRenderingBtn, &QToolButton::clicked, this, &QgsRasterLayerProperties::mResetColorRenderingBtn_clicked );
  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsRasterLayerProperties::showHelp );

  mBtnStyle = new QPushButton( tr( "Style" ) );
  QMenu *menuStyle = new QMenu( this );
  menuStyle->addAction( tr( "Load Style…" ), this, &QgsRasterLayerProperties::loadStyle_clicked );
  menuStyle->addAction( tr( "Save Style…" ), this, &QgsRasterLayerProperties::saveStyleAs_clicked );
  menuStyle->addSeparator();
  menuStyle->addAction( tr( "Save as Default" ), this, &QgsRasterLayerProperties::saveDefaultStyle_clicked );
  menuStyle->addAction( tr( "Restore Default" ), this, &QgsRasterLayerProperties::loadDefaultStyle_clicked );
  mBtnStyle->setMenu( menuStyle );
  connect( menuStyle, &QMenu::aboutToShow, this, &QgsRasterLayerProperties::aboutToShowStyleMenu );
  buttonBox->addButton( mBtnStyle, QDialogButtonBox::ResetRole );

  mBtnMetadata = new QPushButton( tr( "Metadata" ), this );
  QMenu *menuMetadata = new QMenu( this );
  mActionLoadMetadata = menuMetadata->addAction( tr( "Load Metadata…" ), this, &QgsRasterLayerProperties::loadMetadata );
  mActionSaveMetadataAs = menuMetadata->addAction( tr( "Save Metadata…" ), this, &QgsRasterLayerProperties::saveMetadataAs );
  menuMetadata->addSeparator();
  menuMetadata->addAction( tr( "Save as Default" ), this, &QgsRasterLayerProperties::saveDefaultMetadata );
  menuMetadata->addAction( tr( "Restore Default" ), this, &QgsRasterLayerProperties::loadDefaultMetadata );
  mBtnMetadata->setMenu( menuMetadata );
  buttonBox->addButton( mBtnMetadata, QDialogButtonBox::ResetRole );

  connect( lyr->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsRasterLayerProperties::syncToLayer );

  connect( this, &QDialog::accepted, this, &QgsRasterLayerProperties::apply );
  connect( this, &QDialog::rejected, this, &QgsRasterLayerProperties::onCancel );

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsRasterLayerProperties::apply );

  // brightness/contrast controls
  connect( mSliderBrightness, &QAbstractSlider::valueChanged, mBrightnessSpinBox, &QSpinBox::setValue );
  connect( mBrightnessSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), mSliderBrightness, &QAbstractSlider::setValue );

  connect( mSliderContrast, &QAbstractSlider::valueChanged, mContrastSpinBox, &QSpinBox::setValue );
  connect( mContrastSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), mSliderContrast, &QAbstractSlider::setValue );

  // Connect saturation slider and spin box
  connect( sliderSaturation, &QAbstractSlider::valueChanged, spinBoxSaturation, &QSpinBox::setValue );
  connect( spinBoxSaturation, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), sliderSaturation, &QAbstractSlider::setValue );

  // Connect colorize strength slider and spin box
  connect( sliderColorizeStrength, &QAbstractSlider::valueChanged, spinColorizeStrength, &QSpinBox::setValue );
  connect( spinColorizeStrength, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), sliderColorizeStrength, &QAbstractSlider::setValue );

  // enable or disable saturation slider and spin box depending on grayscale combo choice
  connect( comboGrayscale, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRasterLayerProperties::toggleSaturationControls );

  // enable or disable colorize colorbutton with colorize checkbox
  connect( mColorizeCheck, &QAbstractButton::toggled, this, &QgsRasterLayerProperties::toggleColorizeControls );

  // enable or disable Build Pyramids button depending on selection in pyramid list
  connect( lbxPyramidResolutions, &QListWidget::itemSelectionChanged, this, &QgsRasterLayerProperties::toggleBuildPyramidsButton );

  connect( mRefreshLayerCheckBox, &QCheckBox::toggled, mRefreshLayerIntervalSpinBox, &QDoubleSpinBox::setEnabled );

  // set up the scale based layer visibility stuff....
  mScaleRangeWidget->setMapCanvas( mMapCanvas );
  chkUseScaleDependentRendering->setChecked( lyr->hasScaleBasedVisibility() );
  mScaleRangeWidget->setScaleRange( lyr->minimumScale(), lyr->maximumScale() );

  leNoDataValue->setValidator( new QDoubleValidator( -std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 1000, this ) );

  // build GUI components
  QIcon myPyramidPixmap( QgsApplication::getThemeIcon( "/mIconPyramid.svg" ) );
  QIcon myNoPyramidPixmap( QgsApplication::getThemeIcon( "/mIconNoPyramid.svg" ) );

  pbnAddValuesManually->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyAdd.svg" ) ) );
  pbnAddValuesFromDisplay->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionContextHelp.png" ) ) );
  pbnRemoveSelectedRow->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyRemove.svg" ) ) );
  pbnDefaultValues->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ) );
  pbnImportTransparentPixelValues->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileOpen.svg" ) ) );
  pbnExportTransparentPixelValues->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileSave.svg" ) ) );

  if ( mMapCanvas )
  {
    mPixelSelectorTool = qgis::make_unique<QgsMapToolEmitPoint>( canvas );
    connect( mPixelSelectorTool.get(), &QgsMapToolEmitPoint::canvasClicked, this, &QgsRasterLayerProperties::pixelSelected );
    connect( mPixelSelectorTool.get(), &QgsMapToolEmitPoint::deactivated, this, &QgsRasterLayerProperties::restoreWindowModality );
  }
  else
  {
    pbnAddValuesFromDisplay->setEnabled( false );
  }

  if ( !mRasterLayer )
  {
    return;
  }

  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();

  // Only do pyramids if dealing directly with GDAL.
  if ( provider && provider->capabilities() & QgsRasterDataProvider::BuildPyramids )
  {
    // initialize resampling methods
    cboResamplingMethod->clear();

    const auto constProviderType = QgsRasterDataProvider::pyramidResamplingMethods( mRasterLayer->providerType() );
    for ( QPair<QString, QString> method : constProviderType )
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
    QList< QgsRasterPyramid > myPyramidList = provider->buildPyramidList();
    QList< QgsRasterPyramid >::iterator myRasterPyramidIterator;

    for ( myRasterPyramidIterator = myPyramidList.begin();
          myRasterPyramidIterator != myPyramidList.end();
          ++myRasterPyramidIterator )
    {
      if ( myRasterPyramidIterator->exists )
      {
        lbxPyramidResolutions->addItem( new QListWidgetItem( myPyramidPixmap,
                                        QString::number( myRasterPyramidIterator->xDim ) + QStringLiteral( " x " ) +
                                        QString::number( myRasterPyramidIterator->yDim ) ) );
      }
      else
      {
        lbxPyramidResolutions->addItem( new QListWidgetItem( myNoPyramidPixmap,
                                        QString::number( myRasterPyramidIterator->xDim ) + QStringLiteral( " x " ) +
                                        QString::number( myRasterPyramidIterator->yDim ) ) );
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
  if ( !provider || !( provider->capabilities() & QgsRasterDataProvider::Size ) )
  {
    // disable Histogram tab completely
    mOptsPage_Histogram->setEnabled( false );
  }

  QVBoxLayout *layout = new QVBoxLayout( metadataFrame );
  layout->setMargin( 0 );
  mMetadataWidget = new QgsMetadataWidget( this, mRasterLayer );
  mMetadataWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
  mMetadataWidget->setMapCanvas( mMapCanvas );
  layout->addWidget( mMetadataWidget );
  metadataFrame->setLayout( layout );

  QVBoxLayout *temporalLayout = new QVBoxLayout( temporalFrame );
  temporalLayout->setContentsMargins( 0, 0, 0, 0 );
  mTemporalWidget = new QgsRasterLayerTemporalPropertiesWidget( this, mRasterLayer );
  temporalLayout->addWidget( mTemporalWidget );

  QgsDebugMsg( "Setting crs to " + mRasterLayer->crs().toWkt( QgsCoordinateReferenceSystem::WKT2_2018 ) );
  QgsDebugMsg( "Setting crs to " + mRasterLayer->crs().userFriendlyIdentifier() );
  mCrsSelector->setCrs( mRasterLayer->crs() );

  // Set text for pyramid info box
  QString pyramidFormat( QStringLiteral( "<h2>%1</h2><p>%2 %3 %4</p><b><font color='red'><p>%5</p><p>%6</p>" ) );
  QString pyramidHeader    = tr( "Description" );
  QString pyramidSentence1 = tr( "Large resolution raster layers can slow navigation in QGIS." );
  QString pyramidSentence2 = tr( "By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom." );
  QString pyramidSentence3 = tr( "You must have write access in the directory where the original data is stored to build pyramids." );
  QString pyramidSentence4 = tr( "Please note that building internal pyramids may alter the original data file and once created they cannot be removed!" );
  QString pyramidSentence5 = tr( "Please note that building internal pyramids could corrupt your image - always make a backup of your data first!" );

  tePyramidDescription->setHtml( pyramidFormat.arg( pyramidHeader,
                                 pyramidSentence1,
                                 pyramidSentence2,
                                 pyramidSentence3,
                                 pyramidSentence4,
                                 pyramidSentence5 ) );

  //resampling
  mResamplingGroupBox->setSaveCheckedState( true );
  const QgsRasterRenderer *renderer = mRasterLayer->renderer();
  mZoomedInResamplingComboBox->insertItem( 0, tr( "Nearest neighbour" ) );
  mZoomedInResamplingComboBox->insertItem( 1, tr( "Bilinear" ) );
  mZoomedInResamplingComboBox->insertItem( 2, tr( "Cubic" ) );
  mZoomedOutResamplingComboBox->insertItem( 0, tr( "Nearest neighbour" ) );
  mZoomedOutResamplingComboBox->insertItem( 1, tr( "Average" ) );

  const QgsRasterResampleFilter *resampleFilter = mRasterLayer->resampleFilter();
  //set combo boxes to current resampling types
  if ( resampleFilter )
  {
    const QgsRasterResampler *zoomedInResampler = resampleFilter->zoomedInResampler();
    if ( zoomedInResampler )
    {
      if ( zoomedInResampler->type() == QLatin1String( "bilinear" ) )
      {
        mZoomedInResamplingComboBox->setCurrentIndex( 1 );
      }
      else if ( zoomedInResampler->type() == QLatin1String( "cubic" ) )
      {
        mZoomedInResamplingComboBox->setCurrentIndex( 2 );
      }
    }
    else
    {
      mZoomedInResamplingComboBox->setCurrentIndex( 0 );
    }

    const QgsRasterResampler *zoomedOutResampler = resampleFilter->zoomedOutResampler();
    if ( zoomedOutResampler )
    {
      if ( zoomedOutResampler->type() == QLatin1String( "bilinear" ) ) //bilinear resampler does averaging when zooming out
      {
        mZoomedOutResamplingComboBox->setCurrentIndex( 1 );
      }
    }
    else
    {
      mZoomedOutResamplingComboBox->setCurrentIndex( 0 );
    }
    mMaximumOversamplingSpinBox->setValue( resampleFilter->maxOversampling() );
  }

  btnColorizeColor->setColorDialogTitle( tr( "Select Color" ) );
  btnColorizeColor->setContext( QStringLiteral( "symbology" ) );

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
  }

  //blend mode
  mBlendModeComboBox->setBlendMode( mRasterLayer->blendMode() );

  //transparency band
  if ( provider )
  {
    cboxTransparencyBand->setShowNotSetOption( true, tr( "None" ) );
    cboxTransparencyBand->setLayer( mRasterLayer );

// Alpha band is set in sync()
#if 0
    if ( renderer )
    {
      QgsDebugMsg( QStringLiteral( "alphaBand = %1" ).arg( renderer->alphaBand() ) );
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
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "paletted" ), QgsPalettedRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "multibandcolor" ), QgsMultiBandColorRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "singlebandpseudocolor" ), QgsSingleBandPseudoColorRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "singlebandgray" ), QgsSingleBandGrayRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "hillshade" ), QgsHillshadeRendererWidget::create );
  QgsApplication::rasterRendererRegistry()->insertWidgetFunction( QStringLiteral( "contour" ), QgsRasterContourRendererWidget::create );

  //fill available renderers into combo box
  QgsRasterRendererRegistryEntry entry;
  mDisableRenderTypeComboBoxCurrentIndexChanged = true;
  const auto constRenderersList = QgsApplication::rasterRendererRegistry()->renderersList();
  for ( const QString &name : constRenderersList )
  {
    if ( QgsApplication::rasterRendererRegistry()->rendererData( name, entry ) )
    {
      if ( ( mRasterLayer->rasterType() != QgsRasterLayer::ColorLayer && entry.name != QLatin1String( "singlebandcolordata" ) ) ||
           ( mRasterLayer->rasterType() == QgsRasterLayer::ColorLayer && entry.name == QLatin1String( "singlebandcolordata" ) ) )
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

    if ( rendererType == QLatin1String( "singlebandcolordata" ) && mRenderTypeComboBox->count() == 1 )
    {
      // no band rendering options for singlebandcolordata, so minimize group box
      QSizePolicy sizep = mBandRenderingGrpBx->sizePolicy();
      sizep.setVerticalStretch( 0 );
      sizep.setVerticalPolicy( QSizePolicy::Maximum );
      mBandRenderingGrpBx->setSizePolicy( sizep );
      mBandRenderingGrpBx->updateGeometry();
    }

    if ( mRasterLayer->providerType() != QStringLiteral( "wms" ) )
    {
      mWMSPrintGroupBox->hide();
      mPublishDataSourceUrlCheckBox->hide();
      mBackgroundLayerCheckBox->hide();
    }
  }

#ifdef WITH_QTWEBKIT
  // Setup information tab

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  const int horizontalDpi = qApp->desktop()->screen()->logicalDpiX();
#else
  const int horizontalDpi = logicalDpiX();
#endif

  // Adjust zoom: text is ok, but HTML seems rather big at least on Linux/KDE
  if ( horizontalDpi > 96 )
  {
    mMetadataViewer->setZoomFactor( mMetadataViewer->zoomFactor() * 0.9 );
  }
  mMetadataViewer->page()->setLinkDelegationPolicy( QWebPage::LinkDelegationPolicy::DelegateAllLinks );
  connect( mMetadataViewer->page(), &QWebPage::linkClicked, this, &QgsRasterLayerProperties::urlClicked );
  mMetadataViewer->page()->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
  mMetadataViewer->page()->settings()->setAttribute( QWebSettings::JavascriptEnabled, true );

#endif

  mRenderTypeComboBox_currentIndexChanged( widgetIndex );

  // update based on lyr's current state
  sync();

  QgsSettings settings;
  // if dialog hasn't been opened/closed yet, default to Styles tab, which is used most often
  // this will be read by restoreOptionsBaseUi()
  if ( !settings.contains( QStringLiteral( "/Windows/RasterLayerProperties/tab" ) ) )
  {
    settings.setValue( QStringLiteral( "Windows/RasterLayerProperties/tab" ),
                       mOptStackedWidget->indexOf( mOptsPage_Style ) );
  }

  mResetColorRenderingBtn->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUndo.svg" ) ) );

  QString title = QString( tr( "Layer Properties - %1" ) ).arg( lyr->name() );

  if ( !mRasterLayer->styleManager()->isDefault( mRasterLayer->styleManager()->currentStyle() ) )
    title += QStringLiteral( " (%1)" ).arg( mRasterLayer->styleManager()->currentStyle() );
  restoreOptionsBaseUi( title );
  optionsStackedWidget_CurrentChanged( mOptionsStackedWidget->currentIndex() );

  //Add help page references
  mOptsPage_Information->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#information-properties" ) );
  mOptsPage_Source->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#source-properties" ) );
  mOptsPage_Style->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#symbology-properties" ) );
  mOptsPage_Transparency->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#transparency-properties" ) );

  if ( mOptsPage_Histogram )
    mOptsPage_Histogram->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#histogram-properties" ) );

  mOptsPage_Rendering->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#rendering-properties" ) );

  if ( mOptsPage_Pyramids )
    mOptsPage_Pyramids->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#pyramids-properties" ) );

  mOptsPage_Metadata->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#metadata-properties" ) );
  mOptsPage_Legend->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#legend-properties" ) );
  mOptsPage_Server->setProperty( "helpPage", QStringLiteral( "working_with_raster/raster_properties.html#server-properties" ) );
}

void QgsRasterLayerProperties::setCurrentPage( const QString &page )
{
  //find the page with a matching widget name
  for ( int idx = 0; idx < mOptionsStackedWidget->count(); ++idx )
  {
    QWidget *currentPage = mOptionsStackedWidget->widget( idx );
    if ( currentPage->objectName() == page )
    {
      //found the page, set it as current
      mOptionsStackedWidget->setCurrentIndex( idx );
      return;
    }
  }
}

void QgsRasterLayerProperties::setupTransparencyTable( int nBands )
{
  tableTransparency->clear();
  tableTransparency->setColumnCount( 0 );
  tableTransparency->setRowCount( 0 );
  mTransparencyToEdited.clear();

  if ( nBands == 3 )
  {
    tableTransparency->setColumnCount( 4 );
    tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Red" ) ) );
    tableTransparency->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Green" ) ) );
    tableTransparency->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Blue" ) ) );
    tableTransparency->setHorizontalHeaderItem( 3, new QTableWidgetItem( tr( "Percent Transparent" ) ) );
  }
  else //1 band
  {
    tableTransparency->setColumnCount( 3 );
// Is it important to distinguish the header? It becomes difficult with range.
#if 0
    if ( QgsRasterLayer::PalettedColor != mRasterLayer->drawingStyle() &&
         QgsRasterLayer::PalettedSingleBandGray != mRasterLayer->drawingStyle() &&
         QgsRasterLayer::PalettedSingleBandPseudoColor != mRasterLayer->drawingStyle() &&
         QgsRasterLayer::PalettedMultiBandColor != mRasterLayer->drawingStyle() )
    {
      tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Gray" ) ) );
    }
    else
    {
      tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Indexed Value" ) ) );
    }
#endif
    tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "From" ) ) );
    tableTransparency->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "To" ) ) );
    tableTransparency->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Percent Transparent" ) ) );
  }

  tableTransparency->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
  tableTransparency->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
}

void QgsRasterLayerProperties::populateTransparencyTable( QgsRasterRenderer *renderer )
{
  if ( !mRasterLayer )
  {
    return;
  }

  if ( !renderer )
  {
    return;
  }

  int nBands = renderer->usesBands().size();
  setupTransparencyTable( nBands );

  const QgsRasterTransparency *rasterTransparency = renderer->rasterTransparency();
  if ( !rasterTransparency )
  {
    return;
  }

  if ( nBands == 1 )
  {
    QList<QgsRasterTransparency::TransparentSingleValuePixel> pixelList = rasterTransparency->transparentSingleValuePixelList();
    for ( int i = 0; i < pixelList.size(); ++i )
    {
      tableTransparency->insertRow( i );
      setTransparencyCell( i, 0, pixelList[i].min );
      setTransparencyCell( i, 1, pixelList[i].max );
      setTransparencyCell( i, 2, pixelList[i].percentTransparent );
      // break synchronization only if values differ
      if ( pixelList[i].min != pixelList[i].max )
      {
        setTransparencyToEdited( i );
      }
    }
  }
  else if ( nBands == 3 )
  {
    QList<QgsRasterTransparency::TransparentThreeValuePixel> pixelList = rasterTransparency->transparentThreeValuePixelList();
    for ( int i = 0; i < pixelList.size(); ++i )
    {
      tableTransparency->insertRow( i );
      setTransparencyCell( i, 0, pixelList[i].red );
      setTransparencyCell( i, 1, pixelList[i].green );
      setTransparencyCell( i, 2, pixelList[i].blue );
      setTransparencyCell( i, 3, pixelList[i].percentTransparent );
    }
  }

  tableTransparency->resizeColumnsToContents();
  tableTransparency->resizeRowsToContents();
}

void QgsRasterLayerProperties::setRendererWidget( const QString &rendererName )
{
  QgsDebugMsg( "rendererName = " + rendererName );
  QgsRasterRendererWidget *oldWidget = mRendererWidget;
  QgsRasterRenderer *oldRenderer = mRasterLayer->renderer();

  int alphaBand = -1;
  double opacity = 1;
  QColor nodataColor;
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
      QgsDebugMsg( QStringLiteral( "renderer has widgetCreateFunction" ) );
      // Current canvas extent (used to calc min/max) in layer CRS
      QgsRectangle myExtent = mMapCanvas->mapSettings().outputExtentToLayerExtent( mRasterLayer, mMapCanvas->extent() );
      if ( oldWidget && ( !oldRenderer || rendererName != oldRenderer->type() ) )
      {
        if ( rendererName == QLatin1String( "singlebandgray" ) )
        {
          whileBlocking( mRasterLayer )->setRenderer( QgsApplication::rasterRendererRegistry()->defaultRendererForDrawingStyle( QgsRaster::SingleBandGray, mRasterLayer->dataProvider() ) );
          whileBlocking( mRasterLayer )->setDefaultContrastEnhancement();
        }
        else if ( rendererName == QLatin1String( "multibandcolor" ) )
        {
          whileBlocking( mRasterLayer )->setRenderer( QgsApplication::rasterRendererRegistry()->defaultRendererForDrawingStyle( QgsRaster::MultiBandColor, mRasterLayer->dataProvider() ) );
          whileBlocking( mRasterLayer )->setDefaultContrastEnhancement();
        }
      }
      mRasterLayer->renderer()->setAlphaBand( alphaBand );
      mRasterLayer->renderer()->setOpacity( opacity );
      mRasterLayer->renderer()->setNodataColor( nodataColor );
      mRendererWidget = rendererEntry.widgetCreateFunction( mRasterLayer, myExtent );
      mRendererWidget->setMapCanvas( mMapCanvas );
      mRendererStackedWidget->addWidget( mRendererWidget );
      if ( oldWidget )
      {
        //compare used bands in new and old renderer and reset transparency dialog if different
        QgsRasterRenderer *oldRenderer = oldWidget->renderer();
        QgsRasterRenderer *newRenderer = mRendererWidget->renderer();
        QList<int> oldBands = oldRenderer->usesBands();
        QList<int> newBands = newRenderer->usesBands();
        if ( oldBands != newBands )
        {
          populateTransparencyTable( newRenderer );
        }
        delete oldRenderer;
        delete newRenderer;
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

/**
  \note moved from ctor

  Previously this dialog was created anew with each right-click pop-up menu
  invocation.  Changed so that the dialog always exists after first
  invocation, and is just re-synchronized with its layer's state when
  re-shown.

*/
void QgsRasterLayerProperties::sync()
{
  QgsSettings myQSettings;

  const QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
    return;

  if ( provider->dataType( 1 ) == Qgis::ARGB32
       || provider->dataType( 1 ) == Qgis::ARGB32_Premultiplied )
  {
    gboxNoDataValue->setEnabled( false );
    gboxCustomTransparency->setEnabled( false );
    mOptionsStackedWidget->setCurrentWidget( mOptsPage_Server );
  }

  // TODO: Wouldn't it be better to just removeWidget() the tabs than delete them? [LS]
  if ( !( provider->capabilities() & QgsRasterDataProvider::BuildPyramids ) )
  {
    if ( mOptsPage_Pyramids )
    {
      delete mOptsPage_Pyramids;
      mOptsPage_Pyramids = nullptr;
    }
  }

  if ( !( provider->capabilities() & QgsRasterDataProvider::Size ) )
  {
    if ( mOptsPage_Histogram )
    {
      delete mOptsPage_Histogram;
      mOptsPage_Histogram = nullptr;
      delete mHistogramWidget;
      mHistogramWidget = nullptr;
    }
  }

  QgsDebugMsg( QStringLiteral( "populate transparency tab" ) );

  /*
   * Style tab (brightness and contrast)
   */

  QgsBrightnessContrastFilter *brightnessFilter = mRasterLayer->brightnessFilter();
  if ( brightnessFilter )
  {
    mSliderBrightness->setValue( brightnessFilter->brightness() );
    mSliderContrast->setValue( brightnessFilter->contrast() );
  }

  //set the transparency slider

  /*
   * Transparent Pixel Tab
   */

  //set the transparency slider
  QgsRasterRenderer *renderer = mRasterLayer->renderer();
  if ( renderer )
  {
    mOpacityWidget->setOpacity( renderer->opacity() );
    cboxTransparencyBand->setBand( renderer->alphaBand() );
  }

  //add current NoDataValue to NoDataValue line edit
  // TODO: should be per band
  // TODO: no data ranges
  if ( provider->sourceHasNoDataValue( 1 ) )
  {
    lblSrcNoDataValue->setText( QgsRasterBlock::printValue( provider->sourceNoDataValue( 1 ) ) );
  }
  else
  {
    lblSrcNoDataValue->setText( tr( "not defined" ) );
  }

  mSrcNoDataValueCheckBox->setChecked( provider->useSourceNoDataValue( 1 ) );

  bool enableSrcNoData = provider->sourceHasNoDataValue( 1 ) && !std::isnan( provider->sourceNoDataValue( 1 ) );

  mSrcNoDataValueCheckBox->setEnabled( enableSrcNoData );
  lblSrcNoDataValue->setEnabled( enableSrcNoData );

  QgsRasterRangeList noDataRangeList = provider->userNoDataValues( 1 );
  QgsDebugMsg( QStringLiteral( "noDataRangeList.size = %1" ).arg( noDataRangeList.size() ) );
  if ( !noDataRangeList.isEmpty() )
  {
    leNoDataValue->insert( QgsRasterBlock::printValue( noDataRangeList.value( 0 ).min() ) );
  }
  else
  {
    leNoDataValue->insert( QString() );
  }

  mRefreshLayerCheckBox->setChecked( mRasterLayer->hasAutoRefreshEnabled() );
  mRefreshLayerIntervalSpinBox->setEnabled( mRasterLayer->hasAutoRefreshEnabled() );
  mRefreshLayerIntervalSpinBox->setValue( mRasterLayer->autoRefreshInterval() / 1000.0 );

  populateTransparencyTable( mRasterLayer->renderer() );

  QgsDebugMsg( QStringLiteral( "populate colormap tab" ) );
  /*
   * Transparent Pixel Tab
   */

  QgsDebugMsg( QStringLiteral( "populate general tab" ) );
  /*
   * General Tab
   */

  //these properties (layer name and label) are provided by the qgsmaplayer superclass
  mLayerOrigNameLineEd->setText( mRasterLayer->name() );
  leDisplayName->setText( mRasterLayer->name() );

  //get the thumbnail for the layer
  QPixmap thumbnail = QPixmap::fromImage( mRasterLayer->previewAsImage( pixmapThumbnail->size() ) );
  pixmapThumbnail->setPixmap( thumbnail );

  // TODO fix legend + palette pixmap

  //update the legend pixmap on this dialog
#if 0
  pixmapLegend->setPixmap( mRasterLayer->legendAsPixmap() );
  pixmapLegend->setScaledContents( true );
  pixmapLegend->repaint();

  //set the palette pixmap
  pixmapPalette->setPixmap( mRasterLayer->paletteAsPixmap( mRasterLayer->bandNumber( mRasterLayer->grayBandName() ) ) );
  pixmapPalette->setScaledContents( true );
  pixmapPalette->repaint();
#endif

  QgsDebugMsg( QStringLiteral( "populate metadata tab" ) );
  /*
   * Metadata Tab
   */
  //populate the metadata tab's text browser widget with gdal metadata info
  updateInformationContent();

  // WMS Name as layer short name
  mLayerShortNameLineEdit->setText( mRasterLayer->shortName() );
  // WMS Name validator
  QValidator *shortNameValidator = new QRegExpValidator( QgsApplication::shortNameRegExp(), this );
  mLayerShortNameLineEdit->setValidator( shortNameValidator );

  //layer title and abstract
  mLayerTitleLineEdit->setText( mRasterLayer->title() );
  mLayerAbstractTextEdit->setPlainText( mRasterLayer->abstract() );
  mLayerKeywordListLineEdit->setText( mRasterLayer->keywordList() );
  mLayerDataUrlLineEdit->setText( mRasterLayer->dataUrl() );
  mLayerDataUrlFormatComboBox->setCurrentIndex(
    mLayerDataUrlFormatComboBox->findText(
      mRasterLayer->dataUrlFormat()
    )
  );

  //layer attribution and metadataUrl
  mLayerAttributionLineEdit->setText( mRasterLayer->attribution() );
  mLayerAttributionUrlLineEdit->setText( mRasterLayer->attributionUrl() );
  mLayerMetadataUrlLineEdit->setText( mRasterLayer->metadataUrl() );
  mLayerMetadataUrlTypeComboBox->setCurrentIndex(
    mLayerMetadataUrlTypeComboBox->findText(
      mRasterLayer->metadataUrlType()
    )
  );
  mLayerMetadataUrlFormatComboBox->setCurrentIndex(
    mLayerMetadataUrlFormatComboBox->findText(
      mRasterLayer->metadataUrlFormat()
    )
  );

  mLayerLegendUrlLineEdit->setText( mRasterLayer->legendUrl() );
  mLayerLegendUrlFormatComboBox->setCurrentIndex( mLayerLegendUrlFormatComboBox->findText( mRasterLayer->legendUrlFormat() ) );

  //WMS print layer
  QVariant wmsPrintLayer = mRasterLayer->customProperty( QStringLiteral( "WMSPrintLayer" ) );
  if ( wmsPrintLayer.isValid() )
  {
    mWMSPrintLayerLineEdit->setText( wmsPrintLayer.toString() );
  }

  QVariant wmsPublishDataSourceUrl = mRasterLayer->customProperty( QStringLiteral( "WMSPublishDataSourceUrl" ), false );
  mPublishDataSourceUrlCheckBox->setChecked( wmsPublishDataSourceUrl.toBool() );

  QVariant wmsBackgroundLayer = mRasterLayer->customProperty( QStringLiteral( "WMSBackgroundLayer" ), false );
  mBackgroundLayerCheckBox->setChecked( wmsBackgroundLayer.toBool() );

  /*
   * Legend Tab
   */
  mLegendConfigEmbeddedWidget->setLayer( mRasterLayer );

} // QgsRasterLayerProperties::sync()

/*
 *
 * PUBLIC AND PRIVATE SLOTS
 *
 */
void QgsRasterLayerProperties::apply()
{

  // Do nothing on "bad" layers
  if ( !mRasterLayer->isValid() )
    return;

  /*
   * Legend Tab
   */
  mLegendConfigEmbeddedWidget->applyToLayer();

  QgsDebugMsg( QStringLiteral( "apply processing symbology tab" ) );
  /*
   * Symbology Tab
   */

  //set whether the layer histogram should be inverted
  //mRasterLayer->setInvertHistogram( cboxInvertColorMap->isChecked() );

  mRasterLayer->brightnessFilter()->setBrightness( mSliderBrightness->value() );
  mRasterLayer->brightnessFilter()->setContrast( mSliderContrast->value() );

  QgsDebugMsg( QStringLiteral( "processing transparency tab" ) );
  /*
   * Transparent Pixel Tab
   */

  //set NoDataValue
  QgsRasterRangeList myNoDataRangeList;
  if ( "" != leNoDataValue->text() )
  {
    bool myDoubleOk = false;
    double myNoDataValue = leNoDataValue->text().toDouble( &myDoubleOk );
    if ( myDoubleOk )
    {
      QgsRasterRange myNoDataRange( myNoDataValue, myNoDataValue );
      myNoDataRangeList << myNoDataRange;
    }
  }
  for ( int bandNo = 1; bandNo <= mRasterLayer->dataProvider()->bandCount(); bandNo++ )
  {
    mRasterLayer->dataProvider()->setUserNoDataValue( bandNo, myNoDataRangeList );
    mRasterLayer->dataProvider()->setUseSourceNoDataValue( bandNo, mSrcNoDataValueCheckBox->isChecked() );
  }

  //set renderer from widget
  QgsRasterRendererWidget *rendererWidget = dynamic_cast<QgsRasterRendererWidget *>( mRendererStackedWidget->currentWidget() );
  if ( rendererWidget )
  {
    rendererWidget->doComputations();

    mRasterLayer->setRenderer( rendererWidget->renderer() );
  }

  mMetadataWidget->acceptMetadata();
  mMetadataFilled = false;

  //transparency settings
  QgsRasterRenderer *rasterRenderer = mRasterLayer->renderer();
  if ( rasterRenderer )
  {
    rasterRenderer->setAlphaBand( cboxTransparencyBand->currentBand() );

    //Walk through each row in table and test value. If not valid set to 0.0 and continue building transparency list
    QgsRasterTransparency *rasterTransparency = new QgsRasterTransparency();
    if ( tableTransparency->columnCount() == 4 )
    {
      QgsRasterTransparency::TransparentThreeValuePixel myTransparentPixel;
      QList<QgsRasterTransparency::TransparentThreeValuePixel> myTransparentThreeValuePixelList;
      for ( int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++ )
      {
        myTransparentPixel.red = transparencyCellValue( myListRunner, 0 );
        myTransparentPixel.green = transparencyCellValue( myListRunner, 1 );
        myTransparentPixel.blue = transparencyCellValue( myListRunner, 2 );
        myTransparentPixel.percentTransparent = transparencyCellValue( myListRunner, 3 );
        myTransparentThreeValuePixelList.append( myTransparentPixel );
      }
      rasterTransparency->setTransparentThreeValuePixelList( myTransparentThreeValuePixelList );
    }
    else if ( tableTransparency->columnCount() == 3 )
    {
      QgsRasterTransparency::TransparentSingleValuePixel myTransparentPixel;
      QList<QgsRasterTransparency::TransparentSingleValuePixel> myTransparentSingleValuePixelList;
      for ( int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++ )
      {
        myTransparentPixel.min = transparencyCellValue( myListRunner, 0 );
        myTransparentPixel.max = transparencyCellValue( myListRunner, 1 );
        myTransparentPixel.percentTransparent = transparencyCellValue( myListRunner, 2 );

        myTransparentSingleValuePixelList.append( myTransparentPixel );
      }
      rasterTransparency->setTransparentSingleValuePixelList( myTransparentSingleValuePixelList );
    }

    rasterRenderer->setRasterTransparency( rasterTransparency );

    //set global transparency
    rasterRenderer->setOpacity( mOpacityWidget->opacity() );
  }

  QgsDebugMsg( QStringLiteral( "processing general tab" ) );
  /*
   * General Tab
   */
  mRasterLayer->setName( mLayerOrigNameLineEd->text() );

  // set up the scale based layer visibility stuff....
  mRasterLayer->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mRasterLayer->setMinimumScale( mScaleRangeWidget->minimumScale() );
  mRasterLayer->setMaximumScale( mScaleRangeWidget->maximumScale() );

  mRasterLayer->setAutoRefreshInterval( mRefreshLayerIntervalSpinBox->value() * 1000.0 );
  mRasterLayer->setAutoRefreshEnabled( mRefreshLayerCheckBox->isChecked() );

  //update the legend pixmap
  // pixmapLegend->setPixmap( mRasterLayer->legendAsPixmap() );
  // pixmapLegend->setScaledContents( true );
  // pixmapLegend->repaint();

  QgsRasterResampleFilter *resampleFilter = mRasterLayer->resampleFilter();
  if ( resampleFilter )
  {
    QgsRasterResampler *zoomedInResampler = nullptr;
    QString zoomedInResamplingMethod = mZoomedInResamplingComboBox->currentText();
    if ( zoomedInResamplingMethod == tr( "Bilinear" ) )
    {
      zoomedInResampler = new QgsBilinearRasterResampler();
    }
    else if ( zoomedInResamplingMethod == tr( "Cubic" ) )
    {
      zoomedInResampler = new QgsCubicRasterResampler();
    }

    resampleFilter->setZoomedInResampler( zoomedInResampler );

    //raster resampling
    QgsRasterResampler *zoomedOutResampler = nullptr;
    QString zoomedOutResamplingMethod = mZoomedOutResamplingComboBox->currentText();
    if ( zoomedOutResamplingMethod == tr( "Average" ) )
    {
      zoomedOutResampler = new QgsBilinearRasterResampler();
    }

    resampleFilter->setZoomedOutResampler( zoomedOutResampler );

    resampleFilter->setMaxOversampling( mMaximumOversamplingSpinBox->value() );
  }

  // Hue and saturation controls
  QgsHueSaturationFilter *hueSaturationFilter = mRasterLayer->hueSaturationFilter();
  if ( hueSaturationFilter )
  {
    hueSaturationFilter->setSaturation( sliderSaturation->value() );
    hueSaturationFilter->setGrayscaleMode( ( QgsHueSaturationFilter::GrayscaleMode ) comboGrayscale->currentIndex() );
    hueSaturationFilter->setColorizeOn( mColorizeCheck->checkState() );
    hueSaturationFilter->setColorizeColor( btnColorizeColor->color() );
    hueSaturationFilter->setColorizeStrength( sliderColorizeStrength->value() );
  }

  //set the blend mode for the layer
  mRasterLayer->setBlendMode( mBlendModeComboBox->blendMode() );

  // Update temporal properties
  mTemporalWidget->saveTemporalProperties();

  //get the thumbnail for the layer
  QPixmap thumbnail = QPixmap::fromImage( mRasterLayer->previewAsImage( pixmapThumbnail->size() ) );
  pixmapThumbnail->setPixmap( thumbnail );

  if ( mRasterLayer->shortName() != mLayerShortNameLineEdit->text() )
    mMetadataFilled = false;
  mRasterLayer->setShortName( mLayerShortNameLineEdit->text() );

  if ( mRasterLayer->title() != mLayerTitleLineEdit->text() )
    mMetadataFilled = false;
  mRasterLayer->setTitle( mLayerTitleLineEdit->text() );

  if ( mRasterLayer->abstract() != mLayerAbstractTextEdit->toPlainText() )
    mMetadataFilled = false;
  mRasterLayer->setAbstract( mLayerAbstractTextEdit->toPlainText() );

  if ( mRasterLayer->keywordList() != mLayerKeywordListLineEdit->text() )
    mMetadataFilled = false;
  mRasterLayer->setKeywordList( mLayerKeywordListLineEdit->text() );

  if ( mRasterLayer->dataUrl() != mLayerDataUrlLineEdit->text() )
    mMetadataFilled = false;
  mRasterLayer->setDataUrl( mLayerDataUrlLineEdit->text() );

  if ( mRasterLayer->dataUrlFormat() != mLayerDataUrlFormatComboBox->currentText() )
    mMetadataFilled = false;
  mRasterLayer->setDataUrlFormat( mLayerDataUrlFormatComboBox->currentText() );

  //layer attribution and metadataUrl
  if ( mRasterLayer->attribution() != mLayerAttributionLineEdit->text() )
    mMetadataFilled = false;
  mRasterLayer->setAttribution( mLayerAttributionLineEdit->text() );

  if ( mRasterLayer->attributionUrl() != mLayerAttributionUrlLineEdit->text() )
    mMetadataFilled = false;
  mRasterLayer->setAttributionUrl( mLayerAttributionUrlLineEdit->text() );

  if ( mRasterLayer->metadataUrl() != mLayerMetadataUrlLineEdit->text() )
    mMetadataFilled = false;
  mRasterLayer->setMetadataUrl( mLayerMetadataUrlLineEdit->text() );

  if ( mRasterLayer->metadataUrlType() != mLayerMetadataUrlTypeComboBox->currentText() )
    mMetadataFilled = false;
  mRasterLayer->setMetadataUrlType( mLayerMetadataUrlTypeComboBox->currentText() );

  if ( mRasterLayer->metadataUrlFormat() != mLayerMetadataUrlFormatComboBox->currentText() )
    mMetadataFilled = false;
  mRasterLayer->setMetadataUrlFormat( mLayerMetadataUrlFormatComboBox->currentText() );

  if ( mRasterLayer->legendUrl() != mLayerLegendUrlLineEdit->text() )
    mMetadataFilled = false;
  mRasterLayer->setLegendUrl( mLayerLegendUrlLineEdit->text() );

  if ( mRasterLayer->legendUrlFormat() != mLayerLegendUrlFormatComboBox->currentText() )
    mMetadataFilled = false;
  mRasterLayer->setLegendUrlFormat( mLayerLegendUrlFormatComboBox->currentText() );

  if ( !mWMSPrintLayerLineEdit->text().isEmpty() )
  {
    mRasterLayer->setCustomProperty( QStringLiteral( "WMSPrintLayer" ), mWMSPrintLayerLineEdit->text() );
  }

  mRasterLayer->setCustomProperty( "WMSPublishDataSourceUrl", mPublishDataSourceUrlCheckBox->isChecked() );
  mRasterLayer->setCustomProperty( "WMSBackgroundLayer", mBackgroundLayerCheckBox->isChecked() );

  // Force a redraw of the legend
  mRasterLayer->setLegend( QgsMapLayerLegend::defaultRasterLegend( mRasterLayer ) );

  //make sure the layer is redrawn
  mRasterLayer->triggerRepaint();

  // notify the project we've made a change
  QgsProject::instance()->setDirty( true );
}//apply

void QgsRasterLayerProperties::mLayerOrigNameLineEd_textEdited( const QString &text )
{
  leDisplayName->setText( mRasterLayer->formatLayerName( text ) );
}

void QgsRasterLayerProperties::buttonBuildPyramids_clicked()
{
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();

  std::unique_ptr< QgsRasterBlockFeedback > feedback( new QgsRasterBlockFeedback() );

  connect( feedback.get(), &QgsRasterBlockFeedback::progressChanged, mPyramidProgress, &QProgressBar::setValue );
  //
  // Go through the list marking any files that are selected in the listview
  // as true so that we can generate pyramids for them.
  //
  QList< QgsRasterPyramid> myPyramidList = provider->buildPyramidList();
  for ( int myCounterInt = 0; myCounterInt < lbxPyramidResolutions->count(); myCounterInt++ )
  {
    QListWidgetItem *myItem = lbxPyramidResolutions->item( myCounterInt );
    //mark to be pyramided
    myPyramidList[myCounterInt].build = myItem->isSelected() || myPyramidList[myCounterInt].exists;
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
                  ( QgsRaster::RasterPyramidsFormat ) cbxPyramidsFormat->currentIndex(),
                  QStringList(),
                  feedback.get() );
  QApplication::restoreOverrideCursor();
  mPyramidProgress->setValue( 0 );
  buttonBuildPyramids->setEnabled( false );
  if ( !res.isNull() )
  {
    if ( res == QLatin1String( "CANCELED" ) )
    {
      // user canceled
    }
    else if ( res == QLatin1String( "ERROR_WRITE_ACCESS" ) )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ),
                            tr( "Write access denied. Adjust the file permissions and try again." ) );
    }
    else if ( res == QLatin1String( "ERROR_WRITE_FORMAT" ) )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ),
                            tr( "The file was not writable. Some formats do not "
                                "support pyramid overviews. Consult the GDAL documentation if in doubt." ) );
    }
    else if ( res == QLatin1String( "FAILED_NOT_SUPPORTED" ) )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ),
                            tr( "Building pyramid overviews is not supported on this type of raster." ) );
    }
    else if ( res == QLatin1String( "ERROR_JPEG_COMPRESSION" ) )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ),
                            tr( "Building internal pyramid overviews is not supported on raster layers with JPEG compression and your current libtiff library." ) );
    }
    else if ( res == QLatin1String( "ERROR_VIRTUAL" ) )
    {
      QMessageBox::warning( this, tr( "Building Pyramids" ),
                            tr( "Building pyramid overviews is not supported on this type of raster." ) );
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

  QList< QgsRasterPyramid >::iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator = myPyramidList.begin();
        myRasterPyramidIterator != myPyramidList.end();
        ++myRasterPyramidIterator )
  {
    if ( myRasterPyramidIterator->exists )
    {
      lbxPyramidResolutions->addItem( new QListWidgetItem( myPyramidPixmap,
                                      QString::number( myRasterPyramidIterator->xDim ) + QStringLiteral( " x " ) +
                                      QString::number( myRasterPyramidIterator->yDim ) ) );
    }
    else
    {
      lbxPyramidResolutions->addItem( new QListWidgetItem( myNoPyramidPixmap,
                                      QString::number( myRasterPyramidIterator->xDim ) + QStringLiteral( " x " ) +
                                      QString::number( myRasterPyramidIterator->yDim ) ) );
    }
  }
  //update the legend pixmap
  // pixmapLegend->setPixmap( mRasterLayer->legendAsPixmap() );
  // pixmapLegend->setScaledContents( true );
  // pixmapLegend->repaint();

  //populate the metadata tab's text browser widget with gdal metadata info
  updateInformationContent();
}

void QgsRasterLayerProperties::urlClicked( const QUrl &url )
{
  QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::instance()->nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}

void QgsRasterLayerProperties::mRenderTypeComboBox_currentIndexChanged( int index )
{
  if ( index < 0 || mDisableRenderTypeComboBoxCurrentIndexChanged || ! mRasterLayer->renderer() )
  {
    return;
  }

  QString rendererName = mRenderTypeComboBox->itemData( index ).toString();
  setRendererWidget( rendererName );
}

void QgsRasterLayerProperties::pbnAddValuesFromDisplay_clicked()
{
  if ( mMapCanvas && mPixelSelectorTool )
  {
    //Need to work around the modality of the dialog but can not just hide() it.
    // According to Qt5 docs, to change modality the dialog needs to be hidden
    // and shown again.
    hide();
    setModal( false );

    // Transfer focus to the canvas to use the selector tool
    mMapCanvas->window()->raise();
    mMapCanvas->window()->activateWindow();
    mMapCanvas->window()->setFocus();
    mMapCanvas->setMapTool( mPixelSelectorTool.get() );

  }
}

void QgsRasterLayerProperties::pbnAddValuesManually_clicked()
{
  QgsRasterRenderer *renderer = mRendererWidget->renderer();
  if ( !renderer )
  {
    return;
  }

  tableTransparency->insertRow( tableTransparency->rowCount() );

  int n = renderer->usesBands().size();
  if ( n == 1 ) n++;

  for ( int i = 0; i < n; i++ )
  {
    setTransparencyCell( tableTransparency->rowCount() - 1, i, std::numeric_limits<double>::quiet_NaN() );
  }

  setTransparencyCell( tableTransparency->rowCount() - 1, n, 100 );

  tableTransparency->resizeColumnsToContents();
  tableTransparency->resizeRowsToContents();
}

void QgsRasterLayerProperties::mCrsSelector_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  QgsDatumTransformDialog::run( crs, QgsProject::instance()->crs(), this, mMapCanvas, tr( "Select Transformation" ) );
  mRasterLayer->setCrs( crs );
  mMetadataWidget->crsChanged();
}

void QgsRasterLayerProperties::pbnDefaultValues_clicked()
{
  if ( !mRendererWidget )
  {
    return;
  }

  QgsRasterRenderer *r = mRendererWidget->renderer();
  if ( !r )
  {
    return;
  }

  int nBands = r->usesBands().size();
  delete r; // really delete?

  setupTransparencyTable( nBands );

  tableTransparency->resizeColumnsToContents(); // works only with values
  tableTransparency->resizeRowsToContents();
}

void QgsRasterLayerProperties::setTransparencyCell( int row, int column, double value )
{
  QgsDebugMsg( QStringLiteral( "value = %1" ).arg( value, 0, 'g', 17 ) );
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider ) return;

  QgsRasterRenderer *renderer = mRendererWidget->renderer();
  if ( !renderer ) return;
  int nBands = renderer->usesBands().size();

  QLineEdit *lineEdit = new QLineEdit();
  lineEdit->setFrame( false ); // frame looks bad in table
  // Without margins row selection is not displayed (important for delete row)
  lineEdit->setContentsMargins( 1, 1, 1, 1 );

  if ( column == tableTransparency->columnCount() - 1 )
  {
    // transparency
    // Who needs transparency as floating point?
    lineEdit->setValidator( new QIntValidator( nullptr ) );
    lineEdit->setText( QString::number( static_cast<int>( value ) ) );
  }
  else
  {
    // value
    QString valueString;
    switch ( provider->sourceDataType( 1 ) )
    {
      case Qgis::Float32:
      case Qgis::Float64:
        lineEdit->setValidator( new QDoubleValidator( nullptr ) );
        if ( !std::isnan( value ) )
        {
          valueString = QgsRasterBlock::printValue( value );
        }
        break;
      default:
        lineEdit->setValidator( new QIntValidator( nullptr ) );
        if ( !std::isnan( value ) )
        {
          valueString = QString::number( static_cast<int>( value ) );
        }
        break;
    }
    lineEdit->setText( valueString );
  }
  tableTransparency->setCellWidget( row, column, lineEdit );
  adjustTransparencyCellWidth( row, column );

  if ( nBands == 1 && ( column == 0 || column == 1 ) )
  {
    connect( lineEdit, &QLineEdit::textEdited, this, &QgsRasterLayerProperties::transparencyCellTextEdited );
  }
  tableTransparency->resizeColumnsToContents();
}

void QgsRasterLayerProperties::setTransparencyCellValue( int row, int column, double value )
{
  QLineEdit *lineEdit = dynamic_cast<QLineEdit *>( tableTransparency->cellWidget( row, column ) );
  if ( !lineEdit ) return;
  lineEdit->setText( QgsRasterBlock::printValue( value ) );
  lineEdit->adjustSize();
  adjustTransparencyCellWidth( row, column );
  tableTransparency->resizeColumnsToContents();
}

double QgsRasterLayerProperties::transparencyCellValue( int row, int column )
{
  QLineEdit *lineEdit = dynamic_cast<QLineEdit *>( tableTransparency->cellWidget( row, column ) );
  if ( !lineEdit || lineEdit->text().isEmpty() )
  {
    std::numeric_limits<double>::quiet_NaN();
  }
  return lineEdit->text().toDouble();
}

void QgsRasterLayerProperties::adjustTransparencyCellWidth( int row, int column )
{
  QLineEdit *lineEdit = dynamic_cast<QLineEdit *>( tableTransparency->cellWidget( row, column ) );
  if ( !lineEdit ) return;

  int width = std::max( lineEdit->fontMetrics().boundingRect( lineEdit->text() ).width() + 10, 100 );
  width = std::max( width, tableTransparency->columnWidth( column ) );

  lineEdit->setFixedWidth( width );
}

void QgsRasterLayerProperties::pbnExportTransparentPixelValues_clicked()
{
  QgsSettings myQSettings;
  QString myLastDir = myQSettings.value( QStringLiteral( "lastRasterFileFilterDir" ), QDir::homePath() ).toString();
  QString myFileName = QFileDialog::getSaveFileName( this, tr( "Save File" ), myLastDir, tr( "Textfile" ) + " (*.txt)" );
  if ( !myFileName.isEmpty() )
  {
    if ( !myFileName.endsWith( QLatin1String( ".txt" ), Qt::CaseInsensitive ) )
    {
      myFileName = myFileName + ".txt";
    }

    QFile myOutputFile( myFileName );
    if ( myOutputFile.open( QFile::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream myOutputStream( &myOutputFile );
      myOutputStream << "# " << tr( "QGIS Generated Transparent Pixel Value Export File" ) << '\n';
      if ( rasterIsMultiBandColor() )
      {
        myOutputStream << "#\n#\n# " << tr( "Red" ) << "\t" << tr( "Green" ) << "\t" << tr( "Blue" ) << "\t" << tr( "Percent Transparent" );
        for ( int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++ )
        {
          myOutputStream << '\n' << QString::number( transparencyCellValue( myTableRunner, 0 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 1 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 2 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 3 ) );
        }
      }
      else
      {
        myOutputStream << "#\n#\n# " << tr( "Value" ) << "\t" << tr( "Percent Transparent" );

        for ( int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++ )
        {
          myOutputStream << '\n' << QString::number( transparencyCellValue( myTableRunner, 0 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 1 ) ) << "\t"
                         << QString::number( transparencyCellValue( myTableRunner, 2 ) );
        }
      }
    }
    else
    {
      QMessageBox::warning( this, tr( "Export Transparent Pixels" ), tr( "Write access denied. Adjust the file permissions and try again.\n\n" ) );
    }
  }
}

void QgsRasterLayerProperties::transparencyCellTextEdited( const QString &text )
{
  Q_UNUSED( text )
  QgsDebugMsg( QStringLiteral( "text = %1" ).arg( text ) );
  QgsRasterRenderer *renderer = mRendererWidget->renderer();
  if ( !renderer )
  {
    return;
  }
  int nBands = renderer->usesBands().size();
  if ( nBands == 1 )
  {
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>( sender() );
    if ( !lineEdit ) return;
    int row = -1;
    int column = -1;
    for ( int r = 0; r < tableTransparency->rowCount(); r++ )
    {
      for ( int c = 0; c < tableTransparency->columnCount(); c++ )
      {
        if ( tableTransparency->cellWidget( r, c ) == sender() )
        {
          row = r;
          column = c;
          break;
        }
      }
      if ( row != -1 ) break;
    }
    QgsDebugMsg( QStringLiteral( "row = %1 column =%2" ).arg( row ).arg( column ) );

    if ( column == 0 )
    {
      QLineEdit *toLineEdit = dynamic_cast<QLineEdit *>( tableTransparency->cellWidget( row, 1 ) );
      if ( !toLineEdit ) return;
      bool toChanged = mTransparencyToEdited.value( row );
      QgsDebugMsg( QStringLiteral( "toChanged = %1" ).arg( toChanged ) );
      if ( !toChanged )
      {
        toLineEdit->setText( lineEdit->text() );
      }
    }
    else if ( column == 1 )
    {
      setTransparencyToEdited( row );
    }
  }
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

void QgsRasterLayerProperties::setTransparencyToEdited( int row )
{
  if ( row >= mTransparencyToEdited.size() )
  {
    mTransparencyToEdited.resize( row + 1 );
  }
  mTransparencyToEdited[row] = true;
}

void QgsRasterLayerProperties::optionsStackedWidget_CurrentChanged( int index )
{
  QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( index );

  bool isMetadataPanel = ( index == mOptStackedWidget->indexOf( mOptsPage_Metadata ) );
  mBtnStyle->setVisible( ! isMetadataPanel );
  mBtnMetadata->setVisible( isMetadataPanel );

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

void QgsRasterLayerProperties::pbnImportTransparentPixelValues_clicked()
{
  int myLineCounter = 0;
  bool myImportError = false;
  QString myBadLines;
  QgsSettings myQSettings;
  QString myLastDir = myQSettings.value( QStringLiteral( "lastRasterFileFilterDir" ), QDir::homePath() ).toString();
  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Open file" ), myLastDir, tr( "Textfile" ) + " (*.txt)" );
  QFile myInputFile( myFileName );
  if ( myInputFile.open( QFile::ReadOnly ) )
  {
    QTextStream myInputStream( &myInputFile );
    QString myInputLine;
    if ( rasterIsMultiBandColor() )
    {
      for ( int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >= 0; myTableRunner-- )
      {
        tableTransparency->removeRow( myTableRunner );
      }

      while ( !myInputStream.atEnd() )
      {
        myLineCounter++;
        myInputLine = myInputStream.readLine();
        if ( !myInputLine.isEmpty() )
        {
          if ( !myInputLine.simplified().startsWith( '#' ) )
          {
            QStringList myTokens = myInputLine.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
            if ( myTokens.count() != 4 )
            {
              myImportError = true;
              myBadLines = myBadLines + QString::number( myLineCounter ) + ":\t[" + myInputLine + "]\n";
            }
            else
            {
              tableTransparency->insertRow( tableTransparency->rowCount() );
              for ( int col = 0; col < 4; col++ )
              {
                setTransparencyCell( tableTransparency->rowCount() - 1, col, myTokens[col].toDouble() );
              }
            }
          }
        }
      }
    }
    else
    {
      for ( int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >= 0; myTableRunner-- )
      {
        tableTransparency->removeRow( myTableRunner );
      }

      while ( !myInputStream.atEnd() )
      {
        myLineCounter++;
        myInputLine = myInputStream.readLine();
        if ( !myInputLine.isEmpty() )
        {
          if ( !myInputLine.simplified().startsWith( '#' ) )
          {
            QStringList myTokens = myInputLine.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
            if ( myTokens.count() != 3 && myTokens.count() != 2 ) // 2 for QGIS < 1.9 compatibility
            {
              myImportError = true;
              myBadLines = myBadLines + QString::number( myLineCounter ) + ":\t[" + myInputLine + "]\n";
            }
            else
            {
              if ( myTokens.count() == 2 )
              {
                myTokens.insert( 1, myTokens[0] ); // add 'to' value, QGIS < 1.9 compatibility
              }
              tableTransparency->insertRow( tableTransparency->rowCount() );
              for ( int col = 0; col < 3; col++ )
              {
                setTransparencyCell( tableTransparency->rowCount() - 1, col, myTokens[col].toDouble() );
              }
            }
          }
        }
      }
    }

    if ( myImportError )
    {
      QMessageBox::warning( this, tr( "Import Transparent Pixels" ), tr( "The following lines contained errors\n\n%1" ).arg( myBadLines ) );
    }
  }
  else if ( !myFileName.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Import Transparent Pixels" ), tr( "Read access denied. Adjust the file permissions and try again.\n\n" ) );
  }
  tableTransparency->resizeColumnsToContents();
  tableTransparency->resizeRowsToContents();
}

void QgsRasterLayerProperties::pbnRemoveSelectedRow_clicked()
{
  if ( 0 < tableTransparency->rowCount() )
  {
    tableTransparency->removeRow( tableTransparency->currentRow() );
  }
}

void QgsRasterLayerProperties::pixelSelected( const QgsPointXY &canvasPoint, const Qt::MouseButton &btn )
{
  Q_UNUSED( btn )
  QgsRasterRenderer *renderer = mRendererWidget->renderer();
  if ( !renderer )
  {
    return;
  }

  //Get the pixel values and add a new entry to the transparency table
  if ( mMapCanvas && mPixelSelectorTool )
  {
    mMapCanvas->unsetMapTool( mPixelSelectorTool.get() );

    const QgsMapSettings &ms = mMapCanvas->mapSettings();
    QgsPointXY myPoint = ms.mapToLayerCoordinates( mRasterLayer, canvasPoint );

    QgsRectangle myExtent = ms.mapToLayerCoordinates( mRasterLayer, mMapCanvas->extent() );
    double mapUnitsPerPixel = mMapCanvas->mapUnitsPerPixel();
    int myWidth = mMapCanvas->extent().width() / mapUnitsPerPixel;
    int myHeight = mMapCanvas->extent().height() / mapUnitsPerPixel;

    QMap<int, QVariant> myPixelMap = mRasterLayer->dataProvider()->identify( myPoint, QgsRaster::IdentifyFormatValue, myExtent, myWidth, myHeight ).results();

    QList<int> bands = renderer->usesBands();

    QList<double> values;
    for ( int i = 0; i < bands.size(); ++i )
    {
      int bandNo = bands.value( i );
      if ( myPixelMap.count( bandNo ) == 1 )
      {
        if ( myPixelMap.value( bandNo ).isNull() )
        {
          return; // Don't add nodata, transparent anyway
        }
        double value = myPixelMap.value( bandNo ).toDouble();
        QgsDebugMsg( QStringLiteral( "value = %1" ).arg( value, 0, 'g', 17 ) );
        values.append( value );
      }
    }
    if ( bands.size() == 1 )
    {
      // Set 'to'
      values.insert( 1, values.value( 0 ) );
    }
    tableTransparency->insertRow( tableTransparency->rowCount() );
    for ( int i = 0; i < values.size(); i++ )
    {
      setTransparencyCell( tableTransparency->rowCount() - 1, i, values.value( i ) );
    }
    setTransparencyCell( tableTransparency->rowCount() - 1, tableTransparency->columnCount() - 1, 100 );
  }
  delete renderer;

  tableTransparency->resizeColumnsToContents();
  tableTransparency->resizeRowsToContents();
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
void QgsRasterLayerProperties::loadDefaultStyle_clicked()
{
  bool defaultLoadedFlag = false;
  QString myMessage = mRasterLayer->loadDefaultStyle( defaultLoadedFlag );
  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    syncToLayer();
  }
  else
  {
    //otherwise let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              myMessage
                            );
  }
}

void QgsRasterLayerProperties::saveDefaultStyle_clicked()
{

  apply(); // make sure the style to save is up-to-date

  // a flag passed by reference
  bool defaultSavedFlag = false;
  // after calling this the above flag will be set true for success
  // or false if the save operation failed
  QString myMessage = mRasterLayer->saveDefaultStyle( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    //let the user know what went wrong
    QMessageBox::information( this,
                              tr( "Default Style" ),
                              myMessage
                            );
  }
}


void QgsRasterLayerProperties::loadStyle_clicked()
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

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

  mOldStyle = mRasterLayer->styleManager()->style( mRasterLayer->styleManager()->currentStyle() );

  bool defaultLoadedFlag = false;
  QString message = mRasterLayer->loadNamedStyle( fileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( fileName ).absolutePath() );
    syncToLayer();
  }
  else
  {
    QMessageBox::information( this, tr( "Save Style" ), message );
  }
}


void QgsRasterLayerProperties::saveStyleAs_clicked()
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString outputFileName = QFileDialog::getSaveFileName(
                             this,
                             tr( "Save layer properties as style file" ),
                             lastUsedDir,
                             tr( "QGIS Layer Style File" ) + " (*.qml)" + ";;" + tr( "Styled Layer Descriptor" ) + " (*.sld)" );
  if ( outputFileName.isEmpty() )
    return;

  // set style type depending on extension
  StyleType type = StyleType::QML;
  if ( outputFileName.endsWith( QLatin1String( ".sld" ), Qt::CaseInsensitive ) )
    type = StyleType::SLD;
  else
    // ensure the user never omits the extension from the file name
    outputFileName = QgsFileUtils::ensureFileNameHasExtension( outputFileName, QStringList() << QStringLiteral( "qml" ) );

  apply(); // make sure the style to save is up-to-date

  // then export style
  bool defaultLoadedFlag = false;
  QString message;
  switch ( type )
  {
    case QML:
    {
      message = mRasterLayer->saveNamedStyle( outputFileName, defaultLoadedFlag );
      break;
    }
    case SLD:
    {
      message = mRasterLayer->saveSldStyle( outputFileName, defaultLoadedFlag );
      break;
    }
  }
  if ( defaultLoadedFlag )
  {
    settings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( outputFileName ).absolutePath() );
    sync();
  }
  else
    QMessageBox::information( this, tr( "Save Style" ), message );
}

void QgsRasterLayerProperties::restoreWindowModality()
{
  hide();
  setModal( true );
  show();
  raise();
  activateWindow();
}

//
//
// Next four methods for saving and restoring QMD metadata
//
//

void QgsRasterLayerProperties::loadMetadata()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Load layer metadata from metadata file" ), myLastUsedDir,
                       tr( "QGIS Layer Metadata File" ) + " (*.qmd)" );
  if ( myFileName.isNull() )
  {
    return;
  }

  QString myMessage;
  bool defaultLoadedFlag = false;
  myMessage = mRasterLayer->loadNamedMetadata( myFileName, defaultLoadedFlag );

  //reset if the default style was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mRasterLayer->metadata() );
  }
  else
  {
    //let the user know what went wrong
    QMessageBox::warning( this, tr( "Load Metadata" ), myMessage );
  }

  QFileInfo myFI( myFileName );
  QString myPath = myFI.path();
  myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), myPath );

  activateWindow(); // set focus back to properties dialog
}

void QgsRasterLayerProperties::saveMetadataAs()
{
  QgsSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( QStringLiteral( "style/lastStyleDir" ), QDir::homePath() ).toString();

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
  QString message = mRasterLayer->saveNamedMetadata( myOutputFileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
    myQSettings.setValue( QStringLiteral( "style/lastStyleDir" ), QFileInfo( myOutputFileName ).absolutePath() );
  else
    QMessageBox::information( this, tr( "Save Metadata" ), message );
}

void QgsRasterLayerProperties::saveDefaultMetadata()
{
  mMetadataWidget->acceptMetadata();

  bool defaultSavedFlag = false;
  QString errorMsg = mRasterLayer->saveDefaultMetadata( defaultSavedFlag );
  if ( !defaultSavedFlag )
  {
    QMessageBox::warning( this, tr( "Default Metadata" ), errorMsg );
  }
}

void QgsRasterLayerProperties::loadDefaultMetadata()
{
  bool defaultLoadedFlag = false;
  QString myMessage = mRasterLayer->loadNamedMetadata( mRasterLayer->metadataUri(), defaultLoadedFlag );
  //reset if the default metadata was loaded OK only
  if ( defaultLoadedFlag )
  {
    mMetadataWidget->setMetadata( &mRasterLayer->metadata() );
  }
  else
  {
    QMessageBox::information( this, tr( "Default Metadata" ), myMessage );
  }
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
  sliderSaturation->setValue( 0 );
  comboGrayscale->setCurrentIndex( ( int ) QgsHueSaturationFilter::GrayscaleOff );
  mColorizeCheck->setChecked( false );
  sliderColorizeStrength->setValue( 100 );
}

bool QgsRasterLayerProperties::rasterIsMultiBandColor()
{
  return mRasterLayer && nullptr != dynamic_cast<QgsMultiBandColorRenderer *>( mRasterLayer->renderer() );
}

void QgsRasterLayerProperties::updateInformationContent()
{
  const QString myStyle = QgsApplication::reportStyleSheet( QgsApplication::StyleSheetType::WebBrowser );
  // Inject the stylesheet
  const QString html { mRasterLayer->htmlMetadata().replace( QStringLiteral( "<head>" ), QStringLiteral( R"raw(<head><style type="text/css">%1</style>)raw" ) ).arg( myStyle ) };
  mMetadataViewer->setHtml( html );
  mMetadataFilled = true;
}

void QgsRasterLayerProperties::onCancel()
{
  if ( mOldStyle.xmlData() != mRasterLayer->styleManager()->style( mRasterLayer->styleManager()->currentStyle() ).xmlData() )
  {
    // need to reset style to previous - style applied directly to the layer (not in apply())
    QString myMessage;
    QDomDocument doc( QStringLiteral( "qgis" ) );
    int errorLine, errorColumn;
    doc.setContent( mOldStyle.xmlData(), false, &myMessage, &errorLine, &errorColumn );
    mRasterLayer->importNamedStyle( doc, myMessage );
    syncToLayer();
  }
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
    QgsHelp::openHelp( QStringLiteral( "working_with_raster/raster_properties.html" ) );
  }
}
