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
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsproviderregistry.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpyramid.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"

#include "gdalwarper.h"
#include "cpl_conv.h"

#include "qgspseudocolorshader.h"
#include "qgsfreakoutshader.h"
#include "qgscolorrampshader.h"

//renderers
#include "qgspalettedrasterrenderer.h"
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"
#include "qgsmultibandcolorrenderer.h"

#include <cstdio>
#include <cmath>
#include <limits>

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
#include <QList>
#include <QMatrix>
#include <QMessageBox>
#include <QLibrary>
#include <QPainter>
#include <QPixmap>
#include <QRegExp>
#include <QSlider>
#include <QSettings>
#include <QTime>

// typedefs for provider plugin functions of interest
typedef void buildsupportedrasterfilefilter_t( QString & theFileFiltersString );
typedef bool isvalidrasterfilename_t( QString const & theFileNameQString, QString & retErrMsg );

// workaround for MSVC compiler which already has defined macro max
// that interferes with calling std::numeric_limits<int>::max
#ifdef _MSC_VER
# ifdef max
#  undef max
# endif
#endif

// Comparison value for equality; i.e., we shouldn't directly compare two
// floats so it's better to take their difference and see if they're within
// a certain range -- in this case twenty times the smallest value that
// doubles can take for the current system.  (Yes, 20 was arbitrary.)
#define TINY_VALUE  std::numeric_limits<double>::epsilon() * 20


QgsRasterLayer::QgsRasterLayer(
  QString const & path,
  QString const & baseName,
  bool loadDefaultStyleFlag )
    : QgsMapLayer( RasterLayer, baseName, path )
    // Constant that signals property not used.
    , QSTRING_NOT_SET( "Not Set" )
    , TRSTRING_NOT_SET( tr( "Not Set" ) )
    , mStandardDeviations( 0 )
    , mDataProvider( 0 )
    , mWidth( std::numeric_limits<int>::max() )
    , mHeight( std::numeric_limits<int>::max() )
    , mInvertColor( false )
{
  QgsDebugMsg( "Entered" );

  // TODO, call constructor with provider key for now
  init();
  setDataProvider( "gdal", QStringList(), QStringList(), QString(), QString(), loadDefaultStyleFlag );

  if ( mValid && loadDefaultStyleFlag )
  {
    bool defaultLoadedFlag = false;
    loadDefaultStyle( defaultLoadedFlag );
    // I'm no sure if this should be used somehow, in pre raster-providers there was
    // only mLastViewPort init after this block, nothing to do with style
    //if ( defaultLoadedFlag )
    //{
    //return;
    //}
  }
  return;


} // QgsRasterLayer ctor

/**
 * @todo Rename into a general constructor when the old raster interface is retired
 * parameter dummy is just there to distinguish this function signature from the old non-provider one.
 */
QgsRasterLayer::QgsRasterLayer( int dummy,
                                QString const & rasterLayerPath,
                                QString const & baseName,
                                QString const & providerKey,
                                QStringList const & layers,
                                QStringList const & styles,
                                QString const & format,
                                QString const & crs )
    : QgsMapLayer( RasterLayer, baseName, rasterLayerPath )
    , mStandardDeviations( 0 )
    , mDataProvider( 0 )
    , mEditable( false )
    , mWidth( std::numeric_limits<int>::max() )
    , mHeight( std::numeric_limits<int>::max() )
    , mInvertColor( false )
    , mModified( false )
    , mProviderKey( providerKey )
    , mLayers( layers )
    , mStyles( styles )
    , mFormat( format )
    , mCrs( crs )
{
  Q_UNUSED( dummy );

  QgsDebugMsg( "(8 arguments) starting. with layer list of " +
               layers.join( ", " ) +  " and style list of " + styles.join( ", " ) + " and format of " +
               format +  " and CRS of " + crs );


  init();
  // if we're given a provider type, try to create and bind one to this layer
  bool loadDefaultStyleFlag = false ; // ???
  setDataProvider( providerKey, layers, styles, format, crs, loadDefaultStyleFlag );

  // Default for the popup menu
  // TODO: popMenu = 0;

  // Get the update threshold from user settings. We
  // do this only on construction to avoid the penality of
  // fetching this each time the layer is drawn. If the user
  // changes the threshold from the preferences dialog, it will
  // have no effect on existing layers
  // TODO: QSettings settings;
  // updateThreshold = settings.readNumEntry("Map/updateThreshold", 1000);


  // TODO: Connect signals from the dataprovider to the qgisapp

  QgsDebugMsg( "(8 arguments) exiting." );

  emit statusChanged( tr( "QgsRasterLayer created" ) );
} // QgsRasterLayer ctor

QgsRasterLayer::~QgsRasterLayer()
{
  mValid = false;
  delete mRasterShader;
  delete mDataProvider;
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
  QLibrary*  myLib = QgsRasterLayer::loadProviderLibrary( "gdal" );
  if ( !myLib )
  {
    QgsDebugMsg( "Could not load gdal provider library" );
    return;
  }
  buildsupportedrasterfilefilter_t *pBuild = ( buildsupportedrasterfilefilter_t * ) cast_to_fptr( myLib->resolve( "buildSupportedRasterFileFilter" ) );
  if ( ! pBuild )
  {
    QgsDebugMsg( "Could not resolve buildSupportedRasterFileFilter in gdal provider library" );
    return;
  }

  pBuild( theFileFiltersString );
  delete myLib;
}

/**
 * This helper checks to see whether the file name appears to be a valid raster file name
 */
bool QgsRasterLayer::isValidRasterFileName( QString const & theFileNameQString, QString & retErrMsg )
{

  QLibrary*  myLib = QgsRasterLayer::loadProviderLibrary( "gdal" );
  if ( !myLib )
  {
    QgsDebugMsg( "Could not load gdal provider library" );
    return false;
  }
  isvalidrasterfilename_t *pValid = ( isvalidrasterfilename_t * ) cast_to_fptr( myLib->resolve( "isValidRasterFileName" ) );
  if ( ! pValid )
  {
    QgsDebugMsg( "Could not resolve isValidRasterFileName in gdal provider library" );
    return false;
  }

  bool myIsValid = pValid( theFileNameQString, retErrMsg );
  delete myLib;
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

unsigned int QgsRasterLayer::bandCount() const
{
  return mBandCount;
}

const QString QgsRasterLayer::bandName( int theBandNo )
{
  if ( theBandNo <= mRasterStatsList.size() && theBandNo > 0 )
  {
    //vector starts at base 0, band counts at base1!
    return mRasterStatsList[theBandNo - 1].bandName;
  }
  else
  {
    return QString( "" );
  }
}

int QgsRasterLayer::bandNumber( QString const & theBandName ) const
{
  for ( int myIterator = 0; myIterator < mRasterStatsList.size(); ++myIterator )
  {
    //find out the name of this band
    QgsRasterBandStats myRasterBandStats = mRasterStatsList[myIterator];
    QgsDebugMsg( "myRasterBandStats.bandName: " + myRasterBandStats.bandName + "  :: theBandName: "
                 + theBandName );

    if ( myRasterBandStats.bandName == theBandName )
    {
      QgsDebugMsg( "********** band " + QString::number( myRasterBandStats.bandNumber ) +
                   " was found in bandNumber " + theBandName );

      return myRasterBandStats.bandNumber;
    }
  }
  QgsDebugMsg( "********** no band was found in bandNumber " + theBandName );

  return 0;                     //no band was found
}

/**
 * Private method to calculate statistics for a band. Populates rasterStatsMemArray.
 * Calculates:
 *
 * <ul>
 * <li>myRasterBandStats.elementCount
 * <li>myRasterBandStats.minimumValue
 * <li>myRasterBandStats.maximumValue
 * <li>myRasterBandStats.sum
 * <li>myRasterBandStats.range
 * <li>myRasterBandStats.mean
 * <li>myRasterBandStats.sumOfSquares
 * <li>myRasterBandStats.stdDev
 * <li>myRasterBandStats.colorTable
 * </ul>
 *
 * @sa RasterBandStats
 * @note This is a cpu intensive and slow task!
 */
const QgsRasterBandStats QgsRasterLayer::bandStatistics( int theBandNo )
{
  QgsDebugMsg( "theBandNo = " + QString::number( theBandNo ) );
  QgsDebugMsg( "mRasterType = " + QString::number( mRasterType ) );
  if ( mRasterType == ColorLayer )
  {
    // Statistics have no sense for ColorLayer
    QgsRasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }
  // check if we have received a valid band number
  if (( mDataProvider->bandCount() < theBandNo ) && mRasterType != Palette )
  {
    // invalid band id, return nothing
    QgsRasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }
  if ( mRasterType == Palette && ( theBandNo > 3 ) )
  {
    // invalid band id, return nothing
    QgsRasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }
  // check if we have previously gathered stats for this band...
  if ( theBandNo < 1 || theBandNo > mRasterStatsList.size() )
  {
    // invalid band id, return nothing
    QgsRasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }

  QgsRasterBandStats myRasterBandStats = mRasterStatsList[theBandNo - 1];
  myRasterBandStats.bandNumber = theBandNo;

  // don't bother with this if we already have stats
  if ( myRasterBandStats.statsGathered )
  {
    return myRasterBandStats;
  }

  myRasterBandStats = mDataProvider->bandStatistics( theBandNo );
  QgsDebugMsg( "adding stats to stats collection at position " + QString::number( theBandNo - 1 ) );
  //add this band to the class stats map
  mRasterStatsList[theBandNo - 1] = myRasterBandStats;
  emit drawingProgress( mHeight, mHeight ); //reset progress
  QgsDebugMsg( "Stats collection completed returning" );
  return myRasterBandStats;
} // QgsRasterLayer::bandStatistics

const QgsRasterBandStats QgsRasterLayer::bandStatistics( QString const & theBandName )
{
  // only print message if we are actually gathering the stats
  emit statusChanged( tr( "Retrieving stats for %1" ).arg( name() ) );
  qApp->processEvents();
  //reset the main app progress bar
  emit drawingProgress( 0, 0 );
  //we cant use a vector iterator because the iterator is astruct not a class
  //and the qvector model does not like this.
  for ( int i = 1; i <= mDataProvider->bandCount(); i++ )
  {
    QgsRasterBandStats myRasterBandStats = bandStatistics( i );
    if ( myRasterBandStats.bandName == theBandName )
    {
      return myRasterBandStats;
    }
  }

  return QgsRasterBandStats();     // return a null one
}


QString QgsRasterLayer::buildPyramids( RasterPyramidList const & theRasterPyramidList,
                                       QString const & theResamplingMethod, bool theTryInternalFlag )
{
  return mDataProvider->buildPyramids( theRasterPyramidList, theResamplingMethod, theTryInternalFlag );
}


QgsRasterLayer::RasterPyramidList  QgsRasterLayer::buildPyramidList()
{
  return mDataProvider->buildPyramidList();
}

QString QgsRasterLayer::colorShadingAlgorithmAsString() const
{
  switch ( mColorShadingAlgorithm )
  {
    case PseudoColorShader:
      return QString( "PseudoColorShader" );
      break;
    case FreakOutShader:
      return QString( "FreakOutShader" );
      break;
    case ColorRampShader:
      return QString( "ColorRampShader" );
      break;
    case UserDefinedShader:
      return QString( "UserDefinedShader" );
      break;
    default:
      break;
  }

  return QString( "UndefinedShader" );
}

/**
 * @param theBand The band (number) for which to estimate the min max values
 * @param theMinMax Pointer to a double[2] which hold the estimated min max
 */
void QgsRasterLayer::computeMinimumMaximumEstimates( int theBand, double* theMinMax )
{
  if ( !theMinMax )
    return;

  if ( 0 < theBand && theBand <= ( int ) bandCount() )
  {
    theMinMax[0] = mDataProvider->minimumValue( theBand );
    theMinMax[1] = mDataProvider->maximumValue( theBand );
  }
}

/**
 * @param theBand The band (name) for which to estimate the min max values
 * @param theMinMax Pointer to a double[2] which hold the estimated min max
 */
void QgsRasterLayer::computeMinimumMaximumEstimates( QString theBand, double* theMinMax )
{
  computeMinimumMaximumEstimates( bandNumber( theBand ), theMinMax );
}

void QgsRasterLayer::computeMinimumMaximumEstimates( int theBand, double& theMin, double& theMax )
{
  double theMinMax[2];
  computeMinimumMaximumEstimates( theBand, theMinMax );
  theMin = theMinMax[0];
  theMax = theMinMax[1];
}

/**
 * @param theBand The band (number) for which to calculate the min max values
 * @param theMinMax Pointer to a double[2] which hold the estimated min max
 */
void QgsRasterLayer::computeMinimumMaximumFromLastExtent( int theBand, double* theMinMax )
{
  if ( !theMinMax )
    return;

  int myDataType = mDataProvider->dataType( theBand );
  void* myScanData = readData( theBand, &mLastViewPort );

  /* Check for out of memory error */
  if ( myScanData == NULL )
  {
    return;
  }

  if ( 0 < theBand && theBand <= ( int ) bandCount() )
  {
    float myMin = std::numeric_limits<float>::max();
    float myMax = -1 * std::numeric_limits<float>::max();
    float myValue = 0.0;
    for ( int myRow = 0; myRow < mLastViewPort.drawableAreaYDim; ++myRow )
    {
      for ( int myColumn = 0; myColumn < mLastViewPort.drawableAreaXDim; ++myColumn )
      {
        myValue = readValue( myScanData, myDataType, myRow * mLastViewPort.drawableAreaXDim + myColumn );
        if ( mValidNoDataValue && ( qAbs( myValue - mNoDataValue ) <= TINY_VALUE || myValue != myValue ) )
        {
          continue;
        }
        myMin = qMin( myMin, myValue );
        myMax = qMax( myMax, myValue );
      }
    }
    theMinMax[0] = myMin;
    theMinMax[1] = myMax;
  }
}

/**
 * @param theBand The band (name) for which to calculate the min max values
 * @param theMinMax Pointer to a double[2] which hold the estimated min max
 */
void QgsRasterLayer::computeMinimumMaximumFromLastExtent( QString theBand, double* theMinMax )
{
  computeMinimumMaximumFromLastExtent( bandNumber( theBand ), theMinMax );
}

void QgsRasterLayer::computeMinimumMaximumFromLastExtent( int theBand, double& theMin, double& theMax )
{
  double theMinMax[2];
  computeMinimumMaximumFromLastExtent( theBand, theMinMax );
  theMin = theMinMax[0];
  theMax = theMinMax[1];
}

/**
 * @param theBand The band (number) for which to get the contrast enhancement for
 * @return Pointer to the contrast enhancement or 0 on failure
 */
QgsContrastEnhancement* QgsRasterLayer::contrastEnhancement( unsigned int theBand )
{
  if ( 0 < theBand && theBand <= bandCount() )
  {
    return &mContrastEnhancementList[theBand - 1];
  }

  return 0;
}

const QgsContrastEnhancement* QgsRasterLayer::constContrastEnhancement( unsigned int theBand ) const
{
  if ( 0 < theBand && theBand <= bandCount() )
  {
    return &mContrastEnhancementList[theBand - 1];
  }

  return 0;
}

QString QgsRasterLayer::contrastEnhancementAlgorithmAsString() const
{
  switch ( mContrastEnhancementAlgorithm )
  {
    case QgsContrastEnhancement::NoEnhancement:
      return QString( "NoEnhancement" );
      break;
    case QgsContrastEnhancement::StretchToMinimumMaximum:
      return QString( "StretchToMinimumMaximum" );
      break;
    case QgsContrastEnhancement::StretchAndClipToMinimumMaximum:
      return QString( "StretchAndClipToMinimumMaximum" );
      break;
    case QgsContrastEnhancement::ClipToMinimumMaximum:
      return QString( "ClipToMinimumMaximum" );
      break;
    case QgsContrastEnhancement::UserDefinedEnhancement:
      return QString( "UserDefined" );
      break;
  }

  return QString( "NoEnhancement" );
}

/**
 * @note Note implemented yet
 * @return always returns false
 */
bool QgsRasterLayer::copySymbologySettings( const QgsMapLayer& theOther )
{
  //prevent warnings
  if ( theOther.type() < 0 )
  {
    return false;
  }
  return false;
} //todo

/**
 * @param theBandNo the band number
 * @return pointer to the color table
 */
QList<QgsColorRampShader::ColorRampItem>* QgsRasterLayer::colorTable( int theBandNo )
{
  return &( mRasterStatsList[theBandNo-1].colorTable );
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
    myProjectedViewExtent = rendererContext.coordinateTransform()->transformBoundingBox(
                              rendererContext.extent() );
    myProjectedLayerExtent = rendererContext.coordinateTransform()->transformBoundingBox(
                               mLayerExtent );
  }
  else
  {
    QgsDebugMsg( "coordinateTransform not set" );
    myProjectedViewExtent = rendererContext.extent();
    myProjectedLayerExtent = mLayerExtent;
  }

  QPainter* theQPainter = rendererContext.painter();

  if ( !theQPainter )
  {
    return false;
  }

  // clip raster extent to view extent
  //QgsRectangle myRasterExtent = theViewExtent.intersect( &mLayerExtent );
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
  myRasterViewPort->topLeftPoint = theQgsMapToPixel.transform( myRasterExtent.xMinimum(), myRasterExtent.yMaximum() );
  myRasterViewPort->bottomRightPoint = theQgsMapToPixel.transform( myRasterExtent.xMaximum(), myRasterExtent.yMinimum() );

  // align to output device grid, i.e. floor/ceil to integers
  // TODO: this should only be done if paint device is raster - screen, image
  // for other devices (pdf) it can have floating point origin
  // we could use floating point for raster devices as well, but respecting the
  // output device grid should make it more effective as the resampling is done in
  // the provider anyway
  if ( true )
  {
    myRasterViewPort->topLeftPoint.setX( floor( myRasterViewPort->topLeftPoint.x() ) );
    myRasterViewPort->topLeftPoint.setY( floor( myRasterViewPort->topLeftPoint.y() ) );
    myRasterViewPort->bottomRightPoint.setX( ceil( myRasterViewPort->bottomRightPoint.x() ) );
    myRasterViewPort->bottomRightPoint.setY( ceil( myRasterViewPort->bottomRightPoint.y() ) );
    // recalc myRasterExtent to aligned values
    myRasterExtent.set(
      theQgsMapToPixel.toMapCoordinatesF( myRasterViewPort->topLeftPoint.x(),
                                          myRasterViewPort->bottomRightPoint.y() ),
      theQgsMapToPixel.toMapCoordinatesF( myRasterViewPort->bottomRightPoint.x(),
                                          myRasterViewPort->topLeftPoint.y() )
    );

  }

  myRasterViewPort->drawableAreaXDim = static_cast<int>( qAbs(( myRasterExtent.width() / theQgsMapToPixel.mapUnitsPerPixel() ) ) );
  myRasterViewPort->drawableAreaYDim = static_cast<int>( qAbs(( myRasterExtent.height() / theQgsMapToPixel.mapUnitsPerPixel() ) ) );

  //the drawable area can start to get very very large when you get down displaying 2x2 or smaller, this is becasue
  //theQgsMapToPixel.mapUnitsPerPixel() is less then 1,
  //so we will just get the pixel data and then render these special cases differently in paintImageToCanvas()
#if 0
  if ( 2 >= myRasterViewPort->clippedWidth && 2 >= myRasterViewPort->clippedHeight )
  {
    myRasterViewPort->drawableAreaXDim = myRasterViewPort->clippedWidth;
    myRasterViewPort->drawableAreaYDim = myRasterViewPort->clippedHeight;
  }
#endif

  QgsDebugMsgLevel( QString( "mapUnitsPerPixel = %1" ).arg( theQgsMapToPixel.mapUnitsPerPixel() ), 3 );
  QgsDebugMsgLevel( QString( "mWidth = %1" ).arg( mWidth ), 3 );
  QgsDebugMsgLevel( QString( "mHeight = %1" ).arg( mHeight ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.xMinimum() = %1" ).arg( myRasterExtent.xMinimum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.xMaximum() = %1" ).arg( myRasterExtent.xMaximum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.yMinimum() = %1" ).arg( myRasterExtent.yMinimum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.yMaximum() = %1" ).arg( myRasterExtent.yMaximum() ), 3 );

  QgsDebugMsgLevel( QString( "topLeftPoint.x() = %1" ).arg( myRasterViewPort->topLeftPoint.x() ), 3 );
  QgsDebugMsgLevel( QString( "bottomRightPoint.x() = %1" ).arg( myRasterViewPort->bottomRightPoint.x() ), 3 );
  QgsDebugMsgLevel( QString( "topLeftPoint.y() = %1" ).arg( myRasterViewPort->topLeftPoint.y() ), 3 );
  QgsDebugMsgLevel( QString( "bottomRightPoint.y() = %1" ).arg( myRasterViewPort->bottomRightPoint.y() ), 3 );

  QgsDebugMsgLevel( QString( "drawableAreaXDim = %1" ).arg( myRasterViewPort->drawableAreaXDim ), 3 );
  QgsDebugMsgLevel( QString( "drawableAreaYDim = %1" ).arg( myRasterViewPort->drawableAreaYDim ), 3 );

  QgsDebugMsgLevel( "ReadXml: gray band name : " + mGrayBandName, 3 );
  QgsDebugMsgLevel( "ReadXml: red band name : " + mRedBandName, 3 );
  QgsDebugMsgLevel( "ReadXml: green band name : " + mGreenBandName, 3 );
  QgsDebugMsgLevel( "ReadXml: blue band name : " + mBlueBandName, 3 );

  // /\/\/\ - added to handle zoomed-in rasters

  mLastViewPort = *myRasterViewPort;

  // Provider mode: See if a provider key is specified, and if so use the provider instead

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

  QgsDebugMsg( "mDrawingStyle = " + QString::number( mDrawingStyle ) );
  switch ( mDrawingStyle )
  {
      // a "Gray" or "Undefined" layer drawn as a range of gray colors
    case SingleBandGray:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        drawSingleBandGray( theQPainter, theRasterViewPort,
                            theQgsMapToPixel, bandNumber( mGrayBandName ) );
        break;
      }
      // a "Gray" or "Undefined" layer drawn using a pseudocolor algorithm
    case SingleBandPseudoColor:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        drawSingleBandPseudoColor( theQPainter, theRasterViewPort,
                                   theQgsMapToPixel, bandNumber( mGrayBandName ) );
        break;
      }
      // a single band with a color map
    case PalettedColor:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        QgsDebugMsg( "PalettedColor drawing type detected..." );

        //test
        int bNumber = bandNumber( mGrayBandName );
        QList<QgsColorRampShader::ColorRampItem> itemList = mRasterStatsList[ bNumber - 1].colorTable;
        QColor* colorArray = new QColor[itemList.size()];
        QList<QgsColorRampShader::ColorRampItem>::const_iterator colorIt = itemList.constBegin();
        for ( ; colorIt != itemList.constEnd(); ++colorIt )
        {
          colorArray[( int )colorIt->value] =  colorIt->color;
        }

        //QgsBilinearRasterResampler resampler;
        QgsCubicRasterResampler resampler;
        QgsPalettedRasterRenderer renderer( mDataProvider, bNumber, colorArray, itemList.size(), &resampler );
        renderer.draw( theQPainter, theRasterViewPort, theQgsMapToPixel );
