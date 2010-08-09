/* **************************************************************************
        qgsrasterlayer.cpp -  description
   -------------------
begin                : Sat Jun 22 2002
copyright            : (C) 2003 by Tim Sutton, Steve Halasz and Gary E.Sherman
email                : tim at linfiniti.com
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsproviderregistry.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpyramid.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgscoordinatereferencesystem.h"

#include "gdalwarper.h"
#include "cpl_conv.h"

#include "qgspseudocolorshader.h"
#include "qgsfreakoutshader.h"
#include "qgscolorrampshader.h"

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
#include "qgslogger.h"
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
    : QgsMapLayer( RasterLayer, baseName, path ),
    // Constant that signals property not used.
    QSTRING_NOT_SET( "Not Set" ),
    TRSTRING_NOT_SET( tr( "Not Set" ) ),
    mStandardDeviations( 0 ),
    mDataProvider( 0 ),
    mWidth( std::numeric_limits<int>::max() ),
    mHeight( std::numeric_limits<int>::max() ),
    mInvertColor( false )
{

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

  mGdalBaseDataset = 0;
  mGdalDataset = 0;

  // Initialise the affine transform matrix
  mGeoTransform[0] =  0;
  mGeoTransform[1] =  1;
  mGeoTransform[2] =  0;
  mGeoTransform[3] =  0;
  mGeoTransform[4] =  0;
  mGeoTransform[5] = -1;

  // set the layer name (uppercase first character)

  if ( ! baseName.isEmpty() )   // XXX shouldn't this happen in parent?
  {
    setLayerName( baseName );
  }

  // load the file if one specified
  if ( ! path.isEmpty() )
  {
    readFile( path ); // XXX check for failure?

    //readFile() is really an extension of the constructor as many imporant fields are set in this method
    //loadDefaultStyle() can not be called before the layer has actually be opened
    if ( loadDefaultStyleFlag )
    {
      bool defaultLoadedFlag = false;
      loadDefaultStyle( defaultLoadedFlag );
      if ( defaultLoadedFlag )
      {
        return;
      }
    }
  }

  //Initialize the last view port structure, should really be a class
  mLastViewPort.rectXOffset = 0;
  mLastViewPort.rectXOffsetFloat = 0.0;
  mLastViewPort.rectYOffset = 0;
  mLastViewPort.rectYOffsetFloat = 0.0;
  mLastViewPort.clippedXMin = 0.0;
  mLastViewPort.clippedXMax = 0.0;
  mLastViewPort.clippedYMin = 0.0;
  mLastViewPort.clippedYMax = 0.0;
  mLastViewPort.clippedWidth = 0;
  mLastViewPort.clippedHeight = 0;
  mLastViewPort.drawableAreaXDim = 0;
  mLastViewPort.drawableAreaYDim = 0;

} // QgsRasterLayer ctor

/**
 * TODO Rename into a general constructor when the old raster interface is retired
 * @param  dummy  is just there to distinguish this function signature from the old non-provider one.
 */
QgsRasterLayer::QgsRasterLayer( int dummy,
                                QString const & rasterLayerPath,
                                QString const & baseName,
                                QString const & providerKey,
                                QStringList const & layers,
                                QStringList const & styles,
                                QString const & format,
                                QString const & crs )
    : QgsMapLayer( RasterLayer, baseName, rasterLayerPath ),
    mStandardDeviations( 0 ),
    mDataProvider( 0 ),
    mEditable( false ),
    mWidth( std::numeric_limits<int>::max() ),
    mHeight( std::numeric_limits<int>::max() ),
    mInvertColor( false ),
    mModified( false ),
    mProviderKey( providerKey )
{
  QgsDebugMsg( "(8 arguments) starting. with layer list of " +
               layers.join( ", " ) +  " and style list of " + styles.join( ", " ) + " and format of " +
               format +  " and CRS of " + crs );

  mBandCount = 0;
  mRasterShader = new QgsRasterShader();

  // Initialise the affine transform matrix
  mGeoTransform[0] =  0;
  mGeoTransform[1] =  1;
  mGeoTransform[2] =  0;
  mGeoTransform[3] =  0;
  mGeoTransform[4] =  0;
  mGeoTransform[5] = -1;

  // if we're given a provider type, try to create and bind one to this layer
  if ( ! providerKey.isEmpty() )
  {
    setDataProvider( providerKey, layers, styles, format, crs );
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

  // Do a passthrough for the status bar text
#if 0
  connect(
    mDataProvider, SIGNAL( statusChanged( QString ) ),
    this,           SLOT( showStatusMessage( QString ) )
  );
#endif
  QgsDebugMsg( "(8 arguments) exiting." );

  emit statusChanged( tr( "QgsRasterLayer created" ) );
} // QgsRasterLayer ctor

QgsRasterLayer::~QgsRasterLayer()
{

  if ( mProviderKey.isEmpty() )
  {
    if ( mGdalBaseDataset )
    {
      GDALDereferenceDataset( mGdalBaseDataset );
    }

    if ( mGdalDataset )
    {
      GDALClose( mGdalDataset );
    }
  }
  delete mRasterShader;
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
  // first get the GDAL driver manager
  registerGdalDrivers();

  // then iterate through all of the supported drivers, adding the
  // corresponding file filter

  GDALDriverH myGdalDriver;           // current driver

  char **myGdalDriverMetadata;        // driver metadata strings

  QString myGdalDriverLongName( "" ); // long name for the given driver
  QString myGdalDriverExtension( "" );  // file name extension for given driver
  QString myGdalDriverDescription;    // QString wrapper of GDAL driver description

  QStringList metadataTokens;   // essentially the metadata string delimited by '='

  QStringList catchallFilter;   // for Any file(*.*), but also for those
  // drivers with no specific file filter

  GDALDriverH jp2Driver = NULL; // first JPEG2000 driver found

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, well, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.
  // Note that file name extension strings are of the form
  // "DMD_EXTENSION=.*".  We'll also store the long name of the
  // driver, which will be found in DMD_LONGNAME, which will have the
  // same form.

  // start with the default case
  theFileFiltersString = tr( "[GDAL] All files (*)" );

  for ( int i = 0; i < GDALGetDriverCount(); ++i )
  {
    myGdalDriver = GDALGetDriver( i );

    Q_CHECK_PTR( myGdalDriver );

    if ( !myGdalDriver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }
    // now we need to see if the driver is for something currently
    // supported; if not, we give it a miss for the next driver

    myGdalDriverDescription = GDALGetDescription( myGdalDriver );

    // QgsDebugMsg(QString("got driver string %1").arg(myGdalDriverDescription));

    myGdalDriverMetadata = GDALGetMetadata( myGdalDriver, NULL );

    // presumably we know we've run out of metadta if either the
    // address is 0, or the first character is null
    while ( myGdalDriverMetadata && '\0' != myGdalDriverMetadata[0] )
    {
      metadataTokens = QString( *myGdalDriverMetadata ).split( "=", QString::SkipEmptyParts );
      // QgsDebugMsg(QString("\t%1").arg(*myGdalDriverMetadata));

      // XXX add check for malformed metadataTokens

      // Note that it's oddly possible for there to be a
      // DMD_EXTENSION with no corresponding defined extension
      // string; so we check that there're more than two tokens.

      if ( metadataTokens.count() > 1 )
      {
        if ( "DMD_EXTENSION" == metadataTokens[0] )
        {
          myGdalDriverExtension = metadataTokens[1];

        }
        else if ( "DMD_LONGNAME" == metadataTokens[0] )
        {
          myGdalDriverLongName = metadataTokens[1];

          // remove any superfluous (.*) strings at the end as
          // they'll confuse QFileDialog::getOpenFileNames()

          myGdalDriverLongName.remove( QRegExp( "\\(.*\\)$" ) );
        }
      }
      // if we have both the file name extension and the long name,
      // then we've all the information we need for the current
      // driver; therefore emit a file filter string and move to
      // the next driver
      if ( !( myGdalDriverExtension.isEmpty() || myGdalDriverLongName.isEmpty() ) )
      {
        // XXX add check for SDTS; in that case we want (*CATD.DDF)
        QString glob = "*." + myGdalDriverExtension.replace( "/", " *." );
        // Add only the first JP2 driver found to the filter list (it's the one GDAL uses)
        if ( myGdalDriverDescription == "JPEG2000" ||
             myGdalDriverDescription.startsWith( "JP2" ) ) // JP2ECW, JP2KAK, JP2MrSID
        {
          if ( jp2Driver )
            break; // skip if already found a JP2 driver

          jp2Driver = myGdalDriver;   // first JP2 driver found
          glob += " *.j2k";           // add alternate extension
        }
        else if ( myGdalDriverDescription == "GTiff" )
        {
          glob += " *.tiff";
        }
        else if ( myGdalDriverDescription == "JPEG" )
        {
          glob += " *.jpeg";
        }

        theFileFiltersString += ";;[GDAL] " + myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ")";

        break;            // ... to next driver, if any.
      }

      ++myGdalDriverMetadata;

    }                       // each metadata item

    if ( myGdalDriverExtension.isEmpty() && !myGdalDriverLongName.isEmpty() )
    {
      // Then what we have here is a driver with no corresponding
      // file extension; e.g., GRASS.  In which case we append the
      // string to the "catch-all" which will match all file types.
      // (I.e., "*.*") We use the driver description intead of the
      // long time to prevent the catch-all line from getting too
      // large.

      // ... OTOH, there are some drivers with missing
      // DMD_EXTENSION; so let's check for them here and handle
      // them appropriately

      // USGS DEMs use "*.dem"
      if ( myGdalDriverDescription.startsWith( "USGSDEM" ) )
      {
        QString glob = "*.dem";
        theFileFiltersString += ";;[GDAL] " + myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ")";
      }
      else if ( myGdalDriverDescription.startsWith( "DTED" ) )
      {
        // DTED use "*.dt0, *.dt1, *.dt2"
        QString glob = "*.dt0";
        glob += " *.dt1";
        glob += " *.dt2";
        theFileFiltersString += ";;[GDAL] " + myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ")";
      }
      else if ( myGdalDriverDescription.startsWith( "MrSID" ) )
      {
        // MrSID use "*.sid"
        QString glob = "*.sid";
        theFileFiltersString += ";;[GDAL] " + myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ")";
      }
      else
      {
        catchallFilter << QString( GDALGetDescription( myGdalDriver ) );
      }
    }

    myGdalDriverExtension = myGdalDriverLongName = "";  // reset for next driver

  }                           // each loaded GDAL driver

  QgsDebugMsg( "Raster filter list built: " + theFileFiltersString );
}                               // buildSupportedRasterFileFilter_()

/**
 * This helper checks to see whether the file name appears to be a valid raster file name
 */
bool QgsRasterLayer::isValidRasterFileName( QString const & theFileNameQString,
    QString & retErrMsg )
{

  GDALDatasetH myDataset;
  registerGdalDrivers();

  CPLErrorReset();

  //open the file using gdal making sure we have handled locale properly
  myDataset = GDALOpen( QFile::encodeName( theFileNameQString ).constData(), GA_ReadOnly );
  if ( myDataset == NULL )
  {
    if ( CPLGetLastErrorNo() != CPLE_OpenFailed )
      retErrMsg = QString::fromUtf8( CPLGetLastErrorMsg() );
    return false;
  }
  else if ( GDALGetRasterCount( myDataset ) == 0 )
  {
    GDALClose( myDataset );
    myDataset = NULL;
    retErrMsg = tr( "This raster file has no bands and is invalid as a raster layer." );
    return false;
  }
  else
  {
    GDALClose( myDataset );
    return true;
  }
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
  if ( !fi.exists() ) return t;

  t = fi.lastModified();

  QgsDebugMsg( "last modified = " + t.toString() );

  return t;
}



void QgsRasterLayer::registerGdalDrivers()
{
  if ( GDALGetDriverCount() == 0 )
    GDALAllRegister();
}




//////////////////////////////////////////////////////////
//
//Random Static convenience function
//
/////////////////////////////////////////////////////////
//TODO: Change these to private function or make seprate class
// convenience function for building metadata() HTML table cells
static QString makeTableCell_( QString const & value )
{
  return "<p>\n" + value + "</p>\n";
} // makeTableCell_



// convenience function for building metadata() HTML table cells
static QString makeTableCells_( QStringList const & values )
{
  QString s( "<tr>" );

  for ( QStringList::const_iterator i = values.begin();
        i != values.end();
        ++i )
  {
    s += makeTableCell_( *i );
  }

  s += "</tr>";

  return s;
} // makeTableCell_



// convenience function for creating a string list from a C style string list
static QStringList cStringList2Q_( char ** stringList )
{
  QStringList strings;

  // presume null terminated string list
  for ( size_t i = 0; stringList[i]; ++i )
  {
    strings.append( stringList[i] );
  }

  return strings;

} // cStringList2Q_


// typedef for the QgsDataProvider class factory
typedef QgsDataProvider * classFactoryFunction_t( const QString * );


//
// global callback function
//
int CPL_STDCALL progressCallback( double dfComplete,
                                  const char * pszMessage,
                                  void * pProgressArg )
{
  static double dfLastComplete = -1.0;

  QgsRasterLayer * mypLayer = ( QgsRasterLayer * ) pProgressArg;

  if ( dfLastComplete > dfComplete )
  {
    if ( dfLastComplete >= 1.0 )
      dfLastComplete = -1.0;
    else
      dfLastComplete = dfComplete;
  }

  if ( floor( dfLastComplete*10 ) != floor( dfComplete*10 ) )
  {
    int    nPercent = ( int ) floor( dfComplete * 100 );

    if ( nPercent == 0 && pszMessage != NULL )
    {
      //fprintf( stdout, "%s:", pszMessage );
    }

    if ( nPercent == 100 )
    {
      //fprintf( stdout, "%d - done.\n", (int) floor(dfComplete*100) );
      mypLayer->showProgress( 100 );
    }
    else
    {
      int myProgress = ( int ) floor( dfComplete * 100 );
      //fprintf( stdout, "%d.", myProgress);
      mypLayer->showProgress( myProgress );
      //fflush( stdout );
    }
  }
  dfLastComplete = dfComplete;

  return true;
}




//////////////////////////////////////////////////////////
//
// Non Static Public methods
//
/////////////////////////////////////////////////////////

unsigned int QgsRasterLayer::bandCount()
{
  return mBandCount;
}

const QString QgsRasterLayer::bandName( int theBandNo )
{
  if ( theBandNo <= mRasterStatsList.size() && theBandNo > 0 )
  {
    //vector starts at base 0, band counts at base1 !
    return mRasterStatsList[theBandNo - 1].bandName;
  }
  else
  {
    return QString( "" );
  }
}

int QgsRasterLayer::bandNumber( QString const & theBandName )
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
 * @seealso RasterBandStats
 * @note This is a cpu intensive and slow task!
 */
const QgsRasterBandStats QgsRasterLayer::bandStatistics( int theBandNo )
{
  // check if we have received a valid band number
  if (( GDALGetRasterCount( mGdalDataset ) < theBandNo ) && mRasterType != Palette )
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
  // only print message if we are actually gathering the stats
  emit statusChanged( tr( "Retrieving stats for %1" ).arg( name() ) );
  qApp->processEvents();
  QgsDebugMsg( "stats for band " + QString::number( theBandNo ) );
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );


  QString myColorerpretation = GDALGetColorInterpretationName( GDALGetRasterColorInterpretation( myGdalBand ) );

  // XXX this sets the element count to a sensible value; but then you ADD to
  // XXX it later while iterating through all the pixels?
  //myRasterBandStats.elementCount = mWidth * mHeight;

  myRasterBandStats.elementCount = 0; // because we'll be counting only VALID pixels later

  emit statusChanged( tr( "Calculating stats for %1" ).arg( name() ) );
  //reset the main app progress bar
  emit drawingProgress( 0, 0 );

  // let the user know we're going to possibly be taking a while
  //QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );

  int  myNXBlocks, myNYBlocks, myXBlockSize, myYBlockSize;
  GDALGetBlockSize( myGdalBand, &myXBlockSize, &myYBlockSize );

  myNXBlocks = ( GDALGetRasterXSize( myGdalBand ) + myXBlockSize - 1 ) / myXBlockSize;
  myNYBlocks = ( GDALGetRasterYSize( myGdalBand ) + myYBlockSize - 1 ) / myYBlockSize;

  void *myData = CPLMalloc( myXBlockSize * myYBlockSize * ( GDALGetDataTypeSize( myDataType ) / 8 ) );

  // unfortunately we need to make two passes through the data to calculate stddev
  bool myFirstIterationFlag = true;

  //ifdefs below to remove compiler warning about unused vars
#ifdef QGISDEBUG
  int success;
  double GDALminimum = GDALGetRasterMinimum( myGdalBand, &success );

  if ( ! success )
  {
    QgsDebugMsg( "myGdalBand->GetMinimum() failed" );
  }

  double GDALmaximum = GDALGetRasterMaximum( myGdalBand, &success );

  if ( ! success )
  {
    QgsDebugMsg( "myGdalBand->GetMaximum() failed" );
  }

  double GDALnodata = GDALGetRasterNoDataValue( myGdalBand, &success );

  if ( ! success )
  {
    QgsDebugMsg( "myGdalBand->GetNoDataValue() failed" );
  }

  QgsLogger::debug( "GDALminium: ", GDALminimum, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "GDALmaximum: ", GDALmaximum, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "GDALnodata: ", GDALnodata, __FILE__, __FUNCTION__, __LINE__ );

  double GDALrange[2];          // calculated min/max, as opposed to the
  // dataset provided

  GDALComputeRasterMinMax( myGdalBand, 1, GDALrange );
  QgsLogger::debug( "approximate computed GDALminium:", GDALrange[0], __FILE__, __FUNCTION__, __LINE__, 1 );
  QgsLogger::debug( "approximate computed GDALmaximum:", GDALrange[1], __FILE__, __FUNCTION__, __LINE__, 1 );

  GDALComputeRasterMinMax( myGdalBand, 0, GDALrange );
  QgsLogger::debug( "exactly computed GDALminium:", GDALrange[0] );
  QgsLogger::debug( "exactly computed GDALmaximum:", GDALrange[1] );

  QgsDebugMsg( "starting manual stat computation" );
#endif

  int myGdalBandXSize = GDALGetRasterXSize( myGdalBand );
  int myGdalBandYSize = GDALGetRasterYSize( myGdalBand );
  for ( int iYBlock = 0; iYBlock < myNYBlocks; iYBlock++ )
  {
    emit drawingProgress( iYBlock, myNYBlocks * 2 );

    for ( int iXBlock = 0; iXBlock < myNXBlocks; iXBlock++ )
    {
      int  nXValid, nYValid;
      GDALReadBlock( myGdalBand, iXBlock, iYBlock, myData );

      // Compute the portion of the block that is valid
      // for partial edge blocks.
      if (( iXBlock + 1 ) * myXBlockSize > myGdalBandXSize )
        nXValid = myGdalBandXSize - iXBlock * myXBlockSize;
      else
        nXValid = myXBlockSize;

      if (( iYBlock + 1 ) * myYBlockSize > myGdalBandYSize )
        nYValid = myGdalBandYSize - iYBlock * myYBlockSize;
      else
        nYValid = myYBlockSize;

      // Collect the histogram counts.
      for ( int iY = 0; iY < nYValid; iY++ )
      {
        for ( int iX = 0; iX < nXValid; iX++ )
        {
          double myValue = readValue( myData, myDataType, iX + ( iY * myXBlockSize ) );

          if ( mValidNoDataValue && ( fabs( myValue - mNoDataValue ) <= TINY_VALUE || myValue != myValue ) )
          {
            continue; // NULL
          }

          //only use this element if we have a non null element
          if ( myFirstIterationFlag )
          {
            //this is the first iteration so initialise vars
            myFirstIterationFlag = false;
            myRasterBandStats.minimumValue = myValue;
            myRasterBandStats.maximumValue = myValue;
            ++myRasterBandStats.elementCount;
          }               //end of true part for first iteration check
          else
          {
            //this is done for all subsequent iterations
            if ( myValue < myRasterBandStats.minimumValue )
            {
              myRasterBandStats.minimumValue = myValue;
            }
            if ( myValue > myRasterBandStats.maximumValue )
            {
              myRasterBandStats.maximumValue = myValue;
            }

            myRasterBandStats.sum += myValue;
            ++myRasterBandStats.elementCount;
          }               //end of false part for first iteration check
        }
      }
    }                       //end of column wise loop
  }                           //end of row wise loop


  //end of first pass through data now calculate the range
  myRasterBandStats.range = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
  //calculate the mean
  myRasterBandStats.mean = myRasterBandStats.sum / myRasterBandStats.elementCount;

  //for the second pass we will get the sum of the squares / mean
  for ( int iYBlock = 0; iYBlock < myNYBlocks; iYBlock++ )
  {
    emit drawingProgress( iYBlock + myNYBlocks, myNYBlocks * 2 );

    for ( int iXBlock = 0; iXBlock < myNXBlocks; iXBlock++ )
    {
      int  nXValid, nYValid;

      GDALReadBlock( myGdalBand, iXBlock, iYBlock, myData );

      // Compute the portion of the block that is valid
      // for partial edge blocks.
      if (( iXBlock + 1 ) * myXBlockSize > myGdalBandXSize )
        nXValid = myGdalBandXSize - iXBlock * myXBlockSize;
      else
        nXValid = myXBlockSize;

      if (( iYBlock + 1 ) * myYBlockSize > myGdalBandYSize )
        nYValid = myGdalBandYSize - iYBlock * myYBlockSize;
      else
        nYValid = myYBlockSize;

      // Collect the histogram counts.
      for ( int iY = 0; iY < nYValid; iY++ )
      {
        for ( int iX = 0; iX < nXValid; iX++ )
        {
          double myValue = readValue( myData, myDataType, iX + ( iY * myXBlockSize ) );

          if ( mValidNoDataValue && ( fabs( myValue - mNoDataValue ) <= TINY_VALUE || myValue != myValue ) )
          {
            continue; // NULL
          }

          myRasterBandStats.sumOfSquares += static_cast < double >
                                            ( pow( myValue - myRasterBandStats.mean, 2 ) );
        }
      }
    }                       //end of column wise loop
  }                           //end of row wise loop

  //divide result by sample size - 1 and get square root to get stdev
  myRasterBandStats.stdDev = static_cast < double >( sqrt( myRasterBandStats.sumOfSquares /
                             ( myRasterBandStats.elementCount - 1 ) ) );

#ifdef QGISDEBUG
  QgsLogger::debug( "************ STATS **************", 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "VALID NODATA", mValidNoDataValue, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "NULL", mNoDataValue, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "MIN", myRasterBandStats.minimumValue, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "MAX", myRasterBandStats.maximumValue, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "RANGE", myRasterBandStats.range, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "MEAN", myRasterBandStats.mean, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "STDDEV", myRasterBandStats.stdDev, 1, __FILE__, __FUNCTION__, __LINE__ );
#endif

  CPLFree( myData );
  myRasterBandStats.statsGathered = true;

  QgsDebugMsg( "adding stats to stats collection at position " + QString::number( theBandNo - 1 ) );
  //add this band to the class stats map
  mRasterStatsList[theBandNo - 1] = myRasterBandStats;
  emit drawingProgress( mHeight, mHeight ); //reset progress
  //QApplication::restoreOverrideCursor(); //restore the cursor
  QgsDebugMsg( "Stats collection completed returning" );
  return myRasterBandStats;

} // QgsRasterLayer::bandStatistics

