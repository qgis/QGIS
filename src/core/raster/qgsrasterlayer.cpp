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
#include "qgsrasterviewport.h"
#include "qgsrect.h"
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

//////////////////////////////////////////////////////////
//
// Static Methods and members first....
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
  // extension defined for them; the others, welll, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.
  // Note that file name extension strings are of the form
  // "DMD_EXTENSION=.*".  We'll also store the long name of the
  // driver, which will be found in DMD_LONGNAME, which will have the
  // same form.

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
        QString glob = "*." + myGdalDriverExtension;
        // Add only the first JP2 driver found to the filter list (it's the one GDAL uses)
        if ( myGdalDriverDescription == "JPEG2000" ||
             myGdalDriverDescription.startsWith( "JP2" ) ) // JP2ECW, JP2KAK, JP2MrSID
        {
          if ( !jp2Driver )
          {
            jp2Driver = myGdalDriver;   // first JP2 driver found
            glob += " *.j2k";           // add alternate extension
          }
          else break;               // skip if already found a JP2 driver
        }
        theFileFiltersString += myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ");;";

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
        theFileFiltersString += myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ");;";
      }
      else if ( myGdalDriverDescription.startsWith( "DTED" ) )
      {
        // DTED use "*.dt0"
        QString glob = "*.dt0";
        theFileFiltersString += myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ");;";
      }
      else if ( myGdalDriverDescription.startsWith( "MrSID" ) )
      {
        // MrSID use "*.sid"
        QString glob = "*.sid";
        theFileFiltersString += myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ");;";
      }
      else
      {
        catchallFilter << QString( GDALGetDescription( myGdalDriver ) );
      }
    }

    myGdalDriverExtension = myGdalDriverLongName = "";  // reset for next driver

  }                           // each loaded GDAL driver

  // can't forget the default case
  theFileFiltersString += catchallFilter.join( ", " ) + " " + tr( "and all other files" ) + " (*)";
  QgsDebugMsg( "Raster filter list built: " + theFileFiltersString );
}                               // buildSupportedRasterFileFilter_()

/**
 ensures that GDAL drivers are registered, but only once.
*/

void QgsRasterLayer::registerGdalDrivers()
{
  if ( GDALGetDriverCount() == 0 )
    GDALAllRegister();
}


/** This helper checks to see whether the file name appears to be a valid raster file name */
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
      retErrMsg = CPLGetLastErrorMsg();
    return false;
  }
  else if ( GDALGetRasterCount( myDataset ) == 0 )
  {
    GDALClose( myDataset );
    myDataset = NULL;
    retErrMsg = "This raster file has no bands and is invalid as a raster layer.";
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


//////////////////////////////////////////////////////////
//
// Non Static methods now....
//
/////////////////////////////////////////////////////////
QgsRasterLayer::QgsRasterLayer(
  QString const & path,
  QString const & baseName,
  bool loadDefaultStyleFlag )
    : QgsMapLayer( RASTER, baseName, path ),
    // Constant that signals property not used.
    QSTRING_NOT_SET( "Not Set" ),
    TRSTRING_NOT_SET( tr( "Not Set" ) ),
    mRasterXDim( std::numeric_limits<int>::max() ),
    mRasterYDim( std::numeric_limits<int>::max() ),
    mInvertPixelsFlag( false ),
    mStandardDeviations( 0 ),
    mDataProvider( 0 )
{

  mUserDefinedRGBMinMaxFlag = false; //defaults needed to bypass stretch
  mUserDefinedGrayMinMaxFlag = false;
  mRGBActualMinimumMaximum = false;
  mGrayActualMinimumMaximum = false;

  mRasterShader = new QgsRasterShader();

  if ( loadDefaultStyleFlag )
  {
    bool defaultLoadedFlag = false;
    loadDefaultStyle( defaultLoadedFlag );
    if ( defaultLoadedFlag )
    {
      return;
    }
  }

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
  }

} // QgsRasterLayer ctor


QgsRasterLayer::~QgsRasterLayer()
{

  if ( mProviderKey.isEmpty() )
  {
    if ( mGdalBaseDataset )
    {
      GDALDereferenceDataset( mGdalBaseDataset );
      GDALClose( mGdalDataset );
    }
  }
}



bool QgsRasterLayer::readFile( QString const & fileName )
{
  registerGdalDrivers();

  mGdalDataset = NULL;

  //open the dataset making sure we handle char encoding of locale properly
  mGdalBaseDataset = GDALOpen( QFile::encodeName( fileName ).constData(), GA_ReadOnly );

  if ( mGdalBaseDataset == NULL )
  {
    mValid = FALSE;
    return false;
  }

  // Store timestamp
  mLastModified = lastModified( fileName );

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
    mValid = FALSE;
    return false;
  }
  if ( GDALGetOverviewCount( myGDALBand ) > 0 )
  {
    hasPyramidsFlag = true;
  }
  else
  {
    hasPyramidsFlag = false;
  }

  //populate the list of what pyramids exist
  buildRasterPyramidList();

  // Get the layer's projection info and set up the
  // QgsCoordinateTransform for this layer
  // NOTE: we must do this before getMetadata is called

  QgsDebugMsg( "Raster initial CRS" );
  mCRS->debugPrint();

  QString mySourceWKT = getProjectionWKT();

  QgsDebugMsg( "--------------------------------------------------------------------------------------" );
  QgsDebugMsg( "using wkt:\n" + mySourceWKT );
  QgsDebugMsg( "--------------------------------------------------------------------------------------" );

  mCRS->createFromWkt( mySourceWKT );
  //get the project projection, defaulting to this layer's projection
  //if none exists....
  if ( !mCRS->isValid() )
  {
    mCRS->validate();
  }
  QgsDebugMsg( "Raster determined to have the following CRS" );
  mCRS->debugPrint();

  //set up the coordinat transform - in the case of raster this is mainly used to convert
  //the inverese projection of the map extents of the canvas when zzooming in etc. so
  //that they match the coordinate system of this layer
  QgsDebugMsg( "Layer registry has " + QString::number( QgsMapLayerRegistry::instance()->count() ) + "layers" );

  getMetadata();

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
  mRasterXDim = GDALGetRasterXSize( mGdalDataset );
  mRasterYDim = GDALGetRasterYSize( mGdalDataset );

  //
  // Determin the nodatavalue
  //
  mNoDataValue = -9999; //Standard default?
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

  //initialise the raster band stats and contrast enhancement vector
  for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); i++ )
  {
    GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, i );
    QgsRasterBandStats myRasterBandStats;
    //myRasterBandStats.bandName = myColorQString ;
    myRasterBandStats.bandName = "Band " + QString::number( i );
    myRasterBandStats.bandNo = i;
    myRasterBandStats.statsGatheredFlag = false;
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
  setContrastEnhancementAlgorithm( myQSettings.value( "/Raster/defaultContrastEnhancementAlgorithm", "STRETCH_TO_MINMAX" ).toString() );

  //decide what type of layer this is...
  //TODO Change this to look at the color interp and palette interp to decide which type of layer it is
  if (( GDALGetRasterCount( mGdalDataset ) > 1 ) )
  {
    rasterLayerType = MULTIBAND;
  }
  //TODO hasBand is really obsolete and only used in the Palette instance, change to new function hasPalette(int)
  else if ( hasBand( "Palette" ) ) //dont tr() this its a gdal word!
  {
    rasterLayerType = PALETTE;
  }
  else
  {
    rasterLayerType = GRAY_OR_UNDEFINED;
  }

  if ( rasterLayerType == PALETTE )
  {
    mRedBandName = TRSTRING_NOT_SET; // sensible default
    mGreenBandName = TRSTRING_NOT_SET; // sensible default
    mBlueBandName = TRSTRING_NOT_SET;// sensible default
    mTransparencyBandName = TRSTRING_NOT_SET; // sensible default
    mGrayBandName = getRasterBandName( 1 );  //sensible default
    QgsDebugMsg( mGrayBandName );

    drawingStyle = PALETTED_COLOR; //sensible default

    //Set up a new color ramp shader
    setColorShadingAlgorithm( COLOR_RAMP );
    QgsColorRampShader* myColorRampShader = ( QgsColorRampShader* ) mRasterShader->getRasterShaderFunction();
    myColorRampShader->setColorRampType( QgsColorRampShader::INTERPOLATED );
    myColorRampShader->setColorRampItemList( *getColorTable( 1 ) );
  }
  else if ( rasterLayerType == MULTIBAND )
  {
    //we know we have at least 2 layers...
    mRedBandName = getRasterBandName( myQSettings.value( "/Raster/defaultRedBand", 1 ).toInt() );  // sensible default
    mGreenBandName = getRasterBandName( myQSettings.value( "/Raster/defaultGreenBand", 2 ).toInt() );  // sensible default
    //for the third layer we cant be sure so..
    if ( GDALGetRasterCount( mGdalDataset ) > 2 )
    {
      mBlueBandName = getRasterBandName( myQSettings.value( "/Raster/defaultBlueBand", 3 ).toInt() ); // sensible default
    }
    else
    {
      mBlueBandName = getRasterBandName( myQSettings.value( "/Raster/defaultBlueBand", 2 ).toInt() );  // sensible default
    }

    mTransparencyBandName = TRSTRING_NOT_SET;
    mGrayBandName = TRSTRING_NOT_SET;  //sensible default
    drawingStyle = MULTI_BAND_COLOR;  //sensible default
  }
  else                        //GRAY_OR_UNDEFINED
  {
    mRedBandName = TRSTRING_NOT_SET; //sensible default
    mGreenBandName = TRSTRING_NOT_SET; //sensible default
    mBlueBandName = TRSTRING_NOT_SET;  //sensible default
    mTransparencyBandName = TRSTRING_NOT_SET;  //sensible default
    drawingStyle = SINGLE_BAND_GRAY;  //sensible default
    mGrayBandName = getRasterBandName( 1 );
  }

  //mark the layer as valid
  mValid = TRUE;
  return true;

} // QgsRasterLayer::readFile

QString QgsRasterLayer::getProjectionWKT()
{
  QString myWKTString;
  QgsCoordinateReferenceSystem myCRS;
  myWKTString = QString( GDALGetProjectionRef( mGdalDataset ) );
  myCRS.createFromWkt( myWKTString );
  if ( !myCRS.isValid() )
  {
    //try to get the gcp srs from the raster layer if available
    myWKTString = QString( GDALGetGCPProjection( mGdalDataset ) );

// What is the purpose of this piece of code?
// Sideeffects from validate()?
//    myCRS.createFromWkt(myWKTString);
//    if (!myCRS.isValid())
//    {
//      // use force and make CRS valid!
//      myCRS.validate();
//    }

  }

  return myWKTString;
}

void QgsRasterLayer::closeDataset()
{
  if ( !mValid ) return;
  mValid = FALSE;

  GDALDereferenceDataset( mGdalBaseDataset );
  mGdalBaseDataset = NULL;

  GDALClose( mGdalDataset );
  mGdalDataset = NULL;

  hasPyramidsFlag = false;
  mPyramidList.clear();

  mRasterStatsList.clear();
}

bool QgsRasterLayer::update()
{
  QgsDebugMsg( "entered." );

  if ( mLastModified < QgsRasterLayer::lastModified( source() ) )
  {
    QgsDebugMsg( "Outdated -> reload" );
    closeDataset();
    return readFile( source() );
  }
  return true;
}