#if 0
        drawPalettedSingleBandColor( theQPainter, theRasterViewPort,
                                     theQgsMapToPixel, bandNumber( mGrayBandName ) );
#endif //0
        break;
      }
      // a "Palette" layer drawn in gray scale (using only one of the color components)
    case PalettedSingleBandGray:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        QgsDebugMsg( "PalettedSingleBandGray drawing type detected..." );

        int myBandNo = 1;
        drawPalettedSingleBandGray( theQPainter, theRasterViewPort,
                                    theQgsMapToPixel, myBandNo );

        break;
      }
      // a "Palette" layer having only one of its color components rendered as psuedo color
    case PalettedSingleBandPseudoColor:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {

        int myBandNo = 1;
        drawPalettedSingleBandPseudoColor( theQPainter, theRasterViewPort,
                                           theQgsMapToPixel, myBandNo );
        break;
      }
      //a "Palette" image where the bands contains 24bit color info and 8 bits is pulled out per color
    case PalettedMultiBandColor:
      drawPalettedMultiBandColor( theQPainter, theRasterViewPort,
                                  theQgsMapToPixel, 1 );
      break;
      // a layer containing 2 or more bands, but using only one band to produce a grayscale image
    case MultiBandSingleBandGray:
      QgsDebugMsg( "MultiBandSingleBandGray drawing type detected..." );
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        QgsDebugMsg( "MultiBandSingleBandGray Not Set detected..." + mGrayBandName );
        break;
      }
      else
      {

        //get the band number for the mapped gray band
        drawMultiBandSingleBandGray( theQPainter, theRasterViewPort,
                                     theQgsMapToPixel, bandNumber( mGrayBandName ) );
        break;
      }
      //a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
    case MultiBandSingleBandPseudoColor:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {

        drawMultiBandSingleBandPseudoColor( theQPainter, theRasterViewPort,
                                            theQgsMapToPixel, bandNumber( mGrayBandName ) );
        break;
      }
      //a layer containing 2 or more bands, mapped to the three RGBcolors.
      //In the case of a multiband with only two bands,
      //one band will have to be mapped to more than one color
    case MultiBandColor:
      if ( mRedBandName == TRSTRING_NOT_SET ||
           mGreenBandName == TRSTRING_NOT_SET ||
           mBlueBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        int red = bandNumber( mRedBandName );
        int green = bandNumber( mGreenBandName );
        int blue = bandNumber( mBlueBandName );
        QgsBilinearRasterResampler resampler;
        QgsMultiBandColorRenderer r( mDataProvider, red, green, blue, &resampler );
        r.draw( theQPainter, theRasterViewPort, theQgsMapToPixel );
#if 0
        drawMultiBandColor( theQPainter, theRasterViewPort,
                            theQgsMapToPixel );
#endif //0
      }
      break;
    case SingleBandColorDataStyle:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        drawSingleBandColorData( theQPainter, theRasterViewPort,
                                 theQgsMapToPixel, bandNumber( mGrayBandName ) );
        break;
      }

    default:
      break;

  }
  QgsDebugMsg( QString( "raster draw time (ms): %1" ).arg( time.elapsed() ) );
} //end of draw method

QString QgsRasterLayer::drawingStyleAsString() const
{
  switch ( mDrawingStyle )
  {
    case SingleBandGray:
      return QString( "SingleBandGray" ); //no need to tr() this its not shown in ui
      break;
    case SingleBandPseudoColor:
      return QString( "SingleBandPseudoColor" );//no need to tr() this its not shown in ui
      break;
    case PalettedColor:
      return QString( "PalettedColor" );//no need to tr() this its not shown in ui
      break;
    case PalettedSingleBandGray:
      return QString( "PalettedSingleBandGray" );//no need to tr() this its not shown in ui
      break;
    case PalettedSingleBandPseudoColor:
      return QString( "PalettedSingleBandPseudoColor" );//no need to tr() this its not shown in ui
      break;
    case PalettedMultiBandColor:
      return QString( "PalettedMultiBandColor" );//no need to tr() this its not shown in ui
      break;
    case MultiBandSingleBandGray:
      return QString( "MultiBandSingleBandGray" );//no need to tr() this its not shown in ui
      break;
    case MultiBandSingleBandPseudoColor:
      return QString( "MultiBandSingleBandPseudoColor" );//no need to tr() this its not shown in ui
      break;
    case MultiBandColor:
      return QString( "MultiBandColor" );//no need to tr() this its not shown in ui
      break;
    case SingleBandColorDataStyle:
      return QString( "SingleBandColorDataStyle" );//no need to tr() this its not shown in ui
      break;
    default:
      break;
  }

  return QString( "UndefinedDrawingStyle" );

}

/**
 * @note Not implemented yet
 * @return always returns false
 */
bool QgsRasterLayer::hasCompatibleSymbology( const QgsMapLayer& theOther ) const
{
  //prevent warnings
  if ( theOther.type() < 0 )
  {
    return false;
  }
  return false;
} //todo

/**
 * @param theBandNo The number of the band to check
 * @return true if statistics have already been build for this band otherwise false
 */
bool QgsRasterLayer::hasStatistics( int theBandNo )
{
  if ( theBandNo <= mRasterStatsList.size() && theBandNo > 0 )
  {
    //vector starts at base 0, band counts at base1!
    return mRasterStatsList[theBandNo - 1].statsGathered;
  }
  else
  {
    return false;
  }
}

/**
 * @param thePoint the QgsPoint for which to obtain pixel values
 * @param theResults QMap to hold the pixel values at thePoint for each layer in the raster file
 * @return False if WMS layer and true otherwise
 */
bool QgsRasterLayer::identify( const QgsPoint& thePoint, QMap<QString, QString>& theResults )
{
  theResults.clear();

  QgsDebugMsg( "identify provider : " + mProviderKey ) ;
  return ( mDataProvider->identify( thePoint, theResults ) );
}

/**
 * @note  The arbitraryness of the returned document is enforced by WMS standards up to at least v1.3.0
 *
 * @param thePoint  an image pixel coordinate in the last requested extent of layer.
 * @return  A text document containing the return from the WMS server
 */
QString QgsRasterLayer::identifyAsText( const QgsPoint& thePoint )
{
  if ( mProviderKey != "wms" )
  {
    // Currently no meaning for anything other than OGC WMS layers
    return QString();
  }

  return mDataProvider->identifyAsText( thePoint );
}

/**
 * @note  The arbitraryness of the returned document is enforced by WMS standards up to at least v1.3.0
 *
 * @param thePoint  an image pixel coordinate in the last requested extent of layer.
 * @return  A html document containing the return from the WMS server
 */
QString QgsRasterLayer::identifyAsHtml( const QgsPoint& thePoint )
{
  if ( mProviderKey != "wms" )
  {
    // Currently no meaning for anything other than OGC WMS layers
    return QString();
  }

  return mDataProvider->identifyAsHtml( thePoint );
}

/**
 * @note Note implemented yet
 * @return Always returns false
 */