const QgsRasterBandStats QgsRasterLayer::bandStatistics( QString const & theBandName )
{

  //we cant use a vector iterator because the iterator is astruct not a class
  //and the qvector model does not like this.
  for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); i++ )
  {
    QgsRasterBandStats myRasterBandStats = bandStatistics( i );
    if ( myRasterBandStats.bandName == theBandName )
    {
      return myRasterBandStats;
    }
  }

  return QgsRasterBandStats();     // return a null one
}


/*
 * This will speed up performance at the expense of hard drive space.
 * Also, write access to the file is required for creating internal pyramids,
 * and to the directory in which the files exists if external
 * pyramids (.ovr) are to be created. If no parameter is passed in
 * it will default to nearest neighbor resampling.
 *
 * @param theTryInternalFlag - Try to make the pyramids internal if supported (e.g. geotiff). If not supported it will revert to creating external .ovr file anyway.
 * @return null string on success, otherwise a string specifying error
 */
QString QgsRasterLayer::buildPyramids( RasterPyramidList const & theRasterPyramidList,
                                       QString const & theResamplingMethod, bool theTryInternalFlag )
{
  //TODO: Consider making theRasterPyramidList modifyable by this method to indicate if the pyramid exists after build attempt
  //without requiring the user to rebuild the pyramid list to get the updated infomation

  //
  // Note: Make sure the raster is not opened in write mode
  // in order to force overviews to be written to a separate file.
  // Otherwise reoopen it in read/write mode to stick overviews
  // into the same file (if supported)
  //


  emit drawingProgress( 0, 0 );
  //first test if the file is writeable
  QFileInfo myQFile( mDataSource );

  if ( !myQFile.isWritable() )
  {
    return "ERROR_WRITE_ACCESS";
  }

  if ( mGdalDataset != mGdalBaseDataset )
  {
    QgsLogger::warning( "Pyramid building not currently supported for 'warped virtual dataset'." );
    return "ERROR_VIRTUAL";
  }

  if ( theTryInternalFlag )
  {
    // libtiff < 4.0 has a bug that prevents safe building of overviews on JPEG compressed files
    // we detect libtiff < 4.0 by checking that BIGTIFF is not in the creation options of the GTiff driver
    // see https://trac.osgeo.org/qgis/ticket/1357
    const char* pszGTiffCreationOptions =
      GDALGetMetadataItem( GDALGetDriverByName( "GTiff" ), GDAL_DMD_CREATIONOPTIONLIST, "" );
    if ( strstr( pszGTiffCreationOptions, "BIGTIFF" ) == NULL )
    {
      QString myCompressionType = QString( GDALGetMetadataItem( mGdalDataset, "COMPRESSION", "IMAGE_STRUCTURE" ) );
      if ( "JPEG" == myCompressionType )
      {
        return "ERROR_JPEG_COMPRESSION";
      }
    }

    //close the gdal dataset and reopen it in read / write mode
    GDALClose( mGdalDataset );
    mGdalBaseDataset = GDALOpen( QFile::encodeName( mDataSource ).constData(), GA_Update );

    // if the dataset couldn't be opened in read / write mode, tell the user
    if ( !mGdalBaseDataset )
    {
      mGdalBaseDataset = GDALOpen( QFile::encodeName( mDataSource ).constData(), GA_ReadOnly );
      //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
      mGdalDataset = mGdalBaseDataset;
      return "ERROR_WRITE_FORMAT";
    }
  }

  //
  // Iterate through the Raster Layer Pyramid Vector, building any pyramid
  // marked as exists in eaxh RasterPyramid struct.
  //
  CPLErr myError; //in case anything fails
  int myCount = 1;
  int myTotal = theRasterPyramidList.count();
  RasterPyramidList::const_iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator = theRasterPyramidList.begin();
        myRasterPyramidIterator != theRasterPyramidList.end();
        ++myRasterPyramidIterator )
  {
#ifdef QGISDEBUG
    QgsLogger::debug( "Build pyramids:: Level", ( *myRasterPyramidIterator ).level, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "x", ( *myRasterPyramidIterator ).xDim, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "y", ( *myRasterPyramidIterator ).yDim, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "exists :", ( *myRasterPyramidIterator ).exists,  1, __FILE__, __FUNCTION__, __LINE__ );
#endif
    if (( *myRasterPyramidIterator ).build )
    {
      QgsDebugMsg( "Building....." );
      emit drawingProgress( myCount, myTotal );
      int myOverviewLevelsArray[1] = {( *myRasterPyramidIterator ).level };
      /* From : http://remotesensing.org/gdal/classGDALDataset.html#a23
       * pszResampling : one of "NEAREST", "AVERAGE" or "MODE" controlling the downsampling method applied.
       * nOverviews : number of overviews to build.
       * panOverviewList : the list of overview decimation factors to build.
       * nBand : number of bands to build overviews for in panBandList. Build for all bands if this is 0.
       * panBandList : list of band numbers.
       * pfnProgress : a function to call to report progress, or NULL.
       * pProgressData : application data to pass to the progress function.
       */
      //build the pyramid and show progress to console
      try
      {

        //build the pyramid and show progress to console
        //NOTE this (magphase) is disabled in teh gui since it tends
        //to create corrupted images. The images can be repaired
        //by running one of the other resampling strategies below.
        //see ticket #284
        if ( theResamplingMethod == tr( "Average Magphase" ) )
        {
          myError = GDALBuildOverviews( mGdalBaseDataset, "MODE", 1, myOverviewLevelsArray, 0, NULL,
                                        progressCallback, this ); //this is the arg for the gdal progress callback
        }
        else if ( theResamplingMethod == tr( "Average" ) )

        {
          myError = GDALBuildOverviews( mGdalBaseDataset, "AVERAGE", 1, myOverviewLevelsArray, 0, NULL,
                                        progressCallback, this ); //this is the arg for the gdal progress callback
        }
        else // fall back to nearest neighbor
        {
          myError = GDALBuildOverviews( mGdalBaseDataset, "NEAREST", 1, myOverviewLevelsArray, 0, NULL,
                                        progressCallback, this ); //this is the arg for the gdal progress callback
        }

        if ( myError == CE_Failure || CPLGetLastErrorNo() == CPLE_NotSupported )
        {
          //something bad happenend
          //QString myString = QString (CPLGetLastError());
          GDALClose( mGdalBaseDataset );
          mGdalBaseDataset = GDALOpen( QFile::encodeName( mDataSource ).constData(), GA_ReadOnly );
          //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
          mGdalDataset = mGdalBaseDataset;

          emit drawingProgress( 0, 0 );
          return "FAILED_NOT_SUPPORTED";
        }
        else
        {
          //make sure the raster knows it has pyramids
          mHasPyramids = true;
        }
        myCount++;

      }
      catch ( CPLErr )
      {
        QgsLogger::warning( "Pyramid overview building failed!" );
      }
    }
  }

  QgsDebugMsg( "Pyramid overviews built" );
  if ( theTryInternalFlag )
  {
    //close the gdal dataset and reopen it in read only mode
    GDALClose( mGdalBaseDataset );
    mGdalBaseDataset = GDALOpen( QFile::encodeName( mDataSource ).constData(), GA_ReadOnly );
    //Since we are not a virtual warped dataset, mGdalDataSet and mGdalBaseDataset are supposed to be the same
    mGdalDataset = mGdalBaseDataset;
  }

  emit drawingProgress( 0, 0 );
  return NULL; // returning null on success
}