QDateTime QgsRasterLayer::lastModified( QString const & name )
{
  QgsDebugMsg( "name=" + name );
  QDateTime t;

  QFileInfo fi( name );

  // Is it file?
  if ( !fi.exists() ) return t;

  t = fi.lastModified();

  // Check also color table for GRASS
  if ( name.contains( "cellhd" ) > 0 )
  { // most probably GRASS
    QString dir = fi.path();
    QString map = fi.fileName();
    fi.setFile( dir + "/../colr/" + map );

    if ( fi.exists() )
    {
      if ( fi.lastModified() > t ) t = fi.lastModified();
    }
  }

  // Check GRASS group members (bands)
  if ( name.contains( "group" ) > 0 )
  { // probably GRASS group
    fi.setFile( name + "/REF" );

    if ( fi.exists() )
    {  // most probably GRASS group
      QFile f( name + "/REF" );
      if ( f.open( QIODevice::ReadOnly ) )
      {
        QString dir = fi.path() + "/../../../";

        char buf[101];
        while ( f.readLine( buf, 100 ) != -1 )
        {
          QString ln = QString( buf );
          QStringList sl = ln.trimmed().split( ' ', QString::SkipEmptyParts );
          QString map = sl.first();
          sl.pop_front();
          QString mapset = sl.first();

          // header
          fi.setFile( dir + mapset + "/cellhd/" +  map );
          if ( fi.exists() )
          {
            if ( fi.lastModified() > t ) t = fi.lastModified();
          }

          // color
          fi.setFile( dir + mapset + "/colr/" +  map );
          if ( fi.exists() )
          {
            if ( fi.lastModified() > t ) t = fi.lastModified();
          }
        }
      }
    }
  }

  QgsDebugMsg( "last modified = " + t.toString() );

  return t;
}


// emit a signal asking for a repaint
void QgsRasterLayer::triggerRepaint()
{
  emit repaintRequested();
}


QString QgsRasterLayer::getDrawingStyleAsQString()
{
  switch ( drawingStyle )
  {
    case SINGLE_BAND_GRAY:
      return QString( "SINGLE_BAND_GRAY" ); //no need to tr() this its not shown in ui
      break;
    case SINGLE_BAND_PSEUDO_COLOR:
      return QString( "SINGLE_BAND_PSEUDO_COLOR" );//no need to tr() this its not shown in ui
      break;
    case PALETTED_COLOR:
      return QString( "PALETTED_COLOR" );//no need to tr() this its not shown in ui
      break;
    case PALETTED_SINGLE_BAND_GRAY:
      return QString( "PALETTED_SINGLE_BAND_GRAY" );//no need to tr() this its not shown in ui
      break;
    case PALETTED_SINGLE_BAND_PSEUDO_COLOR:
      return QString( "PALETTED_SINGLE_BAND_PSEUDO_COLOR" );//no need to tr() this its not shown in ui
      break;
    case PALETTED_MULTI_BAND_COLOR:
      return QString( "PALETTED_MULTI_BAND_COLOR" );//no need to tr() this its not shown in ui
      break;
    case MULTI_BAND_SINGLE_BAND_GRAY:
      return QString( "MULTI_BAND_SINGLE_BAND_GRAY" );//no need to tr() this its not shown in ui
      break;
    case MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR:
      return QString( "MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR" );//no need to tr() this its not shown in ui
      break;
    case MULTI_BAND_COLOR:
      return QString( "MULTI_BAND_COLOR" );//no need to tr() this its not shown in ui
      break;
    default:
      break;
  }

  return QString( "UNDEFINED_DRAWING_STYLE" );

}

void QgsRasterLayer::setDrawingStyle( QString const & theDrawingStyleQString )
{
  if ( theDrawingStyleQString == "SINGLE_BAND_GRAY" )//no need to tr() this its not shown in ui
  {
    drawingStyle = SINGLE_BAND_GRAY;
  }
  else if ( theDrawingStyleQString == "SINGLE_BAND_PSEUDO_COLOR" )//no need to tr() this its not shown in ui
  {
    drawingStyle = SINGLE_BAND_PSEUDO_COLOR;
  }
  else if ( theDrawingStyleQString == "PALETTED_COLOR" )//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_COLOR;
  }
  else if ( theDrawingStyleQString == "PALETTED_SINGLE_BAND_GRAY" )//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_SINGLE_BAND_GRAY;
  }
  else if ( theDrawingStyleQString == "PALETTED_SINGLE_BAND_PSEUDO_COLOR" )//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_SINGLE_BAND_PSEUDO_COLOR;
  }
  else if ( theDrawingStyleQString == "PALETTED_MULTI_BAND_COLOR" )//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_MULTI_BAND_COLOR;
  }
  else if ( theDrawingStyleQString == "MULTI_BAND_SINGLE_BAND_GRAY" )//no need to tr() this its not shown in ui
  {
    drawingStyle = MULTI_BAND_SINGLE_BAND_GRAY;
  }
  else if ( theDrawingStyleQString == "MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR" )//no need to tr() this its not shown in ui
  {
    drawingStyle = MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR;
  }
  else if ( theDrawingStyleQString == "MULTI_BAND_COLOR" )//no need to tr() this its not shown in ui
  {
    drawingStyle = MULTI_BAND_COLOR;
  }
  else
  {
    drawingStyle = UNDEFINED_DRAWING_STYLE;
  }
}


/** This method looks to see if a given band name exists.

  @note

  muliband layers may have more than one "Undefined" band!
  */