bool QgsRasterLayer::isEditable() const
{
  return false;
}

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
  if ( mDrawingStyle == SingleBandGray || mDrawingStyle == PalettedSingleBandGray || mDrawingStyle == MultiBandSingleBandGray )
  {
    //add min/max from contrast enhancement
    QString grayBand = grayBandName();
    if ( !grayBand.isEmpty() )
    {
      int grayBandNr = bandNumber( grayBand );
      const QgsContrastEnhancement* ceh = constContrastEnhancement( grayBandNr );
      if ( ceh )
      {
        QgsContrastEnhancement::ContrastEnhancementAlgorithm alg = ceh->contrastEnhancementAlgorithm();
        if ( alg == QgsContrastEnhancement::NoEnhancement
             || alg == QgsContrastEnhancement::ClipToMinimumMaximum )
        {
          //diffcult to display a meaningful item
          symbolList.push_back( qMakePair( QString::number( ceh->minimumValue() ) + "-" + QString::number( ceh->maximumValue() ), QColor( 125, 125, 125 ) ) );
        }
        else
        {
          symbolList.push_back( qMakePair( QString::number( ceh->minimumValue() ), QColor( 0, 0, 0 ) ) );
          symbolList.push_back( qMakePair( QString::number( ceh->maximumValue() ), QColor( 255, 255, 255 ) ) );
        }
      }
    }
  }
  else
  {
    switch ( mColorShadingAlgorithm )
    {
      case ColorRampShader:
      {
        const QgsColorRampShader* crShader = dynamic_cast<QgsColorRampShader*>( mRasterShader->rasterShaderFunction() );
        if ( crShader )
        {
          QList<QgsColorRampShader::ColorRampItem> shaderItems = crShader->colorRampItemList();
          QList<QgsColorRampShader::ColorRampItem>::const_iterator itemIt = shaderItems.constBegin();
          for ( ; itemIt != shaderItems.constEnd(); ++itemIt )
          {
            symbolList.push_back( qMakePair( itemIt->label, itemIt->color ) );
          }
        }
        break;
      }
      case PseudoColorShader:
      {
        //class breaks have fixed color for the pseudo color shader
        const QgsPseudoColorShader* pcShader = dynamic_cast<QgsPseudoColorShader*>( mRasterShader->rasterShaderFunction() );
        if ( pcShader )
        {
          symbolList.push_back( qMakePair( QString::number( pcShader->classBreakMin1() ), QColor( 0, 0, 255 ) ) );
          symbolList.push_back( qMakePair( QString::number( pcShader->classBreakMax1() ), QColor( 0, 255, 255 ) ) );
          symbolList.push_back( qMakePair( QString::number( pcShader->classBreakMax2() ), QColor( 255, 255, 0 ) ) );
          symbolList.push_back( qMakePair( QString::number( pcShader->maximumValue() ), QColor( 255, 0, 0 ) ) );
        }
        break;
      }
      case FreakOutShader:
      {
        const QgsFreakOutShader* foShader = dynamic_cast<QgsFreakOutShader*>( mRasterShader->rasterShaderFunction() );
        if ( foShader )
        {
          symbolList.push_back( qMakePair( QString::number( foShader->classBreakMin1() ), QColor( 255, 0, 255 ) ) );
          symbolList.push_back( qMakePair( QString::number( foShader->classBreakMax1() ), QColor( 0, 255, 255 ) ) );
          symbolList.push_back( qMakePair( QString::number( foShader->classBreakMax2() ), QColor( 255, 0, 0 ) ) );
          symbolList.push_back( qMakePair( QString::number( foShader->maximumValue() ), QColor( 0, 255, 0 ) ) );
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  return symbolList;
}

/**
 * This is an overloaded version of the legendAsPixmap( bool ) assumes false for the legend name flag.
 * @return a pixmap representing a legend image
 */
QPixmap QgsRasterLayer::legendAsPixmap()
{
  return legendAsPixmap( false );
}

/**
 * @param theWithNameFlag - boolena flag whether to overlay the legend name in the text
 * @return a pixmap representing a legend image
 */
QPixmap QgsRasterLayer::legendAsPixmap( bool theWithNameFlag )
{
  QgsDebugMsg( "called (" + drawingStyleAsString() + ")" );

  QPixmap myLegendQPixmap;      //will be initialised once we know what drawing style is active
  QPainter myQPainter;


  if ( !mProviderKey.isEmpty() )
  {
    QgsDebugMsg( "provider Key (" + mProviderKey + ")" );
    myLegendQPixmap = QPixmap( 3, 1 );
    myQPainter.begin( &myLegendQPixmap );
    //draw legend red part
    myQPainter.setPen( QPen( QColor( 255,   0,   0 ), 0 ) );
    myQPainter.drawPoint( 0, 0 );
    //draw legend green part
    myQPainter.setPen( QPen( QColor( 0, 255,   0 ), 0 ) );
    myQPainter.drawPoint( 1, 0 );
    //draw legend blue part
    myQPainter.setPen( QPen( QColor( 0,   0, 255 ), 0 ) );
    myQPainter.drawPoint( 2, 0 );

  }
  else
  {
    // Legacy GDAL (non-provider)

    //
    // Get the adjusted matrix stats
    //
    QString myColorerpretation = mDataProvider->colorInterpretationName( 1 );

    //
    // Create the legend pixmap - note it is generated on the preadjusted stats
    //
    if ( mDrawingStyle == MultiBandSingleBandGray || mDrawingStyle == PalettedSingleBandGray || mDrawingStyle == SingleBandGray )
    {

      myLegendQPixmap = QPixmap( 100, 1 );
      myQPainter.begin( &myLegendQPixmap );
      int myPos = 0;
      for ( double my = 0; my < 255; my += 2.55 )
      {
        if ( !mInvertColor ) //histogram is not inverted
        {
          //draw legend as grayscale
          int myGray = static_cast < int >( my );
          myQPainter.setPen( QPen( QColor( myGray, myGray, myGray ), 0 ) );
        }
        else                //histogram is inverted
        {
          //draw legend as inverted grayscale
          int myGray = 255 - static_cast < int >( my );
          myQPainter.setPen( QPen( QColor( myGray, myGray, myGray ), 0 ) );
        }                   //end of invert histogram  check
        myQPainter.drawPoint( myPos++, 0 );
      }
    }                           //end of gray check
    else if ( mDrawingStyle == MultiBandSingleBandPseudoColor ||
              mDrawingStyle == PalettedSingleBandPseudoColor || mDrawingStyle == SingleBandPseudoColor )
    {

      //set up the three class breaks for pseudocolor mapping
      double myRangeSize = 90;  //hard coded for now
      double myBreakSize = myRangeSize / 3;
      double myClassBreakMin1 = 0;
      double myClassBreakMax1 = myClassBreakMin1 + myBreakSize;
      double myClassBreakMin2 = myClassBreakMax1;
      double myClassBreakMax2 = myClassBreakMin2 + myBreakSize;
      double myClassBreakMin3 = myClassBreakMax2;

      //
      // Create the legend pixmap - note it is generated on the preadjusted stats
      //
      myLegendQPixmap = QPixmap( 100, 1 );
      myQPainter.begin( &myLegendQPixmap );
      int myPos = 0;
      for ( double my = 0; my < myRangeSize; my += myRangeSize / 100.0 )
      {
        //draw pseudocolor legend
        if ( !mInvertColor )
        {
          //check if we are in the first class break
          if (( my >= myClassBreakMin1 ) && ( my < myClassBreakMax1 ) )
          {
            int myRed = 0;
            int myBlue = 255;
            int myGreen = static_cast < int >((( 255 / myRangeSize ) * ( my - myClassBreakMin1 ) ) * 3 );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myRed = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
          //check if we are in the second class break
          else if (( my >= myClassBreakMin2 ) && ( my < myClassBreakMax2 ) )
          {
            int myRed = static_cast < int >((( 255 / myRangeSize ) * (( my - myClassBreakMin2 ) / 1 ) ) * 3 );
            int myBlue = static_cast < int >( 255 - ((( 255 / myRangeSize ) * (( my - myClassBreakMin2 ) / 1 ) ) * 3 ) );
            int myGreen = 255;
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myGreen = myBlue;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRed = 255;
            int myBlue = 0;
            int myGreen = static_cast < int >( 255 - ((( 255 / myRangeSize ) * (( my - myClassBreakMin3 ) / 1 ) * 3 ) ) );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myRed = myGreen;
              myGreen = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
        }                   //end of invert !histogram false check
        else                  //invert histogram toggle is off
        {
          //check if we are in the first class break
          if (( my >= myClassBreakMin1 ) && ( my < myClassBreakMax1 ) )
          {
            int myRed = 255;
            int myBlue = 0;
            int myGreen = static_cast < int >((( 255 / myRangeSize ) * (( my - myClassBreakMin1 ) / 1 ) * 3 ) );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myRed = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
          //check if we are in the second class break
          else if (( my >= myClassBreakMin2 ) && ( my < myClassBreakMax2 ) )
          {
            int myRed = static_cast < int >( 255 - ((( 255 / myRangeSize ) * (( my - myClassBreakMin2 ) / 1 ) ) * 3 ) );
            int myBlue = static_cast < int >((( 255 / myRangeSize ) * (( my - myClassBreakMin2 ) / 1 ) ) * 3 );
            int myGreen = 255;
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myGreen = myBlue;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRed = 0;
            int myBlue = 255;
            int myGreen = static_cast < int >( 255 - ((( 255 / myRangeSize ) * ( my - myClassBreakMin3 ) ) * 3 ) );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myRed = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }

        }                   //end of invert histogram check
        myQPainter.drawPoint( myPos++, 0 );
      }

    }                           //end of pseudocolor check
    else if ( mDrawingStyle == PalettedMultiBandColor || mDrawingStyle == MultiBandColor || mDrawingStyle == PalettedColor )
    {
      //
      // Create the legend pixmap showing red green and blue band mappings
      //
      // TODO update this so it actually shows the mappings for paletted images
      myLegendQPixmap = QPixmap( 3, 1 );
      myQPainter.begin( &myLegendQPixmap );
      //draw legend red part
      myQPainter.setPen( QPen( QColor( 224, 103, 103 ), 0 ) );
      myQPainter.drawPoint( 0, 0 );
      //draw legend green part
      myQPainter.setPen( QPen( QColor( 132, 224, 127 ), 0 ) );
      myQPainter.drawPoint( 1, 0 );
      //draw legend blue part
      myQPainter.setPen( QPen( QColor( 127, 160, 224 ), 0 ) );
      myQPainter.drawPoint( 2, 0 );
    }
  }

  myQPainter.end();


  // see if the caller wants the name of the layer in the pixmap (used for legend bar)
  if ( theWithNameFlag )
  {
    QFont myQFont( "arial", 10, QFont::Normal );
    QFontMetrics myQFontMetrics( myQFont );

    int myHeight = ( myQFontMetrics.height() + 10 > 35 ) ? myQFontMetrics.height() + 10 : 35;

    //create a matrix to
    QMatrix myQWMatrix;
    //scale the raster legend up a bit bigger to the legend item size
    //note that scaling parameters are factors, not absolute values,
    // so scale (0.25,1) scales the painter to a quarter of its size in the x direction
    //TODO We need to decide how much to scale by later especially for rgb images which are only 3x1 pix
    //hard coding thes values for now.
    if ( myLegendQPixmap.width() == 3 )
    {
      //scale width by factor of 50 (=150px wide)
      myQWMatrix.scale( 60, myHeight );
    }
    else
    {
      //assume 100px so scale by factor of 1.5 (=150px wide)
      myQWMatrix.scale( 1.8, myHeight );
    }
    //apply the matrix
    QPixmap myQPixmap2 = myLegendQPixmap.transformed( myQWMatrix );
    QPainter myQPainter( &myQPixmap2 );

    //load  up the pyramid icons
    QString myThemePath = QgsApplication::activeThemePath();
    QPixmap myPyramidPixmap( myThemePath + "/mIconPyramid.png" );
    QPixmap myNoPyramidPixmap( myThemePath + "/mIconNoPyramid.png" );

    //
    // Overlay a pyramid icon
    //
    if ( mHasPyramids )
    {
      myQPainter.drawPixmap( 0, myHeight - myPyramidPixmap.height(), myPyramidPixmap );
    }
    else
    {
      myQPainter.drawPixmap( 0, myHeight - myNoPyramidPixmap.height(), myNoPyramidPixmap );
    }
    //
    // Overlay the layer name
    //
    if ( mDrawingStyle == MultiBandSingleBandGray || mDrawingStyle == PalettedSingleBandGray || mDrawingStyle == SingleBandGray )
    {
      myQPainter.setPen( Qt::white );
    }
    else
    {
      myQPainter.setPen( Qt::black );
    }
    myQPainter.setFont( myQFont );
    myQPainter.drawText( 25, myHeight - 10, name() );
    //
    // finish up
    //
    myLegendQPixmap = myQPixmap2;
    myQPainter.end();
  }
  //finish up

  return myLegendQPixmap;

}                               //end of legendAsPixmap function

/**
 * \param theLabelCount number of vertical labels to display
 * @return a pixmap representing a legend image
 */
QPixmap QgsRasterLayer::legendAsPixmap( int theLabelCount )
{
  QgsDebugMsg( "entered." );
  QFont myQFont( "arial", 10, QFont::Normal );
  QFontMetrics myQFontMetrics( myQFont );

  int myFontHeight = ( myQFontMetrics.height() );
  const int myerLabelSpacing = 5;
  int myImageHeight = (( myFontHeight + ( myerLabelSpacing * 2 ) ) * theLabelCount );
  //these next two vars are not used anywhere so commented out for now
  //int myLongestLabelWidth =  myQFontMetrics.width(name());
  //const int myHorizontalLabelSpacing = 5;
  const int myColorBarWidth = 10;
  //
  // Get the adjusted matrix stats
  //
  QString myColorerpretation = mDataProvider->colorInterpretationName( 1 );
  QPixmap myLegendQPixmap;      //will be initialised once we know what drawing style is active
  QPainter myQPainter;
  //
  // Create the legend pixmap - note it is generated on the preadjusted stats
  //
  if ( mDrawingStyle == MultiBandSingleBandGray || mDrawingStyle == PalettedSingleBandGray || mDrawingStyle == SingleBandGray )
  {

    myLegendQPixmap = QPixmap( 1, myImageHeight );
    const double myIncrement = static_cast<double>( myImageHeight ) / 255.0;
    myQPainter.begin( &myLegendQPixmap );
    int myPos = 0;
    for ( double my = 0; my < 255; my += myIncrement )
    {
      if ( !mInvertColor ) //histogram is not inverted
      {
        //draw legend as grayscale
        int myGray = static_cast < int >( my );
        myQPainter.setPen( QPen( QColor( myGray, myGray, myGray ), 0 ) );
      }
      else                //histogram is inverted
      {
        //draw legend as inverted grayscale
        int myGray = 255 - static_cast < int >( my );
        myQPainter.setPen( QPen( QColor( myGray, myGray, myGray ), 0 ) );
      }                   //end of invert histogram  check
      myQPainter.drawPoint( 0, myPos++ );
    }
  }                           //end of gray check
  else if ( mDrawingStyle == MultiBandSingleBandPseudoColor ||
            mDrawingStyle == PalettedSingleBandPseudoColor || mDrawingStyle == SingleBandPseudoColor )
  {

    //set up the three class breaks for pseudocolor mapping
    double myRangeSize = 90;  //hard coded for now
    double myBreakSize = myRangeSize / 3;
    double myClassBreakMin1 = 0;
    double myClassBreakMax1 = myClassBreakMin1 + myBreakSize;
    double myClassBreakMin2 = myClassBreakMax1;
    double myClassBreakMax2 = myClassBreakMin2 + myBreakSize;
    double myClassBreakMin3 = myClassBreakMax2;

    //
    // Create the legend pixmap - note it is generated on the preadjusted stats
    //
    myLegendQPixmap = QPixmap( 1, myImageHeight );
    const double myIncrement = myImageHeight / myRangeSize;
    myQPainter.begin( &myLegendQPixmap );
    int myPos = 0;
    for ( double my = 0; my < 255; my += myIncrement )
      for ( double my = 0; my < myRangeSize; my += myIncrement )
      {
        //draw pseudocolor legend
        if ( !mInvertColor )
        {
          //check if we are in the first class break
          if (( my >= myClassBreakMin1 ) && ( my < myClassBreakMax1 ) )
          {
            int myRed = 0;
            int myBlue = 255;
            int myGreen = static_cast < int >((( 255 / myRangeSize ) * ( my - myClassBreakMin1 ) ) * 3 );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myRed = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
          //check if we are in the second class break
          else if (( my >= myClassBreakMin2 ) && ( my < myClassBreakMax2 ) )
          {
            int myRed = static_cast < int >((( 255 / myRangeSize ) * (( my - myClassBreakMin2 ) / 1 ) ) * 3 );
            int myBlue = static_cast < int >( 255 - ((( 255 / myRangeSize ) * (( my - myClassBreakMin2 ) / 1 ) ) * 3 ) );
            int myGreen = 255;
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myGreen = myBlue;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRed = 255;
            int myBlue = 0;
            int myGreen = static_cast < int >( 255 - ((( 255 / myRangeSize ) * (( my - myClassBreakMin3 ) / 1 ) * 3 ) ) );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myRed = myGreen;
              myGreen = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
        }                   //end of invert !histogram check
        else                  //invert histogram toggle is off
        {
          //check if we are in the first class break
          if (( my >= myClassBreakMin1 ) && ( my < myClassBreakMax1 ) )
          {
            int myRed = 255;
            int myBlue = 0;
            int myGreen = static_cast < int >((( 255 / myRangeSize ) * (( my - myClassBreakMin1 ) / 1 ) * 3 ) );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myRed = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
          //check if we are in the second class break
          else if (( my >= myClassBreakMin2 ) && ( my < myClassBreakMax2 ) )
          {
            int myRed = static_cast < int >( 255 - ((( 255 / myRangeSize ) * (( my - myClassBreakMin2 ) / 1 ) ) * 3 ) );
            int myBlue = static_cast < int >((( 255 / myRangeSize ) * (( my - myClassBreakMin2 ) / 1 ) ) * 3 );
            int myGreen = 255;
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myGreen = myBlue;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRed = 0;
            int myBlue = 255;
            int myGreen = static_cast < int >( 255 - ((( 255 / myRangeSize ) * ( my - myClassBreakMin3 ) ) * 3 ) );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FreakOutShader )
            {
              myRed = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }

        }                   //end of invert histogram check
        myQPainter.drawPoint( 0, myPos++ );
      }

  }                           //end of pseudocolor check
  else if ( mDrawingStyle == PalettedMultiBandColor || mDrawingStyle == MultiBandColor )
  {
    //
    // Create the legend pixmap showing red green and blue band mappings
    //
    // TODO update this so it actually shows the mappings for paletted images
    myLegendQPixmap = QPixmap( 1, 3 );
    myQPainter.begin( &myLegendQPixmap );
    //draw legend red part
    myQPainter.setPen( QPen( QColor( 224, 103, 103 ), 0 ) );
    myQPainter.drawPoint( 0, 0 );
    //draw legend green part
    myQPainter.setPen( QPen( QColor( 132, 224, 127 ), 0 ) );
    myQPainter.drawPoint( 0, 1 );
    //draw legend blue part
    myQPainter.setPen( QPen( QColor( 127, 160, 224 ), 0 ) );
    myQPainter.drawPoint( 0, 2 );
  }


  myQPainter.end();



  //create a matrix to
  QMatrix myQWMatrix;
  //scale the raster legend up a bit bigger to the legend item size
  //note that scaling parameters are factors, not absolute values,
  // so scale (0.25,1) scales the painter to a quarter of its size in the x direction
  //TODO We need to decide how much to scale by later especially for rgb images which are only 3x1 pix
  //hard coding thes values for now.
  if ( myLegendQPixmap.height() == 3 )
  {
    myQWMatrix.scale( myColorBarWidth, 2 );
  }
  else
  {
    myQWMatrix.scale( myColorBarWidth, 2 );
  }
  //apply the matrix
  QPixmap myQPixmap2 = myLegendQPixmap.transformed( myQWMatrix );
  QPainter myQPainter2( &myQPixmap2 );
  //
  // Overlay the layer name
  //
  if ( mDrawingStyle == MultiBandSingleBandGray || mDrawingStyle == PalettedSingleBandGray || mDrawingStyle == SingleBandGray )
  {
    myQPainter2.setPen( Qt::white );
  }
  else
  {
    myQPainter2.setPen( Qt::black );
  }
  myQPainter2.setFont( myQFont );
  myQPainter2.drawText( 25, myImageHeight - 10, name() );
  //
  // finish up
  //
  myLegendQPixmap = myQPixmap2;
  myQPainter2.end();
  //finish up

  return myLegendQPixmap;

}//end of getDetailedLegend

/**
 * @param theBand the band number for which to get the maximum pixel value
 * @return the maximum pixel value
 */
double QgsRasterLayer::maximumValue( unsigned int theBand )
{
  if ( 0 < theBand && theBand <= bandCount() )
  {
    return mContrastEnhancementList[theBand - 1].maximumValue();
  }

  return 0.0;
}

/**
 * @param theBand the band name for which to get the maximum pixel value
 * @return the maximum pixel value
 */
double QgsRasterLayer::maximumValue( QString theBand )
{
  if ( theBand != tr( "Not Set" ) )
  {
    return maximumValue( bandNumber( theBand ) );
  }

  return 0.0;
}


QString QgsRasterLayer::metadata()
{
  QString myMetadata ;
  myMetadata += "<p class=\"glossy\">" + tr( "Driver:" ) + "</p>\n";
  myMetadata += "<p>";
  myMetadata += mDataProvider->description();
  myMetadata += "</p>\n";

  // Insert provider-specific (e.g. WMS-specific) metadata
  // crashing
  //QString s =  mDataProvider->metadata();
  //QgsDebugMsg( s );
  myMetadata += mDataProvider->metadata();

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "No Data Value" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  if ( mValidNoDataValue )
  {
    myMetadata += QString::number( mNoDataValue );
  }
  else
  {
    myMetadata += "*" + tr( "NoDataValue not set" ) + "*";
  }
  myMetadata += "</p>\n";

  myMetadata += "</p>\n";
  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Data Type:" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  //just use the first band
  switch ( mDataProvider->srcDataType( 1 ) )
  {
    case GDT_Byte:
      myMetadata += tr( "GDT_Byte - Eight bit unsigned integer" );
      break;
    case GDT_UInt16:
      myMetadata += tr( "GDT_UInt16 - Sixteen bit unsigned integer " );
      break;
    case GDT_Int16:
      myMetadata += tr( "GDT_Int16 - Sixteen bit signed integer " );
      break;
    case GDT_UInt32:
      myMetadata += tr( "GDT_UInt32 - Thirty two bit unsigned integer " );
      break;
    case GDT_Int32:
      myMetadata += tr( "GDT_Int32 - Thirty two bit signed integer " );
      break;
    case GDT_Float32:
      myMetadata += tr( "GDT_Float32 - Thirty two bit floating point " );
      break;
    case GDT_Float64:
      myMetadata += tr( "GDT_Float64 - Sixty four bit floating point " );
      break;
    case GDT_CInt16:
      myMetadata += tr( "GDT_CInt16 - Complex Int16 " );
      break;
    case GDT_CInt32:
      myMetadata += tr( "GDT_CInt32 - Complex Int32 " );
      break;
    case GDT_CFloat32:
      myMetadata += tr( "GDT_CFloat32 - Complex Float32 " );
      break;
    case GDT_CFloat64:
      myMetadata += tr( "GDT_CFloat64 - Complex Float64 " );
      break;
    default:
      myMetadata += tr( "Could not determine raster data type." );
  }
  myMetadata += "</p>\n";

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Pyramid overviews:" );
  myMetadata += "</p>\n";
  myMetadata += "<p>";

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Layer Spatial Reference System: " );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata += mCRS->toProj4();
  myMetadata += "</p>\n";

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Layer Extent (layer original source projection): " );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata += mDataProvider->extent().toString();
  myMetadata += "</p>\n";

  // output coordinate system
  // TODO: this is not related to layer, to be removed? [MD]
#if 0
  myMetadata += "<tr><td class=\"glossy\">";
  myMetadata += tr( "Project Spatial Reference System: " );
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
    if ( !hasStatistics( myIteratorInt ) )  //not collected
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

      QgsRasterBandStats myRasterBandStats = bandStatistics( myIteratorInt );
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
 * @param theBand the band number for which to get the minimum pixel value
 * @return the minimum pixel value
 */
double QgsRasterLayer::minimumValue( unsigned int theBand )
{
  if ( 0 < theBand && theBand <= bandCount() )
  {
    return mContrastEnhancementList[theBand - 1].minimumValue();
  }

  return 0.0;
}

/**
 * @param theBand the band name for which to get the minimum pixel value
 * @return the minimum pixel value
 */
double QgsRasterLayer::minimumValue( QString theBand )
{
  return minimumValue( bandNumber( theBand ) );
}

/**
 * @param theBandNumber the number of the band to use for generating a pixmap of the associated palette
 * @return a 100x100 pixel QPixmap of the bands palette
 */
QPixmap QgsRasterLayer::paletteAsPixmap( int theBandNumber )
{
  //TODO: This function should take dimensions
  QgsDebugMsg( "entered." );

  // Only do this for the non-provider (hard-coded GDAL) scenario...
  // Maybe WMS can do this differently using QImage::numColors and QImage::color()
  if ( mProviderKey.isEmpty() && hasBand( "Palette" ) && theBandNumber > 0 ) //don't tr() this its a gdal word!
  {
    QgsDebugMsg( "....found paletted image" );
    QgsColorRampShader myShader;
    //QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = myShader.colorRampItemList();

    //if ( readColorTable( 1, &myColorRampItemList ) )
    QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = mDataProvider->colorTable( 1 );
    // TODO: add CT capability? It can depends on band (?)
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
          int c1, c2, c3;
          myShader.shade( myValue, &c1, &c2, &c3 );
          myLineBuffer[ myCol ] = qRgb( c1, c2, c3 );
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

/*
 * @param theBandNoInt - which band to compute the histogram for
 * @param theBinCountInt - how many 'bins' to categorise the data into
 * @param theIgnoreOutOfRangeFlag - whether to ignore values that are out of range (default=true)
 * @param theThoroughBandScanFlag - whether to visit each cell when computing the histogram (default=false)
 */
void QgsRasterLayer::populateHistogram( int theBandNo, int theBinCount, bool theIgnoreOutOfRangeFlag, bool theHistogramEstimatedFlag )
{
  QgsRasterBandStats myRasterBandStats = bandStatistics( theBandNo );
  mDataProvider->populateHistogram( theBandNo, myRasterBandStats, theBinCount, theIgnoreOutOfRangeFlag, theHistogramEstimatedFlag );
}

QString QgsRasterLayer::providerType() const
{
  if ( mProviderKey.isEmpty() )
  {
    return QString();
  }
  else
  {
    return mProviderKey;
  }
}

/**
 * @return the horizontal units per pixel as reported in the  GDAL geotramsform[1]
 */
double QgsRasterLayer::rasterUnitsPerPixel()
{
// We return one raster pixel per map unit pixel
// One raster pixel can have several raster units...

// We can only use one of the mGeoTransform[], so go with the
// horisontal one.

  //return qAbs( mGeoTransform[1] );
  if ( mDataProvider->capabilities() & QgsRasterDataProvider::ExactResolution && mDataProvider->xSize() > 0 )
  {
    return mDataProvider->extent().width() / mDataProvider->xSize();
  }
  return 1;
}


void QgsRasterLayer::resetNoDataValue()
{
  mNoDataValue = std::numeric_limits<int>::max();
  mValidNoDataValue = false;
  if ( mDataProvider != NULL && mDataProvider->bandCount() > 0 )
  {
    // TODO: add 'has null value' to capabilities
#if 0
    int myRequestValid;
    myRequestValid = 1;
    double myValue = mDataProvider->noDataValue();

    if ( 0 != myRequestValid )
    {
      setNoDataValue( myValue );
    }
    else
    {
      setNoDataValue( -9999.0 );
      mValidNoDataValue = false;

    }
#endif
    setNoDataValue( mDataProvider->noDataValue() );
    mValidNoDataValue = mDataProvider->isNoDataValueValid();
  }
}


void QgsRasterLayer::setBlueBandName( QString const & theBandName )
{
  mBlueBandName = validateBandName( theBandName );
}

void QgsRasterLayer::init()
{
  // keep this until mGeoTransform occurences are removed!
  mGeoTransform[0] =  0;
  mGeoTransform[1] =  1;
  mGeoTransform[2] =  0;
  mGeoTransform[3] =  0;
  mGeoTransform[4] =  0;
  mGeoTransform[5] = -1;


  mRasterType = QgsRasterLayer::GrayOrUndefined;

  mRedBandName = TRSTRING_NOT_SET;
  mGreenBandName = TRSTRING_NOT_SET;
  mBlueBandName = TRSTRING_NOT_SET;
  mGrayBandName = TRSTRING_NOT_SET;
  mTransparencyBandName = TRSTRING_NOT_SET;


  mUserDefinedRGBMinimumMaximum = false; //defaults needed to bypass enhanceContrast
  mUserDefinedGrayMinimumMaximum = false;
  mRGBMinimumMaximumEstimated = true;
  mGrayMinimumMaximumEstimated = true;

  mDrawingStyle = QgsRasterLayer::UndefinedDrawingStyle;
  mContrastEnhancementAlgorithm = QgsContrastEnhancement::NoEnhancement;
  mColorShadingAlgorithm = QgsRasterLayer::UndefinedShader;
  mRasterShader = new QgsRasterShader();

  mBandCount = 0;
  mHasPyramids = false;
  mNoDataValue = -9999.0;
  mValidNoDataValue = false;

  //Initialize the last view port structure, should really be a class
  mLastViewPort.drawableAreaXDim = 0;
  mLastViewPort.drawableAreaYDim = 0;
}

QLibrary* QgsRasterLayer::loadProviderLibrary( QString theProviderKey )
{
  QgsDebugMsg( "theProviderKey = " + theProviderKey );
  // load the plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QString myLibPath = pReg->library( theProviderKey );
  QgsDebugMsg( "myLibPath = " + myLibPath );

#ifdef TESTPROVIDERLIB
  const char *cOgrLib = ( const char * ) myLibPath;
  // test code to help debug provider loading problems
  //  void *handle = dlopen(cOgrLib, RTLD_LAZY);
  void *handle = dlopen( cOgrLib, RTLD_LAZY | RTLD_GLOBAL );
  if ( !handle )
  {
    QgsLogger::warning( "Error in dlopen: " );
  }
  else
  {
    QgsDebugMsg( "dlopen suceeded" );
    dlclose( handle );
  }

#endif

  // load the data provider
  QLibrary*  myLib = new QLibrary( myLibPath );

  QgsDebugMsg( "Library name is " + myLib->fileName() );
  bool loaded = myLib->load();

  if ( !loaded )
  {
    QgsMessageLog::logMessage( tr( "Failed to load provider %1 (Reason: %2)" ).arg( myLib->fileName() ).arg( myLib->errorString() ), tr( "Raster" ) );
    return NULL;
  }
  QgsDebugMsg( "Loaded data provider library" );
  return myLib;
}

// This code should be shared also by vector layer -> move it to QgsMapLayer
QgsRasterDataProvider* QgsRasterLayer::loadProvider( QString theProviderKey, QString theDataSource )
{
  QgsDebugMsg( "Entered" );
  QLibrary*  myLib = QgsRasterLayer::loadProviderLibrary( theProviderKey );
  QgsDebugMsg( "Library loaded" );
  if ( !myLib )
  {
    QgsDebugMsg( "myLib is NULL" );
    return NULL;
  }

  QgsDebugMsg( "Attempting to resolve the classFactory function" );
  classFactoryFunction_t * classFactory = ( classFactoryFunction_t * ) cast_to_fptr( myLib->resolve( "classFactory" ) );

  if ( !classFactory )
  {
    QgsMessageLog::logMessage( tr( "Cannot resolve the classFactory function" ), tr( "Raster" ) );
    return NULL;
  }
  QgsDebugMsg( "Getting pointer to a mDataProvider object from the library" );
  //XXX - This was a dynamic cast but that kills the Windows
  //      version big-time with an abnormal termination error
  //      mDataProvider = (QgsRasterDataProvider*)(classFactory((const
  //                                              char*)(dataSource.utf8())));

  // Copied from qgsproviderregistry in preference to the above.
  QgsRasterDataProvider* myDataProvider = ( QgsRasterDataProvider* )( *classFactory )( &theDataSource );

  if ( !myDataProvider )
  {
    QgsMessageLog::logMessage( tr( "Cannot to instantiate the data provider" ), tr( "Raster" ) );
    return NULL;
  }
  QgsDebugMsg( "Data driver created" );
  return myDataProvider;
}

void QgsRasterLayer::setDataProvider( QString const & provider,
                                      QStringList const & layers,
                                      QStringList const & styles,
                                      QString const & format,
                                      QString const & crs )
{
  setDataProvider( provider, layers, styles, format, crs, false );
}

/** Copied from QgsVectorLayer::setDataProvider
 *  TODO: Make it work in the raster environment
 */
void QgsRasterLayer::setDataProvider( QString const & provider,
                                      QStringList const & layers,
                                      QStringList const & styles,
                                      QString const & format,
                                      QString const & crs,
                                      bool loadDefaultStyleFlag )
{
  Q_UNUSED( loadDefaultStyleFlag );
  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  mProviderKey = provider;
  mValid = false;            // assume the layer is invalid until we determine otherwise

  // set the layer name (uppercase first character)

  if ( ! mLayerName.isEmpty() )   // XXX shouldn't this happen in parent?
  {
    setLayerName( mLayerName );
  }

  mBandCount = 0;
  mRasterShader = new QgsRasterShader();

  mDataProvider = QgsRasterLayer::loadProvider( mProviderKey, mDataSource );
  if ( !mDataProvider )
  {
    return;
  }


  QgsDebugMsg( "Instantiated the data provider plugin"
               + QString( " with layer list of " ) + layers.join( ", " )
               + " and style list of " + styles.join( ", " )
               + " and format of " + format +  " and CRS of " + crs );
  if ( !mDataProvider->isValid() )
  {
    if ( provider != "gdal" || !layers.isEmpty() || !styles.isEmpty() || !format.isNull() || !crs.isNull() )
    {
      QgsMessageLog::logMessage( tr( "Data provider is invalid (layers %1, styles %2, formats: %3)" )
                                 .arg( layers.join( ", " ) )
                                 .arg( styles.join( ", " ) )
                                 .arg( format ),
                                 tr( "Raster" ) );
    }
    return;
  }

  mDataProvider->addLayers( layers, styles );
  mDataProvider->setImageEncoding( format );
  mDataProvider->setImageCrs( crs );

  setNoDataValue( mDataProvider->noDataValue() );

  // get the extent
  QgsRectangle mbr = mDataProvider->extent();

  // show the extent
  QString s = mbr.toString();
  QgsDebugMsg( "Extent of layer: " + s );
  // store the extent
  mLayerExtent.setXMaximum( mbr.xMaximum() );
  mLayerExtent.setXMinimum( mbr.xMinimum() );
  mLayerExtent.setYMaximum( mbr.yMaximum() );
  mLayerExtent.setYMinimum( mbr.yMinimum() );

  mWidth = mDataProvider->xSize();
  mHeight = mDataProvider->ySize();


  // upper case the first letter of the layer name
  QgsDebugMsg( "mLayerName: " + name() );

  // set up the raster drawing style
  mDrawingStyle = MultiBandColor;  //sensible default

  // Setup source CRS
  if ( mProviderKey == "wms" )
  {
    *mCRS = QgsCoordinateReferenceSystem();
    mCRS->createFromOgcWmsCrs( crs );
  }
  else
  {
    *mCRS = QgsCoordinateReferenceSystem( mDataProvider->crs() );
  }
  //get the project projection, defaulting to this layer's projection
  //if none exists....
  if ( !mCRS->isValid() )
  {
    mCRS->setValidationHint( tr( "Specify CRS for layer %1" ).arg( name() ) );
    mCRS->validate();
  }
  QString mySourceWkt = mCRS->toWkt();

  QgsDebugMsg( "using wkt:\n" + mySourceWkt );

  mBandCount = mDataProvider->bandCount( );
  for ( int i = 1; i <= mBandCount; i++ )
  {
    QgsRasterBandStats myRasterBandStats;
    myRasterBandStats.bandName = mDataProvider->generateBandName( i );
    myRasterBandStats.bandNumber = i;
    myRasterBandStats.statsGathered = false;
    myRasterBandStats.histogramVector->clear();
    //Store the default color table
    //readColorTable( i, &myRasterBandStats.colorTable );
    QList<QgsColorRampShader::ColorRampItem> ct;
    ct = mDataProvider->colorTable( i );
    myRasterBandStats.colorTable = ct;

    mRasterStatsList.push_back( myRasterBandStats );

    //Build a new contrast enhancement for the band and store in list
    //QgsContrastEnhancement myContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )mDataProvider->dataType( i ) );
    QgsContrastEnhancement myContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )mDataProvider->srcDataType( i ) );
    mContrastEnhancementList.append( myContrastEnhancement );
  }

  //defaults - Needs to be set after the Contrast list has been build
  //Try to read the default contrast enhancement from the config file

  QSettings myQSettings;
  setContrastEnhancementAlgorithm( myQSettings.value( "/Raster/defaultContrastEnhancementAlgorithm", "NoEnhancement" ).toString() );

  //decide what type of layer this is...
  //TODO Change this to look at the color interp and palette interp to decide which type of layer it is
  QgsDebugMsg( "bandCount = " + QString::number( mDataProvider->bandCount() ) );
  QgsDebugMsg( "dataType = " + QString::number( mDataProvider->dataType( 1 ) ) );
  if (( mDataProvider->bandCount() > 1 ) )
  {
    mRasterType = Multiband;
  }
  else if ( mDataProvider->dataType( 1 ) == QgsRasterDataProvider::ARGBDataType )
  {
    mRasterType = ColorLayer;
  }
  //TODO hasBand is really obsolete and only used in the Palette instance, change to new function hasPalette(int)
  //else if ( hasBand( "Palette" ) ) //don't tr() this its a gdal word!
  // not sure if is worth to add colorTable capability - CT can be empty in any case
  // Calc bandStatistics is very slow!!!
  //else if ( bandStatistics(1).colorTable.count() > 0 )
  else if ( mDataProvider->colorInterpretation( 1 ) == QgsRasterDataProvider::PaletteIndex )
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
    QgsDebugMsg( "Setting mDrawingStyle to SingleBandColorDataStyle " + QString::number( SingleBandColorDataStyle ) );
    mDrawingStyle = SingleBandColorDataStyle;
    mGrayBandName = bandName( 1 );  //sensible default
  }
  else if ( mRasterType == Palette )
  {
    mRedBandName = TRSTRING_NOT_SET; // sensible default
    mGreenBandName = TRSTRING_NOT_SET; // sensible default
    mBlueBandName = TRSTRING_NOT_SET;// sensible default
    mTransparencyBandName = TRSTRING_NOT_SET; // sensible default
    mGrayBandName = bandName( 1 );  //sensible default
    QgsDebugMsg( mGrayBandName );

    mDrawingStyle = PalettedColor; //sensible default

    //Set up a new color ramp shader
    setColorShadingAlgorithm( ColorRampShader );
    QgsColorRampShader* myColorRampShader = ( QgsColorRampShader* ) mRasterShader->rasterShaderFunction();
    //TODO: Make sure the set algorithm and cast was successful,
    //e.g., if ( 0 != myColorRampShader && myColorRampShader->shaderTypeAsString == "ColorRampShader" )
    myColorRampShader->setColorRampType( QgsColorRampShader::INTERPOLATED );
    myColorRampShader->setColorRampItemList( *colorTable( 1 ) );
  }
  else if ( mRasterType == Multiband )
  {
    //we know we have at least 2 layers...
    mRedBandName = bandName( myQSettings.value( "/Raster/defaultRedBand", 1 ).toInt() );  // sensible default
    mGreenBandName = bandName( myQSettings.value( "/Raster/defaultGreenBand", 2 ).toInt() );  // sensible default

    //Check to make sure preferred bands combinations are valid
    if ( mRedBandName.isEmpty() )
    {
      mRedBandName = bandName( 1 );
    }

    if ( mGreenBandName.isEmpty() )
    {
      mGreenBandName = bandName( 2 );
    }

    //for the third band we cant be sure so..
    if (( mDataProvider->bandCount() > 2 ) )
    {
      mBlueBandName = bandName( myQSettings.value( "/Raster/defaultBlueBand", 3 ).toInt() ); // sensible default
      if ( mBlueBandName.isEmpty() )
      {
        mBlueBandName = bandName( 3 );
      }
    }
    else
    {
      mBlueBandName = bandName( myQSettings.value( "/Raster/defaultBlueBand", 2 ).toInt() );  // sensible default
      if ( mBlueBandName.isEmpty() )
      {
        mBlueBandName = bandName( 2 );
      }
    }


    mTransparencyBandName = TRSTRING_NOT_SET;
    mGrayBandName = TRSTRING_NOT_SET;  //sensible default
    mDrawingStyle = MultiBandColor;  //sensible default

    // read standard deviations
    if ( mContrastEnhancementAlgorithm == QgsContrastEnhancement::StretchToMinimumMaximum )
    {
      setStandardDeviations( myQSettings.value( "/Raster/defaultStandardDeviation", 2.0 ).toInt() );
    }
  }
  else                        //GrayOrUndefined
  {
    mRedBandName = TRSTRING_NOT_SET; //sensible default
    mGreenBandName = TRSTRING_NOT_SET; //sensible default
    mBlueBandName = TRSTRING_NOT_SET;  //sensible default
    mTransparencyBandName = TRSTRING_NOT_SET;  //sensible default
    mDrawingStyle = SingleBandGray;  //sensible default
    mGrayBandName = bandName( 1 );

    // read standard deviations
    if ( mContrastEnhancementAlgorithm == QgsContrastEnhancement::StretchToMinimumMaximum )
    {
      setStandardDeviations( myQSettings.value( "/Raster/defaultStandardDeviation", 2.0 ).toInt() );
    }
  }
  // Debug
  //mDrawingStyle = SingleBandPseudoColor;

  // Store timestamp
  // TODO move to provider
  mLastModified = lastModified( mDataSource );

  mValidNoDataValue = mDataProvider->isNoDataValueValid();
  if ( mValidNoDataValue )
  {
    mRasterTransparency.initializeTransparentPixelList( mNoDataValue, mNoDataValue, mNoDataValue );
    mRasterTransparency.initializeTransparentPixelList( mNoDataValue );
  }

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
  delete mRasterShader;
  mRasterShader = 0;
  delete mDataProvider;
  mDataProvider = 0;

  mRasterStatsList.clear();
  mContrastEnhancementList.clear();

  mHasPyramids = false;
  mPyramidList.clear();
}

void QgsRasterLayer::setColorShadingAlgorithm( ColorShadingAlgorithm theShadingAlgorithm )
{
  QgsDebugMsg( "called with [" + QString::number( theShadingAlgorithm ) + "]" );
  if ( mColorShadingAlgorithm != theShadingAlgorithm )
  {
    if ( !mRasterShader )
    {
      mRasterShader = new QgsRasterShader();
    }

    switch ( theShadingAlgorithm )
    {
      case PseudoColorShader:
        mRasterShader->setRasterShaderFunction( new QgsPseudoColorShader() );
        break;
      case FreakOutShader:
        mRasterShader->setRasterShaderFunction( new QgsFreakOutShader() );
        break;
      case ColorRampShader:
        mRasterShader->setRasterShaderFunction( new QgsColorRampShader() );
        break;
      case UserDefinedShader:
        //do nothing
        break;
      default:
        mRasterShader->setRasterShaderFunction( new QgsRasterShaderFunction() );
        break;
    }

    //Set the class variable after the call to setRasterShader(), so memory recovery can happen
    mColorShadingAlgorithm = theShadingAlgorithm;
  }
  QgsDebugMsg( "mColorShadingAlgorithm = " + QString::number( theShadingAlgorithm ) );
}

void QgsRasterLayer::setColorShadingAlgorithm( QString theShaderAlgorithm )
{
  QgsDebugMsg( "called with [" + theShaderAlgorithm + "]" );

  if ( theShaderAlgorithm == "PseudoColorShader" )
    setColorShadingAlgorithm( PseudoColorShader );
  else if ( theShaderAlgorithm == "FreakOutShader" )
    setColorShadingAlgorithm( FreakOutShader );
  else if ( theShaderAlgorithm == "ColorRampShader" )
    setColorShadingAlgorithm( ColorRampShader );
  else if ( theShaderAlgorithm == "UserDefinedShader" )
    setColorShadingAlgorithm( UserDefinedShader );
  else
    setColorShadingAlgorithm( UndefinedShader );
}

void QgsRasterLayer::setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm theAlgorithm, bool theGenerateLookupTableFlag )
{
  QList<QgsContrastEnhancement>::iterator myIterator = mContrastEnhancementList.begin();
  while ( myIterator !=  mContrastEnhancementList.end() )
  {
    ( *myIterator ).setContrastEnhancementAlgorithm( theAlgorithm, theGenerateLookupTableFlag );
    ++myIterator;
  }
  mContrastEnhancementAlgorithm = theAlgorithm;
}

void QgsRasterLayer::setContrastEnhancementAlgorithm( QString theAlgorithm, bool theGenerateLookupTableFlag )
{
  QgsDebugMsg( "called with [" + theAlgorithm + "] and flag=" + QString::number(( int )theGenerateLookupTableFlag ) );

  if ( theAlgorithm == "NoEnhancement" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::NoEnhancement, theGenerateLookupTableFlag );
  }
  else if ( theAlgorithm == "StretchToMinimumMaximum" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::StretchToMinimumMaximum, theGenerateLookupTableFlag );
  }
  else if ( theAlgorithm == "StretchAndClipToMinimumMaximum" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::StretchAndClipToMinimumMaximum, theGenerateLookupTableFlag );
  }
  else if ( theAlgorithm == "ClipToMinimumMaximum" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::ClipToMinimumMaximum, theGenerateLookupTableFlag );
  }
  else if ( theAlgorithm == "UserDefined" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::UserDefinedEnhancement, theGenerateLookupTableFlag );
  }
  else
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::NoEnhancement, theGenerateLookupTableFlag );
  }
}

void QgsRasterLayer::setContrastEnhancementFunction( QgsContrastEnhancementFunction* theFunction )
{
  if ( theFunction )
  {
    QList<QgsContrastEnhancement>::iterator myIterator = mContrastEnhancementList.begin();
    while ( myIterator !=  mContrastEnhancementList.end() )
    {
      ( *myIterator ).setContrastEnhancementFunction( theFunction );
      ++myIterator;
    }
  }
}

/**
 *
 * Implemented mainly for serialisation / deserialisation of settings to xml.
 * \note May be deprecated in the future! Use setDrawingStyle( DrawingStyle ) instead.
 */
void QgsRasterLayer::setDrawingStyle( QString const & theDrawingStyleQString )
{
  QgsDebugMsg( "DrawingStyle = " + theDrawingStyleQString );
  if ( theDrawingStyleQString == "SingleBandGray" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = SingleBandGray;
  }
  else if ( theDrawingStyleQString == "SingleBandPseudoColor" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = SingleBandPseudoColor;
  }
  else if ( theDrawingStyleQString == "PalettedColor" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = PalettedColor;
  }
  else if ( theDrawingStyleQString == "PalettedSingleBandGray" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = PalettedSingleBandGray;
  }
  else if ( theDrawingStyleQString == "PalettedSingleBandPseudoColor" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = PalettedSingleBandPseudoColor;
  }
  else if ( theDrawingStyleQString == "PalettedMultiBandColor" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = PalettedMultiBandColor;
  }
  else if ( theDrawingStyleQString == "MultiBandSingleBandGray" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = MultiBandSingleBandGray;
  }
  else if ( theDrawingStyleQString == "MultiBandSingleBandPseudoColor" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = MultiBandSingleBandPseudoColor;
  }
  else if ( theDrawingStyleQString == "MultiBandColor" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = MultiBandColor;
  }
  else if ( theDrawingStyleQString == "SingleBandColorDataStyle" )//no need to tr() this its not shown in ui
  {
    QgsDebugMsg( "Setting mDrawingStyle to SingleBandColorDataStyle " + QString::number( SingleBandColorDataStyle ) );
    mDrawingStyle = SingleBandColorDataStyle;
    QgsDebugMsg( "Setted mDrawingStyle to " + QString::number( mDrawingStyle ) );
  }
  else
  {
    mDrawingStyle = UndefinedDrawingStyle;
  }
}

void QgsRasterLayer::setGrayBandName( QString const & theBandName )
{
  mGrayBandName = validateBandName( theBandName );
}

void QgsRasterLayer::setGreenBandName( QString const & theBandName )
{
  mGreenBandName = validateBandName( theBandName );
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

void QgsRasterLayer::setMaximumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag )
{
  QgsDebugMsg( "setMaximumValue theValue = " + QString::number( theValue ) );
  if ( 0 < theBand && theBand <= bandCount() )
  {
    mContrastEnhancementList[theBand - 1].setMaximumValue( theValue, theGenerateLookupTableFlag );
  }
}

void QgsRasterLayer::setMaximumValue( QString theBand, double theValue, bool theGenerateLookupTableFlag )
{
  if ( theBand != tr( "Not Set" ) )
  {
    setMaximumValue( bandNumber( theBand ), theValue, theGenerateLookupTableFlag );
  }
}

void QgsRasterLayer::setMinimumMaximumUsingLastExtent()
{
  double myMinMax[2];
  if ( rasterType() == QgsRasterLayer::GrayOrUndefined || drawingStyle() == QgsRasterLayer::SingleBandGray || drawingStyle() == QgsRasterLayer::MultiBandSingleBandGray )
  {
    computeMinimumMaximumFromLastExtent( grayBandName(), myMinMax );
    setMinimumValue( grayBandName(), myMinMax[0] );
    setMaximumValue( grayBandName(), myMinMax[1] );

    setUserDefinedGrayMinimumMaximum( true );
  }
  else if ( rasterType() == QgsRasterLayer::Multiband )
  {
    computeMinimumMaximumFromLastExtent( redBandName(), myMinMax );
    setMinimumValue( redBandName(), myMinMax[0], false );
    setMaximumValue( redBandName(), myMinMax[1], false );

    computeMinimumMaximumFromLastExtent( greenBandName(), myMinMax );
    setMinimumValue( greenBandName(), myMinMax[0], false );
    setMaximumValue( greenBandName(), myMinMax[1], false );

    computeMinimumMaximumFromLastExtent( blueBandName(), myMinMax );
    setMinimumValue( blueBandName(), myMinMax[0], false );
    setMaximumValue( blueBandName(), myMinMax[1], false );

    setUserDefinedRGBMinimumMaximum( true );
  }
}

void QgsRasterLayer::setMinimumMaximumUsingDataset()
{
  if ( rasterType() == QgsRasterLayer::GrayOrUndefined || drawingStyle() == QgsRasterLayer::SingleBandGray || drawingStyle() == QgsRasterLayer::MultiBandSingleBandGray )
  {
    QgsRasterBandStats myRasterBandStats = bandStatistics( bandNumber( mGrayBandName ) );
    float myMin = myRasterBandStats.minimumValue;
    float myMax = myRasterBandStats.maximumValue;
    setMinimumValue( grayBandName(), myMin );
    setMaximumValue( grayBandName(), myMax );
    setUserDefinedGrayMinimumMaximum( false );
  }
  else if ( rasterType() == QgsRasterLayer::Multiband )
  {
    QgsRasterBandStats myRasterBandStats = bandStatistics( bandNumber( mRedBandName ) );
    float myMin = myRasterBandStats.minimumValue;
    float myMax = myRasterBandStats.maximumValue;
    setMinimumValue( redBandName(), myMin );
    setMaximumValue( redBandName(), myMax );

    myRasterBandStats = bandStatistics( bandNumber( mGreenBandName ) );
    myMin = myRasterBandStats.minimumValue;
    myMax = myRasterBandStats.maximumValue;
    setMinimumValue( greenBandName(), myMin );
    setMaximumValue( greenBandName(), myMax );

    myRasterBandStats = bandStatistics( bandNumber( mGreenBandName ) );
    myMin = myRasterBandStats.minimumValue;
    myMax = myRasterBandStats.maximumValue;
    setMinimumValue( greenBandName(), myMin );
    setMaximumValue( greenBandName(), myMax );

    setUserDefinedRGBMinimumMaximum( false );
  }
}

void QgsRasterLayer::setMinimumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag )
{
  QgsDebugMsg( "setMinimumValue theValue = " + QString::number( theValue ) );
  if ( 0 < theBand && theBand <= bandCount() )
  {
    mContrastEnhancementList[theBand - 1].setMinimumValue( theValue, theGenerateLookupTableFlag );
  }
}