QgsRasterLayer::RasterPyramidList  QgsRasterLayer::buildPyramidList()
{
  //
  // First we build up a list of potential pyramid layers
  //
  int myWidth = mWidth;
  int myHeight = mHeight;
  int myDivisor = 2;

  if ( mDataProvider ) return mPyramidList;

  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band

  mPyramidList.clear();
  QgsDebugMsg( "Building initial pyramid list" );
  while (( myWidth / myDivisor > 32 ) && (( myHeight / myDivisor ) > 32 ) )
  {

    QgsRasterPyramid myRasterPyramid;
    myRasterPyramid.level = myDivisor;
    myRasterPyramid.xDim = ( int )( 0.5 + ( myWidth / ( double )myDivisor ) );
    myRasterPyramid.yDim = ( int )( 0.5 + ( myHeight / ( double )myDivisor ) );
    myRasterPyramid.exists = false;
#ifdef QGISDEBUG
    QgsLogger::debug( "Pyramid", myRasterPyramid.level, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "xDim", myRasterPyramid.xDim, 1, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug( "yDim", myRasterPyramid.yDim, 1, __FILE__, __FUNCTION__, __LINE__ );
#endif

    //
    // Now we check if it actually exists in the raster layer
    // and also adjust the dimensions if the dimensions calculated
    // above are only a near match.
    //
    const int myNearMatchLimit = 5;
    if ( GDALGetOverviewCount( myGDALBand ) > 0 )
    {
      int myOverviewCount;
      for ( myOverviewCount = 0;
            myOverviewCount < GDALGetOverviewCount( myGDALBand );
            ++myOverviewCount )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGDALBand, myOverviewCount );
        int myOverviewXDim = GDALGetRasterBandXSize( myOverview );
        int myOverviewYDim = GDALGetRasterBandYSize( myOverview );
        //
        // here is where we check if its a near match:
        // we will see if its within 5 cells either side of
        //
        QgsDebugMsg( "Checking whether " + QString::number( myRasterPyramid.xDim ) + " x " +
                     QString::number( myRasterPyramid.yDim ) + " matches " +
                     QString::number( myOverviewXDim ) + " x " + QString::number( myOverviewYDim ) );


        if (( myOverviewXDim <= ( myRasterPyramid.xDim + myNearMatchLimit ) ) &&
            ( myOverviewXDim >= ( myRasterPyramid.xDim - myNearMatchLimit ) ) &&
            ( myOverviewYDim <= ( myRasterPyramid.yDim + myNearMatchLimit ) ) &&
            ( myOverviewYDim >= ( myRasterPyramid.yDim - myNearMatchLimit ) ) )
        {
          //right we have a match so adjust the a / y before they get added to the list
          myRasterPyramid.xDim = myOverviewXDim;
          myRasterPyramid.yDim = myOverviewYDim;
          myRasterPyramid.exists = true;
          QgsDebugMsg( ".....YES!" );
        }
        else
        {
          //no match
          QgsDebugMsg( ".....no." );
        }
      }
    }
    mPyramidList.append( myRasterPyramid );
    //sqare the divisor each step
    myDivisor = ( myDivisor * 2 );
  }

  return mPyramidList;
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
  if ( 0 == theMinMax ) { return; }

  if ( 0 < theBand && theBand <= ( int ) bandCount() )
  {
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBand );
    GDALComputeRasterMinMax( myGdalBand, 1, theMinMax );
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
  if ( 0 == theMinMax ) { return; }

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBand );
  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );
  void* myGdalScanData = readData( myGdalBand, &mLastViewPort );

  /* Check for out of memory error */
  if ( myGdalScanData == NULL )
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
        myValue = readValue( myGdalScanData, myDataType, myRow * mLastViewPort.drawableAreaXDim + myColumn );
        if ( mValidNoDataValue && ( fabs( myValue - mNoDataValue ) <= TINY_VALUE || myValue != myValue ) )
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
  //preventwarnings
  if ( theOther.type() < 0 )
  {
    return false;
  }
  return false;
} //todo