bool QgsRasterLayer::hasBand( QString const & theBandName )
{
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

void QgsRasterLayer::drawThumbnail( QPixmap * theQPixmap )
{
  theQPixmap->fill(); //defaults to white

  // Raster providers are disabled (for the moment)
  if ( mProviderKey.isEmpty() )
  {
    QgsRasterViewPort *myRasterViewPort = new QgsRasterViewPort();
    myRasterViewPort->rectXOffset = 0;
    myRasterViewPort->rectYOffset = 0;
    myRasterViewPort->clippedXMin = 0;
    myRasterViewPort->clippedXMax = mRasterXDim;
    myRasterViewPort->clippedYMin = mRasterYDim;
    myRasterViewPort->clippedYMax = 0;
    myRasterViewPort->clippedWidth   = mRasterXDim;
    myRasterViewPort->clippedHeight  = mRasterYDim;
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



QPixmap QgsRasterLayer::getPaletteAsPixmap( int theBandNumber )
{
  QgsDebugMsg( "entered." );

  // Only do this for the non-provider (hard-coded GDAL) scenario...
  // Maybe WMS can do this differently using QImage::numColors and QImage::color()
  if ( mProviderKey.isEmpty() && hasBand( "Palette" ) && theBandNumber > 0 ) //dont tr() this its a gdal word!
  {
    QgsDebugMsg( "....found paletted image" );
    QgsColorRampShader myShader;
    QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = myShader.getColorRampItemList();

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
        for ( int myCol = 0; myCol < mySize; myCol++ )
        {

          myValue = myStep * ( double )( myCol + myRow * mySize );
          int c1, c2, c3;
          myShader.generateShadedValue( myValue, &c1, &c2, &c3 );
          myQImage.setPixel( myCol, myRow, qRgb( c1, c2, c3 ) );
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

bool QgsRasterLayer::draw( QgsRenderContext& rendererContext )
{
  QgsDebugMsg( "entered. (renderContext)" );

  //Dont waste time drawing if transparency is at 0 (completely transparent)
  if ( mTransparencyLevel == 0 )
    return TRUE;

  QgsDebugMsg( "checking timestamp." );

  // Check timestamp
  if ( !update() )
  {
    return FALSE;
  }

  const QgsMapToPixel& theQgsMapToPixel = rendererContext.mapToPixel();
  const QgsRect& theViewExtent = rendererContext.extent();
  QPainter* theQPainter = rendererContext.painter();

  if ( !theQPainter )
  {
    return false;
  }

  // clip raster extent to view extent
  QgsRect myRasterExtent = theViewExtent.intersect( &mLayerExtent );
  if ( myRasterExtent.isEmpty() )
  {
    // nothing to do
    return TRUE;
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
  myRasterViewPort->rectXOffsetFloat = ( theViewExtent.xMin() - mLayerExtent.xMin() ) / fabs( mGeoTransform[1] );
  myRasterViewPort->rectYOffsetFloat = ( mLayerExtent.yMax() - theViewExtent.yMax() ) / fabs( mGeoTransform[5] );

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

  QgsDebugMsg( QString( "mGeoTransform[0] = %1" ).arg( mGeoTransform[0] ) );
  QgsDebugMsg( QString( "mGeoTransform[1] = %1" ).arg( mGeoTransform[1] ) );
  QgsDebugMsg( QString( "mGeoTransform[2] = %1" ).arg( mGeoTransform[2] ) );
  QgsDebugMsg( QString( "mGeoTransform[3] = %1" ).arg( mGeoTransform[3] ) );
  QgsDebugMsg( QString( "mGeoTransform[4] = %1" ).arg( mGeoTransform[4] ) );
  QgsDebugMsg( QString( "mGeoTransform[5] = %1" ).arg( mGeoTransform[5] ) );

  // get dimensions of clipped raster image in raster pixel space/ RasterIO will do the scaling for us.
  // So for example, if the user is zoomed in a long way, there may only be e.g. 5x5 pixels retrieved from
  // the raw raster data, but rasterio will seamlessly scale the up to whatever the screen coordinats are
  // e.g. a 600x800 display window (see next section below)
  myRasterViewPort->clippedXMin = ( myRasterExtent.xMin() - mGeoTransform[0] ) / mGeoTransform[1];
  myRasterViewPort->clippedXMax = ( myRasterExtent.xMax() - mGeoTransform[0] ) / mGeoTransform[1];
  myRasterViewPort->clippedYMin = ( myRasterExtent.yMin() - mGeoTransform[3] ) / mGeoTransform[5];
  myRasterViewPort->clippedYMax = ( myRasterExtent.yMax() - mGeoTransform[3] ) / mGeoTransform[5];

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
      > mRasterXDim )
  {
    myRasterViewPort->clippedWidth =
      mRasterXDim - myRasterViewPort->rectXOffset;
  }
  if (( myRasterViewPort->rectYOffset + myRasterViewPort->clippedHeight )
      > mRasterYDim )
  {
    myRasterViewPort->clippedHeight =
      mRasterYDim - myRasterViewPort->rectYOffset;
  }

  // get dimensions of clipped raster image in device coordinate space (this is the size of the viewport)
  myRasterViewPort->topLeftPoint = theQgsMapToPixel.transform( myRasterExtent.xMin(), myRasterExtent.yMax() );
  myRasterViewPort->bottomRightPoint = theQgsMapToPixel.transform( myRasterExtent.xMax(), myRasterExtent.yMin() );

  myRasterViewPort->drawableAreaXDim = static_cast<int>( fabs(( myRasterViewPort->clippedWidth / theQgsMapToPixel.mapUnitsPerPixel() * mGeoTransform[1] ) ) + 0.5 );
  myRasterViewPort->drawableAreaYDim = static_cast<int>( fabs(( myRasterViewPort->clippedHeight / theQgsMapToPixel.mapUnitsPerPixel() * mGeoTransform[5] ) ) + 0.5 );

  QgsDebugMsg( QString( "mapUnitsPerPixel = %1" ).arg( theQgsMapToPixel.mapUnitsPerPixel() ) );
  QgsDebugMsg( QString( "mRasterXDim = %1" ).arg( mRasterXDim ) );
  QgsDebugMsg( QString( "mRasterYDim = %1" ).arg( mRasterYDim ) );
  QgsDebugMsg( QString( "rectXOffset = %1" ).arg( myRasterViewPort->rectXOffset ) );
  QgsDebugMsg( QString( "rectXOffsetFloat = %1" ).arg( myRasterViewPort->rectXOffsetFloat ) );
  QgsDebugMsg( QString( "rectYOffset = %1" ).arg( myRasterViewPort->rectYOffset ) );
  QgsDebugMsg( QString( "rectYOffsetFloat = %1" ).arg( myRasterViewPort->rectYOffsetFloat ) );

  QgsDebugMsg( QString( "myRasterExtent.xMin() = %1" ).arg( myRasterExtent.xMin() ) );
  QgsDebugMsg( QString( "myRasterExtent.xMax() = %1" ).arg( myRasterExtent.xMax() ) );
  QgsDebugMsg( QString( "myRasterExtent.yMin() = %1" ).arg( myRasterExtent.yMin() ) );
  QgsDebugMsg( QString( "myRasterExtent.yMax() = %1" ).arg( myRasterExtent.yMax() ) );

  QgsDebugMsg( QString( "topLeftPoint.x() = %1" ).arg( myRasterViewPort->topLeftPoint.x() ) );
  QgsDebugMsg( QString( "bottomRightPoint.x() = %1" ).arg( myRasterViewPort->bottomRightPoint.x() ) );
  QgsDebugMsg( QString( "topLeftPoint.y() = %1" ).arg( myRasterViewPort->topLeftPoint.y() ) );
  QgsDebugMsg( QString( "bottomRightPoint.y() = %1" ).arg( myRasterViewPort->bottomRightPoint.y() ) );

  QgsDebugMsg( QString( "clippedXMin = %1" ).arg( myRasterViewPort->clippedXMin ) );
  QgsDebugMsg( QString( "clippedXMax = %1" ).arg( myRasterViewPort->clippedXMax ) );
  QgsDebugMsg( QString( "clippedYMin = %1" ).arg( myRasterViewPort->clippedYMin ) );
  QgsDebugMsg( QString( "clippedYMax = %1" ).arg( myRasterViewPort->clippedYMax ) );

  QgsDebugMsg( QString( "clippedWidth = %1" ).arg( myRasterViewPort->clippedWidth ) );
  QgsDebugMsg( QString( "clippedHeight = %1" ).arg( myRasterViewPort->clippedHeight ) );
  QgsDebugMsg( QString( "drawableAreaXDim = %1" ).arg( myRasterViewPort->drawableAreaXDim ) );
  QgsDebugMsg( QString( "drawableAreaYDim = %1" ).arg( myRasterViewPort->drawableAreaYDim ) );

  QgsDebugMsg( "ReadXml: gray band name : " + mGrayBandName );
  QgsDebugMsg( "ReadXml: red band name : " + mRedBandName );
  QgsDebugMsg( "ReadXml: green band name : " + mGreenBandName );
  QgsDebugMsg( "ReadXml: blue band name : " + mBlueBandName );

  // /\/\/\ - added to handle zoomed-in rasters


  // Provider mode: See if a provider key is specified, and if so use the provider instead

  QgsDebugMsg( "Checking for provider key." );

  if ( !mProviderKey.isEmpty() )
  {
    QgsDebugMsg( "Wanting a '" + mProviderKey + "' provider to draw this." );

    emit setStatus( QString( "Retrieving using " ) + mProviderKey );

    QImage* image =
      mDataProvider->draw(
        myRasterExtent,
        // Below should calculate to the actual pixel size of the
        // part of the layer that's visible.
        static_cast<int>( fabs(( myRasterViewPort->clippedXMax -  myRasterViewPort->clippedXMin )
                               / theQgsMapToPixel.mapUnitsPerPixel() * mGeoTransform[1] ) + 1 ),
        static_cast<int>( fabs(( myRasterViewPort->clippedYMax -  myRasterViewPort->clippedYMin )
                               / theQgsMapToPixel.mapUnitsPerPixel() * mGeoTransform[5] ) + 1 )
//                         myRasterViewPort->drawableAreaXDim,
//                         myRasterViewPort->drawableAreaYDim
      );

    if ( !image )
    {
      // An error occurred.
      mErrorCaption = mDataProvider->errorCaptionString();
      mError        = mDataProvider->errorString();

      delete myRasterViewPort;
      return FALSE;
    }

    QgsDebugMsg( "Done mDataProvider->draw." );
    QgsDebugMsg( "image stats: " );

    QgsDebugMsg( QString( "depth=%1" ).arg( image->depth() ) );
    QgsDebugMsg( QString( "bytes=%1" ).arg( image->numBytes() ) );
    QgsDebugMsg( QString( "width=%1" ).arg( image->width() ) );
    QgsDebugMsg( QString( "height=%1" ).arg( image->height() ) );

    QgsDebugMsg( "Want to theQPainter->drawImage with" );

    QgsDebugMsg( QString( "origin x: %1" ).arg( myRasterViewPort->topLeftPoint.x() ) );
    QgsDebugMsg( QString( "(int)origin x: %1" ).arg( static_cast<int>( myRasterViewPort->topLeftPoint.x() ) ) );
    QgsDebugMsg( QString( "origin y: %1" ).arg( myRasterViewPort->topLeftPoint.y() ) );
    QgsDebugMsg( QString( "(int)origin y: %1" ).arg( static_cast<int>( myRasterViewPort->topLeftPoint.y() ) ) );

    // Since GDAL's RasterIO can't handle floating point, we have to round to
    // the nearest pixel.  Add 0.5 to get rounding instead of truncation
    // out of static_cast<int>.

    theQPainter->drawImage( static_cast<int>(
                              myRasterViewPort->topLeftPoint.x()
                              + 0.5    // try simulating rounding instead of truncation, to avoid off-by-one errors
                              // TODO: Check for rigorous correctness
                            ),
                            static_cast<int>(
                              myRasterViewPort->topLeftPoint.y()
                              + 0.5    // try simulating rounding instead of truncation, to avoid off-by-one errors
                              // TODO: Check for rigorous correctness
                            ),
                            *image );

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

  return TRUE;

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

  switch ( drawingStyle )
  {
      // a "Gray" or "Undefined" layer drawn as a range of gray colors
    case SINGLE_BAND_GRAY:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        drawSingleBandGray( theQPainter, theRasterViewPort,
                            theQgsMapToPixel, getRasterBandNumber( mGrayBandName ) );
        break;
      }
      // a "Gray" or "Undefined" layer drawn using a pseudocolor algorithm
    case SINGLE_BAND_PSEUDO_COLOR:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        drawSingleBandPseudoColor( theQPainter, theRasterViewPort,
                                   theQgsMapToPixel, getRasterBandNumber( mGrayBandName ) );
        break;
      }
      // a single band with a color map
    case PALETTED_COLOR:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        QgsDebugMsg( "PALETTED_COLOR drawing type detected..." );

        drawPalettedSingleBandColor( theQPainter, theRasterViewPort,
                                     theQgsMapToPixel, getRasterBandNumber( mGrayBandName ) );

        break;
      }
      // a "Palette" layer drawn in gray scale (using only one of the color components)
    case PALETTED_SINGLE_BAND_GRAY:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {
        QgsDebugMsg( "PALETTED_SINGLE_BAND_GRAY drawing type detected..." );

        int myBandNo = 1;
        drawPalettedSingleBandGray( theQPainter, theRasterViewPort,
                                    theQgsMapToPixel, myBandNo );

        break;
      }
      // a "Palette" layer having only one of its color components rendered as psuedo color
    case PALETTED_SINGLE_BAND_PSEUDO_COLOR:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {

        int myBandNo = 1;
        drawPalettedSingleBandPseudoColor( theQPainter, theRasterViewPort,
                                           theQgsMapToPixel, myBandNo, mGrayBandName );
        break;
      }
      //a "Palette" image where the bands contains 24bit color info and 8 bits is pulled out per color
    case PALETTED_MULTI_BAND_COLOR:
      drawPalettedMultiBandColor( theQPainter, theRasterViewPort,
                                  theQgsMapToPixel, 1 );
      break;
      // a layer containing 2 or more bands, but using only one band to produce a grayscale image
    case MULTI_BAND_SINGLE_BAND_GRAY:
      QgsDebugMsg( "MULTI_BAND_SINGLE_BAND_GRAY drawing type detected..." );
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        QgsDebugMsg( "MULTI_BAND_SINGLE_BAND_GRAY Not Set detected..." + mGrayBandName );
        break;
      }
      else
      {

        //get the band number for the mapped gray band
        drawMultiBandSingleBandGray( theQPainter, theRasterViewPort,
                                     theQgsMapToPixel, getRasterBandNumber( mGrayBandName ) );
        break;
      }
      //a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
    case MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR:
      //check the band is set!
      if ( mGrayBandName == TRSTRING_NOT_SET )
      {
        break;
      }
      else
      {

        drawMultiBandSingleBandPseudoColor( theQPainter, theRasterViewPort,
                                            theQgsMapToPixel, getRasterBandNumber( mGrayBandName ) );
        break;
      }
      //a layer containing 2 or more bands, mapped to the three RGBcolors.
      //In the case of a multiband with only two bands,
      //one band will have to be mapped to more than one color
    case MULTI_BAND_COLOR:
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

}                               //end of draw method


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
  void *myGdalScanData = readData( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if ( myGdalScanData == NULL )
  {
    return;
  }

  QImage myQImage = QImage( theRasterViewPort->drawableAreaXDim, theRasterViewPort->drawableAreaYDim, QImage::Format_ARGB32 );
  myQImage.fill( qRgba( 255, 255, 255, 0 ) ); // fill transparent

  QgsRasterBandStats myGrayBandStats;

  if ( QgsContrastEnhancement::NO_STRETCH != getContrastEnhancementAlgorithm() && !mUserDefinedGrayMinMaxFlag && mStandardDeviations > 0 )
  {
    mGrayActualMinimumMaximum = true;
    myGrayBandStats = getRasterBandStats( theBandNo );
    setMaximumValue( theBandNo, myGrayBandStats.mean + ( mStandardDeviations * myGrayBandStats.stdDev ) );
    setMinimumValue( theBandNo, myGrayBandStats.mean - ( mStandardDeviations * myGrayBandStats.stdDev ) );
  }
  else if ( QgsContrastEnhancement::NO_STRETCH != getContrastEnhancementAlgorithm() && !mUserDefinedGrayMinMaxFlag )
  {
    //This case will be true the first time the image is loaded, so just approimate the min max to keep
    //from calling generate raster band stats
    double GDALrange[2];
    GDALComputeRasterMinMax( myGdalBand, 1, GDALrange ); //Approximate
    mGrayActualMinimumMaximum = false;
    setMaximumValue( theBandNo, GDALrange[1] );
    setMinimumValue( theBandNo, GDALrange[0] );

  }

  QgsDebugMsg( "Starting main render loop" );
  // print each point in myGdalScanData with equal parts R, G, B or make it show as gray
  double myGrayValue = 0.0;
  int myGrayVal = 0;
  int myAlphaValue = 0;
  QgsContrastEnhancement* myContrastEnhancement = getContrastEnhancement( theBandNo );
  for ( int myColumn = 0; myColumn < theRasterViewPort->drawableAreaYDim; ++myColumn )
  {
    for ( int myRow = 0; myRow < theRasterViewPort->drawableAreaXDim; ++myRow )
    {
      myGrayValue = readValue( myGdalScanData, myDataType,
                               myColumn * theRasterViewPort->drawableAreaXDim + myRow );

      // If mNoDataValue is 'nan', the comparison
      // against myGrayVal will always fail ( nan==nan always
      // returns false, by design), hence the slightly odd comparison
      // of myGrayVal against itself.
      if ( mValidNoDataValue && ( myGrayValue == mNoDataValue || myGrayValue != myGrayValue ) )
      {
        continue;
      }

      if ( !myContrastEnhancement->isValueInDisplayableRange( myGrayValue ) )
      {
        continue;
      }

      myAlphaValue = mRasterTransparency.getAlphaValue( myGrayValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        continue;
      }


      myGrayVal = myContrastEnhancement->stretch( myGrayValue );

      if ( mInvertPixelsFlag )
      {
        myGrayVal = 255 - myGrayVal;
      }

      myQImage.setPixel( myRow, myColumn, qRgba( myGrayVal, myGrayVal, myGrayVal, myAlphaValue ) );

    }
  }

  CPLFree( myGdalScanData );

  QgsDebugMsg( "Render done, preparing to copy to canvas" );
  //render any inline filters
  filterLayer( &myQImage );

  paintImageToCanvas( theQPainter, theRasterViewPort, theQgsMapToPixel, &myQImage );

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

  QgsRasterBandStats myRasterBandStats = getRasterBandStats( theBandNo );
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );
  void *myGdalScanData = readData( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if ( myGdalScanData == NULL )
  {
    return;
  }

  QImage myQImage = QImage( theRasterViewPort->drawableAreaXDim, theRasterViewPort->drawableAreaYDim, QImage::Format_ARGB32 );
  myQImage.fill( qRgba( 255, 255, 255, 0 ) ); // fill transparent

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
    myMinimumValue = myRasterBandStats.minVal;
    myMaximumValue = myRasterBandStats.maxVal;
  }

  mRasterShader->setMinimumValue( myMinimumValue );
  mRasterShader->setMaximumValue( myMaximumValue );


  int myRedValue = 255;
  int myGreenValue = 255;
  int myBlueValue = 255;

  double myPixelValue = 0.0;
  int myAlphaValue = 0;
  QgsDebugMsg( "Starting main render loop" );
  for ( int myColumn = 0; myColumn < theRasterViewPort->drawableAreaYDim; ++myColumn )
  {
    for ( int myRow = 0; myRow < theRasterViewPort->drawableAreaXDim; ++myRow )
    {
      myPixelValue = readValue( myGdalScanData, myDataType,
                                myColumn * theRasterViewPort->drawableAreaXDim + myRow );

      if ( mValidNoDataValue && ( myPixelValue == mNoDataValue || myPixelValue != myPixelValue ) )
      {
        continue;
      }

      myAlphaValue = mRasterTransparency.getAlphaValue( myPixelValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        continue;
      }

      if ( !mRasterShader->generateShadedValue( myPixelValue, &myRedValue, &myGreenValue, &myBlueValue ) )
      {
        continue;
      }

      if ( mInvertPixelsFlag )
      {
        //Invert flag, flip blue and read
        myQImage.setPixel( myRow, myColumn, qRgba( myBlueValue, myGreenValue, myRedValue, myAlphaValue ) );
      }
      else
      {
        //Normal
        myQImage.setPixel( myRow, myColumn, qRgba( myRedValue, myGreenValue, myBlueValue, myAlphaValue ) );
      }
    }                       //end of columnwise loop
  }                           //end of rowwise loop

  CPLFree( myGdalScanData );

  //render any inline filters
  filterLayer( &myQImage );

  paintImageToCanvas( theQPainter, theRasterViewPort, theQgsMapToPixel, &myQImage );
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
  void *myGdalScanData = readData( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if ( myGdalScanData == NULL )
  {
    return;
  }

  QImage myQImage = QImage( theRasterViewPort->drawableAreaXDim, theRasterViewPort->drawableAreaYDim, QImage::Format_ARGB32 );
  myQImage.fill( qRgba( 255, 255, 255, 0 ) ); // fill transparent

  double myPixelValue = 0.0;
  int myRedValue = 0;
  int myGreenValue = 0;
  int myBlueValue = 0;
  int myAlphaValue = 0;

  QgsDebugMsg( "Starting main render loop" );
  for ( int myColumn = 0; myColumn < theRasterViewPort->drawableAreaYDim; ++myColumn )
  {
    for ( int myRow = 0; myRow < theRasterViewPort->drawableAreaXDim; ++myRow )
    {
      myRedValue = 0;
      myGreenValue = 0;
      myBlueValue = 0;
      myPixelValue = readValue( myGdalScanData, ( GDALDataType )myDataType,
                                myColumn * theRasterViewPort->drawableAreaXDim + myRow );

      if ( mValidNoDataValue && ( myPixelValue == mNoDataValue || myPixelValue != myPixelValue ) )
      {
        continue;
      }

      myAlphaValue = mRasterTransparency.getAlphaValue( myPixelValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        continue;
      }

      if ( !mRasterShader->generateShadedValue( myPixelValue, &myRedValue, &myGreenValue, &myBlueValue ) )
      {
        continue;
      }

      if ( mInvertPixelsFlag )
      {
        //Invert flag, flip blue and read
        myQImage.setPixel( myRow, myColumn, qRgba( myBlueValue, myGreenValue, myRedValue, myAlphaValue ) );
      }
      else
      {
        //Normal
        myQImage.setPixel( myRow, myColumn, qRgba( myRedValue, myGreenValue, myBlueValue, myAlphaValue ) );
      }
    }
  }
  CPLFree( myGdalScanData );

  //render any inline filters
  filterLayer( &myQImage );

  paintImageToCanvas( theQPainter, theRasterViewPort, theQgsMapToPixel, &myQImage );
}


/**
 * This method is used to render a paletted raster layer as a gray image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 * @param theColorQString - QString containing either 'Red' 'Green' or 'Blue' indicating which part of the rgb triplet will be used to render gray.
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
  void *myGdalScanData = readData( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if ( myGdalScanData == NULL )
  {
    return;
  }

  QImage myQImage = QImage( theRasterViewPort->drawableAreaXDim, theRasterViewPort->drawableAreaYDim, QImage::Format_ARGB32 );
  myQImage.fill( qRgba( 255, 255, 255, 0 ) ); // fill transparent

  double myPixelValue = 0.0;
  int myRedValue = 0;
  int myGreenValue = 0;
  int myBlueValue = 0;
  int myAlphaValue = 0;

  QgsDebugMsg( "Starting main render loop" );
  for ( int myColumn = 0; myColumn < theRasterViewPort->drawableAreaYDim; ++myColumn )
  {
    for ( int myRow = 0; myRow < theRasterViewPort->drawableAreaXDim; ++myRow )
    {
      myRedValue = 0;
      myGreenValue = 0;
      myBlueValue = 0;
      myPixelValue = readValue( myGdalScanData, ( GDALDataType )myDataType,
                                myColumn * theRasterViewPort->drawableAreaXDim + myRow );

      if ( mValidNoDataValue && ( myPixelValue == mNoDataValue || myPixelValue != myPixelValue ) )
      {
        continue;
      }

      myAlphaValue = mRasterTransparency.getAlphaValue( myPixelValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        continue;
      }

      if ( !mRasterShader->generateShadedValue( myPixelValue, &myRedValue, &myGreenValue, &myBlueValue ) )
      {
        continue;
      }

      if ( mInvertPixelsFlag )
      {
        //Invert flag, flip blue and read
        double myGrayValue = ( 0.3 * ( double )myRedValue ) + ( 0.59 * ( double )myGreenValue ) + ( 0.11 * ( double )myBlueValue );
        myQImage.setPixel( myRow, myColumn, qRgba(( int )myGrayValue, ( int )myGrayValue, ( int )myGrayValue, myAlphaValue ) );
      }
      else
      {
        //Normal
        double myGrayValue = ( 0.3 * ( double )myBlueValue ) + ( 0.59 * ( double )myGreenValue ) + ( 0.11 * ( double )myRedValue );
        myQImage.setPixel( myRow, myColumn, qRgba(( int )myGrayValue, ( int )myGrayValue, ( int )myGrayValue, myAlphaValue ) );
      }
    }
  }
  CPLFree( myGdalScanData );

  //render any inline filters
  filterLayer( &myQImage );

  paintImageToCanvas( theQPainter, theRasterViewPort, theQgsMapToPixel, &myQImage );
}


/**
 * This method is used to render a paletted raster layer as a pseudocolor image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 * @param theColorQString - QString containing either 'Red' 'Green' or 'Blue' indicating which part of the rgb triplet will be used to render gray.
 */
void QgsRasterLayer::drawPalettedSingleBandPseudoColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo,
    QString const & theColorQString )
{
  QgsDebugMsg( "entered." );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNo )
  {
    return;
  }

  QgsRasterBandStats myRasterBandStats = getRasterBandStats( theBandNo );
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );
  void *myGdalScanData = readData( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if ( myGdalScanData == NULL )
  {
    return;
  }

  QImage myQImage = QImage( theRasterViewPort->drawableAreaXDim, theRasterViewPort->drawableAreaYDim, QImage::Format_ARGB32 );
  myQImage.fill( qRgba( 255, 255, 255, 0 ) ); // fill transparent

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
    myMinimumValue = myRasterBandStats.minVal;
    myMaximumValue = myRasterBandStats.maxVal;
  }

  mRasterShader->setMinimumValue( myMinimumValue );
  mRasterShader->setMaximumValue( myMaximumValue );

  double myPixelValue = 0.0;
  int myRedValue = 0;
  int myGreenValue = 0;
  int myBlueValue = 0;
  int myAlphaValue = 0;

  QgsDebugMsg( "Starting main render loop" );
  for ( int myColumn = 0; myColumn < theRasterViewPort->drawableAreaYDim; ++myColumn )
  {
    for ( int myRow = 0; myRow < theRasterViewPort->drawableAreaXDim; ++myRow )
    {
      myRedValue = 0;
      myGreenValue = 0;
      myBlueValue = 0;
      myPixelValue = readValue( myGdalScanData, ( GDALDataType )myDataType,
                                myColumn * theRasterViewPort->drawableAreaXDim + myRow );

      if ( mValidNoDataValue && ( myPixelValue == mNoDataValue || myPixelValue != myPixelValue ) )
      {
        continue;
      }

      myAlphaValue = mRasterTransparency.getAlphaValue( myPixelValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        continue;
      }

      if ( !mRasterShader->generateShadedValue( myPixelValue, &myRedValue, &myGreenValue, &myBlueValue ) )
      {
        continue;
      }

      if ( mInvertPixelsFlag )
      {
        //Invert flag, flip blue and read
        myQImage.setPixel( myRow, myColumn, qRgba( myBlueValue, myGreenValue, myRedValue, myAlphaValue ) );
      }
      else
      {
        //Normal
        myQImage.setPixel( myRow, myColumn, qRgba( myRedValue, myGreenValue, myBlueValue, myAlphaValue ) );
      }
    }
  }
  CPLFree( myGdalScanData );

  //render any inline filters
  filterLayer( &myQImage );

  paintImageToCanvas( theQPainter, theRasterViewPort, theQgsMapToPixel, &myQImage );
}

/**
 * This method is used to render a paletted raster layer as a colour image -- currently not supported
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 */
void QgsRasterLayer::drawPalettedMultiBandColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel, int theBandNo )
{
  QgsDebugMsg( "Not supported at this time" );
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


void QgsRasterLayer::drawMultiBandColor( QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    const QgsMapToPixel* theQgsMapToPixel )
{
  QgsDebugMsg( "entered." );
  int myRedBandNo = getRasterBandNumber( mRedBandName );
  //Invalid band number, segfault prevention
  if ( 0 >= myRedBandNo )
  {
    return;
  }

  int myGreenBandNo = getRasterBandNumber( mGreenBandName );
  //Invalid band number, segfault prevention
  if ( 0 >= myGreenBandNo )
  {
    return;
  }

  int myBlueBandNo = getRasterBandNumber( mBlueBandName );
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

  void *myGdalRedData = readData( myGdalRedBand, theRasterViewPort );
  void *myGdalGreenData = readData( myGdalGreenBand, theRasterViewPort );
  void *myGdalBlueData = readData( myGdalBlueBand, theRasterViewPort );

  /* Check for out of memory error */
  if ( myGdalRedData == NULL || myGdalGreenData == NULL || myGdalBlueData == NULL )
  {
    // Safe to free NULL-pointer */
    VSIFree( myGdalRedData );
    VSIFree( myGdalGreenData );
    VSIFree( myGdalBlueData );
    return;
  }

  QImage myQImage = QImage( theRasterViewPort->drawableAreaXDim, theRasterViewPort->drawableAreaYDim, QImage::Format_ARGB32 );
  myQImage.fill( qRgba( 255, 255, 255, 0 ) ); // fill transparent

  QgsRasterBandStats myRedBandStats;
  QgsRasterBandStats myGreenBandStats;
  QgsRasterBandStats myBlueBandStats;
  /*
   * If a stetch is requested and there are no user defined Min Max values
   * we need to get these values from the bands themselves.
   *
   */
  if ( QgsContrastEnhancement::NO_STRETCH != getContrastEnhancementAlgorithm() && !mUserDefinedRGBMinMaxFlag && mStandardDeviations > 0 )
  {
    myRedBandStats = getRasterBandStats( myRedBandNo );
    myGreenBandStats = getRasterBandStats( myGreenBandNo );
    myBlueBandStats = getRasterBandStats( myBlueBandNo );
    mRGBActualMinimumMaximum = true;
    setMaximumValue( myRedBandNo, myRedBandStats.mean + ( mStandardDeviations * myRedBandStats.stdDev ) );
    setMinimumValue( myRedBandNo, myRedBandStats.mean - ( mStandardDeviations * myRedBandStats.stdDev ) );
    setMaximumValue( myGreenBandNo, myGreenBandStats.mean + ( mStandardDeviations * myGreenBandStats.stdDev ) );
    setMinimumValue( myGreenBandNo, myGreenBandStats.mean - ( mStandardDeviations * myGreenBandStats.stdDev ) );
    setMaximumValue( myBlueBandNo, myBlueBandStats.mean + ( mStandardDeviations * myBlueBandStats.stdDev ) );
    setMinimumValue( myBlueBandNo, myBlueBandStats.mean - ( mStandardDeviations * myBlueBandStats.stdDev ) );
  }
  else if ( QgsContrastEnhancement::NO_STRETCH != getContrastEnhancementAlgorithm() && !mUserDefinedRGBMinMaxFlag )
  {
    //This case will be true the first time the image is loaded, so just approimate the min max to keep
    //from calling generate raster band stats
    double GDALrange[2];
    mRGBActualMinimumMaximum = false;

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
  QgsContrastEnhancement* myRedContrastEnhancement = getContrastEnhancement( myRedBandNo );
  QgsContrastEnhancement* myGreenContrastEnhancement = getContrastEnhancement( myGreenBandNo );
  QgsContrastEnhancement* myBlueContrastEnhancement = getContrastEnhancement( myBlueBandNo );

  QgsDebugMsg( "Starting main render loop" );
  for ( int myColumn = 0; myColumn < theRasterViewPort->drawableAreaYDim; ++myColumn )
  {
    for ( int myRow = 0; myRow < theRasterViewPort->drawableAreaXDim; ++myRow )
    {
      myRedValue   = readValue( myGdalRedData, ( GDALDataType )myRedType,
                                myColumn * theRasterViewPort->drawableAreaXDim + myRow );
      myGreenValue = readValue( myGdalGreenData, ( GDALDataType )myGreenType,
                                myColumn * theRasterViewPort->drawableAreaXDim + myRow );
      myBlueValue  = readValue( myGdalBlueData, ( GDALDataType )myBlueType,
                                myColumn * theRasterViewPort->drawableAreaXDim + myRow );

      if ( mValidNoDataValue && (( myRedValue == mNoDataValue || myRedValue != myRedValue ) || ( myGreenValue == mNoDataValue || myGreenValue != myGreenValue ) || ( myBlueValue == mNoDataValue || myBlueValue != myBlueValue ) ) )
      {
        continue;
      }

      if ( !myRedContrastEnhancement->isValueInDisplayableRange( myRedValue ) || !myGreenContrastEnhancement->isValueInDisplayableRange( myGreenValue ) || !myBlueContrastEnhancement->isValueInDisplayableRange( myBlueValue ) )
      {
        continue;
      }

      myAlphaValue = mRasterTransparency.getAlphaValue( myRedValue, myGreenValue, myBlueValue, mTransparencyLevel );
      if ( 0 == myAlphaValue )
      {
        continue;
      }

      myStretchedRedValue = myRedContrastEnhancement->stretch( myRedValue );
      myStretchedGreenValue = myGreenContrastEnhancement->stretch( myGreenValue );
      myStretchedBlueValue = myBlueContrastEnhancement->stretch( myBlueValue );

      if ( mInvertPixelsFlag )
      {
        myStretchedRedValue = 255 - myStretchedRedValue;
        myStretchedGreenValue = 255 - myStretchedGreenValue;
        myStretchedBlueValue = 255 - myStretchedBlueValue;
      }

      myQImage.setPixel( myRow, myColumn, qRgba( myStretchedRedValue, myStretchedGreenValue, myStretchedBlueValue, myAlphaValue ) );
    }
  }
  //free the scanline memory
  CPLFree( myGdalRedData );
  CPLFree( myGdalGreenData );
  CPLFree( myGdalBlueData );

  //render any inline filters
  filterLayer( &myQImage );

#ifdef QGISDEBUG
  QPixmap* pm = dynamic_cast<QPixmap*>( theQPainter->device() );
  if ( pm )
  {
    QgsDebugMsg( "theQPainter stats: " );
    QgsDebugMsg( "width = " + QString::number( pm->width() ) );
    QgsDebugMsg( "height = " + QString::number( pm->height() ) );
    pm->save( "/tmp/qgis-rasterlayer-drawmultibandcolor-test-a.png", "PNG" );
  }
#endif

  paintImageToCanvas( theQPainter, theRasterViewPort, theQgsMapToPixel, &myQImage );

#ifdef QGISDEBUG
  QgsDebugMsg( "theQPainter->drawImage." );
  if ( pm )
  {
    pm->save( "/tmp/qgis-rasterlayer-drawmultibandcolor-test-b.png", "PNG" );
  }
#endif
}



/**
 * Call any inline filters
 */
void QgsRasterLayer::filterLayer( QImage * theQImage )
{
  //do stuff here....
  //return;
}


const QgsRasterBandStats QgsRasterLayer::getRasterBandStats( QString const & theBandName )
{

  //we cant use a vector iterator because the iterator is astruct not a class
  //and the qvector model does not like this.
  for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); i++ )
  {
    QgsRasterBandStats myRasterBandStats = getRasterBandStats( i );
    if ( myRasterBandStats.bandName == theBandName )
    {
      return myRasterBandStats;
    }
  }

  return QgsRasterBandStats();     // return a null one
}

