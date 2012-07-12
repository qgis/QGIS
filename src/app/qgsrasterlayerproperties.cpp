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

#include "qgsmaptopixel.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"
#include "qgscoordinatetransform.h"
#include "qgsrasterlayerproperties.h"
#include "qgsrasterrenderer.h"
#include "qgsgenericprojectionselector.h"
#include "qgsproject.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpyramid.h"
#include "qgscontexthelp.h"
#include "qgsmaplayerregistry.h"
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include "qgsmaptoolemitpoint.h"

#include "qgsrasterrendererregistry.h"
#include "qgsmultibandcolorrendererwidget.h"
#include "qgspalettedrendererwidget.h"
#include "qgssinglebandgrayrendererwidget.h"
#include "qgssinglebandpseudocolorrendererwidget.h"

#include <QTableWidgetItem>
#include <QHeaderView>

#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QLinearGradient>
#include <QPainterPath>
#include <QPolygonF>
#include <QColorDialog>
#include <QList>
#include <QSettings>
#include <QMouseEvent>
#include <QVector>

// QWT Charting widget
#include <qwt_global.h>
#include <qwt_plot_canvas.h>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_zoomer.h>

QgsRasterLayerProperties::QgsRasterLayerProperties( QgsMapLayer* lyr, QgsMapCanvas* theCanvas, QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
    // Constant that signals property not used.
    TRSTRING_NOT_SET( tr( "Not Set" ) ),
    mRasterLayer( qobject_cast<QgsRasterLayer *>( lyr ) ), mRendererWidget( 0 )
{
  mGrayMinimumMaximumEstimated = true;
  mRGBMinimumMaximumEstimated = true;

  setupUi( this );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( apply() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

  connect( sliderTransparency, SIGNAL( valueChanged( int ) ), this, SLOT( sliderTransparency_valueChanged( int ) ) );

  // enable or disable Build Pyramids button depending on selection in pyramid list
  connect( lbxPyramidResolutions, SIGNAL( itemSelectionChanged() ), this, SLOT( toggleBuildPyramidsButton() ) );

  // set up the scale based layer visibility stuff....
  chkUseScaleDependentRendering->setChecked( lyr->hasScaleBasedVisibility() );
  leMinimumScale->setText( QString::number( lyr->minimumScale(), 'f' ) );
  leMinimumScale->setValidator( new QDoubleValidator( 0, std::numeric_limits<float>::max(), 1000, this ) );
  leMaximumScale->setText( QString::number( lyr->maximumScale(), 'f' ) );
  leMaximumScale->setValidator( new QDoubleValidator( 0, std::numeric_limits<float>::max(), 1000, this ) );
  leNoDataValue->setValidator( new QDoubleValidator( -std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 1000, this ) );

  // build GUI components
  QIcon myPyramidPixmap( QgisApp::getThemeIcon( "/mIconPyramid.png" ) );
  QIcon myNoPyramidPixmap( QgisApp::getThemeIcon( "/mIconNoPyramid.png" ) );

  pbnAddValuesManually->setIcon( QgisApp::getThemeIcon( "/mActionNewAttribute.png" ) );
  pbnAddValuesFromDisplay->setIcon( QgisApp::getThemeIcon( "/mActionContextHelp.png" ) );
  pbnRemoveSelectedRow->setIcon( QgisApp::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  pbnDefaultValues->setIcon( QgisApp::getThemeIcon( "/mActionCopySelected.png" ) );
  pbnImportTransparentPixelValues->setIcon( QgisApp::getThemeIcon( "/mActionFileOpen.png" ) );
  pbnExportTransparentPixelValues->setIcon( QgisApp::getThemeIcon( "/mActionFileSave.png" ) );

  mSaveAsImageButton->setIcon( QgisApp::getThemeIcon( "/mActionFileSave.png" ) );

  mMapCanvas = theCanvas;
  mPixelSelectorTool = 0;
  if ( mMapCanvas )
  {
    mPixelSelectorTool = new QgsMapToolEmitPoint( theCanvas );
    connect( mPixelSelectorTool, SIGNAL( canvasClicked( const QgsPoint&, Qt::MouseButton ) ), this, SLOT( pixelSelected( const QgsPoint& ) ) );
  }
  else
  {
    pbnAddValuesFromDisplay->setEnabled( false );
  }

  if ( !mRasterLayer )
  {
    return;
  }
  QgsRasterDataProvider* provider = mRasterLayer->dataProvider();

  // Only do pyramids if dealing directly with GDAL.
  if ( provider->capabilities() & QgsRasterDataProvider::BuildPyramids )
  {
    QgsRasterLayer::RasterPyramidList myPyramidList = mRasterLayer->buildPyramidList();
    QgsRasterLayer::RasterPyramidList::iterator myRasterPyramidIterator;

    for ( myRasterPyramidIterator = myPyramidList.begin();
          myRasterPyramidIterator != myPyramidList.end();
          ++myRasterPyramidIterator )
    {
      if ( myRasterPyramidIterator->exists )
      {
        lbxPyramidResolutions->addItem( new QListWidgetItem( myPyramidPixmap,
                                        QString::number( myRasterPyramidIterator->xDim ) + QString( " x " ) +
                                        QString::number( myRasterPyramidIterator->yDim ) ) );
      }
      else
      {
        lbxPyramidResolutions->addItem( new QListWidgetItem( myNoPyramidPixmap,
                                        QString::number( myRasterPyramidIterator->xDim ) + QString( " x " ) +
                                        QString::number( myRasterPyramidIterator->yDim ) ) );
      }
    }
  }
  else
  {
    // disable Pyramids tab completely
    tabPagePyramids->setEnabled( false );

    // disable Histogram tab completely
    tabPageHistogram->setEnabled( false );
  }

  QgsDebugMsg( "Setting crs to " + mRasterLayer->crs().toWkt() );
  QgsDebugMsg( "Setting crs to " + mRasterLayer->crs().authid() + " - " + mRasterLayer->crs().description() );
  leSpatialRefSys->setText( mRasterLayer->crs().authid() + " - " + mRasterLayer->crs().description() );
  leSpatialRefSys->setCursorPosition( 0 );

  // Set text for pyramid info box
  QString pyramidFormat( "<h2>%1</h2><p>%2 %3 %4</p><b><font color='red'><p>%5</p><p>%6</p>" );
  QString pyramidHeader    = tr( "Description" );
  QString pyramidSentence1 = tr( "Large resolution raster layers can slow navigation in QGIS." );
  QString pyramidSentence2 = tr( "By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom." );
  QString pyramidSentence3 = tr( "You must have write access in the directory where the original data is stored to build pyramids." );
  QString pyramidSentence4 = tr( "Please note that building internal pyramids may alter the original data file and once created they cannot be removed!" );
  QString pyramidSentence5 = tr( "Please note that building internal pyramids could corrupt your image - always make a backup of your data first!" );

  tePyramidDescription->setHtml( pyramidFormat.arg( pyramidHeader ).arg( pyramidSentence1 )
                                 .arg( pyramidSentence2 ).arg( pyramidSentence3 )
                                 .arg( pyramidSentence4 ).arg( pyramidSentence5 ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/RasterLayerProperties/geometry" ).toByteArray() );

  setWindowTitle( tr( "Layer Properties - %1" ).arg( lyr->name() ) );

  tableTransparency->horizontalHeader()->setResizeMode( 0, QHeaderView::Stretch );
  tableTransparency->horizontalHeader()->setResizeMode( 1, QHeaderView::Stretch );

  //resampling
  const QgsRasterRenderer* renderer = mRasterLayer->renderer();
  mZoomedInResamplingComboBox->insertItem( 0, tr( "Nearest neighbour" ) );
  mZoomedInResamplingComboBox->insertItem( 1, tr( "Bilinear" ) );
  mZoomedInResamplingComboBox->insertItem( 2, tr( "Cubic" ) );
  mZoomedOutResamplingComboBox->insertItem( 0, tr( "Nearest neighbour" ) );
  mZoomedOutResamplingComboBox->insertItem( 1, tr( "Average" ) );
  //set combo boxes to current resampling types
  if ( renderer )
  {
    //invert color map
    if ( renderer->invertColor() )
    {
      mInvertColorMapCheckBox->setCheckState( Qt::Checked );
    }

    const QgsRasterResampler* zoomedInResampler = renderer->zoomedInResampler();
    if ( zoomedInResampler )
    {
      if ( zoomedInResampler->type() == "bilinear" )
      {
        mZoomedInResamplingComboBox->setCurrentIndex( 1 );
      }
      else if ( zoomedInResampler->type() == "cubic" )
      {
        mZoomedInResamplingComboBox->setCurrentIndex( 2 );
      }
    }
    else
    {
      mZoomedInResamplingComboBox->setCurrentIndex( 0 );
    }

    const QgsRasterResampler* zoomedOutResampler = renderer->zoomedOutResampler();
    if ( zoomedOutResampler )
    {
      if ( zoomedOutResampler->type() == "bilinear" ) //bilinear resampler does averaging when zooming out
      {
        mZoomedOutResamplingComboBox->setCurrentIndex( 1 );
      }
    }
    else
    {
      mZoomedOutResamplingComboBox->setCurrentIndex( 0 );
    }
    mMaximumOversamplingSpinBox->setValue( renderer->maxOversampling() );
  }

  //transparency band
  if ( provider )
  {
    cboxTransparencyBand->addItem( tr( "None" ), -1 );
    int nBands = provider->bandCount();
    QString bandName;

    if ( nBands == 1 )
    {
      /* Color interpretation name only makes sense for 1-band rasters */
      bandName = provider->colorInterpretationName( 1 );
      if ( bandName == "Undefined" )
        bandName = provider->generateBandName( 1 );
      cboxTransparencyBand->addItem( bandName, 1 );
    }
    else if ( nBands > 1 )
    {
      for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
      {
        bandName = provider->generateBandName( i );
        cboxTransparencyBand->addItem( bandName, i );
      }
    }
    if ( renderer )
    {
      cboxTransparencyBand->setCurrentIndex( cboxTransparencyBand->findData( renderer->alphaBand() ) );
    }
  }

  //insert renderer widgets into registry
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "paletted", QgsPalettedRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "multibandcolor", QgsMultiBandColorRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "singlebandpseudocolor", QgsSingleBandPseudoColorRendererWidget::create );
  QgsRasterRendererRegistry::instance()->insertWidgetFunction( "singlebandgray", QgsSingleBandGrayRendererWidget::create );

  //fill available renderers into combo box
  QList< QgsRasterRendererRegistryEntry > rendererEntries = QgsRasterRendererRegistry::instance()->entries();
  QList< QgsRasterRendererRegistryEntry >::const_iterator rendererIt = rendererEntries.constBegin();
  for ( ; rendererIt != rendererEntries.constEnd(); ++rendererIt )
  {
    mRenderTypeComboBox->addItem( rendererIt->visibleName, rendererIt->name );
  }

  if ( renderer )
  {
    QString rendererType = renderer->type();
    int widgetIndex = mRenderTypeComboBox->findData( rendererType );
    if ( widgetIndex != -1 )
    {
      mRenderTypeComboBox->setCurrentIndex( widgetIndex );
    }

    //prevent change between singleband color renderer and the other renderers
    if ( rendererType == "singlebandcolordata" )
    {
      mRenderTypeComboBox->setEnabled( false );
    }
    else
    {
      mRenderTypeComboBox->removeItem( mRenderTypeComboBox->findData( "singlebandcolordata" ) );
    }
  }
  on_mRenderTypeComboBox_currentIndexChanged( mRenderTypeComboBox->currentIndex() );

  // histogram
  mHistoPicker = NULL;
  mHistoZoomer = NULL;
  mHistoMarkerMin = NULL;
  mHistoMarkerMax = NULL;
  if ( tabPageHistogram->isEnabled() )
  {
    //band selector
    int myBandCountInt = mRasterLayer->bandCount();
    for ( int myIteratorInt = 1;
          myIteratorInt <= myBandCountInt;
          ++myIteratorInt )
    {
      cboHistoBand->addItem( mRasterLayer->bandName( myIteratorInt ) );
    }

    // histo min/max selectors
    leHistoMin->setValidator( new QDoubleValidator( this ) );
    leHistoMax->setValidator( new QDoubleValidator( this ) );
    // this might generate many refresh events! test..
    // connect( leHistoMin, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateHistoMarkers() ) );
    // connect( leHistoMax, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateHistoMarkers() ) );
    // connect( leHistoMin, SIGNAL( textChanged( const QString & ) ), this, SLOT( applyHistoMin() ) );
    // connect( leHistoMax, SIGNAL( textChanged( const QString & ) ), this, SLOT( applyHistoMax() ) );
    connect( leHistoMin, SIGNAL( editingFinished() ), this, SLOT( applyHistoMin() ) );
    connect( leHistoMax, SIGNAL( editingFinished() ), this, SLOT( applyHistoMax() ) );
    connect( cbxHistoShow, SIGNAL( toggled( bool ) ), this, SLOT( updateHistoMarkers() ) );

    // histo actions
    QMenu* menu = new QMenu( this );
    menu->setSeparatorsCollapsible( false );
    btnHistoActions->setMenu( menu );
    QActionGroup* group;

    group = new QActionGroup( this );
    group->setExclusive( false );
    connect( group, SIGNAL( triggered( QAction* ) ), this, SLOT( histoActionTriggered( QAction* ) ) );
    QAction* action;
    // action = new QAction( tr( "Load min/max from band" ), group );
    action = new QAction( tr( "Load min/max" ), group );
    action->setSeparator( true );
    menu->addAction( action );
    action = new QAction( tr( "Estimate (faster)" ), group );
    action->setData( QVariant( "Load estimate" ) );
    menu->addAction( action );
    action = new QAction( tr( "Actual (slower)" ), group );
    action->setData( QVariant( "Load actual" ) );
    menu->addAction( action );
    action = new QAction( tr( "Current extent" ), group );
    action->setData( QVariant( "Load extent" ) );
    menu->addAction( action );
    // stddev was removed...
    action = new QAction( tr( "Use 1 stddev" ), group );
    action->setData( QVariant( "Load 1 stddev" ) );
    menu->addAction( action );
    /*
    action = new QAction( tr( "Use custom stddev" ), group );
    action->setData( QVariant( "Load stddev" ) );
    menu->addAction( action );
    */
    action = new QAction( tr( "Reset" ), group );
    action->setData( QVariant( "Load reset" ) );
    menu->addAction( action );
    action = new QAction( tr( "Load for all bands" ), group );
    action->setData( QVariant( "Load apply all" ) );
    action->setCheckable( true );
    action->setChecked( true );
    menu->addAction( action );

    menu->addSeparator( );
    group = new QActionGroup( this );
    connect( group, SIGNAL( triggered( QAction* ) ), this, SLOT( histoActionTriggered( QAction* ) ) );
    action = new QAction( tr( "Compute Histogram" ), group );
    action->setData( QVariant( "Compute Histogram" ) );
    menu->addAction( action );
  }

  // update based on lyr's current state
  sync();

  // set current tab after everything has been initialized
  tabBar->setCurrentIndex( settings.value( "/Windows/RasterLayerProperties/row" ).toInt() );

} // QgsRasterLayerProperties ctor


QgsRasterLayerProperties::~QgsRasterLayerProperties()
{
  QSettings settings;
  settings.setValue( "/Windows/RasterLayerProperties/geometry", saveGeometry() );
  settings.setValue( "/Windows/RasterLayerProperties/row", tabBar->currentIndex() );
  if ( mPixelSelectorTool )
  {
    delete mPixelSelectorTool;
  }
}

void QgsRasterLayerProperties::populateTransparencyTable( QgsRasterRenderer* renderer )
{
  QgsDebugMsg( "entering." );
  if ( !mRasterLayer )
  {
    return;
  }

  if ( !renderer )
  {
    return;
  }

  tableTransparency->clear();
  tableTransparency->setColumnCount( 0 );
  tableTransparency->setRowCount( 0 );

  QList<int> bandList = renderer->usesBands();
  tableTransparency->setColumnCount( bandList.size() + 1 );
  for ( int i = 0; i < bandList.size(); ++i )
  {
    tableTransparency->setHorizontalHeaderItem( i, new QTableWidgetItem( QString::number( bandList.at( i ) ) ) );
  }
  tableTransparency->setHorizontalHeaderItem( bandList.size(), new QTableWidgetItem( tr( "Percent Transparent" ) ) );

  const QgsRasterTransparency* rasterTransparency = renderer->rasterTransparency();
  if ( !rasterTransparency )
  {
    return;
  }

  if ( bandList.count() == 1 )
  {
    QList<QgsRasterTransparency::TransparentSingleValuePixel> pixelList = rasterTransparency->transparentSingleValuePixelList();
    for ( int i = 0; i < pixelList.size(); ++i )
    {
      tableTransparency->insertRow( i );
      QTableWidgetItem* grayItem = new QTableWidgetItem( QString::number( pixelList[i].pixelValue ) );
      QTableWidgetItem* percentItem = new QTableWidgetItem( QString::number( pixelList[i].percentTransparent ) );
      tableTransparency->setItem( i, 0, grayItem );
      tableTransparency->setItem( i, 1, percentItem );
    }
  }
  else if ( bandList.count() == 3 )
  {
    QList<QgsRasterTransparency::TransparentThreeValuePixel> pixelList = rasterTransparency->transparentThreeValuePixelList();
    for ( int i = 0; i < pixelList.size(); ++i )
    {
      tableTransparency->insertRow( i );
      QTableWidgetItem* redItem = new QTableWidgetItem( QString::number( pixelList[i].red ) );
      QTableWidgetItem* greenItem = new QTableWidgetItem( QString::number( pixelList[i].green ) );
      QTableWidgetItem* blueItem = new QTableWidgetItem( QString::number( pixelList[i].blue ) );
      QTableWidgetItem* transparentItem = new QTableWidgetItem( QString::number( pixelList[i].percentTransparent ) );

      tableTransparency->setItem( i, 0, redItem );
      tableTransparency->setItem( i, 1, greenItem );
      tableTransparency->setItem( i, 2, blueItem );
      tableTransparency->setItem( i, 3, transparentItem );
    }
  }

  tableTransparency->resizeColumnsToContents();
  tableTransparency->resizeRowsToContents();
}

void QgsRasterLayerProperties::setRendererWidget( const QString& rendererName )
{
  QgsRasterRendererWidget* oldWidget = mRendererWidget;

  QgsRasterRendererRegistryEntry rendererEntry;
  if ( QgsRasterRendererRegistry::instance()->rendererData( rendererName, rendererEntry ) )
  {
    if ( rendererEntry.widgetCreateFunction ) //single band color data renderer e.g. has no widget
    {
      mRendererWidget = ( *rendererEntry.widgetCreateFunction )( mRasterLayer );
      mRendererStackedWidget->addWidget( mRendererWidget );
      if ( oldWidget )
      {
        //compare used bands in new and old renderer and reset transparency dialog if different
        QgsRasterRenderer* oldRenderer = oldWidget->renderer();
        QgsRasterRenderer* newRenderer = mRendererWidget->renderer();
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
  delete oldWidget;
}

/**
  @note moved from ctor

  Previously this dialog was created anew with each right-click pop-up menu
  invokation.  Changed so that the dialog always exists after first
  invocation, and is just re-synchronized with its layer's state when
  re-shown.

*/
void QgsRasterLayerProperties::sync()
{
  QSettings myQSettings;

  if ( mRasterLayer->dataProvider()->dataType( 1 ) == QgsRasterDataProvider::ARGBDataType )
  {
    gboxNoDataValue->setEnabled( false );
    gboxCustomTransparency->setEnabled( false );
    tabBar->setCurrentWidget( tabPageMetadata );
  }

  if ( !( mRasterLayer->dataProvider()->capabilities() & QgsRasterDataProvider::BuildPyramids ) )
  {
    if ( tabPagePyramids != NULL )
    {
      delete tabPagePyramids;
      tabPagePyramids = NULL;
    }
  }

  if ( !( mRasterLayer->dataProvider()->capabilities() & QgsRasterDataProvider::Histogram ) )
  {
    if ( tabPageHistogram != NULL )
    {
      delete tabPageHistogram;
      tabPageHistogram = NULL;
    }
  }

  QgsDebugMsg( "populate transparency tab" );

  /*
   * Transparent Pixel Tab
   */

  //set the transparency slider
  QgsRasterRenderer* renderer = mRasterLayer->renderer();
  if ( renderer )
  {
    sliderTransparency->setValue(( 1.0 - renderer->opacity() ) * 255 );
    //update the transparency percentage label
    sliderTransparency_valueChanged(( 1.0 - renderer->opacity() ) * 255 );

    int myIndex = renderer->alphaBand();
    if ( -1 != myIndex )
    {
      cboxTransparencyBand->setCurrentIndex( myIndex );
    }
    else
    {
      cboxTransparencyBand->setCurrentIndex( cboxTransparencyBand->findText( TRSTRING_NOT_SET ) );
    }
  }

  //add current NoDataValue to NoDataValue line edit
  if ( mRasterLayer->isNoDataValueValid() )
  {
    leNoDataValue->insert( QString::number( mRasterLayer->noDataValue(), 'g' ) );
  }
  else
  {
    leNoDataValue->insert( "" );
  }

  populateTransparencyTable( mRasterLayer->renderer() );

  QgsDebugMsg( "populate colormap tab" );
  /*
   * Transparent Pixel Tab
   */

  QgsDebugMsg( "populate general tab" );
  /*
   * General Tab
   */

  //these properties (layer name and label) are provided by the qgsmaplayer superclass
  leLayerSource->setText( mRasterLayer->source() );
  leDisplayName->setText( mRasterLayer->name() );

  //display the raster dimensions and no data value
  if ( mRasterLayer->dataProvider()->capabilities() & QgsRasterDataProvider::Size )
  {
    lblColumns->setText( tr( "Columns: %1" ).arg( mRasterLayer->width() ) );
    lblRows->setText( tr( "Rows: %1" ).arg( mRasterLayer->height() ) );
  }
  else
  {
    // TODO: Account for fixedWidth and fixedHeight WMS layers
    lblColumns->setText( tr( "Columns: " ) + tr( "n/a" ) );
    lblRows->setText( tr( "Rows: " ) + tr( "n/a" ) );
  }

  if ( mRasterLayer->dataProvider()->dataType( 1 ) == QgsRasterDataProvider::ARGBDataType )
  {
    lblNoData->setText( tr( "No-Data Value: " ) + tr( "n/a" ) );
  }
  else
  {
    if ( mRasterLayer->isNoDataValueValid() )
    {
      lblNoData->setText( tr( "No-Data Value: %1" ).arg( mRasterLayer->noDataValue() ) );
    }
    else
    {
      lblNoData->setText( tr( "No-Data Value: Not Set" ) );
    }
  }

  //get the thumbnail for the layer
  QPixmap myQPixmap = QPixmap( pixmapThumbnail->width(), pixmapThumbnail->height() );
  mRasterLayer->thumbnailAsPixmap( &myQPixmap );
  pixmapThumbnail->setPixmap( myQPixmap );

  //update the legend pixmap on this dialog
  //pixmapLegend->setPixmap( mRasterLayer->legendAsPixmap() );
  pixmapLegend->setScaledContents( true );
  pixmapLegend->repaint();

  //set the palette pixmap
  //pixmapPalette->setPixmap( mRasterLayer->paletteAsPixmap( mRasterLayer->bandNumber( mRasterLayer->grayBandName() ) ) );
  pixmapPalette->setScaledContents( true );
  pixmapPalette->repaint();

  QgsDebugMsg( "populate metadata tab" );
  /*
   * Metadata Tab
   */
  //populate the metadata tab's text browser widget with gdal metadata info
  QString myStyle = QgsApplication::reportStyleSheet();
  txtbMetadata->document()->setDefaultStyleSheet( myStyle );
  txtbMetadata->setHtml( mRasterLayer->metadata() );

  mLayerTitleLineEdit->setText( mRasterLayer->title() );
  mLayerAbstractTextEdit->setPlainText( mRasterLayer->abstract() );

} // QgsRasterLayerProperties::sync()

/*
 *
 * PUBLIC AND PRIVATE SLOTS
 *
 */
void QgsRasterLayerProperties::apply()
{
  QgsDebugMsg( "apply processing symbology tab" );
  /*
   * Symbology Tab
   */

  //set whether the layer histogram should be inverted
  //mRasterLayer->setInvertHistogram( cboxInvertColorMap->isChecked() );

  QgsDebugMsg( "processing transparency tab" );
  /*
   * Transparent Pixel Tab
   */

  //set NoDataValue
  bool myDoubleOk = false;
  if ( "" != leNoDataValue->text() )
  {
    double myNoDataValue = leNoDataValue->text().toDouble( &myDoubleOk );
    if ( myDoubleOk )
    {
      mRasterLayer->setNoDataValue( myNoDataValue );
    }
  }

  //set renderer from widget
  QgsRasterRendererWidget* rendererWidget = dynamic_cast<QgsRasterRendererWidget*>( mRendererStackedWidget->currentWidget() );
  if ( rendererWidget )
  {
    mRasterLayer->setRenderer( rendererWidget->renderer() );
  }

  //transparency settings
  QgsRasterRenderer* rasterRenderer = mRasterLayer->renderer();
  rasterRenderer->setAlphaBand( cboxTransparencyBand->itemData( cboxTransparencyBand->currentIndex() ).toInt() );

  //Walk through each row in table and test value. If not valid set to 0.0 and continue building transparency list
  QgsRasterTransparency* rasterTransparency = new QgsRasterTransparency();
  if ( tableTransparency->columnCount() == 4 )
  {
    double myTransparentValue;
    double myPercentTransparent;
    QgsRasterTransparency::TransparentThreeValuePixel myTransparentPixel;
    QList<QgsRasterTransparency::TransparentThreeValuePixel> myTransparentThreeValuePixelList;
    for ( int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++ )
    {
      if ( !tableTransparency->item( myListRunner, 0 ) )
      {
        myTransparentPixel.red = 0.0;
        QTableWidgetItem* newItem = new QTableWidgetItem( "0.0" );
        tableTransparency->setItem( myListRunner, 0, newItem );
      }
      else
      {
        myTransparentValue = tableTransparency->item( myListRunner, 0 )->text().toDouble( &myDoubleOk );
        if ( myDoubleOk )
        {
          myTransparentPixel.red = myTransparentValue;
        }
        else
        {
          myTransparentPixel.red = 0.0;
          tableTransparency->item( myListRunner, 0 )->setText( "0.0" );
        }
      }

      if ( !tableTransparency->item( myListRunner, 1 ) )
      {
        myTransparentPixel.green = 0.0;
        QTableWidgetItem* newItem = new QTableWidgetItem( "0.0" );
        tableTransparency->setItem( myListRunner, 1, newItem );
      }
      else
      {
        myTransparentValue = tableTransparency->item( myListRunner, 1 )->text().toDouble( &myDoubleOk );
        if ( myDoubleOk )
        {
          myTransparentPixel.green = myTransparentValue;
        }
        else
        {
          myTransparentPixel.green = 0.0;
          tableTransparency->item( myListRunner, 1 )->setText( "0.0" );
        }
      }

      if ( !tableTransparency->item( myListRunner, 2 ) )
      {
        myTransparentPixel.blue = 0.0;
        QTableWidgetItem* newItem = new QTableWidgetItem( "0.0" );
        tableTransparency->setItem( myListRunner, 2, newItem );
      }
      else
      {
        myTransparentValue = tableTransparency->item( myListRunner, 2 )->text().toDouble( &myDoubleOk );
        if ( myDoubleOk )
        {
          myTransparentPixel.blue = myTransparentValue;
        }
        else
        {
          myTransparentPixel.blue = 0.0;
          tableTransparency->item( myListRunner, 2 )->setText( "0.0" );
        }
      }

      if ( !tableTransparency->item( myListRunner, 3 ) )
      {
        myTransparentPixel.percentTransparent = 100.0;
        QTableWidgetItem* newItem = new QTableWidgetItem( "100.0" );
        tableTransparency->setItem( myListRunner, 3, newItem );
      }
      else
      {
        QString myNumberFormatter;
        myPercentTransparent = tableTransparency->item( myListRunner, 3 )->text().toDouble( &myDoubleOk );
        if ( myDoubleOk )
        {
          if ( myPercentTransparent > 100.0 )
            myTransparentPixel.percentTransparent = 100.0;
          else if ( myPercentTransparent < 0.0 )
            myTransparentPixel.percentTransparent = 0.0;
          else
            myTransparentPixel.percentTransparent = myPercentTransparent;

          tableTransparency->item( myListRunner, 3 )->setText( myNumberFormatter.sprintf( "%.2f", myTransparentPixel.percentTransparent ) );
        }
        else
        {
          myTransparentPixel.percentTransparent = 100.0;
          tableTransparency->item( myListRunner, 3 )->setText( "100.0" );
        }
      }

      myTransparentThreeValuePixelList.append( myTransparentPixel );
    }
    rasterTransparency->setTransparentThreeValuePixelList( myTransparentThreeValuePixelList );
  }
  else if ( tableTransparency->columnCount() == 2 )
  {
    double myTransparentValue;
    double myPercentTransparent;
    QgsRasterTransparency::TransparentSingleValuePixel myTransparentPixel;
    QList<QgsRasterTransparency::TransparentSingleValuePixel> myTransparentSingleValuePixelList;
    for ( int myListRunner = 0; myListRunner < tableTransparency->rowCount(); myListRunner++ )
    {
      if ( !tableTransparency->item( myListRunner, 0 ) )
      {
        myTransparentPixel.pixelValue = 0.0;
        QTableWidgetItem* newItem = new QTableWidgetItem( "0.0" );
        tableTransparency->setItem( myListRunner, 0, newItem );
      }
      else
      {
        myTransparentValue = tableTransparency->item( myListRunner, 0 )->text().toDouble( &myDoubleOk );
        if ( myDoubleOk )
        {
          myTransparentPixel.pixelValue = myTransparentValue;
        }
        else
        {
          myTransparentPixel.pixelValue = 0.0;
          tableTransparency->item( myListRunner, 0 )->setText( "0.0" );
        }
      }

      if ( !tableTransparency->item( myListRunner, 1 ) )
      {
        myTransparentPixel.percentTransparent = 100.0;
        QTableWidgetItem* newItem = new QTableWidgetItem( "100.0" );
        tableTransparency->setItem( myListRunner, 1, newItem );
      }
      else
      {
        QString myNumberFormatter;
        myPercentTransparent = tableTransparency->item( myListRunner, 1 )->text().toDouble( &myDoubleOk );
        if ( myDoubleOk )
        {
          if ( myPercentTransparent > 100.0 )
            myTransparentPixel.percentTransparent = 100.0;
          else if ( myPercentTransparent < 0.0 )
            myTransparentPixel.percentTransparent = 0.0;
          else
            myTransparentPixel.percentTransparent = myPercentTransparent;

          tableTransparency->item( myListRunner, 1 )->setText( myNumberFormatter.sprintf( "%.2f", myTransparentPixel.percentTransparent ) );
        }
        else
        {
          myTransparentPixel.percentTransparent = 100.0;
          tableTransparency->item( myListRunner, 1 )->setText( "100.0" );
        }
      }

      myTransparentSingleValuePixelList.append( myTransparentPixel );
    }
    rasterTransparency->setTransparentSingleValuePixelList( myTransparentSingleValuePixelList );
  }
  rasterRenderer->setRasterTransparency( rasterTransparency );

  //set global transparency
  rasterRenderer->setOpacity(( 255 - sliderTransparency->value() ) / 255.0 );

  //invert color map
  rasterRenderer->setInvertColor( mInvertColorMapCheckBox->isChecked() );

  QgsDebugMsg( "processing general tab" );
  /*
   * General Tab
   */
  mRasterLayer->setLayerName( leDisplayName->text() );

  // set up the scale based layer visibility stuff....
  mRasterLayer->toggleScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mRasterLayer->setMinimumScale( leMinimumScale->text().toFloat() );
  mRasterLayer->setMaximumScale( leMaximumScale->text().toFloat() );

  //update the legend pixmap
  pixmapLegend->setPixmap( mRasterLayer->legendAsPixmap() );
  pixmapLegend->setScaledContents( true );
  pixmapLegend->repaint();

  QgsRasterResampler* zoomedInResampler = 0;
  QString zoomedInResamplingMethod = mZoomedInResamplingComboBox->currentText();
  if ( zoomedInResamplingMethod == tr( "Bilinear" ) )
  {
    zoomedInResampler = new QgsBilinearRasterResampler();
  }
  else if ( zoomedInResamplingMethod == tr( "Cubic" ) )
  {
    zoomedInResampler = new QgsCubicRasterResampler();
  }

  if ( rasterRenderer )
  {
    rasterRenderer->setZoomedInResampler( zoomedInResampler );
  }

  //raster resampling
  QgsRasterResampler* zoomedOutResampler = 0;
  QString zoomedOutResamplingMethod = mZoomedOutResamplingComboBox->currentText();
  if ( zoomedOutResamplingMethod == tr( "Average" ) )
  {
    zoomedOutResampler = new QgsBilinearRasterResampler();
  }

  if ( rasterRenderer )
  {
    rasterRenderer->setZoomedOutResampler( zoomedOutResampler );
  }

  if ( rasterRenderer )
  {
    rasterRenderer->setMaxOversampling( mMaximumOversamplingSpinBox->value() );
  }


  //get the thumbnail for the layer
  QPixmap myQPixmap = QPixmap( pixmapThumbnail->width(), pixmapThumbnail->height() );
  mRasterLayer->thumbnailAsPixmap( &myQPixmap );
  pixmapThumbnail->setPixmap( myQPixmap );

  mRasterLayer->setTitle( mLayerTitleLineEdit->text() );
  mRasterLayer->setAbstract( mLayerAbstractTextEdit->toPlainText() );

  // update symbology
  emit refreshLegend( mRasterLayer->id(), false );

  //no need to delete the old one, maplayer will do it if needed
  mRasterLayer->setCacheImage( 0 );

  //make sure the layer is redrawn
  mRasterLayer->triggerRepaint();

  // notify the project we've made a change
  QgsProject::instance()->dirty( true );
}//apply

void QgsRasterLayerProperties::on_buttonBuildPyramids_clicked()
{

  connect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mPyramidProgress, SLOT( setValue( int ) ) );
  //
  // Go through the list marking any files that are selected in the listview
  // as true so that we can generate pyramids for them.
  //
  QgsRasterLayer::RasterPyramidList myPyramidList = mRasterLayer->buildPyramidList();
  for ( int myCounterInt = 0; myCounterInt < lbxPyramidResolutions->count(); myCounterInt++ )
  {
    QListWidgetItem *myItem = lbxPyramidResolutions->item( myCounterInt );
    //mark to be pyramided
    myPyramidList[myCounterInt].build = myItem->isSelected() || myPyramidList[myCounterInt].exists;
  }
  //
  // Ask raster layer to build the pyramids
  //

  // let the user know we're going to possibly be taking a while
  QApplication::setOverrideCursor( Qt::WaitCursor );
  bool myBuildInternalFlag = cbxInternalPyramids->isChecked();
  QString res = mRasterLayer->buildPyramids(
                  myPyramidList,
                  cboResamplingMethod->currentText(),
                  myBuildInternalFlag );
  QApplication::restoreOverrideCursor();
  mPyramidProgress->setValue( 0 );
  buttonBuildPyramids->setEnabled( false );
  disconnect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mPyramidProgress, SLOT( setValue( int ) ) );
  if ( !res.isNull() )
  {
    if ( res == "ERROR_WRITE_ACCESS" )
    {
      QMessageBox::warning( this, tr( "Write access denied" ),
                            tr( "Write access denied. Adjust the file permissions and try again." ) );
    }
    else if ( res == "ERROR_WRITE_FORMAT" )
    {
      QMessageBox::warning( this, tr( "Building pyramids failed." ),
                            tr( "The file was not writable. Some formats do not "
                                "support pyramid overviews. Consult the GDAL documentation if in doubt." ) );
    }
    else if ( res == "FAILED_NOT_SUPPORTED" )
    {
      QMessageBox::warning( this, tr( "Building pyramids failed." ),
                            tr( "Building pyramid overviews is not supported on this type of raster." ) );
    }
    else if ( res == "ERROR_JPEG_COMPRESSION" )
    {
      QMessageBox::warning( this, tr( "Building pyramids failed." ),
                            tr( "Building internal pyramid overviews is not supported on raster layers with JPEG compression and your current libtiff library." ) );
    }
    else if ( res == "ERROR_VIRTUAL" )
    {
      QMessageBox::warning( this, tr( "Building pyramids failed." ),
                            tr( "Building pyramid overviews is not supported on this type of raster." ) );
    }

  }


  //
  // repopulate the pyramids list
  //
  lbxPyramidResolutions->clear();
  // Need to rebuild list as some or all pyramids may have failed to build
  myPyramidList = mRasterLayer->buildPyramidList();
  QIcon myPyramidPixmap( QgisApp::getThemeIcon( "/mIconPyramid.png" ) );
  QIcon myNoPyramidPixmap( QgisApp::getThemeIcon( "/mIconNoPyramid.png" ) );

  QgsRasterLayer::RasterPyramidList::iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator = myPyramidList.begin();
        myRasterPyramidIterator != myPyramidList.end();
        ++myRasterPyramidIterator )
  {
    if ( myRasterPyramidIterator->exists )
    {
      lbxPyramidResolutions->addItem( new QListWidgetItem( myPyramidPixmap,
                                      QString::number( myRasterPyramidIterator->xDim ) + QString( " x " ) +
                                      QString::number( myRasterPyramidIterator->yDim ) ) );
    }
    else
    {
      lbxPyramidResolutions->addItem( new QListWidgetItem( myNoPyramidPixmap,
                                      QString::number( myRasterPyramidIterator->xDim ) + QString( " x " ) +
                                      QString::number( myRasterPyramidIterator->yDim ) ) );
    }
  }
  //update the legend pixmap
  pixmapLegend->setPixmap( mRasterLayer->legendAsPixmap() );
  pixmapLegend->setScaledContents( true );
  pixmapLegend->repaint();
  //populate the metadata tab's text browser widget with gdal metadata info
  QString myStyle = QgsApplication::reportStyleSheet();
  txtbMetadata->setHtml( mRasterLayer->metadata() );
  txtbMetadata->document()->setDefaultStyleSheet( myStyle );
}

void QgsRasterLayerProperties::on_mRenderTypeComboBox_currentIndexChanged( int index )
{
  if ( index < 0 )
  {
    return;
  }

  QString rendererName = mRenderTypeComboBox->itemData( index ).toString();
  setRendererWidget( rendererName );
}

void QgsRasterLayerProperties::on_pbnAddValuesFromDisplay_clicked()
{
  if ( mMapCanvas && mPixelSelectorTool )
  {
    mMapCanvas->setMapTool( mPixelSelectorTool );
    //Need to work around the modality of the dialog but can not just hide() it.
    setModal( false );
    lower();
  }
}

void QgsRasterLayerProperties::on_pbnAddValuesManually_clicked()
{
  tableTransparency->insertRow( tableTransparency->rowCount() );
  tableTransparency->setItem( tableTransparency->rowCount() - 1, tableTransparency->columnCount() - 1, new QTableWidgetItem( "100.0" ) );
}

void QgsRasterLayerProperties::on_pbnChangeSpatialRefSys_clicked()
{

  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setSelectedCrsId( mRasterLayer->crs().srsid() );
  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem srs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    mRasterLayer->setCrs( srs );
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }
  delete mySelector;

  leSpatialRefSys->setText( mRasterLayer->crs().authid() + " - " + mRasterLayer->crs().description() );
  leSpatialRefSys->setCursorPosition( 0 );
}

void QgsRasterLayerProperties::on_pbnDefaultValues_clicked()
{
  if ( !mRendererWidget )
  {
    return;
  }

  QgsRasterRenderer* r = mRendererWidget->renderer();
  if ( !r )
  {
    return;
  }

  int nBands = r->usesBands().size();
  delete r;

  if ( nBands == 3 )
  {
    tableTransparency->clear();
    tableTransparency->setColumnCount( 4 );
    tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Red" ) ) );
    tableTransparency->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Green" ) ) );
    tableTransparency->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Blue" ) ) );
    tableTransparency->setHorizontalHeaderItem( 3, new QTableWidgetItem( tr( "Percent Transparent" ) ) );
    if ( mRasterLayer->isNoDataValueValid() )
    {
      tableTransparency->insertRow( tableTransparency->rowCount() );
      tableTransparency->setItem( 0, 0, new QTableWidgetItem( QString::number( mRasterLayer->noDataValue(), 'f' ) ) );
      tableTransparency->setItem( 0, 1, new QTableWidgetItem( QString::number( mRasterLayer->noDataValue(), 'f' ) ) );
      tableTransparency->setItem( 0, 2, new QTableWidgetItem( QString::number( mRasterLayer->noDataValue(), 'f' ) ) );
      tableTransparency->setItem( 0, 3, new QTableWidgetItem( "100.0" ) );
    }
  }
  else //1 band
  {
    tableTransparency->clear();
    tableTransparency->setColumnCount( 2 );
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
    tableTransparency->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Percent Transparent" ) ) );

    if ( mRasterLayer->isNoDataValueValid() )
    {
      tableTransparency->insertRow( tableTransparency->rowCount() );
      tableTransparency->setItem( 0, 0, new QTableWidgetItem( QString::number( mRasterLayer->noDataValue(), 'f' ) ) );
      tableTransparency->setItem( 0, 1, new QTableWidgetItem( "100.0" ) );
    }
  }
}

void QgsRasterLayerProperties::on_pbnExportTransparentPixelValues_clicked()
{
  QSettings myQSettings;
  QString myLastDir = myQSettings.value( "lastRasterFileFilterDir", "" ).toString();
  QString myFileName = QFileDialog::getSaveFileName( this, tr( "Save file" ), myLastDir, tr( "Textfile" ) + " (*.txt)" );
  if ( !myFileName.isEmpty() )
  {
    if ( !myFileName.endsWith( ".txt", Qt::CaseInsensitive ) )
    {
      myFileName = myFileName + ".txt";
    }

    QFile myOutputFile( myFileName );
    if ( myOutputFile.open( QFile::WriteOnly ) )
    {
      QTextStream myOutputStream( &myOutputFile );
      myOutputStream << "# " << tr( "QGIS Generated Transparent Pixel Value Export File" ) << "\n";
      if ( /*rbtnThreeBand->isChecked() &&*/ QgsRasterLayer::MultiBandColor == mRasterLayer->drawingStyle() )
      {
        myOutputStream << "#\n#\n# " << tr( "Red" ) << "\t" << tr( "Green" ) << "\t" << tr( "Blue" ) << "\t" << tr( "Percent Transparent" );
        for ( int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++ )
        {
          myOutputStream << "\n" << tableTransparency->item( myTableRunner, 0 )->text() << "\t" << tableTransparency->item( myTableRunner, 1 )->text() << "\t" << tableTransparency->item( myTableRunner, 2 )->text() << "\t" << tableTransparency->item( myTableRunner, 3 )->text();
        }
      }
      else
      {
        if ( QgsRasterLayer::PalettedColor != mRasterLayer->drawingStyle() &&
             QgsRasterLayer::PalettedSingleBandGray != mRasterLayer->drawingStyle() &&
             QgsRasterLayer::PalettedSingleBandPseudoColor != mRasterLayer->drawingStyle() &&
             QgsRasterLayer::PalettedMultiBandColor != mRasterLayer->drawingStyle() )
        {
          myOutputStream << "#\n#\n# " << tr( "Gray" ) << "\t" << tr( "Percent Transparent" );
        }
        else
        {
          myOutputStream << "#\n#\n# " << tr( "Indexed Value" ) << "\t" << tr( "Percent Transparent" );
        }

        for ( int myTableRunner = 0; myTableRunner < tableTransparency->rowCount(); myTableRunner++ )
        {
          myOutputStream << "\n" << tableTransparency->item( myTableRunner, 0 )->text() << "\t" << tableTransparency->item( myTableRunner, 1 )->text();
        }
      }
    }
    else
    {
      QMessageBox::warning( this, tr( "Write access denied" ), tr( "Write access denied. Adjust the file permissions and try again.\n\n" ) );
    }
  }
}

void QgsRasterLayerProperties::on_tabBar_currentChanged( int theTab )
{
  int myHistogramTab = 5;
  if ( theTab == myHistogramTab )
  {
    refreshHistogram();
  }
  else if ( tabPageHistogram->isEnabled() )
  {
    if ( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    btnHistoMin->setChecked( false );
    btnHistoMax->setChecked( false );
  }
}

void QgsRasterLayerProperties::on_btnHistoCompute_clicked()
{
// Histogram computation can be called either by clicking the "Compute Histogram" button
// which is only visible if there is no cached histogram or by calling the
// "Compute Histogram" action. Due to limitations in the gdal api, it is not possible
// to re-calculate the histogramif it has already been calculated
  QgsDebugMsg( "Entered" );
  computeHistogram( true );
  refreshHistogram();
}

bool QgsRasterLayerProperties::computeHistogram( bool forceComputeFlag )
{
  const int BINCOUNT = RASTER_HISTOGRAM_BINS; // 256 - defined in qgsrasterdataprovider.h
  bool myIgnoreOutOfRangeFlag = true;
  bool myThoroughBandScanFlag = false;
  int myBandCountInt = mRasterLayer->bandCount();

  // if forceComputeFlag = false make sure raster has cached histogram, else return false
  if ( ! forceComputeFlag )
  {
    for ( int myIteratorInt = 1;
          myIteratorInt <= myBandCountInt;
          ++myIteratorInt )
    {
      if ( ! mRasterLayer->hasCachedHistogram( myIteratorInt, BINCOUNT ) )
      {
        QgsDebugMsg( QString( "band %1 does not have cached histo" ).arg( myIteratorInt ) );
        return false;
      }
    }
  }

  // compute histogram
  stackedWidget2->setCurrentIndex( 1 );
  connect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mHistogramProgress, SLOT( setValue( int ) ) );
  QApplication::setOverrideCursor( Qt::WaitCursor );

  for ( int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt )
  {
    mRasterLayer->populateHistogram( myIteratorInt, BINCOUNT, myIgnoreOutOfRangeFlag, myThoroughBandScanFlag );
  }

  disconnect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mHistogramProgress, SLOT( setValue( int ) ) );
  // mHistogramProgress->hide();
  stackedWidget2->setCurrentIndex( 0 );
  QApplication::restoreOverrideCursor();

  return true;
}

void QgsRasterLayerProperties::refreshHistogram()
{
  // Explanation:
  // We use the gdal histogram creation routine is called for each selected
  // layer. Currently the hist is hardcoded to create 256 bins. Each bin stores
  // the total number of cells that fit into the range defined by that bin.
  //
  // The graph routine below determines the greatest number of pixels in any given
  // bin in all selected layers, and the min. It then draws a scaled line between min
  // and max - scaled to image height. 1 line drawn per selected band
  //
  const int BINCOUNT = RASTER_HISTOGRAM_BINS; // 256 - defined in qgsrasterdataprovider.h
  int myBandCountInt = mRasterLayer->bandCount();

  QgsDebugMsg( "entered." );

  if ( ! computeHistogram( false ) )
  {
    QgsDebugMsg( QString( "raster does not have cached histo" ) );
    stackedWidget2->setCurrentIndex( 2 );
    return;
  }

#if !defined(QWT_VERSION) || QWT_VERSION<0x060000
  mpPlot->clear();
#endif
  //ensure all children get removed
  mpPlot->setAutoDelete( true );
  mpPlot->setTitle( QObject::tr( "Raster Histogram" ) );
  mpPlot->insertLegend( new QwtLegend(), QwtPlot::BottomLegend );
  // Set axis titles
  mpPlot->setAxisTitle( QwtPlot::xBottom, QObject::tr( "Pixel Value" ) );
  mpPlot->setAxisTitle( QwtPlot::yLeft, QObject::tr( "Frequency" ) );
  mpPlot->setAxisAutoScale( QwtPlot::yLeft );
  // x axis scale only set after computing global min/max across bands (see below)
  // add a grid
  QwtPlotGrid * myGrid = new QwtPlotGrid();
  myGrid->attach( mpPlot );

  // make colors list
  mHistoColors.clear();
  mHistoColors << Qt::black; // first element, not used
  // get a list fo colors
  QVector<QColor> myColors;
  myColors << Qt::red << Qt::green << Qt::blue << Qt::magenta << Qt::darkYellow << Qt::cyan;
  while ( myColors.size() <= myBandCountInt )
  {
    myColors <<
    QColor( 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) ),
            1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) ),
            1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) ) );
  }

  // assign colors to each band, depending on the current RGB/gray band selection
  // TODO paletted + pseudo ?
  QString rendererName = mRenderTypeComboBox->itemData( mRenderTypeComboBox->currentIndex() ).toString();
  // greyscale
  if ( rendererName == "singlebandgray" )
  {
    int myGrayBand = mRendererWidget->selectedBand();
    for ( int i = 1; i <= myBandCountInt; i++ )
    {
      if ( i == myGrayBand )
        mHistoColors << Qt::black;
      else
      {
        if ( ! myColors.isEmpty() )
        {
          mHistoColors << myColors.first();
          myColors.pop_front();
        }
        else
        {
          mHistoColors << Qt::black;
        }
      }
    }
  }
  // RGB
  else if ( rendererName == "multibandcolor" )
  {
    int myRedBand = mRendererWidget->selectedBand( 0 );
    int myGreenBand = mRendererWidget->selectedBand( 1 );
    int myBlueBand = mRendererWidget->selectedBand( 2 );
    // remove RGB, which are reserved for the actual RGB bands
    // show name of RGB bands in appropriate color in bold
    myColors.remove( 0, 3 );
    for ( int i = 1; i <= myBandCountInt; i++ )
    {
      QColor myColor;
      if ( i == myRedBand )
        myColor = Qt::red;
      else if ( i == myGreenBand )
        myColor = Qt::green;
      else if ( i == myBlueBand )
        myColor = Qt::blue;
      else
      {
        if ( ! myColors.isEmpty() )
        {
          myColor = myColors.first();
          myColors.pop_front();
        }
        else
        {
          myColor = Qt::black;
        }
        cboHistoBand->setItemData( i - 1, Qt::black, Qt::ForegroundRole );
      }
      if ( i == myRedBand ||  i == myGreenBand || i == myBlueBand )
      {
        cboHistoBand->setItemData( i - 1, myColor, Qt::ForegroundRole );
      }
      mHistoColors << myColor;
    }
  }

  //
  //now draw actual graphs
  //

  //somtimes there are more bins than needed
  //we find out the last one that actually has data in it
  //so we can discard the rest and set the x-axis scales correctly
  //
  // scan through to get counts from layers' histograms
  //
  mHistoMin = 0;
  mHistoMax = 0;
  bool myFirstIteration = true;
  for ( int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt )
  {
    QgsRasterBandStats myRasterBandStats = mRasterLayer->bandStatistics( myIteratorInt );
    // mRasterLayer->populateHistogram( myIteratorInt, BINCOUNT, myIgnoreOutOfRangeFlag, myThoroughBandScanFlag );
    QwtPlotCurve * mypCurve = new QwtPlotCurve( tr( "Band %1" ).arg( myIteratorInt ) );
    mypCurve->setCurveAttribute( QwtPlotCurve::Fitted );
    mypCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
    mypCurve->setPen( QPen( mHistoColors.at( myIteratorInt ) ) );
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
    QVector<QPointF> data;
#else
    QVector<double> myX2Data;
    QVector<double> myY2Data;
#endif
    for ( int myBin = 0; myBin < BINCOUNT; myBin++ )
    {
      int myBinValue = myRasterBandStats.histogramVector->at( myBin );
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
      data << QPointF( myBin, myBinValue );
#else
      myX2Data.append( double( myBin ) );
      myY2Data.append( double( myBinValue ) );
#endif
    }
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
    mypCurve->setSamples( data );
#else
    mypCurve->setData( myX2Data, myY2Data );
#endif
    mypCurve->attach( mpPlot );
    if ( myFirstIteration || mHistoMin > myRasterBandStats.minimumValue )
    {
      mHistoMin = myRasterBandStats.minimumValue;
    }
    if ( myFirstIteration || mHistoMax < myRasterBandStats.maximumValue )
    {
      mHistoMax = myRasterBandStats.maximumValue;
    }
    QgsDebugMsg( QString( "computed histo min = %1 max = %2" ).arg( mHistoMin ).arg( mHistoMax ) );
    myFirstIteration = false;
  }
  // for x axis use band pixel values rather than gdal hist. bin values
  // subtract -0.5 to prevent rounding errors
  // see http://www.gdal.org/classGDALRasterBand.html#3f8889607d3b2294f7e0f11181c201c8
  mpPlot->setAxisScale( QwtPlot::xBottom,
                        mHistoMin - 0.5,
                        mHistoMax + 0.5 );

  mpPlot->replot();

  // histo plot markers
  // memory leak?
  mHistoMarkerMin = new QwtPlotMarker();
  mHistoMarkerMin->attach( mpPlot );
  mHistoMarkerMax = new QwtPlotMarker();
  mHistoMarkerMax->attach( mpPlot );
  updateHistoMarkers();

  // histo picker
  if ( ! mHistoPicker )
  {
    mHistoPicker = new QwtPlotPicker( mpPlot->canvas() );
    mHistoPicker->setSelectionFlags( QwtPicker::PointSelection | QwtPicker::DragSelection );
    // mHistoPicker->setTrackerMode( QwtPicker::ActiveOnly );
    mHistoPicker->setTrackerMode( QwtPicker::AlwaysOff );
    mHistoPicker->setRubberBand( QwtPicker::VLineRubberBand );
    mHistoPicker->setEnabled( false );
    connect( mHistoPicker, SIGNAL( selected( const QwtDoublePoint & ) ), this, SLOT( histoPickerSelected( const QwtDoublePoint & ) ) );
  }

  // plot zoomer
  if ( ! mHistoZoomer )
  {
    mHistoZoomer = new QwtPlotZoomer( mpPlot->canvas() );
    mHistoZoomer->setSelectionFlags( QwtPicker::RectSelection | QwtPicker::DragSelection );
    mHistoZoomer->setTrackerMode( QwtPicker::AlwaysOff );
    mHistoZoomer->setEnabled( true );
  }

  disconnect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mHistogramProgress, SLOT( setValue( int ) ) );
  // mHistogramProgress->hide();
  stackedWidget2->setCurrentIndex( 0 );
  // mpPlot->canvas()->setCursor( Qt::ArrowCursor );
  // icon from http://findicons.com/icon/169577/14_zoom?id=171427
  mpPlot->canvas()->setCursor( QCursor( QgisApp::getThemePixmap( "/mIconZoom.png" ) ) );
  on_cboHistoBand_currentIndexChanged( -1 );
  QApplication::restoreOverrideCursor();
}

