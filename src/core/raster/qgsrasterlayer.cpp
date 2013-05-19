/***************************************************************************
        qgsrasterlayer.cpp -  description
   -------------------
begin                : Sat Jun 22 2002
copyright            : (C) 2003 by Tim Sutton, Steve Halasz and Gary E.Sherman
email                : tim at linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgscolorrampshader.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsmessagelog.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgspalettedrasterrenderer.h"
#include "qgsprojectfiletransform.h"
#include "qgsproviderregistry.h"
#include "qgspseudocolorshader.h"
#include "qgsrasterdrawer.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include "qgsrasterprojector.h"
#include "qgsrasterrange.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgssinglebandcolordatarenderer.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"

#include <cmath>
#include <cstdio>
#include <limits>
#include <typeinfo>

#include <QApplication>
#include <QCursor>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QLibrary>
#include <QList>
#include <QMatrix>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QRegExp>
#include <QSettings>
#include <QSlider>
#include <QTime>

// typedefs for provider plugin functions of interest
typedef void buildsupportedrasterfilefilter_t( QString & theFileFiltersString );
typedef bool isvalidrasterfilename_t( QString const & theFileNameQString, QString & retErrMsg );

#define ERR(message) QGS_ERROR_MESSAGE(message,"Raster layer")

const double QgsRasterLayer::CUMULATIVE_CUT_LOWER = 0.02;
const double QgsRasterLayer::CUMULATIVE_CUT_UPPER = 0.98;
const double QgsRasterLayer::SAMPLE_SIZE = 250000;

QgsRasterLayer::QgsRasterLayer()
    : QgsMapLayer( RasterLayer )
    , QSTRING_NOT_SET( "Not Set" )
    , TRSTRING_NOT_SET( tr( "Not Set" ) )
    , mDataProvider( 0 )
{
  init();
  mValid = false;
}

QgsRasterLayer::QgsRasterLayer(
  QString const & path,
  QString const & baseName,
  bool loadDefaultStyleFlag )
    : QgsMapLayer( RasterLayer, baseName, path )
    , QSTRING_NOT_SET( "Not Set" )
    , TRSTRING_NOT_SET( tr( "Not Set" ) )
    , mDataProvider( 0 )
{
  QgsDebugMsg( "Entered" );

  // TODO, call constructor with provider key
  init();
  setDataProvider( "gdal" );
  if ( !mValid ) return;

  bool defaultLoadedFlag = false;
  if ( mValid && loadDefaultStyleFlag )
  {
    loadDefaultStyle( defaultLoadedFlag );
  }
  if ( !defaultLoadedFlag )
  {
    setDefaultContrastEnhancement();
  }
  return;
} // QgsRasterLayer ctor

/**
 * @todo Rename into a general constructor when the old raster interface is retired
 * parameter dummy is just there to distinguish this function signature from the old non-provider one.
 */
QgsRasterLayer::QgsRasterLayer( const QString & uri,
                                const QString & baseName,
                                const QString & providerKey,
                                bool loadDefaultStyleFlag )
    : QgsMapLayer( RasterLayer, baseName, uri )
    // Constant that signals property not used.
    , QSTRING_NOT_SET( "Not Set" )
    , TRSTRING_NOT_SET( tr( "Not Set" ) )
    , mDataProvider( 0 )
    , mProviderKey( providerKey )
{
  QgsDebugMsg( "Entered" );
  init();
  setDataProvider( providerKey );
  if ( !mValid ) return;

  // load default style
  bool defaultLoadedFlag = false;
  if ( mValid && loadDefaultStyleFlag )
  {
    loadDefaultStyle( defaultLoadedFlag );
  }
  if ( !defaultLoadedFlag )
  {
    setDefaultContrastEnhancement();
  }

  // TODO: Connect signals from the dataprovider to the qgisapp

  emit statusChanged( tr( "QgsRasterLayer created" ) );
} // QgsRasterLayer ctor

QgsRasterLayer::~QgsRasterLayer()
{
  mValid = false;
  // Note: provider and other interfaces are owned and deleted by pipe
}

//////////////////////////////////////////////////////////
//
// Static Methods and members
//
/////////////////////////////////////////////////////////
/**
  Builds the list of file filter strings to later be used by
  QgisApp::addRasterLayer()
  We query GDAL for a list of supported raster formats; we then build
  a list of file filter strings from that list.  We return a string
  that contains this list that is suitable for use in a
  QFileDialog::getOpenFileNames() call.
*/
void QgsRasterLayer::buildSupportedRasterFileFilter( QString & theFileFiltersString )
{
  QgsDebugMsg( "Entered" );
  buildsupportedrasterfilefilter_t *pBuild = ( buildsupportedrasterfilefilter_t * ) cast_to_fptr( QgsProviderRegistry::instance()->function( "gdal", "buildSupportedRasterFileFilter" ) );
  if ( ! pBuild )
  {
    QgsDebugMsg( "Could get buildSupportedRasterFileFilter in gdal provider library" );
    return;
  }

  pBuild( theFileFiltersString );
}

/**
 * This helper checks to see whether the file name appears to be a valid raster file name
 */
bool QgsRasterLayer::isValidRasterFileName( QString const & theFileNameQString, QString & retErrMsg )
{
  isvalidrasterfilename_t *pValid = ( isvalidrasterfilename_t * ) cast_to_fptr( QgsProviderRegistry::instance()->function( "gdal",  "isValidRasterFileName" ) );
  if ( ! pValid )
  {
    QgsDebugMsg( "Could not resolve isValidRasterFileName in gdal provider library" );
    return false;
  }

  bool myIsValid = pValid( theFileNameQString, retErrMsg );
  return myIsValid;
}

bool QgsRasterLayer::isValidRasterFileName( QString const & theFileNameQString )
{
  QString retErrMsg;
  return isValidRasterFileName( theFileNameQString, retErrMsg );
}

QDateTime QgsRasterLayer::lastModified( QString const & name )
{
  QgsDebugMsg( "name=" + name );
  QDateTime t;

  QFileInfo fi( name );

  // Is it file?
  if ( !fi.exists() )
    return t;

  t = fi.lastModified();

  QgsDebugMsg( "last modified = " + t.toString() );

  return t;
}

// typedef for the QgsDataProvider class factory
typedef QgsDataProvider * classFactoryFunction_t( const QString * );

//////////////////////////////////////////////////////////
//
// Non Static Public methods
//
/////////////////////////////////////////////////////////

int QgsRasterLayer::bandCount() const
{
  if ( !mDataProvider ) return 0;
  return mDataProvider->bandCount();
}

const QString QgsRasterLayer::bandName( int theBandNo )
{
  return dataProvider()->generateBandName( theBandNo );
}

void QgsRasterLayer::setRendererForDrawingStyle( const DrawingStyle &  theDrawingStyle )
{
  setRenderer( QgsRasterRendererRegistry::instance()->defaultRendererForDrawingStyle( theDrawingStyle, mDataProvider ) );
}

/**
 * @return 0 if not using the data provider model (i.e. directly using GDAL)
 */
QgsRasterDataProvider* QgsRasterLayer::dataProvider()
{
  return mDataProvider;
}

/**
 * @return 0 if not using the data provider model (i.e. directly using GDAL)
 */
const QgsRasterDataProvider* QgsRasterLayer::dataProvider() const
{
  return mDataProvider;
}

void QgsRasterLayer::reload()
{
  if ( mDataProvider )
  {
    mDataProvider->reloadData();
  }
}