/**
 * @param band number
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
  const QgsRectangle& theViewExtent = rendererContext.extent();
  QPainter* theQPainter = rendererContext.painter();

  if ( !theQPainter )
  {
    return false;
  }

  // clip raster extent to view extent
  QgsRectangle myRasterExtent = theViewExtent.intersect( &mLayerExtent );
  if ( myRasterExtent.isEmpty() )
  {
    QgsDebugMsg( "draw request outside view extent." );
    // nothing to do
    return true;
  }

  QgsDebugMsg( "theViewExtent is " + theViewExtent.toString() );
  QgsDebugMsg( "myRasterExtent is " + myRasterExtent.toString() );

  //
  // The first thing we do is set up the QgsRasterViewPort. This struct stores all the settings
  // relating to the size (in pixels and coordinate system units) of the raster part that is
  // in view in the map window. It also stores the origin.
  //
  //this is not a class level member because every time the user pans or zooms
  //the contents of the rasterViewPort will change
  QgsRasterViewPort *myRasterViewPort = new QgsRasterViewPort();


  // calculate raster pixel offsets from origin to clipped rect
  // we're only interested in positive offsets where the origin of the raster
  // is northwest of the origin of the view
  myRasterViewPort->rectXOffsetFloat = ( theViewExtent.xMinimum() - mLayerExtent.xMinimum() ) / fabs( mGeoTransform[1] );
  myRasterViewPort->rectYOffsetFloat = ( mLayerExtent.yMaximum() - theViewExtent.yMaximum() ) / fabs( mGeoTransform[5] );

  if ( myRasterViewPort->rectXOffsetFloat < 0 )
  {
    myRasterViewPort->rectXOffsetFloat = 0;
  }

  if ( myRasterViewPort->rectYOffsetFloat < 0 )
  {
    myRasterViewPort->rectYOffsetFloat = 0;
  }

  myRasterViewPort->rectXOffset = static_cast < int >( myRasterViewPort->rectXOffsetFloat );
  myRasterViewPort->rectYOffset = static_cast < int >( myRasterViewPort->rectYOffsetFloat );

  QgsDebugMsgLevel( QString( "mGeoTransform: %1, %2, %3, %4, %5, %6" )
                    .arg( mGeoTransform[0] )
                    .arg( mGeoTransform[1] )
                    .arg( mGeoTransform[2] )
                    .arg( mGeoTransform[3] )
                    .arg( mGeoTransform[4] )
                    .arg( mGeoTransform[5] ), 3 );

  // get dimensions of clipped raster image in raster pixel space/ RasterIO will do the scaling for us.
  // So for example, if the user is zoomed in a long way, there may only be e.g. 5x5 pixels retrieved from
  // the raw raster data, but rasterio will seamlessly scale the up to whatever the screen coordinats are
  // e.g. a 600x800 display window (see next section below)
  myRasterViewPort->clippedXMin = ( myRasterExtent.xMinimum() - mGeoTransform[0] ) / mGeoTransform[1];
  myRasterViewPort->clippedXMax = ( myRasterExtent.xMaximum() - mGeoTransform[0] ) / mGeoTransform[1];
  myRasterViewPort->clippedYMin = ( myRasterExtent.yMinimum() - mGeoTransform[3] ) / mGeoTransform[5];
  myRasterViewPort->clippedYMax = ( myRasterExtent.yMaximum() - mGeoTransform[3] ) / mGeoTransform[5];

  // Sometimes the Ymin/Ymax are reversed.
  if ( myRasterViewPort->clippedYMin > myRasterViewPort->clippedYMax )
  {
    double t = myRasterViewPort->clippedYMin;
    myRasterViewPort->clippedYMin = myRasterViewPort->clippedYMax;
    myRasterViewPort->clippedYMax = t;
  }

  // Set the clipped width and height to encompass all of the source pixels
  // that could end up being displayed.
  myRasterViewPort->clippedWidth =
    static_cast<int>( ceil( myRasterViewPort->clippedXMax ) - floor( myRasterViewPort->clippedXMin ) );

  myRasterViewPort->clippedHeight =
    static_cast<int>( ceil( myRasterViewPort->clippedYMax ) - floor( myRasterViewPort->clippedYMin ) );

  // but make sure the intended SE corner extent doesn't exceed the SE corner
  // of the source raster, otherwise GDAL's RasterIO gives an error and returns nothing.
  // The SE corner = NW origin + dimensions of the image itself.
  if (( myRasterViewPort->rectXOffset + myRasterViewPort->clippedWidth )
      > mWidth )
  {
    myRasterViewPort->clippedWidth =
      mWidth - myRasterViewPort->rectXOffset;
  }
  if (( myRasterViewPort->rectYOffset + myRasterViewPort->clippedHeight )
      > mHeight )
  {
    myRasterViewPort->clippedHeight =
      mHeight - myRasterViewPort->rectYOffset;
  }

  // get dimensions of clipped raster image in device coordinate space (this is the size of the viewport)
  myRasterViewPort->topLeftPoint = theQgsMapToPixel.transform( myRasterExtent.xMinimum(), myRasterExtent.yMaximum() );
  myRasterViewPort->bottomRightPoint = theQgsMapToPixel.transform( myRasterExtent.xMaximum(), myRasterExtent.yMinimum() );

  myRasterViewPort->drawableAreaXDim = static_cast<int>( fabs(( myRasterViewPort->clippedWidth / theQgsMapToPixel.mapUnitsPerPixel() * mGeoTransform[1] ) ) + 0.5 );
  myRasterViewPort->drawableAreaYDim = static_cast<int>( fabs(( myRasterViewPort->clippedHeight / theQgsMapToPixel.mapUnitsPerPixel() * mGeoTransform[5] ) ) + 0.5 );

  //the drawable area can start to get very very large when you get down displaying 2x2 or smaller, this is becasue
  //theQgsMapToPixel.mapUnitsPerPixel() is less then 1,
  //so we will just get the pixel data and then render these special cases differently in paintImageToCanvas()
  if ( 2 >= myRasterViewPort->clippedWidth && 2 >= myRasterViewPort->clippedHeight )
  {
    myRasterViewPort->drawableAreaXDim = myRasterViewPort->clippedWidth;
    myRasterViewPort->drawableAreaYDim = myRasterViewPort->clippedHeight;
  }

  QgsDebugMsgLevel( QString( "mapUnitsPerPixel = %1" ).arg( theQgsMapToPixel.mapUnitsPerPixel() ), 3 );
  QgsDebugMsgLevel( QString( "mWidth = %1" ).arg( mWidth ), 3 );
  QgsDebugMsgLevel( QString( "mHeight = %1" ).arg( mHeight ), 3 );
  QgsDebugMsgLevel( QString( "rectXOffset = %1" ).arg( myRasterViewPort->rectXOffset ), 3 );
  QgsDebugMsgLevel( QString( "rectXOffsetFloat = %1" ).arg( myRasterViewPort->rectXOffsetFloat ), 3 );
  QgsDebugMsgLevel( QString( "rectYOffset = %1" ).arg( myRasterViewPort->rectYOffset ), 3 );
  QgsDebugMsgLevel( QString( "rectYOffsetFloat = %1" ).arg( myRasterViewPort->rectYOffsetFloat ), 3 );

  QgsDebugMsgLevel( QString( "myRasterExtent.xMinimum() = %1" ).arg( myRasterExtent.xMinimum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.xMaximum() = %1" ).arg( myRasterExtent.xMaximum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.yMinimum() = %1" ).arg( myRasterExtent.yMinimum() ), 3 );
  QgsDebugMsgLevel( QString( "myRasterExtent.yMaximum() = %1" ).arg( myRasterExtent.yMaximum() ), 3 );

  QgsDebugMsgLevel( QString( "topLeftPoint.x() = %1" ).arg( myRasterViewPort->topLeftPoint.x() ), 3 );
  QgsDebugMsgLevel( QString( "bottomRightPoint.x() = %1" ).arg( myRasterViewPort->bottomRightPoint.x() ), 3 );
  QgsDebugMsgLevel( QString( "topLeftPoint.y() = %1" ).arg( myRasterViewPort->topLeftPoint.y() ), 3 );
  QgsDebugMsgLevel( QString( "bottomRightPoint.y() = %1" ).arg( myRasterViewPort->bottomRightPoint.y() ), 3 );

  QgsDebugMsgLevel( QString( "clippedXMin = %1" ).arg( myRasterViewPort->clippedXMin ), 3 );
  QgsDebugMsgLevel( QString( "clippedXMax = %1" ).arg( myRasterViewPort->clippedXMax ), 3 );
  QgsDebugMsgLevel( QString( "clippedYMin = %1" ).arg( myRasterViewPort->clippedYMin ), 3 );
  QgsDebugMsgLevel( QString( "clippedYMax = %1" ).arg( myRasterViewPort->clippedYMax ), 3 );

  QgsDebugMsgLevel( QString( "clippedWidth = %1" ).arg( myRasterViewPort->clippedWidth ), 3 );
  QgsDebugMsgLevel( QString( "clippedHeight = %1" ).arg( myRasterViewPort->clippedHeight ), 3 );
  QgsDebugMsgLevel( QString( "drawableAreaXDim = %1" ).arg( myRasterViewPort->drawableAreaXDim ), 3 );
  QgsDebugMsgLevel( QString( "drawableAreaYDim = %1" ).arg( myRasterViewPort->drawableAreaYDim ), 3 );

  QgsDebugMsgLevel( "ReadXml: gray band name : " + mGrayBandName, 3 );
  QgsDebugMsgLevel( "ReadXml: red band name : " + mRedBandName, 3 );
  QgsDebugMsgLevel( "ReadXml: green band name : " + mGreenBandName, 3 );
  QgsDebugMsgLevel( "ReadXml: blue band name : " + mBlueBandName, 3 );

  // /\/\/\ - added to handle zoomed-in rasters

  mLastViewPort = *myRasterViewPort;

  // Provider mode: See if a provider key is specified, and if so use the provider instead

  QgsDebugMsg( "Checking for provider key." );

  if ( !mProviderKey.isEmpty() )
  {
    QgsDebugMsg( "Wanting a '" + mProviderKey + "' provider to draw this." );

    mDataProvider->setDpi( rendererContext.rasterScaleFactor() * 25.4 * rendererContext.scaleFactor() );

    //fetch image in several parts if it is too memory consuming
    //also some WMS servers have a pixel limit, so it's better to make several requests
    int totalPixelWidth = fabs(( myRasterViewPort->clippedXMax -  myRasterViewPort->clippedXMin )
                               / theQgsMapToPixel.mapUnitsPerPixel() * mGeoTransform[1] ) + 1;
    int totalPixelHeight = fabs(( myRasterViewPort->clippedYMax -  myRasterViewPort->clippedYMin )
                                / theQgsMapToPixel.mapUnitsPerPixel() * mGeoTransform[5] ) + 1;
    int numParts = totalPixelWidth * totalPixelHeight / 5000000 + 1.0;
    int numRowsPerPart = totalPixelHeight / numParts + 1.0;


    int currentPixelOffsetY = 0; //top y-coordinate of current raster part
    //the width of a WMS image part
    int pixelWidth = ( myRasterExtent.xMaximum() - myRasterExtent.xMinimum() ) / theQgsMapToPixel.mapUnitsPerPixel() + 0.5;
    for ( int i = 0; i < numParts; ++i )
    {
      //fetch a small overlap of 2 pixels between two adjacent tiles to avoid white stripes
      QgsRectangle rasterPartRect( myRasterExtent.xMinimum(), myRasterExtent.yMaximum() - ( currentPixelOffsetY + numRowsPerPart + 2 ) * theQgsMapToPixel.mapUnitsPerPixel(),
                                   myRasterExtent.xMaximum(), myRasterExtent.yMaximum() - currentPixelOffsetY * theQgsMapToPixel.mapUnitsPerPixel() );

      int pixelHeight = rasterPartRect.height() / theQgsMapToPixel.mapUnitsPerPixel() + 0.5;

      /*
      QgsDebugMsg( "**********WMS tile parameter***************" );
      QgsDebugMsg( "pixelWidth: " + QString::number( pixelWidth ) );
      QgsDebugMsg( "pixelHeight: " + QString::number( pixelHeight ) );
      QgsDebugMsg( "mapWidth: " + QString::number( rasterPartRect.width() ) );
      QgsDebugMsg( "mapHeight: " + QString::number( rasterPartRect.height(), 'f', 8 ) );
      QgsDebugMsg( "mapUnitsPerPixel: " + QString::number( theQgsMapToPixel.mapUnitsPerPixel() ) );*/

      QImage* image = mDataProvider->draw( rasterPartRect, pixelWidth, pixelHeight );

      if ( !image )
      {
        // An error occurred.
        mErrorCaption = mDataProvider->lastErrorTitle();
        mError        = mDataProvider->lastError();

        delete myRasterViewPort;
        return false;
      }

      QgsDebugMsg( "done mDataProvider->draw." );

      QgsDebugMsgLevel( QString( "image stats: depth=%1 bytes=%2 width=%3 height=%4" ).arg( image->depth() )
                        .arg( image->numBytes() )
                        .arg( image->width() )
                        .arg( image->height() ),
                        3 );

      QgsDebugMsgLevel( QString( "Want to theQPainter->drawImage with origin x: %1 (%2) %3 (%4)" )
                        .arg( myRasterViewPort->topLeftPoint.x() ).arg( static_cast<int>( myRasterViewPort->topLeftPoint.x() ) )
                        .arg( myRasterViewPort->topLeftPoint.y() ).arg( static_cast<int>( myRasterViewPort->topLeftPoint.y() ) ),
                        3 );

      //Set the transparency for the whole layer
      //QImage::setAlphaChannel does not work quite as expected so set each pixel individually
      //Currently this is only done for WMS images, which should be small enough not to impact performance

      if ( mTransparencyLevel != 255 ) //improve performance if layer transparency not altered
      {
        QImage* transparentImageCopy = new QImage( *image ); //copy image if there is user transparency
        image = transparentImageCopy;
        int myWidth = image->width();
        int myHeight = image->height();
        QRgb myRgb;
        int newTransparency;
        for ( int myHeightRunner = 0; myHeightRunner < myHeight; myHeightRunner++ )
        {
          QRgb* myLineBuffer = ( QRgb* ) transparentImageCopy->scanLine( myHeightRunner );
          for ( int myWidthRunner = 0; myWidthRunner < myWidth; myWidthRunner++ )
          {
            myRgb = image->pixel( myWidthRunner, myHeightRunner );
            //combine transparency from WMS and layer transparency
            newTransparency = ( double ) mTransparencyLevel / 255.0 * ( double )( qAlpha( myRgb ) );
            myLineBuffer[ myWidthRunner ] = qRgba( qRed( myRgb ), qGreen( myRgb ), qBlue( myRgb ), newTransparency );
          }
        }
      }

      theQPainter->drawImage( myRasterViewPort->topLeftPoint.x(), myRasterViewPort->topLeftPoint.y() + currentPixelOffsetY, *image );
      currentPixelOffsetY += numRowsPerPart;

      if ( mTransparencyLevel != 255 )
      {
        delete image;
      }
    }
  }
  else
  {
    // Otherwise use the old-fashioned GDAL direct-drawing style
    // TODO: Move into its own GDAL provider.

    // \/\/\/ - commented-out to handle zoomed-in rasters
    //    draw(theQPainter,myRasterViewPort);
    // /\/\/\ - commented-out to handle zoomed-in rasters
    // \/\/\/ - added to handle zoomed-in rasters
    draw( theQPainter, myRasterViewPort, &theQgsMapToPixel );
    // /\/\/\ - added to handle zoomed-in rasters
  }

  delete myRasterViewPort;
  QgsDebugMsg( "exiting." );

  return true;

}

void QgsRasterLayer::draw( QPainter * theQPainter,
                           QgsRasterViewPort * theRasterViewPort,
                           const QgsMapToPixel* theQgsMapToPixel )
{
  QgsDebugMsg( " 3 arguments" );
  //
  //
  // The goal here is to make as many decisions as possible early on (outside of the rendering loop)
  // so that we can maximise performance of the rendering process. So now we check which drawing
  // procedure to use :
  //

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

        drawPalettedSingleBandColor( theQPainter, theRasterViewPort,
                                     theQgsMapToPixel, bandNumber( mGrayBandName ) );

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
    case MultiBandSingleGandGray:
      QgsDebugMsg( "MultiBandSingleGandGray drawing type detected..." );
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        QgsDebugMsg( "MultiBandSingleGandGray Not Set detected..." + mGrayBandName );
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
        drawMultiBandColor( theQPainter, theRasterViewPort,
                            theQgsMapToPixel );
      }
      break;

    default:
      break;

  }

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
    case MultiBandSingleGandGray:
      return QString( "MultiBandSingleGandGray" );//no need to tr() this its not shown in ui
      break;
    case MultiBandSingleBandPseudoColor:
      return QString( "MultiBandSingleBandPseudoColor" );//no need to tr() this its not shown in ui
      break;
    case MultiBandColor:
      return QString( "MultiBandColor" );//no need to tr() this its not shown in ui
      break;
    default:
      break;
  }

  return QString( "UndefinedDrawingStyle" );

}

/**
 * @note Note implemented yet
 * @return always returns false
 */