int QgsRasterLayer::getRasterBandNumber( QString const & theBandName )
{
  for ( int myIterator = 0; myIterator < mRasterStatsList.size(); ++myIterator )
  {
    //find out the name of this band
    QgsRasterBandStats myRasterBandStats = mRasterStatsList[myIterator];
    QgsDebugMsg( "myRasterBandStats.bandName: " + myRasterBandStats.bandName + "  :: theBandName: "
                 + theBandName );

    if ( myRasterBandStats.bandName == theBandName )
    {
      QgsDebugMsg( "********** band " + QString::number( myRasterBandStats.bandNo ) +
                   " was found in getRasterBandNumber " + theBandName );

      return myRasterBandStats.bandNo;
    }
  }
  QgsDebugMsg( "********** no band was found in getRasterBandNumber " + theBandName );

  return 0;                     //no band was found
}



// get the name of a band given its number
const QString QgsRasterLayer::getRasterBandName( int theBandNo )
{
  QgsDebugMsg( "entered." );
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



/** Check whether a given band number has stats associated with it */
bool QgsRasterLayer::hasStats( int theBandNo )
{
  if ( theBandNo <= mRasterStatsList.size() && theBandNo > 0 )
  {
    //vector starts at base 0, band counts at base1 !
    return mRasterStatsList[theBandNo - 1].statsGatheredFlag;
  }
  else
  {
    return false;
  }
}


/** Private method to calculate statistics for a band. Populates rasterStatsMemArray.

Calculates:

<ul>
<li>myRasterBandStats.elementCount
<li>myRasterBandStats.minVal
<li>myRasterBandStats.maxVal
<li>myRasterBandStats.sum
<li>myRasterBandStats.range
<li>myRasterBandStats.mean
<li>myRasterBandStats.sumSqrDev
<li>myRasterBandStats.stdDev
<li>myRasterBandStats.colorTable
</ul>

@seealso RasterBandStats

@note

That this is a cpu intensive and slow task!

*/
//TODO: This method needs some cleaning up PJE 2007-12-30
const QgsRasterBandStats QgsRasterLayer::getRasterBandStats( int theBandNo )
{
  // check if we have received a valid band number
  if (( GDALGetRasterCount( mGdalDataset ) < theBandNo ) && rasterLayerType != PALETTE )
  {
    // invalid band id, return nothing
    QgsRasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }
  if ( rasterLayerType == PALETTE && ( theBandNo > 3 ) )
  {
    // invalid band id, return nothing
    QgsRasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }
  // check if we have previously gathered stats for this band...

  QgsRasterBandStats myRasterBandStats = mRasterStatsList[theBandNo - 1];
  myRasterBandStats.bandNo = theBandNo;

  // don't bother with this if we already have stats
  if ( myRasterBandStats.statsGatheredFlag )
  {
    return myRasterBandStats;
  }
  // only print message if we are actually gathering the stats
  emit setStatus( QString( "Retrieving stats for " ) + name() );
  qApp->processEvents();
  QgsDebugMsg( "stats for band " + QString::number( theBandNo ) );
  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );


  QString myColorerpretation = GDALGetColorInterpretationName( GDALGetRasterColorInterpretation( myGdalBand ) );

  // XXX this sets the element count to a sensible value; but then you ADD to
  // XXX it later while iterating through all the pixels?
  //myRasterBandStats.elementCount = mRasterXDim * mRasterYDim;

  myRasterBandStats.elementCount = 0; // because we'll be counting only VALID pixels later

  emit setStatus( QString( "Calculating stats for " ) + name() );
  //reset the main app progress bar
  emit drawingProgress( 0, 0 );

  // let the user know we're going to possibly be taking a while
  //QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  GDALDataType myDataType = GDALGetRasterDataType( myGdalBand );

  int  myNXBlocks, myNYBlocks, myXBlockSize, myYBlockSize;
  GDALGetBlockSize( myGdalBand, &myXBlockSize, &myYBlockSize );

  myNXBlocks = ( GDALGetRasterXSize( myGdalBand ) + myXBlockSize - 1 ) / myXBlockSize;
  myNYBlocks = ( GDALGetRasterYSize( myGdalBand ) + myYBlockSize - 1 ) / myYBlockSize;

  void *myData = CPLMalloc( myXBlockSize * myYBlockSize * GDALGetDataTypeSize( myDataType ) / 8 );

  // unfortunately we need to make two passes through the data to calculate stddev
  bool myFirstIterationFlag = true;

  // Comparison value for equality; i.e., we shouldn't directly compare two
  // floats so it's better to take their difference and see if they're within
  // a certain range -- in this case twenty times the smallest value that
  // doubles can take for the current system.  (Yes, 20 was arbitrary.)
  double myPrecision = std::numeric_limits<double>::epsilon() * 20;
  Q_UNUSED( myPrecision );

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
          double my = readValue( myData, myDataType, iX + iY * myXBlockSize );

          //if ( mValidNoDataValue && (fabs(my - mNoDataValue) < myPrecision || my == mNoDataValue || my != my))
          if ( mValidNoDataValue && ( my == mNoDataValue || my != my ) )
          {
            continue; // NULL
          }

          //only use this element if we have a non null element
          if ( myFirstIterationFlag )
          {
            //this is the first iteration so initialise vars
            myFirstIterationFlag = false;
            myRasterBandStats.minVal = my;
            myRasterBandStats.maxVal = my;
            ++myRasterBandStats.elementCount;
          }               //end of true part for first iteration check
          else
          {
            //this is done for all subsequent iterations
            if ( my < myRasterBandStats.minVal )
            {
              myRasterBandStats.minVal = my;
            }
            if ( my > myRasterBandStats.maxVal )
            {
              myRasterBandStats.maxVal = my;
            }

            myRasterBandStats.sum += my;
            ++myRasterBandStats.elementCount;
          }               //end of false part for first iteration check
        }
      }
    }                       //end of column wise loop
  }                           //end of row wise loop


  //end of first pass through data now calculate the range
  myRasterBandStats.range = myRasterBandStats.maxVal - myRasterBandStats.minVal;
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
          double my = readValue( myData, myDataType, iX + iY * myXBlockSize );

          //if ( mValidNoDataValue && (fabs(my - mNoDataValue) < myPrecision || my == mNoDataValue || my != my))
          if ( mValidNoDataValue && ( my == mNoDataValue || my != my ) )
          {
            continue; // NULL
          }

          myRasterBandStats.sumSqrDev += static_cast < double >
                                         ( pow( my - myRasterBandStats.mean, 2 ) );
        }
      }
    }                       //end of column wise loop
  }                           //end of row wise loop

  //divide result by sample size - 1 and get square root to get stdev
  myRasterBandStats.stdDev = static_cast < double >( sqrt( myRasterBandStats.sumSqrDev /
                             ( myRasterBandStats.elementCount - 1 ) ) );

