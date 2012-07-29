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
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsprojectfiletransform.h"
#include "qgsproviderregistry.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpyramid.h"
#include "qgsrasterrendererregistry.h"
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
#include "qgsmultibandcolorrenderer.h"
#include "qgssinglebandcolordatarenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgssinglebandgrayrenderer.h"

#include "qgsrasterprojector.h"

#include "qgsrasteriterator.h"
#include "qgsrasterdrawer.h"

#include <cstdio>
#include <cmath>
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

const double QgsRasterLayer::CUMULATIVE_CUT_LOWER = 0.02;
const double QgsRasterLayer::CUMULATIVE_CUT_UPPER = 0.98;
const double QgsRasterLayer::SAMPLE_SIZE = 250000;

QgsRasterLayer::QgsRasterLayer()
    : QgsMapLayer( RasterLayer )
    , QSTRING_NOT_SET( "Not Set" )
    , TRSTRING_NOT_SET( tr( "Not Set" ) )
    , mStandardDeviations( 0 )
    , mDataProvider( 0 )
    , mWidth( std::numeric_limits<int>::max() )
    , mHeight( std::numeric_limits<int>::max() )
{
  init();
  mValid = false;
}

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
{
  QgsDebugMsg( "Entered" );

  // TODO, call constructor with provider key for now
  init();
  setDataProvider( "gdal" );

  if ( mValid && loadDefaultStyleFlag )
  {
    bool defaultLoadedFlag = false;
    loadDefaultStyle( defaultLoadedFlag );
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
    , mStandardDeviations( 0 )
    , mDataProvider( 0 )
    , mEditable( false )
    , mWidth( std::numeric_limits<int>::max() )
    , mHeight( std::numeric_limits<int>::max() )
    , mModified( false )
    , mProviderKey( providerKey )
{
  QgsDebugMsg( "Entered" );
  init();
  setDataProvider( providerKey );

  // load default style if provider is gdal and if no style was given
  // this should be an argument like in the other constructor
  if ( mValid && providerKey == "gdal" && loadDefaultStyleFlag )
  {
    bool defaultLoadedFlag = false;
    loadDefaultStyle( defaultLoadedFlag );
  }

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

  emit statusChanged( tr( "QgsRasterLayer created" ) );
} // QgsRasterLayer ctor

QgsRasterLayer::~QgsRasterLayer()
{
  mValid = false;
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
#if 0
  if ( theBandNo <= mRasterStatsList.size() && theBandNo > 0 )
  {
    //vector starts at base 0, band counts at base1!
    return mRasterStatsList[theBandNo - 1].bandName;
  }
  else
  {
    return QString( "" );
  }
#endif
  return dataProvider()->generateBandName( theBandNo );
}

int QgsRasterLayer::bandNumber( QString const & theBandName ) const
{
  if ( !mDataProvider ) return 0;
  for ( int myIterator = 1; myIterator <= dataProvider()->bandCount(); ++myIterator )
  {
    //find out the name of this band
#if 0
    QgsRasterBandStats myRasterBandStats = mRasterStatsList[myIterator];
    QgsDebugMsg( "myRasterBandStats.bandName: " + myRasterBandStats.bandName + "  :: theBandName: "
                 + theBandName );

    if ( myRasterBandStats.bandName == theBandName )
#endif
      QString myBandName =  dataProvider()->generateBandName( myIterator );
    if ( myBandName == theBandName )
    {
      QgsDebugMsg( "********** band " + QString::number( myIterator ) +
                   " was found in bandNumber " + theBandName );

      return myIterator;
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
/*
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

  // TODO this is buggy - because the stats might have changed (e.g. theIgnoreOutOfRangeFlag in populateHistogram())
  // should have a pointer to the stats instead
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
*/

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
    // Was there any reason to use float for myMin, myMax, myValue?
    // It was breaking Float64 data obviously, especially if an extreme value
    // was used for NoDataValue.
    double myMin = std::numeric_limits<double>::max();
    double myMax = -1 * std::numeric_limits<double>::max();
    double myValue = 0.0;
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

void QgsRasterLayer::setRendererForDrawingStyle( const DrawingStyle &  theDrawingStyle )
{
  setRenderer( QgsRasterRendererRegistry::instance()->defaultRendererForDrawingStyle( theDrawingStyle, mDataProvider ) );
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
 * @return ointer to the color table
 */
QList<QgsColorRampShader::ColorRampItem> QgsRasterLayer::colorTable( int theBandNo )
{
  //return &( mRasterStatsList[theBandNo-1].colorTable );
  if ( !mDataProvider ) return QList<QgsColorRampShader::ColorRampItem>();
  return dataProvider()->colorTable( theBandNo );
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
      myProjectedLayerExtent = rendererContext.coordinateTransform()->transformBoundingBox( mLayerExtent );
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

  QgsRasterProjector *projector = mPipe.projector();

  // TODO add a method to interface to get provider and get provider
  // params in QgsRasterProjector
  if ( projector )
  {
    projector->setCRS( theRasterViewPort->mSrcCRS, theRasterViewPort->mDestCRS );
  }

#ifdef QGISDEBUG
  // Collect stats only for larger sizes to avoid confusion (small time values)
  // after thumbnail render e.g. 120 is current thumbnail size
  // TODO: consider another way to switch stats on/off or storing of last significant
  //       stats somehow
  if ( theRasterViewPort->drawableAreaXDim > 120 && theRasterViewPort->drawableAreaYDim > 120 )
  {
    mPipe.setStatsOn( true );
  }
#endif

  // Drawer to pipe?
  QgsRasterIterator iterator( mPipe.last() );
  QgsRasterDrawer drawer( &iterator );
  drawer.draw( theQPainter, theRasterViewPort, theQgsMapToPixel );

#ifdef QGISDEBUG
  mPipe.setStatsOn( false );
  // Print time stats
  QgsDebugMsg( QString( "interface                  bands  time" ) );
  for ( int i = 0; i < mPipe.size(); i++ )
  {
    QgsRasterInterface * interface = mPipe.at( i );
    QString name = QString( typeid( *interface ).name() ).replace( QRegExp( ".*Qgs" ), "Qgs" ).left( 30 );
    QgsDebugMsg( QString( "%1 %2 %3" ).arg( name, -30 ).arg( interface->bandCount() ).arg( interface->time(), 5 ) );
  }
  QgsDebugMsg( QString( "total raster draw time (ms):     %1" ).arg( time.elapsed(), 5 ) );
#endif
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

#if 0
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
#endif

/**
 * @param thePoint the QgsPoint for which to obtain pixel values
 * @param theResults QMap to hold the pixel values at thePoint for each layer in the raster file
 * @return False if WMS layer and true otherwise
 */
bool QgsRasterLayer::identify( const QgsPoint& thePoint, QMap<QString, QString>& theResults )
{
  theResults.clear();
  // QgsDebugMsg( "identify provider : " + mProviderKey ) ;
  return ( mDataProvider->identify( thePoint, theResults ) );
}

bool QgsRasterLayer::identify( const QgsPoint & point, QMap<int, QString>& results )
{
  if ( !mDataProvider )
  {
    return false;
  }

  results.clear();
  return mDataProvider->identify( point, results );
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
  //if ( mPipe.renderer() )
  //{
  //  mPipe.renderer()->legendSymbologyItems( symbolList );
  //}
  QgsRasterRenderer *renderer = mPipe.renderer();
  if ( renderer )
  {
    renderer->legendSymbologyItems( symbolList );
  }
  return symbolList;

#if 0
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
#endif //0
}

/**
 * This is an overloaded version of the legendAsPixmap( bool ) assumes false for the legend name flag.
 * @return a pixmap representing a legend image
 */
QPixmap QgsRasterLayer::legendAsPixmap()
{
  return QPixmap();
}

/**
 * @param theWithNameFlag - boolean flag whether to overlay the legend name in the text
 * @return a pixmap representing a legend image
 */
QPixmap QgsRasterLayer::legendAsPixmap( bool theWithNameFlag )
{
  Q_UNUSED( theWithNameFlag );
  return QPixmap();
}                               //end of legendAsPixmap function

/**
 * \param theLabelCount number of vertical labels to display
 * @return a pixmap representing a legend image
 */
QPixmap QgsRasterLayer::legendAsPixmap( int )
{
  return QPixmap();
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
  myMetadata += crs().toProj4();
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

  setDrawingStyle( QgsRasterLayer::UndefinedDrawingStyle );

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
    QgsMessageLog::logMessage( tr( "Cannot instantiate the data provider" ), tr( "Raster" ) );
    return NULL;
  }
  QgsDebugMsg( "Data driver created" );
  return myDataProvider;
}

/** Copied from QgsVectorLayer::setDataProvider
 *  TODO: Make it work in the raster environment
 */
void QgsRasterLayer::setDataProvider( QString const & provider )
{
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

  mDataProvider = QgsRasterLayer::loadProvider( mProviderKey, mDataSource );
  if ( !mDataProvider )
  {
    return;
  }
  if ( !mDataProvider->isValid() )
  {
    return;
  }
  mPipe.set( mDataProvider );

  if ( provider == "gdal" )
  {
    // make sure that the /vsigzip or /vsizip is added to uri, if applicable
    mDataSource = mDataProvider->dataSourceUri();
  }

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

  mValidNoDataValue = mDataProvider->isNoDataValueValid();
  if ( mValidNoDataValue )
  {
  }

  // set up the raster drawing style
  setDrawingStyle( MultiBandColor );  //sensible default

  // Setup source CRS
  setCrs( QgsCoordinateReferenceSystem( mDataProvider->crs() ) );

  QString mySourceWkt = crs().toWkt();

  QgsDebugMsg( "using wkt:\n" + mySourceWkt );

  mBandCount = mDataProvider->bandCount( );
  for ( int i = 1; i <= mBandCount; i++ )
  {
#if 0
    QgsRasterBandStats myRasterBandStats;
    myRasterBandStats.bandName = mDataProvider->generateBandName( i );
    myRasterBandStats.bandNumber = i;
    myRasterBandStats.statsGathered = false;
    //Store the default color table
    //readColorTable( i, &myRasterBandStats.colorTable );
    QList<QgsColorRampShader::ColorRampItem> ct;
    ct = mDataProvider->colorTable( i );
    myRasterBandStats.colorTable = ct;

    mRasterStatsList.push_back( myRasterBandStats );
#endif

    //Build a new contrast enhancement for the band and store in list
    //QgsContrastEnhancement myContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )mDataProvider->dataType( i ) );
    QgsContrastEnhancement myContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )mDataProvider->srcDataType( i ) );
    mContrastEnhancementList.append( myContrastEnhancement );
  }

  //defaults - Needs to be set after the Contrast list has been build
  //Try to read the default contrast enhancement from the config file

  QSettings myQSettings;
  //setContrastEnhancementAlgorithm( myQSettings.value( "/Raster/defaultContrastEnhancementAlgorithm", "NoEnhancement" ).toString() );

  //decide what type of layer this is...
  //TODO Change this to look at the color interp and palette interp to decide which type of layer it is
  QgsDebugMsg( "bandCount = " + QString::number( mDataProvider->bandCount() ) );
  QgsDebugMsg( "dataType = " + QString::number( mDataProvider->dataType( 1 ) ) );
  if (( mDataProvider->bandCount() > 1 ) )
  {
    mRasterType = Multiband;
  }
  else if ( mDataProvider->dataType( 1 ) == QgsRasterDataProvider::ARGB32
            ||  mDataProvider->dataType( 1 ) == QgsRasterDataProvider::ARGB32_Premultiplied )
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
    setDrawingStyle( SingleBandColorDataStyle );
  }
  else if ( mRasterType == Palette )
  {

    setDrawingStyle( PalettedColor ); //sensible default
  }
  else if ( mRasterType == Multiband )
  {
    setDrawingStyle( MultiBandColor );  //sensible default
  }
  else                        //GrayOrUndefined
  {
    setDrawingStyle( SingleBandGray );  //sensible default
  }

  //resampler (must be after renderer)
  QgsRasterResampleFilter * resampleFilter = new QgsRasterResampleFilter();
  mPipe.set( resampleFilter );

  // projector (may be anywhere in pipe)
  QgsRasterProjector * projector = new QgsRasterProjector;
  mPipe.set( projector );

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

  //mRasterStatsList.clear();
  mContrastEnhancementList.clear();

  mHasPyramids = false;
  mPyramidList.clear();
}

void QgsRasterLayer::setColorShadingAlgorithm( ColorShadingAlgorithm )
{
  //legacy method
}

void QgsRasterLayer::setColorShadingAlgorithm( QString )
{
  //legacy method
}

void QgsRasterLayer::setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm theAlgorithm, bool theGenerateLookupTableFlag )
{
  setContrastEnhancementAlgorithm( theAlgorithm, ContrastEnhancementMinMax, QgsRectangle(), SAMPLE_SIZE, theGenerateLookupTableFlag );
}

void QgsRasterLayer::setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm theAlgorithm, ContrastEnhancementLimits theLimits, QgsRectangle theExtent, int theSampleSize, bool theGenerateLookupTableFlag )
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

  foreach( int myBand, myBands )
  {
    if ( myBand != -1 )
    {
      QgsRasterDataProvider::DataType myType = ( QgsRasterDataProvider::DataType )mDataProvider->dataType( myBand );
      QgsContrastEnhancement* myEnhancement = new QgsContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )myType );
      myEnhancement->setContrastEnhancementAlgorithm( theAlgorithm, theGenerateLookupTableFlag );

      double myMin = std::numeric_limits<double>::quiet_NaN();
      double myMax = std::numeric_limits<double>::quiet_NaN();

      if ( theLimits == ContrastEnhancementMinMax )
      {
        // minimumValue/maximumValue are not well defined (estimation) and will be removed
        //myMin = mDataProvider->minimumValue( myBand );
        //myMax = mDataProvider->maximumValue( myBand );
        QgsRasterBandStats myRasterBandStats = mDataProvider->bandStatistics( myBand, QgsRasterBandStats::Min | QgsRasterBandStats::Max, theExtent, theSampleSize );
        myMin = myRasterBandStats.minimumValue;
        myMax = myRasterBandStats.maximumValue;
      }
      else if ( theLimits == ContrastEnhancementStdDev )
      {
        double myStdDev = 1; // make optional?
        QgsRasterBandStats myRasterBandStats = mDataProvider->bandStatistics( myBand, QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev, theExtent, theSampleSize );
        myMin = myRasterBandStats.mean - ( myStdDev * myRasterBandStats.stdDev );
        myMax = myRasterBandStats.mean + ( myStdDev * myRasterBandStats.stdDev );
      }
      else if ( theLimits == ContrastEnhancementCumulativeCut )
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

void QgsRasterLayer::setGrayBandName( QString const & )
{
  //legacy method
}

void QgsRasterLayer::setGreenBandName( QString const & )
{
  //legacy method
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
  //legacy method
}

void QgsRasterLayer::setMinimumMaximumUsingDataset()
{
  //legacy method
}

void QgsRasterLayer::setMinimumValue( unsigned int, double, bool )
{
  //legacy method
}

void QgsRasterLayer::setMinimumValue( QString, double, bool )
{
  //legacy method
}

void QgsRasterLayer::setNoDataValue( double theNoDataValue )
{
  if ( theNoDataValue != mNoDataValue )
  {
    mNoDataValue = theNoDataValue;
    mValidNoDataValue = true;
    //Basically set the raster stats as invalid
    // TODO! No data value must be set on provider and stats cleared
#if 0
    QList<QgsRasterBandStats>::iterator myIterator = mRasterStatsList.begin();
    while ( myIterator !=  mRasterStatsList.end() )
    {
      ( *myIterator ).statsGathered = false;
      ++myIterator;
    }
#endif
  }
}

void QgsRasterLayer::setRasterShaderFunction( QgsRasterShaderFunction* )
{
  //legacy method
}

void QgsRasterLayer::setRedBandName( QString const & )
{
  //legacy method
}

void QgsRasterLayer::setSubLayerVisibility( QString name, bool vis )
{

  if ( mDataProvider )
  {
    QgsDebugMsg( "About to mDataProvider->setSubLayerVisibility(name, vis)." );
    mDataProvider->setSubLayerVisibility( name, vis );
  }

}

void QgsRasterLayer::setTransparentBandName( QString const & )
{
  //legacy method
}

void QgsRasterLayer::setRenderer( QgsRasterRenderer* theRenderer )
{
  QgsDebugMsg( "Entered" );
  if ( !theRenderer ) { return; }
  mPipe.set( theRenderer );
}

// not sure if we want it
/*
void QgsRasterLayer::setResampleFilter( QgsRasterResampleFilter* resampleFilter )
{
  QgsDebugMsg( "Entered" );
  if ( !resampleFilter ) { return; }
  if ( !mPipe.set( resampleFilter ) )
  {
    // TODO: somehow notify (and delete?)
    QgsDebugMsg( "Cannot set resample filter." );
  }
}
*/
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
  QDomElement rasterRendererElem;

  //rasterlayerproperties element there -> old format
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
    rasterRendererElem = layer_node.firstChildElement( "rasterrenderer" );
  }

  if ( !rasterRendererElem.isNull() )
  {
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
  }

  //resampler
  QgsRasterResampleFilter * resampleFilter = new QgsRasterResampleFilter();
  mPipe.set( resampleFilter );

  //max oversampling
  QDomElement resampleElem = layer_node.firstChildElement( "rasterresampler" );
  if ( !resampleElem.isNull() )
  {
    resampleFilter->readXML( resampleElem );
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

  QString theError;
  bool res = readSymbology( layer_node, theError );

  // old wms settings we need to correct
  if ( res &&
       mProviderKey == "wms" &&
       mDrawingStyle == MultiBandColor )
  {
    mDrawingStyle = SingleBandColorDataStyle;
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
      setDataProvider( mProviderKey );
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

  QgsRasterRenderer *renderer = mPipe.renderer();
  if ( renderer )
  {
    renderer->writeXML( document, layerElem );
  }

  QgsRasterResampleFilter *resampleFilter = mPipe.resampleFilter();
  if ( resampleFilter )
  {
    QDomElement layerElem = layer_node.toElement();
    resampleFilter->writeXML( document, layerElem );
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
  //TODO: This function is no longer really needed and about be removed
  //-- it is only used to see if "Palette" exists which is not the correct way to see if a band is paletted or not
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
    mDataProvider->readBlock( bandNo, partExtent,
                              viewPort->drawableAreaXDim,
                              viewPort->drawableAreaYDim,
                              viewPort->mSrcCRS,
                              viewPort->mDestCRS, data );
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
    setDataProvider( mProviderKey );
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

  if ( !mDataProvider ) return TRSTRING_NOT_SET;

  //check that a valid band name was passed
  QgsDebugMsg( "Looking through raster band stats for matching band name" );
  for ( int myIterator = 1; myIterator < mDataProvider->bandCount(); ++myIterator )
  {
    //find out the name of this band
    if ( mDataProvider->generateBandName( myIterator ) == theBandName );
    {
      QgsDebugMsg( "Matching band name found" );
      return theBandName;
    }
  }
  QgsDebugMsg( "No matching band name found in raster band stats" );

#if 0
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
#endif

  //if no matches were found default to not set
  QgsDebugMsg( "All checks failed, returning '" + QSTRING_NOT_SET + "'" );
  return TRSTRING_NOT_SET;
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