bool QgsRasterLayer::hasCompatibleSymbology( const QgsMapLayer& theOther ) const
{
  //preventwarnings
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
    //vector starts at base 0, band counts at base1 !
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
  if ( mProviderKey == "wms" )
  {
    return false;
  }

  QgsDebugMsg( thePoint.toString() );

  if ( !mProviderKey.isEmpty() )
  {
    QgsDebugMsg( "identify provider : " + mProviderKey ) ;
    return ( mDataProvider->identify( thePoint, theResults ) );
  }

  if ( !mLayerExtent.contains( thePoint ) )
  {
    // Outside the raster
    for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); i++ )
    {
      theResults[ generateBandName( i )] = tr( "out of extent" );
    }
  }
  else
  {
    double x = thePoint.x();
    double y = thePoint.y();

    /* Calculate the row / column where the point falls */
    double xres = ( mLayerExtent.xMaximum() - mLayerExtent.xMinimum() ) / mWidth;
    double yres = ( mLayerExtent.yMaximum() - mLayerExtent.yMinimum() ) / mHeight;

    // Offset, not the cell index -> flor
    int col = ( int ) floor(( x - mLayerExtent.xMinimum() ) / xres );
    int row = ( int ) floor(( mLayerExtent.yMaximum() - y ) / yres );

    QgsDebugMsg( "row = " + QString::number( row ) + " col = " + QString::number( col ) );

    for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); i++ )
    {
      GDALRasterBandH gdalBand = GDALGetRasterBand( mGdalDataset, i );
      GDALDataType type = GDALGetRasterDataType( gdalBand );
      int size = GDALGetDataTypeSize( type ) / 8;
      void *data = CPLMalloc( size );

      CPLErr err = GDALRasterIO( gdalBand, GF_Read, col, row, 1, 1,
                                 data, 1, 1, type, 0, 0 );

      if ( err != CPLE_None )
      {
        QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
      }

      double value = readValue( data, type, 0 );
#ifdef QGISDEBUG
      QgsLogger::debug( "value", value, 1, __FILE__, __FUNCTION__, __LINE__ );
#endif
      QString v;

      if ( mValidNoDataValue && ( fabs( value - mNoDataValue ) <= TINY_VALUE || value != value ) )
      {
        v = tr( "null (no data)" );
      }
      else
      {
        v.setNum( value );
      }

      theResults[ generateBandName( i )] = v;

      CPLFree( data );
    }
  }

  return true;
} // bool QgsRasterLayer::identify

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
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, 1 );
    QString myColorerpretation = GDALGetColorInterpretationName( GDALGetRasterColorInterpretation( myGdalBand ) );



    //
    // Create the legend pixmap - note it is generated on the preadjusted stats
    //
    if ( mDrawingStyle == MultiBandSingleGandGray || mDrawingStyle == PalettedSingleBandGray || mDrawingStyle == SingleBandGray )
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
    if ( mDrawingStyle == MultiBandSingleGandGray || mDrawingStyle == PalettedSingleBandGray || mDrawingStyle == SingleBandGray )
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
 * \param int theLabelCountInt Number of vertical labels to display
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
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, 1 );
  QString myColorerpretation = GDALGetColorInterpretationName( GDALGetRasterColorInterpretation( myGdalBand ) );
  QPixmap myLegendQPixmap;      //will be initialised once we know what drawing style is active
  QPainter myQPainter;
  //
  // Create the legend pixmap - note it is generated on the preadjusted stats
  //
  if ( mDrawingStyle == MultiBandSingleGandGray || mDrawingStyle == PalettedSingleBandGray || mDrawingStyle == SingleBandGray )
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
  if ( mDrawingStyle == MultiBandSingleGandGray || mDrawingStyle == PalettedSingleBandGray || mDrawingStyle == SingleBandGray )
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
  if ( mProviderKey.isEmpty() )
  {
    myMetadata += QString( GDALGetDescription( GDALGetDatasetDriver( mGdalDataset ) ) );
    myMetadata += "<br>";
    myMetadata += QString( GDALGetMetadataItem( GDALGetDatasetDriver( mGdalDataset ), GDAL_DMD_LONGNAME, NULL ) );
  }
  else
  {
    myMetadata += mDataProvider->description();
  }
  myMetadata += "</p>\n";

  if ( !mProviderKey.isEmpty() )
  {
    // Insert provider-specific (e.g. WMS-specific) metadata
    myMetadata += mDataProvider->metadata();
  }
  else
  {

    // my added code (MColetti)

    myMetadata += "<p class=\"glossy\">";
    myMetadata += tr( "Dataset Description" );
    myMetadata += "</p>\n";
    myMetadata += "<p>";
    myMetadata += QFile::decodeName( GDALGetDescription( mGdalDataset ) );
    myMetadata += "</p>\n";


    char ** GDALmetadata = GDALGetMetadata( mGdalDataset, NULL );

    if ( GDALmetadata )
    {
      QStringList metadata = cStringList2Q_( GDALmetadata );
      myMetadata += makeTableCells_( metadata );
    }
    else
    {
      QgsDebugMsg( "dataset has no metadata" );
    }

    for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); ++i )
    {
      myMetadata += "<p class=\"glossy\">" + tr( "Band %1" ).arg( i ) + "</p>\n";
      GDALRasterBandH gdalBand = GDALGetRasterBand( mGdalDataset, i );
      GDALmetadata = GDALGetMetadata( gdalBand, NULL );

      if ( GDALmetadata )
      {
        QStringList metadata = cStringList2Q_( GDALmetadata );
        myMetadata += makeTableCells_( metadata );
      }
      else
      {
        QgsDebugMsg( "band " + QString::number( i ) + "has no metadata" );
      }

      char ** GDALcategories = GDALGetRasterCategoryNames( gdalBand );

      if ( GDALcategories )
      {
        QStringList categories = cStringList2Q_( GDALcategories );
        myMetadata += makeTableCells_( categories );
      }
      else
      {
        QgsDebugMsg( "band " + QString::number( i ) + " has no categories" );
      }

    }

    // end my added code

    myMetadata += "<p class=\"glossy\">";
    myMetadata += tr( "Dimensions:" );
    myMetadata += "</p>\n";
    myMetadata += "<p>";
    myMetadata += tr( "X: %1 Y: %2 Bands: %3" )
                  .arg( GDALGetRasterXSize( mGdalDataset ) )
                  .arg( GDALGetRasterYSize( mGdalDataset ) )
                  .arg( GDALGetRasterCount( mGdalDataset ) );
    myMetadata += "</p>\n";

    //just use the first band
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, 1 );

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
    switch ( GDALGetRasterDataType( myGdalBand ) )
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

    if ( GDALGetOverviewCount( myGdalBand ) > 0 )
    {
      int myOverviewInt;
      for ( myOverviewInt = 0;
            myOverviewInt < GDALGetOverviewCount( myGdalBand );
            myOverviewInt++ )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGdalBand, myOverviewInt );
        myMetadata += "<p>X : " + QString::number( GDALGetRasterBandXSize( myOverview ) );
        myMetadata += ",Y " + QString::number( GDALGetRasterBandYSize( myOverview ) ) + "</p>";
      }
    }
    myMetadata += "</p>\n";
  }  // if (mProviderKey.isEmpty())

  myMetadata += "<p class=\"glossy\">";
  myMetadata += tr( "Layer Spatial Reference System: " );
  myMetadata += "</p>\n";
  myMetadata += "<p>";
  myMetadata += mCRS->toProj4();
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

  if ( mProviderKey.isEmpty() )
  {
    if ( GDALGetGeoTransform( mGdalDataset, mGeoTransform ) != CE_None )
    {
      // if the raster does not have a valid transform we need to use
      // a pixel size of (1,-1), but GDAL returns (1,1)
      mGeoTransform[5] = -1;
    }
    else
    {
      myMetadata += "<p class=\"glossy\">";
      myMetadata += tr( "Origin:" );
      myMetadata += "</p>\n";
      myMetadata += "<p>";
      myMetadata += QString::number( mGeoTransform[0] );
      myMetadata += ",";
      myMetadata += QString::number( mGeoTransform[3] );
      myMetadata += "</p>\n";

      myMetadata += "<p class=\"glossy\">";
      myMetadata += tr( "Pixel Size:" );
      myMetadata += "</p>\n";
      myMetadata += "<p>";
      myMetadata += QString::number( mGeoTransform[1] );
      myMetadata += ",";
      myMetadata += QString::number( mGeoTransform[5] );
      myMetadata += "</p>\n";
    }

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
  } // if (mProviderKey.isEmpty())

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
    QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = myShader.colorRampItemList();

    if ( readColorTable( 1, &myColorRampItemList ) )
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

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  QgsRasterBandStats myRasterBandStats = bandStatistics( theBandNo );
  //calculate the histogram for this band
  //we assume that it only needs to be calculated if the length of the histogram
  //vector is not equal to the number of bins
  //i.e if the histogram has never previously been generated or the user has
  //selected a new number of bins.
  if ( myRasterBandStats.histogramVector->size() != theBinCount ||
       theIgnoreOutOfRangeFlag != myRasterBandStats.isHistogramOutOfRange ||
       theHistogramEstimatedFlag != myRasterBandStats.isHistogramEstimated )
  {
    myRasterBandStats.histogramVector->clear();
    myRasterBandStats.isHistogramEstimated = theHistogramEstimatedFlag;
    myRasterBandStats.isHistogramOutOfRange = theIgnoreOutOfRangeFlag;
    int *myHistogramArray = new int[theBinCount];


    /*
     *  CPLErr GDALRasterBand::GetHistogram (
     *          double       dfMin,
     *          double      dfMax,
     *          int     nBuckets,
     *          int *   panHistogram,
     *          int     bIncludeOutOfRange,
     *          int     bApproxOK,
     *          GDALProgressFunc    pfnProgress,
     *          void *      pProgressData
     *          )
     */
    double myerval = ( myRasterBandStats.maximumValue - myRasterBandStats.minimumValue ) / theBinCount;
    GDALGetRasterHistogram( myGdalBand, myRasterBandStats.minimumValue - 0.1*myerval,
                            myRasterBandStats.maximumValue + 0.1*myerval, theBinCount, myHistogramArray,
                            theIgnoreOutOfRangeFlag, theHistogramEstimatedFlag, progressCallback,
                            this ); //this is the arg for our custome gdal progress callback

    for ( int myBin = 0; myBin < theBinCount; myBin++ )
    {
      myRasterBandStats.histogramVector->push_back( myHistogramArray[myBin] );
      QgsDebugMsg( "Added " + QString::number( myHistogramArray[myBin] ) + " to histogram vector" );
    }

  }
  QgsDebugMsg( ">>>>> Histogram vector now contains " + QString::number( myRasterBandStats.histogramVector->size() ) +
               " elements" );
}


QString QgsRasterLayer::providerKey()
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

  return fabs( mGeoTransform[1] );
}

/**
 * @param theBandNumber the number of the band for which you want a color table
 * @param theList a pointer the object that will hold the color table
 * @return true of a color table was able to be read, false otherwise
 */
bool QgsRasterLayer::readColorTable( int theBandNumber, QList<QgsColorRampShader::ColorRampItem>* theList )
{
  QgsDebugMsg( "entered." );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNumber || 0 == theList )
  {
    QgsDebugMsg( "Invalid parameter" );
    return false;
  }

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNumber );
  GDALColorTableH myGdalColorTable = GDALGetRasterColorTable( myGdalBand );

  if ( myGdalColorTable )
  {
    QgsDebugMsg( "Color table found" );
    int myEntryCount = GDALGetColorEntryCount( myGdalColorTable );
    GDALColorInterp myColorInterpretation =  GDALGetRasterColorInterpretation( myGdalBand );
    QgsDebugMsg( "Color Interpretation: " + QString::number(( int )myColorInterpretation ) );
    GDALPaletteInterp myPaletteInterpretation  = GDALGetPaletteInterpretation( myGdalColorTable );
    QgsDebugMsg( "Palette Interpretation: " + QString::number(( int )myPaletteInterpretation ) );

    const GDALColorEntry* myColorEntry = 0;
    for ( int myIterator = 0; myIterator < myEntryCount; myIterator++ )
    {
      myColorEntry = GDALGetColorEntry( myGdalColorTable, myIterator );

      if ( !myColorEntry )
      {
        continue;
      }
      else
      {
        //Branch on the color interpretation type
        if ( myColorInterpretation == GCI_GrayIndex )
        {
          QgsColorRampShader::ColorRampItem myColorRampItem;
          myColorRampItem.label = "";
          myColorRampItem.value = ( double )myIterator;
          myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c1, myColorEntry->c1, myColorEntry->c4 );
          theList->append( myColorRampItem );
        }
        else if ( myColorInterpretation == GCI_PaletteIndex )
        {
          QgsColorRampShader::ColorRampItem myColorRampItem;
          myColorRampItem.label = "";
          myColorRampItem.value = ( double )myIterator;
          //Branch on palette interpretation
          if ( myPaletteInterpretation  == GPI_RGB )
          {
            myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c2, myColorEntry->c3, myColorEntry->c4 );
          }
          else if ( myPaletteInterpretation  == GPI_CMYK )
          {
            myColorRampItem.color = QColor::fromCmyk( myColorEntry->c1, myColorEntry->c2, myColorEntry->c3, myColorEntry->c4 );
          }
          else if ( myPaletteInterpretation  == GPI_HLS )
          {
            myColorRampItem.color = QColor::fromHsv( myColorEntry->c1, myColorEntry->c3, myColorEntry->c2, myColorEntry->c4 );
          }
          else
          {
            myColorRampItem.color = QColor::fromRgb( myColorEntry->c1, myColorEntry->c1, myColorEntry->c1, myColorEntry->c4 );
          }
          theList->append( myColorRampItem );
        }
        else
        {
          QgsDebugMsg( "Color interpretation type not supported yet" );
          return false;
        }
      }
    }
  }
  else
  {
    QgsDebugMsg( "No color table found for band " + QString::number( theBandNumber ) );
    return false;
  }

  QgsDebugMsg( "Color table loaded sucessfully" );
  return true;
}

void QgsRasterLayer::resetNoDataValue()
{
  mNoDataValue = std::numeric_limits<int>::max();
  mValidNoDataValue = false;
  if ( mGdalDataset != NULL && GDALGetRasterCount( mGdalDataset ) > 0 )
  {
    int myRequestValid;
    double myValue = GDALGetRasterNoDataValue(
                       GDALGetRasterBand( mGdalDataset, 1 ), &myRequestValid );

    if ( 0 != myRequestValid )
    {
      setNoDataValue( myValue );
    }
    else
    {
      setNoDataValue( -9999.0 );
      mValidNoDataValue = false;
    }
  }
}


void QgsRasterLayer::setBlueBandName( QString const & theBandName )
{
  mBlueBandName = validateBandName( theBandName );
}

/** Copied from QgsVectorLayer::setDataProvider
 *  TODO: Make it work in the raster environment
 */
void QgsRasterLayer::setDataProvider( QString const & provider,
                                      QStringList const & layers,
                                      QStringList const & styles,
                                      QString const & format,
                                      QString const & crs )
{
  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  mProviderKey = provider;     // XXX is this necessary?  Usually already set
  // XXX when execution gets here.

  // load the plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QString ogrlib = pReg->library( provider );

  //QString ogrlib = libDir + "/libpostgresprovider.so";

#ifdef TESTPROVIDERLIB
  const char *cOgrLib = ( const char * ) ogrlib;
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
  mLib = new QLibrary( ogrlib );
  QgsDebugMsg( "Library name is " + mLib->fileName() );
  bool loaded = mLib->load();

  if ( loaded )
  {
    QgsDebugMsg( "Loaded data provider library" );
    QgsDebugMsg( "Attempting to resolve the classFactory function" );
    classFactoryFunction_t * classFactory = ( classFactoryFunction_t * ) cast_to_fptr( mLib->resolve( "classFactory" ) );

    mValid = false;            // assume the layer is invalid until we
    // determine otherwise
    if ( classFactory )
    {
      QgsDebugMsg( "Getting pointer to a mDataProvider object from the library" );
      //XXX - This was a dynamic cast but that kills the Windows
      //      version big-time with an abnormal termination error
      //      mDataProvider = (QgsRasterDataProvider*)(classFactory((const
      //                                              char*)(dataSource.utf8())));

      // Copied from qgsproviderregistry in preference to the above.
      mDataProvider = ( QgsRasterDataProvider* )( *classFactory )( &mDataSource );

      if ( mDataProvider )
      {
        QgsDebugMsg( "Instantiated the data provider plugin" +
                     QString( " with layer list of " ) + layers.join( ", " ) + " and style list of " + styles.join( ", " ) +
                     " and format of " + format +  " and CRS of " + crs );
        if ( mDataProvider->isValid() )
        {
          mValid = true;

          mDataProvider->addLayers( layers, styles );
          mDataProvider->setImageEncoding( format );
          mDataProvider->setImageCrs( crs );

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
        }
      }
      else
      {
        QgsLogger::warning( "QgsRasterLayer::setDataProvider: Unable to instantiate the data provider plugin" );
        mValid = false;
      }
    }
  }
  else
  {
    mValid = false;
    QgsLogger::warning( "QgsRasterLayer::setDataProvider: Failed to load ../providers/libproviders.so" );

  }
  QgsDebugMsg( "exiting." );

} // QgsRasterLayer::setDataProvider