void QgsRasterLayerProperties::on_mSaveAsImageButton_clicked()
{
  if ( mpPlot == 0 )
  {
    return;
  }

  QPixmap myPixmap( 600, 600 );
  myPixmap.fill( Qt::white ); // Qt::transparent ?

#if (QWT_VERSION<0x060000)
  QwtPlotPrintFilter myFilter;
  int myOptions = QwtPlotPrintFilter::PrintAll;
  myOptions &= ~QwtPlotPrintFilter::PrintBackground;
  myOptions |= QwtPlotPrintFilter::PrintFrameWithScales;
  myFilter.setOptions( myOptions );

  mpPlot->print( myPixmap, myFilter );
#else
  QPainter painter;
  painter.begin( &myPixmap );
  mpPlot->drawCanvas( &painter );
  painter.end();
#endif
  QPair< QString, QString> myFileNameAndFilter = QgisGui::getSaveAsImageName( this, tr( "Choose a file name to save the map image as" ) );
  if ( myFileNameAndFilter.first != "" )
  {
    myPixmap.save( myFileNameAndFilter.first );
  }
}
void QgsRasterLayerProperties::on_pbnImportTransparentPixelValues_clicked()
{
  int myLineCounter = 0;
  bool myImportError = false;
  QString myBadLines;
  QSettings myQSettings;
  QString myLastDir = myQSettings.value( "lastRasterFileFilterDir", "" ).toString();
  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Open file" ), myLastDir, tr( "Textfile" ) + " (*.txt)" );
  QFile myInputFile( myFileName );
  if ( myInputFile.open( QFile::ReadOnly ) )
  {
    QTextStream myInputStream( &myInputFile );
    QString myInputLine;
    if ( /*rbtnThreeBand->isChecked() &&*/ QgsRasterLayer::MultiBandColor == mRasterLayer->drawingStyle() )
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
          if ( !myInputLine.simplified().startsWith( "#" ) )
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
              tableTransparency->setItem( tableTransparency->rowCount() - 1, 0, new QTableWidgetItem( myTokens[0] ) );
              tableTransparency->setItem( tableTransparency->rowCount() - 1, 1, new QTableWidgetItem( myTokens[1] ) );
              tableTransparency->setItem( tableTransparency->rowCount() - 1, 2, new QTableWidgetItem( myTokens[2] ) );
              tableTransparency->setItem( tableTransparency->rowCount() - 1, 3, new QTableWidgetItem( myTokens[3] ) );
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
          if ( !myInputLine.simplified().startsWith( "#" ) )
          {
            QStringList myTokens = myInputLine.split( QRegExp( "\\s+" ), QString::SkipEmptyParts );
            if ( myTokens.count() != 2 )
            {
              myImportError = true;
              myBadLines = myBadLines + QString::number( myLineCounter ) + ":\t[" + myInputLine + "]\n";
            }
            else
            {
              tableTransparency->insertRow( tableTransparency->rowCount() );
              tableTransparency->setItem( tableTransparency->rowCount() - 1, 0, new QTableWidgetItem( myTokens[0] ) );
              tableTransparency->setItem( tableTransparency->rowCount() - 1, 1, new QTableWidgetItem( myTokens[1] ) );
            }
          }
        }
      }
    }

    if ( myImportError )
    {
      QMessageBox::warning( this, tr( "Import Error" ), tr( "The following lines contained errors\n\n%1" ).arg( myBadLines ) );
    }
  }
  else if ( !myFileName.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Read access denied" ), tr( "Read access denied. Adjust the file permissions and try again.\n\n" ) );
  }
}