#ifdef QGISDEBUG
  QgsLogger::debug( "************ STATS **************", 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "VALID NODATA", mValidNoDataValue, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "NULL", mNoDataValue, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "MIN", myRasterBandStats.minVal, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "MAX", myRasterBandStats.maxVal, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "RANGE", myRasterBandStats.range, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "MEAN", myRasterBandStats.mean, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "STDDEV", myRasterBandStats.stdDev, 1, __FILE__, __FUNCTION__, __LINE__ );
#endif

  CPLFree( myData );
  myRasterBandStats.statsGatheredFlag = true;

  QgsDebugMsg( "adding stats to stats collection at position " + QString::number( theBandNo - 1 ) );
  //add this band to the class stats map
  mRasterStatsList[theBandNo - 1] = myRasterBandStats;
  emit drawingProgress( mRasterYDim, mRasterYDim ); //reset progress
  //QApplication::restoreOverrideCursor(); //restore the cursor
  QgsDebugMsg( "Stats collection completed returning" );
  return myRasterBandStats;

} // QgsRasterLayer::getRasterBandStats



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

  QgsDebugMsg( "Testing older naming format" );
  //See of the band in an older format #:something.
  //TODO Remove test in v2.0
  if ( theBandName.contains( ':' ) )
  {
    QStringList myBandNameComponents = theBandName.split( ":" );
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

//mutator for red band name (allows alternate mappings e.g. map blue as red colour)
void QgsRasterLayer::setRedBandName( QString const & theBandName )
{
  QgsDebugMsg( "setRedBandName :  " + theBandName );
  mRedBandName = validateBandName( theBandName );
}

//mutator for green band name
void QgsRasterLayer::setGreenBandName( QString const & theBandName )
{
  mGreenBandName = validateBandName( theBandName );
}

//mutator for blue band name
void QgsRasterLayer::setBlueBandName( QString const & theBandName )
{
  mBlueBandName = validateBandName( theBandName );
}

//mutator for transparent band name
void QgsRasterLayer::setTransparentBandName( QString const & theBandName )
{
  mTransparencyBandName = validateBandName( theBandName );
}

//mutator for gray band name
void QgsRasterLayer::setGrayBandName( QString const & theBandName )
{
  mGrayBandName = validateBandName( theBandName );
}

/** Return a pixmap representing a legend image. This is an overloaded
 * version of the method below and assumes false for the legend name flag.
 */
QPixmap QgsRasterLayer::getLegendQPixmap()
{
  return getLegendQPixmap( false );
}

/** Return a pixmap representing a legend image
 * @param theWithNameFlag - boolena flag whether to overlay the legend name in the text
 */
QPixmap QgsRasterLayer::getLegendQPixmap( bool theWithNameFlag )
{
  QgsDebugMsg( "called (" + getDrawingStyleAsQString() + ")" );

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
    if ( drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY )
    {

      myLegendQPixmap = QPixmap( 100, 1 );
      myQPainter.begin( &myLegendQPixmap );
      int myPos = 0;
      for ( double my = 0; my < 255; my += 2.55 )
      {
        if ( !mInvertPixelsFlag ) //histogram is not inverted
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
    else if ( drawingStyle == MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR ||
              drawingStyle == PALETTED_SINGLE_BAND_PSEUDO_COLOR || drawingStyle == SINGLE_BAND_PSEUDO_COLOR )
    {

      //set up the three class breaks for pseudocolour mapping
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
        if ( !mInvertPixelsFlag )
        {
          //check if we are in the first class break
          if (( my >= myClassBreakMin1 ) && ( my < myClassBreakMax1 ) )
          {
            int myRed = 0;
            int myBlue = 255;
            int myGreen = static_cast < int >((( 255 / myRangeSize ) * ( my - myClassBreakMin1 ) ) * 3 );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FREAK_OUT )
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
            if ( mColorShadingAlgorithm == FREAK_OUT )
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
            if ( mColorShadingAlgorithm == FREAK_OUT )
            {
              myRed = myGreen;
              myGreen = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
        }                   //end of invert histogram == false check
        else                  //invert histogram toggle is off
        {
          //check if we are in the first class break
          if (( my >= myClassBreakMin1 ) && ( my < myClassBreakMax1 ) )
          {
            int myRed = 255;
            int myBlue = 0;
            int myGreen = static_cast < int >((( 255 / myRangeSize ) * (( my - myClassBreakMin1 ) / 1 ) * 3 ) );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FREAK_OUT )
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
            if ( mColorShadingAlgorithm == FREAK_OUT )
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
            if ( mColorShadingAlgorithm == FREAK_OUT )
            {
              myRed = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }

        }                   //end of invert histogram check
        myQPainter.drawPoint( myPos++, 0 );
      }

    }                           //end of pseudocolor check
    else if ( drawingStyle == PALETTED_MULTI_BAND_COLOR || drawingStyle == MULTI_BAND_COLOR )
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
    if ( hasPyramidsFlag )
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
    if ( drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY )
    {
      myQPainter.setPen( Qt::white );
    }
    else
    {
      myQPainter.setPen( Qt::black );
    }
    myQPainter.setFont( myQFont );
    myQPainter.drawText( 25, myHeight - 10, this->name() );
    //
    // finish up
    //
    myLegendQPixmap = myQPixmap2;
    myQPainter.end();
  }
  //finish up

  return myLegendQPixmap;

}                               //end of getLegendQPixmap function


QPixmap QgsRasterLayer::getDetailedLegendQPixmap( int theLabelCount = 3 )
{
  QgsDebugMsg( "entered." );
  QFont myQFont( "arial", 10, QFont::Normal );
  QFontMetrics myQFontMetrics( myQFont );

  int myFontHeight = ( myQFontMetrics.height() );
  const int myerLabelSpacing = 5;
  int myImageHeight = (( myFontHeight + ( myerLabelSpacing * 2 ) ) * theLabelCount );
  //these next two vars are not used anywhere so commented out for now
  //int myLongestLabelWidth =  myQFontMetrics.width(this->name());
  //const int myHorizontalLabelSpacing = 5;
  const int myColourBarWidth = 10;
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
  if ( drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY )
  {

    myLegendQPixmap = QPixmap( 1, myImageHeight );
    const double myIncrement = static_cast<double>( myImageHeight ) / 255.0;
    myQPainter.begin( &myLegendQPixmap );
    int myPos = 0;
    for ( double my = 0; my < 255; my += myIncrement )
    {
      if ( !mInvertPixelsFlag ) //histogram is not inverted
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
  else if ( drawingStyle == MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR ||
            drawingStyle == PALETTED_SINGLE_BAND_PSEUDO_COLOR || drawingStyle == SINGLE_BAND_PSEUDO_COLOR )
  {

    //set up the three class breaks for pseudocolour mapping
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
        if ( !mInvertPixelsFlag )
        {
          //check if we are in the first class break
          if (( my >= myClassBreakMin1 ) && ( my < myClassBreakMax1 ) )
          {
            int myRed = 0;
            int myBlue = 255;
            int myGreen = static_cast < int >((( 255 / myRangeSize ) * ( my - myClassBreakMin1 ) ) * 3 );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FREAK_OUT )
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
            if ( mColorShadingAlgorithm == FREAK_OUT )
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
            if ( mColorShadingAlgorithm == FREAK_OUT )
            {
              myRed = myGreen;
              myGreen = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }
        }                   //end of invert histogram == false check
        else                  //invert histogram toggle is off
        {
          //check if we are in the first class break
          if (( my >= myClassBreakMin1 ) && ( my < myClassBreakMax1 ) )
          {
            int myRed = 255;
            int myBlue = 0;
            int myGreen = static_cast < int >((( 255 / myRangeSize ) * (( my - myClassBreakMin1 ) / 1 ) * 3 ) );
            // testing this stuff still ...
            if ( mColorShadingAlgorithm == FREAK_OUT )
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
            if ( mColorShadingAlgorithm == FREAK_OUT )
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
            if ( mColorShadingAlgorithm == FREAK_OUT )
            {
              myRed = 255 - myGreen;
            }
            myQPainter.setPen( QPen( QColor( myRed, myGreen, myBlue ), 0 ) );
          }

        }                   //end of invert histogram check
        myQPainter.drawPoint( 0, myPos++ );
      }

  }                           //end of pseudocolor check
  else if ( drawingStyle == PALETTED_MULTI_BAND_COLOR || drawingStyle == MULTI_BAND_COLOR )
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
    myQWMatrix.scale( myColourBarWidth, 2 );
  }
  else
  {
    myQWMatrix.scale( myColourBarWidth, 2 );
  }
  //apply the matrix
  QPixmap myQPixmap2 = myLegendQPixmap.transformed( myQWMatrix );
  QPainter myQPainter2( &myQPixmap2 );
  //
  // Overlay the layer name
  //
  if ( drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY )
  {
    myQPainter2.setPen( Qt::white );
  }
  else
  {
    myQPainter2.setPen( Qt::black );
  }
  myQPainter2.setFont( myQFont );
  myQPainter2.drawText( 25, myImageHeight - 10, this->name() );
  //
  // finish up
  //
  myLegendQPixmap = myQPixmap2;
  myQPainter2.end();
  //finish up

  return myLegendQPixmap;

}//end of getDetailedLegend

// Useful for Provider mode

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


// Useful for Provider mode

void QgsRasterLayer::setLayerOrder( QStringList const & layers )
{
  QgsDebugMsg( "entered." );

  if ( mDataProvider )
  {
    QgsDebugMsg( "About to mDataProvider->setLayerOrder(layers)." );
    mDataProvider->setLayerOrder( layers );
  }

}

// Useful for Provider mode

void QgsRasterLayer::setSubLayerVisibility( QString const & name, bool vis )
{

  if ( mDataProvider )
  {
    QgsDebugMsg( "About to mDataProvider->setSubLayerVisibility(name, vis)." );
    mDataProvider->setSubLayerVisibility( name, vis );
  }

}


void QgsRasterLayer::updateProgress( int theProgress, int theMax )
{
  //simply propogate it on!
  emit drawingProgress( theProgress, theMax );
}



// convenience function for building getMetadata() HTML table cells
static
QString
makeTableCell_( QString const & value )
{
  return "<p>\n" + value + "</p>\n";
} // makeTableCell_



// convenience function for building getMetadata() HTML table cells
static
QString
makeTableCells_( QStringList const & values )
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
static
QStringList
cStringList2Q_( char ** stringList )
{
  QStringList strings;

  // presume null terminated string list
  for ( size_t i = 0; stringList[i]; ++i )
  {
    strings.append( stringList[i] );
  }

  return strings;

} // cStringList2Q_




QString QgsRasterLayer::getMetadata()
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
    myMetadata += mDataProvider->getMetadata();
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
    myMetadata += tr( "X: " ) + QString::number( GDALGetRasterXSize( mGdalDataset ) ) +
                  tr( " Y: " ) + QString::number( GDALGetRasterYSize( mGdalDataset ) ) + tr( " Bands: " ) + QString::number( GDALGetRasterCount( mGdalDataset ) );
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
  myMetadata += mCRS->proj4String();
  myMetadata += "</p>\n";

  // output coordinate system
  // TODO: this is not related to layer, to be removed? [MD]
  /*
      myMetadata += "<tr><td class=\"glossy\">";
      myMetadata += tr("Project Spatial Reference System: ");
      myMetadata += "</p>\n";
      myMetadata += "<p>";
      myMetadata +=  mCoordinateTransform->destCRS().proj4String();
      myMetadata += "</p>\n";
      */

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
    int myBandCountInt = getBandCount();
    for ( int myIteratorInt = 1; myIteratorInt <= myBandCountInt; ++myIteratorInt )
    {
      QgsDebugMsg( "Raster properties : checking if band " + QString::number( myIteratorInt ) + " has stats? " );
      //band name
      myMetadata += "<p class=\"glossy\">\n";
      myMetadata += tr( "Band" );
      myMetadata += "</p>\n";
      myMetadata += "<p>";
      myMetadata += getRasterBandName( myIteratorInt );
      myMetadata += "</p>\n";
      //band number
      myMetadata += "<p>";
      myMetadata += tr( "Band No" );
      myMetadata += "</p>\n";
      myMetadata += "<p>\n";
      myMetadata += QString::number( myIteratorInt );
      myMetadata += "</p>\n";

      //check if full stats for this layer have already been collected
      if ( !hasStats( myIteratorInt ) )  //not collected
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

        QgsRasterBandStats myRasterBandStats = getRasterBandStats( myIteratorInt );
        //Min Val
        myMetadata += "<p>";
        myMetadata += tr( "Min Val" );
        myMetadata += "</p>\n";
        myMetadata += "<p>\n";
        myMetadata += QString::number( myRasterBandStats.minVal, 'f', 10 );
        myMetadata += "</p>\n";

        // Max Val
        myMetadata += "<p>";
        myMetadata += tr( "Max Val" );
        myMetadata += "</p>\n";
        myMetadata += "<p>\n";
        myMetadata += QString::number( myRasterBandStats.maxVal, 'f', 10 );
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
        myMetadata += QString::number( myRasterBandStats.sumSqrDev, 'f', 10 );
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
          hasPyramidsFlag = true;
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

QgsRasterLayer::RasterPyramidList  QgsRasterLayer::buildRasterPyramidList()
{
  //
  // First we build up a list of potential pyramid layers
  //
  int myWidth = mRasterXDim;
  int myHeight = mRasterYDim;
  int myDivisor = 2;
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


bool QgsRasterLayer::isEditable() const
{
  return false;
}

bool QgsRasterLayer::readColorTable( int theBandNumber, QList<QgsColorRampShader::ColorRampItem>* theList )
{
  QgsDebugMsg( "entered." );
  //Invalid band number, segfault prevention
  if ( 0 >= theBandNumber || 0 == theList )
  {
    QgsDebugMsg( "Invalid paramter" );
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

QList<QgsColorRampShader::ColorRampItem>* QgsRasterLayer::getColorTable( int theBandNo )
{
  return &( mRasterStatsList[theBandNo-1].colorTable );
}


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
    QgsDebugMsg( "Layer " + this->name() + " couldn't allocate enough memory. Ignoring" );
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
      QgsLogger::warning( "RaterIO error: " + QString( CPLGetLastErrorMsg() ) );
    }
  }
  return data;
}


double QgsRasterLayer::readValue( void *data, GDALDataType type, int index )
{
  double val;

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
      val = (( double * )data )[index];
      return ( double )(( double * )data )[index];
      break;
    default:
      QgsLogger::warning( "GDAL data type is not supported" );
  }
  return 0.0;
}


/**

  Raster layer project file XML of form:

  <maplayer type="raster" visible="1" showInOverviewFlag="1">
  <layername>Wynoochee_dem</layername>
  <datasource>/home/mcoletti/mnt/MCOLETTIF8F9/c/Toolkit_Course/Answers/Training_Data/wynoochee_dem.img</datasource>
  <zorder>0</zorder>
  <transparencyLevelInt>255</transparencyLevelInt>
  <rasterproperties>
  <drawingStyle>SINGLE_BAND_GRAY</drawingStyle>
  <mInvertPixelsFlag boolean="false"/>
  <mStandardDeviations>0</mStandardDeviations>
  <mRedBandName>Not Set</mRedBandName>
  <mGreenBandName>Not Set</mGreenBandName>
  <mBlueBandName>Not Set</mBlueBandName>
  <mGrayBandName>Undefined</mGrayBandName>
  </rasterproperties>
  </maplayer>
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
    QString crs = QString( "EPSG:%1" ).arg( srs().epsg() );

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

  QDomNode mnl = layer_node.namedItem( "rasterproperties" );
  QDomNode snode = mnl.namedItem( "drawingStyle" );
  QDomElement myElement = snode.toElement();
  setDrawingStyle( myElement.text() );

  snode = mnl.namedItem( "mColorShadingAlgorithm" );
  myElement = snode.toElement();
  setColorShadingAlgorithm( myElement.text() );

  snode = mnl.namedItem( "mInvertPixelsFlag" );
  myElement = snode.toElement();
  QVariant myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setInvertHistogramFlag( myVariant.toBool() );

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
  setStdDevsToPlot( myElement.text().toDouble() );

  snode = mnl.namedItem( "mUserDefinedRGBMinMaxFlag" );
  myElement = snode.toElement();
  myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setUserDefinedRGBMinMax( myVariant.toBool() );

  snode = mnl.namedItem( "mRGBActualMinimumMaximum" );
  myElement = snode.toElement();
  myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setActualRGBMinMaxFlag( myVariant.toBool() );

  snode = mnl.namedItem( "mUserDefinedGrayMinMaxFlag" );
  myElement = snode.toElement();
  myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setUserDefinedGrayMinMax( myVariant.toBool() );

  snode = mnl.namedItem( "mGrayActualMinimumMaximum" );
  myElement = snode.toElement();
  myVariant = ( QVariant ) myElement.attribute( "boolean" );
  setActualGrayMinMaxFlag( myVariant.toBool() );

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
  QgsDebugMsg( "Drawing style " + getDrawingStyleAsQString() );

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
    QgsColorRampShader* myColorRampShader = ( QgsColorRampShader* ) mRasterShader->getRasterShaderFunction();

    //TODO: Remove the customColorRampType check and following if() in v2.0, added for compatability with older ( bugged ) project files
    QDomNode customColorRampTypeNode = customColorRampNode.namedItem( "customColorRampType" );
    QDomNode colorRampTypeNode = customColorRampNode.namedItem( "colorRampType" );
    QString myRampType = "";
    if( "" == customColorRampTypeNode.toElement().text() )
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

} // QgsRasterLayer::readXml( QDomNode & layer_node )



/* virtual */
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

  // <rasterproperties>
  QDomElement rasterPropertiesElement = document.createElement( "rasterproperties" );
  mapLayerNode.appendChild( rasterPropertiesElement );

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

  // <drawingStyle>
  QDomElement drawStyleElement = document.createElement( "drawingStyle" );
  QDomText    drawStyleText    = document.createTextNode( getDrawingStyleAsQString() );

  drawStyleElement.appendChild( drawStyleText );

  rasterPropertiesElement.appendChild( drawStyleElement );

  // <colorShadingAlgorithm>
  QDomElement colorShadingAlgorithmElement = document.createElement( "mColorShadingAlgorithm" );
  QDomText    colorShadingAlgorithmText    = document.createTextNode( getColorShadingAlgorithmAsQString() );

  colorShadingAlgorithmElement.appendChild( colorShadingAlgorithmText );

  rasterPropertiesElement.appendChild( colorShadingAlgorithmElement );

  // <mInvertPixelsFlag>
  QDomElement mInvertPixelsFlagElement = document.createElement( "mInvertPixelsFlag" );

  if ( getInvertHistogramFlag() )
  {
    mInvertPixelsFlagElement.setAttribute( "boolean", "true" );
  }
  else
  {
    mInvertPixelsFlagElement.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( mInvertPixelsFlagElement );


  // <mRedBandName>
  QDomElement mRedBandNameElement = document.createElement( "mRedBandName" );
  QString writtenRedBandName =  getRedBandName();
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
  QString writtenGreenBandName =  getGreenBandName();
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
  QString writtenBlueBandName =  getBlueBandName();
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
  QString writtenGrayBandName =  getGrayBandName();
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
  QDomText    mStandardDeviationsText    = document.createTextNode( QString::number( getStdDevsToPlot() ) );

  mStandardDeviationsElement.appendChild( mStandardDeviationsText );

  rasterPropertiesElement.appendChild( mStandardDeviationsElement );

  // <mUserDefinedRGBMinMaxFlag>
  QDomElement userDefinedRGBMinMaxFlag = document.createElement( "mUserDefinedRGBMinMaxFlag" );

  if ( getUserDefinedRGBMinMax() )
  {
    userDefinedRGBMinMaxFlag.setAttribute( "boolean", "true" );
  }
  else
  {
    userDefinedRGBMinMaxFlag.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( userDefinedRGBMinMaxFlag );

  // <mRGBActualMinimumMaximum>
  QDomElement RGBActualMinimumMaximum = document.createElement( "mRGBActualMinimumMaximum" );

  if ( getActualRGBMinMaxFlag() )
  {
    RGBActualMinimumMaximum.setAttribute( "boolean", "true" );
  }
  else
  {
    RGBActualMinimumMaximum.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( RGBActualMinimumMaximum );

  // <mUserDefinedGrayMinMaxFlag>
  QDomElement userDefinedGrayMinMaxFlag = document.createElement( "mUserDefinedGrayMinMaxFlag" );

  if ( getUserDefinedGrayMinMax() )
  {
    userDefinedGrayMinMaxFlag.setAttribute( "boolean", "true" );
  }
  else
  {
    userDefinedGrayMinMaxFlag.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( userDefinedGrayMinMaxFlag );

  // <mGrayActualMinimumMaximum>
  QDomElement GrayActualMinimumMaximum = document.createElement( "mGrayActualMinimumMaximum" );

  if ( getActualGrayMinMaxFlag() )
  {
    GrayActualMinimumMaximum.setAttribute( "boolean", "true" );
  }
  else
  {
    GrayActualMinimumMaximum.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( GrayActualMinimumMaximum );

  // <contrastEnhancementAlgorithm>
  QDomElement contrastEnhancementAlgorithmElement = document.createElement( "mContrastEnhancementAlgorithm" );
  QDomText    contrastEnhancementAlgorithmText    = document.createTextNode( getContrastEnhancementAlgorithmAsQString() );

  contrastEnhancementAlgorithmElement.appendChild( contrastEnhancementAlgorithmText );

  rasterPropertiesElement.appendChild( contrastEnhancementAlgorithmElement );

  // <minMaxValues>
  QList<QgsContrastEnhancement>::iterator it;
  QDomElement contrastEnhancementMinMaxValuesElement = document.createElement( "contrastEnhancementMinMaxValues" );
  for ( it =  mContrastEnhancementList.begin(); it != mContrastEnhancementList.end(); ++it )
  {
    QDomElement minMaxEntry = document.createElement( "minMaxEntry" );
    QDomElement minEntry = document.createElement( "min" );
    QDomElement maxEntry = document.createElement( "max" );

    QDomText minEntryText = document.createTextNode( QString::number( it->getMinimumValue() ) );
    minEntry.appendChild( minEntryText );

    QDomText maxEntryText = document.createTextNode( QString::number( it->getMaximumValue() ) );
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


  if ( mRasterTransparency.getTransparentSingleValuePixelList().count() > 0 )
  {
    QDomElement singleValuePixelListElement = document.createElement( "singleValuePixelList" );


    QList<QgsRasterTransparency::TransparentSingleValuePixel> myPixelList = mRasterTransparency.getTransparentSingleValuePixelList();
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

  if ( mRasterTransparency.getTransparentThreeValuePixelList().count() > 0 )
  {
    QDomElement threeValuePixelListElement = document.createElement( "threeValuePixelList" );


    QList<QgsRasterTransparency::TransparentThreeValuePixel> myPixelList = mRasterTransparency.getTransparentThreeValuePixelList();
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
  if ( QgsRasterLayer::COLOR_RAMP ==  getColorShadingAlgorithm() )
  {
    QDomElement customColorRampElement = document.createElement( "customColorRamp" );

    QDomElement customColorRampType = document.createElement( "colorRampType" );
    QDomText customColorRampTypeText = document.createTextNode((( QgsColorRampShader* )mRasterShader->getRasterShaderFunction() )->getColorRampTypeAsQString() );
    customColorRampType.appendChild( customColorRampTypeText );
    customColorRampElement.appendChild( customColorRampType );

    QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = (( QgsColorRampShader* )mRasterShader->getRasterShaderFunction() )->getColorRampItemList();
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
} // bool QgsRasterLayer::writeXml



bool QgsRasterLayer::identify( const QgsPoint& point, QMap<QString, QString>& results )
{
  results.clear();
  if ( mProviderKey == "wms" )
  {
    return false;
  }
  
  double x = point.x();
  double y = point.y();

  QgsDebugMsg( QString::number( x ) + ", " + QString::number( y ) );

  if ( x < mLayerExtent.xMin() || x > mLayerExtent.xMax() || y < mLayerExtent.yMin() || y > mLayerExtent.yMax() )
  {
    // Outside the raster
    for ( int i = 1; i <= GDALGetRasterCount( mGdalDataset ); i++ )
    {
      results[tr( "Band" ) + QString::number( i )] = tr( "out of extent" );
    }
  }
  else
  {
    /* Calculate the row / column where the point falls */
    double xres = ( mLayerExtent.xMax() - mLayerExtent.xMin() ) / mRasterXDim;
    double yres = ( mLayerExtent.yMax() - mLayerExtent.yMin() ) / mRasterYDim;

    // Offset, not the cell index -> flor
    int col = ( int ) floor(( x - mLayerExtent.xMin() ) / xres );
    int row = ( int ) floor(( mLayerExtent.yMax() - y ) / yres );

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
        QgsLogger::warning( "RaterIO error: " + QString( CPLGetLastErrorMsg() ) );
      }

      double value = readValue( data, type, 0 );
#ifdef QGISDEBUG
      QgsLogger::debug( "value", value, 1, __FILE__, __FUNCTION__, __LINE__ );
#endif
      QString v;

      if ( mValidNoDataValue && ( mNoDataValue == value || value != value ) )
      {
        v = tr( "null (no data)" );
      }
      else
      {
        v.setNum( value );
      }
      results[tr( "Band" ) + QString::number( i )] = v;

      free( data );
    }
  }

  return true;
} // bool QgsRasterLayer::identify


QString QgsRasterLayer::identifyAsText( const QgsPoint& point )
{
  if ( mProviderKey != "wms" )
  {
    // Currently no meaning for anything other than OGC WMS layers
    return QString();
  }

  return ( mDataProvider->identifyAsText( point ) );
}

void QgsRasterLayer::populateHistogram( int theBandNo, int theBinCount, bool theIgnoreOutOfRangeFlag, bool theHistogramEstimatedFlag )
{

  GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBandNo );
  QgsRasterBandStats myRasterBandStats = getRasterBandStats( theBandNo );
  //calculate the histogram for this band
  //we assume that it only needs to be calculated if the lenght of the histogram
  //vector is not equal to the number of bins
  //i.e if the histogram has never previously been generated or the user has
  //selected a new number of bins.
  if ( myRasterBandStats.histogramVector->size() != theBinCount ||
       theIgnoreOutOfRangeFlag != myRasterBandStats.histogramOutOfRangeFlag ||
       theHistogramEstimatedFlag != myRasterBandStats.histogramEstimatedFlag )
  {
    myRasterBandStats.histogramVector->clear();
    myRasterBandStats.histogramEstimatedFlag = theHistogramEstimatedFlag;
    myRasterBandStats.histogramOutOfRangeFlag = theIgnoreOutOfRangeFlag;
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
    double myerval = ( myRasterBandStats.maxVal - myRasterBandStats.minVal ) / theBinCount;
    GDALGetRasterHistogram( myGdalBand, myRasterBandStats.minVal - 0.1*myerval,
                            myRasterBandStats.maxVal + 0.1*myerval, theBinCount, myHistogramArray,
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


/*
 *
 * New functions that will convert this class to a data provider interface
 * (B Morley)
 *
 */



QgsRasterLayer::QgsRasterLayer( int dummy,
                                QString const & rasterLayerPath,
                                QString const & baseName,
                                QString const & providerKey,
                                QStringList const & layers,
                                QStringList const & styles,
                                QString const & format,
                                QString const & crs )
    : QgsMapLayer( RASTER, baseName, rasterLayerPath ),
    mRasterXDim( std::numeric_limits<int>::max() ),
    mRasterYDim( std::numeric_limits<int>::max() ),
    mInvertPixelsFlag( false ),
    mStandardDeviations( 0 ),
    mProviderKey( providerKey ),
    mDataProvider( 0 ),
    mEditable( false ),
    mModified( false )

{
  QgsDebugMsg( "(8 arguments) starting. with layer list of " +
               layers.join( ", " ) +  " and style list of " + styles.join( ", " ) + " and format of " +
               format +  " and CRS of " + crs );

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
  connect(
    mDataProvider, SIGNAL( setStatus( QString ) ),
    this,           SLOT( showStatusMessage( QString ) )
  );
  QgsDebugMsg( "(8 arguments) exiting." );

  emit setStatus( "QgsRasterLayer created" );
} // QgsRasterLayer ctor


// typedef for the QgsDataProvider class factory
typedef QgsDataProvider * classFactoryFunction_t( const QString * );


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
          QgsRect mbr = mDataProvider->extent();

          // show the extent
          QString s = mbr.toString();
          QgsDebugMsg( "Extent of layer: " + s );
          // store the extent
          mLayerExtent.setXMaximum( mbr.xMax() );
          mLayerExtent.setXMinimum( mbr.xMin() );
          mLayerExtent.setYMaximum( mbr.yMax() );
          mLayerExtent.setYMinimum( mbr.yMin() );

          // upper case the first letter of the layer name
          QgsDebugMsg( "mLayerName: " + name() );

          // set up the raster drawing style
          drawingStyle = MULTI_BAND_COLOR;  //sensible default

          // Setup source CRS
          *mCRS = QgsCoordinateReferenceSystem();
          mCRS->createFromOgcWmsCrs( crs );
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


bool QgsRasterLayer::usesProvider()
{
  if ( mProviderKey.isEmpty() )
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
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


void QgsRasterLayer::showStatusMessage( QString const & theMessage )
{
  // QgsDebugMsg(QString("entered with '%1'.").arg(theMessage));

  // Pass-through
  // TODO: See if we can connect signal-to-signal.  This is a kludge according to the Qt doc.
  emit setStatus( theMessage );
}


QString QgsRasterLayer::errorCaptionString()
{
  return mErrorCaption;
}


QString QgsRasterLayer::errorString()
{
  return mError;
}


QgsRasterDataProvider* QgsRasterLayer::dataProvider()
{
  return mDataProvider;
}

const QgsRasterDataProvider* QgsRasterLayer::dataProvider() const
{
  return mDataProvider;
}

unsigned int QgsRasterLayer::getBandCount()
{
  return mRasterStatsList.size();
}

double QgsRasterLayer::rasterUnitsPerPixel()
{
// We return one raster pixel per map unit pixel
// One raster pixel can have several raster units...

// We can only use one of the mGeoTransform[], so go with the
// horisontal one.

  return fabs( mGeoTransform[1] );
}

void QgsRasterLayer::setColorShadingAlgorithm( COLOR_SHADING_ALGORITHM theShadingAlgorithm )
{
  QgsDebugMsg( "called with [" + QString::number( theShadingAlgorithm ) + "]" );
  if ( mColorShadingAlgorithm != theShadingAlgorithm )
  {
    if ( 0 == mRasterShader )
    {
      mRasterShader = new QgsRasterShader();
    }

    mColorShadingAlgorithm = theShadingAlgorithm;

    switch ( theShadingAlgorithm )
    {
      case PSEUDO_COLOR:
        mRasterShader->setRasterShaderFunction( new QgsPseudoColorShader() );
        break;
      case FREAK_OUT:
        mRasterShader->setRasterShaderFunction( new QgsFreakOutShader() );
        break;
      case COLOR_RAMP:
        mRasterShader->setRasterShaderFunction( new QgsColorRampShader() );
        break;
      case USER_DEFINED:
        //do nothing
        break;
      default:
        mRasterShader->setRasterShaderFunction( new QgsRasterShaderFunction() );
        break;
    }
  }
  QgsDebugMsg( "mColorShadingAlgorithm = " + QString::number( theShadingAlgorithm ) );
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
      ( *myIterator ).statsGatheredFlag = false;
      ++myIterator;
    }
  }
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

  theQPainter->drawImage( static_cast<int>( theRasterViewPort->topLeftPoint.x() + 0.5 ),
                          static_cast<int>( theRasterViewPort->topLeftPoint.y() + 0.5 ),
                          *theImage,
                          paintXoffset,
                          paintYoffset );
}

QString QgsRasterLayer::getColorShadingAlgorithmAsQString()
{
  switch ( mColorShadingAlgorithm )
  {
    case PSEUDO_COLOR:
      return QString( "PSEUDO_COLOR" );
      break;
    case FREAK_OUT:
      return QString( "FREAK_OUT" );
      break;
    case COLOR_RAMP:
      return QString( "COLOR_RAMP" );
      break;
    case USER_DEFINED:
      return QString( "USER_DEFINED" );
      break;
    default:
      break;
  }

  return QString( "UNDEFINED_SHADING_ALGORITHM" );
}

void QgsRasterLayer::setColorShadingAlgorithm( QString theShaderAlgorithm )
{
  QgsDebugMsg( "called with [" + theShaderAlgorithm + "]" );

  if ( theShaderAlgorithm == "PSEUDO_COLOR" )
    setColorShadingAlgorithm( PSEUDO_COLOR );
  else if ( theShaderAlgorithm == "FREAK_OUT" )
    setColorShadingAlgorithm( FREAK_OUT );
  else if ( theShaderAlgorithm == "COLOR_RAMP" )
    setColorShadingAlgorithm( COLOR_RAMP );
  else if ( theShaderAlgorithm == "USER_DEFINED" )
    setColorShadingAlgorithm( USER_DEFINED );
  else
    setColorShadingAlgorithm( UNDEFINED_SHADING_ALGORITHM );
}

QString QgsRasterLayer::getContrastEnhancementAlgorithmAsQString()
{
  switch ( mContrastEnhancementAlgorithm )
  {
    case QgsContrastEnhancement::NO_STRETCH:
      return QString( "NO_STRETCH" );
      break;
    case QgsContrastEnhancement::STRETCH_TO_MINMAX:
      return QString( "STRETCH_TO_MINMAX" );
      break;
    case QgsContrastEnhancement::STRETCH_AND_CLIP_TO_MINMAX:
      return QString( "STRETCH_AND_CLIP_TO_MINMAX" );
      break;
    case QgsContrastEnhancement::CLIP_TO_MINMAX:
      return QString( "CLIP_TO_MINMAX" );
      break;
    case QgsContrastEnhancement::USER_DEFINED:
      return QString( "USER_DEFINED" );
      break;
  }

  return QString( "NO_STRETCH" );
}

void QgsRasterLayer::setContrastEnhancementAlgorithm( QString theAlgorithm, bool theGenerateLookupTableFlag )
{
  QgsDebugMsg( "called with [" + theAlgorithm + "] and flag=" + QString::number(( int )theGenerateLookupTableFlag ) );

  if ( theAlgorithm == "NO_STRETCH" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::NO_STRETCH, theGenerateLookupTableFlag );
  }
  else if ( theAlgorithm == "STRETCH_TO_MINMAX" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::STRETCH_TO_MINMAX, theGenerateLookupTableFlag );
  }
  else if ( theAlgorithm == "STRETCH_AND_CLIP_TO_MINMAX" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::STRETCH_AND_CLIP_TO_MINMAX, theGenerateLookupTableFlag );
  }
  else if ( theAlgorithm == "CLIP_TO_MINMAX" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::CLIP_TO_MINMAX, theGenerateLookupTableFlag );
  }
  else if ( theAlgorithm == "USER_DEFINED" )
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::USER_DEFINED, theGenerateLookupTableFlag );
  }
  else
  {
    setContrastEnhancementAlgorithm( QgsContrastEnhancement::NO_STRETCH, theGenerateLookupTableFlag );
  }
}

void QgsRasterLayer::showProgress( int theValue )
{
  emit progressUpdate( theValue );
}
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

  return TRUE;
}
