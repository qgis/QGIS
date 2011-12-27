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

QgsRasterLayerProperties::QgsRasterLayerProperties( QgsMapLayer* lyr, QgsMapCanvas* theCanvas, QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
    // Constant that signals property not used.
    TRSTRING_NOT_SET( tr( "Not Set" ) ),
    mRasterLayer( qobject_cast<QgsRasterLayer *>( lyr ) )
{
  ignoreSpinBoxEvent = false; //Short circuit signal loop between min max field and stdDev spin box
  mGrayMinimumMaximumEstimated = true;
  mRGBMinimumMaximumEstimated = true;

  setupUi( this );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( apply() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

  connect( sliderTransparency, SIGNAL( valueChanged( int ) ), this, SLOT( sliderTransparency_valueChanged( int ) ) );

  //clear either stdDev or min max entries depending which is changed
  connect( sboxSingleBandStdDev, SIGNAL( valueChanged( double ) ), this, SLOT( sboxSingleBandStdDev_valueChanged( double ) ) );
  connect( sboxThreeBandStdDev, SIGNAL( valueChanged( double ) ), this, SLOT( sboxThreeBandStdDev_valueChanged( double ) ) );
  connect( leGrayMin, SIGNAL( textEdited( QString ) ), this, SLOT( userDefinedMinMax_textEdited( QString ) ) );
  connect( leGrayMax, SIGNAL( textEdited( QString ) ), this, SLOT( userDefinedMinMax_textEdited( QString ) ) );
  connect( leRedMin, SIGNAL( textEdited( QString ) ), this, SLOT( userDefinedMinMax_textEdited( QString ) ) );
  connect( leRedMax, SIGNAL( textEdited( QString ) ), this, SLOT( userDefinedMinMax_textEdited( QString ) ) );
  connect( leGreenMin, SIGNAL( textEdited( QString ) ), this, SLOT( userDefinedMinMax_textEdited( QString ) ) );
  connect( leGreenMax, SIGNAL( textEdited( QString ) ), this, SLOT( userDefinedMinMax_textEdited( QString ) ) );
  connect( leBlueMin, SIGNAL( textEdited( QString ) ), this, SLOT( userDefinedMinMax_textEdited( QString ) ) );
  connect( leBlueMax, SIGNAL( textEdited( QString ) ), this, SLOT( userDefinedMinMax_textEdited( QString ) ) );
  connect( mColormapTreeWidget, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( handleColormapTreeWidgetDoubleClick( QTreeWidgetItem*, int ) ) );
  // enable or disable Build Pyramids button depending on selection in pyramid list
  connect( lbxPyramidResolutions, SIGNAL( itemSelectionChanged() ), this, SLOT( toggleBuildPyramidsButton() ) );

  // set up the scale based layer visibility stuff....
  chkUseScaleDependentRendering->setChecked( lyr->hasScaleBasedVisibility() );
  leMinimumScale->setText( QString::number( lyr->minimumScale(), 'f' ) );
  leMinimumScale->setValidator( new QDoubleValidator( 0, std::numeric_limits<float>::max(), 1000, this ) );
  leMaximumScale->setText( QString::number( lyr->maximumScale(), 'f' ) );
  leMaximumScale->setValidator( new QDoubleValidator( 0, std::numeric_limits<float>::max(), 1000, this ) );
  leNoDataValue->setValidator( new QDoubleValidator( -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 1000, this ) );

  leRedMin->setValidator( new QDoubleValidator( this ) );
  leRedMax->setValidator( new QDoubleValidator( this ) );
  leBlueMin->setValidator( new QDoubleValidator( this ) );
  leBlueMax->setValidator( new QDoubleValidator( this ) );
  leGreenMin->setValidator( new QDoubleValidator( this ) );
  leGreenMax->setValidator( new QDoubleValidator( this ) );
  leGrayMin->setValidator( new QDoubleValidator( this ) );
  leGrayMax->setValidator( new QDoubleValidator( this ) );

  // build GUI components
  cboxColorMap->addItem( tr( "Grayscale" ) );
  cboxColorMap->addItem( tr( "Pseudocolor" ) );
  cboxColorMap->addItem( tr( "Freak Out" ) );
  cboxColorMap->addItem( tr( "Colormap" ) );

  //add items to the color enhanceContrast combo box
  cboxContrastEnhancementAlgorithm->addItem( tr( "No Stretch" ) );
  cboxContrastEnhancementAlgorithm->addItem( tr( "Stretch To MinMax" ) );
  cboxContrastEnhancementAlgorithm->addItem( tr( "Stretch And Clip To MinMax" ) );
  cboxContrastEnhancementAlgorithm->addItem( tr( "Clip To MinMax" ) );

  //set initial states of all Min Max and StdDev fields and labels to disabled
  sboxThreeBandStdDev->setEnabled( false );
  sboxSingleBandStdDev->setEnabled( false );
  lblGrayMin->setEnabled( false );
  leGrayMin->setEnabled( false );
  lblGrayMax->setEnabled( false );
  leGrayMax->setEnabled( false );
  lblRedMin->setEnabled( false );
  leRedMin->setEnabled( false );
  lblRedMax->setEnabled( false );
  leRedMax->setEnabled( false );
  lblGreenMin->setEnabled( false );
  leGreenMin->setEnabled( false );
  lblGreenMax->setEnabled( false );
  leGreenMax->setEnabled( false );
  lblBlueMin->setEnabled( false );
  leBlueMin->setEnabled( false );
  lblBlueMax->setEnabled( false );
  leBlueMax->setEnabled( false );

  pbtnLoadMinMax->setEnabled( false );

  //setup custom colormap tab
  cboxColorInterpolation->addItem( tr( "Discrete" ) );
  cboxColorInterpolation->addItem( tr( "Linear" ) );
  cboxColorInterpolation->addItem( tr( "Exact" ) );
  cboxClassificationMode->addItem( tr( "Equal interval" ) );
  //cboxClassificationMode->addItem( tr( "Quantiles" ) );

  QStringList headerLabels;
  headerLabels << tr( "Value" );
  headerLabels << tr( "Color" );
  headerLabels << tr( "Label" );
  mColormapTreeWidget->setHeaderLabels( headerLabels );

  //disable colormap tab completely until 'Colormap' is selected (and only for type GrayOrUndefined)
  tabPageColormap->setEnabled( false );

  //
  // Set up the combo boxes that contain band lists using the qstring list generated above
  //

  QgsDebugMsg( "Populating band combo boxes" );

  int myBandCountInt = mRasterLayer->bandCount();
  for ( int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt )
  {
    QString myRasterBandName = mRasterLayer->bandName( myIteratorInt ) ;
    cboGray->addItem( myRasterBandName );
    cboRed->addItem( myRasterBandName );
    cboGreen->addItem( myRasterBandName );
    cboBlue->addItem( myRasterBandName );
    cboxColorMapBand->addItem( myRasterBandName );
    cboxTransparencyBand->addItem( myRasterBandName );
    cboxTransparencyBand->setEnabled( true );
  }

  cboRed->addItem( TRSTRING_NOT_SET );
  cboGreen->addItem( TRSTRING_NOT_SET );
  cboBlue->addItem( TRSTRING_NOT_SET );
  cboGray->addItem( TRSTRING_NOT_SET );
  cboxTransparencyBand->addItem( TRSTRING_NOT_SET );

  QIcon myPyramidPixmap( QgisApp::getThemeIcon( "/mIconPyramid.png" ) );
  QIcon myNoPyramidPixmap( QgisApp::getThemeIcon( "/mIconNoPyramid.png" ) );

  pbnAddValuesManually->setIcon( QgisApp::getThemeIcon( "/mActionNewAttribute.png" ) );
  pbnAddValuesFromDisplay->setIcon( QgisApp::getThemeIcon( "/mActionContextHelp.png" ) );
  pbnRemoveSelectedRow->setIcon( QgisApp::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  pbnDefaultValues->setIcon( QgisApp::getThemeIcon( "/mActionCopySelected.png" ) );
  pbnImportTransparentPixelValues->setIcon( QgisApp::getThemeIcon( "/mActionFileOpen.png" ) );
  pbnExportTransparentPixelValues->setIcon( QgisApp::getThemeIcon( "/mActionFileSave.png" ) );
  pbtnMakeBandCombinationDefault->setIcon( QgisApp::getThemeIcon( "/mActionFileSave.png" ) );
  pbtnMakeStandardDeviationDefault->setIcon( QgisApp::getThemeIcon( "/mActionFileSave.png" ) );
  pbtnMakeContrastEnhancementAlgorithmDefault->setIcon( QgisApp::getThemeIcon( "/mActionFileSave.png" ) );

  pbtnLoadColorMapFromBand->setIcon( QgisApp::getThemeIcon( "/mActionNewAttribute.png" ) );
  pbtnExportColorMapToFile->setIcon( QgisApp::getThemeIcon( "/mActionFileSave.png" ) );
  pbtnLoadColorMapFromFile->setIcon( QgisApp::getThemeIcon( "/mActionFileOpen.png" ) );

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


  // Only do pyramids if dealing directly with GDAL.
  if ( mRasterLayer->dataProvider()->capabilities() & QgsRasterDataProvider::BuildPyramids )
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

  // update based on lyr's current state
  sync();

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/RasterLayerProperties/geometry" ).toByteArray() );
  tabBar->setCurrentIndex( settings.value( "/Windows/RasterLayerProperties/row" ).toInt() );

  setWindowTitle( tr( "Layer Properties - %1" ).arg( lyr->name() ) );
  int myHistogramTab = 6;
  if ( tabBar->currentIndex() == myHistogramTab )
  {
    refreshHistogram();
  }
  tableTransparency->horizontalHeader()->setResizeMode( 0, QHeaderView::Stretch );
  tableTransparency->horizontalHeader()->setResizeMode( 1, QHeaderView::Stretch );

  //resampling
  const QgsRasterResampler* resampler = mRasterLayer->resampler();
  if ( !resampler )
  {
    mNearestNeighbourRadioButton->setChecked( true );
  }
  else if ( resampler->type() == "bilinear" )
  {
    mBilinearRadioButton->setChecked( true );
  }
  else if ( resampler->type() == "cubic" )
  {
    mCubicRadioButton->setChecked( true );
  }

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

/*
 *
 * PUBLIC METHODS
 *
 */
void QgsRasterLayerProperties::populateColorMapTable( const QList<QgsColorRampShader::ColorRampItem>& theColorRampList )
{
  if ( theColorRampList.size() > 0 )
  {
    mColormapTreeWidget->clear();
    QList<QgsColorRampShader::ColorRampItem>::const_iterator it;
    for ( it = theColorRampList.begin(); it != theColorRampList.end(); ++it )
    {
      QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
      newItem->setText( 0, QString::number( it->value, 'f' ) );
      newItem->setBackground( 1, QBrush( it->color ) );
      newItem->setText( 2, it->label );
    }
  }
}
void QgsRasterLayerProperties::populateTransparencyTable()
{
  QgsDebugMsg( "entering." );

  //Clear existing color transparency list
  //NOTE: May want to just tableTransparency->clearContents() and fill back in after checking to be sure list and table are the same size
  QString myNumberFormatter;
  if ( rbtnThreeBand->isChecked() &&
       QgsRasterLayer::PalettedColor != mRasterLayer->drawingStyle() &&
       QgsRasterLayer::PalettedMultiBandColor != mRasterLayer->drawingStyle() )
  {
    for ( int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >= 0; myTableRunner-- )
    {
      tableTransparency->removeRow( myTableRunner );
    }

    tableTransparency->clear();
    tableTransparency->setColumnCount( 4 );
    tableTransparency->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Red" ) ) );
    tableTransparency->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Green" ) ) );
    tableTransparency->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Blue" ) ) );
    tableTransparency->setHorizontalHeaderItem( 3, new QTableWidgetItem( tr( "Percent Transparent" ) ) );

    //populate three band transparency list
    QList<QgsRasterTransparency::TransparentThreeValuePixel> myTransparentThreeValuePixelList = mRasterLayer->rasterTransparency()->transparentThreeValuePixelList();
    for ( int myListRunner = 0; myListRunner < myTransparentThreeValuePixelList.count(); myListRunner++ )
    {
      tableTransparency->insertRow( myListRunner );
      QTableWidgetItem* myRedItem = new QTableWidgetItem( myNumberFormatter.sprintf( "%.2f", myTransparentThreeValuePixelList[myListRunner].red ) );
      QTableWidgetItem* myGreenItem = new QTableWidgetItem( myNumberFormatter.sprintf( "%.2f", myTransparentThreeValuePixelList[myListRunner].green ) );
      QTableWidgetItem* myBlueItem = new QTableWidgetItem( myNumberFormatter.sprintf( "%.2f", myTransparentThreeValuePixelList[myListRunner].blue ) );
      QTableWidgetItem* myPercentTransparentItem = new QTableWidgetItem( myNumberFormatter.sprintf( "%.2f", myTransparentThreeValuePixelList[myListRunner].percentTransparent ) );

      tableTransparency->setItem( myListRunner, 0, myRedItem );
      tableTransparency->setItem( myListRunner, 1, myGreenItem );
      tableTransparency->setItem( myListRunner, 2, myBlueItem );
      tableTransparency->setItem( myListRunner, 3, myPercentTransparentItem );
    }
  }
  else
  {
    //Clear existing single band or palette values transparency list
    for ( int myTableRunner = tableTransparency->rowCount() - 1; myTableRunner >= 0; myTableRunner-- )
    {
      tableTransparency->removeRow( myTableRunner );
    }

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

    //populate gray transparency list
    QList<QgsRasterTransparency::TransparentSingleValuePixel> myTransparentSingleValuePixelList = mRasterLayer->rasterTransparency()->transparentSingleValuePixelList();
    for ( int myListRunner = 0; myListRunner < myTransparentSingleValuePixelList.count(); myListRunner++ )
    {
      tableTransparency->insertRow( myListRunner );
      QTableWidgetItem* myGrayItem = new QTableWidgetItem( myNumberFormatter.sprintf( "%.2f", myTransparentSingleValuePixelList[myListRunner].pixelValue ) );
      QTableWidgetItem* myPercentTransparentItem = new QTableWidgetItem( myNumberFormatter.sprintf( "%.2f", myTransparentSingleValuePixelList[myListRunner].percentTransparent ) );

      tableTransparency->setItem( myListRunner, 0, myGrayItem );
      tableTransparency->setItem( myListRunner, 1, myPercentTransparentItem );
    }
  }

  tableTransparency->resizeColumnsToContents();
  tableTransparency->resizeRowsToContents();
}

// Set the message indicating if any min max values are estimates
void QgsRasterLayerProperties::setMinimumMaximumEstimateWarning()
{
  bool myEstimatedValues = false;
  if ( rbtnThreeBand->isChecked() )
  {
    myEstimatedValues = mRGBMinimumMaximumEstimated;
  }
  else
  {
    myEstimatedValues = mGrayMinimumMaximumEstimated;
  }

  if ( myEstimatedValues )
  {
    lblMinMaxEstimateWarning->setText( tr( "Note: Minimum Maximum values are estimates, user defined, or calculated from the current extent" ) );
  }
  else
  {
    lblMinMaxEstimateWarning->setText( tr( "Note: Minimum Maximum values are actual values computed from the band(s)" ) );
  }
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
  QgsDebugMsg( "sync populate symbology tab" );
  /*
   * Symbology Tab
   */
  //decide whether user can change rgb settings
  QgsDebugMsg( "DrawingStyle = " + QString::number( mRasterLayer->drawingStyle() ) );
  switch ( mRasterLayer->drawingStyle() )
  {
    case QgsRasterLayer::SingleBandGray:
      rbtnThreeBand->setEnabled( false );
      rbtnSingleBand->setEnabled( true );
      rbtnSingleBand->setChecked( true );
      pbtnLoadMinMax->setEnabled( true );
      cboxContrastEnhancementAlgorithm->setEnabled( true );
      labelContrastEnhancement->setEnabled( true );
      break;
    case QgsRasterLayer::SingleBandPseudoColor:
      rbtnThreeBand->setEnabled( false );
      rbtnSingleBand->setEnabled( true );
      rbtnSingleBand->setChecked( true );
      pbtnLoadMinMax->setEnabled( false );
      cboxContrastEnhancementAlgorithm->setEnabled( false );
      labelContrastEnhancement->setEnabled( false );
      break;
    case QgsRasterLayer::PalettedColor:
      rbtnThreeBand->setEnabled( false );
      rbtnSingleBand->setEnabled( true );
      rbtnSingleBand->setChecked( true );
      rbtnThreeBandMinMax->setEnabled( false );
      rbtnThreeBandStdDev->setEnabled( false );
      pbtnLoadMinMax->setEnabled( false );
      cboxContrastEnhancementAlgorithm->setEnabled( false );
      labelContrastEnhancement->setEnabled( false );
      break;
    case QgsRasterLayer::PalettedSingleBandGray:
      rbtnThreeBand->setEnabled( true );
      rbtnSingleBand->setEnabled( true );
      rbtnSingleBand->setChecked( true );
      rbtnThreeBandMinMax->setEnabled( false );
      rbtnThreeBandStdDev->setEnabled( false );
      pbtnLoadMinMax->setEnabled( false );
      cboxContrastEnhancementAlgorithm->setEnabled( false );
      labelContrastEnhancement->setEnabled( false );
      break;
    case QgsRasterLayer::PalettedSingleBandPseudoColor:
      rbtnThreeBand->setEnabled( false );
      rbtnSingleBand->setEnabled( true );
      rbtnSingleBand->setChecked( true );
      rbtnThreeBandMinMax->setEnabled( false );
      rbtnThreeBandStdDev->setEnabled( false );
      pbtnLoadMinMax->setEnabled( false );
      cboxContrastEnhancementAlgorithm->setEnabled( false );
      labelContrastEnhancement->setEnabled( false );
      break;
    case QgsRasterLayer::PalettedMultiBandColor:
      rbtnThreeBand->setEnabled( true );
      rbtnSingleBand->setEnabled( true );
      rbtnThreeBand->setChecked( true );
      rbtnThreeBandMinMax->setEnabled( false );
      rbtnThreeBandStdDev->setEnabled( false );
      pbtnLoadMinMax->setEnabled( false );
      cboxContrastEnhancementAlgorithm->setEnabled( false );
      labelContrastEnhancement->setEnabled( false );
      break;
    case QgsRasterLayer::MultiBandSingleBandGray:
      rbtnThreeBand->setEnabled( true );
      rbtnSingleBand->setEnabled( true );
      rbtnSingleBand->setChecked( true );
      pbtnLoadMinMax->setEnabled( true );
      cboxContrastEnhancementAlgorithm->setEnabled( true );
      labelContrastEnhancement->setEnabled( true );
      break;
    case QgsRasterLayer::MultiBandSingleBandPseudoColor:
      rbtnThreeBand->setEnabled( true );
      rbtnSingleBand->setEnabled( true );
      rbtnSingleBand->setChecked( true );
      pbtnLoadMinMax->setEnabled( false );
      cboxContrastEnhancementAlgorithm->setEnabled( false );
      labelContrastEnhancement->setEnabled( false );
      break;
    case QgsRasterLayer::MultiBandColor:
      rbtnThreeBand->setEnabled( true );
      rbtnSingleBand->setEnabled( true );
      rbtnThreeBand->setChecked( true );
      pbtnLoadMinMax->setEnabled( true );
      cboxContrastEnhancementAlgorithm->setEnabled( true );
      labelContrastEnhancement->setEnabled( true );
      break;
    default:
      break;
  }

  if ( mRasterLayer->dataProvider()->dataType( 1 ) == QgsRasterDataProvider::ARGBDataType )
  {
    if ( tabPageSymbology != NULL )
    {
      delete tabPageSymbology;
      tabPageSymbology = NULL;
    }
    if ( tabPageColormap != NULL )
    {
      delete tabPageColormap;
      tabPageColormap = NULL;
    }
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

#if 0
  if ( mRasterLayer->rasterType() == QgsRasterLayer::Multiband )
  {
    //multiband images can also be rendered as single band (using only one of the bands)
    txtSymologyNotes->
    setText( tr
             ( "<h3>Multiband Image Notes</h3><p>This is a multiband image. You can choose to render it as grayscale or color (RGB). For color images, you can associate bands to colors arbitarily. For example, if you have a seven band landsat image, you may choose to render it as:</p><ul><li>Visible Blue (0.45 to 0.52 microns) - not mapped</li><li>Visible Green (0.52 to 0.60 microns) - not mapped</li></li>Visible Red (0.63 to 0.69 microns) - mapped to red in image</li><li>Near Infrared (0.76 to 0.90 microns) - mapped to green in image</li><li>Mid Infrared (1.55 to 1.75 microns) - not mapped</li><li>Thermal Infrared (10.4 to 12.5 microns) - not mapped</li><li>Mid Infrared (2.08 to 2.35 microns) - mapped to blue in image</li></ul>", "COMMENTED OUT" ) );
  }
  else if ( mRasterLayer->rasterType() == QgsRasterLayer::Palette )
  {
    //paletted images (e.g. tif) can only be rendered as three band rgb images
    txtSymologyNotes->
    setText( tr
             ( "<h3>Paletted Image Notes</h3> <p>This image uses a fixed color palette. You can remap these colors in different combinations e.g.</p><ul><li>Red - blue in image</li><li>Green - blue in image</li><li>Blue - green in image</li></ul>", "COMMENTED OUT" ) );
  }
  else                        //only grayscale settings allowed
  {
    //grayscale images can only be rendered as singleband
    txtSymologyNotes->
    setText( tr
             ( "<h3>Grayscale Image Notes</h3> <p>You can remap these grayscale colors to a pseudocolor image using an automatically generated color ramp.</p>", "COMMENTED OUT" ) );
  }
#endif

  //
  // Populate the various controls on the form
  //
  if ( mRasterLayer->dataProvider()->dataType( 1 ) != QgsRasterDataProvider::ARGBDataType )
  {
    QgsDebugMsg( "colorShadingAlgorithm = " + QString::number( mRasterLayer->colorShadingAlgorithm() ) );
    if ( mRasterLayer->drawingStyle() == QgsRasterLayer::SingleBandPseudoColor ||
         mRasterLayer->drawingStyle() == QgsRasterLayer::PalettedColor ||
         mRasterLayer->drawingStyle() == QgsRasterLayer::PalettedSingleBandPseudoColor ||
         mRasterLayer->drawingStyle() == QgsRasterLayer::MultiBandSingleBandPseudoColor )
    {
      if ( mRasterLayer->colorShadingAlgorithm() == QgsRasterLayer::PseudoColorShader )
      {
        cboxColorMap->setCurrentIndex( cboxColorMap->findText( tr( "Pseudocolor" ) ) );
      }
      else if ( mRasterLayer->colorShadingAlgorithm() == QgsRasterLayer::FreakOutShader )
      {
        cboxColorMap->setCurrentIndex( cboxColorMap->findText( tr( "Freak Out" ) ) );
      }
      else if ( mRasterLayer->colorShadingAlgorithm() == QgsRasterLayer::ColorRampShader )
      {
        cboxColorMap->setCurrentIndex( cboxColorMap->findText( tr( "Colormap" ) ) );
      }
      else if ( mRasterLayer->colorShadingAlgorithm() == QgsRasterLayer::UserDefinedShader )
      {
        cboxColorMap->setCurrentIndex( cboxColorMap->findText( tr( "User Defined" ) ) );
      }
    }
    else
    {
      cboxColorMap->setCurrentIndex( cboxColorMap->findText( tr( "Grayscale" ) ) );
    }
  }

  if ( mRasterLayer->dataProvider()->dataType( 1 ) != QgsRasterDataProvider::ARGBDataType )
  {
    if ( rbtnThreeBand->isChecked() )
    {
      mRGBMinimumMaximumEstimated = mRasterLayer->isRGBMinimumMaximumEstimated();
      if ( mRasterLayer->hasUserDefinedRGBMinimumMaximum() )
      {
        sboxThreeBandStdDev->setValue( mDefaultStandardDeviation );
        rbtnThreeBandStdDev->setChecked( false );
        rbtnThreeBandMinMax->setChecked( true );
      }
      else
      {
        sboxThreeBandStdDev->setValue( mRasterLayer->standardDeviations() );
        if ( mRasterLayer->standardDeviations() == 0.0 )
        {
          sboxThreeBandStdDev->setValue( mDefaultStandardDeviation );
          rbtnThreeBandStdDev->setChecked( false );
        }
        else
        {
          rbtnThreeBandStdDev->setChecked( true );
        }
        rbtnThreeBandMinMax->setChecked( false );
      }

      if ( QgsRasterLayer::PalettedColor != mRasterLayer->drawingStyle() &&
           QgsRasterLayer::PalettedSingleBandGray != mRasterLayer->drawingStyle() &&
           QgsRasterLayer::PalettedSingleBandPseudoColor != mRasterLayer->drawingStyle() &&
           QgsRasterLayer::PalettedMultiBandColor != mRasterLayer->drawingStyle() )
      {
        if ( mRasterLayer->redBandName() != TRSTRING_NOT_SET )
        {
          leRedMin->setText( QString::number( mRasterLayer->minimumValue( mRasterLayer->redBandName() ) ) );
          leRedMax->setText( QString::number( mRasterLayer->maximumValue( mRasterLayer->redBandName() ) ) );
        }
        if ( mRasterLayer->greenBandName() != TRSTRING_NOT_SET )
        {
          leGreenMin->setText( QString::number( mRasterLayer->minimumValue( mRasterLayer->greenBandName() ) ) );
          leGreenMax->setText( QString::number( mRasterLayer->maximumValue( mRasterLayer->greenBandName() ) ) );
        }
        if ( mRasterLayer->blueBandName() != TRSTRING_NOT_SET )
        {
          leBlueMin->setText( QString::number( mRasterLayer->minimumValue( mRasterLayer->blueBandName() ) ) );
          leBlueMax->setText( QString::number( mRasterLayer->maximumValue( mRasterLayer->blueBandName() ) ) );
        }
        setMinimumMaximumEstimateWarning();
      }
    }
    else
    {
      mGrayMinimumMaximumEstimated = mRasterLayer->isGrayMinimumMaximumEstimated();
      if ( mRasterLayer->hasUserDefinedGrayMinimumMaximum() )
      {
        sboxSingleBandStdDev->setValue( mDefaultStandardDeviation );
        rbtnSingleBandStdDev->setChecked( false );
        rbtnSingleBandMinMax->setChecked( true );
      }
      else
      {
        sboxSingleBandStdDev->setValue( mRasterLayer->standardDeviations() );
        if ( mRasterLayer->standardDeviations() == 0.0 )
        {
          sboxSingleBandStdDev->setValue( mDefaultStandardDeviation );
          rbtnSingleBandStdDev->setChecked( false );
        }
        else
        {
          rbtnSingleBandStdDev->setChecked( true );
        }
        rbtnSingleBandMinMax->setChecked( false );
      }

      if ( QgsRasterLayer::PalettedColor != mRasterLayer->drawingStyle() &&
           QgsRasterLayer::PalettedSingleBandGray != mRasterLayer->drawingStyle() &&
           QgsRasterLayer::PalettedSingleBandPseudoColor != mRasterLayer->drawingStyle() &&
           QgsRasterLayer::PalettedMultiBandColor != mRasterLayer->drawingStyle() )
      {
        if ( mRasterLayer->grayBandName() != TRSTRING_NOT_SET )
        {
          leGrayMin->setText( QString::number( mRasterLayer->minimumValue( mRasterLayer->grayBandName() ) ) );
          leGrayMax->setText( QString::number( mRasterLayer->maximumValue( mRasterLayer->grayBandName() ) ) );
        }
      }
      setMinimumMaximumEstimateWarning();
    }

    //set color scaling algorithm
    if ( QgsContrastEnhancement::StretchToMinimumMaximum == mRasterLayer->contrastEnhancementAlgorithm() )
    {
      cboxContrastEnhancementAlgorithm->setCurrentIndex( cboxContrastEnhancementAlgorithm->findText( tr( "Stretch To MinMax" ) ) );
    }
    else if ( QgsContrastEnhancement::StretchAndClipToMinimumMaximum == mRasterLayer->contrastEnhancementAlgorithm() )
    {
      cboxContrastEnhancementAlgorithm->setCurrentIndex( cboxContrastEnhancementAlgorithm->findText( tr( "Stretch And Clip To MinMax" ) ) );
    }
    else if ( QgsContrastEnhancement::ClipToMinimumMaximum == mRasterLayer->contrastEnhancementAlgorithm() )
    {
      cboxContrastEnhancementAlgorithm->setCurrentIndex( cboxContrastEnhancementAlgorithm->findText( tr( "Clip To MinMax" ) ) );
    }
    else if ( QgsContrastEnhancement::UserDefinedEnhancement == mRasterLayer->contrastEnhancementAlgorithm() )
    {
      cboxContrastEnhancementAlgorithm->setCurrentIndex( cboxContrastEnhancementAlgorithm->findText( tr( "User Defined" ) ) );
    }
    else
    {
      cboxContrastEnhancementAlgorithm->setCurrentIndex( cboxContrastEnhancementAlgorithm->findText( tr( "No Stretch" ) ) );
    }

    // Display the current default band combination
    mDefaultRedBand = myQSettings.value( "/Raster/defaultRedBand", 1 ).toInt();
    mDefaultGreenBand = myQSettings.value( "/Raster/defaultGreenBand", 2 ).toInt();
    mDefaultBlueBand = myQSettings.value( "/Raster/defaultBlueBand", 3 ).toInt();
    labelDefaultBandCombination->setText( tr( "Default R:%1 G:%2 B:%3" ).arg( mDefaultRedBand ) .arg( mDefaultGreenBand ) .arg( mDefaultBlueBand ) );

    // and used band combination
    cboRed->setCurrentIndex( cboRed->findText( mRasterLayer->redBandName() ) );
    cboGreen->setCurrentIndex( cboGreen->findText( mRasterLayer->greenBandName() ) );
    cboBlue->setCurrentIndex( cboBlue->findText( mRasterLayer->blueBandName() ) );

    //Display the current default contrast enhancement algorithm
    mDefaultContrastEnhancementAlgorithm = myQSettings.value( "/Raster/defaultContrastEnhancementAlgorithm", "NoEnhancement" ).toString();
    if ( mDefaultContrastEnhancementAlgorithm == "NoEnhancement" )
    {
      labelDefaultContrastEnhancementAlgorithm->setText( tr( "No Stretch" ) );
    }
    if ( mDefaultContrastEnhancementAlgorithm == "StretchToMinimumMaximum" )
    {
      labelDefaultContrastEnhancementAlgorithm->setText( tr( "Stretch To MinMax" ) );
    }
    else if ( mDefaultContrastEnhancementAlgorithm == "StretchAndClipToMinimumMaximum" )
    {
      labelDefaultContrastEnhancementAlgorithm->setText( tr( "Stretch And Clip To MinMax" ) );
    }
    else if ( mDefaultContrastEnhancementAlgorithm == "ClipToMinimumMaximum" )
    {
      labelDefaultContrastEnhancementAlgorithm->setText( tr( "Clip To MinMax" ) );
    }
    else
    {
      labelDefaultContrastEnhancementAlgorithm->setText( tr( "No Stretch" ) );
    }
    mDefaultStandardDeviation = myQSettings.value( "/Raster/defaultStandardDeviation", 2.0 ).toDouble();
    sboxThreeBandStdDev->setValue( mDefaultStandardDeviation );
  }

  QgsDebugMsg( "populate transparency tab" );

  /*
   * Transparent Pixel Tab
   */

  //set the transparency slider
  sliderTransparency->setValue( 255 - mRasterLayer->getTransparency() );
  //update the transparency percentage label
  sliderTransparency_valueChanged( 255 - mRasterLayer->getTransparency() );

  int myIndex = cboxTransparencyBand->findText( mRasterLayer->transparentBandName() );
  if ( -1 != myIndex )
  {
    cboxTransparencyBand->setCurrentIndex( myIndex );
  }
  else
  {
    cboxTransparencyBand->setCurrentIndex( cboxTransparencyBand->findText( TRSTRING_NOT_SET ) );
  }
  //add current NoDataValue to NoDataValue line edit
  if ( mRasterLayer->isNoDataValueValid() )
  {
    leNoDataValue->insert( QString::number( mRasterLayer->noDataValue(), 'f' ) );
  }
  else
  {
    leNoDataValue->insert( "" );
  }

  populateTransparencyTable();

  QgsDebugMsg( "populate colormap tab" );
  /*
   * Transparent Pixel Tab
   */
  //restore colormap tab if the layer has custom classification
  syncColormapTab();

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
  pixmapLegend->setPixmap( mRasterLayer->legendAsPixmap() );
  pixmapLegend->setScaledContents( true );
  pixmapLegend->repaint();

  //set the palette pixmap
  pixmapPalette->setPixmap( mRasterLayer->paletteAsPixmap( mRasterLayer->bandNumber( mRasterLayer->grayBandName() ) ) );
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

void QgsRasterLayerProperties::syncColormapTab()
{
  QgsDebugMsg( "populate color ramp tab" );
  if ( !mRasterLayer || !mRasterLayer->dataProvider() )
  {
    return;
  }

  if ( mRasterLayer->dataProvider()->dataType( 1 ) == QgsRasterDataProvider::ARGBDataType )
  {
    return;
  }

  if ( QgsRasterLayer::ColorRampShader != mRasterLayer->colorShadingAlgorithm() )
  {
    return;
  }

  QgsColorRampShader* myRasterShaderFunction = ( QgsColorRampShader* )mRasterLayer->rasterShader()->rasterShaderFunction();
  if ( !myRasterShaderFunction )
  {
    return;
  }
  //restore the colormap tab if layer has custom symbology
  populateColorMapTable( myRasterShaderFunction->colorRampItemList() );

  sboxNumberOfEntries->setValue( mColormapTreeWidget->topLevelItemCount() );

  //restor state of 'color interpolation' combo box
  if ( QgsColorRampShader::INTERPOLATED == myRasterShaderFunction->colorRampType() )
  {
    cboxColorInterpolation->setCurrentIndex( cboxColorInterpolation->findText( tr( "Linear" ) ) );
  }
  else if ( QgsColorRampShader::DISCRETE == myRasterShaderFunction->colorRampType() )
  {
    cboxColorInterpolation->setCurrentIndex( cboxColorInterpolation->findText( tr( "Discrete" ) ) );
  }
  else
  {
    cboxColorInterpolation->setCurrentIndex( cboxColorInterpolation->findText( tr( "Exact" ) ) );
  }

}

bool QgsRasterLayerProperties::validUserDefinedMinMax()
{
  if ( rbtnThreeBand->isChecked() )
  {
    bool myDoubleOk;
    leRedMin->text().toDouble( &myDoubleOk );
    if ( myDoubleOk )
    {
      leRedMax->text().toDouble( &myDoubleOk );
      if ( myDoubleOk )
      {
        leGreenMin->text().toDouble( &myDoubleOk );
        if ( myDoubleOk )
        {
          leGreenMax->text().toDouble( &myDoubleOk );
          if ( myDoubleOk )
          {
            leBlueMin->text().toDouble( &myDoubleOk );
            if ( myDoubleOk )
            {
              leBlueMax->text().toDouble( &myDoubleOk );
              if ( myDoubleOk )
              {
                return true;
              }
            }
          }
        }
      }
    }
  }
  else
  {
    bool myDoubleOk;
    leGrayMin->text().toDouble( &myDoubleOk );
    if ( myDoubleOk )
    {
      leGrayMax->text().toDouble( &myDoubleOk );
      if ( myDoubleOk )
      {
        return true;
      }
    }
  }

  return false;
}


/*
 *
 * PUBLIC AND PRIVATE SLOTS
 *
 */
void QgsRasterLayerProperties::apply()
{
  if ( mRasterLayer->dataProvider()->dataType( 1 ) != QgsRasterDataProvider::ARGBDataType )
  {
    QgsDebugMsg( "apply processing symbology tab" );
    /*
     * Symbology Tab
     */
    //set the appropriate render style
    if ( rbtnSingleBand->isChecked() )
    {
      //
      // Grayscale
      //
      if ( mRasterLayer->rasterType() == QgsRasterLayer::GrayOrUndefined )
      {

        if ( cboxColorMap->currentText() != tr( "Grayscale" ) )
        {
          QgsDebugMsg( "Raster Drawing Style to :: SingleBandPseudoColor" );

          mRasterLayer->setDrawingStyle( QgsRasterLayer::SingleBandPseudoColor );
        }
        else
        {
          QgsDebugMsg( "Setting Raster Drawing Style to :: SingleBandGray" );

          mRasterLayer->setDrawingStyle( QgsRasterLayer::SingleBandGray );
        }
      }
      //
      // Paletted Image
      //
      else if ( mRasterLayer->rasterType() == QgsRasterLayer::Palette )
      {
        if ( cboxColorMap->currentText() == tr( "Grayscale" ) )
        {
          QgsDebugMsg( "Setting Raster Drawing Style to :: PalettedSingleBandGray" );
          QgsDebugMsg( QString( "Combo value : %1 GrayBand Mapping : %2" )
                       .arg( cboGray->currentText() ).arg( mRasterLayer->grayBandName() ) );

          mRasterLayer->setDrawingStyle( QgsRasterLayer::PalettedSingleBandGray );
        }
        else if ( cboxColorMap->currentText() == tr( "Colormap" ) )
        {
          QgsDebugMsg( "Setting Raster Drawing Style to :: PalettedColor" );
          QgsDebugMsg( QString( "Combo value : %1 GrayBand Mapping : %2" ).arg( cboGray->currentText() ).arg( mRasterLayer->
                       grayBandName() ) );

          mRasterLayer->setDrawingStyle( QgsRasterLayer::PalettedColor );
        }
        else
        {
          QgsDebugMsg( "Setting Raster Drawing Style to :: PalettedSingleBandPseudoColor" );

          mRasterLayer->setDrawingStyle( QgsRasterLayer::PalettedSingleBandPseudoColor );
        }

      }
      //
      // Mutltiband
      //
      else if ( mRasterLayer->rasterType() == QgsRasterLayer::Multiband )
      {
        if ( cboxColorMap->currentText() != tr( "Grayscale" ) )
        {
          QgsDebugMsg( "Setting Raster Drawing Style to ::MultiBandSingleBandPseudoColor " );

          mRasterLayer->setDrawingStyle( QgsRasterLayer::MultiBandSingleBandPseudoColor );
        }
        else
        {
          QgsDebugMsg( "Setting Raster Drawing Style to :: MultiBandSingleBandGray" );
          QgsDebugMsg( QString( "Combo value : %1 GrayBand Mapping : %2" ).arg( cboGray->currentText() ).arg( mRasterLayer->
                       grayBandName() ) );

          mRasterLayer->setDrawingStyle( QgsRasterLayer::MultiBandSingleBandGray );

        }
      }
    }                           //end of grayscale box enabled and rbtnsingleband checked
    else                          //assume that rbtnThreeBand is checked and render in rgb color
    {
      //set the grayscale color table type if the groupbox is enabled

      if ( mRasterLayer->rasterType() == QgsRasterLayer::Palette )
      {
        QgsDebugMsg( "Setting Raster Drawing Style to :: PalettedMultiBandColor" );

        mRasterLayer->setDrawingStyle( QgsRasterLayer::PalettedMultiBandColor );
      }
      else if ( mRasterLayer->rasterType() == QgsRasterLayer::Multiband )
      {

        QgsDebugMsg( "Setting Raster Drawing Style to :: MultiBandColor" );

        mRasterLayer->setDrawingStyle( QgsRasterLayer::MultiBandColor );
      }

    }

    //set whether the layer histogram should be inverted
    mRasterLayer->setInvertHistogram( cboxInvertColorMap->isChecked() );

    //now set the color -> band mapping combos to the correct values
    mRasterLayer->setRedBandName( cboRed->currentText() );
    mRasterLayer->setGreenBandName( cboGreen->currentText() );
    mRasterLayer->setBlueBandName( cboBlue->currentText() );
    mRasterLayer->setGrayBandName( cboGray->currentText() );
    mRasterLayer->setTransparentBandName( cboxTransparencyBand->currentText() );

    //set the appropriate color shading type
    //If UserDefined do nothing, user defined can only be set programatically
    if ( cboxColorMap->currentText() == tr( "Pseudocolor" ) )
    {
      mRasterLayer->setColorShadingAlgorithm( QgsRasterLayer::PseudoColorShader );
    }
    else if ( cboxColorMap->currentText() == tr( "Freak Out" ) )
    {
      mRasterLayer->setColorShadingAlgorithm( QgsRasterLayer::FreakOutShader );
    }
    else if ( cboxColorMap->currentText() == tr( "Colormap" ) )
    {
      mRasterLayer->setColorShadingAlgorithm( QgsRasterLayer::ColorRampShader );
    }

    //set the color scaling algorithm
    //Since the maximum, minimum values are going to be set anyway, pass in false for the second parameter of setContrastEnahancementAlgorithm
    //so the the look up tables are not generated for each band, since their parameters are about to change anyway.This will also generate the
    //lookup tables for the three or one band(s) that are immediately needed
    if ( cboxContrastEnhancementAlgorithm->currentText() == tr( "Stretch To MinMax" ) )
    {
      mRasterLayer->setContrastEnhancementAlgorithm( QgsContrastEnhancement::StretchToMinimumMaximum, false );
    }
    else if ( cboxContrastEnhancementAlgorithm->currentText() == tr( "Stretch And Clip To MinMax" ) )
    {
      mRasterLayer->setContrastEnhancementAlgorithm( QgsContrastEnhancement::StretchAndClipToMinimumMaximum, false );
    }
    else if ( cboxContrastEnhancementAlgorithm->currentText() == tr( "Clip To MinMax" ) )
    {
      mRasterLayer->setContrastEnhancementAlgorithm( QgsContrastEnhancement::ClipToMinimumMaximum, false );
    }
    else if ( QgsContrastEnhancement::UserDefinedEnhancement == mRasterLayer->contrastEnhancementAlgorithm() )
    {
      //do nothing
    }
    else
    {
      mRasterLayer->setContrastEnhancementAlgorithm( QgsContrastEnhancement::NoEnhancement, false );
    }

    //set the std deviations to be plotted and check for user defined Min Max values
    if ( rbtnThreeBand->isChecked() )
    {
      //Set min max based on user defined values if all are set and stdDev is 0.0
      if ( rbtnThreeBandMinMax->isEnabled() && rbtnThreeBandMinMax->isChecked() && validUserDefinedMinMax() )
      {
        mRasterLayer->setRGBMinimumMaximumEstimated( mRGBMinimumMaximumEstimated );
        if ( mRasterLayer->redBandName() != TRSTRING_NOT_SET )
        {
          mRasterLayer->setMinimumValue( cboRed->currentText(), leRedMin->text().toDouble(), false );
          mRasterLayer->setMaximumValue( cboRed->currentText(), leRedMax->text().toDouble() );
        }
        if ( mRasterLayer->greenBandName() != TRSTRING_NOT_SET )
        {
          mRasterLayer->setMinimumValue( cboGreen->currentText(), leGreenMin->text().toDouble(), false );
          mRasterLayer->setMaximumValue( cboGreen->currentText(), leGreenMax->text().toDouble() );
        }
        if ( mRasterLayer->blueBandName() != TRSTRING_NOT_SET )
        {
          mRasterLayer->setMinimumValue( cboBlue->currentText(), leBlueMin->text().toDouble(), false );
          mRasterLayer->setMaximumValue( cboBlue->currentText(), leBlueMax->text().toDouble() );
        }
        mRasterLayer->setStandardDeviations( 0.0 );
        mRasterLayer->setUserDefinedRGBMinimumMaximum( true );
      }
      else if ( rbtnThreeBandStdDev->isEnabled() && rbtnThreeBandStdDev->isChecked() )
      {
        mRasterLayer->setStandardDeviations( sboxThreeBandStdDev->value() );
        mRasterLayer->setUserDefinedRGBMinimumMaximum( false );
      }
      else
      {
        mRasterLayer->setStandardDeviations( 0.0 );
        mRasterLayer->setUserDefinedRGBMinimumMaximum( false );
      }
    }
    else
    {
      //Set min max based on user defined values if all are set and stdDev is 0.0
      if ( rbtnSingleBandMinMax->isEnabled() && rbtnSingleBandMinMax->isChecked() && validUserDefinedMinMax() )
      {
        mRasterLayer->setGrayMinimumMaximumEstimated( mGrayMinimumMaximumEstimated );
        if ( mRasterLayer->grayBandName() != TRSTRING_NOT_SET )
        {
          mRasterLayer->setMinimumValue( cboGray->currentText(), leGrayMin->text().toDouble(), false );
          mRasterLayer->setMaximumValue( cboGray->currentText(), leGrayMax->text().toDouble() );
        }
        mRasterLayer->setStandardDeviations( 0.0 );
        mRasterLayer->setUserDefinedGrayMinimumMaximum( true );
      }
      else if ( rbtnSingleBandStdDev->isEnabled() && rbtnSingleBandStdDev->isChecked() )
      {
        mRasterLayer->setStandardDeviations( sboxSingleBandStdDev->value() );
        mRasterLayer->setUserDefinedGrayMinimumMaximum( false );
      }
      else
      {
        mRasterLayer->setStandardDeviations( 0.0 );
        mRasterLayer->setUserDefinedGrayMinimumMaximum( false );
      }
    }

    QSettings myQSettings;
    myQSettings.setValue( "/Raster/defaultRedBand", mDefaultRedBand );
    myQSettings.setValue( "/Raster/defaultGreenBand", mDefaultGreenBand );
    myQSettings.setValue( "/Raster/defaultBlueBand", mDefaultBlueBand );

    myQSettings.setValue( "/Raster/defaultContrastEnhancementAlgorithm", mDefaultContrastEnhancementAlgorithm );

    myQSettings.setValue( "/Raster/defaultStandardDeviation", mDefaultStandardDeviation );

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

    //Walk through each row in table and test value. If not valid set to 0.0 and continue building transparency list
    if ( rbtnThreeBand->isChecked() && QgsRasterLayer::MultiBandColor == mRasterLayer->drawingStyle() )
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

      mRasterLayer->rasterTransparency()->setTransparentThreeValuePixelList( myTransparentThreeValuePixelList );
    }
    else
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

      mRasterLayer->rasterTransparency()->setTransparentSingleValuePixelList( myTransparentSingleValuePixelList );
    }

    QgsDebugMsg( "apply processing Colormap tab" );
    /*
     * ColorMap Tab
     */
    if ( cboxColorMap->currentText() == tr( "Colormap" ) )
    {
      QgsColorRampShader* myRasterShaderFunction = ( QgsColorRampShader* )mRasterLayer->rasterShader()->rasterShaderFunction();
      if ( myRasterShaderFunction )
      {
        //iterate through mColormapTreeWidget and set colormap info of layer
        QList<QgsColorRampShader::ColorRampItem> myColorRampItems;

        int myTopLevelItemCount = mColormapTreeWidget->topLevelItemCount();
        QTreeWidgetItem* myCurrentItem;
        for ( int i = 0; i < myTopLevelItemCount; ++i )
        {
          myCurrentItem = mColormapTreeWidget->topLevelItem( i );
          if ( !myCurrentItem )
          {
            continue;
          }
          QgsColorRampShader::ColorRampItem myNewColorRampItem;
          myNewColorRampItem.value = myCurrentItem->text( 0 ).toDouble();
          myNewColorRampItem.color = myCurrentItem->background( 1 ).color();
          myNewColorRampItem.label = myCurrentItem->text( 2 );

          myColorRampItems.append( myNewColorRampItem );
        }

        // sort the shader items
        qSort( myColorRampItems );

        myRasterShaderFunction->setColorRampItemList( myColorRampItems );
        //Reload table in GUI because it may have been sorted or contained invalid values
        populateColorMapTable( myColorRampItems );

        if ( cboxColorInterpolation->currentText() == tr( "Linear" ) )
        {
          myRasterShaderFunction->setColorRampType( QgsColorRampShader::INTERPOLATED );
        }
        else if ( cboxColorInterpolation->currentText() == tr( "Discrete" ) )
        {
          myRasterShaderFunction->setColorRampType( QgsColorRampShader::DISCRETE );
        }
        else
        {
          myRasterShaderFunction->setColorRampType( QgsColorRampShader::EXACT );
        }
      }
      else
      {
        QgsDebugMsg( "color ramp was NOT set because RasterShaderFunction was NULL" );
      }
    }
  }

  //set transparency
  mRasterLayer->setTransparency( static_cast < unsigned int >( 255 - sliderTransparency->value() ) );

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

  //resampling
  if ( mNearestNeighbourRadioButton->isChecked() )
  {
    mRasterLayer->setResampler( 0 );
  }
  else if ( mBilinearRadioButton->isChecked() )
  {
    mRasterLayer->setResampler( new QgsBilinearRasterResampler() );
  }
  else if ( mCubicRadioButton->isChecked() )
  {
    mRasterLayer->setResampler( new QgsCubicRasterResampler() );
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

  //Because Min Max values can be set during the redraw if a strech is requested we need to resync after apply
  if ( mRasterLayer->dataProvider()->dataType( 1 ) != QgsRasterDataProvider::ARGBDataType )
  {
    if ( QgsContrastEnhancement::NoEnhancement != mRasterLayer->contrastEnhancementAlgorithm() )
    {
      //set the stdDevs and min max values
      if ( rbtnThreeBand->isChecked() )
      {
        if ( rbtnThreeBandStdDev->isEnabled() )
        {
          sboxThreeBandStdDev->setValue( mRasterLayer->standardDeviations() );
        }

        if ( rbtnThreeBandMinMax->isEnabled() )
        {
          mRGBMinimumMaximumEstimated = mRasterLayer->isRGBMinimumMaximumEstimated();
          if ( mRasterLayer->redBandName() != TRSTRING_NOT_SET )
          {
            leRedMin->setText( QString::number( mRasterLayer->minimumValue( mRasterLayer->redBandName() ) ) );
            leRedMax->setText( QString::number( mRasterLayer->maximumValue( mRasterLayer->redBandName() ) ) );
          }
          if ( mRasterLayer->greenBandName() != TRSTRING_NOT_SET )
          {
            leGreenMin->setText( QString::number( mRasterLayer->minimumValue( mRasterLayer->greenBandName() ) ) );
            leGreenMax->setText( QString::number( mRasterLayer->maximumValue( mRasterLayer->greenBandName() ) ) );
          }
          if ( mRasterLayer->blueBandName() != TRSTRING_NOT_SET )
          {
            leBlueMin->setText( QString::number( mRasterLayer->minimumValue( mRasterLayer->blueBandName() ) ) );
            leBlueMax->setText( QString::number( mRasterLayer->maximumValue( mRasterLayer->blueBandName() ) ) );
          }
          setMinimumMaximumEstimateWarning();
        }
      }
      else
      {
        if ( rbtnSingleBandStdDev->isEnabled() )
        {
          sboxSingleBandStdDev->setValue( mRasterLayer->standardDeviations() );
        }

        if ( rbtnSingleBandMinMax->isEnabled() )
        {
          mGrayMinimumMaximumEstimated = mRasterLayer->isGrayMinimumMaximumEstimated();
          if ( mRasterLayer->grayBandName() != TRSTRING_NOT_SET )
          {
            leGrayMin->setText( QString::number( mRasterLayer->minimumValue( mRasterLayer->grayBandName() ) ) );
            leGrayMax->setText( QString::number( mRasterLayer->maximumValue( mRasterLayer->grayBandName() ) ) );
          }
          setMinimumMaximumEstimateWarning();
        }
      }
    }

    //GUI Cleanup
    //Once the user has applied the changes, user defined function will not longer be a valid option so it should be
    //removed from the list
    if ( -1 != cboxColorMap->findText( tr( "User Defined" ) ) && tr( "User Defined" ) != cboxColorMap->currentText() )
    {
      cboxColorMap->removeItem( cboxColorMap->findText( tr( "User Defined" ) ) );
    }

    if ( -1 != cboxContrastEnhancementAlgorithm->findText( tr( "User Defined" ) ) && tr( "User Defined" ) != cboxContrastEnhancementAlgorithm->currentText() )
    {
      cboxContrastEnhancementAlgorithm->removeItem( cboxContrastEnhancementAlgorithm->findText( tr( "User Defined" ) ) );
    }
  }

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

void QgsRasterLayerProperties::on_cboBlue_currentIndexChanged( const QString& theText )
{
  if ( TRSTRING_NOT_SET != theText )
  {
    leBlueMin->setText( QString::number( mRasterLayer->minimumValue( theText ) ) );
    leBlueMax->setText( QString::number( mRasterLayer->maximumValue( theText ) ) );
  }
}

void QgsRasterLayerProperties::on_cboGray_currentIndexChanged( const QString& theText )
{
  if ( TRSTRING_NOT_SET != theText )
  {
    leGrayMin->setText( QString::number( mRasterLayer->minimumValue( theText ) ) );
    leGrayMax->setText( QString::number( mRasterLayer->maximumValue( theText ) ) );
  }
}

void QgsRasterLayerProperties::on_cboGreen_currentIndexChanged( const QString& theText )
{
  if ( TRSTRING_NOT_SET != theText )
  {
    leGreenMin->setText( QString::number( mRasterLayer->minimumValue( theText ) ) );
    leGreenMax->setText( QString::number( mRasterLayer->maximumValue( theText ) ) );
  }
}

void QgsRasterLayerProperties::on_cboRed_currentIndexChanged( const QString& theText )
{
  if ( TRSTRING_NOT_SET != theText )
  {
    leRedMin->setText( QString::number( mRasterLayer->minimumValue( theText ) ) );
    leRedMax->setText( QString::number( mRasterLayer->maximumValue( theText ) ) );
  }
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

void QgsRasterLayerProperties::on_cboxColorMap_currentIndexChanged( const QString& theText )
{
  if ( mRasterLayer->dataProvider()->dataType( 1 ) == QgsRasterDataProvider::ARGBDataType )
  {
    return;
  }

  if ( theText == tr( "Pseudocolor" ) || theText == tr( "Freak Out" ) )
  {
    tabPageColormap->setEnabled( false );
    rbtnSingleBandMinMax->setEnabled( false );
    rbtnSingleBandStdDev->setEnabled( true );
    sboxSingleBandStdDev->setEnabled( true );
    pbtnLoadMinMax->setEnabled( false );
    cboxContrastEnhancementAlgorithm->setEnabled( false );
    labelContrastEnhancement->setEnabled( false );
  }
  else if ( theText == tr( "Colormap" ) )
  {
    tabPageColormap->setEnabled( true );
    rbtnSingleBandMinMax->setEnabled( false );
    rbtnSingleBandStdDev->setEnabled( false );
    sboxSingleBandStdDev->setEnabled( false );
    pbtnLoadMinMax->setEnabled( false );
    cboxContrastEnhancementAlgorithm->setEnabled( false );
    labelContrastEnhancement->setEnabled( false );
  }
  else if ( theText == tr( "User Defined" ) )
  {
    tabPageColormap->setEnabled( false );
    rbtnSingleBandMinMax->setEnabled( true );
    rbtnSingleBandStdDev->setEnabled( true );
    sboxSingleBandStdDev->setEnabled( true );
    pbtnLoadMinMax->setEnabled( true );
    cboxContrastEnhancementAlgorithm->setEnabled( false );
    labelContrastEnhancement->setEnabled( false );
  }
  else
  {
    tabPageColormap->setEnabled( false );
    rbtnSingleBandMinMax->setEnabled( true );
    rbtnSingleBandStdDev->setEnabled( true );
    sboxSingleBandStdDev->setEnabled( true );
    if ( mRasterLayer->rasterType() != QgsRasterLayer::Palette )
    {
      pbtnLoadMinMax->setEnabled( true );
      cboxContrastEnhancementAlgorithm->setEnabled( true );
      labelContrastEnhancement->setEnabled( true );
    }
    else
    {
      pbtnLoadMinMax->setEnabled( false );
      cboxContrastEnhancementAlgorithm->setEnabled( false );
      labelContrastEnhancement->setEnabled( false );
    }
  }
}

void QgsRasterLayerProperties::on_pbnDefaultValues_clicked()
{

  if ( rbtnThreeBand->isChecked() && QgsRasterLayer::PalettedColor != mRasterLayer->drawingStyle() &&
       QgsRasterLayer::PalettedMultiBandColor != mRasterLayer->drawingStyle() )
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
  else
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
  QString myFileName = QFileDialog::getSaveFileName( this, tr( "Save file" ), myLastDir, tr( "Textfile (*.txt)" ) );
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
      if ( rbtnThreeBand->isChecked() && QgsRasterLayer::MultiBandColor == mRasterLayer->drawingStyle() )
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
  int myHistogramTab = 6;
  if ( theTab == myHistogramTab )
  {
    refreshHistogram();
  }
}

void QgsRasterLayerProperties::refreshHistogram()
{
#if !defined(QWT_VERSION) || QWT_VERSION<0x060000
  mpPlot->clear();
#endif
  mHistogramProgress->show();
  connect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mHistogramProgress, SLOT( setValue( int ) ) );
  QApplication::setOverrideCursor( Qt::WaitCursor );
  QgsDebugMsg( "entered." );
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
  // Explanation:
  // We use the gdal histogram creation routine is called for each selected
  // layer. Currently the hist is hardcoded
  // to create 256 bins. Each bin stores the total number of cells that
  // fit into the range defined by that bin.
  //
  // The graph routine below determines the greatest number of pixesl in any given
  // bin in all selected layers, and the min. It then draws a scaled line between min
  // and max - scaled to image height. 1 line drawn per selected band
  //
  const int BINCOUNT = 256;
  bool myIgnoreOutOfRangeFlag = true;
  bool myThoroughBandScanFlag = false;
  int myBandCountInt = mRasterLayer->bandCount();
  QList<QColor> myColors;
  myColors << Qt::black << Qt::red << Qt::green << Qt::blue << Qt::magenta << Qt::darkRed << Qt::darkGreen << Qt::darkBlue;

  while ( myColors.size() <= myBandCountInt )
  {
    myColors <<
    QColor( 1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) ),
            1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) ),
            1 + ( int )( 255.0 * rand() / ( RAND_MAX + 1.0 ) ) );
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
  float myGlobalMin = 0;
  float myGlobalMax = 0;
  bool myFirstIteration = true;
  for ( int myIteratorInt = 1;
        myIteratorInt <= myBandCountInt;
        ++myIteratorInt )
  {
    QgsRasterBandStats myRasterBandStats = mRasterLayer->bandStatistics( myIteratorInt );
    mRasterLayer->populateHistogram( myIteratorInt, BINCOUNT, myIgnoreOutOfRangeFlag, myThoroughBandScanFlag );
    QwtPlotCurve * mypCurve = new QwtPlotCurve( tr( "Band %1" ).arg( myIteratorInt ) );
    mypCurve->setCurveAttribute( QwtPlotCurve::Fitted );
    mypCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
    mypCurve->setPen( QPen( myColors.at( myIteratorInt ) ) );
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
    if ( myFirstIteration || myGlobalMin < myRasterBandStats.minimumValue )
    {
      myGlobalMin = myRasterBandStats.minimumValue;
    }
    if ( myFirstIteration || myGlobalMax < myRasterBandStats.maximumValue )
    {
      myGlobalMax = myRasterBandStats.maximumValue;
    }
    myFirstIteration = false;
  }
  // for x axis use band pixel values rather than gdal hist. bin values
  // subtract -0.5 to prevent rounding errors
  // see http://www.gdal.org/classGDALRasterBand.html#3f8889607d3b2294f7e0f11181c201c8
  mpPlot->setAxisScale( QwtPlot::xBottom,
                        myGlobalMin - 0.5,
                        myGlobalMax + 0.5 );
  mpPlot->replot();
  disconnect( mRasterLayer, SIGNAL( progressUpdate( int ) ), mHistogramProgress, SLOT( setValue( int ) ) );
  mHistogramProgress->hide();
  mpPlot->canvas()->setCursor( Qt::ArrowCursor );
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
  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Open file" ), myLastDir, tr( "Textfile (*.txt)" ) );
  QFile myInputFile( myFileName );
  if ( myInputFile.open( QFile::ReadOnly ) )
  {
    QTextStream myInputStream( &myInputFile );
    QString myInputLine;
    if ( rbtnThreeBand->isChecked() && QgsRasterLayer::MultiBandColor == mRasterLayer->drawingStyle() )
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

void QgsRasterLayerProperties::on_rbtnSingleBand_toggled( bool theState )
{
  if ( theState )
  {
    //--- enable and disable appropriate controls
    stackedWidget->setCurrentIndex( 1 );
    rbtnThreeBand->setChecked( false );
    cboxColorMap->setEnabled( true );

    if ( cboxColorMap->currentText() == tr( "Pseudocolor" ) )
    {
      tabPageColormap->setEnabled( true );
    }

    if ( cboxColorMap->currentText() == tr( "Pseudocolor" ) || cboxColorMap->currentText() == tr( "Color Ramp" ) || cboxColorMap->currentText() == tr( "Freak Out" ) || mRasterLayer->rasterType() == QgsRasterLayer::Palette )
    {
      pbtnLoadMinMax->setEnabled( false );
      labelContrastEnhancement->setEnabled( false );
      cboxContrastEnhancementAlgorithm->setEnabled( false );
    }
    else
    {
      pbtnLoadMinMax->setEnabled( true );
      labelContrastEnhancement->setEnabled( true );
      cboxContrastEnhancementAlgorithm->setEnabled( true );
    }

    grpRgbBands->setEnabled( false );

    if ( mRasterLayer->hasUserDefinedGrayMinimumMaximum() )
    {
      sboxSingleBandStdDev->setValue( mDefaultStandardDeviation );
      rbtnSingleBandMinMax->setChecked( true );
      leGrayMin->setText( QString::number( mRasterLayer->minimumValue( cboGray->currentText() ) ) );
      leGrayMax->setText( QString::number( mRasterLayer->maximumValue( cboGray->currentText() ) ) );
    }
    else
    {
      sboxSingleBandStdDev->setValue( mRasterLayer->standardDeviations() );
      if ( mRasterLayer->standardDeviations() == 0.0 )
      {
        sboxSingleBandStdDev->setValue( mDefaultStandardDeviation );
        rbtnSingleBandStdDev->setChecked( false );
      }
      else
      {
        rbtnSingleBandStdDev->setChecked( true );
      }
    }

    // Populate transparency table with single value transparency pixels
    populateTransparencyTable();
    // If no band is selected but there are multiple bands, selcet the first as the default
    if ( cboGray->currentText() == TRSTRING_NOT_SET && 1 < cboGray->count() )
    {
      cboGray->setCurrentIndex( 0 );
    }
  }
  else if ( !rbtnThreeBand->isEnabled() )
  {
    rbtnSingleBand->setChecked( true );
  }
  else
  {
    rbtnThreeBand->setChecked( true );
  }
}

void QgsRasterLayerProperties::on_rbtnSingleBandMinMax_toggled( bool theState )
{
  lblGrayMin->setEnabled( theState );
  leGrayMin->setEnabled( theState );
  lblGrayMax->setEnabled( theState );
  leGrayMax->setEnabled( theState );
}

void QgsRasterLayerProperties::on_rbtnSingleBandStdDev_toggled( bool theState )
{
  sboxSingleBandStdDev->setEnabled( theState );
}

void QgsRasterLayerProperties::on_rbtnThreeBand_toggled( bool theState )
{
  if ( theState )
  {
    //--- enable and disable appropriate controls
    stackedWidget->setCurrentIndex( 0 );
    rbtnSingleBand->setChecked( false );
    cboxColorMap->setEnabled( false );
    tabPageColormap->setEnabled( false );

    grpRgbBands->setEnabled( true );

    pbtnLoadMinMax->setEnabled( true );
    labelContrastEnhancement->setEnabled( true );
    cboxContrastEnhancementAlgorithm->setEnabled( true );

    /*
     *This may seem strange at first, but the single bands need to be include here for switching
     *from gray back to color with a multi-band palleted image
     */
    if ( QgsRasterLayer::PalettedColor == mRasterLayer->drawingStyle() ||
         QgsRasterLayer::PalettedSingleBandGray == mRasterLayer->drawingStyle() ||
         QgsRasterLayer::PalettedSingleBandPseudoColor == mRasterLayer->drawingStyle() ||
         QgsRasterLayer::PalettedMultiBandColor == mRasterLayer->drawingStyle() ||
         mRasterLayer->rasterType() == QgsRasterLayer::Palette )
    {
      pbtnLoadMinMax->setEnabled( false );
      cboxContrastEnhancementAlgorithm->setCurrentIndex( cboxContrastEnhancementAlgorithm->findText( tr( "No Stretch" ) ) );
      cboxContrastEnhancementAlgorithm->setEnabled( false );
      labelContrastEnhancement->setEnabled( false );
      sboxThreeBandStdDev->setEnabled( false );
    }

    if ( mRasterLayer->hasUserDefinedRGBMinimumMaximum() )
    {
      sboxThreeBandStdDev->setValue( 0.0 );
      rbtnThreeBandMinMax->setChecked( true );
      leRedMin->setText( QString::number( mRasterLayer->minimumValue( cboRed->currentText() ) ) );
      leRedMax->setText( QString::number( mRasterLayer->maximumValue( cboRed->currentText() ) ) );
      leGreenMin->setText( QString::number( mRasterLayer->minimumValue( cboGreen->currentText() ) ) );
      leGreenMax->setText( QString::number( mRasterLayer->maximumValue( cboGreen->currentText() ) ) );
      leBlueMin->setText( QString::number( mRasterLayer->minimumValue( cboBlue->currentText() ) ) );
      leBlueMax->setText( QString::number( mRasterLayer->maximumValue( cboBlue->currentText() ) ) );
    }
    else
    {
      sboxThreeBandStdDev->setValue( mRasterLayer->standardDeviations() );
      if ( mRasterLayer->standardDeviations() == 0.0 )
      {
        sboxThreeBandStdDev->setValue( mDefaultStandardDeviation );
        rbtnThreeBandStdDev->setChecked( false );
      }
      else
      {
        rbtnThreeBandStdDev->setChecked( true );
      }

    }

    // Populate transparency table with single value transparency pixels
    populateTransparencyTable();
  }
  else if ( !rbtnSingleBand->isEnabled() )
  {
    rbtnThreeBand->setChecked( true );
  }
  else
  {
    rbtnSingleBand->setChecked( true );
  }
}

void QgsRasterLayerProperties::on_rbtnThreeBandMinMax_toggled( bool theState )
{
  lblRedMin->setEnabled( theState );
  leRedMin->setEnabled( theState );
  lblRedMax->setEnabled( theState );
  leRedMax->setEnabled( theState );
  lblGreenMin->setEnabled( theState );
  leGreenMin->setEnabled( theState );
  lblGreenMax->setEnabled( theState );
  leGreenMax->setEnabled( theState );
  lblBlueMin->setEnabled( theState );
  leBlueMin->setEnabled( theState );
  lblBlueMax->setEnabled( theState );
  leBlueMax->setEnabled( theState );
}

void QgsRasterLayerProperties::on_rbtnThreeBandStdDev_toggled( bool theState )
{
  sboxThreeBandStdDev->setEnabled( theState );
  sboxThreeBandStdDev->setValue( mDefaultStandardDeviation );
}

void QgsRasterLayerProperties::pixelSelected( const QgsPoint& canvasPoint )
{
  //PixelSelectorTool has registered a mouse click on the canvas, so bring the dialog back to the front
  raise();
  setModal( true );
  activateWindow();

  //Get the pixel values and add a new entry to the transparency table
  if ( mMapCanvas && mPixelSelectorTool )
  {
    QMap< QString, QString > myPixelMap;
    mMapCanvas->unsetMapTool( mPixelSelectorTool );
    mRasterLayer->identify( mMapCanvas->mapRenderer()->mapToLayerCoordinates( mRasterLayer, canvasPoint ), myPixelMap );
    if ( tableTransparency->columnCount() == 2 )
    {
      QString myValue = myPixelMap[ mRasterLayer->grayBandName()];
      if ( myValue != tr( "out of extent" ) )
      {
        tableTransparency->insertRow( tableTransparency->rowCount() );
        tableTransparency->setItem( tableTransparency->rowCount() - 1, tableTransparency->columnCount() - 1, new QTableWidgetItem( "100.0" ) );
        tableTransparency->setItem( tableTransparency->rowCount() - 1, 0, new QTableWidgetItem( myValue ) );
      }
    }
    else
    {
      QString myValue = myPixelMap[ mRasterLayer->redBandName()];
      if ( myValue != tr( "out of extent" ) )
      {
        tableTransparency->insertRow( tableTransparency->rowCount() );
        tableTransparency->setItem( tableTransparency->rowCount() - 1, tableTransparency->columnCount() - 1, new QTableWidgetItem( "100.0" ) );
        tableTransparency->setItem( tableTransparency->rowCount() - 1, 0, new QTableWidgetItem( myValue ) );
        tableTransparency->setItem( tableTransparency->rowCount() - 1, 1, new QTableWidgetItem( myPixelMap[ mRasterLayer->greenBandName()] ) );
        tableTransparency->setItem( tableTransparency->rowCount() - 1, 2, new QTableWidgetItem( myPixelMap[ mRasterLayer->blueBandName()] ) );
      }
    }
  }

}

void QgsRasterLayerProperties::sboxSingleBandStdDev_valueChanged( double theValue )
{
  Q_UNUSED( theValue );

  if ( !ignoreSpinBoxEvent )
  {
    leGrayMin->setText( "" );
    leGrayMax->setText( "" );
  }
  else
    ignoreSpinBoxEvent = false;
}

void QgsRasterLayerProperties::sboxThreeBandStdDev_valueChanged( double theValue )
{
  Q_UNUSED( theValue );
  if ( !ignoreSpinBoxEvent )
  {
    leRedMin->setText( "" );
    leRedMax->setText( "" );
    leGreenMin->setText( "" );
    leGreenMax->setText( "" );
    leBlueMin->setText( "" );
    leBlueMax->setText( "" );
  }
  else
    ignoreSpinBoxEvent = false;
}

void QgsRasterLayerProperties::sliderTransparency_valueChanged( int theValue )
{
  //set the transparency percentage label to a suitable value
  int myInt = static_cast < int >(( theValue / 255.0 ) * 100 );  //255.0 to prevent integer division
  lblTransparencyPercent->setText( QString::number( myInt ) + "%" );
}//sliderTransparency_valueChanged

void QgsRasterLayerProperties::userDefinedMinMax_textEdited( QString theString )
{
  Q_UNUSED( theString );
  /*
   * If all min max values are set and valid, then reset stdDev to 0.0
   */
  if ( rbtnThreeBand->isChecked() )
  {
    if ( validUserDefinedMinMax() && sboxThreeBandStdDev->value() != 0.0 )
    {
      ignoreSpinBoxEvent = true;
      sboxThreeBandStdDev->setValue( 0.0 );
    }
    mRGBMinimumMaximumEstimated = true;
  }
  else
  {
    if ( validUserDefinedMinMax() && sboxSingleBandStdDev->value() != 0.0 )
    {
      ignoreSpinBoxEvent = true;
      sboxSingleBandStdDev->setValue( 0.0 );
    }
    mGrayMinimumMaximumEstimated = true;
  }
  setMinimumMaximumEstimateWarning();
}

void QgsRasterLayerProperties::on_mClassifyButton_clicked()
{
  QgsRasterBandStats myRasterBandStats = mRasterLayer->bandStatistics( 1 );
  int numberOfEntries = sboxNumberOfEntries->value();

  std::list<double> entryValues;
  std::list<QColor> entryColors;

  if ( cboxClassificationMode->currentText() == tr( "Equal interval" ) )
  {
    double currentValue = myRasterBandStats.minimumValue;
    double intervalDiff;
    if ( numberOfEntries > 1 )
    {
      //because the highest value is also an entry, there are (numberOfEntries - 1)
      //intervals
      intervalDiff = ( myRasterBandStats.maximumValue - myRasterBandStats.minimumValue ) /
                     ( numberOfEntries - 1 );
    }
    else
    {
      intervalDiff = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
    }

    for ( int i = 0; i < numberOfEntries; ++i )
    {
      entryValues.push_back( currentValue );
      currentValue += intervalDiff;
    }
  }
#if 0
  else if ( cboxClassificationMode->currentText() == tr( "Quantiles" ) )
  {
    //todo
  }
#endif

  //hard code color range from blue -> red for now. Allow choice of ramps in future
  int colorDiff = 0;
  if ( numberOfEntries != 0 )
  {
    colorDiff = ( int )( 255 / numberOfEntries );
  }

  for ( int i = 0; i < numberOfEntries; ++i )
  {
    QColor currentColor;
    currentColor.setRgb( colorDiff*i, 0, 255 - colorDiff * i );
    entryColors.push_back( currentColor );
  }

  mColormapTreeWidget->clear();

  std::list<double>::const_iterator value_it = entryValues.begin();
  std::list<QColor>::const_iterator color_it = entryColors.begin();

  for ( ; value_it != entryValues.end(); ++value_it, ++color_it )
  {
    QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
    newItem->setText( 0, QString::number( *value_it, 'f' ) );
    newItem->setBackground( 1, QBrush( *color_it ) );
    newItem->setText( 2, QString::number( *value_it, 'f' ) );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
  }
}

void QgsRasterLayerProperties::on_mDeleteEntryButton_clicked()
{
  QTreeWidgetItem* currentItem = mColormapTreeWidget->currentItem();
  if ( currentItem )
  {
    delete currentItem;
  }
}

void QgsRasterLayerProperties::handleColormapTreeWidgetDoubleClick( QTreeWidgetItem* item, int column )
{
  if ( item )
  {
    if ( column == 1 )
    {
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      //show color dialog
      QColor newColor = QColorDialog::getColor( item->background( column ).color() );
      if ( newColor.isValid() )
      {
        item->setBackground( 1, QBrush( newColor ) );
      }
    }
    else
    {
      item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
    }
  }
}

void QgsRasterLayerProperties::on_pbtnAddColorMapEntry_clicked()
{
  QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
  newItem->setText( 0, "0.0" );
  newItem->setBackground( 1, QBrush( QColor( Qt::magenta ) ) );
  newItem->setText( 2, tr( "Custom color map entry" ) );
}

void QgsRasterLayerProperties::on_pbtnExportColorMapToFile_clicked()
{
  QSettings myQSettings;
  QString myLastDir = myQSettings.value( "lastRasterFileFilterDir", "" ).toString();
  QString myFileName = QFileDialog::getSaveFileName( this, tr( "Save file" ), myLastDir, tr( "Textfile (*.txt)" ) );
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
      myOutputStream << "# " << tr( "QGIS Generated Color Map Export File" ) << "\n";
      myOutputStream << "INTERPOLATION:";
      if ( cboxColorInterpolation->currentText() == tr( "Linear" ) )
      {
        myOutputStream << "INTERPOLATED\n";
      }
      else if ( cboxColorInterpolation->currentText() == tr( "Discrete" ) )
      {
        myOutputStream << "DISCRETE\n";
      }
      else
      {
        myOutputStream << "EXACT\n";
      }

      int myTopLevelItemCount = mColormapTreeWidget->topLevelItemCount();
      QTreeWidgetItem* myCurrentItem;
      QColor myColor;
      for ( int i = 0; i < myTopLevelItemCount; ++i )
      {
        myCurrentItem = mColormapTreeWidget->topLevelItem( i );
        if ( !myCurrentItem )
        {
          continue;
        }
        myColor = myCurrentItem->background( 1 ).color();
        myOutputStream << myCurrentItem->text( 0 ).toDouble() << ",";
        myOutputStream << myColor.red() << "," << myColor.green() << "," << myColor.blue() << "," << myColor.alpha() << ",";
        if ( myCurrentItem->text( 2 ) == "" )
        {
          myOutputStream << "Color entry " << i + 1 << "\n";
        }
        else
        {
          myOutputStream << myCurrentItem->text( 2 ) << "\n";
        }
      }
      myOutputStream.flush();
      myOutputFile.close();
    }
    else
    {
      QMessageBox::warning( this, tr( "Write access denied" ), tr( "Write access denied. Adjust the file permissions and try again.\n\n" ) );
    }
  }
}

void QgsRasterLayerProperties::on_pbtnLoadColorMapFromBand_clicked()
{
  QList<QgsColorRampShader::ColorRampItem> myColorRampList;
  if ( mRasterLayer->readColorTable( cboxColorMapBand->currentIndex() + 1, &myColorRampList ) )
  {
    populateColorMapTable( myColorRampList );
    cboxColorInterpolation->setCurrentIndex( cboxColorInterpolation->findText( tr( "Linear" ) ) );
    QgsDebugMsg( "Color map loaded" );
  }
  else
  {
    QMessageBox::warning( this, tr( "Load Color Map" ), tr( "The color map for band %1 failed to load" ).arg( cboxColorMapBand->currentIndex() + 1 ) );
    QgsDebugMsg( "Color map failed to load" );
  }
}

void QgsRasterLayerProperties::on_pbtnLoadColorMapFromFile_clicked()
{
  int myLineCounter = 0;
  bool myImportError = false;
  QString myBadLines;
  QSettings myQSettings;
  QString myLastDir = myQSettings.value( "lastRasterFileFilterDir", "" ).toString();
  QString myFileName = QFileDialog::getOpenFileName( this, tr( "Open file" ), myLastDir, tr( "Textfile (*.txt)" ) );
  QFile myInputFile( myFileName );
  if ( myInputFile.open( QFile::ReadOnly ) )
  {
    //clear the current tree
    mColormapTreeWidget->clear();

    QTextStream myInputStream( &myInputFile );
    QString myInputLine;
    QStringList myInputStringComponents;

    //read through the input looking for valid data
    while ( !myInputStream.atEnd() )
    {
      myLineCounter++;
      myInputLine = myInputStream.readLine();
      if ( !myInputLine.isEmpty() )
      {
        if ( !myInputLine.simplified().startsWith( "#" ) )
        {
          if ( myInputLine.contains( "INTERPOLATION", Qt::CaseInsensitive ) )
          {
            myInputStringComponents = myInputLine.split( ":" );
            if ( myInputStringComponents.size() == 2 )
            {
              if ( myInputStringComponents[1].trimmed().toUpper().compare( "INTERPOLATED", Qt::CaseInsensitive ) == 0 )
              {
                cboxColorInterpolation->setCurrentIndex( cboxColorInterpolation->findText( tr( "Linear" ) ) );
              }
              else if ( myInputStringComponents[1].trimmed().toUpper().compare( "DISCRETE", Qt::CaseInsensitive ) == 0 )
              {
                cboxColorInterpolation->setCurrentIndex( cboxColorInterpolation->findText( tr( "Discrete" ) ) );
              }
              else
              {
                cboxColorInterpolation->setCurrentIndex( cboxColorInterpolation->findText( tr( "Exact" ) ) );
              }
            }
            else
            {
              myImportError = true;
              myBadLines = myBadLines + QString::number( myLineCounter ) + ":\t[" + myInputLine + "]\n";
            }
          }
          else
          {
            myInputStringComponents = myInputLine.split( "," );
            if ( myInputStringComponents.size() == 6 )
            {
              QTreeWidgetItem* newItem = new QTreeWidgetItem( mColormapTreeWidget );
              newItem->setText( 0, myInputStringComponents[0] );
              newItem->setBackground( 1, QBrush( QColor::fromRgb( myInputStringComponents[1].toInt(), myInputStringComponents[2].toInt(), myInputStringComponents[3].toInt(), myInputStringComponents[4].toInt() ) ) );
              newItem->setText( 2, myInputStringComponents[5] );
            }
            else
            {
              myImportError = true;
              myBadLines = myBadLines + QString::number( myLineCounter ) + ":\t[" + myInputLine + "]\n";
            }
          }
        }
      }
      myLineCounter++;
    }


    if ( myImportError )
    {
      QMessageBox::warning( this, tr( "Import Error" ), tr( "The following lines contained errors\n\n" ) + myBadLines );
    }
  }
  else if ( !myFileName.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Read access denied" ), tr( "Read access denied. Adjust the file permissions and try again.\n\n" ) );
  }
}

void QgsRasterLayerProperties::on_pbtnLoadMinMax_clicked()
{
  if ( mRasterLayer->drawingStyle() == QgsRasterLayer::SingleBandGray
       || mRasterLayer->drawingStyle() == QgsRasterLayer::MultiBandSingleBandGray
       || mRasterLayer->drawingStyle() == QgsRasterLayer::MultiBandColor )
  {
    QgsRasterBandStats myRasterBandStats;
    double myMinimumMaximum[2];
    myMinimumMaximum[0] = 0;
    myMinimumMaximum[1] = 0;
    if ( rbtnThreeBand->isChecked() )
    {
      rbtnThreeBandMinMax->setChecked( true );

      if ( rbtnActualMinMax->isChecked() )
      {
        myRasterBandStats = mRasterLayer->bandStatistics( mRasterLayer->bandNumber( cboRed->currentText() ) );
        leRedMin->setText( QString::number( myRasterBandStats.minimumValue ) );
        leRedMax->setText( QString::number( myRasterBandStats.maximumValue ) );
        myRasterBandStats = mRasterLayer->bandStatistics( mRasterLayer->bandNumber( cboGreen->currentText() ) );
        leGreenMin->setText( QString::number( myRasterBandStats.minimumValue ) );
        leGreenMax->setText( QString::number( myRasterBandStats.maximumValue ) );
        myRasterBandStats = mRasterLayer->bandStatistics( mRasterLayer->bandNumber( cboBlue->currentText() ) );
        leBlueMin->setText( QString::number( myRasterBandStats.minimumValue ) );
        leBlueMax->setText( QString::number( myRasterBandStats.maximumValue ) );
        mRGBMinimumMaximumEstimated = false;
      }
      else if ( rbtnExtentMinMax->isChecked() )
      {
        mRasterLayer->computeMinimumMaximumFromLastExtent( mRasterLayer->bandNumber( cboRed->currentText() ), myMinimumMaximum );
        leRedMin->setText( QString::number( myMinimumMaximum[0] ) );
        leRedMax->setText( QString::number( myMinimumMaximum[1] ) );
        mRasterLayer->computeMinimumMaximumFromLastExtent( mRasterLayer->bandNumber( cboGreen->currentText() ), myMinimumMaximum );
        leGreenMin->setText( QString::number( myMinimumMaximum[0] ) );
        leGreenMax->setText( QString::number( myMinimumMaximum[1] ) );
        mRasterLayer->computeMinimumMaximumFromLastExtent( mRasterLayer->bandNumber( cboBlue->currentText() ), myMinimumMaximum );
        leBlueMin->setText( QString::number( myMinimumMaximum[0] ) );
        leBlueMax->setText( QString::number( myMinimumMaximum[1] ) );
        mRGBMinimumMaximumEstimated = true;
      }
      else
      {
        rbtnEstimateMinMax->setChecked( true );
        mRasterLayer->computeMinimumMaximumEstimates( mRasterLayer->bandNumber( cboRed->currentText() ), myMinimumMaximum );
        leRedMin->setText( QString::number( myMinimumMaximum[0] ) );
        leRedMax->setText( QString::number( myMinimumMaximum[1] ) );
        mRasterLayer->computeMinimumMaximumEstimates( mRasterLayer->bandNumber( cboGreen->currentText() ), myMinimumMaximum );
        leGreenMin->setText( QString::number( myMinimumMaximum[0] ) );
        leGreenMax->setText( QString::number( myMinimumMaximum[1] ) );
        mRasterLayer->computeMinimumMaximumEstimates( mRasterLayer->bandNumber( cboBlue->currentText() ), myMinimumMaximum );
        leBlueMin->setText( QString::number( myMinimumMaximum[0] ) );
        leBlueMax->setText( QString::number( myMinimumMaximum[1] ) );
        mRGBMinimumMaximumEstimated = true;
      }

    }
    else
    {
      rbtnSingleBandMinMax->setChecked( true );
      if ( rbtnActualMinMax->isChecked() )
      {
        myRasterBandStats = mRasterLayer->bandStatistics( mRasterLayer->bandNumber( cboGray->currentText() ) );
        leGrayMin->setText( QString::number( myRasterBandStats.minimumValue ) );
        leGrayMax->setText( QString::number( myRasterBandStats.maximumValue ) );
        mGrayMinimumMaximumEstimated = false;
      }
      else if ( rbtnExtentMinMax->isChecked() )
      {
        mRasterLayer->computeMinimumMaximumFromLastExtent( mRasterLayer->bandNumber( cboGray->currentText() ), myMinimumMaximum );
        leGrayMin->setText( QString::number( myMinimumMaximum[0] ) );
        leGrayMax->setText( QString::number( myMinimumMaximum[1] ) );
        mGrayMinimumMaximumEstimated = true;
      }
      else
      {
        rbtnEstimateMinMax->setChecked( true );
        mRasterLayer->computeMinimumMaximumEstimates( mRasterLayer->bandNumber( cboGray->currentText() ), myMinimumMaximum );
        leGrayMin->setText( QString::number( myMinimumMaximum[0] ) );
        leGrayMax->setText( QString::number( myMinimumMaximum[1] ) );
        mGrayMinimumMaximumEstimated = true;
      }
    }
    setMinimumMaximumEstimateWarning();
  }
}

void QgsRasterLayerProperties::on_pbtnMakeBandCombinationDefault_clicked()
{
  mDefaultRedBand = cboRed->currentIndex() + 1;
  mDefaultGreenBand = cboGreen->currentIndex() + 1;
  mDefaultBlueBand = cboBlue->currentIndex() + 1;
  labelDefaultBandCombination->setText( tr( "Default R:%1 G:%2 B:%3" ).arg( mDefaultRedBand ).arg( mDefaultGreenBand ).arg( mDefaultBlueBand ) );
}

void QgsRasterLayerProperties::on_pbtnMakeContrastEnhancementAlgorithmDefault_clicked()
{
  if ( cboxContrastEnhancementAlgorithm->currentText() != tr( "User Defined" ) )
  {
    if ( cboxContrastEnhancementAlgorithm->currentText() == tr( "No Stretch" ) )
    {
      mDefaultContrastEnhancementAlgorithm = "NoEnhancement";
      labelDefaultContrastEnhancementAlgorithm->setText( cboxContrastEnhancementAlgorithm->currentText() );
    }
    else if ( cboxContrastEnhancementAlgorithm->currentText() == tr( "Stretch To MinMax" ) )
    {
      mDefaultContrastEnhancementAlgorithm = "StretchToMinimumMaximum";
      labelDefaultContrastEnhancementAlgorithm->setText( cboxContrastEnhancementAlgorithm->currentText() );
    }
    else if ( cboxContrastEnhancementAlgorithm->currentText() == tr( "Stretch And Clip To MinMax" ) )
    {
      mDefaultContrastEnhancementAlgorithm =  "StretchAndClipToMinimumMaximum";
      labelDefaultContrastEnhancementAlgorithm->setText( cboxContrastEnhancementAlgorithm->currentText() );
    }
    else if ( cboxContrastEnhancementAlgorithm->currentText() == tr( "Clip To MinMax" ) )
    {
      mDefaultContrastEnhancementAlgorithm = "ClipToMinimumMaximum";
      labelDefaultContrastEnhancementAlgorithm->setText( cboxContrastEnhancementAlgorithm->currentText() );
    }
    else
    {
      //do nothing
    }
  }
}

void QgsRasterLayerProperties::on_pbtnMakeStandardDeviationDefault_clicked()
{
  mDefaultStandardDeviation = sboxThreeBandStdDev->value();
}

void QgsRasterLayerProperties::on_pbtnSortColorMap_clicked()
{
  bool inserted = false;
  int myCurrentIndex = 0;
  int myTopLevelItemCount = mColormapTreeWidget->topLevelItemCount();
  QTreeWidgetItem* myCurrentItem;
  QList<QgsColorRampShader::ColorRampItem> myColorRampItems;
  for ( int i = 0; i < myTopLevelItemCount; ++i )
  {
    myCurrentItem = mColormapTreeWidget->topLevelItem( i );
    //If the item is null or does not have a pixel values set, skip
    if ( !myCurrentItem || myCurrentItem->text( 0 ) == "" )
    {
      continue;
    }

    //Create a copy of the new Color ramp Item
    QgsColorRampShader::ColorRampItem myNewColorRampItem;
    myNewColorRampItem.value = myCurrentItem->text( 0 ).toDouble();
    myNewColorRampItem.color = myCurrentItem->background( 1 ).color();
    myNewColorRampItem.label = myCurrentItem->text( 2 );

    //Simple insertion sort - speed is not a huge factor here
    inserted = false;
    myCurrentIndex = 0;
    while ( !inserted )
    {
      if ( 0 == myColorRampItems.size() || myCurrentIndex == myColorRampItems.size() )
      {
        myColorRampItems.push_back( myNewColorRampItem );
        inserted = true;
      }
      else if ( myColorRampItems[myCurrentIndex].value > myNewColorRampItem.value )
      {
        myColorRampItems.insert( myCurrentIndex, myNewColorRampItem );
        inserted = true;
      }
      else if ( myColorRampItems[myCurrentIndex].value <= myNewColorRampItem.value  && myCurrentIndex == myColorRampItems.size() - 1 )
      {
        myColorRampItems.push_back( myNewColorRampItem );
        inserted = true;
      }
      else if ( myColorRampItems[myCurrentIndex].value <= myNewColorRampItem.value && myColorRampItems[myCurrentIndex+1].value > myNewColorRampItem.value )
      {
        myColorRampItems.insert( myCurrentIndex + 1, myNewColorRampItem );
        inserted = true;
      }
      myCurrentIndex++;
    }
  }
  populateColorMapTable( myColorRampItems );
}

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
    //it worked so do it quietly
    sync();
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
  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", "." ).toString();

  //create a file dialog
  std::auto_ptr < QFileDialog > myFileDialog
  (
    new QFileDialog(
      this,
      QFileDialog::tr( "Load layer properties from style file (.qml)" ),
      myLastUsedDir,
      tr( "QGIS Layer Style File (*.qml)" )
    )
  );
  myFileDialog->setFileMode( QFileDialog::AnyFile );
  myFileDialog->setAcceptMode( QFileDialog::AcceptOpen );

  //prompt the user for a file name
  QString myFileName;
  if ( myFileDialog->exec() == QDialog::Accepted )
  {
    QStringList myFiles = myFileDialog->selectedFiles();
    if ( !myFiles.isEmpty() )
    {
      myFileName = myFiles[0];
    }
  }

  if ( !myFileName.isEmpty() )
  {
    if ( myFileDialog->selectedFilter() == tr( "QGIS Layer Style File (*.qml)" ) )
    {
      //ensure the user never omitted the extension from the file name
      if ( !myFileName.toUpper().endsWith( ".QML" ) )
      {
        myFileName += ".qml";
      }
      bool defaultLoadedFlag = false;
      QString myMessage = mRasterLayer->loadNamedStyle( myFileName, defaultLoadedFlag );
      //reset if the default style was loaded ok only
      if ( defaultLoadedFlag )
      {
        sync();
      }
      else
      {
        //let the user know something went wrong...
        QMessageBox::information( this,
                                  tr( "Saved Style" ),
                                  myMessage
                                );
      }
    }
    else
    {
      QMessageBox::warning( this, tr( "QGIS" ), tr( "Unknown style format: %1" ).arg( myFileDialog->selectedFilter() ) );

    }
    myQSettings.setValue( "style/lastStyleDir", myFileDialog->directory().absolutePath() );
  }
}


void QgsRasterLayerProperties::on_pbnSaveStyleAs_clicked()
{

  QSettings myQSettings;  // where we keep last used filter in persistent state
  QString myLastUsedDir = myQSettings.value( "style/lastStyleDir", "." ).toString();

  //create a file dialog
  std::auto_ptr < QFileDialog > myFileDialog
  (
    new QFileDialog(
      this,
      QFileDialog::tr( "Save layer properties as style file (.qml)" ),
      myLastUsedDir,
      tr( "QGIS Layer Style File (*.qml)" )
    )
  );
  myFileDialog->setFileMode( QFileDialog::AnyFile );
  myFileDialog->setAcceptMode( QFileDialog::AcceptSave );

  //prompt the user for a file name
  QString myOutputFileName;
  if ( myFileDialog->exec() == QDialog::Accepted )
  {
    QStringList myFiles = myFileDialog->selectedFiles();
    if ( !myFiles.isEmpty() )
    {
      myOutputFileName = myFiles[0];
    }
  }

  if ( !myOutputFileName.isEmpty() )
  {
    if ( myFileDialog->selectedFilter() == tr( "QGIS Layer Style File (*.qml)" ) )
    {
      //ensure the user never omitted the extension from the file name
      if ( !myOutputFileName.toUpper().endsWith( ".QML" ) )
      {
        myOutputFileName += ".qml";
      }
      bool defaultLoadedFlag = false;
      QString myMessage = mRasterLayer->saveNamedStyle( myOutputFileName, defaultLoadedFlag );
      //reset if the default style was loaded ok only
      if ( defaultLoadedFlag )
      {
        //don't show the message if all went well...
        sync();
      }
      else
      {
        //if something went wrong let the user know why
        QMessageBox::information( this,
                                  tr( "Saved Style" ),
                                  myMessage
                                );
      }
    }
    else
    {
      QMessageBox::warning( this, tr( "QGIS" ), tr( "Unknown style format: %1" ).arg( myFileDialog->selectedFilter() ) );

    }
    myQSettings.setValue( "style/lastStyleDir", myFileDialog->directory().absolutePath() );
  }
}

void QgsRasterLayerProperties::on_btnResetNull_clicked( )
{
  //If reset NoDataValue is checked do this first, will ignore what ever is in the LineEdit
  mRasterLayer->resetNoDataValue();
  if ( mRasterLayer->isNoDataValueValid() )
  {
    leNoDataValue->setText( QString::number( mRasterLayer->noDataValue(), 'f' ) );
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