void QgsRasterLayerProperties::on_pbnRemoveSelectedRow_clicked()
{
  if ( 0 < tableTransparency->rowCount() )
  {
    tableTransparency->removeRow( tableTransparency->currentRow() );
  }
}

void QgsRasterLayerProperties::pixelSelected( const QgsPoint& canvasPoint )
{
  QgsRasterRenderer* renderer = mRendererWidget->renderer();
  if ( !renderer )
  {
    return;
  }

  raise();
  setModal( true );
  activateWindow();

  //Get the pixel values and add a new entry to the transparency table
  if ( mMapCanvas && mPixelSelectorTool )
  {
    QMap< int, QString > myPixelMap;
    mMapCanvas->unsetMapTool( mPixelSelectorTool );
    mRasterLayer->identify( mMapCanvas->mapRenderer()->mapToLayerCoordinates( mRasterLayer, canvasPoint ), myPixelMap );

    QList<int> bands = renderer->usesBands();
    tableTransparency->insertRow( tableTransparency->rowCount() );
    tableTransparency->setItem( tableTransparency->rowCount() - 1, tableTransparency->columnCount() - 1, new QTableWidgetItem( "100.0" ) );

    for ( int i = 0; i < bands.size(); ++i )
    {
      QMap< int, QString >::const_iterator pixelResult = myPixelMap.find( bands.at( i ) );
      if ( pixelResult != myPixelMap.constEnd() )
      {
        QString value = pixelResult.value();
        if ( value != tr( "out of extent" ) )
        {
          tableTransparency->setItem( tableTransparency->rowCount() - 1, i, new QTableWidgetItem( value ) );
        }
      }
    }
  }

  delete renderer;
}