void QgsRasterLayer::setMinimumValue( QString theBand, double theValue, bool theGenerateLookupTableFlag )
{
  if ( theBand != tr( "Not Set" ) )
  {
    setMinimumValue( bandNumber( theBand ), theValue, theGenerateLookupTableFlag );
  }

}

void QgsRasterLayer::setNoDataValue( double theNoDataValue )
{
  if ( theNoDataValue != mNoDataValue )
  {
    mNoDataValue = theNoDataValue;
    mValidNoDataValue = true;
    //Basically set the raster stats as invalid
    QList<QgsRasterBandStats>::iterator myIterator = mRasterStatsList.begin();
    while ( myIterator !=  mRasterStatsList.end() )
    {
      ( *myIterator ).statsGathered = false;
      ++myIterator;
    }
  }
}

void QgsRasterLayer::setRasterShaderFunction( QgsRasterShaderFunction* theFunction )
{
  if ( theFunction )
  {
    mRasterShader->setRasterShaderFunction( theFunction );
    mColorShadingAlgorithm = QgsRasterLayer::UserDefinedShader;
  }
  else
  {
    //If NULL as passed in, set a default shader function to prevent segfaults
    mRasterShader->setRasterShaderFunction( new QgsRasterShaderFunction() );
    mColorShadingAlgorithm = QgsRasterLayer::UndefinedShader;
  }
}