void QgsRasterLayer::setColorShadingAlgorithm( ColorShadingAlgorithm theShadingAlgorithm )
{
  QgsDebugMsg( "called with [" + QString::number( theShadingAlgorithm ) + "]" );
  if ( mColorShadingAlgorithm != theShadingAlgorithm )
  {
    if ( 0 == mRasterShader )
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
  else if ( theDrawingStyleQString == "MultiBandSingleGandGray" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = MultiBandSingleGandGray;
  }
  else if ( theDrawingStyleQString == "MultiBandSingleBandPseudoColor" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = MultiBandSingleBandPseudoColor;
  }
  else if ( theDrawingStyleQString == "MultiBandColor" )//no need to tr() this its not shown in ui
  {
    mDrawingStyle = MultiBandColor;
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
  if ( rasterType() == QgsRasterLayer::GrayOrUndefined || drawingStyle() == QgsRasterLayer::SingleBandGray || drawingStyle() == QgsRasterLayer::MultiBandSingleGandGray )
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

void QgsRasterLayer::setMinimumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag )
{
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

  if ( mDataProvider )
  {
    return mDataProvider->subLayers();
  }
  else
  {
    return QStringList();   // Empty
  }

}

void QgsRasterLayer::thumbnailAsPixmap( QPixmap * theQPixmap )
{
  //TODO: This should be depreciated and a new function written that just returns a new QPixmap, it will be safer
  if ( 0 == theQPixmap ) { return; }

  theQPixmap->fill(); //defaults to white

  // Raster providers are disabled (for the moment)
  if ( mProviderKey.isEmpty() )
  {
    QgsRasterViewPort *myRasterViewPort = new QgsRasterViewPort();
    myRasterViewPort->rectXOffset = 0;
    myRasterViewPort->rectYOffset = 0;
    myRasterViewPort->clippedXMin = 0;
    myRasterViewPort->clippedXMax = mWidth;
    myRasterViewPort->clippedYMin = mHeight;
    myRasterViewPort->clippedYMax = 0;
    myRasterViewPort->clippedWidth   = mWidth;
    myRasterViewPort->clippedHeight  = mHeight;
    myRasterViewPort->topLeftPoint = QgsPoint( 0, 0 );
    myRasterViewPort->bottomRightPoint = QgsPoint( theQPixmap->width(), theQPixmap->height() );
    myRasterViewPort->drawableAreaXDim = theQPixmap->width();
    myRasterViewPort->drawableAreaYDim = theQPixmap->height();

    QPainter * myQPainter = new QPainter( theQPixmap );
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
  setNoDataValue( myElement.text().toDouble() );
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

  @note Called by QgsMapLayer::readXML().
*/
bool QgsRasterLayer::readXml( QDomNode & layer_node )
{
  //! @NOTE Make sure to read the file first so stats etc are initialised properly!

  //process provider key
  QDomNode pkeyNode = layer_node.namedItem( "provider" );

  if ( pkeyNode.isNull() )
  {
    mProviderKey = "";
  }
  else
  {
    QDomElement pkeyElt = pkeyNode.toElement();
    mProviderKey = pkeyElt.text();
  }

  // Open the raster source based on provider and datasource

  if ( !mProviderKey.isEmpty() )
  {
    // Go down the raster-data-provider paradigm

    // Collect provider-specific information

    QDomNode rpNode = layer_node.namedItem( "rasterproperties" );

    // Collect sublayer names and styles
    QStringList layers;
    QStringList styles;
    QDomElement layerElement = rpNode.firstChildElement( "wmsSublayer" );
    while ( !layerElement.isNull() )
    {
      // TODO: sublayer visibility - post-0.8 release timeframe

      // collect name for the sublayer
      layers += layerElement.namedItem( "name" ).toElement().text();

      // collect style for the sublayer
      styles += layerElement.namedItem( "style" ).toElement().text();

      layerElement = layerElement.nextSiblingElement( "wmsSublayer" );
    }

    // Collect format
    QString format = rpNode.namedItem( "wmsFormat" ).toElement().text();

    // Collect CRS
    QString crs = srs().authid();

    setDataProvider( mProviderKey, layers, styles, format, crs );
  }
  else
  {
    // Go down the monolithic-gdal-provider paradigm

    if ( !readFile( source() ) )   // Data source name set in
      // QgsMapLayer::readXML()
    {
      QgsLogger::warning( QString( __FILE__ ) + ":" + QString( __LINE__ ) +
                          " unable to read from raster file " + source() );
      return false;
    }

  }

  QString theError;
  return readSymbology( layer_node, theError );


} // QgsRasterLayer::readXml( QDomNode & layer_node )


/*
 * @param QDomNode the node that will have the style element added to it.
 * @param QDomDocument the document that will have the QDomNode added.
 * @param errorMessage reference to string that will be updated with any error messages
 * @return true in case of success.
 */
bool QgsRasterLayer::writeSymbology( QDomNode & layer_node, QDomDocument & document, QString& errorMessage ) const
{
  // <rasterproperties>
  QDomElement rasterPropertiesElement = document.createElement( "rasterproperties" );
  layer_node.appendChild( rasterPropertiesElement );

  if ( !mProviderKey.isEmpty() )
  {
    QStringList sl = subLayers();
    QStringList sls = mDataProvider->subLayerStyles();

    QStringList::const_iterator layerStyle = sls.begin();

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

  if ( mapLayerNode.isNull() || ( "maplayer" != mapLayerNode.nodeName() ) )
  {
    QgsLogger::warning( "QgsRasterLayer::writeXML() can't find <maplayer>" );
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

  GDALRasterBandH myGdalRedBand = GDALGetRasterBand( mGdalDataset, myRedBandNo );
  GDALRasterBandH myGdalGreenBand = GDALGetRasterBand( mGdalDataset, myGreenBandNo );
  GDALRasterBandH myGdalBlueBand = GDALGetRasterBand( mGdalDataset, myBlueBandNo );

  GDALDataType myRedType = GDALGetRasterDataType( myGdalRedBand );
  GDALDataType myGreenType = GDALGetRasterDataType( myGdalGreenBand );
  GDALDataType myBlueType = GDALGetRasterDataType( myGdalBlueBand );

  QRgb* redImageScanLine = 0;
  void* redRasterScanLine = 0;
  QRgb* greenImageScanLine = 0;
  void* greenRasterScanLine = 0;
  QRgb* blueImageScanLine = 0;
  void* blueRasterScanLine = 0;

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
    double GDALrange[2];
    mRGBMinimumMaximumEstimated = true;

    GDALComputeRasterMinMax( myGdalRedBand, 1, GDALrange ); //Approximate
    setMaximumValue( myRedBandNo, GDALrange[1] );
    setMinimumValue( myRedBandNo, GDALrange[0] );

    GDALComputeRasterMinMax( myGdalGreenBand, 1, GDALrange ); //Approximate
    setMaximumValue( myGreenBandNo, GDALrange[1] );
    setMinimumValue( myGreenBandNo, GDALrange[0] );

    GDALComputeRasterMinMax( myGdalBlueBand, 1, GDALrange ); //Approximate
    setMaximumValue( myBlueBandNo, GDALrange[1] );
    setMinimumValue( myBlueBandNo, GDALrange[0] );
  }

  //Read and display pixels
  double myRedValue = 0.0;
  double myGreenValue = 0.0;
  double myBlueValue = 0.0;

  int myStretchedRedValue   = 0;
  int myStretchedGreenValue = 0;
  int myStretchedBlueValue  = 0;
  int myAlphaValue = 0;
  QgsContrastEnhancement* myRedContrastEnhancement = contrastEnhancement( myRedBandNo );
  QgsContrastEnhancement* myGreenContrastEnhancement = contrastEnhancement( myGreenBandNo );
  QgsContrastEnhancement* myBlueContrastEnhancement = contrastEnhancement( myBlueBandNo );

  QgsRasterImageBuffer redImageBuffer( myGdalRedBand, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  redImageBuffer.reset();
  QgsRasterImageBuffer greenImageBuffer( myGdalGreenBand, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  greenImageBuffer.setWritingEnabled( false ); //only draw to redImageBuffer
  greenImageBuffer.reset();
  QgsRasterImageBuffer blueImageBuffer( myGdalBlueBand, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  blueImageBuffer.setWritingEnabled( false ); //only draw to redImageBuffer
  blueImageBuffer.reset();

  while ( redImageBuffer.nextScanLine( &redImageScanLine, &redRasterScanLine )
          && greenImageBuffer.nextScanLine( &greenImageScanLine, &greenRasterScanLine )
          && blueImageBuffer.nextScanLine( &blueImageScanLine, &blueRasterScanLine ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      myRedValue   = readValue( redRasterScanLine, ( GDALDataType )myRedType, i );
      myGreenValue = readValue( greenRasterScanLine, ( GDALDataType )myGreenType, i );
      myBlueValue  = readValue( blueRasterScanLine, ( GDALDataType )myBlueType, i );

      if ( mValidNoDataValue &&
           (
             ( fabs( myRedValue - mNoDataValue ) <= TINY_VALUE || myRedValue != myRedValue ) ||
             ( fabs( myGreenValue - mNoDataValue ) <= TINY_VALUE || myGreenValue != myGreenValue ) ||
             ( fabs( myBlueValue - mNoDataValue ) <= TINY_VALUE || myBlueValue != myBlueValue )
           )
         )
      {
        redImageScanLine[ i ] = myDefaultColor;
        continue;
      }

      if ( !myRedContrastEnhancement->isValueInDisplayableRange( myRedValue ) ||
           !myGreenContrastEnhancement->isValueInDisplayableRange( myGreenValue ) ||
           !myBlueContrastEnhancement->isValueInDisplayableRange( myBlueValue ) )
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

      myStretchedRedValue = myRedContrastEnhancement->enhanceContrast( myRedValue );
      myStretchedGreenValue = myGreenContrastEnhancement->enhanceContrast( myGreenValue );
      myStretchedBlueValue = myBlueContrastEnhancement->enhanceContrast( myBlueValue );

      if ( mInvertColor )
      {
        myStretchedRedValue = 255 - myStretchedRedValue;
        myStretchedGreenValue = 255 - myStretchedGreenValue;
        myStretchedBlueValue = 255 - myStretchedBlueValue;
      }

      redImageScanLine[ i ] = qRgba( myStretchedRedValue, myStretchedGreenValue, myStretchedBlueValue, myAlphaValue );
    }
  }
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
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
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

  if ( NULL == mRasterShader )
  {
    return;
  }

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );

  QgsRasterImageBuffer imageBuffer( myGdalBand, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );
  double myPixelValue = 0.0;
  int myRedValue = 0;
  int myGreenValue = 0;
  int myBlueValue = 0;
  int myAlphaValue = 0;

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      myRedValue = 0;
      myGreenValue = 0;
      myBlueValue = 0;
      myPixelValue = readValue( rasterScanLine, ( GDALDataType )myDataType, i );

      if ( mValidNoDataValue && ( fabs( myPixelValue - mNoDataValue ) <= TINY_VALUE || myPixelValue != myPixelValue ) )
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

      if ( mInvertColor )
      {
        //Invert flag, flip blue and read
        imageScanLine[ i ] = qRgba( myBlueValue, myGreenValue, myRedValue, myAlphaValue );
      }
      else
      {
        //Normal
        imageScanLine[ i ] = qRgba( myRedValue, myGreenValue, myBlueValue, myAlphaValue );
      }
    }
  }
}

/**
 * This method is used to render a paletted raster layer as a gray image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
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

  if ( NULL == mRasterShader )
  {
    return;
  }

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );

  QgsRasterImageBuffer imageBuffer( myGdalBand, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );
  double myPixelValue = 0.0;
  int myRedValue = 0;
  int myGreenValue = 0;
  int myBlueValue = 0;
  int myAlphaValue = 0;

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      myRedValue = 0;
      myGreenValue = 0;
      myBlueValue = 0;
      myPixelValue = readValue( rasterScanLine, ( GDALDataType )myDataType, i );

      if ( mValidNoDataValue && ( fabs( myPixelValue - mNoDataValue ) <= TINY_VALUE || myPixelValue != myPixelValue ) )
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

      if ( mInvertColor )
      {
        //Invert flag, flip blue and read
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
}

/**
 * This method is used to render a paletted raster layer as a pseudocolor image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
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

  QgsRasterBandStats myRasterBandStats = bandStatistics( theBandNo );
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );

  QgsRasterImageBuffer imageBuffer( myGdalBand, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;

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
  int myAlphaValue = 0;

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      myRedValue = 0;
      myGreenValue = 0;
      myBlueValue = 0;
      myPixelValue = readValue( rasterScanLine, ( GDALDataType )myDataType, i );

      if ( mValidNoDataValue && ( fabs( myPixelValue - mNoDataValue ) <= TINY_VALUE || myPixelValue != myPixelValue ) )
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

      if ( mInvertColor )
      {
        //Invert flag, flip blue and read
        imageScanLine[ i ] = qRgba( myBlueValue, myGreenValue, myRedValue, myAlphaValue );
      }
      else
      {
        //Normal
        imageScanLine[ i ] = qRgba( myRedValue, myGreenValue, myBlueValue, myAlphaValue );
      }
    }
  }
}

/**
 * This method is used to render a paletted raster layer as a color image -- currently not supported
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 */
void QgsRasterLayer::drawPalettedMultiBandColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  QgsDebugMsg( "Not supported at this time" );
}

void QgsRasterLayer::drawSingleBandGray( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort, const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  QgsDebugMsg( "layer=" + QString::number( theBandNo ) );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNo )
  {
    return;
  }

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );
  QgsRasterImageBuffer imageBuffer( myGdalBand, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );
  double myGrayValue = 0.0;
  int myGrayVal = 0;
  int myAlphaValue = 0;
  QgsContrastEnhancement* myContrastEnhancement = contrastEnhancement( theBandNo );

  QgsRasterBandStats myGrayBandStats;
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
    double GDALrange[2];
    GDALComputeRasterMinMax( myGdalBand, 1, GDALrange ); //Approximate
    mGrayMinimumMaximumEstimated = true;
    setMaximumValue( theBandNo, GDALrange[1] );
    setMinimumValue( theBandNo, GDALrange[0] );

  }

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      myGrayValue = readValue( rasterScanLine, ( GDALDataType )myDataType, i );

      if ( mValidNoDataValue && ( fabs( myGrayValue - mNoDataValue ) <= TINY_VALUE || myGrayValue != myGrayValue ) )
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

      imageScanLine[ i ] = qRgba( myGrayVal, myGrayVal, myGrayVal, myAlphaValue );
    }
  }
} // QgsRasterLayer::drawSingleBandGray