void QgsRasterLayerProperties::sliderTransparency_valueChanged( int theValue )
{
  //set the transparency percentage label to a suitable value
  int myInt = static_cast < int >(( theValue / 255.0 ) * 100 );  //255.0 to prevent integer division
  lblTransparencyPercent->setText( QString::number( myInt ) + "%" );
}//sliderTransparency_valueChanged


QLinearGradient QgsRasterLayerProperties::redGradient()
{
  //define a gradient
  ///@TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient( mGradientWidth, 0, mGradientWidth, mGradientHeight );
  myGradient.setColorAt( 0.0, QColor( 242, 14, 25, 190 ) );
  myGradient.setColorAt( 0.5, QColor( 175, 29, 37, 190 ) );
  myGradient.setColorAt( 1.0, QColor( 114, 17, 22, 190 ) );
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::greenGradient()
{
  //define a gradient
  ///@TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient( mGradientWidth, 0, mGradientWidth, mGradientHeight );
  myGradient.setColorAt( 0.0, QColor( 48, 168, 5, 190 ) );
  myGradient.setColorAt( 0.8, QColor( 36, 122, 4, 190 ) );
  myGradient.setColorAt( 1.0, QColor( 21, 71, 2, 190 ) );
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::blueGradient()
{
  //define a gradient
  ///@TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient( mGradientWidth, 0, mGradientWidth, mGradientHeight );
  myGradient.setColorAt( 0.0, QColor( 30, 0, 106, 190 ) );
  myGradient.setColorAt( 0.2, QColor( 30, 72, 128, 190 ) );
  myGradient.setColorAt( 1.0, QColor( 30, 223, 196, 190 ) );
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::grayGradient()
{
  //define a gradient
  ///@TODO change this to actual polygon dims
  QLinearGradient myGradient = QLinearGradient( mGradientWidth, 0, mGradientWidth, mGradientHeight );
  myGradient.setColorAt( 0.0, QColor( 5, 5, 5, 190 ) );
  myGradient.setColorAt( 0.8, QColor( 122, 122, 122, 190 ) );
  myGradient.setColorAt( 1.0, QColor( 220, 220, 220, 190 ) );
  return myGradient;
}
QLinearGradient QgsRasterLayerProperties::highlightGradient()
{
  //define another gradient for the highlight
  ///@TODO change this to actual polygon dims
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
void QgsRasterLayerProperties::on_pbnLoadDefaultStyle_clicked()
{
  bool defaultLoadedFlag = false;
  QString myMessage = mRasterLayer->loadDefaultStyle( defaultLoadedFlag );
  //reset if the default style was loaded ok only
  if ( defaultLoadedFlag )
  {
    QgsRasterRenderer* renderer = mRasterLayer->renderer();
    if ( renderer )
    {
      setRendererWidget( renderer->type() );
    }
    mRasterLayer->triggerRepaint();
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

void QgsRasterLayerProperties::on_pbnSaveDefaultStyle_clicked()
{
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


void QgsRasterLayerProperties::on_pbnLoadStyle_clicked()
{
  QSettings settings;
  QString lastUsedDir = settings.value( "style/lastStyleDir", "." ).toString();

  QString fileName = QFileDialog::getOpenFileName(
                       this,
                       tr( "Load layer properties from style file" ),
                       lastUsedDir,
                       tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( fileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  if ( !fileName.endsWith( ".qml", Qt::CaseInsensitive ) )
    fileName += ".qml";

  bool defaultLoadedFlag = false;
  QString message = mRasterLayer->loadNamedStyle( fileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
  {
    settings.setValue( "style/lastStyleDir", QFileInfo( fileName ).absolutePath() );
    QgsRasterRenderer* renderer = mRasterLayer->renderer();
    if ( renderer )
    {
      setRendererWidget( renderer->type() );
    }
    mRasterLayer->triggerRepaint();
  }
  else
  {
    QMessageBox::information( this, tr( "Saved Style" ), message );
  }
}


void QgsRasterLayerProperties::on_pbnSaveStyleAs_clicked()
{
  QSettings settings;
  QString lastUsedDir = settings.value( "style/lastStyleDir", "." ).toString();

  QString outputFileName = QFileDialog::getSaveFileName(
                             this,
                             tr( "Save layer properties as style file" ),
                             lastUsedDir,
                             tr( "QGIS Layer Style File" ) + " (*.qml)" );
  if ( outputFileName.isEmpty() )
    return;

  // ensure the user never omits the extension from the file name
  if ( !outputFileName.endsWith( ".qml", Qt::CaseInsensitive ) )
    outputFileName += ".qml";

  bool defaultLoadedFlag = false;
  QString message = mRasterLayer->saveNamedStyle( outputFileName, defaultLoadedFlag );
  if ( defaultLoadedFlag )
  {
    sync();
  }
  else
  {
    QMessageBox::information( this, tr( "Saved Style" ), message );
  }

  settings.setValue( "style/lastStyleDir", QFileInfo( outputFileName ).absolutePath() );
}

void QgsRasterLayerProperties::on_btnResetNull_clicked( )
{
  //If reset NoDataValue is checked do this first, will ignore what ever is in the LineEdit
  mRasterLayer->resetNoDataValue();
  if ( mRasterLayer->isNoDataValueValid() )
  {
    leNoDataValue->setText( QString::number( mRasterLayer->noDataValue(), 'g' ) );
  }
  else
  {
    leNoDataValue->clear();
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

void QgsRasterLayerProperties::on_cboHistoBand_currentIndexChanged( int index )
{
  // get the current index value, index can be -1
  index = cboHistoBand->currentIndex();
  if ( mHistoPicker != NULL )
  {
    mHistoPicker->setEnabled( false );
    mHistoPicker->setRubberBandPen( QPen( mHistoColors.at( index + 1 ) ) );
  }
  if ( mHistoZoomer != NULL )
    mHistoZoomer->setEnabled( true );
  btnHistoMin->setEnabled( true );
  btnHistoMax->setEnabled( true );

  int theBandNo = index + 1;
  QString minStr, maxStr;
  // TODO - there are 2 definitions of raster data type that should be unified
  // QgsRasterDataProvider::DataType and QgsContrastEnhancement::QgsRasterDataType
  // TODO - fix gdal provider: changes data type when nodata value is not found
  // this prevents us from getting proper min and max values here
  // minStr = QString::number( QgsContrastEnhancement::minimumValuePossible( ( QgsContrastEnhancement::QgsRasterDataType )
  //                                                                         mRasterLayer->dataProvider()->dataType( theBandNo ) ) );
  // maxStr = QString::number( QgsContrastEnhancement::maximumValuePossible( ( QgsContrastEnhancement::QgsRasterDataType )
  //                                                                         mRasterLayer->dataProvider()->dataType( theBandNo ) ) );
  QString rendererName = mRenderTypeComboBox->itemData( mRenderTypeComboBox->currentIndex() ).toString();

  // TODO paletted + pseudo ?
  if ( rendererName == "singlebandgray" )
  {
    if ( theBandNo == mRendererWidget->selectedBand() )
    {
      minStr = mRendererWidget->min();
      maxStr = mRendererWidget->max();
    }
  }
  else if ( rendererName == "multibandcolor" )
  {
    for ( int i = 0; i <= 2; i++ )
    {
      if ( theBandNo == mRendererWidget->selectedBand( i ) )
      {
        minStr = mRendererWidget->min( i );
        maxStr = mRendererWidget->max( i );
      }
    }
  }

  leHistoMin->setText( minStr );
  leHistoMax->setText( maxStr );
  applyHistoMin();
  applyHistoMax();
}

void QgsRasterLayerProperties::histoActionTriggered( QAction* action )
{
  if ( ! action )
    return;

  // this approach is a bit of a hack, but we don't have to define slots for each action
  QString actionName = action->data().toString();

  QgsDebugMsg( QString( "band = %1 action = %2" ).arg( cboHistoBand->currentIndex() + 1 ).arg( actionName ) );

  if ( actionName.left( 5 ) == "Load " )
  {
    if ( actionName == "Load apply all" )
      return;

    QgsRasterBandStats myRasterBandStats;
    QVector<int> myBands;
    double minMaxValues[2];
    bool ok = false;

    // get "Load apply all" value to find which band(s) need updating
    QList< QAction* > actions;
    if ( action->actionGroup() )
      actions = action->actionGroup()->actions();
    foreach( QAction* tmpAction, actions )
    {
      if ( tmpAction && ( tmpAction->data().toString() == "Load apply all" ) )
      {
        // add all bands
        if ( tmpAction->isChecked() )
        {
          int myBandCountInt = mRasterLayer->bandCount();
          for ( int myIteratorInt = 1;
                myIteratorInt <= myBandCountInt;
                ++myIteratorInt )
          {
            if ( myIteratorInt != cboHistoBand->currentIndex() + 1 )
              myBands << myIteratorInt;
          }
        }
        // add current band to the end
        myBands << cboHistoBand->currentIndex() + 1;
        break;
      }
    }
    // don't update markers every time
    if ( myBands.size() > 1 )
    {
      leHistoMin->blockSignals( true );
      leHistoMax->blockSignals( true );
    }
    foreach( int theBandNo, myBands )
    {
      ok = false;
      if ( actionName == "Load actual" )
      {
        ok = mRendererWidget->bandMinMax( QgsRasterRendererWidget::Actual,
                                          theBandNo, minMaxValues );
      }
      else if ( actionName == "Load estimate" )
      {
        ok = mRendererWidget->bandMinMax( QgsRasterRendererWidget::Estimate,
                                          theBandNo, minMaxValues );
      }
      else if ( actionName == "Load extent" )
      {
        ok = mRendererWidget->bandMinMax( QgsRasterRendererWidget::CurrentExtent,
                                          theBandNo, minMaxValues );
      }
      else if ( actionName == "Load 1 stddev" )
      {
        double myStdDev = 1.0;
        ok = mRendererWidget->bandMinMaxFromStdDev( myStdDev, theBandNo, minMaxValues );
      }

      // apply current item
      cboHistoBand->setCurrentIndex( theBandNo - 1 );
      if ( !ok || actionName == "Load reset" )
      {
        leHistoMin->clear();
        leHistoMax->clear();
        // TODO - fix gdal provider: changes data type when nodata value is not found
        // this prevents us from getting proper min and max values here
        // minMaxValues[0] = QgsContrastEnhancement::minimumValuePossible( ( QgsContrastEnhancement::QgsRasterDataType )
        //                                                                 mRasterLayer->dataProvider()->dataType( theBandNo ) );
        // minMaxValues[1] = QgsContrastEnhancement::maximumValuePossible( ( QgsContrastEnhancement::QgsRasterDataType )
        //                                                                 mRasterLayer->dataProvider()->dataType( theBandNo ) );
      }
      else
      {
        leHistoMin->setText( QString::number( minMaxValues[0] ) );
        leHistoMax->setText( QString::number( minMaxValues[1] ) );
      }
      applyHistoMin( );
      applyHistoMax( );
    }
    // update markers
    if ( myBands.size() > 1 )
    {
      leHistoMin->blockSignals( false );
      leHistoMax->blockSignals( false );
      updateHistoMarkers();
    }
  }
  else if ( actionName == "Compute Histogram" )
  {
    on_btnHistoCompute_clicked();
  }
  else
  {
    return;
  }
}

void QgsRasterLayerProperties::applyHistoMin( )
{
  int theBandNo = cboHistoBand->currentIndex() + 1;
  QString rendererName = mRenderTypeComboBox->itemData( mRenderTypeComboBox->currentIndex() ).toString();

  // TMP ET TODO - paletted + pseudo
  if ( rendererName.startsWith( "singlebandgray" ) )
  {
    if ( theBandNo == mRendererWidget->selectedBand( ) )
    {
      mRendererWidget->setMin( leHistoMin->text() );
    }
  }
  else if ( rendererName == "multibandcolor" )
  {
    for ( int i = 0; i <= 2; i++ )
    {
      if ( theBandNo == mRendererWidget->selectedBand( i ) )
        mRendererWidget->setMin( leHistoMin->text(), i );
    }
  }

  updateHistoMarkers();
}

void QgsRasterLayerProperties::applyHistoMax( )
{
  int theBandNo = cboHistoBand->currentIndex() + 1;
  QString rendererName = mRenderTypeComboBox->itemData( mRenderTypeComboBox->currentIndex() ).toString();

  if ( rendererName.startsWith( "singleband" ) )
  {
    if ( theBandNo == mRendererWidget->selectedBand( ) )
    {
      mRendererWidget->setMax( leHistoMax->text() );
    }
  }
  else if ( rendererName == "multibandcolor" )
  {
    for ( int i = 0; i <= 2; i++ )
    {
      if ( theBandNo == mRendererWidget->selectedBand( i ) )
        mRendererWidget->setMax( leHistoMax->text(), i );
    }
  }

  updateHistoMarkers();
}

void QgsRasterLayerProperties::on_btnHistoMin_toggled()
{
  if ( mpPlot != NULL && mHistoPicker != NULL )
  {
    if ( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    if ( btnHistoMin->isChecked() )
    {
      btnHistoMax->setChecked( false );
      QApplication::setOverrideCursor( Qt::PointingHandCursor );
    }
    if ( mHistoZoomer != NULL )
      mHistoZoomer->setEnabled( ! btnHistoMax->isChecked() );
    mHistoPicker->setEnabled( btnHistoMin->isChecked() );
  }
}

void QgsRasterLayerProperties::on_btnHistoMax_toggled()
{
  if ( mpPlot != NULL && mHistoPicker != NULL )
  {
    if ( QApplication::overrideCursor() )
      QApplication::restoreOverrideCursor();
    if ( btnHistoMax->isChecked() )
    {
      btnHistoMin->setChecked( false );
      QApplication::setOverrideCursor( Qt::PointingHandCursor );
    }
    if ( mHistoZoomer != NULL )
      mHistoZoomer->setEnabled( ! btnHistoMax->isChecked() );
    mHistoPicker->setEnabled( btnHistoMax->isChecked() );
  }
}

// local function used by histoPickerSelected(), to get a rounded picked value
// this is sensitive and may not always be correct, needs more testing
QString findClosestTickVal( double target, QwtScaleDiv * scale, int div = 100 )
{
  if ( scale == NULL ) return "";

  QList< double > minorTicks = scale->ticks( QwtScaleDiv::MinorTick );
  QList< double > majorTicks = scale->ticks( QwtScaleDiv::MajorTick );
  double diff = ( minorTicks[1] - minorTicks[0] ) / div;
  double min = majorTicks[0] - diff;
  if ( min > target )
    min -= ( majorTicks[1] - majorTicks[0] );
  double max = scale->upperBound();
  double closest = target;
  double current = min;

  while ( current < max )
  {
    current += diff;
    if ( current > target )
    {
      closest = ( abs( target - current + diff ) < abs( target - current ) ) ? current - diff : current;
      break;
    }
  }

  QgsDebugMsg( QString( "target=%1 div=%2 closest=%3" ).arg( target ).arg( div ).arg( closest ) );
  return QString::number( closest );
}

void QgsRasterLayerProperties::histoPickerSelected( const QwtDoublePoint & pos )
{
  if ( btnHistoMin->isChecked() )
  {
    leHistoMin->setText( findClosestTickVal( pos.x(), mpPlot->axisScaleDiv( QwtPlot::xBottom ) ) );
    applyHistoMin();
    btnHistoMin->setChecked( false );
  }
  else if ( btnHistoMax->isChecked() )
  {
    leHistoMax->setText( findClosestTickVal( pos.x(), mpPlot->axisScaleDiv( QwtPlot::xBottom ) ) );
    applyHistoMax();
    btnHistoMax->setChecked( false );
  }
  if ( QApplication::overrideCursor() )
    QApplication::restoreOverrideCursor();

}

void QgsRasterLayerProperties::updateHistoMarkers( )
{
  // hack to not update markers
  if ( leHistoMin->signalsBlocked() )
    return;
  // todo error checking
  if ( mpPlot == NULL || mHistoMarkerMin == NULL || mHistoMarkerMax == NULL )
    return;

  if ( ! cbxHistoShow->isChecked() )
  {
    mHistoMarkerMin->hide();
    mHistoMarkerMax->hide();
    mpPlot->replot();
    return;
  }

  int theBandNo = cboHistoBand->currentIndex() + 1;
  double minVal = mHistoMin;
  double maxVal = mHistoMax;
  QString minStr = leHistoMin->text();
  QString maxStr = leHistoMax->text();
  if ( minStr != "" )
    minVal = minStr.toDouble();
  if ( maxStr != "" )
    maxVal = maxStr.toDouble();

  QPen linePen = QPen( mHistoColors.at( theBandNo ) );
  linePen.setStyle( Qt::DashLine );
  mHistoMarkerMin->setLineStyle( QwtPlotMarker::VLine );
  mHistoMarkerMin->setLinePen( linePen );
  mHistoMarkerMin->setXValue( minVal );
  mHistoMarkerMin->show();
  mHistoMarkerMax->setLineStyle( QwtPlotMarker::VLine );
  mHistoMarkerMax->setLinePen( linePen );
  mHistoMarkerMax->setXValue( maxVal );
  mHistoMarkerMax->show();

  mpPlot->replot();

}