bool QgsRasterLayer::draw( QgsRenderContext& rendererContext )
{
  QgsDebugMsg( "entered. (renderContext)" );

  // Don't waste time drawing if transparency is at 0 (completely transparent)
  if ( mTransparencyLevel == 0 )
    return true;

  QgsDebugMsg( "checking timestamp." );

  // Check timestamp
  if ( !update() )
  {
    return false;
  }

  const QgsMapToPixel& theQgsMapToPixel = rendererContext.mapToPixel();

  QgsRectangle myProjectedViewExtent;
  QgsRectangle myProjectedLayerExtent;

  if ( rendererContext.coordinateTransform() )
  {
    QgsDebugMsg( "coordinateTransform set -> project extents." );
    try
    {
      myProjectedViewExtent = rendererContext.coordinateTransform()->transformBoundingBox( rendererContext.extent() );
    }
    catch ( QgsCsException &cs )
    {
      QgsMessageLog::logMessage( tr( "Could not reproject view extent: %1" ).arg( cs.what() ), tr( "Raster" ) );
      myProjectedViewExtent.setMinimal();
    }

    try
    {
      myProjectedLayerExtent = rendererContext.coordinateTransform()->transformBoundingBox( extent() );
    }
    catch ( QgsCsException &cs )
    {
      QgsMessageLog::logMessage( tr( "Could not reproject layer extent: %1" ).arg( cs.what() ), tr( "Raster" ) );
      myProjectedViewExtent.setMinimal();
    }
  }
  else
  {
    QgsDebugMsg( "coordinateTransform not set" );
    myProjectedViewExtent = rendererContext.extent();
    myProjectedLayerExtent = extent();
  }

  QPainter* theQPainter = rendererContext.painter();

  if ( !theQPainter )
  {
    return false;
  }

  // clip raster extent to view extent
  QgsRectangle myRasterExtent = myProjectedViewExtent.intersect( &myProjectedLayerExtent );
  if ( myRasterExtent.isEmpty() )
  {
    QgsDebugMsg( "draw request outside view extent." );
    // nothing to do
    return true;
  }

  QgsDebugMsg( "theViewExtent is " + rendererContext.extent().toString() );
  QgsDebugMsg( "myProjectedViewExtent is " + myProjectedViewExtent.toString() );
  QgsDebugMsg( "myProjectedLayerExtent is " + myProjectedLayerExtent.toString() );
  QgsDebugMsg( "myRasterExtent is " + myRasterExtent.toString() );

  //
  // The first thing we do is set up the QgsRasterViewPort. This struct stores all the settings
  // relating to the size (in pixels and coordinate system units) of the raster part that is
  // in view in the map window. It also stores the origin.
  //
  //this is not a class level member because every time the user pans or zooms
  //the contents of the rasterViewPort will change
  QgsRasterViewPort *myRasterViewPort = new QgsRasterViewPort();

  myRasterViewPort->mDrawnExtent = myRasterExtent;
  if ( rendererContext.coordinateTransform() )
  {
    myRasterViewPort->mSrcCRS = crs();
    myRasterViewPort->mDestCRS = rendererContext.coordinateTransform()->destCRS();
  }
  else
  {
    myRasterViewPort->mSrcCRS = QgsCoordinateReferenceSystem(); // will be invalid
    myRasterViewPort->mDestCRS = QgsCoordinateReferenceSystem(); // will be invalid
  }

  // get dimensions of clipped raster image in device coordinate space (this is the size of the viewport)
  myRasterViewPort->mTopLeftPoint = theQgsMapToPixel.transform( myRasterExtent.xMinimum(), myRasterExtent.yMaximum() );
  myRasterViewPort->mBottomRightPoint = theQgsMapToPixel.transform( myRasterExtent.xMaximum(), myRasterExtent.yMinimum() );

  // align to output device grid, i.e. floor/ceil to integers
  // TODO: this should only be done if paint device is raster - screen, image
  // for other devices (pdf) it can have floating point origin
  // we could use floating point for raster devices as well, but respecting the
  // output device grid should make it more effective as the resampling is done in
  // the provider anyway
  if ( true )
  {
    myRasterViewPort->mTopLeftPoint.setX( floor( myRasterViewPort->mTopLeftPoint.x() ) );
    myRasterViewPort->mTopLeftPoint.setY( floor( myRasterViewPort->mTopLeftPoint.y() ) );
    myRasterViewPort->mBottomRightPoint.setX( ceil( myRasterViewPort->mBottomRightPoint.x() ) );
    myRasterViewPort->mBottomRightPoint.setY( ceil( myRasterViewPort->mBottomRightPoint.y() ) );
    // recalc myRasterExtent to aligned values
    myRasterExtent.set(
      theQgsMapToPixel.toMapCoordinatesF( myRasterViewPort->mTopLeftPoint.x(),
                                          myRasterViewPort->mBottomRightPoint.y() ),
      theQgsMapToPixel.toMapCoordinatesF( myRasterViewPort->mBottomRightPoint.x(),
                                          myRasterViewPort->mTopLeftPoint.y() )
    );

  }

  myRasterViewPort->mWidth = static_cast<int>( qAbs(( myRasterExtent.width() / theQgsMapToPixel.mapUnitsPerPixel() ) ) );
  myRasterViewPort->mHeight = static_cast<int>( qAbs(( myRasterExtent.height() / theQgsMapToPixel.mapUnitsPerPixel() ) ) );

  //the drawable area can start to get very very large when you get down displaying 2x2 or smaller, this is becasue
  //theQgsMapToPixel.mapUnitsPerPixel() is less then 1,
  //so we will just get the pixel data and then render these special cases differently in paintImageToCanvas()

  QgsDebugMsgLevel( QString( "mapUnitsPerPixel = %1" ).arg( theQgsMapToPixel.mapUnitsPerPixel() ), 3 );
  QgsDebugMsgLevel( QString( "mWidth = %1" ).arg( width() ), 3 );
  QgsDebugMsgLevel( QString( "mHeight = %1" ).arg( height() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.xMinimum() = %1" ).arg( myRasterExtent.xMinimum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.xMaximum() = %1" ).arg( myRasterExtent.xMaximum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.yMinimum() = %1" ).arg( myRasterExtent.yMinimum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.yMaximum() = %1" ).arg( myRasterExtent.yMaximum() ), 3 );

  QgsDebugMsgLevel( QString( "mTopLeftPoint.x() = %1" ).arg( myRasterViewPort->mTopLeftPoint.x() ), 3 );
  QgsDebugMsgLevel( QString( "mBottomRightPoint.x() = %1" ).arg( myRasterViewPort->mBottomRightPoint.x() ), 3 );
  QgsDebugMsgLevel( QString( "mTopLeftPoint.y() = %1" ).arg( myRasterViewPort->mTopLeftPoint.y() ), 3 );
  QgsDebugMsgLevel( QString( "mBottomRightPoint.y() = %1" ).arg( myRasterViewPort->mBottomRightPoint.y() ), 3 );

  QgsDebugMsgLevel( QString( "mWidth = %1" ).arg( myRasterViewPort->mWidth ), 3 );
  QgsDebugMsgLevel( QString( "mHeight = %1" ).arg( myRasterViewPort->mHeight ), 3 );

  // /\/\/\ - added to handle zoomed-in rasters

  mLastViewPort = *myRasterViewPort;

  // TODO: is it necessary? Probably WMS only?
  mDataProvider->setDpi( rendererContext.rasterScaleFactor() * 25.4 * rendererContext.scaleFactor() );

  draw( theQPainter, myRasterViewPort, &theQgsMapToPixel );

  delete myRasterViewPort;
  QgsDebugMsg( "exiting." );

  return true;

}