void QgsRasterLayer::drawSingleBandPseudoColor( QPainter * theQPainter,
    QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel,
    int theBandNo )
{
  QgsDebugMsg( "entered." );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNo )
  {
    return;
  }

  QgsRasterBandStats myRasterBandStats = bandStatistics( theBandNo );
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );

  QgsRasterImageBuffer imageBuffer( myGdalBand, theQPainter, theRasterViewPort, theQgsMapToPixel, &mGeoTransform[0] );
  imageBuffer.reset();

  QRgb* imageScanLine = 0;
  void* rasterScanLine = 0;

  QRgb myDefaultColor = qRgba( 255, 255, 255, 0 );
  if ( NULL == mRasterShader )
  {
    return;
  }

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

  double myPixelValue = 0.0;
  int myAlphaValue = 0;

  while ( imageBuffer.nextScanLine( &imageScanLine, &rasterScanLine ) )
  {
    for ( int i = 0; i < theRasterViewPort->drawableAreaXDim; ++i )
    {
      myPixelValue = readValue( rasterScanLine, myDataType, i );

      if ( mValidNoDataValue && ( fabs( myPixelValue - mNoDataValue ) <= TINY_VALUE || myPixelValue != myPixelValue ) )
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

      if ( mInvertColor )
      {
        //Invert flag, flip blue and read
        imageScanLine[ i ] = qRgba( myBlueValue, myGreenValue, myRedValue, myAlphaValue );
      }
      else
      {
        //Normal
        imageScanLine[ i ] = qRgba( myRedValue, myGreenValue, myBlueValue, myAlphaValue );
      }
    }
  }
}


void QgsRasterLayer::closeDataset()
{
  if ( !mValid ) return;
  mValid = false;

  GDALDereferenceDataset( mGdalBaseDataset );
  mGdalBaseDataset = NULL;

  GDALClose( mGdalDataset );
  mGdalDataset = NULL;

  mHasPyramids = false;
  mPyramidList.clear();

  mRasterStatsList.clear();
}

QString QgsRasterLayer::generateBandName( int theBandNumber )
{
  return tr( "Band" ) + QString( " %1" ) .arg( theBandNumber,  1 + ( int ) log10(( float ) bandCount() ), 10, QChar( '0' ) );
}

/**
 * This method looks to see if a given band name exists.
 *@note This function is no longer really needed and about to be removed
 */
bool QgsRasterLayer::hasBand( QString const & theBandName )
{
  //TODO: This function is no longer really needed and about be removed -- it is only used to see if "Palette" exists which is not the correct way to see if a band is paletted or not
  QgsDebugMsg( "Looking for band : " + theBandName );

  for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); i++ )
  {
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, i );
    QString myColorQString = GDALGetColorInterpretationName( GDALGetRasterColorInterpretation( myGdalBand ) );
#ifdef QGISDEBUG
    QgsLogger::debug( "band", i, __FILE__, __FUNCTION__, __LINE__, 2 );
#endif

    if ( myColorQString == theBandName )
    {
#ifdef QGISDEBUG
      QgsLogger::debug( "band", i, __FILE__, __FUNCTION__, __LINE__, 2 );
      QgsDebugMsgLevel( "Found band : " + theBandName, 2 );
#endif

      return true;
    }
    QgsDebugMsgLevel( "Found unmatched band : " + QString::number( i ) + " " + myColorQString, 2 );
  }
  return false;
}

void QgsRasterLayer::paintImageToCanvas( QPainter* theQPainter, QgsRasterViewPort * theRasterViewPort, const QgsMapToPixel* theQgsMapToPixel, QImage* theImage )
{
  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;

  if ( theQgsMapToPixel )
  {
    paintXoffset = static_cast<int>(
                     ( theRasterViewPort->rectXOffsetFloat -
                       theRasterViewPort->rectXOffset )
                     / theQgsMapToPixel->mapUnitsPerPixel()
                     * fabs( mGeoTransform[1] )
                   );

    paintYoffset = static_cast<int>(
                     ( theRasterViewPort->rectYOffsetFloat -
                       theRasterViewPort->rectYOffset )
                     / theQgsMapToPixel->mapUnitsPerPixel()
                     * fabs( mGeoTransform[5] )
                   );
  }



  QgsDebugMsg( "painting image to canvas from "
               + QString::number( paintXoffset ) + ", " + QString::number( paintYoffset )
               + " to "
               + QString::number( static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ) )
               + ", "
               + QString::number( static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ) )
               + "." );

  //Catch special rendering cases
  //INSTANCE: 1x1
  if ( 1 == theRasterViewPort->clippedWidth && 1 == theRasterViewPort->clippedHeight )
  {
    QColor myColor( theImage->pixel( 0, 0 ) );
    myColor.setAlpha( qAlpha( theImage->pixel( 0, 0 ) ) );
    theQPainter->fillRect( static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ),
                           static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ),
                           static_cast<int>( theRasterViewPort->bottomRightPoint.x() ),
                           static_cast<int>( theRasterViewPort->bottomRightPoint.y() ),
                           QBrush( myColor ) );
  }
  //1x2, 2x1 or 2x2
  else if ( 2 >= theRasterViewPort->clippedWidth && 2 >= theRasterViewPort->clippedHeight )
  {
    int myPixelBoundaryX = 0;
    int myPixelBoundaryY = 0;
    if ( theQgsMapToPixel )
    {
      myPixelBoundaryX = static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ) + static_cast<int>( fabs( mGeoTransform[1] / theQgsMapToPixel->mapUnitsPerPixel() ) ) - paintXoffset;
      myPixelBoundaryY = static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ) + static_cast<int>( fabs( mGeoTransform[5] / theQgsMapToPixel->mapUnitsPerPixel() ) ) - paintYoffset;
    }

    //INSTANCE: 1x2
    if ( 1 == theRasterViewPort->clippedWidth )
    {
      QColor myColor( theImage->pixel( 0, 0 ) );
      myColor.setAlpha( qAlpha( theImage->pixel( 0, 0 ) ) );
      theQPainter->fillRect( static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ),
                             static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ),
                             static_cast<int>( theRasterViewPort->bottomRightPoint.x() ),
                             static_cast<int>( myPixelBoundaryY ),
                             QBrush( myColor ) );
      myColor = QColor( theImage->pixel( 0, 1 ) );
      myColor.setAlpha( qAlpha( theImage->pixel( 0, 1 ) ) );
      theQPainter->fillRect( static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ),
                             static_cast<int>( myPixelBoundaryY ),
                             static_cast<int>( theRasterViewPort->bottomRightPoint.x() ),
                             static_cast<int>( theRasterViewPort->bottomRightPoint.y() ),
                             QBrush( myColor ) );
    }
    else
    {
      //INSTANCE: 2x1
      if ( 1 == theRasterViewPort->clippedHeight )
      {
        QColor myColor( theImage->pixel( 0, 0 ) );
        myColor.setAlpha( qAlpha( theImage->pixel( 0, 0 ) ) );
        theQPainter->fillRect( static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ),
                               static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ),
                               static_cast<int>( myPixelBoundaryX ),
                               static_cast<int>( theRasterViewPort->bottomRightPoint.y() ),
                               QBrush( myColor ) );
        myColor = QColor( theImage->pixel( 1, 0 ) );
        myColor.setAlpha( qAlpha( theImage->pixel( 1, 0 ) ) );
        theQPainter->fillRect( static_cast<int>( myPixelBoundaryX ),
                               static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ),
                               static_cast<int>( theRasterViewPort->bottomRightPoint.x() ),
                               static_cast<int>( theRasterViewPort->bottomRightPoint.y() ),
                               QBrush( myColor ) );
      }
      //INSTANCE: 2x2
      else
      {
        QColor myColor( theImage->pixel( 0, 0 ) );
        myColor.setAlpha( qAlpha( theImage->pixel( 0, 0 ) ) );
        theQPainter->fillRect( static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ),
                               static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ),
                               static_cast<int>( myPixelBoundaryX ),
                               static_cast<int>( myPixelBoundaryY ),
                               QBrush( myColor ) );
        myColor = QColor( theImage->pixel( 1, 0 ) );
        myColor.setAlpha( qAlpha( theImage->pixel( 1, 0 ) ) );
        theQPainter->fillRect( static_cast<int>( myPixelBoundaryX ),
                               static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ),
                               static_cast<int>( theRasterViewPort->bottomRightPoint.x() ),
                               static_cast<int>( myPixelBoundaryY ),
                               QBrush( myColor ) );
        myColor = QColor( theImage->pixel( 0, 1 ) );
        myColor.setAlpha( qAlpha( theImage->pixel( 0, 1 ) ) );
        theQPainter->fillRect( static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ),
                               static_cast<int>( myPixelBoundaryY ),
                               static_cast<int>( myPixelBoundaryX ),
                               static_cast<int>( theRasterViewPort->bottomRightPoint.y() ),
                               QBrush( myColor ) );
        myColor = QColor( theImage->pixel( 1, 1 ) );
        myColor.setAlpha( qAlpha( theImage->pixel( 1, 1 ) ) );
        theQPainter->fillRect( static_cast<int>( myPixelBoundaryX ),
                               static_cast<int>( myPixelBoundaryY ),
                               static_cast<int>( theRasterViewPort->bottomRightPoint.x() ),
                               static_cast<int>( theRasterViewPort->bottomRightPoint.y() ),
                               QBrush( myColor ) );
      }
    }

  }
  // INSTANCE: > 2x2, so just use the image filled by GDAL
  else
  {
    theQPainter->drawImage( static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ),
                            static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ),
                            *theImage,
                            paintXoffset,
                            paintYoffset );
  }
}

QString QgsRasterLayer::projectionWkt()
{
  QString myWktString;
  QgsCoordinateReferenceSystem myCRS;
  myWktString = QString( GDALGetProjectionRef( mGdalDataset ) );
  myCRS.createFromWkt( myWktString );
  if ( !myCRS.isValid() )
  {
    //try to get the gcp srs from the raster layer if available
    myWktString = QString( GDALGetGCPProjection( mGdalDataset ) );

// What is the purpose of this piece of code?
// Sideeffects from validate()?
//    myCRS.createFromWkt(myWktString);
//    if (!myCRS.isValid())
//    {
//      // use force and make CRS valid!
//      myCRS.validate();
//    }

  }

  return myWktString;
}

/*
 *data type is the same as raster band. The memory must be released later!
 *  \return pointer to the memory
 */