void QgsRasterLayer::setRedBandName( QString const & theBandName )
{
  QgsDebugMsg( "setRedBandName :  " + theBandName );
  mRedBandName = validateBandName( theBandName );
}

void QgsRasterLayer::setSubLayerVisibility( QString const & name, bool vis )
{

  if ( mDataProvider )
  {
    QgsDebugMsg( "About to mDataProvider->setSubLayerVisibility(name, vis)." );
    mDataProvider->setSubLayerVisibility( name, vis );
  }

}

void QgsRasterLayer::setTransparentBandName( QString const & theBandName )
{
  mTransparencyBandName = validateBandName( theBandName );
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


void QgsRasterLayer::thumbnailAsPixmap( QPixmap * theQPixmap )
{
  //TODO: This should be depreciated and a new function written that just returns a new QPixmap, it will be safer
  if ( !theQPixmap )
    return;

  theQPixmap->fill( );  //defaults to white

  QgsRasterViewPort *myRasterViewPort = new QgsRasterViewPort();

  double myMapUnitsPerPixel;
  double myX = 0.0;
  double myY = 0.0;
  QgsRectangle myExtent = mDataProvider->extent();
  if ( myExtent.width() / myExtent.height() >=  theQPixmap->width() / theQPixmap->height() )
  {
    myMapUnitsPerPixel = myExtent.width() / theQPixmap->width();
    myY = ( theQPixmap->height() - myExtent.height() / myMapUnitsPerPixel ) / 2;
  }
  else
  {
    myMapUnitsPerPixel = myExtent.height() / theQPixmap->height();
    myX = ( theQPixmap->width() - myExtent.width() / myMapUnitsPerPixel ) / 2;
  }

  double myPixelWidth = myExtent.width() / myMapUnitsPerPixel;
  double myPixelHeight = myExtent.height() / myMapUnitsPerPixel;

  //myRasterViewPort->topLeftPoint = QgsPoint( 0, 0 );
  myRasterViewPort->topLeftPoint = QgsPoint( myX, myY );

  //myRasterViewPort->bottomRightPoint = QgsPoint( theQPixmap->width(), theQPixmap->height() );

  myRasterViewPort->bottomRightPoint = QgsPoint( myPixelWidth, myPixelHeight );
  myRasterViewPort->drawableAreaXDim = theQPixmap->width();
  myRasterViewPort->drawableAreaYDim = theQPixmap->height();
  //myRasterViewPort->drawableAreaXDim = myPixelWidth;
  //myRasterViewPort->drawableAreaYDim = myPixelHeight;

  myRasterViewPort->mDrawnExtent = myExtent;
  myRasterViewPort->mSrcCRS = QgsCoordinateReferenceSystem(); // will be invalid
  myRasterViewPort->mDestCRS = QgsCoordinateReferenceSystem(); // will be invalid

  QgsMapToPixel *myMapToPixel = new QgsMapToPixel( myMapUnitsPerPixel );

  QPainter * myQPainter = new QPainter( theQPixmap );
  draw( myQPainter, myRasterViewPort, myMapToPixel );
  delete myRasterViewPort;
  delete myMapToPixel;
  myQPainter->end();
  delete myQPainter;
}

void QgsRasterLayer::thumbnailAsImage( QImage * thepImage )
{
  //TODO: This should be depreciated and a new function written that just returns a new QImage, it will be safer
  if ( !thepImage )
    return;


  thepImage->fill( Qt::white ); //defaults to white

  // Raster providers are disabled (for the moment)
  if ( mProviderKey.isEmpty() )
  {
    QgsRasterViewPort *myRasterViewPort = new QgsRasterViewPort();
    myRasterViewPort->topLeftPoint = QgsPoint( 0, 0 );
    myRasterViewPort->bottomRightPoint = QgsPoint( thepImage->width(), thepImage->height() );
    myRasterViewPort->drawableAreaXDim = thepImage->width();
    myRasterViewPort->drawableAreaYDim = thepImage->height();

    QPainter * myQPainter = new QPainter( thepImage );
    draw( myQPainter, myRasterViewPort );
    delete myRasterViewPort;
    myQPainter->end();
    delete myQPainter;
  }

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
  QDomNode mnl = layer_node.namedItem( "rasterproperties" );
  QDomNode snode = mnl.namedItem( "mDrawingStyle" );
  QDomElement myElement = snode.toElement();
  setDrawingStyle( myElement.text() );

  snode = mnl.namedItem( "mColorShadingAlgorithm" );
  myElement = snode.toElement();
  setColorShadingAlgorithm( myElement.text() );

  snode = mnl.namedItem( "mInvertColor" );
  myElement = snode.toElement();
  QVariant myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setInvertHistogram( myVariant.toBool() );

  snode = mnl.namedItem( "mRedBandName" );
  myElement = snode.toElement();
  setRedBandName( myElement.text() );

  snode = mnl.namedItem( "mGreenBandName" );
  myElement = snode.toElement();
  setGreenBandName( myElement.text() );

  snode = mnl.namedItem( "mBlueBandName" );
  myElement = snode.toElement();
  setBlueBandName( myElement.text() );

  snode = mnl.namedItem( "mGrayBandName" );
  myElement = snode.toElement();
  QgsDebugMsg( QString( " Setting gray band to : " ) + myElement.text() );
  setGrayBandName( myElement.text() );

  snode = mnl.namedItem( "mStandardDeviations" );
  myElement = snode.toElement();
  setStandardDeviations( myElement.text().toDouble() );

  snode = mnl.namedItem( "mUserDefinedRGBMinimumMaximum" );
  myElement = snode.toElement();
  myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setUserDefinedRGBMinimumMaximum( myVariant.toBool() );

  snode = mnl.namedItem( "mRGBMinimumMaximumEstimated" );
  myElement = snode.toElement();
  myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setRGBMinimumMaximumEstimated( myVariant.toBool() );

  snode = mnl.namedItem( "mUserDefinedGrayMinimumMaximum" );
  myElement = snode.toElement();
  myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setUserDefinedGrayMinimumMaximum( myVariant.toBool() );

  snode = mnl.namedItem( "mGrayMinimumMaximumEstimated" );
  myElement = snode.toElement();
  myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setGrayMinimumMaximumEstimated( myVariant.toBool() );

  snode = mnl.namedItem( "mContrastEnhancementAlgorithm" );
  myElement = snode.toElement();
  setContrastEnhancementAlgorithm( myElement.text(), false );

  QDomNode contrastEnhancementMinMaxValues = mnl.namedItem( "contrastEnhancementMinMaxValues" );
  QDomNodeList minMaxValueList = contrastEnhancementMinMaxValues.toElement().elementsByTagName( "minMaxEntry" );
  for ( int i = 0; i < minMaxValueList.size(); ++i )
  {
    QDomNode minMaxEntry = minMaxValueList.at( i ).toElement();
    if ( minMaxEntry.isNull() )
    {
      continue;
    }
    QDomNode minEntry = minMaxEntry.namedItem( "min" );
    QDomNode maxEntry = minMaxEntry.namedItem( "max" );

    setMinimumValue( i + 1, minEntry.toElement().text().toDouble(), false );
    setMaximumValue( i + 1, maxEntry.toElement().text().toDouble(), false );
  }

  QgsDebugMsg( "ReadXml: gray band name " + mGrayBandName );
  QgsDebugMsg( "ReadXml: red band name " + mRedBandName );
  QgsDebugMsg( "ReadXml: green band name  " + mGreenBandName );
  QgsDebugMsg( "Drawing style " + drawingStyleAsString() );

  /*
   * Transparency tab
   */
  snode = mnl.namedItem( "mNoDataValue" );
  myElement = snode.toElement();
  QgsDebugMsg( "ReadXml: mNoDataValue = " + myElement.text() );
  setNoDataValue( myElement.text().toDouble() );
  QgsDebugMsg( "ReadXml: mNoDataValue = " + QString::number( mNoDataValue ) );
  if ( myElement.attribute( "mValidNoDataValue", "false" ).compare( "true" ) )
  {
    // If flag element is not true, set to false.
    mValidNoDataValue = false;
  }

  QDomNode singleValuePixelListNode = mnl.namedItem( "singleValuePixelList" );
  if ( !singleValuePixelListNode.isNull() )
  {
    QList<QgsRasterTransparency::TransparentSingleValuePixel> newSingleValuePixelList;

    //entries
    QDomNodeList singleValuePixelList = singleValuePixelListNode.toElement().elementsByTagName( "pixelListEntry" );
    for ( int i = 0; i < singleValuePixelList.size(); ++i )
    {
      QgsRasterTransparency::TransparentSingleValuePixel myNewItem;
      QDomElement singleValuePixelListElement = singleValuePixelList.at( i ).toElement();
      if ( singleValuePixelListElement.isNull() )
      {
        continue;
      }

      myNewItem.pixelValue = singleValuePixelListElement.attribute( "pixelValue" ).toDouble();
      myNewItem.percentTransparent = singleValuePixelListElement.attribute( "percentTransparent" ).toDouble();

      newSingleValuePixelList.push_back( myNewItem );
    }
    mRasterTransparency.setTransparentSingleValuePixelList( newSingleValuePixelList );
  }

  QDomNode threeValuePixelListNode = mnl.namedItem( "threeValuePixelList" );
  if ( !threeValuePixelListNode.isNull() )
  {
    QList<QgsRasterTransparency::TransparentThreeValuePixel> newThreeValuePixelList;

    //entries
    QDomNodeList threeValuePixelList = threeValuePixelListNode.toElement().elementsByTagName( "pixelListEntry" );
    for ( int i = 0; i < threeValuePixelList.size(); ++i )
    {
      QgsRasterTransparency::TransparentThreeValuePixel myNewItem;
      QDomElement threeValuePixelListElement = threeValuePixelList.at( i ).toElement();
      if ( threeValuePixelListElement.isNull() )
      {
        continue;
      }

      myNewItem.red = threeValuePixelListElement.attribute( "red" ).toDouble();
      myNewItem.green = threeValuePixelListElement.attribute( "green" ).toDouble();
      myNewItem.blue = threeValuePixelListElement.attribute( "blue" ).toDouble();
      myNewItem.percentTransparent = threeValuePixelListElement.attribute( "percentTransparent" ).toDouble();

      newThreeValuePixelList.push_back( myNewItem );
    }
    mRasterTransparency.setTransparentThreeValuePixelList( newThreeValuePixelList );
  }

  /*
   * Color Ramp tab
   */
  //restore custom color ramp settings
  QDomNode customColorRampNode = mnl.namedItem( "customColorRamp" );
  if ( !customColorRampNode.isNull() )
  {
    QgsColorRampShader* myColorRampShader = ( QgsColorRampShader* ) mRasterShader->rasterShaderFunction();

    //TODO: Remove the customColorRampType check and following if() in v2.0, added for compatibility with older ( bugged ) project files
    QDomNode customColorRampTypeNode = customColorRampNode.namedItem( "customColorRampType" );
    QDomNode colorRampTypeNode = customColorRampNode.namedItem( "colorRampType" );
    QString myRampType = "";
    if ( "" == customColorRampTypeNode.toElement().text() )
    {
      myRampType = colorRampTypeNode.toElement().text();
    }
    else
    {
      myRampType = customColorRampTypeNode.toElement().text();
    }
    myColorRampShader->setColorRampType( myRampType );


    //entries
    QList<QgsColorRampShader::ColorRampItem> myColorRampItemList;
    QDomNodeList colorRampEntryList = customColorRampNode.toElement().elementsByTagName( "colorRampEntry" );
    for ( int i = 0; i < colorRampEntryList.size(); ++i )
    {
      QgsColorRampShader::ColorRampItem myNewItem;
      QDomElement colorRampEntryElement = colorRampEntryList.at( i ).toElement();
      if ( colorRampEntryElement.isNull() )
      {
        continue;
      }

      myNewItem.color = QColor( colorRampEntryElement.attribute( "red" ).toInt(), colorRampEntryElement.attribute( "green" ).toInt(), colorRampEntryElement.attribute( "blue" ).toInt() );
      myNewItem.label = colorRampEntryElement.attribute( "label" );
      myNewItem.value = colorRampEntryElement.attribute( "value" ).toDouble();

      myColorRampItemList.push_back( myNewItem );
    }
    myColorRampShader->setColorRampItemList( myColorRampItemList );
  }
  return true;
} //readSymbology

/**

  Raster layer project file XML of form:

  \verbatim
  <maplayer type="raster" visible="1" showInOverviewFlag="1">
  <layername>Wynoochee_dem</layername>
  <datasource>/home/mcoletti/mnt/MCOLETTIF8F9/c/Toolkit_Course/Answers/Training_Data/wynoochee_dem.img</datasource>
  <zorder>0</zorder>
  <transparencyLevelInt>255</transparencyLevelInt>
  <rasterproperties>
  <mDrawingStyle>SingleBandGray</mDrawingStyle>
  <mInvertColor boolean="false"/>
  <mStandardDeviations>0</mStandardDeviations>
  <mRedBandName>Not Set</mRedBandName>
  <mGreenBandName>Not Set</mGreenBandName>
  <mBlueBandName>Not Set</mBlueBandName>
  <mGrayBandName>Undefined</mGrayBandName>
  </rasterproperties>
  </maplayer>
  \endverbatim

  @note Called by QgsMapLayer::readXML().
*/
bool QgsRasterLayer::readXml( const QDomNode& layer_node )
{
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

  // Collect sublayer names and styles
  mLayers.clear();
  mStyles.clear();

  if ( mProviderKey == "wms" )
  {
    QDomElement layerElement = rpNode.firstChildElement( "wmsSublayer" );
    while ( !layerElement.isNull() )
    {
      // TODO: sublayer visibility - post-0.8 release timeframe

      // collect name for the sublayer
      mLayers += layerElement.namedItem( "name" ).toElement().text();

      // collect style for the sublayer
      mStyles += layerElement.namedItem( "style" ).toElement().text();

      layerElement = layerElement.nextSiblingElement( "wmsSublayer" );
    }

    // Collect format
    mFormat = rpNode.namedItem( "wmsFormat" ).toElement().text();
  }

  mCrs = crs().authid();
  // Collect CRS
  setDataProvider( mProviderKey, mLayers, mStyles, mFormat, mCrs );

  QString theError;
  bool res = readSymbology( layer_node, theError );

  // old wms settings we need to correct
  if ( res &&
       mProviderKey == "wms" &&
       mDrawingStyle == MultiBandColor &&
       mRedBandName == TRSTRING_NOT_SET &&
       mGreenBandName == TRSTRING_NOT_SET &&
       mBlueBandName == TRSTRING_NOT_SET )
  {
    mDrawingStyle = SingleBandColorDataStyle;
    mGrayBandName = bandName( 1 );
  }

  // Check timestamp
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
      setDataProvider( mProviderKey, mLayers, mStyles, mFormat, mCrs );
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
  // <rasterproperties>
  QDomElement rasterPropertiesElement = document.createElement( "rasterproperties" );
  layer_node.appendChild( rasterPropertiesElement );

  QStringList sl = subLayers();
  QStringList sls = mDataProvider->subLayerStyles();

  QStringList::const_iterator layerStyle = sls.begin();

  if ( mProviderKey == "wms" )
  {
    // <rasterproperties><wmsSublayer>
    for ( QStringList::const_iterator layerName  = sl.begin();
          layerName != sl.end();
          ++layerName )
    {

      QgsDebugMsg( QString( "<rasterproperties><wmsSublayer> %1" ).arg( layerName->toLocal8Bit().data() ) );

      QDomElement sublayerElement = document.createElement( "wmsSublayer" );

      // TODO: sublayer visibility - post-0.8 release timeframe

      // <rasterproperties><wmsSublayer><name>
      QDomElement sublayerNameElement = document.createElement( "name" );
      QDomText sublayerNameText = document.createTextNode( *layerName );
      sublayerNameElement.appendChild( sublayerNameText );
      sublayerElement.appendChild( sublayerNameElement );

      // <rasterproperties><wmsSublayer><style>
      QDomElement sublayerStyleElement = document.createElement( "style" );
      QDomText sublayerStyleText = document.createTextNode( *layerStyle );
      sublayerStyleElement.appendChild( sublayerStyleText );
      sublayerElement.appendChild( sublayerStyleElement );

      rasterPropertiesElement.appendChild( sublayerElement );

      // This assumes there are exactly the same number of "layerName"s as there are "layerStyle"s
      ++layerStyle;
    }

    // <rasterproperties><wmsFormat>
    QDomElement formatElement = document.createElement( "wmsFormat" );
    QDomText formatText =
      document.createTextNode( mDataProvider->imageEncoding() );
    formatElement.appendChild( formatText );
    rasterPropertiesElement.appendChild( formatElement );
  }

  // <mDrawingStyle>
  QDomElement drawStyleElement = document.createElement( "mDrawingStyle" );
  QDomText    drawStyleText    = document.createTextNode( drawingStyleAsString() );

  drawStyleElement.appendChild( drawStyleText );

  rasterPropertiesElement.appendChild( drawStyleElement );

  // <colorShadingAlgorithm>
  QDomElement colorShadingAlgorithmElement = document.createElement( "mColorShadingAlgorithm" );
  QDomText    colorShadingAlgorithmText    = document.createTextNode( colorShadingAlgorithmAsString() );

  colorShadingAlgorithmElement.appendChild( colorShadingAlgorithmText );

  rasterPropertiesElement.appendChild( colorShadingAlgorithmElement );

  // <mInvertColor>
  QDomElement mInvertColorElement = document.createElement( "mInvertColor" );

  if ( invertHistogram() )
  {
    mInvertColorElement.setAttribute( "boolean", "true" );
  }
  else
  {
    mInvertColorElement.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( mInvertColorElement );

  // <mRedBandName>
  QDomElement mRedBandNameElement = document.createElement( "mRedBandName" );
  QString writtenRedBandName =  redBandName();
  if ( writtenRedBandName == TRSTRING_NOT_SET )
  {
    // Write english "not set" only.
    writtenRedBandName = QSTRING_NOT_SET;
  }
  QDomText    mRedBandNameText    = document.createTextNode( writtenRedBandName );

  mRedBandNameElement.appendChild( mRedBandNameText );

  rasterPropertiesElement.appendChild( mRedBandNameElement );


  // <mGreenBandName>
  QDomElement mGreenBandNameElement = document.createElement( "mGreenBandName" );
  QString writtenGreenBandName =  greenBandName();
  if ( writtenGreenBandName == TRSTRING_NOT_SET )
  {
    // Write english "not set" only.
    writtenGreenBandName = QSTRING_NOT_SET;
  }
  QDomText    mGreenBandNameText    = document.createTextNode( writtenGreenBandName );

  mGreenBandNameElement.appendChild( mGreenBandNameText );

  rasterPropertiesElement.appendChild( mGreenBandNameElement );

  // <mBlueBandName>
  QDomElement mBlueBandNameElement = document.createElement( "mBlueBandName" );
  QString writtenBlueBandName =  blueBandName();
  if ( writtenBlueBandName == TRSTRING_NOT_SET )
  {
    // Write english "not set" only.
    writtenBlueBandName = QSTRING_NOT_SET;
  }
  QDomText    mBlueBandNameText    = document.createTextNode( writtenBlueBandName );

  mBlueBandNameElement.appendChild( mBlueBandNameText );

  rasterPropertiesElement.appendChild( mBlueBandNameElement );

  // <mGrayBandName>
  QDomElement mGrayBandNameElement = document.createElement( "mGrayBandName" );
  QString writtenGrayBandName =  grayBandName();
  if ( writtenGrayBandName == TRSTRING_NOT_SET )
  {
    // Write english "not set" only.
    writtenGrayBandName = QSTRING_NOT_SET;
  }
  QDomText    mGrayBandNameText    = document.createTextNode( writtenGrayBandName );

  mGrayBandNameElement.appendChild( mGrayBandNameText );
  rasterPropertiesElement.appendChild( mGrayBandNameElement );

  // <mStandardDeviations>
  QDomElement mStandardDeviationsElement = document.createElement( "mStandardDeviations" );
  QDomText    mStandardDeviationsText    = document.createTextNode( QString::number( standardDeviations() ) );

  mStandardDeviationsElement.appendChild( mStandardDeviationsText );

  rasterPropertiesElement.appendChild( mStandardDeviationsElement );

  // <mUserDefinedRGBMinimumMaximum>
  QDomElement userDefinedRGBMinMaxFlag = document.createElement( "mUserDefinedRGBMinimumMaximum" );

  if ( hasUserDefinedRGBMinimumMaximum() )
  {
    userDefinedRGBMinMaxFlag.setAttribute( "boolean", "true" );
  }
  else
  {
    userDefinedRGBMinMaxFlag.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( userDefinedRGBMinMaxFlag );

  // <mRGBMinimumMaximumEstimated>
  QDomElement RGBMinimumMaximumEstimated = document.createElement( "mRGBMinimumMaximumEstimated" );

  if ( isRGBMinimumMaximumEstimated() )
  {
    RGBMinimumMaximumEstimated.setAttribute( "boolean", "true" );
  }
  else
  {
    RGBMinimumMaximumEstimated.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( RGBMinimumMaximumEstimated );

  // <mUserDefinedGrayMinimumMaximum>
  QDomElement userDefinedGrayMinMaxFlag = document.createElement( "mUserDefinedGrayMinimumMaximum" );

  if ( hasUserDefinedGrayMinimumMaximum() )
  {
    userDefinedGrayMinMaxFlag.setAttribute( "boolean", "true" );
  }
  else
  {
    userDefinedGrayMinMaxFlag.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( userDefinedGrayMinMaxFlag );

  // <mGrayMinimumMaximumEstimated>
  QDomElement GrayMinimumMaximumEstimated = document.createElement( "mGrayMinimumMaximumEstimated" );

  if ( isGrayMinimumMaximumEstimated() )
  {
    GrayMinimumMaximumEstimated.setAttribute( "boolean", "true" );
  }
  else
  {
    GrayMinimumMaximumEstimated.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( GrayMinimumMaximumEstimated );

  // <contrastEnhancementAlgorithm>
  QDomElement contrastEnhancementAlgorithmElement = document.createElement( "mContrastEnhancementAlgorithm" );
  QDomText    contrastEnhancementAlgorithmText    = document.createTextNode( contrastEnhancementAlgorithmAsString() );

  contrastEnhancementAlgorithmElement.appendChild( contrastEnhancementAlgorithmText );

  rasterPropertiesElement.appendChild( contrastEnhancementAlgorithmElement );

  // <minMaxValues>
  QList<QgsContrastEnhancement>::const_iterator it;
  QDomElement contrastEnhancementMinMaxValuesElement = document.createElement( "contrastEnhancementMinMaxValues" );
  for ( it =  mContrastEnhancementList.constBegin(); it != mContrastEnhancementList.constEnd(); ++it )
  {
    QDomElement minMaxEntry = document.createElement( "minMaxEntry" );
    QDomElement minEntry = document.createElement( "min" );
    QDomElement maxEntry = document.createElement( "max" );

    QDomText minEntryText = document.createTextNode( QString::number( it->minimumValue() ) );
    minEntry.appendChild( minEntryText );

    QDomText maxEntryText = document.createTextNode( QString::number( it->maximumValue() ) );
    maxEntry.appendChild( maxEntryText );

    minMaxEntry.appendChild( minEntry );
    minMaxEntry.appendChild( maxEntry );

    contrastEnhancementMinMaxValuesElement.appendChild( minMaxEntry );
  }

  rasterPropertiesElement.appendChild( contrastEnhancementMinMaxValuesElement );

  /*
   * Transparency tab
   */
  // <mNodataValue>
  QDomElement mNoDataValueElement = document.createElement( "mNoDataValue" );
  QDomText    mNoDataValueText    = document.createTextNode( QString::number( mNoDataValue, 'f' ) );
  if ( mValidNoDataValue )
  {
    mNoDataValueElement.setAttribute( "mValidNoDataValue", "true" );
  }
  else
  {
    mNoDataValueElement.setAttribute( "mValidNoDataValue", "false" );
  }

  mNoDataValueElement.appendChild( mNoDataValueText );

  rasterPropertiesElement.appendChild( mNoDataValueElement );


  if ( mRasterTransparency.transparentSingleValuePixelList().count() > 0 )
  {
    QDomElement singleValuePixelListElement = document.createElement( "singleValuePixelList" );

    QList<QgsRasterTransparency::TransparentSingleValuePixel> myPixelList = mRasterTransparency.transparentSingleValuePixelList();
    QList<QgsRasterTransparency::TransparentSingleValuePixel>::iterator it;
    for ( it =  myPixelList.begin(); it != myPixelList.end(); ++it )
    {
      QDomElement pixelListElement = document.createElement( "pixelListEntry" );
      pixelListElement.setAttribute( "pixelValue", QString::number( it->pixelValue, 'f' ) );
      pixelListElement.setAttribute( "percentTransparent", QString::number( it->percentTransparent ) );

      singleValuePixelListElement.appendChild( pixelListElement );
    }

    rasterPropertiesElement.appendChild( singleValuePixelListElement );
  }

  if ( mRasterTransparency.transparentThreeValuePixelList().count() > 0 )
  {
    QDomElement threeValuePixelListElement = document.createElement( "threeValuePixelList" );

    QList<QgsRasterTransparency::TransparentThreeValuePixel> myPixelList = mRasterTransparency.transparentThreeValuePixelList();
    QList<QgsRasterTransparency::TransparentThreeValuePixel>::iterator it;
    for ( it =  myPixelList.begin(); it != myPixelList.end(); ++it )
    {
      QDomElement pixelListElement = document.createElement( "pixelListEntry" );
      pixelListElement.setAttribute( "red", QString::number( it->red, 'f' ) );
      pixelListElement.setAttribute( "green", QString::number( it->green, 'f' ) );
      pixelListElement.setAttribute( "blue", QString::number( it->blue, 'f' ) );
      pixelListElement.setAttribute( "percentTransparent", QString::number( it->percentTransparent ) );

      threeValuePixelListElement.appendChild( pixelListElement );
    }

    rasterPropertiesElement.appendChild( threeValuePixelListElement );
  }

  /*
   * Color Ramp tab
   */
  if ( QgsRasterLayer::ColorRampShader ==  colorShadingAlgorithm() )
  {
    QDomElement customColorRampElement = document.createElement( "customColorRamp" );

    QDomElement customColorRampType = document.createElement( "colorRampType" );
    QDomText customColorRampTypeText = document.createTextNode((( QgsColorRampShader* )mRasterShader->rasterShaderFunction() )->colorRampTypeAsQString() );
    customColorRampType.appendChild( customColorRampTypeText );
    customColorRampElement.appendChild( customColorRampType );

    QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = (( QgsColorRampShader* )mRasterShader->rasterShaderFunction() )->colorRampItemList();
    QList<QgsColorRampShader::ColorRampItem>::iterator it;
    for ( it =  myColorRampItemList.begin(); it != myColorRampItemList.end(); ++it )
    {
      QDomElement colorRampEntryElement = document.createElement( "colorRampEntry" );
      colorRampEntryElement.setAttribute( "red", QString::number( it->color.red() ) );
      colorRampEntryElement.setAttribute( "green", QString::number( it->color.green() ) );
      colorRampEntryElement.setAttribute( "blue", QString::number( it->color.blue() ) );
      colorRampEntryElement.setAttribute( "value", QString::number( it->value, 'f' ) );
      colorRampEntryElement.setAttribute( "label", it->label );

      customColorRampElement.appendChild( colorRampEntryElement );
    }

    rasterPropertiesElement.appendChild( customColorRampElement );
  }

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

  //write out the symbology
  QString errorMsg;
  return writeSymbology( layer_node, document, errorMsg );
}

//////////////////////////////////////////////////////////
//
// Private methods
//
/////////////////////////////////////////////////////////
void QgsRasterLayer::drawSingleBandColorData( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  QgsDebugMsg( "entered." );

  QgsRasterImageBuffer imageBuffer( mDataProvider, theBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine ) )
  {
    if ( mTransparencyLevel == 255 )
    {
      int size = theRasterViewPort->drawableAreaXDim * 4;
      memcpy( imageScanLine, rasterScanLine, size );
    }
    else
    {
      uint *p = ( uint* ) rasterScanLine;
      for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
      {
        QRgb c( *p++ );

        imageScanLine[ i ] = qRgba( qRed( c ), qGreen( c ), qBlue( c ), qAlpha( c )  * mTransparencyLevel  / 255 );
      }
    }
  }
}

void QgsRasterLayer::drawMultiBandColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel )
{
  QgsDebugMsg( "entered." );
  int myRedBandNo = bandNumber( mRedBandName );
  //Invalid band number, segfault prevention
  if ( 0 >= myRedBandNo )
  {
    return;
  }

  int myGreenBandNo = bandNumber( mGreenBandName );
  //Invalid band number, segfault prevention
  if ( 0 >= myGreenBandNo )
  {
    return;
  }

  int myBlueBandNo = bandNumber( mBlueBandName );
  //Invalid band number, segfault prevention
  if ( 0 >= myBlueBandNo )
  {
    return;
  }

  int myTransparencyBandNo = bandNumber( mTransparencyBandName );
  bool hasTransparencyBand = 0 < myTransparencyBandNo;

  int myRedType = mDataProvider->dataType( myRedBandNo );
  int myGreenType = mDataProvider->dataType( myGreenBandNo );
  int myBlueType = mDataProvider->dataType( myBlueBandNo );
  int myTransparencyType = hasTransparencyBand ? mDataProvider->dataType( myTransparencyBandNo ) : 0;

  QRgb* redImageScanLine = 0;
  void* redRasterScanLine = 0;
  QRgb* greenImageScanLine = 0;
  void* greenRasterScanLine = 0;
  QRgb* blueImageScanLine = 0;
  void* blueRasterScanLine = 0;
  QRgb* transparencyImageScanLine = 0;
  void* transparencyRasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );

  QgsRasterBandStats myRedBandStats;
  QgsRasterBandStats myGreenBandStats;
  QgsRasterBandStats myBlueBandStats;

  /*
   * If a stetch is requested and there are no user defined Min Max values
   * we need to get these values from the bands themselves.
   *
   */
  if ( QgsContrastEnhancement::NoEnhancement != contrastEnhancementAlgorithm() && !mUserDefinedRGBMinimumMaximum && mStandardDeviations > 0 )
  {
    myRedBandStats = bandStatistics( myRedBandNo );
    myGreenBandStats = bandStatistics( myGreenBandNo );
    myBlueBandStats = bandStatistics( myBlueBandNo );
    mRGBMinimumMaximumEstimated = false;
    setMaximumValue( myRedBandNo, myRedBandStats.mean + ( mStandardDeviations * myRedBandStats.stdDev ) );
    setMinimumValue( myRedBandNo, myRedBandStats.mean - ( mStandardDeviations * myRedBandStats.stdDev ) );
    setMaximumValue( myGreenBandNo, myGreenBandStats.mean + ( mStandardDeviations * myGreenBandStats.stdDev ) );
    setMinimumValue( myGreenBandNo, myGreenBandStats.mean - ( mStandardDeviations * myGreenBandStats.stdDev ) );
    setMaximumValue( myBlueBandNo, myBlueBandStats.mean + ( mStandardDeviations * myBlueBandStats.stdDev ) );
    setMinimumValue( myBlueBandNo, myBlueBandStats.mean - ( mStandardDeviations * myBlueBandStats.stdDev ) );
  }
  else if ( QgsContrastEnhancement::NoEnhancement != contrastEnhancementAlgorithm() && !mUserDefinedRGBMinimumMaximum )
  {
    //This case will be true the first time the image is loaded, so just approimate the min max to keep
    //from calling generate raster band stats
    mRGBMinimumMaximumEstimated = true;

    setMinimumValue( myRedBandNo, mDataProvider->minimumValue( myRedBandNo ) );
    setMaximumValue( myRedBandNo, mDataProvider->maximumValue( myRedBandNo ) );

    setMinimumValue( myGreenBandNo, mDataProvider->minimumValue( myGreenBandNo ) );
    setMaximumValue( myGreenBandNo, mDataProvider->maximumValue( myGreenBandNo ) );

    setMinimumValue( myBlueBandNo, mDataProvider->minimumValue( myBlueBandNo ) );
    setMaximumValue( myBlueBandNo, mDataProvider->maximumValue( myBlueBandNo ) );
  }

  //Read and display pixels
  double myRedValue = 0.0;
  double myGreenValue = 0.0;
  double myBlueValue = 0.0;
  int myTransparencyValue = 0;

  int myStretchedRedValue   = 0;
  int myStretchedGreenValue = 0;
  int myStretchedBlueValue  = 0;
  int myAlphaValue = 0;
  QgsContrastEnhancement* myRedContrastEnhancement = contrastEnhancement( myRedBandNo );
  QgsContrastEnhancement* myGreenContrastEnhancement = contrastEnhancement( myGreenBandNo );
  QgsContrastEnhancement* myBlueContrastEnhancement = contrastEnhancement( myBlueBandNo );

  QgsRasterImageBuffer redImageBuffer( mDataProvider, myRedBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  redImageBuffer.reset();
  QgsRasterImageBuffer greenImageBuffer( mDataProvider, myGreenBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  greenImageBuffer.setWritingEnabled( false ); //only draw to redImageBuffer
  greenImageBuffer.reset();
  QgsRasterImageBuffer blueImageBuffer( mDataProvider, myBlueBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  blueImageBuffer.setWritingEnabled( false ); //only draw to redImageBuffer
  blueImageBuffer.reset();

  QgsRasterImageBuffer *transparencyImageBuffer = 0;
  if ( hasTransparencyBand )
  {
    transparencyImageBuffer = new QgsRasterImageBuffer( mDataProvider, myTransparencyBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
    transparencyImageBuffer->setWritingEnabled( false ); //only draw to redImageBuffer
    transparencyImageBuffer->reset();
  }

  while ( redImageBuffer.nextScanLine( &redImageScanLine, &redRasterScanLine )
          && greenImageBuffer.nextScanLine( &greenImageScanLine, &greenRasterScanLine )
          && blueImageBuffer.nextScanLine( &blueImageScanLine, &blueRasterScanLine )
          && ( !transparencyImageBuffer || transparencyImageBuffer->nextScanLine( &transparencyImageScanLine, &transparencyRasterScanLine ) ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      if ( transparencyImageBuffer )
      {
        myTransparencyValue = readValue( transparencyRasterScanLine, myTransparencyType, i );
        if ( 0 == myTransparencyValue )
        {
          redImageScanLine[ i ] = myDefaultColor;
          continue;
        }
      }

      myRedValue   = readValue( redRasterScanLine, myRedType, i );
      myGreenValue = readValue( greenRasterScanLine, myGreenType, i );
      myBlueValue  = readValue( blueRasterScanLine, myBlueType, i );

      if ( mValidNoDataValue &&
           (
             ( qAbs( myRedValue - mNoDataValue ) <= TINY_VALUE || myRedValue != myRedValue ) ||
             ( qAbs( myGreenValue - mNoDataValue ) <= TINY_VALUE || myGreenValue != myGreenValue ) ||
             ( qAbs( myBlueValue - mNoDataValue ) <= TINY_VALUE || myBlueValue != myBlueValue )
           )
         )
      {
        redImageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( QgsContrastEnhancement::NoEnhancement != contrastEnhancementAlgorithm() &&
           ( !myRedContrastEnhancement->isValueInDisplayableRange( myRedValue ) ||
             !myGreenContrastEnhancement->isValueInDisplayableRange( myGreenValue ) ||
             !myBlueContrastEnhancement->isValueInDisplayableRange( myBlueValue ) ) )
      {
        redImageScanLine[ i ] = myDefaultColor;
        continue;
      }

      myAlphaValue = mRasterTransparency.alphaValue( myRedValue, myGreenValue, myBlueValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        redImageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( QgsContrastEnhancement::NoEnhancement == contrastEnhancementAlgorithm() )
      {
        myStretchedRedValue = myRedValue;
        myStretchedGreenValue = myGreenValue;
        myStretchedBlueValue = myBlueValue;
      }
      else
      {
        myStretchedRedValue = myRedContrastEnhancement->enhanceContrast( myRedValue );
        myStretchedGreenValue = myGreenContrastEnhancement->enhanceContrast( myGreenValue );
        myStretchedBlueValue = myBlueContrastEnhancement->enhanceContrast( myBlueValue );
      }

      if ( mInvertColor )
      {
        myStretchedRedValue = 255 - myStretchedRedValue;
        myStretchedGreenValue = 255 - myStretchedGreenValue;
        myStretchedBlueValue = 255 - myStretchedBlueValue;
      }

      if ( myTransparencyValue )
        myAlphaValue *= myTransparencyValue / 255.0;

      redImageScanLine[ i ] = qRgba( myStretchedRedValue, myStretchedGreenValue, myStretchedBlueValue, myAlphaValue );
    }
  }

  if ( transparencyImageBuffer )
    delete transparencyImageBuffer;
}

void QgsRasterLayer::drawMultiBandSingleBandGray( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  //delegate to drawSingleBandGray!
  drawSingleBandGray( theQPainter, theRasterViewPort, theQgsMapToPixel, theBandNo );
}

void QgsRasterLayer::drawMultiBandSingleBandPseudoColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  //delegate to drawSinglePseudocolor!
  drawSingleBandPseudoColor( theQPainter, theRasterViewPort, theQgsMapToPixel, theBandNo );
}

/**
 * This method is used to render a single band with a color map.
 * @param theQPainter pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theQgsMapToPixel transformation coordinate to map canvas pixel
 * @param theBandNo band number
 */
void QgsRasterLayer::drawPalettedSingleBandColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  QgsDebugMsg( "entered." );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNo )
  {
    return;
  }

  int myTransparencyBandNo = bandNumber( mTransparencyBandName );
  bool hasTransparencyBand = 0 < myTransparencyBandNo;

  if ( NULL == mRasterShader )
  {
    return;
  }

  int myDataType = mDataProvider->dataType( theBandNo );
  int myTransparencyType = hasTransparencyBand ? mDataProvider->dataType( myTransparencyBandNo ) : 0;

  QgsRasterImageBuffer imageBuffer( mDataProvider, theBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QgsRasterImageBuffer *transparencyImageBuffer = 0;
  if ( hasTransparencyBand )
  {
    transparencyImageBuffer = new QgsRasterImageBuffer( mDataProvider, myTransparencyBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
    transparencyImageBuffer->setWritingEnabled( false ); //only draw to imageBuffer
    transparencyImageBuffer->reset();
  }

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;
  QRgb* transparencyImageScanLine = 0;
  void* transparencyRasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );
  double myPixelValue = 0.0;
  int myRedValue = 0;
  int myGreenValue = 0;
  int myBlueValue = 0;
  int myTransparencyValue = 0;
  int myAlphaValue = 0;

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine )
          && ( !transparencyImageBuffer || transparencyImageBuffer->nextScanLine( &transparencyImageScanLine, &transparencyRasterScanLine ) ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      if ( transparencyImageBuffer )
      {
        myTransparencyValue = readValue( transparencyRasterScanLine, myTransparencyType, i );
        if ( 0 == myTransparencyValue )
        {
          imageScanLine[ i ] = myDefaultColor;
          continue;
        }
      }

      myRedValue = 0;
      myGreenValue = 0;
      myBlueValue = 0;
      //myPixelValue = readValue( rasterScanLine, ( GDALDataType )myDataType, i );
      myPixelValue = readValue( rasterScanLine, myDataType, i );

      if ( mValidNoDataValue && ( qAbs( myPixelValue - mNoDataValue ) <= TINY_VALUE || myPixelValue != myPixelValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      myAlphaValue = mRasterTransparency.alphaValue( myPixelValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( !mRasterShader->shade( myPixelValue, &myRedValue, &myGreenValue, &myBlueValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( myTransparencyValue )
        myAlphaValue *= myTransparencyValue / 255.0;

      if ( mInvertColor )
      {
        //Invert flag, flip blue and red
        imageScanLine[ i ] = qRgba( myBlueValue, myGreenValue, myRedValue, myAlphaValue );
      }
      else
      {
        //Normal
        imageScanLine[ i ] = qRgba( myRedValue, myGreenValue, myBlueValue, myAlphaValue );
      }
    }
  }

  if ( transparencyImageBuffer )
    delete transparencyImageBuffer;
}

/**
 * This method is used to render a paletted raster layer as a gray image.
 * @param theQPainter pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theQgsMapToPixel transformation between map coordinates and canvas pixels
 * @param theBandNo band number
 */
void QgsRasterLayer::drawPalettedSingleBandGray( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  QgsDebugMsg( "entered." );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNo )
  {
    return;
  }

  int myTransparencyBandNo = bandNumber( mTransparencyBandName );
  bool hasTransparencyBand = 0 < myTransparencyBandNo;

  if ( NULL == mRasterShader )
  {
    return;
  }

  int myDataType = mDataProvider->dataType( theBandNo );
  int myTransparencyType = hasTransparencyBand ? mDataProvider->dataType( myTransparencyBandNo ) : 0;

  QgsRasterImageBuffer imageBuffer( mDataProvider, theBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QgsRasterImageBuffer *transparencyImageBuffer = 0;
  if ( hasTransparencyBand )
  {
    transparencyImageBuffer = new QgsRasterImageBuffer( mDataProvider, myTransparencyBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
    transparencyImageBuffer->setWritingEnabled( false ); //only draw to redImageBuffer
    transparencyImageBuffer->reset();
  }

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;
  QRgb* transparencyImageScanLine = 0;
  void* transparencyRasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );
  double myPixelValue = 0.0;
  int myRedValue = 0;
  int myGreenValue = 0;
  int myBlueValue = 0;
  int myTransparencyValue = 0;
  int myAlphaValue = 0;

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine )
          && ( !transparencyImageBuffer || transparencyImageBuffer->nextScanLine( &transparencyImageScanLine, &transparencyRasterScanLine ) ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      if ( transparencyImageBuffer )
      {
        myTransparencyValue = readValue( transparencyRasterScanLine, myTransparencyType, i );
        if ( 0 == myTransparencyValue )
        {
          imageScanLine[ i ] = myDefaultColor;
          continue;
        }
      }

      myRedValue = 0;
      myGreenValue = 0;
      myBlueValue = 0;
      //myPixelValue = readValue( rasterScanLine, ( GDALDataType )myDataType, i );
      myPixelValue = readValue( rasterScanLine, myDataType, i );

      if ( mValidNoDataValue && ( qAbs( myPixelValue - mNoDataValue ) <= TINY_VALUE || myPixelValue != myPixelValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      myAlphaValue = mRasterTransparency.alphaValue( myPixelValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( !mRasterShader->shade( myPixelValue, &myRedValue, &myGreenValue, &myBlueValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( myTransparencyValue )
        myAlphaValue *= myTransparencyValue / 255.0;

      if ( mInvertColor )
      {
        //Invert flag, flip blue and red
        double myGrayValue = ( 0.3 * ( double )myRedValue ) + ( 0.59 * ( double )myGreenValue ) + ( 0.11 * ( double )myBlueValue );
        imageScanLine[ i ] = qRgba(( int )myGrayValue, ( int )myGrayValue, ( int )myGrayValue, myAlphaValue );
      }
      else
      {
        //Normal
        double myGrayValue = ( 0.3 * ( double )myBlueValue ) + ( 0.59 * ( double )myGreenValue ) + ( 0.11 * ( double )myRedValue );
        imageScanLine[ i ] = qRgba(( int )myGrayValue, ( int )myGrayValue, ( int )myGrayValue, myAlphaValue );
      }
    }
  }

  if ( transparencyImageBuffer )
    delete transparencyImageBuffer;
}

/**
 * This method is used to render a paletted raster layer as a pseudocolor image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theQgsMapToPixel transformation between map coordinates and canvas pixels
 * @param theBandNo band number
 gray.
 */
void QgsRasterLayer::drawPalettedSingleBandPseudoColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  QgsDebugMsg( "entered." );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNo )
  {
    return;
  }

  int myTransparencyBandNo = bandNumber( mTransparencyBandName );
  bool hasTransparencyBand = 0 < myTransparencyBandNo;

  if ( NULL == mRasterShader )
  {
    return;
  }

  QgsRasterBandStats myRasterBandStats = bandStatistics( theBandNo );
  int myDataType = mDataProvider->dataType( theBandNo );
  int myTransparencyType = hasTransparencyBand ? mDataProvider->dataType( myTransparencyBandNo ) : 0;

  QgsRasterImageBuffer imageBuffer( mDataProvider, theBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QgsRasterImageBuffer *transparencyImageBuffer = 0;
  if ( hasTransparencyBand )
  {
    transparencyImageBuffer = new QgsRasterImageBuffer( mDataProvider, myTransparencyBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
    transparencyImageBuffer->setWritingEnabled( false ); //only draw to imageBuffer
    transparencyImageBuffer->reset();
  }

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;
  QRgb* transparencyImageScanLine = 0;
  void* transparencyRasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );
  double myMinimumValue = 0.0;
  double myMaximumValue = 0.0;
  //Use standard deviations if set, otherwise, use min max of band
  if ( mStandardDeviations > 0 )
  {
    myMinimumValue = ( myRasterBandStats.mean - ( mStandardDeviations * myRasterBandStats.stdDev ) );
    myMaximumValue = ( myRasterBandStats.mean + ( mStandardDeviations * myRasterBandStats.stdDev ) );
  }
  else
  {
    myMinimumValue = myRasterBandStats.minimumValue;
    myMaximumValue = myRasterBandStats.maximumValue;
  }

  mRasterShader->setMinimumValue( myMinimumValue );
  mRasterShader->setMaximumValue( myMaximumValue );

  double myPixelValue = 0.0;
  int myRedValue = 0;
  int myGreenValue = 0;
  int myBlueValue = 0;
  int myTransparencyValue = 0;
  int myAlphaValue = 0;

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine )
          && ( !transparencyImageBuffer || transparencyImageBuffer->nextScanLine( &transparencyImageScanLine, &transparencyRasterScanLine ) ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      if ( transparencyImageBuffer )
      {
        myTransparencyValue = readValue( transparencyRasterScanLine, myTransparencyType, i );
        if ( 0 == myTransparencyValue )
        {
          imageScanLine[ i ] = myDefaultColor;
          continue;
        }
      }

      myPixelValue = readValue( rasterScanLine, myDataType, i );

      if ( mValidNoDataValue && ( qAbs( myPixelValue - mNoDataValue ) <= TINY_VALUE || myPixelValue != myPixelValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      myAlphaValue = mRasterTransparency.alphaValue( myPixelValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( !mRasterShader->shade( myPixelValue, &myRedValue, &myGreenValue, &myBlueValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( myTransparencyValue )
        myAlphaValue *= myTransparencyValue / 255.0;

      if ( mInvertColor )
      {
        //Invert flag, flip blue and red
        imageScanLine[ i ] = qRgba( myBlueValue, myGreenValue, myRedValue, myAlphaValue );
      }
      else
      {
        //Normal
        imageScanLine[ i ] = qRgba( myRedValue, myGreenValue, myBlueValue, myAlphaValue );
      }
    }
  }

  if ( transparencyImageBuffer )
    delete transparencyImageBuffer;
}

/**
 * This method is used to render a paletted raster layer as a color image -- currently not supported
 * @param theQPainter pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theQgsMapToPixel transformation coordinate to map canvas pixel
 * @param theBandNo the number of the band which should be rendered.
 * @note not supported at this time
 */
void QgsRasterLayer::drawPalettedMultiBandColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  Q_UNUSED( theQPainter );
  Q_UNUSED( theRasterViewPort );
  Q_UNUSED( theQgsMapToPixel );
  Q_UNUSED( theBandNo );
  QgsDebugMsg( "Not supported at this time" );
}

void QgsRasterLayer::drawSingleBandGray( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  QgsDebugMsg( "layer=" + QString::number( theBandNo ) );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNo )
  {
    return;
  }

  int myTransparencyBandNo = bandNumber( mTransparencyBandName );
  bool hasTransparencyBand = 0 < myTransparencyBandNo;

  int myDataType = mDataProvider->dataType( theBandNo );
  QgsDebugMsg( "myDataType = " + QString::number( myDataType ) );
  int myTransparencyType = hasTransparencyBand ? mDataProvider->dataType( myTransparencyBandNo ) : 0;

  QgsRasterImageBuffer imageBuffer( mDataProvider, theBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QgsRasterImageBuffer *transparencyImageBuffer = 0;
  if ( hasTransparencyBand )
  {
    transparencyImageBuffer = new QgsRasterImageBuffer( mDataProvider, myTransparencyBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
    transparencyImageBuffer->setWritingEnabled( false ); //only draw to imageBuffer
    transparencyImageBuffer->reset();
  }

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;
  QRgb* transparencyImageScanLine = 0;
  void* transparencyRasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );
  double myGrayValue = 0.0;
  int myGrayVal = 0;
  int myTransparencyValue = 0;
  int myAlphaValue = 0;
  QgsContrastEnhancement* myContrastEnhancement = contrastEnhancement( theBandNo );

  QgsRasterBandStats myGrayBandStats;
  //myGrayBandStats = bandStatistics( theBandNo ); // debug
  if ( QgsContrastEnhancement::NoEnhancement != contrastEnhancementAlgorithm() && !mUserDefinedGrayMinimumMaximum && mStandardDeviations > 0 )
  {
    mGrayMinimumMaximumEstimated = false;
    myGrayBandStats = bandStatistics( theBandNo );
    setMaximumValue( theBandNo, myGrayBandStats.mean + ( mStandardDeviations * myGrayBandStats.stdDev ) );
    setMinimumValue( theBandNo, myGrayBandStats.mean - ( mStandardDeviations * myGrayBandStats.stdDev ) );
  }
  else if ( QgsContrastEnhancement::NoEnhancement != contrastEnhancementAlgorithm() && !mUserDefinedGrayMinimumMaximum )
  {
    //This case will be true the first time the image is loaded, so just approimate the min max to keep
    //from calling generate raster band stats
    mGrayMinimumMaximumEstimated = true;
    setMaximumValue( theBandNo, mDataProvider->maximumValue( theBandNo ) );
    setMinimumValue( theBandNo, mDataProvider->minimumValue( theBandNo ) );
  }

  QgsDebugMsg( " -> imageBuffer.nextScanLine" );
  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine )
          && ( !transparencyImageBuffer || transparencyImageBuffer->nextScanLine( &transparencyImageScanLine, &transparencyRasterScanLine ) ) )
  {
    //QgsDebugMsg( " rendering line");
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      if ( transparencyImageBuffer )
      {
        myTransparencyValue = readValue( transparencyRasterScanLine, myTransparencyType, i );
        if ( 0 == myTransparencyValue )
        {
          imageScanLine[ i ] = myDefaultColor;
          continue;
        }
      }

      myGrayValue = readValue( rasterScanLine, myDataType, i );
      //QgsDebugMsg( QString( "i = %1 myGrayValue = %2 ").arg(i).arg( myGrayValue ) );
      //if ( myGrayValue != -2147483647 ) {
      //QgsDebugMsg( "myGrayValue = " + QString::number( myGrayValue ) );
      //}

      if ( mValidNoDataValue && ( qAbs( myGrayValue - mNoDataValue ) <= TINY_VALUE || myGrayValue != myGrayValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( !myContrastEnhancement->isValueInDisplayableRange( myGrayValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      myAlphaValue = mRasterTransparency.alphaValue( myGrayValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }


      myGrayVal = myContrastEnhancement->enhanceContrast( myGrayValue );

      if ( mInvertColor )
      {
        myGrayVal = 255 - myGrayVal;
      }

      if ( myTransparencyValue )
        myAlphaValue *= myTransparencyValue / 255.0;

      //QgsDebugMsg( QString( "i = %1 myGrayValue = %2 myGrayVal = %3 myAlphaValue = %4").arg(i).arg( myGrayValue ).arg(myGrayVal).arg(myAlphaValue) );
      imageScanLine[ i ] = qRgba( myGrayVal, myGrayVal, myGrayVal, myAlphaValue );
    }
  }

  if ( transparencyImageBuffer )
    delete transparencyImageBuffer;
} // QgsRasterLayer::drawSingleBandGray

void QgsRasterLayer::drawSingleBandPseudoColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  QgsDebugMsg( "entered." );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNo )
  {
    return;
  }

  int myTransparencyBandNo = bandNumber( mTransparencyBandName );
  bool hasTransparencyBand = 0 < myTransparencyBandNo;

  if ( NULL == mRasterShader )
  {
    return;
  }

  QgsRasterBandStats myRasterBandStats = bandStatistics( theBandNo );
  int myDataType = mDataProvider->dataType( theBandNo );
  int myTransparencyType = hasTransparencyBand ? mDataProvider->dataType( myTransparencyBandNo ) : 0;

  QgsRasterImageBuffer imageBuffer( mDataProvider, theBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QgsRasterImageBuffer *transparencyImageBuffer = 0;
  if ( hasTransparencyBand )
  {
    transparencyImageBuffer = new QgsRasterImageBuffer( mDataProvider, myTransparencyBandNo, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
    transparencyImageBuffer->setWritingEnabled( false ); //only draw to imageBuffer
    transparencyImageBuffer->reset();
  }

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;
  QRgb* transparencyImageScanLine = 0;
  void* transparencyRasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );

  double myMinimumValue = 0.0;
  double myMaximumValue = 0.0;
  //Use standard deviations if set, otherwise, use min max of band
  if ( mStandardDeviations > 0 )
  {
    myMinimumValue = ( myRasterBandStats.mean - ( mStandardDeviations * myRasterBandStats.stdDev ) );
    myMaximumValue = ( myRasterBandStats.mean + ( mStandardDeviations * myRasterBandStats.stdDev ) );
  }
  else
  {
    myMinimumValue = myRasterBandStats.minimumValue;
    myMaximumValue = myRasterBandStats.maximumValue;
  }

  mRasterShader->setMinimumValue( myMinimumValue );
  mRasterShader->setMaximumValue( myMaximumValue );

  int myRedValue = 255;
  int myGreenValue = 255;
  int myBlueValue = 255;
  int myTransparencyValue = 0;

  double myPixelValue = 0.0;
  int myAlphaValue = 0;

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine )
          && ( !transparencyImageBuffer || transparencyImageBuffer->nextScanLine( &transparencyImageScanLine, &transparencyRasterScanLine ) ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      if ( transparencyImageBuffer )
      {
        myTransparencyValue = readValue( transparencyRasterScanLine, myTransparencyType, i );
        if ( 0 == myTransparencyValue )
        {
          imageScanLine[ i ] = myDefaultColor;
          continue;
        }
      }

      myPixelValue = readValue( rasterScanLine, myDataType, i );

      if ( mValidNoDataValue && ( qAbs( myPixelValue - mNoDataValue ) <= TINY_VALUE || myPixelValue != myPixelValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      myAlphaValue = mRasterTransparency.alphaValue( myPixelValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( !mRasterShader->shade( myPixelValue, &myRedValue, &myGreenValue, &myBlueValue ) )
      {
        imageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( myTransparencyValue )
        myAlphaValue *= myTransparencyValue / 255.0;

      if ( mInvertColor )
      {
        //Invert flag, flip blue and red
        imageScanLine[ i ] = qRgba( myBlueValue, myGreenValue, myRedValue, myAlphaValue );
      }
      else
      {
        //Normal
        imageScanLine[ i ] = qRgba( myRedValue, myGreenValue, myBlueValue, myAlphaValue );
        //QgsDebugMsg ( QString ( "%1 value :  %2 rgba : %3 %4 %5 %6" ).arg (i).arg( myPixelValue ).arg(myRedValue).arg(myGreenValue).arg(myBlueValue).arg(myAlphaValue) );
      }
    }
  }

  if ( transparencyImageBuffer )
    delete transparencyImageBuffer;
}

#if 0
QString QgsRasterLayer::generateBandName( int theBandNumber )
{
  return tr( "Band" ) + QString( " %1" ) .arg( theBandNumber,  1 + ( int ) log10(( float ) bandCount() ), 10, QChar( '0' ) );
}
#endif

/**
 * This method looks to see if a given band name exists.
 *@note This function is no longer really needed and about to be removed
 */
bool QgsRasterLayer::hasBand( QString const & theBandName )
{
  //TODO: This function is no longer really needed and about be removed -- it is only used to see if "Palette" exists which is not the correct way to see if a band is paletted or not
  QgsDebugMsg( "Looking for band : " + theBandName );

  for ( int i = 1; i <= mDataProvider->bandCount(); i++ )
  {
    QString myColorQString = mDataProvider->colorInterpretationName( i );
    QgsDebugMsgLevel( QString( "band%1" ).arg( i ), 2 );

    if ( myColorQString == theBandName )
    {
      QgsDebugMsgLevel( QString( "band%1" ).arg( i ), 2 );
      QgsDebugMsgLevel( "Found band : " + theBandName, 2 );

      return true;
    }
    QgsDebugMsgLevel( "Found unmatched band : " + QString::number( i ) + " " + myColorQString, 2 );
  }
  return false;
}

QString QgsRasterLayer::projectionWkt()
{
  // TODO: where is it used? It would be better to use crs.
  return mDataProvider->crs().toWkt();
}

/*
 *data type is the same as raster band. The memory must be released later!
 *  \return pointer to the memory
 */
void *QgsRasterLayer::readData( int bandNo, QgsRasterViewPort *viewPort )
{
  int size = mDataProvider->dataTypeSize( bandNo ) / 8;

#if 0
  QgsDebugMsg( "calling RasterIO with " +
               QString( ", source NW corner: " ) + QString::number( viewPort->rectXOffset ) +
               ", " + QString::number( viewPort->rectYOffset ) +
               ", source size: " + QString::number( viewPort->clippedWidth ) +
               ", " + QString::number( viewPort->clippedHeight ) +
               ", dest size: " + QString::number( viewPort->drawableAreaXDim ) +
               ", " + QString::number( viewPort->drawableAreaYDim ) );
#endif
  void *data = VSIMalloc( size * viewPort->drawableAreaXDim * viewPort->drawableAreaYDim );

  /* Abort if out of memory */
  if ( data == NULL )
  {
    QgsDebugMsg( "Layer " + name() + " couldn't allocate enough memory. Ignoring" );
  }
  else
  {
    // TODO: check extent
    QgsRectangle partExtent(
      viewPort->mDrawnExtent.xMinimum(),
      viewPort->mDrawnExtent.yMinimum(),
      viewPort->mDrawnExtent.xMaximum(),
      viewPort->mDrawnExtent.yMaximum()
    );
    mDataProvider->readBlock( bandNo, partExtent, viewPort->drawableAreaXDim, viewPort->drawableAreaYDim, viewPort->mSrcCRS, viewPort->mDestCRS, data );
  }
  return data;
}

/*
 * @note Called from ctor if a raster image given there
 *
 * @param theFilename absolute path and filename of the raster to be loaded
 * @returns true if successfully read file
 */
bool QgsRasterLayer::readFile( QString const &theFilename )
{
  Q_UNUSED( theFilename );
  mValid = false;
  return true;
} // QgsRasterLayer::readFile

/*
 *  @param index index in memory block
 */
double QgsRasterLayer::readValue( void *data, int type, int index )
{
  if ( !data )
    return mValidNoDataValue ? mNoDataValue : 0.0;

  switch ( type )
  {
    case QgsRasterDataProvider::Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case QgsRasterDataProvider::UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case QgsRasterDataProvider::Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case QgsRasterDataProvider::UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case QgsRasterDataProvider::Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case QgsRasterDataProvider::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QgsRasterDataProvider::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }

  return mValidNoDataValue ? mNoDataValue : 0.0;
}

bool QgsRasterLayer::update()
{
  QgsDebugMsg( "entered." );
  // Check if data changed
  if ( mDataProvider->dataTimestamp() > mDataProvider->timestamp() )
  {
    QgsDebugMsg( "reload data" );
    closeDataProvider();
    init();
    setDataProvider( mProviderKey, mLayers, mStyles, mFormat, mCrs );
    emit dataChanged();
  }
  return mValid;
}

bool QgsRasterLayer::usesProvider()
{
  return !mProviderKey.isEmpty();
}

QString QgsRasterLayer::validateBandName( QString const & theBandName )
{
  QgsDebugMsg( "Checking..." );
  //check if the band is unset
  if ( theBandName == TRSTRING_NOT_SET || theBandName == QSTRING_NOT_SET )
  {
    QgsDebugMsg( "Band name is '" + QSTRING_NOT_SET + "'. Nothing to do." );
    // Use translated name internally
    return TRSTRING_NOT_SET;
  }

  //check that a valid band name was passed
  QgsDebugMsg( "Looking through raster band stats for matching band name" );
  for ( int myIterator = 0; myIterator < mRasterStatsList.size(); ++myIterator )
  {
    //find out the name of this band
    if ( mRasterStatsList[myIterator].bandName == theBandName )
    {
      QgsDebugMsg( "Matching band name found" );
      return theBandName;
    }
  }
  QgsDebugMsg( "No matching band name found in raster band stats" );

  QgsDebugMsg( "Testing for non zero-buffered names" );
  //TODO Remove test in v2.0 or earlier
  QStringList myBandNameComponents = theBandName.split( " " );
  if ( myBandNameComponents.size() == 2 )
  {
    int myBandNumber = myBandNameComponents.at( 1 ).toInt();
    if ( myBandNumber > 0 )
    {
      QString myBandName = mDataProvider->generateBandName( myBandNumber );
      for ( int myIterator = 0; myIterator < mRasterStatsList.size(); ++myIterator )
      {
        //find out the name of this band
        if ( mRasterStatsList[myIterator].bandName == myBandName )
        {
          QgsDebugMsg( "Matching band name found" );
          return myBandName;
        }
      }
    }
  }

  QgsDebugMsg( "Testing older naming format" );
  //See of the band in an older format #:something.
  //TODO Remove test in v2.0 or earlier
  myBandNameComponents.clear();
  if ( theBandName.contains( ':' ) )
  {
    myBandNameComponents = theBandName.split( ":" );
    if ( myBandNameComponents.size() == 2 )
    {
      int myBandNumber = myBandNameComponents.at( 0 ).toInt();
      if ( myBandNumber > 0 )
      {
        QgsDebugMsg( "Transformed older name format to current format" );
        return "Band " + QString::number( myBandNumber );
      }
    }
  }

  //if no matches were found default to not set
  QgsDebugMsg( "All checks failed, returning '" + QSTRING_NOT_SET + "'" );
  return TRSTRING_NOT_SET;
}

QgsRasterImageBuffer::QgsRasterImageBuffer( QgsRasterDataProvider *dataProvider, int bandNo, QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* mapToPixel, double* geoTransform ):
    mDataProvider( dataProvider ), mBandNo( bandNo ), mPainter( p ), mViewPort( viewPort ), mMapToPixel( mapToPixel ), mGeoTransform( geoTransform ), mValid( false ), mWritingEnabled( true ), mDrawPixelRect( false ), mCurrentImage( 0 ), mCurrentGDALData( 0 )
{

}

QgsRasterImageBuffer::~QgsRasterImageBuffer()
{
  delete mCurrentImage;
  CPLFree( mCurrentGDALData );
}

void QgsRasterImageBuffer::reset( int maxPixelsInVirtualMemory )
{
  QgsDebugMsg( "Start" );
  //if ( !mRasterBand || !mPainter || !mViewPort )
  if ( !mDataProvider || mBandNo <= 0 || !mPainter || !mViewPort )
  {
    mValid = false;
    return;
  }

  mValid = true;

  //decide on the partition of the image

  int pixels = mViewPort->drawableAreaXDim * mViewPort->drawableAreaYDim;
  int mNumPartImages = pixels / maxPixelsInVirtualMemory + 1.0;
  mNumRasterRowsPerPart = ( double )mViewPort->drawableAreaYDim / ( double )mNumPartImages + 0.5;

  mCurrentPartRasterMin = -1;
  mCurrentPartRasterMax = -1;
  mCurrentPartImageRow = 0;
  mNumCurrentImageRows = 0;

  mCurrentPart = 0;

  createNextPartImage();

  // TODO
  //if ( 2 >= mViewPort->clippedWidth && 2 >= mViewPort->clippedHeight )
  if ( false )
  {
    //use Peter's fix for zoomed in rasters
    mDrawPixelRect = true;
  }
}

bool QgsRasterImageBuffer::nextScanLine( QRgb** imageScanLine, void** rasterScanLine )
{
  //QgsDebugMsg( "mCurrentRow = " + QString::number( mCurrentRow ) );
  if ( !mValid )
    return false;

  if ( !mCurrentImage && !mCurrentGDALData )
  {
    return false;
  }

  if ( mCurrentPartImageRow >= mNumCurrentImageRows )
  {
    if ( !createNextPartImage() )
    {
      return false;
    }
  }

  if ( mWritingEnabled )
  {
    *imageScanLine = ( QRgb* ) mCurrentImage->scanLine( mCurrentPartImageRow );
  }
  else
  {
    *imageScanLine = 0;
  }
  int size = mDataProvider->dataTypeSize( mBandNo ) / 8;
  *rasterScanLine = ( unsigned char * )mCurrentGDALData + mCurrentPartImageRow * mViewPort->drawableAreaXDim * size;

  ++mCurrentPartImageRow;
  ++mCurrentRow;
  return !mWritingEnabled || *imageScanLine;
}

bool QgsRasterImageBuffer::createNextPartImage()
{
  QgsDebugMsg( "Entered" );
  //draw the last image if mCurrentImage exists
  if ( mCurrentImage )
  {
    if ( mWritingEnabled )
    {
      // TODO: consider similar system with raster providers, see the comment
      // in QgsRasterImageBuffer::drawPixelRectangle()
      // e.g request the block with raster resolution and draw pixels as rectangles
      //if ( 2 >= mViewPort->clippedWidth && 2 >= mViewPort->clippedHeight )
      if ( false )
      {
        drawPixelRectangle();
      }
      else
      {
        //int paintXoffset = 0;
        //int paintYoffset = 0;
        double imageX = 0;
        double imageY = 0;

        if ( mMapToPixel )
        {
          imageX = mViewPort->topLeftPoint.x();
          imageY = mViewPort->topLeftPoint.y() + mCurrentPartRasterMin;
        }

        QgsDebugMsg( QString( "mCurrentPartRasterMin = %1" ).arg( mCurrentPartRasterMin ) );
        QgsDebugMsg( QString( "imageX = %1 imageY = %2" ).arg( imageX ).arg( imageY ) );
        mPainter->drawImage( QPointF( imageX, imageY ),  //the top-left point in the paint device
                             *mCurrentImage );
      }
    }
  }

  delete mCurrentImage; mCurrentImage = 0;
  CPLFree( mCurrentGDALData ); mCurrentGDALData = 0;

  mCurrentPart++; // NEW
  QgsDebugMsg( QString( "mCurrentPartRasterMax = %1 mViewPort->drawableAreaYDim = %2" ).arg( mCurrentPartRasterMax ).arg( mViewPort->drawableAreaYDim ) );
  if ( mCurrentPartRasterMax >= mViewPort->drawableAreaYDim )
  {
    return false; //already at the end...
  }

  mCurrentPartRasterMin = mCurrentPartRasterMax + 1;
  mCurrentPartRasterMax = mCurrentPartRasterMin + mNumRasterRowsPerPart;
  if ( mCurrentPartRasterMax > mViewPort->drawableAreaYDim )
  {
    mCurrentPartRasterMax = mViewPort->drawableAreaYDim;
  }
  mCurrentRow = mCurrentPartRasterMin;
  mCurrentPartImageRow = 0;

  //read GDAL image data
  int size = mDataProvider->dataTypeSize( mBandNo ) / 8 ;
  int xSize = mViewPort->drawableAreaXDim;
  int ySize = mViewPort->drawableAreaYDim;

  //make the raster tiles overlap at least 2 pixels to avoid white stripes
  int overlapRows = 0;
  if ( mMapToPixel )
  {
    // TODO: do we still need overlaps?
    overlapRows = mMapToPixel->mapUnitsPerPixel() / qAbs( mGeoTransform[5] ) + 2;
  }
  //if ( mCurrentPartRasterMax + overlapRows >= mViewPort->clippedHeight )
  if ( mCurrentPartRasterMax + overlapRows >= mViewPort->drawableAreaYDim )
  {
    overlapRows = 0;
  }
  int rasterYSize = mCurrentPartRasterMax - mCurrentPartRasterMin + overlapRows;
  QgsDebugMsg( "rasterYSize = " + QString::number( rasterYSize ) );

  // TODO: consider something like this
  //if ( 2 >= mViewPort->clippedWidth && 2 >= mViewPort->clippedHeight ) //for zoomed in rasters
  if ( false )
  {
    //rasterYSize = mViewPort->clippedHeight;
    //ySize = mViewPort->drawableAreaYDim;
  }
  else //normal mode
  {
    if ( mMapToPixel )
    {
      // makes no more sense
      //ySize = qAbs((( rasterYSize ) / mMapToPixel->mapUnitsPerPixel() * mGeoTransform[5] ) ) + 0.5;
    }
  }
  QgsDebugMsg( QString( "xSize = %1 ySize = %2" ).arg( xSize ).arg( ySize ) );
  if ( ySize < 1 || xSize < 1 )
  {
    return false;
  }
  mNumCurrentImageRows = rasterYSize;
  QgsDebugMsg( "alloc " + QString::number( size * xSize * rasterYSize ) );
  mCurrentGDALData = VSIMalloc( size * xSize * rasterYSize );

  double yMax = mViewPort->mDrawnExtent.yMaximum() - mCurrentRow * mMapToPixel->mapUnitsPerPixel();
  double yMin = yMax - rasterYSize * mMapToPixel->mapUnitsPerPixel();

  QgsDebugMsg( QString( "mCurrentRow = %1 yMaximum = %2 ySize = %3 mapUnitsPerPixel = %4" ).arg( mCurrentRow ).arg( mViewPort->mDrawnExtent.yMaximum() ).arg( ySize ).arg( mMapToPixel->mapUnitsPerPixel() ) );
  QgsRectangle myPartExtent( mViewPort->mDrawnExtent.xMinimum(), yMin,
                             mViewPort->mDrawnExtent.xMaximum(), yMax );
  QgsDebugMsg( "myPartExtent is " + myPartExtent.toString() );
  mDataProvider->readBlock( mBandNo, myPartExtent, xSize, rasterYSize , mViewPort->mSrcCRS, mViewPort->mDestCRS, mCurrentGDALData );

  //create the QImage
  if ( mWritingEnabled )
  {
    mCurrentImage = new QImage( xSize, ySize, QImage::Format_ARGB32 );
    mCurrentImage->fill( qRgba( 255, 255, 255, 0 ) );
  }
  else
  {
    mCurrentImage = 0;
  }
  return true;
}

void QgsRasterImageBuffer::drawPixelRectangle()
{
// TODO: consider using similar with raster providers, originaly it was used only with
//    2 >= mViewPort->clippedWidth && 2 >= mViewPort->clippedHeight
// but why? but I believe that it should be used always if the ration of original
// raster resolution and device resolution is under certain limit
#if 0
  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;

  if ( mMapToPixel )
  {
    paintXoffset = static_cast<int>(
                     ( mViewPort->rectXOffsetFloat -
                       mViewPort->rectXOffset )
                     / mMapToPixel->mapUnitsPerPixel()
                     * qAbs( mGeoTransform[1] )
                   );

    paintYoffset = static_cast<int>(
                     ( mViewPort->rectYOffsetFloat -
                       mViewPort->rectYOffset )
                     / mMapToPixel->mapUnitsPerPixel()
                     * qAbs( mGeoTransform[5] )
                   );


  }

  //fix for zoomed in rasters
  //Catch special rendering cases
  //INSTANCE: 1x1
  if ( 1 == mViewPort->clippedWidth && 1 == mViewPort->clippedHeight )
  {
    QColor myColor( mCurrentImage->pixel( 0, 0 ) );
    myColor.setAlpha( qAlpha( mCurrentImage->pixel( 0, 0 ) ) );
    mPainter->fillRect( static_cast<int>( mViewPort->topLeftPoint.x() + 0.5 ),
                        static_cast<int>( mViewPort->topLeftPoint.y() + 0.5 ),
                        static_cast<int>( mViewPort->bottomRightPoint.x() ),
                        static_cast<int>( mViewPort->bottomRightPoint.y() ),
                        QBrush( myColor ) );
  }
  //1x2, 2x1 or 2x2
  else if ( 2 >= mViewPort->clippedWidth && 2 >= mViewPort->clippedHeight )
  {
    int myPixelBoundaryX = 0;
    int myPixelBoundaryY = 0;
    if ( mMapToPixel )
    {
      myPixelBoundaryX = static_cast<int>( mViewPort->topLeftPoint.x() + 0.5 ) + static_cast<int>( qAbs( mGeoTransform[1] / mMapToPixel->mapUnitsPerPixel() ) ) - paintXoffset;
      myPixelBoundaryY = static_cast<int>( mViewPort->topLeftPoint.y() + 0.5 ) + static_cast<int>( qAbs( mGeoTransform[5] / mMapToPixel->mapUnitsPerPixel() ) ) - paintYoffset;
    }

    //INSTANCE: 1x2
    if ( 1 == mViewPort->clippedWidth )
    {
      QColor myColor( mCurrentImage->pixel( 0, 0 ) );
      myColor.setAlpha( qAlpha( mCurrentImage->pixel( 0, 0 ) ) );
      mPainter->fillRect( static_cast<int>( mViewPort->topLeftPoint.x() + 0.5 ),
                          static_cast<int>( mViewPort->topLeftPoint.y() + 0.5 ),
                          static_cast<int>( mViewPort->bottomRightPoint.x() ),
                          static_cast<int>( myPixelBoundaryY ),
                          QBrush( myColor ) );
      myColor = QColor( mCurrentImage->pixel( 0, 1 ) );
      myColor.setAlpha( qAlpha( mCurrentImage->pixel( 0, 1 ) ) );
      mPainter->fillRect( static_cast<int>( mViewPort->topLeftPoint.x() + 0.5 ),
                          static_cast<int>( myPixelBoundaryY ),
                          static_cast<int>( mViewPort->bottomRightPoint.x() ),
                          static_cast<int>( mViewPort->bottomRightPoint.y() ),
                          QBrush( myColor ) );
    }
    else
    {
      //INSTANCE: 2x1
      if ( 1 == mViewPort->clippedHeight )
      {
        QColor myColor( mCurrentImage->pixel( 0, 0 ) );
        myColor.setAlpha( qAlpha( mCurrentImage->pixel( 0, 0 ) ) );
        mPainter->fillRect( static_cast<int>( mViewPort->topLeftPoint.x() + 0.5 ),
                            static_cast<int>( mViewPort->topLeftPoint.y() + 0.5 ),
                            static_cast<int>( myPixelBoundaryX ),
                            static_cast<int>( mViewPort->bottomRightPoint.y() ),
                            QBrush( myColor ) );
        myColor = QColor( mCurrentImage->pixel( 1, 0 ) );
        myColor.setAlpha( qAlpha( mCurrentImage->pixel( 1, 0 ) ) );
        mPainter->fillRect( static_cast<int>( myPixelBoundaryX ),
                            static_cast<int>( mViewPort->topLeftPoint.y() + 0.5 ),
                            static_cast<int>( mViewPort->bottomRightPoint.x() ),
                            static_cast<int>( mViewPort->bottomRightPoint.y() ),
                            QBrush( myColor ) );
      }
      //INSTANCE: 2x2
      else
      {
        QColor myColor( mCurrentImage->pixel( 0, 0 ) );
        myColor.setAlpha( qAlpha( mCurrentImage->pixel( 0, 0 ) ) );
        mPainter->fillRect( static_cast<int>( mViewPort->topLeftPoint.x() + 0.5 ),
                            static_cast<int>( mViewPort->topLeftPoint.y() + 0.5 ),
                            static_cast<int>( myPixelBoundaryX ),
                            static_cast<int>( myPixelBoundaryY ),
                            QBrush( myColor ) );
        myColor = QColor( mCurrentImage->pixel( 1, 0 ) );
        myColor.setAlpha( qAlpha( mCurrentImage->pixel( 1, 0 ) ) );
        mPainter->fillRect( static_cast<int>( myPixelBoundaryX ),
                            static_cast<int>( mViewPort->topLeftPoint.y() + 0.5 ),
                            static_cast<int>( mViewPort->bottomRightPoint.x() ),
                            static_cast<int>( myPixelBoundaryY ),
                            QBrush( myColor ) );
        myColor = QColor( mCurrentImage->pixel( 0, 1 ) );
        myColor.setAlpha( qAlpha( mCurrentImage->pixel( 0, 1 ) ) );
        mPainter->fillRect( static_cast<int>( mViewPort->topLeftPoint.x() + 0.5 ),
                            static_cast<int>( myPixelBoundaryY ),
                            static_cast<int>( myPixelBoundaryX ),
                            static_cast<int>( mViewPort->bottomRightPoint.y() ),
                            QBrush( myColor ) );
        myColor = QColor( mCurrentImage->pixel( 1, 1 ) );
        myColor.setAlpha( qAlpha( mCurrentImage->pixel( 1, 1 ) ) );
        mPainter->fillRect( static_cast<int>( myPixelBoundaryX ),
                            static_cast<int>( myPixelBoundaryY ),
                            static_cast<int>( mViewPort->bottomRightPoint.x() ),
                            static_cast<int>( mViewPort->bottomRightPoint.y() ),
                            QBrush( myColor ) );
      }
    }
  }
#endif
}

// Keep this for now, it is used by Python interface!!!
void QgsRasterLayer::registerGdalDrivers()
{
  if ( GDALGetDriverCount() == 0 )
    GDALAllRegister();
}

// Keep this for QgsRasterLayerProperties
bool QgsRasterLayer::readColorTable( int theBandNumber, QList<QgsColorRampShader::ColorRampItem>* theList )
{
  // TODO : check if exists - returned vale?
  QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = mDataProvider->colorTable( theBandNumber );
  if ( myColorRampItemList.size() == 0 )
  {
    return false;
  }
  *theList = myColorRampItemList;
  return true;
}