void QgsRasterLayer::draw( QPainter * theQPainter,
                           QgsRasterViewPort * theRasterViewPort,
                           const QgsMapToPixel* theQgsMapToPixel )
{
  QgsDebugMsg( " 3 arguments" );
  QTime time;
  time.start();
  //
  //
  // The goal here is to make as many decisions as possible early on (outside of the rendering loop)
  // so that we can maximise performance of the rendering process. So now we check which drawing
  // procedure to use :
  //

  QgsRasterProjector *projector = mPipe.projector();

  // TODO add a method to interface to get provider and get provider
  // params in QgsRasterProjector
  if ( projector )
  {
    projector->setCRS( theRasterViewPort->mSrcCRS, theRasterViewPort->mDestCRS );
  }

  // Drawer to pipe?
  QgsRasterIterator iterator( mPipe.last() );
  QgsRasterDrawer drawer( &iterator );
  drawer.draw( theQPainter, theRasterViewPort, theQgsMapToPixel );

  QgsDebugMsg( QString( "total raster draw time (ms):     %1" ).arg( time.elapsed(), 5 ) );
} //end of draw method

QString QgsRasterLayer::lastError()
{
  return mError;
}

QString QgsRasterLayer::lastErrorTitle()
{
  return mErrorCaption;
}

QList< QPair< QString, QColor > > QgsRasterLayer::legendSymbologyItems() const
{
  QList< QPair< QString, QColor > > symbolList;
  QgsRasterRenderer *renderer = mPipe.renderer();
  if ( renderer )
  {
    renderer->legendSymbologyItems( symbolList );
  }
  return symbolList;
}