void *QgsRasterLayer::readData( GDALRasterBandH gdalBand, QgsRasterViewPort *viewPort )
{
  GDALDataType type = GDALGetRasterDataType( gdalBand );
  int size = GDALGetDataTypeSize( type ) / 8;

  QgsDebugMsg( "calling RasterIO with " +
               QString( ", source NW corner: " ) + QString::number( viewPort->rectXOffset ) +
               ", " + QString::number( viewPort->rectYOffset ) +
               ", source size: " + QString::number( viewPort->clippedWidth ) +
               ", " + QString::number( viewPort->clippedHeight ) +
               ", dest size: " + QString::number( viewPort->drawableAreaXDim ) +
               ", " + QString::number( viewPort->drawableAreaYDim ) );

  void *data = VSIMalloc( size * viewPort->drawableAreaXDim * viewPort->drawableAreaYDim );

  /* Abort if out of memory */
  if ( data == NULL )
  {
    QgsDebugMsg( "Layer " + name() + " couldn't allocate enough memory. Ignoring" );
  }
  else
  {
    CPLErr myErr = GDALRasterIO( gdalBand, GF_Read,
                                 viewPort->rectXOffset,
                                 viewPort->rectYOffset,
                                 viewPort->clippedWidth,
                                 viewPort->clippedHeight,
                                 data,
                                 viewPort->drawableAreaXDim,
                                 viewPort->drawableAreaYDim,
                                 type, 0, 0 );
    if ( myErr != CPLE_None )
    {
      QgsLogger::warning( "RasterIO error: " + QString::fromUtf8( CPLGetLastErrorMsg() ) );
    }
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
  registerGdalDrivers();

  mGdalDataset = NULL;

  //open the dataset making sure we handle char encoding of locale properly
  mGdalBaseDataset = GDALOpen( QFile::encodeName( theFilename ).constData(), GA_ReadOnly );

  if ( mGdalBaseDataset == NULL )
  {
    mValid = false;
    return false;
  }

  // Store timestamp
  mLastModified = lastModified( theFilename );

  // Check if we need a warped VRT for this file.
  if (( GDALGetGeoTransform( mGdalBaseDataset, mGeoTransform ) == CE_None
        && ( mGeoTransform[1] < 0.0
             || mGeoTransform[2] != 0.0
             || mGeoTransform[4] != 0.0
             || mGeoTransform[5] > 0.0 ) )
      || GDALGetGCPCount( mGdalBaseDataset ) > 0 )
  {
    QgsLogger::warning( "Creating Warped VRT." );

    mGdalDataset =
      GDALAutoCreateWarpedVRT( mGdalBaseDataset, NULL, NULL,
                               GRA_NearestNeighbour, 0.2, NULL );
    if ( mGdalDataset == NULL )
    {
      QgsLogger::warning( "Warped VRT Creation failed." );
      mGdalDataset = mGdalBaseDataset;
      GDALReferenceDataset( mGdalDataset );
    }
  }
  else
  {
    mGdalDataset = mGdalBaseDataset;
    GDALReferenceDataset( mGdalDataset );
  }

  //check f this file has pyramids
  GDALRasterBandH myGDALBand = GDALGetRasterBand( mGdalDataset, 1 ); //just use the first band
  if ( myGDALBand == NULL )
  {
    GDALDereferenceDataset( mGdalBaseDataset );
    mGdalBaseDataset = NULL;

    GDALClose( mGdalDataset );
    mGdalDataset = NULL;
    mValid = false;
    return false;
  }
  if ( GDALGetOverviewCount( myGDALBand ) > 0 )
  {
    mHasPyramids = true;
  }
  else
  {
    mHasPyramids = false;
  }

  //populate the list of what pyramids exist
  buildPyramidList();

  // Get the layer's projection info and set up the
  // QgsCoordinateTransform for this layer
  // NOTE: we must do this before metadata is called

  QString mySourceWkt = projectionWkt();

  QgsDebugMsg( "--------------------------------------------------------------------------------------" );
  QgsDebugMsg( "using wkt:\n" + mySourceWkt );
  QgsDebugMsg( "--------------------------------------------------------------------------------------" );

  mCRS->createFromWkt( mySourceWkt );
  //get the project projection, defaulting to this layer's projection
  //if none exists....
  if ( !mCRS->isValid() )
  {
    mCRS->validate();
  }

  //set up the coordinat transform - in the case of raster this is mainly used to convert
  //the inverese projection of the map extents of the canvas when zzooming in etc. so
  //that they match the coordinate system of this layer
  QgsDebugMsg( "Layer registry has " + QString::number( QgsMapLayerRegistry::instance()->count() ) + "layers" );

  metadata();

  // Use the affine transform to get geo coordinates for
  // the corners of the raster
  double myXMax = mGeoTransform[0] +
                  GDALGetRasterXSize( mGdalDataset ) * mGeoTransform[1] +
                  GDALGetRasterYSize( mGdalDataset ) * mGeoTransform[2];
  double myYMin = mGeoTransform[3] +
                  GDALGetRasterXSize( mGdalDataset ) * mGeoTransform[4] +
                  GDALGetRasterYSize( mGdalDataset ) * mGeoTransform[5];

  mLayerExtent.setXMaximum( myXMax );
  // The affine transform reduces to these values at the
  // top-left corner of the raster
  mLayerExtent.setXMinimum( mGeoTransform[0] );
  mLayerExtent.setYMaximum( mGeoTransform[3] );
  mLayerExtent.setYMinimum( myYMin );

  //
  // Set up the x and y dimensions of this raster layer
  //
  mWidth = GDALGetRasterXSize( mGdalDataset );
  mHeight = GDALGetRasterYSize( mGdalDataset );

  //
  // Determine the nodata value
  //
  mNoDataValue = -9999.0; //Standard default?
  mValidNoDataValue = false;
  int isValid = false;
  double myNoDataValue = GDALGetRasterNoDataValue( GDALGetRasterBand( mGdalDataset, 1 ), &isValid );
  if ( isValid )
  {
    mNoDataValue = myNoDataValue;
    mValidNoDataValue = true;
  }

  if ( mValidNoDataValue )
  {
    mRasterTransparency.initializeTransparentPixelList( mNoDataValue, mNoDataValue, mNoDataValue );
    mRasterTransparency.initializeTransparentPixelList( mNoDataValue );
  }

  mBandCount = GDALGetRasterCount( mGdalDataset );
  for ( int i = 1; i <= mBandCount; i++ )
  {
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, i );
    QgsRasterBandStats myRasterBandStats;
    myRasterBandStats.bandName = generateBandName( i );
    myRasterBandStats.bandNumber = i;
    myRasterBandStats.statsGathered = false;
    myRasterBandStats.histogramVector = new QgsRasterBandStats::HistogramVector();
    //Store the default color table
    readColorTable( i, &myRasterBandStats.colorTable );

    mRasterStatsList.push_back( myRasterBandStats );

    //Build a new contrast enhancement for the band and store in list
    QgsContrastEnhancement myContrastEnhancement(( QgsContrastEnhancement::QgsRasterDataType )GDALGetRasterDataType( myGdalBand ) );
    mContrastEnhancementList.append( myContrastEnhancement );
  }

  //defaults - Needs to be set after the Contrast list has been build
  //Try to read the default contrast enhancement from the config file
  QSettings myQSettings;
  setContrastEnhancementAlgorithm( myQSettings.value( "/Raster/defaultContrastEnhancementAlgorithm", "StretchToMinimumMaximum" ).toString() );

  //decide what type of layer this is...
  //TODO Change this to look at the color interp and palette interp to decide which type of layer it is
  if (( GDALGetRasterCount( mGdalDataset ) > 1 ) )
  {
    mRasterType = Multiband;
  }
  //TODO hasBand is really obsolete and only used in the Palette instance, change to new function hasPalette(int)
  else if ( hasBand( "Palette" ) ) //don't tr() this its a gdal word!
  {
    mRasterType = Palette;
  }
  else
  {
    mRasterType = GrayOrUndefined;
  }

  if ( mRasterType == Palette )
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

    //for the third layer we cant be sure so..
    if ( GDALGetRasterCount( mGdalDataset ) > 2 )
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
  }
  else                        //GrayOrUndefined
  {
    mRedBandName = TRSTRING_NOT_SET; //sensible default
    mGreenBandName = TRSTRING_NOT_SET; //sensible default
    mBlueBandName = TRSTRING_NOT_SET;  //sensible default
    mTransparencyBandName = TRSTRING_NOT_SET;  //sensible default
    mDrawingStyle = SingleBandGray;  //sensible default
    mGrayBandName = bandName( 1 );
  }

  //mark the layer as valid
  mValid = true;
  return true;

} // QgsRasterLayer::readFile

/*
 *  @param index index in memory block
 */
double QgsRasterLayer::readValue( void *data, GDALDataType type, int index )
{
  if ( !data )
    return mValidNoDataValue ? mNoDataValue : 0.0;

  switch ( type )
  {
    case GDT_Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case GDT_UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case GDT_Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case GDT_UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case GDT_Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case GDT_Float32:
      return ( double )(( float * )data )[index];
      break;
    case GDT_Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      QgsLogger::warning( "GDAL data type is not supported" );
  }

  return mValidNoDataValue ? mNoDataValue : 0.0;
}

bool QgsRasterLayer::update()
{
  QgsDebugMsg( "entered." );

  if ( mLastModified < QgsRasterLayer::lastModified( source() ) )
  {
    if ( !usesProvider() )
    {
      QgsDebugMsg( "Outdated -> reload" );
      closeDataset();
      return readFile( source() );
    }
  }
  return true;
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
      QString myBandName = generateBandName( myBandNumber );
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

QgsRasterImageBuffer::QgsRasterImageBuffer( GDALRasterBandH rasterBand, QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* mapToPixel, double* geoTransform ):
    mRasterBand( rasterBand ), mPainter( p ), mViewPort( viewPort ), mMapToPixel( mapToPixel ), mGeoTransform( geoTransform ), mValid( false ), mWritingEnabled( true ), mDrawPixelRect( false ), mCurrentImage( 0 ), mCurrentGDALData( 0 )
{

}

QgsRasterImageBuffer::~QgsRasterImageBuffer()
{
  delete mCurrentImage;
  CPLFree( mCurrentGDALData );
}

void QgsRasterImageBuffer::reset( int maxPixelsInVirtualMemory )
{
  if ( !mRasterBand || !mPainter || !mViewPort )
  {
    mValid = false;
    return;
  }

  mValid = true;

  //decide on the partition of the image

  int pixels = mViewPort->drawableAreaXDim * mViewPort->drawableAreaYDim;
  int mNumPartImages = pixels / maxPixelsInVirtualMemory + 1.0;
  mNumRasterRowsPerPart = ( double )mViewPort->clippedHeight / ( double )mNumPartImages + 0.5;

  mCurrentPartRasterMin = -1;
  mCurrentPartRasterMax = -1;
  mCurrentPartImageRow = 0;
  mNumCurrentImageRows = 0;

  createNextPartImage();

  if ( 2 >= mViewPort->clippedWidth && 2 >= mViewPort->clippedHeight )
  {
    //use Peter's fix for zoomed in rasters
    mDrawPixelRect = true;
  }
}

bool QgsRasterImageBuffer::nextScanLine( QRgb** imageScanLine, void** rasterScanLine )
{
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
  GDALDataType type = GDALGetRasterDataType( mRasterBand );
  int size = GDALGetDataTypeSize( type ) / 8;
  *rasterScanLine = ( unsigned char * )mCurrentGDALData + mCurrentPartImageRow * mViewPort->drawableAreaXDim * size;

  ++mCurrentPartImageRow;
  ++mCurrentRow;
  return !mWritingEnabled || *imageScanLine;
}

bool QgsRasterImageBuffer::createNextPartImage()
{
  //draw the last image if mCurrentImage exists
  if ( mCurrentImage )
  {
    if ( mWritingEnabled )
    {
      if ( 2 >= mViewPort->clippedWidth && 2 >= mViewPort->clippedHeight )
      {
        drawPixelRectangle();
      }
      else
      {
        int paintXoffset = 0;
        int paintYoffset = 0;
        int imageX = 0;
        int imageY = 0;

        if ( mMapToPixel )
        {
          paintXoffset = static_cast<int>(
                           ( mViewPort->rectXOffsetFloat -
                             mViewPort->rectXOffset )
                           / mMapToPixel->mapUnitsPerPixel()
                           * fabs( mGeoTransform[1] )
                         );

          paintYoffset = static_cast<int>(
                           ( mViewPort->rectYOffsetFloat -
                             mViewPort->rectYOffset )
                           / mMapToPixel->mapUnitsPerPixel()
                           * fabs( mGeoTransform[5] )
                         );

          imageX = static_cast<int>( mViewPort->topLeftPoint.x() + 0.5 );
          imageY = static_cast<int>( mViewPort->topLeftPoint.y() + 0.5 +  fabs( mGeoTransform[5] ) * mCurrentPartRasterMin / mMapToPixel->mapUnitsPerPixel() );
        }

        mPainter->drawImage( imageX,
                             imageY,
                             *mCurrentImage,
                             paintXoffset,
                             paintYoffset );
      }
    }
  }

  delete mCurrentImage; mCurrentImage = 0;
  CPLFree( mCurrentGDALData ); mCurrentGDALData = 0;

  if ( mCurrentPartRasterMax >= mViewPort->clippedHeight )
  {
    return false; //already at the end...
  }

  mCurrentPartRasterMin = mCurrentPartRasterMax + 1;
  mCurrentPartRasterMax = mCurrentPartRasterMin + mNumRasterRowsPerPart;
  if ( mCurrentPartRasterMax > mViewPort->clippedHeight )
  {
    mCurrentPartRasterMax = mViewPort->clippedHeight;
  }
  mCurrentRow = mCurrentPartRasterMin;
  mCurrentPartImageRow = 0;

  //read GDAL image data
  GDALDataType type = GDALGetRasterDataType( mRasterBand );
  int size = GDALGetDataTypeSize( type ) / 8;
  int xSize = mViewPort->drawableAreaXDim;
  int ySize = mViewPort->drawableAreaYDim;

  //make the raster tiles overlap at least 2 pixels to avoid white stripes
  int overlapRows = 0;
  if ( mMapToPixel )
  {
    overlapRows = mMapToPixel->mapUnitsPerPixel() / fabs( mGeoTransform[5] ) + 2;
  }
  if ( mCurrentPartRasterMax + overlapRows >= mViewPort->clippedHeight )
  {
    overlapRows = 0;
  }
  int rasterYSize = mCurrentPartRasterMax - mCurrentPartRasterMin + overlapRows;

  if ( 2 >= mViewPort->clippedWidth && 2 >= mViewPort->clippedHeight ) //for zoomed in rasters
  {
    rasterYSize = mViewPort->clippedHeight;
    ySize = mViewPort->drawableAreaYDim;
  }
  else //normal mode
  {
    if ( mMapToPixel )
    {
      ySize = fabs((( rasterYSize ) / mMapToPixel->mapUnitsPerPixel() * mGeoTransform[5] ) ) + 0.5;
    }
  }
  if ( ySize < 1 || xSize < 1 )
  {
    return false;
  }
  mNumCurrentImageRows = ySize;
  mCurrentGDALData = VSIMalloc( size * xSize * ySize );
  CPLErr myErr = GDALRasterIO( mRasterBand, GF_Read, mViewPort->rectXOffset,
                               mViewPort->rectYOffset + mCurrentRow, mViewPort->clippedWidth, rasterYSize,
                               mCurrentGDALData, xSize, ySize, type, 0, 0 );

  if ( myErr != CPLE_None )
  {
    CPLFree( mCurrentGDALData );
    mCurrentGDALData = 0;
    return false;
  }

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
                     * fabs( mGeoTransform[1] )
                   );

    paintYoffset = static_cast<int>(
                     ( mViewPort->rectYOffsetFloat -
                       mViewPort->rectYOffset )
                     / mMapToPixel->mapUnitsPerPixel()
                     * fabs( mGeoTransform[5] )
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
      myPixelBoundaryX = static_cast<int>( mViewPort->topLeftPoint.x() + 0.5 ) + static_cast<int>( fabs( mGeoTransform[1] / mMapToPixel->mapUnitsPerPixel() ) ) - paintXoffset;
      myPixelBoundaryY = static_cast<int>( mViewPort->topLeftPoint.y() + 0.5 ) + static_cast<int>( fabs( mGeoTransform[5] / mMapToPixel->mapUnitsPerPixel() ) ) - paintYoffset;
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
}