QString QgsRasterLayer::metadata()
{
  QString myMetadata ;
  myMetadata += "<p class=\"glossy\">" + tr( "Driver" ) + "</p>\n";
  myMetadata += "<p>";
  myMetadata += mDataProvider->description();
  myMetadata += "</p>\n";

  // Insert provider-specific (e.g. WMS-specific) metadata
  // crashing
  myMetadata += mDataProvider->metadata();

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "No Data Value" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  // TODO: all bands
  if ( mDataProvider->srcHasNoDataValue( 1 ) )
  {
    myMetadata += QString::number( mDataProvider->srcNoDataValue( 1 ) );
  }
  else
  {
    myMetadata += "*" + tr( "NoDataValue not set" ) + "*";
  }
  myMetadata += "</p>\n";

  myMetadata += "</p>\n";
  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Data Type" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  //just use the first band
  switch ( mDataProvider->srcDataType( 1 ) )
  {
    case QGis::Byte:
      myMetadata += tr( "Byte - Eight bit unsigned integer" );
      break;
    case QGis::UInt16:
      myMetadata += tr( "UInt16 - Sixteen bit unsigned integer " );
      break;
    case QGis::Int16:
      myMetadata += tr( "Int16 - Sixteen bit signed integer " );
      break;
    case QGis::UInt32:
      myMetadata += tr( "UInt32 - Thirty two bit unsigned integer " );
      break;
    case QGis::Int32:
      myMetadata += tr( "Int32 - Thirty two bit signed integer " );
      break;
    case QGis::Float32:
      myMetadata += tr( "Float32 - Thirty two bit floating point " );
      break;
    case QGis::Float64:
      myMetadata += tr( "Float64 - Sixty four bit floating point " );
      break;
    case QGis::CInt16:
      myMetadata += tr( "CInt16 - Complex Int16 " );
      break;
    case QGis::CInt32:
      myMetadata += tr( "CInt32 - Complex Int32 " );
      break;
    case QGis::CFloat32:
      myMetadata += tr( "CFloat32 - Complex Float32 " );
      break;
    case QGis::CFloat64:
      myMetadata += tr( "CFloat64 - Complex Float64 " );
      break;
    default:
      myMetadata += tr( "Could not determine raster data type." );
  }
  myMetadata += "</p>\n";

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Pyramid overviews" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Layer Spatial Reference System" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata += crs().toProj4();
  myMetadata += "</p>\n";

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Layer Extent (layer original source projection)" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata += mDataProvider->extent().toString();
  myMetadata += "</p>\n";

  // output coordinate system
  // TODO: this is not related to layer, to be removed? [MD]
#if 0
  myMetadata += "<tr><td class=\"glossy\">";
  myMetadata += tr( "Project Spatial Reference System" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata +=  mCoordinateTransform->destCRS().toProj4();
  myMetadata += "</p>\n";
#endif

  //
  // Add the stats for each band to the output table
  //
  int myBandCountInt = bandCount();
  for ( int myIteratorInt = 1; myIteratorInt <= myBandCountInt; ++myIteratorInt )
  {
    QgsDebugMsg( "Raster properties : checking if band " + QString::number( myIteratorInt ) + " has stats? " );
    //band name
    myMetadata += "<p class=\"glossy\">\n";
    myMetadata += tr( "Band" );
    myMetadata += "</p>\n";
    myMetadata += "<p>";
    myMetadata += bandName( myIteratorInt );
    myMetadata += "</p>\n";
    //band number
    myMetadata += "<p>";
    myMetadata += tr( "Band No" );
    myMetadata += "</p>\n";
    myMetadata += "<p>\n";
    myMetadata += QString::number( myIteratorInt );
    myMetadata += "</p>\n";

    //check if full stats for this layer have already been collected
    if ( !dataProvider()->hasStatistics( myIteratorInt ) )  //not collected
    {
      QgsDebugMsg( ".....no" );

      myMetadata += "<p>";
      myMetadata += tr( "No Stats" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += tr( "No stats collected yet" );
      myMetadata += "</p>\n";
    }
    else                    // collected - show full detail
    {
      QgsDebugMsg( ".....yes" );

      QgsRasterBandStats myRasterBandStats = dataProvider()->bandStatistics( myIteratorInt );
      //Min Val
      myMetadata += "<p>";
      myMetadata += tr( "Min Val" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += QString::number( myRasterBandStats.minimumValue, 'f', 10 );
      myMetadata += "</p>\n";

      // Max Val
      myMetadata += "<p>";
      myMetadata += tr( "Max Val" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += QString::number( myRasterBandStats.maximumValue, 'f', 10 );
      myMetadata += "</p>\n";

      // Range
      myMetadata += "<p>";
      myMetadata += tr( "Range" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += QString::number( myRasterBandStats.range, 'f', 10 );
      myMetadata += "</p>\n";

      // Mean
      myMetadata += "<p>";
      myMetadata += tr( "Mean" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += QString::number( myRasterBandStats.mean, 'f', 10 );
      myMetadata += "</p>\n";

      //sum of squares
      myMetadata += "<p>";
      myMetadata += tr( "Sum of squares" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += QString::number( myRasterBandStats.sumOfSquares, 'f', 10 );
      myMetadata += "</p>\n";

      //standard deviation
      myMetadata += "<p>";
      myMetadata += tr( "Standard Deviation" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += QString::number( myRasterBandStats.stdDev, 'f', 10 );
      myMetadata += "</p>\n";

      //sum of all cells
      myMetadata += "<p>";
      myMetadata += tr( "Sum of all cells" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += QString::number( myRasterBandStats.sum, 'f', 10 );
      myMetadata += "</p>\n";

      //number of cells
      myMetadata += "<p>";
      myMetadata += tr( "Cell Count" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += QString::number( myRasterBandStats.elementCount );
      myMetadata += "</p>\n";
    }
  }

  QgsDebugMsg( myMetadata );
  return myMetadata;
}

/**
 * @param theBandNumber the number of the band to use for generating a pixmap of the associated palette
 * @return a 100x100 pixel QPixmap of the bands palette
 */
QPixmap QgsRasterLayer::paletteAsPixmap( int theBandNumber )
{
  //TODO: This function should take dimensions
  QgsDebugMsg( "entered." );

  // Only do this for the GDAL provider?
  // Maybe WMS can do this differently using QImage::numColors and QImage::color()
  if ( mDataProvider->colorInterpretation( theBandNumber ) == QgsRaster::PaletteIndex )
  {
    QgsDebugMsg( "....found paletted image" );
    QgsColorRampShader myShader;
    QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = mDataProvider->colorTable( theBandNumber );
    if ( myColorRampItemList.size() > 0 )
    {
      QgsDebugMsg( "....got color ramp item list" );
      myShader.setColorRampItemList( myColorRampItemList );
      myShader.setColorRampType( QgsColorRampShader::DISCRETE );
      // Draw image
      int mySize = 100;
      QPixmap myPalettePixmap( mySize, mySize );
      QPainter myQPainter( &myPalettePixmap );

      QImage myQImage = QImage( mySize, mySize, QImage::Format_RGB32 );
      myQImage.fill( 0 );
      myPalettePixmap.fill();

      double myStep = (( double )myColorRampItemList.size() - 1 ) / ( double )( mySize * mySize );
      double myValue = 0.0;
      for ( int myRow = 0; myRow < mySize; myRow++ )
      {
        QRgb* myLineBuffer = ( QRgb* )myQImage.scanLine( myRow );
        for ( int myCol = 0; myCol < mySize; myCol++ )
        {
          myValue = myStep * ( double )( myCol + myRow * mySize );
          int c1, c2, c3, c4;
          myShader.shade( myValue, &c1, &c2, &c3, &c4 );
          myLineBuffer[ myCol ] = qRgba( c1, c2, c3, c4 );
        }
      }

      myQPainter.drawImage( 0, 0, myQImage );
      return myPalettePixmap;
    }
    QPixmap myNullPixmap;
    return myNullPixmap;
  }
  else
  {
    //invalid layer  was requested
    QPixmap myNullPixmap;
    return myNullPixmap;
  }
}

QString QgsRasterLayer::providerType() const
{
  return mProviderKey;
}

/**
 * @return the horizontal units per pixel as reported in the  GDAL geotramsform[1]
 */
double QgsRasterLayer::rasterUnitsPerPixelX()
{
// We return one raster pixel per map unit pixel
// One raster pixel can have several raster units...

// We can only use one of the mGeoTransform[], so go with the
// horisontal one.

  if ( mDataProvider->capabilities() & QgsRasterDataProvider::Size && mDataProvider->xSize() > 0 )
  {
    return mDataProvider->extent().width() / mDataProvider->xSize();
  }
  return 1;
}

double QgsRasterLayer::rasterUnitsPerPixelY()
{
  if ( mDataProvider->capabilities() & QgsRasterDataProvider::Size && mDataProvider->xSize() > 0 )
  {
    return mDataProvider->extent().height() / mDataProvider->ySize();
  }
  return 1;
}

void QgsRasterLayer::init()
{
  mRasterType = QgsRasterLayer::GrayOrUndefined;

  setRendererForDrawingStyle( QgsRasterLayer::UndefinedDrawingStyle );

  //Initialize the last view port structure, should really be a class
  mLastViewPort.mWidth = 0;
  mLastViewPort.mHeight = 0;
}

void QgsRasterLayer::setDataProvider( QString const & provider )
{
  QgsDebugMsg( "Entered" );
  mValid = false; // assume the layer is invalid until we determine otherwise

  mPipe.remove( mDataProvider ); // deletes if exists
  mDataProvider = 0;

  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  mProviderKey = provider;
  // set the layer name (uppercase first character)
  if ( ! mLayerName.isEmpty() )   // XXX shouldn't this happen in parent?
  {
    setLayerName( mLayerName );
  }

  //mBandCount = 0;

  mDataProvider = ( QgsRasterDataProvider* )QgsProviderRegistry::instance()->provider( mProviderKey, mDataSource );
  if ( !mDataProvider )
  {
    //QgsMessageLog::logMessage( tr( "Cannot instantiate the data provider" ), tr( "Raster" ) );
    appendError( ERR( tr( "Cannot instantiate the '%1' data provider" ).arg( mProviderKey ) ) );
    return;
  }
  QgsDebugMsg( "Data provider created" );

  // Set data provider into pipe even if not valid so that it is deleted with pipe (with layer)
  mPipe.set( mDataProvider );
  if ( !mDataProvider->isValid() )
  {
    setError( mDataProvider->error() );
    appendError( ERR( tr( "Provider is not valid (provider: %1, URI: %2" ).arg( mProviderKey ).arg( mDataSource ) ) );
    return;
  }

  if ( provider == "gdal" )
  {
    // make sure that the /vsigzip or /vsizip is added to uri, if applicable
    mDataSource = mDataProvider->dataSourceUri();
  }

  // get the extent
  QgsRectangle mbr = mDataProvider->extent();

  // show the extent
  QString s = mbr.toString();
  QgsDebugMsg( "Extent of layer: " + s );
  // store the extent
  setExtent( mbr );

  // upper case the first letter of the layer name
  QgsDebugMsg( "mLayerName: " + name() );

  // set up the raster drawing style
  // Do not set any 'sensible' style here, the style is set later

  // Setup source CRS
  setCrs( QgsCoordinateReferenceSystem( mDataProvider->crs() ) );

  QString mySourceWkt = crs().toWkt();

  QgsDebugMsg( "using wkt:\n" + mySourceWkt );

  //defaults - Needs to be set after the Contrast list has been build
  //Try to read the default contrast enhancement from the config file

  QSettings myQSettings;

  //decide what type of layer this is...
  //TODO Change this to look at the color interp and palette interp to decide which type of layer it is
  QgsDebugMsg( "bandCount = " + QString::number( mDataProvider->bandCount() ) );
  QgsDebugMsg( "dataType = " + QString::number( mDataProvider->dataType( 1 ) ) );
  if (( mDataProvider->bandCount() > 1 ) )
  {
    mRasterType = Multiband;
  }
  else if ( mDataProvider->dataType( 1 ) == QGis::ARGB32
            ||  mDataProvider->dataType( 1 ) == QGis::ARGB32_Premultiplied )
  {
    mRasterType = ColorLayer;
  }
  else if ( mDataProvider->colorInterpretation( 1 ) == QgsRaster::PaletteIndex )
  {
    mRasterType = Palette;
  }
  else if ( mDataProvider->colorInterpretation( 1 ) == QgsRaster::ContinuousPalette )
  {
    mRasterType = Palette;
  }
  else
  {
    mRasterType = GrayOrUndefined;
  }

  QgsDebugMsg( "mRasterType = " + QString::number( mRasterType ) );
  if ( mRasterType == ColorLayer )
  {
    QgsDebugMsg( "Setting drawing style to SingleBandColorDataStyle " + QString::number( SingleBandColorDataStyle ) );
    setRendererForDrawingStyle( SingleBandColorDataStyle );
  }
  else if ( mRasterType == Palette && mDataProvider->colorInterpretation( 1 ) == QgsRaster::PaletteIndex )
  {
    setRendererForDrawingStyle( PalettedColor ); //sensible default
  }
  else if ( mRasterType == Palette && mDataProvider->colorInterpretation( 1 ) == QgsRaster::ContinuousPalette )
  {
    setRendererForDrawingStyle( SingleBandPseudoColor );
    // Load color table
    QList<QgsColorRampShader::ColorRampItem> colorTable = mDataProvider->colorTable( 1 );
    QgsSingleBandPseudoColorRenderer* r = dynamic_cast<QgsSingleBandPseudoColorRenderer*>( renderer() );
    if ( r )
    {
      // TODO: this should go somewhere else
      QgsRasterShader* shader = new QgsRasterShader();
      QgsColorRampShader* colorRampShader = new QgsColorRampShader();
      colorRampShader->setColorRampType( QgsColorRampShader::INTERPOLATED );
      colorRampShader->setColorRampItemList( colorTable );
      shader->setRasterShaderFunction( colorRampShader );
      r->setShader( shader );
    }
  }
  else if ( mRasterType == Multiband )
  {
    setRendererForDrawingStyle( MultiBandColor );  //sensible default
  }
  else                        //GrayOrUndefined
  {
    setRendererForDrawingStyle( SingleBandGray );  //sensible default
  }

  // Auto set alpha band
  for ( int bandNo = 1; bandNo <= mDataProvider->bandCount(); bandNo++ )
  {
    if ( mDataProvider->colorInterpretation( bandNo ) == QgsRaster::AlphaBand )
    {
      if ( mPipe.renderer() )
      {
        mPipe.renderer()->setAlphaBand( bandNo );
      }
      break;
    }
  }

  // brightness filter
  QgsBrightnessContrastFilter * brightnessFilter = new QgsBrightnessContrastFilter();
  mPipe.set( brightnessFilter );

  // hue/saturation filter
  QgsHueSaturationFilter * hueSaturationFilter = new QgsHueSaturationFilter();
  mPipe.set( hueSaturationFilter );

  //resampler (must be after renderer)
  QgsRasterResampleFilter * resampleFilter = new QgsRasterResampleFilter();
  mPipe.set( resampleFilter );

  // projector (may be anywhere in pipe)
  QgsRasterProjector * projector = new QgsRasterProjector;
  mPipe.set( projector );

  // Set default identify format - use the richest format available
  int capabilities = mDataProvider->capabilities();
  QgsRaster::IdentifyFormat identifyFormat = QgsRaster::IdentifyFormatUndefined;
  if ( capabilities & QgsRasterInterface::IdentifyHtml )
  {
    // HTML is usually richest
    identifyFormat = QgsRaster::IdentifyFormatHtml;
  }
  else if ( capabilities & QgsRasterInterface::IdentifyFeature )
  {
    identifyFormat = QgsRaster::IdentifyFormatFeature;
  }
  else if ( capabilities & QgsRasterInterface::IdentifyText )
  {
    identifyFormat = QgsRaster::IdentifyFormatText;
  }
  else if ( capabilities & QgsRasterInterface::IdentifyValue )
  {
    identifyFormat = QgsRaster::IdentifyFormatValue;
  }
  setCustomProperty( "identify/format", QgsRasterDataProvider::identifyFormatName( identifyFormat ) );

  // Store timestamp
  // TODO move to provider
  mLastModified = lastModified( mDataSource );

  // Connect provider signals
  connect(
    mDataProvider, SIGNAL( progress( int, double, QString ) ),
    this,          SLOT( onProgress( int, double, QString ) )
  );

  // Do a passthrough for the status bar text
  connect(
    mDataProvider, SIGNAL( statusChanged( QString ) ),
    this,          SIGNAL( statusChanged( QString ) )
  );

  //mark the layer as valid
  mValid = true;

  QgsDebugMsg( "exiting." );
} // QgsRasterLayer::setDataProvider

void QgsRasterLayer::closeDataProvider()
{
  mValid = false;
  mPipe.remove( mDataProvider );
  mDataProvider = 0;
}

void QgsRasterLayer::setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm theAlgorithm, QgsRaster::ContrastEnhancementLimits theLimits, QgsRectangle theExtent, int theSampleSize, bool theGenerateLookupTableFlag )
{
  QgsDebugMsg( QString( "theAlgorithm = %1 theLimits = %2 theExtent.isEmpty() = %3" ).arg( theAlgorithm ).arg( theLimits ).arg( theExtent.isEmpty() ) );
  if ( !mPipe.renderer() || !mDataProvider )
  {
    return;
  }

  QList<int> myBands;
  QList<QgsContrastEnhancement*> myEnhancements;
  QgsSingleBandGrayRenderer* myGrayRenderer = 0;
  QgsMultiBandColorRenderer* myMultiBandRenderer = 0;
  QString rendererType  = mPipe.renderer()->type();
  if ( rendererType == "singlebandgray" )
  {
    myGrayRenderer = dynamic_cast<QgsSingleBandGrayRenderer*>( mPipe.renderer() );
    if ( !myGrayRenderer ) return;
    myBands << myGrayRenderer->grayBand();
  }
  else if ( rendererType == "multibandcolor" )
  {
    myMultiBandRenderer = dynamic_cast<QgsMultiBandColorRenderer*>( mPipe.renderer() );
    if ( !myMultiBandRenderer ) return;
    myBands << myMultiBandRenderer->redBand() << myMultiBandRenderer->greenBand() << myMultiBandRenderer->blueBand();
  }

  foreach ( int myBand, myBands )
  {
    if ( myBand != -1 )
    {
      QGis::DataType myType = ( QGis::DataType )mDataProvider->dataType( myBand );
      QgsContrastEnhancement* myEnhancement = new QgsContrastEnhancement(( QGis::DataType )myType );
      myEnhancement->setContrastEnhancementAlgorithm( theAlgorithm, theGenerateLookupTableFlag );

      double myMin = std::numeric_limits<double>::quiet_NaN();
      double myMax = std::numeric_limits<double>::quiet_NaN();

      if ( theLimits == QgsRaster::ContrastEnhancementMinMax )
      {
        QgsRasterBandStats myRasterBandStats = mDataProvider->bandStatistics( myBand, QgsRasterBandStats::Min | QgsRasterBandStats::Max, theExtent, theSampleSize );
        myMin = myRasterBandStats.minimumValue;
        myMax = myRasterBandStats.maximumValue;
      }
      else if ( theLimits == QgsRaster::ContrastEnhancementStdDev )
      {
        double myStdDev = 1; // make optional?
        QgsRasterBandStats myRasterBandStats = mDataProvider->bandStatistics( myBand, QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev, theExtent, theSampleSize );
        myMin = myRasterBandStats.mean - ( myStdDev * myRasterBandStats.stdDev );
        myMax = myRasterBandStats.mean + ( myStdDev * myRasterBandStats.stdDev );
      }
      else if ( theLimits == QgsRaster::ContrastEnhancementCumulativeCut )
      {
        QSettings mySettings;
        double myLower = mySettings.value( "/Raster/cumulativeCutLower", QString::number( CUMULATIVE_CUT_LOWER ) ).toDouble();
        double myUpper = mySettings.value( "/Raster/cumulativeCutUpper", QString::number( CUMULATIVE_CUT_UPPER ) ).toDouble();
        QgsDebugMsg( QString( "myLower = %1 myUpper = %2" ).arg( myLower ).arg( myUpper ) );
        mDataProvider->cumulativeCut( myBand, myLower, myUpper, myMin, myMax, theExtent, theSampleSize );
      }

      QgsDebugMsg( QString( "myBand = %1 myMin = %2 myMax = %3" ).arg( myBand ).arg( myMin ).arg( myMax ) );
      myEnhancement->setMinimumValue( myMin );
      myEnhancement->setMaximumValue( myMax );
      myEnhancements.append( myEnhancement );
    }
    else
    {
      myEnhancements.append( 0 );
    }
  }

  if ( rendererType == "singlebandgray" )
  {
    if ( myEnhancements.value( 0 ) ) myGrayRenderer->setContrastEnhancement( myEnhancements.value( 0 ) );
  }
  else if ( rendererType == "multibandcolor" )
  {
    if ( myEnhancements.value( 0 ) ) myMultiBandRenderer->setRedContrastEnhancement( myEnhancements.value( 0 ) );
    if ( myEnhancements.value( 1 ) ) myMultiBandRenderer->setGreenContrastEnhancement( myEnhancements.value( 1 ) );
    if ( myEnhancements.value( 2 ) ) myMultiBandRenderer->setBlueContrastEnhancement( myEnhancements.value( 2 ) );
  }
}

void QgsRasterLayer::setDefaultContrastEnhancement()
{
  QgsDebugMsg( "Entered" );

  QSettings mySettings;

  QString myKey;
  QString myDefault;

  // TODO: we should not test renderer class here, move it somehow to renderers
  if ( dynamic_cast<QgsSingleBandGrayRenderer*>( renderer() ) )
  {
    myKey = "singleBand";
    myDefault = "StretchToMinimumMaximum";
  }
  else if ( dynamic_cast<QgsMultiBandColorRenderer*>( renderer() ) )
  {
    if ( QgsRasterBlock::typeSize( dataProvider()->srcDataType( 1 ) ) == 1 )
    {
      myKey = "multiBandSingleByte";
      myDefault = "NoEnhancement";
    }
    else
    {
      myKey = "multiBandMultiByte";
      myDefault = "StretchToMinimumMaximum";
    }
  }

  if ( myKey.isEmpty() )
  {
    QgsDebugMsg( "No default contrast enhancement for this drawing style" );
  }
  QgsDebugMsg( "myKey = " + myKey );

  QString myAlgorithmString = mySettings.value( "/Raster/defaultContrastEnhancementAlgorithm/" + myKey, myDefault ).toString();
  QgsDebugMsg( "myAlgorithmString = " + myAlgorithmString );

  QgsContrastEnhancement::ContrastEnhancementAlgorithm myAlgorithm = QgsContrastEnhancement::contrastEnhancementAlgorithmFromString( myAlgorithmString );

  if ( myAlgorithm == QgsContrastEnhancement::NoEnhancement )
  {
    return;
  }

  QString myLimitsString = mySettings.value( "/Raster/defaultContrastEnhancementLimits", "CumulativeCut" ).toString();
  QgsRaster::ContrastEnhancementLimits myLimits = QgsRaster::contrastEnhancementLimitsFromString( myLimitsString );

  setContrastEnhancement( myAlgorithm, myLimits );
}

/**
 *
 * Implemented mainly for serialisation / deserialisation of settings to xml.
 * \note May be deprecated in the future! Use setDrawingStyle( DrawingStyle ) instead.
 */
void QgsRasterLayer::setDrawingStyle( QString const & theDrawingStyleQString )
{
  QgsDebugMsg( "DrawingStyle = " + theDrawingStyleQString );
  DrawingStyle drawingStyle;
  if ( theDrawingStyleQString == "SingleBandGray" )//no need to tr() this its not shown in ui
  {
    drawingStyle = SingleBandGray;
  }
  else if ( theDrawingStyleQString == "SingleBandPseudoColor" )//no need to tr() this its not shown in ui
  {
    drawingStyle = SingleBandPseudoColor;
  }
  else if ( theDrawingStyleQString == "PalettedColor" )//no need to tr() this its not shown in ui
  {
    drawingStyle = PalettedColor;
  }
  else if ( theDrawingStyleQString == "PalettedSingleBandGray" )//no need to tr() this its not shown in ui
  {
    drawingStyle = PalettedSingleBandGray;
  }
  else if ( theDrawingStyleQString == "PalettedSingleBandPseudoColor" )//no need to tr() this its not shown in ui
  {
    drawingStyle = PalettedSingleBandPseudoColor;
  }
  else if ( theDrawingStyleQString == "PalettedMultiBandColor" )//no need to tr() this its not shown in ui
  {
    drawingStyle = PalettedMultiBandColor;
  }
  else if ( theDrawingStyleQString == "MultiBandSingleBandGray" )//no need to tr() this its not shown in ui
  {
    drawingStyle = MultiBandSingleBandGray;
  }
  else if ( theDrawingStyleQString == "MultiBandSingleBandPseudoColor" )//no need to tr() this its not shown in ui
  {
    drawingStyle = MultiBandSingleBandPseudoColor;
  }
  else if ( theDrawingStyleQString == "MultiBandColor" )//no need to tr() this its not shown in ui
  {
    drawingStyle = MultiBandColor;
  }
  else if ( theDrawingStyleQString == "SingleBandColorDataStyle" )//no need to tr() this its not shown in ui
  {
    QgsDebugMsg( "Setting drawingStyle to SingleBandColorDataStyle " + QString::number( SingleBandColorDataStyle ) );
    drawingStyle = SingleBandColorDataStyle;
    QgsDebugMsg( "Setted drawingStyle to " + QString::number( drawingStyle ) );
  }
  else
  {
    drawingStyle = UndefinedDrawingStyle;
  }
  setRendererForDrawingStyle( drawingStyle );
}

void QgsRasterLayer::setLayerOrder( QStringList const & layers )
{
  QgsDebugMsg( "entered." );

  if ( mDataProvider )
  {
    QgsDebugMsg( "About to mDataProvider->setLayerOrder(layers)." );
    mDataProvider->setLayerOrder( layers );
  }

}

void QgsRasterLayer::setSubLayerVisibility( QString name, bool vis )
{

  if ( mDataProvider )
  {
    QgsDebugMsg( "About to mDataProvider->setSubLayerVisibility(name, vis)." );
    mDataProvider->setSubLayerVisibility( name, vis );
  }

}

void QgsRasterLayer::setRenderer( QgsRasterRenderer* theRenderer )
{
  QgsDebugMsg( "Entered" );
  if ( !theRenderer ) { return; }
  mPipe.set( theRenderer );
}

void QgsRasterLayer::showProgress( int theValue )
{
  emit progressUpdate( theValue );
}


void QgsRasterLayer::showStatusMessage( QString const & theMessage )
{
  // QgsDebugMsg(QString("entered with '%1'.").arg(theMessage));

  // Pass-through
  // TODO: See if we can connect signal-to-signal.  This is a kludge according to the Qt doc.
  emit statusChanged( theMessage );
}

QStringList QgsRasterLayer::subLayers() const
{
  return mDataProvider->subLayers();
}

QPixmap QgsRasterLayer::previewAsPixmap( QSize size, QColor bgColor )
{
  QPixmap myQPixmap( size );

  myQPixmap.fill( bgColor );  //defaults to white, set to transparent for rendering on a map

  QgsRasterViewPort *myRasterViewPort = new QgsRasterViewPort();

  double myMapUnitsPerPixel;
  double myX = 0.0;
  double myY = 0.0;
  QgsRectangle myExtent = mDataProvider->extent();
  if ( myExtent.width() / myExtent.height() >=  myQPixmap.width() / myQPixmap.height() )
  {
    myMapUnitsPerPixel = myExtent.width() / myQPixmap.width();
    myY = ( myQPixmap.height() - myExtent.height() / myMapUnitsPerPixel ) / 2;
  }
  else
  {
    myMapUnitsPerPixel = myExtent.height() / myQPixmap.height();
    myX = ( myQPixmap.width() - myExtent.width() / myMapUnitsPerPixel ) / 2;
  }

  double myPixelWidth = myExtent.width() / myMapUnitsPerPixel;
  double myPixelHeight = myExtent.height() / myMapUnitsPerPixel;

  myRasterViewPort->mTopLeftPoint = QgsPoint( myX, myY );
  myRasterViewPort->mBottomRightPoint = QgsPoint( myPixelWidth, myPixelHeight );
  myRasterViewPort->mWidth = myQPixmap.width();
  myRasterViewPort->mHeight = myQPixmap.height();

  myRasterViewPort->mDrawnExtent = myExtent;
  myRasterViewPort->mSrcCRS = QgsCoordinateReferenceSystem(); // will be invalid
  myRasterViewPort->mDestCRS = QgsCoordinateReferenceSystem(); // will be invalid

  QgsMapToPixel *myMapToPixel = new QgsMapToPixel( myMapUnitsPerPixel );

  QPainter * myQPainter = new QPainter( &myQPixmap );
  draw( myQPainter, myRasterViewPort, myMapToPixel );
  delete myRasterViewPort;
  delete myMapToPixel;
  myQPainter->end();
  delete myQPainter;

  return myQPixmap;
}

void QgsRasterLayer::triggerRepaint()
{
  emit repaintRequested();
}

void QgsRasterLayer::updateProgress( int theProgress, int theMax )
{
  //simply propogate it on!
  emit drawingProgress( theProgress, theMax );
}

void QgsRasterLayer::onProgress( int theType, double theProgress, QString theMessage )
{
  Q_UNUSED( theType );
  Q_UNUSED( theMessage );
  QgsDebugMsg( QString( "theProgress = %1" ).arg( theProgress ) );
  emit progressUpdate(( int )theProgress );
}

//////////////////////////////////////////////////////////
//
// Protected methods
//
/////////////////////////////////////////////////////////
/*
 * @param QDomNode node that will contain the symbology definition for this layer.
 * @param errorMessage reference to string that will be updated with any error messages
 * @return true in case of success.
 */
bool QgsRasterLayer::readSymbology( const QDomNode& layer_node, QString& errorMessage )
{
  Q_UNUSED( errorMessage );
  QDomElement rasterRendererElem;

  // pipe element was introduced in the end of 1.9 development when there were
  // already many project files in use so we support 1.9 backward compatibility
  // even it was never officialy released -> use pipe element if present, otherwise
  // use layer node
  QDomNode pipeNode = layer_node.firstChildElement( "pipe" );
  if ( pipeNode.isNull() ) // old project
  {
    pipeNode = layer_node;
  }

  //rasterlayerproperties element there -> old format (1.8 and early 1.9)
  if ( !layer_node.firstChildElement( "rasterproperties" ).isNull() )
  {
    //copy node because layer_node is const
    QDomNode layerNodeCopy = layer_node.cloneNode();
    QDomDocument doc = layerNodeCopy.ownerDocument();
    QDomElement rasterPropertiesElem = layerNodeCopy.firstChildElement( "rasterproperties" );
    QgsProjectFileTransform::convertRasterProperties( doc, layerNodeCopy, rasterPropertiesElem,
        this );
    rasterRendererElem = layerNodeCopy.firstChildElement( "rasterrenderer" );
    QgsDebugMsg( doc.toString() );
  }
  else
  {
    rasterRendererElem = pipeNode.firstChildElement( "rasterrenderer" );
  }

  if ( !rasterRendererElem.isNull() )
  {
    QString rendererType = rasterRendererElem.attribute( "type" );
    QgsRasterRendererRegistryEntry rendererEntry;
    if ( QgsRasterRendererRegistry::instance()->rendererData( rendererType, rendererEntry ) )
    {
      QgsRasterRenderer *renderer = rendererEntry.rendererCreateFunction( rasterRendererElem, dataProvider() );
      mPipe.set( renderer );
    }
  }

  //brightness
  QgsBrightnessContrastFilter * brightnessFilter = new QgsBrightnessContrastFilter();
  mPipe.set( brightnessFilter );

  //brightness coefficient
  QDomElement brightnessElem = pipeNode.firstChildElement( "brightnesscontrast" );
  if ( !brightnessElem.isNull() )
  {
    brightnessFilter->readXML( brightnessElem );
  }

  //hue/saturation
  QgsHueSaturationFilter * hueSaturationFilter = new QgsHueSaturationFilter();
  mPipe.set( hueSaturationFilter );

  //saturation coefficient
  QDomElement hueSaturationElem = pipeNode.firstChildElement( "huesaturation" );
  if ( !hueSaturationElem.isNull() )
  {
    hueSaturationFilter->readXML( hueSaturationElem );
  }

  //resampler
  QgsRasterResampleFilter * resampleFilter = new QgsRasterResampleFilter();
  mPipe.set( resampleFilter );

  //max oversampling
  QDomElement resampleElem = pipeNode.firstChildElement( "rasterresampler" );
  if ( !resampleElem.isNull() )
  {
    resampleFilter->readXML( resampleElem );
  }

  // get and set the blend mode if it exists
  QDomNode blendModeNode = layer_node.namedItem( "blendMode" );
  if ( !blendModeNode.isNull() )
  {
    QDomElement e = blendModeNode.toElement();
    setBlendMode( QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode ) e.text().toInt() ) );
  }

  return true;
} //readSymbology

/**

  Raster layer project file XML of form:

  @note Called by QgsMapLayer::readXML().
*/
bool QgsRasterLayer::readXml( const QDomNode& layer_node )
{
  QgsDebugMsg( "Entered" );
  //! @note Make sure to read the file first so stats etc are initialised properly!

  //process provider key
  QDomNode pkeyNode = layer_node.namedItem( "provider" );

  if ( pkeyNode.isNull() )
  {
    mProviderKey = "gdal";
  }
  else
  {
    QDomElement pkeyElt = pkeyNode.toElement();
    mProviderKey = pkeyElt.text();
    if ( mProviderKey.isEmpty() )
    {
      mProviderKey = "gdal";
    }
  }

  // Open the raster source based on provider and datasource

  // Go down the raster-data-provider paradigm

  // Collect provider-specific information

  QDomNode rpNode = layer_node.namedItem( "rasterproperties" );

  if ( mProviderKey == "wms" )
  {
    // >>> BACKWARD COMPATIBILITY < 1.9
    // The old WMS URI format does not contain all the informations, we add them here.
    if ( !mDataSource.contains( "crs=" ) && !mDataSource.contains( "format=" ) )
    {
      QgsDebugMsg( "Old WMS URI format detected -> adding params" );
      QgsDataSourceURI uri;
      uri.setEncodedUri( mDataSource );
      QDomElement layerElement = rpNode.firstChildElement( "wmsSublayer" );
      while ( !layerElement.isNull() )
      {
        // TODO: sublayer visibility - post-0.8 release timeframe

        // collect name for the sublayer
        uri.setParam( "layers",  layerElement.namedItem( "name" ).toElement().text() );

        // collect style for the sublayer
        uri.setParam( "styles", layerElement.namedItem( "style" ).toElement().text() );

        layerElement = layerElement.nextSiblingElement( "wmsSublayer" );
      }

      // Collect format
      QDomNode formatNode = rpNode.namedItem( "wmsFormat" );
      uri.setParam( "format", rpNode.namedItem( "wmsFormat" ).toElement().text() );

      // WMS CRS URL param should not be mixed with that assigned to the layer.
      // In the old WMS URI version there was no CRS and layer crs().authid() was used.
      uri.setParam( "crs", crs().authid() );
      mDataSource = uri.encodedUri();
    }
    // <<< BACKWARD COMPATIBILITY < 1.9
  }

  setDataProvider( mProviderKey );
  if ( !mValid ) return false;

  QString theError;
  bool res = readSymbology( layer_node, theError );

  // old wms settings we need to correct
  if ( res && mProviderKey == "wms" )
  {
    setRendererForDrawingStyle( SingleBandColorDataStyle );
  }

  // Check timestamp
  // This was probably introduced to reload completely raster if data changed and
  // reset completly symbology to reflect new data type etc. It creates however
  // problems, because user defined symbology is complete lost if data file time
  // changed (the content may be the same). See also 6900.
#if 0
  QDomNode stampNode = layer_node.namedItem( "timestamp" );
  if ( !stampNode.isNull() )
  {
    QDateTime stamp = QDateTime::fromString( stampNode.toElement().text(), Qt::ISODate );
    // TODO: very bad, we have to load twice!!! Make QgsDataProvider::timestamp() static?
    if ( stamp < mDataProvider->dataTimestamp() )
    {
      QgsDebugMsg( "data changed, reload provider" );
      closeDataProvider();
      init();
      setDataProvider( mProviderKey );
      if ( !mValid ) return false;
    }
  }
#endif

  // Load user no data value
  QDomElement noDataElement = layer_node.firstChildElement( "noData" );

  QDomNodeList noDataBandList = noDataElement.elementsByTagName( "noDataList" );

  for ( int i = 0; i < noDataBandList.size(); ++i )
  {
    QDomElement bandElement = noDataBandList.at( i ).toElement();
    bool ok;
    int bandNo = bandElement.attribute( "bandNo" ).toInt( &ok );
    QgsDebugMsg( QString( "bandNo = %1" ).arg( bandNo ) );
    if ( ok && ( bandNo > 0 ) && ( bandNo <= mDataProvider->bandCount() ) )
    {
      mDataProvider->setUseSrcNoDataValue( bandNo, bandElement.attribute( "useSrcNoData" ).toInt() );
      QgsRasterRangeList myNoDataRangeList;

      QDomNodeList rangeList = bandElement.elementsByTagName( "noDataRange" );

      for ( int j = 0; j < rangeList.size(); ++j )
      {
        QDomElement rangeElement = rangeList.at( j ).toElement();
        QgsRasterRange myNoDataRange( rangeElement.attribute( "min" ).toDouble(),
                                      rangeElement.attribute( "max" ).toDouble() );
        QgsDebugMsg( QString( "min = %1 %2" ).arg( rangeElement.attribute( "min" ) ).arg( myNoDataRange.min() ) );
        myNoDataRangeList << myNoDataRange;
      }
      mDataProvider->setUserNoDataValue( bandNo, myNoDataRangeList );
    }
  }

  return res;
} // QgsRasterLayer::readXml( QDomNode & layer_node )

/*
 * @param QDomNode the node that will have the style element added to it.
 * @param QDomDocument the document that will have the QDomNode added.
 * @param errorMessage reference to string that will be updated with any error messages
 * @return true in case of success.
 */
bool QgsRasterLayer::writeSymbology( QDomNode & layer_node, QDomDocument & document, QString& errorMessage ) const
{
  Q_UNUSED( errorMessage );
  QDomElement layerElem = layer_node.toElement();

  // Store pipe members (except provider) into pipe element, in future, it will be
  // possible to add custom filters into the pipe
  QDomElement pipeElement  = document.createElement( "pipe" );

  for ( int i = 1; i < mPipe.size(); i++ )
  {
    QgsRasterInterface * interface = mPipe.at( i );
    if ( !interface ) continue;
    interface->writeXML( document, pipeElement );
  }

  layer_node.appendChild( pipeElement );

  // add blend mode node
  QDomElement blendModeElement  = document.createElement( "blendMode" );
  QDomText blendModeText = document.createTextNode( QString::number( QgsMapRenderer::getBlendModeEnum( blendMode() ) ) );
  blendModeElement.appendChild( blendModeText );
  layer_node.appendChild( blendModeElement );

  return true;
} // bool QgsRasterLayer::writeSymbology

/*
 *  virtual
 *  @note Called by QgsMapLayer::writeXML().
 */
bool QgsRasterLayer::writeXml( QDomNode & layer_node,
                               QDomDocument & document )
{
  // first get the layer element so that we can append the type attribute

  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || "maplayer" != mapLayerNode.nodeName() )
  {
    QgsMessageLog::logMessage( tr( "<maplayer> not found." ), tr( "Raster" ) );
    return false;
  }

  mapLayerNode.setAttribute( "type", "raster" );

  // add provider node

  QDomElement provider  = document.createElement( "provider" );
  QDomText providerText = document.createTextNode( mProviderKey );
  provider.appendChild( providerText );
  layer_node.appendChild( provider );

  // User no data
  QDomElement noData  = document.createElement( "noData" );

  for ( int bandNo = 1; bandNo <= mDataProvider->bandCount(); bandNo++ )
  {
    if ( mDataProvider->userNoDataValues( bandNo ).isEmpty() ) continue;

    QDomElement noDataRangeList = document.createElement( "noDataList" );
    noDataRangeList.setAttribute( "bandNo", bandNo );
    noDataRangeList.setAttribute( "useSrcNoData", mDataProvider->useSrcNoDataValue( bandNo ) );

    foreach ( QgsRasterRange range, mDataProvider->userNoDataValues( bandNo ) )
    {
      QDomElement noDataRange =  document.createElement( "noDataRange" );

      noDataRange.setAttribute( "min", range.min() );
      noDataRange.setAttribute( "max", range.max() );
      noDataRangeList.appendChild( noDataRange );
    }

    noData.appendChild( noDataRangeList );

  }
  if ( noData.hasChildNodes() )
  {
    layer_node.appendChild( noData );
  }

  //write out the symbology
  QString errorMsg;
  return writeSymbology( layer_node, document, errorMsg );
}

int QgsRasterLayer::width() const
{
  if ( !mDataProvider ) return 0;
  return mDataProvider->xSize();
}

int QgsRasterLayer::height() const
{
  if ( !mDataProvider ) return 0;
  return mDataProvider->ySize();
}

//////////////////////////////////////////////////////////
//
// Private methods
//
/////////////////////////////////////////////////////////
bool QgsRasterLayer::update()
{
  QgsDebugMsg( "entered." );
  // Check if data changed
  if ( mDataProvider->dataTimestamp() > mDataProvider->timestamp() )
  {
    QgsDebugMsg( "reload data" );
    closeDataProvider();
    init();
    setDataProvider( mProviderKey );
    emit dataChanged();
  }
  return mValid;
}
