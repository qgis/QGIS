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
#include "qgsspatialrefsys.h"


#include <cstdio>
#include <cmath>
#include <limits>
#include <iostream>

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
#include <QMatrix>
#include <QMessageBox>
#include <QLibrary>
#include <QPainter>
#include <QPixmap>
#include <QRegExp>
#include <QSlider>



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
  Static member variable storing the subset of GDAL formats
  that we currently support.

  @note

  Some day this won't be necessary as there'll be a time when
  theoretically we'll support everything that GDAL can throw at us.

  These are GDAL driver description strings.
  */
static const char *const mSupportedRasterFormats[] =
{
  "AAIGrid",
  "AIG",
  "DTED",
  "ECW",
  "GRASS",
  "GTiff",
  "HFA",
  "JP2ECW",
  "JP2KAK",
  "JP2MrSID",
  "JPEG2000",
  "MrSID",
  "SDTS",
  "USGSDEM",
  ""   // used to indicate end of list
};



/**
  Builds the list of file filter strings to later be used by
  QgisApp::addRasterLayer()

  We query GDAL for a list of supported raster formats; we then build
  a list of file filter strings from that list.  We return a string
  that contains this list that is suitable for use in a a
  QFileDialog::getOpenFileNames() call.

*/
void QgsRasterLayer::buildSupportedRasterFileFilter(QString & theFileFiltersString)
{
  // first get the GDAL driver manager
  GDALAllRegister();
  GDALDriverManager *myGdalDriverManager = GetGDALDriverManager();

  if (!myGdalDriverManager)
  {
    QgsLogger::warning("unable to get GDALDriverManager");
    return;                   // XXX good place to throw exception if we
  }                           // XXX decide to do exceptions

  // then iterate through all of the supported drivers, adding the
  // corresponding file filter

  GDALDriver *myGdalDriver;           // current driver

  char **myGdalDriverMetadata;        // driver metadata strings

  QString myGdalDriverLongName("");   // long name for the given driver
  QString myGdalDriverExtension("");  // file name extension for given driver
  QString myGdalDriverDescription;    // QString wrapper of GDAL driver description

  QStringList metadataTokens;   // essentially the metadata string delimited by '='

  QStringList catchallFilter;   // for Any file(*.*), but also for those
  // drivers with no specific file filter

  GDALDriver *jp2Driver = NULL; // first JPEG2000 driver found

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

  for (int i = 0; i < myGdalDriverManager->GetDriverCount(); ++i)
  {
    myGdalDriver = myGdalDriverManager->GetDriver(i);

    Q_CHECK_PTR(myGdalDriver);

    if (!myGdalDriver)
    {
      QgsLogger::warning("unable to get driver " + QString::number(i));
      continue;
    }
    // now we need to see if the driver is for something currently
    // supported; if not, we give it a miss for the next driver

    myGdalDriverDescription = myGdalDriver->GetDescription();

    if (!isSupportedRasterDriver(myGdalDriverDescription))
    {
      // not supported, therefore skip
      QgsDebugMsg("skipping unsupported driver " + QString(myGdalDriver->GetDescription()));
      continue;
    }
    // std::cerr << "got driver string " << myGdalDriver->GetDescription() << "\n";

    myGdalDriverMetadata = myGdalDriver->GetMetadata();

    // presumably we know we've run out of metadta if either the
    // address is 0, or the first character is null
    while (myGdalDriverMetadata && '\0' != myGdalDriverMetadata[0])
    {
      metadataTokens = QStringList::split("=", *myGdalDriverMetadata);
      // std::cerr << "\t" << *myGdalDriverMetadata << "\n";

      // XXX add check for malformed metadataTokens

      // Note that it's oddly possible for there to be a
      // DMD_EXTENSION with no corresponding defined extension
      // string; so we check that there're more than two tokens.

      if (metadataTokens.count() > 1)
      {
        if ("DMD_EXTENSION" == metadataTokens[0])
        {
          myGdalDriverExtension = metadataTokens[1];

        }
        else if ("DMD_LONGNAME" == metadataTokens[0])
        {
          myGdalDriverLongName = metadataTokens[1];

          // remove any superfluous (.*) strings at the end as
          // they'll confuse QFileDialog::getOpenFileNames()

          myGdalDriverLongName.remove(QRegExp("\\(.*\\)$"));
        }
      }
      // if we have both the file name extension and the long name,
      // then we've all the information we need for the current
      // driver; therefore emit a file filter string and move to
      // the next driver
      if (!(myGdalDriverExtension.isEmpty() || myGdalDriverLongName.isEmpty()))
      {
        // XXX add check for SDTS; in that case we want (*CATD.DDF)
        QString glob = "*." + myGdalDriverExtension;
        // Add only the first JP2 driver found to the filter list (it's the one GDAL uses)
        if (myGdalDriverDescription == "JPEG2000" ||
            myGdalDriverDescription.startsWith("JP2"))	// JP2ECW, JP2KAK, JP2MrSID
        {
          if (!jp2Driver)
          {
            jp2Driver = myGdalDriver;   // first JP2 driver found
            glob += " *.j2k";           // add alternate extension
          }
          else break;               // skip if already found a JP2 driver
        }
        theFileFiltersString += myGdalDriverLongName + " (" + glob.lower() + " " + glob.upper() + ");;";

        break;            // ... to next driver, if any.
      }

      ++myGdalDriverMetadata;

    }                       // each metadata item

    if (myGdalDriverExtension.isEmpty() && !myGdalDriverLongName.isEmpty())
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
      if (myGdalDriverDescription.startsWith("USGSDEM"))
      {
        QString glob = "*.dem";
        theFileFiltersString += myGdalDriverLongName + " (" + glob.lower() + " " + glob.upper() + ");;";
      }
      else if (myGdalDriverDescription.startsWith("DTED"))
      {
        // DTED use "*.dt0"
        QString glob = "*.dt0";
        theFileFiltersString += myGdalDriverLongName + " (" + glob.lower() + " " + glob.upper() + ");;";
      }
      else if (myGdalDriverDescription.startsWith("MrSID"))
      {
        // MrSID use "*.sid"
        QString glob = "*.sid";
        theFileFiltersString += myGdalDriverLongName + " (" + glob.lower() + " " + glob.upper() + ");;";
      }
      else
      {
        catchallFilter << QString(myGdalDriver->GetDescription());
      }
    }

    myGdalDriverExtension = myGdalDriverLongName = "";  // reset for next driver

  }                           // each loaded GDAL driver

  // can't forget the default case
  theFileFiltersString += catchallFilter.join(", ") + " " + tr("and all other files") + " (*)";
  QgsDebugMsg("Raster filter list built: " + theFileFiltersString);
}                               // buildSupportedRasterFileFilter_()



/**
  returns true if the given raster driver name is one currently
  supported, otherwise it returns false

  @param theDriverName GDAL driver description string
  */
bool QgsRasterLayer::isSupportedRasterDriver(QString const &theDriverName)
{
  size_t i = 0;

  while (mSupportedRasterFormats[i][0]) // while not end of string list
  {
    // If we've got a case-insensitive match for a GDAL aware driver
    // description, then we've got a match.  Why case-insensitive?
    // I'm just being paranoid in that I can envision a situation
    // whereby GDAL slightly changes driver description string case,
    // in which case we'd catch it here.  Not that that would likely
    // happen, but if it does, we'll already compensate.
    // GS - At Qt 3.1.2, the case sensitive argument. So we change the
    // driverName to lower case before testing
    QString format = mSupportedRasterFormats[i];
    if (theDriverName.lower().startsWith(format.lower()))
    {
      return true;
    }

    i++;
  }

  return false;
}   // isSupportedRasterDriver



/** This helper checks to see whether the filename appears to be a valid raster file name */
bool QgsRasterLayer::isValidRasterFileName(QString const & theFileNameQString)
{

  GDALDatasetH myDataset;
  GDALAllRegister();

  //open the file using gdal making sure we have handled locale properly
  myDataset = GDALOpen( QFile::encodeName(theFileNameQString).constData(), GA_ReadOnly );
  if( myDataset == NULL )
  {
    return false;
  }
  else
  {
    GDALClose(myDataset);
    return true;
  }

  /*
   * This way is no longer a good idea because it does not
   * cater for filetypes such as grass rasters that dont
   * have a predictable file extension.
   *
   QString name = theFileNameQString.lower();
   return (name.endsWith(".adf") ||
   name.endsWith(".asc") ||
   name.endsWith(".grd") ||
   name.endsWith(".img") ||
   name.endsWith(".tif") ||
   name.endsWith(".png") ||
   name.endsWith(".jpg") ||
   name.endsWith(".dem") ||
   name.endsWith(".ddf")) ||
   name.endsWith(".dt0");

*/
}





//////////////////////////////////////////////////////////
//
// Non Static methods now....
//
/////////////////////////////////////////////////////////
QgsRasterLayer::QgsRasterLayer(QString const & path, QString const & baseName)
  : QgsMapLayer(RASTER, baseName, path),
  // XXX where is this? popMenu(0), //popMenu is the contextmenu obtained by right clicking on the legend
  rasterXDimInt( std::numeric_limits<int>::max() ),
  rasterYDimInt( std::numeric_limits<int>::max() ),
  showDebugOverlayFlag(false),
  invertHistogramFlag(false),
  stdDevsToPlotDouble(0),
  dataProvider(0)
{
  userDefinedColorMinMax = false; //defaults needed to bypass stretch
  userDefinedGrayMinMax = false;
  setColorScalingAlgorithm(QgsRasterLayer::NO_STRETCH); //defaults needed to bypass stretch

  // Initialise the affine transform matrix
  adfGeoTransform[0] =  0;
  adfGeoTransform[1] =  1;
  adfGeoTransform[2] =  0;
  adfGeoTransform[3] =  0;
  adfGeoTransform[4] =  0;
  adfGeoTransform[5] = -1;

  // set the layer name (uppercase first character)

  if ( ! baseName.isEmpty() )   // XXX shouldn't this happen in parent?
  {
    setLayerName(baseName);
  }

  // load the file if one specified
  if ( ! path.isEmpty() )
  {
    readFile( path ); // XXX check for failure?
  }

  //  Transparency slider for popup meni
  //  QSlider ( int minValue, int maxValue, int pageStep, int value, Orientation orientation, QWidget * parent, const char * name = 0 )


  //   // emit a signal asking for a repaint
  //   emit repaintRequested();

} // QgsRasterLayer ctor


QgsRasterLayer::~QgsRasterLayer()
{

  if (mProviderKey.isEmpty())
  {
    GDALClose(gdalDataset);
  }  
}



  bool
QgsRasterLayer::readFile( QString const & fileName )
{
  GDALAllRegister();

  //open the dataset making sure we handle char encoding of locale properly
  gdalDataset = (GDALDataset *) GDALOpen(QFile::encodeName(fileName).constData(), GA_ReadOnly);

  if (gdalDataset == NULL)
  {
    mValid = FALSE;
    return false;
  }

  // Store timestamp
  mLastModified = lastModified ( fileName );

  //check f this file has pyramids
  GDALRasterBandH myGDALBand = GDALGetRasterBand( gdalDataset, 1 ); //just use the first band
  if( GDALGetOverviewCount(myGDALBand) > 0 )
  {
    hasPyramidsFlag=true;
  }
  else
  {
    hasPyramidsFlag=false;
  }

  //populate the list of what pyramids exist
  buildRasterPyramidList();

  // Get the layer's projection info and set up the
  // QgsCoordinateTransform for this layer
  // NOTE: we must do this before getMetadata is called

  QgsDebugMsg("Raster initial SRS");
  mSRS->debugPrint();

  QString mySourceWKT = getProjectionWKT();

  QgsDebugMsg("--------------------------------------------------------------------------------------");
  QgsDebugMsg("QgsRasterLayer::readFile --- using wkt\n" + mySourceWKT);
  QgsDebugMsg("--------------------------------------------------------------------------------------");

  mSRS->createFromWkt(mySourceWKT);
  //get the project projection, defaulting to this layer's projection
  //if none exists....
  if (!mSRS->isValid())
  {
    mSRS->validate();
  }
  QgsDebugMsg("Raster determined to have the following SRS");
  mSRS->debugPrint();

  //set up the coordinat transform - in the case of raster this is mainly used to convert
  //the inverese projection of the map extents of the canvas when zzooming in etc. so
  //that they match the coordinate system of this layer
  QgsDebugMsg("Layer registry has " + QString::number(QgsMapLayerRegistry::instance()->count()) + "layers");

  getMetadata();

  // Use the affine transform to get geo coordinates for
  // the corners of the raster
  double myXMaxDouble = adfGeoTransform[0] +
    gdalDataset->GetRasterXSize() * adfGeoTransform[1] +
    gdalDataset->GetRasterYSize() * adfGeoTransform[2];
  double myYMinDouble = adfGeoTransform[3] +
    gdalDataset->GetRasterXSize() * adfGeoTransform[4] +
    gdalDataset->GetRasterYSize() * adfGeoTransform[5];

  mLayerExtent.setXmax(myXMaxDouble);
  // The affine transform reduces to these values at the
  // top-left corner of the raster
  mLayerExtent.setXmin(adfGeoTransform[0]);
  mLayerExtent.setYmax(adfGeoTransform[3]);
  mLayerExtent.setYmin(myYMinDouble);

  //
  // Set up the x and y dimensions of this raster layer
  //
  rasterXDimInt = gdalDataset->GetRasterXSize();
  rasterYDimInt = gdalDataset->GetRasterYSize();

  //
  // Determin the no data value
  //
  noDataValueDouble = gdalDataset->GetRasterBand(1)->GetNoDataValue();
  //initialise the raster band stats vector
  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++)
  {
    GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(i);
    QString myColorQString = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    QgsRasterBandStats myRasterBandStats;
    //myRasterBandStats.bandName = myColorQString ;
    myRasterBandStats.bandName=QString::number(i) + " : " + myColorQString;
    myRasterBandStats.bandNoInt = i;
    myRasterBandStats.statsGatheredFlag = false;
    myRasterBandStats.histogramVector = new QgsRasterBandStats::HistogramVector();
    // Read color table
    readColorTable ( myGdalBand, &(myRasterBandStats.colorTable) );

    rasterStatsVector.push_back(myRasterBandStats);
  }


  //decide what type of layer this is...
  //note that multiband images can have one or more 'undefindd' bands,
  //so we must do this check first!
  if ((gdalDataset->GetRasterCount() > 1))
  {
    rasterLayerType = MULTIBAND;
  }
  else if (hasBand("Palette")) //dont tr() this its a gdal word!
  {
    rasterLayerType = PALETTE;
  }
  else
  {
    rasterLayerType = GRAY_OR_UNDEFINED;
  }

  if (rasterLayerType == PALETTE)
  {
    redBandNameQString = "Red"; // sensible default
    greenBandNameQString = "Green"; // sensible default
    blueBandNameQString = "Blue";// sensible default
    transparentBandNameQString = tr("Not Set"); // sensible default
    grayBandNameQString = tr("Not Set");  //sensible default
    drawingStyle = PALETTED_MULTI_BAND_COLOR; //sensible default
  }
  else if (rasterLayerType == MULTIBAND)
  {
    //we know we have at least 2 layers...
    redBandNameQString = getRasterBandName(1);  // sensible default
    greenBandNameQString = getRasterBandName(2);  // sensible default
    //for the third layer we cant be sure so..
    if (gdalDataset->GetRasterCount() > 2)
    {
      blueBandNameQString = getRasterBandName(3); // sensible default
    }
    else
    {
      blueBandNameQString = tr("Not Set");  // sensible default
    }
    if (gdalDataset->GetRasterCount() > 3)
      transparentBandNameQString = getRasterBandName(4);
    else
      transparentBandNameQString = tr("Not Set");

    grayBandNameQString = tr("Not Set");  //sensible default
    drawingStyle = MULTI_BAND_COLOR;  //sensible default
  }
  else                        //GRAY_OR_UNDEFINED
  {
    getRasterBandStats(1);
    redBandNameQString = tr("Not Set"); //sensible default
    greenBandNameQString = tr("Not Set"); //sensible default
    blueBandNameQString = tr("Not Set");  //sensible default
    transparentBandNameQString = tr("Not Set");  //sensible default
    drawingStyle = SINGLE_BAND_GRAY;  //sensible default
    grayBandNameQString = getRasterBandName(1); // usually gdal will return gray or undefined  
  }




  //mark the layer as valid
  mValid=TRUE;
  return true;

} // QgsRasterLayer::readFile

QString QgsRasterLayer::getProjectionWKT() 
{ 
  QString myWKTString;
  QgsSpatialRefSys mySRS;   
  myWKTString=QString (gdalDataset->GetProjectionRef());
  mySRS.createFromWkt(myWKTString);
  if (!mySRS.isValid())
  {
    //try to get the gcp srs from the raster layer if available
    myWKTString=QString(gdalDataset->GetGCPProjection());

// What is the purpose of this piece of code?
// Sideeffects from validate()?
//    mySRS.createFromWkt(myWKTString);
//    if (!mySRS.isValid())
//    {
//      // use force and make SRS valid!
//      mySRS.validate();
//    }

  }

  return myWKTString;
}

void QgsRasterLayer::closeDataset()
{
  if ( !mValid  ) return;
  mValid = FALSE;

  GDALClose(gdalDataset);
  gdalDataset = 0;

  hasPyramidsFlag=false;
  mPyramidList.clear();

  rasterStatsVector.clear();
}

bool QgsRasterLayer::update()
{
  QgsDebugMsg("QgsRasterLayer::update");

  if ( mLastModified < QgsRasterLayer::lastModified ( source() ) )
  {
    QgsDebugMsg("Outdated -> reload");
    closeDataset();
    return readFile ( source() );
  }
  return true;
}

QDateTime QgsRasterLayer::lastModified ( QString const & name )
{
  QgsDebugMsg("QgsRasterLayer::lastModified: " + name);
  QDateTime t;

  QFileInfo fi ( name );

  // Is it file?
  if ( !fi.exists() ) return t;

  t = fi.lastModified();

  // Check also color table for GRASS
  if ( name.contains( "cellhd" ) > 0 )
  { // most probably GRASS
    QString dir = fi.dirPath();
    QString map = fi.fileName();
    fi.setFile ( dir + "/../colr/" + map );

    if ( fi.exists() )
    {
      if ( fi.lastModified() > t ) t = fi.lastModified();
    }
  }

  // Check GRASS group members (bands)
  if ( name.contains( "group" ) > 0 )
  { // probably GRASS group
    fi.setFile ( name + "/REF" );

    if ( fi.exists() )
    {  // most probably GRASS group
      QFile f ( name + "/REF" );
      if ( f.open ( QIODevice::ReadOnly ) )
      {
        QString dir = fi.dirPath() + "/../../../";

        char buf[101];
        while ( f.readLine(buf,100) != -1 )
        {
          QString ln = QString(buf);
          QStringList sl = QStringList::split ( ' ', ln.stripWhiteSpace() );
          QString map = sl.first();
          sl.pop_front();
          QString mapset = sl.first();

          // header
          fi.setFile ( dir + mapset + "/cellhd/" +  map );
          if ( fi.exists() )
          {
            if ( fi.lastModified() > t ) t = fi.lastModified();
          }

          // color
          fi.setFile ( dir + mapset + "/colr/" +  map );
          if ( fi.exists() )
          {
            if ( fi.lastModified() > t ) t = fi.lastModified();
          }
        }
      }
    }
  }

  QgsDebugMsg("last modified = " + t.toString());

  return t;
}


// emit a signal asking for a repaint
void QgsRasterLayer::triggerRepaint()
{
  emit repaintRequested();
}


QString QgsRasterLayer::getDrawingStyleAsQString()
{
  switch (drawingStyle)
  {
    case SINGLE_BAND_GRAY:
      return QString("SINGLE_BAND_GRAY"); //no need to tr() this its not shown in ui
      break;
    case SINGLE_BAND_PSEUDO_COLOR:
      return QString("SINGLE_BAND_PSEUDO_COLOR");//no need to tr() this its not shown in ui
      break;
    case PALETTED_SINGLE_BAND_GRAY:
      return QString("PALETTED_SINGLE_BAND_GRAY");//no need to tr() this its not shown in ui
      break;
    case PALETTED_SINGLE_BAND_PSEUDO_COLOR:
      return QString("PALETTED_SINGLE_BAND_PSEUDO_COLOR");//no need to tr() this its not shown in ui
      break;
    case PALETTED_MULTI_BAND_COLOR:
      return QString("PALETTED_MULTI_BAND_COLOR");//no need to tr() this its not shown in ui
      break;
    case MULTI_BAND_SINGLE_BAND_GRAY:
      return QString("MULTI_BAND_SINGLE_BAND_GRAY");//no need to tr() this its not shown in ui
      break;
    case MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR:
      return QString("MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR");//no need to tr() this its not shown in ui
      break;
    case MULTI_BAND_COLOR:
      return QString("MULTI_BAND_COLOR");//no need to tr() this its not shown in ui
      break;
    default:
      break;
  }

  return QString("INVALID_DRAWING_STYLE"); // XXX I hope this is ok to return

}

void QgsRasterLayer::setDrawingStyle(QString const & theDrawingStyleQString)
{
  if (theDrawingStyleQString == "SINGLE_BAND_GRAY")//no need to tr() this its not shown in ui
  {
    drawingStyle = SINGLE_BAND_GRAY;
    return;
  }
  if (theDrawingStyleQString == "SINGLE_BAND_PSEUDO_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = SINGLE_BAND_PSEUDO_COLOR;
    return;
  }
  if (theDrawingStyleQString == "PALETTED_SINGLE_BAND_GRAY")//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_SINGLE_BAND_GRAY;
    return;
  }
  if (theDrawingStyleQString == "PALETTED_SINGLE_BAND_PSEUDO_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_SINGLE_BAND_PSEUDO_COLOR;
    return;
  }
  if (theDrawingStyleQString == "PALETTED_MULTI_BAND_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_MULTI_BAND_COLOR;
    return;
  }
  if (theDrawingStyleQString == "MULTI_BAND_SINGLE_BAND_GRAY")//no need to tr() this its not shown in ui
  {
    drawingStyle = MULTI_BAND_SINGLE_BAND_GRAY;
    return;
  }
  if (theDrawingStyleQString == "MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR;
    return;
  }
  if (theDrawingStyleQString == "MULTI_BAND_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = MULTI_BAND_COLOR;
    return;
  }
}


/** This method looks to see if a given band name exists.

  @note

  muliband layers may have more than one "Undefined" band!
  */
bool QgsRasterLayer::hasBand(QString const & theBandName)
{
  QgsDebugMsg("Looking for band : " + theBandName);

  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++)
  {
    GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(i);
    QString myColorQString = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
#ifdef QGISDEBUG
    QgsLogger::debug("band", i, __FILE__, __FUNCTION__, __LINE__, 2);
#endif

    if (myColorQString == theBandName)
    {
#ifdef QGISDEBUG
      QgsLogger::debug("band", i, __FILE__, __FUNCTION__, __LINE__, 2);
      QgsDebugMsgLevel("Found band : " + theBandName, 2);
#endif

      return true;
    }
    QgsDebugMsgLevel("Found unmatched band : " + QString::number(i) + " " + myColorQString, 2);
  }
  return false;
}

void QgsRasterLayer::drawThumbnail(QPixmap * theQPixmap)
{
  theQPixmap->fill(); //defaults to white

  // Raster providers are disabled (for the moment)
  if (mProviderKey.isEmpty())
  {
    QgsRasterViewPort *myRasterViewPort = new QgsRasterViewPort();
    myRasterViewPort->rectXOffsetInt = 0;
    myRasterViewPort->rectYOffsetInt = 0;
    myRasterViewPort->clippedXMinDouble = 0;
    myRasterViewPort->clippedXMaxDouble = rasterXDimInt;
    myRasterViewPort->clippedYMinDouble = rasterYDimInt;
    myRasterViewPort->clippedYMaxDouble = 0;
    myRasterViewPort->clippedWidthInt   = rasterXDimInt;
    myRasterViewPort->clippedHeightInt  = rasterYDimInt;
    myRasterViewPort->topLeftPoint = QgsPoint(0,0);
    myRasterViewPort->bottomRightPoint = QgsPoint(theQPixmap->width(), theQPixmap->height());
    myRasterViewPort->drawableAreaXDimInt = theQPixmap->width();
    myRasterViewPort->drawableAreaYDimInt = theQPixmap->height();

    QPainter * myQPainter=new QPainter(theQPixmap);
    draw(myQPainter,myRasterViewPort);
    delete myRasterViewPort;
    myQPainter->end();
    delete myQPainter;
  }

}



QPixmap QgsRasterLayer::getPaletteAsPixmap()
{
  QgsDebugMsg("QgsRasterLayer::getPaletteAsPixmap");

  // Only do this for the non-provider (hard-coded GDAL) scenario...
  // Maybe WMS can do this differently using QImage::numColors and QImage::color()
  if (
      (mProviderKey.isEmpty()) &&
      (hasBand("Palette") ) //dont tr() this its a gdal word!
     )
  {
    QgsDebugMsg("....found paletted image");
    QgsColorTable *myColorTable = colorTable ( 1 );
    GDALRasterBandH myGdalBand = gdalDataset->GetRasterBand(1);
    if( GDALGetRasterColorInterpretation(myGdalBand) == GCI_PaletteIndex && myColorTable->defined() )
    {
      QgsDebugMsg("....found GCI_PaletteIndex");
      double myMinDouble = myColorTable->rmin();
      double myMaxDouble = myColorTable->rmax();
      QgsDebugMsg("myMinDouble = " + QString::number(myMinDouble) + " myMaxDouble = " + QString::number(myMaxDouble));

      // Draw image
      int mySizeInt = 100;
      QPixmap myPalettePixmap( mySizeInt, mySizeInt);
      QPainter myQPainter(&myPalettePixmap);

      QImage myQImage = QImage( mySizeInt, mySizeInt, 32);
      myQImage.fill(0);
      myQImage.setAlphaBuffer(false);
      myPalettePixmap.fill();

      double myStepDouble = ( myMaxDouble - myMinDouble ) / ( mySizeInt * mySizeInt);

      for( int myRowInt = 0; myRowInt < mySizeInt; myRowInt++ )
      {
        for( int myColInt = 0; myColInt < mySizeInt; myColInt++ )
        {

          double myValueDouble = myMinDouble + myStepDouble * (myColInt + myRowInt * mySizeInt);

          int c1, c2, c3;
          bool found = myColorTable->color ( myValueDouble, &c1, &c2, &c3 );

          if ( found )
            myQImage.setPixel( myColInt, myRowInt, qRgb(c1, c2, c3));
        }
      }

      myQPainter.drawImage(0,0,myQImage);
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



bool QgsRasterLayer::draw(QPainter * theQPainter,
    QgsRect & theViewExtent,
    QgsMapToPixel * theQgsMapToPixel,
    QgsCoordinateTransform*,
    bool drawingToEditingCanvas)
{
  QgsDebugMsg("QgsRasterLayer::draw(4 arguments): entered.");

  //Dont waste time drawing if transparency is at 0 (completely transparent)
  if (mTransparencyLevel == 0)
    return TRUE;
  QgsDebugMsg("QgsRasterLayer::draw(4 arguments): checking timestamp.");

  // Check timestamp
  if ( !update() )
  {
    return FALSE;
  }    

  // clip raster extent to view extent
  QgsRect myRasterExtent = theViewExtent.intersect(&mLayerExtent);
  if (myRasterExtent.isEmpty())
  {
    // nothing to do
    return TRUE;
  }

#ifdef QGISDEBUG
  QgsLogger::debug<QgsRect>("QgsRasterLayer::draw(4 arguments): theViewExtent is ", theViewExtent, __FILE__, __FUNCTION__, __LINE__, 1);
  QgsLogger::debug<QgsRect>("QgsRasterLayer::draw(4 arguments): myRasterExtent is ", myRasterExtent, __FILE__, __FUNCTION__, __LINE__, 1);
#endif




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
  myRasterViewPort->rectXOffsetFloat = (theViewExtent.xMin() - mLayerExtent.xMin()) / fabs(adfGeoTransform[1]);
  myRasterViewPort->rectYOffsetFloat = (mLayerExtent.yMax() - theViewExtent.yMax()) / fabs(adfGeoTransform[5]);

  if (myRasterViewPort->rectXOffsetFloat < 0 )
  {
    myRasterViewPort->rectXOffsetFloat = 0;
  }

  if (myRasterViewPort->rectYOffsetFloat < 0 )
  {
    myRasterViewPort->rectYOffsetFloat = 0;
  }

  myRasterViewPort->rectXOffsetInt = static_cast < int >(myRasterViewPort->rectXOffsetFloat);
  myRasterViewPort->rectYOffsetInt = static_cast < int >(myRasterViewPort->rectYOffsetFloat);

#ifdef QGISDEBUG
  QgsLogger::debug("QgsRasterLayer::draw(4 arguments): adfGeoTransform[0] = ", adfGeoTransform[0], 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw(4 arguments): adfGeoTransform[1] = ", adfGeoTransform[1], 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw(4 arguments): adfGeoTransform[2] = ", adfGeoTransform[2], 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw(4 arguments): adfGeoTransform[3] = ", adfGeoTransform[3], 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw(4 arguments): adfGeoTransform[4] = ", adfGeoTransform[4], 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw(4 arguments): adfGeoTransform[5] = ", adfGeoTransform[5], 1, __FILE__,\
      __FUNCTION__, __LINE__);
#endif

  // get dimensions of clipped raster image in raster pixel space/ RasterIO will do the scaling for us.
  // So for example, if the user is zoomed in a long way, there may only be e.g. 5x5 pixels retrieved from
  // the raw raster data, but rasterio will seamlessly scale the up to whatever the screen coordinats are
  // e.g. a 600x800 display window (see next section below)
  myRasterViewPort->clippedXMinDouble = (myRasterExtent.xMin() - adfGeoTransform[0]) / adfGeoTransform[1];
  myRasterViewPort->clippedXMaxDouble = (myRasterExtent.xMax() - adfGeoTransform[0]) / adfGeoTransform[1];
  myRasterViewPort->clippedYMinDouble = (myRasterExtent.yMin() - adfGeoTransform[3]) / adfGeoTransform[5];
  myRasterViewPort->clippedYMaxDouble = (myRasterExtent.yMax() - adfGeoTransform[3]) / adfGeoTransform[5];

  // Sometimes the Ymin/Ymax are reversed.
  if (myRasterViewPort->clippedYMinDouble > myRasterViewPort->clippedYMaxDouble)
  {
    double t = myRasterViewPort->clippedYMinDouble;
    myRasterViewPort->clippedYMinDouble = myRasterViewPort->clippedYMaxDouble;
    myRasterViewPort->clippedYMaxDouble = t;
  }

  // Set the clipped width and height to encompass all of the source pixels
  // that could end up being displayed.
  myRasterViewPort->clippedWidthInt = 
    static_cast<int>(ceil(myRasterViewPort->clippedXMaxDouble) - floor(myRasterViewPort->clippedXMinDouble));

  myRasterViewPort->clippedHeightInt = 
    static_cast<int>(ceil(myRasterViewPort->clippedYMaxDouble) - floor(myRasterViewPort->clippedYMinDouble));

  // but make sure the intended SE corner extent doesn't exceed the SE corner
  // of the source raster, otherwise GDAL's RasterIO gives an error and returns nothing.
  // The SE corner = NW origin + dimensions of the image itself.
  if ( (myRasterViewPort->rectXOffsetInt + myRasterViewPort->clippedWidthInt)
      > rasterXDimInt)
  {
    myRasterViewPort->clippedWidthInt =
      rasterXDimInt - myRasterViewPort->rectXOffsetInt;
  }
  if ( (myRasterViewPort->rectYOffsetInt + myRasterViewPort->clippedHeightInt)
      > rasterYDimInt)
  {
    myRasterViewPort->clippedHeightInt =
      rasterYDimInt - myRasterViewPort->rectYOffsetInt;
  }

  // get dimensions of clipped raster image in device coordinate space (this is the size of the viewport)
  myRasterViewPort->topLeftPoint = theQgsMapToPixel->transform(myRasterExtent.xMin(), myRasterExtent.yMax());
  myRasterViewPort->bottomRightPoint = theQgsMapToPixel->transform(myRasterExtent.xMax(), myRasterExtent.yMin());

  myRasterViewPort->drawableAreaXDimInt = static_cast<int>
    (fabs( (myRasterViewPort->clippedWidthInt  / theQgsMapToPixel->mapUnitsPerPixel() * adfGeoTransform[1])) + 0.5);
  myRasterViewPort->drawableAreaYDimInt = static_cast<int>
    (fabs( (myRasterViewPort->clippedHeightInt / theQgsMapToPixel->mapUnitsPerPixel() * adfGeoTransform[5])) + 0.5);

#ifdef QGISDEBUG
  QgsLogger::debug("QgsRasterLayer::draw: mapUnitsPerPixel", theQgsMapToPixel->mapUnitsPerPixel(), 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: rasterXDimInt", rasterXDimInt, 1, __FILE__, __FUNCTION__, __LINE__); 
  QgsLogger::debug("QgsRasterLayer::draw: rasterYDimInt", rasterYDimInt, 1, __FILE__, __FUNCTION__, __LINE__);

  QgsLogger::debug("QgsRasterLayer::draw: rectXOffsetFloat", myRasterViewPort->rectXOffsetFloat, 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: rectXOffsetInt", myRasterViewPort->rectXOffsetInt, 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: rectYOffsetFloat", myRasterViewPort->rectYOffsetFloat, 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: rectYOffsetInt", myRasterViewPort->rectYOffsetInt, 1, __FILE__,\
      __FUNCTION__, __LINE__);

  QgsLogger::debug("QgsRasterLayer::draw: myRasterExtent.xMin()", myRasterExtent.xMin(), 1, __FILE__, __FUNCTION__,\
      __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: myRasterExtent.xMax()", myRasterExtent.xMax(), 1, __FILE__, __FUNCTION__,\
      __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: myRasterExtent.yMin()", myRasterExtent.yMin(), 1, __FILE__, __FUNCTION__,\
      __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: myRasterExtent.yMax()", myRasterExtent.yMax(), 1, __FILE__, __FUNCTION__,\
      __LINE__);

  QgsLogger::debug("QgsRasterLayer::draw: topLeftPoint.x()", myRasterViewPort->topLeftPoint.x(), 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: bottomRightPoint.x()", myRasterViewPort->bottomRightPoint.x(), 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: topLeftPoint.y()", myRasterViewPort->topLeftPoint.y(), 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: bottomRightPoint.y()", myRasterViewPort->bottomRightPoint.y(), 1, __FILE__,\
      __FUNCTION__, __LINE__);

  QgsLogger::debug("QgsRasterLayer::draw: clippedXMinDouble", myRasterViewPort->clippedXMinDouble, 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: clippedXMaxDouble", myRasterViewPort->clippedXMaxDouble, 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: clippedYMinDouble", myRasterViewPort->clippedYMinDouble, 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: clippedYMaxDouble", myRasterViewPort->clippedYMaxDouble, 1, __FILE__,\
      __FUNCTION__, __LINE__); 

  QgsLogger::debug("QgsRasterLayer::draw: clippedWidthInt", myRasterViewPort->clippedWidthInt, 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: clippedHeightInt", myRasterViewPort->clippedHeightInt, 1, __FILE__,\
      __FUNCTION__, __LINE__);  
  QgsLogger::debug("QgsRasterLayer::draw: drawableAreaXDimInt", myRasterViewPort->drawableAreaXDimInt, 1, __FILE__,\
      __FUNCTION__, __LINE__);
  QgsLogger::debug("QgsRasterLayer::draw: drawableAreaYDimInt", myRasterViewPort->drawableAreaYDimInt, 1, __FILE__,\
      __FUNCTION__, __LINE__);

  QgsDebugMsg("ReadXml: gray band name : " + grayBandNameQString);
  QgsDebugMsg("ReadXml: red band name : " + redBandNameQString);
  QgsDebugMsg("ReadXml: green band name : " + greenBandNameQString);
  QgsDebugMsg("ReadXml: blue band name : " + blueBandNameQString);
#endif

  // /\/\/\ - added to handle zoomed-in rasters


  // Provider mode: See if a provider key is specified, and if so use the provider instead
  
  QgsDebugMsg("QgsRasterLayer::draw: Checking for provider key.");
  
  if (!mProviderKey.isEmpty())
  {
    QgsDebugMsg("QgsRasterLayer::draw: Wanting a '" + mProviderKey + "' provider to draw this.");

    emit setStatus(QString("Retrieving using ")+mProviderKey);

    QImage* image = 
      dataProvider->draw(
                         myRasterExtent,
                         // Below should calculate to the actual pixel size of the
                         // part of the layer that's visible.
                         static_cast<int>( fabs( (myRasterViewPort->clippedXMaxDouble -  myRasterViewPort->clippedXMinDouble)
                                                 / theQgsMapToPixel->mapUnitsPerPixel() * adfGeoTransform[1]) + 1),
                         static_cast<int>( fabs( (myRasterViewPort->clippedYMaxDouble -  myRasterViewPort->clippedYMinDouble)
                                                 / theQgsMapToPixel->mapUnitsPerPixel() * adfGeoTransform[5]) + 1)
//                         myRasterViewPort->drawableAreaXDimInt,
//                         myRasterViewPort->drawableAreaYDimInt
                        );

    if (!image)
    {
      // An error occurred.
      mErrorCaption = dataProvider->errorCaptionString();
      mError        = dataProvider->errorString();

      delete myRasterViewPort;
      return FALSE;
    }

    QgsDebugMsg("QgsRasterLayer::draw: Done dataProvider->draw.");
    QgsDebugMsg("QgsRasterLayer::draw: image stats: ");
#ifdef QGISDEBUG
    QgsLogger::debug("depth", image->depth(), __FILE__, __FUNCTION__, __LINE__, 1);
    QgsLogger::debug("bytes", image->numBytes(), __FILE__, __FUNCTION__, __LINE__, 1);
    QgsLogger::debug("width", image->width(), __FILE__, __FUNCTION__, __LINE__, 1);
    QgsLogger::debug("height", image->height(), __FILE__, __FUNCTION__, __LINE__, 1);
#endif

    QgsDebugMsg("QgsRasterLayer::draw: Want to theQPainter->drawImage with");
#ifdef QGISDEBUG
    QgsLogger::debug("origin x", myRasterViewPort->topLeftPoint.x(), __FILE__, __FUNCTION__, __LINE__, 1);
    QgsLogger::debug("(int)origin x", static_cast<int>(myRasterViewPort->topLeftPoint.x()), __FILE__,\
        __FUNCTION__, __LINE__, 1);
    QgsLogger::debug("origin y", myRasterViewPort->topLeftPoint.y(), __FILE__, __FUNCTION__, __LINE__, 1);
    QgsLogger::debug("(int)origin y", static_cast<int>(myRasterViewPort->topLeftPoint.y()), __FILE__,\
        __FUNCTION__, __LINE__, 1);
#endif

    // Since GDAL's RasterIO can't handle floating point, we have to round to
    // the nearest pixel.  Add 0.5 to get rounding instead of truncation
    // out of static_cast<int>.

    theQPainter->drawImage(static_cast<int>(
          myRasterViewPort->topLeftPoint.x()
          + 0.5    // try simulating rounding instead of truncation, to avoid off-by-one errors
          // TODO: Check for rigorous correctness
          ),
        static_cast<int>(
          myRasterViewPort->topLeftPoint.y()
          + 0.5    // try simulating rounding instead of truncation, to avoid off-by-one errors
          // TODO: Check for rigorous correctness
          ),
        *image);

  }
  else
  {
    if ((myRasterViewPort->drawableAreaXDimInt) > 4000 &&  (myRasterViewPort->drawableAreaYDimInt > 4000))
    {
      // We have scaled one raster pixel to more than 4000 screen pixels. What's the point of showing this layer?
      // Instead, we just stop displaying the layer. Prevents allocating the entire world of memory for showing
      // very few pixels.
      // (Alternatively, we have a very big screen > 2000 x 2000)
      QgsDebugMsg("Too zoomed in! Displaying raster requires too much memory. Raster will not display");
    } else {
      // Otherwise use the old-fashioned GDAL direct-drawing style
      // TODO: Move into its own GDAL provider.

      // \/\/\/ - commented-out to handle zoomed-in rasters
    //    draw(theQPainter,myRasterViewPort);
      // /\/\/\ - commented-out to handle zoomed-in rasters
      // \/\/\/ - added to handle zoomed-in rasters
      draw(theQPainter, myRasterViewPort, theQgsMapToPixel);
      // /\/\/\ - added to handle zoomed-in rasters
    }

  }

  delete myRasterViewPort;
  QgsDebugMsg("QgsRasterLayer::draw: exiting.");

  return TRUE;

}

void QgsRasterLayer::draw (QPainter * theQPainter, 
    QgsRasterViewPort * theRasterViewPort,
    QgsMapToPixel * theQgsMapToPixel)
{
  QgsDebugMsg("QgsRasterLayer::draw (3 arguments)");
  //
  //
  // The goal here is to make as many decisions as possible early on (outside of the rendering loop)
  // so that we can maximise performance of the rendering process. So now we check which drawing
  // procedure to use :
  //


  switch (drawingStyle)
  {
    // a "Gray" or "Undefined" layer drawn as a range of gray colors
    case SINGLE_BAND_GRAY:
      //check the band is set!
      if (grayBandNameQString == tr("Not Set"))
      {
        break;
      }
      else
      {
        drawSingleBandGray(theQPainter, theRasterViewPort,
            theQgsMapToPixel, getRasterBandNumber(grayBandNameQString));
        break;
      }
      // a "Gray" or "Undefined" layer drawn using a pseudocolor algorithm
    case SINGLE_BAND_PSEUDO_COLOR:
      //check the band is set!
      if (grayBandNameQString == tr("Not Set"))
      {
        break;
      }
      else
      {
        drawSingleBandPseudoColor(theQPainter, theRasterViewPort, 
            theQgsMapToPixel, getRasterBandNumber(grayBandNameQString));
        break;
      }
      // a "Palette" layer drawn in gray scale (using only one of the color components)
    case PALETTED_SINGLE_BAND_GRAY:
      //check the band is set!
      if (grayBandNameQString == tr("Not Set"))
      {
        break;
      }
      else
      {
        QgsDebugMsg("PALETTED_SINGLE_BAND_GRAY drawing type detected...");

        int myBandNoInt = 1;
        drawPalettedSingleBandGray(theQPainter, theRasterViewPort, 
            theQgsMapToPixel, myBandNoInt, grayBandNameQString);

        break;
      }
      // a "Palette" layer having only one of its color components rendered as psuedo color
    case PALETTED_SINGLE_BAND_PSEUDO_COLOR:
      //check the band is set!
      if (grayBandNameQString == tr("Not Set"))
      {
        break;
      }
      else
      {

        int myBandNoInt = 1;
        drawPalettedSingleBandPseudoColor(theQPainter, theRasterViewPort,
            theQgsMapToPixel, myBandNoInt, grayBandNameQString);
        break;
      }
      //a "Palette" image where the bands contains 24bit color info and 8 bits is pulled out per color
    case PALETTED_MULTI_BAND_COLOR:
      drawPalettedMultiBandColor(theQPainter, theRasterViewPort,
          theQgsMapToPixel, 1);
      break;
      // a layer containing 2 or more bands, but using only one band to produce a grayscale image
    case MULTI_BAND_SINGLE_BAND_GRAY:
      QgsDebugMsg("MULTI_BAND_SINGLE_BAND_GRAY drawing type detected...");
      //check the band is set!
      if (grayBandNameQString == tr("Not Set"))
      {
        QgsDebugMsg("MULTI_BAND_SINGLE_BAND_GRAY Not Set detected..." + grayBandNameQString);
        break;
      }
      else
      {

        //get the band number for the mapped gray band
        drawMultiBandSingleBandGray(theQPainter, theRasterViewPort,
            theQgsMapToPixel, getRasterBandNumber(grayBandNameQString));
        break;
      }
      //a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
    case MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR:
      //check the band is set!
      if (grayBandNameQString == tr("Not Set"))
      {
        break;
      }
      else
      {

        drawMultiBandSingleBandPseudoColor(theQPainter, theRasterViewPort,
            theQgsMapToPixel, getRasterBandNumber(grayBandNameQString));
        break;
      }
      //a layer containing 2 or more bands, mapped to the three RGBcolors.
      //In the case of a multiband with only two bands, 
      //one band will have to be mapped to more than one color
    case MULTI_BAND_COLOR:
      if(redBandNameQString == tr("Not Set") || 
         greenBandNameQString == tr("Not Set") || 
         blueBandNameQString == tr("Not Set"))
      {
        break;
      }
      else
      {
        drawMultiBandColor(theQPainter, theRasterViewPort,
            theQgsMapToPixel);
      }
      break;

    default:
      break;

  }
  
  //see if debug info is wanted
  if (showDebugOverlayFlag)
  {
    showDebugOverlay(theQPainter, theRasterViewPort);
  };

}                               //end of draw method


void QgsRasterLayer::drawSingleBandGray(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,                                                                             QgsMapToPixel * theQgsMapToPixel, int theBandNoInt)
{
  QgsDebugMsg("QgsRasterLayer::drawSingleBandGray called for layer " + QString::number(theBandNoInt));
  //QgsRasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if (myGdalScanData == NULL)
  {
    return;
  }

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  //myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  myQImage.fill(qRgba(255,255,255,0 )); // fill transparent

  //double myRangeDouble = myRasterBandStats.rangeDouble;
  QgsRasterBandStats myGrayBandStats;
  /*
   * If stDevsToPlotDouble is set it will override any user defined Min Max values
   */
  if(QgsRasterLayer::NO_STRETCH != getColorScalingAlgorithm() && (!userDefinedGrayMinMax || stdDevsToPlotDouble > 0))
  {
    myGrayBandStats = getRasterBandStats(theBandNoInt);

    /*
     * This may upset some, but these values are set directly so that the userDefinedColorMinMax variable is not set, 
     * though it really does not matter by this point.
     */
    if(stdDevsToPlotDouble > 0)
    {
      minGrayDouble = myGrayBandStats.meanDouble - (stdDevsToPlotDouble * myGrayBandStats.stdDevDouble);
      maxGrayDouble = myGrayBandStats.meanDouble + (stdDevsToPlotDouble * myGrayBandStats.stdDevDouble);
    }
    else 
    {
      minGrayDouble = myGrayBandStats.minValDouble;
      maxGrayDouble = myGrayBandStats.maxValDouble;
    }
  }
  
  /*
   * Check for invalid min max value based on GDALDataType.
   * Invalid values can happen if the user uses stdDevs to set min and max
   * TODO:Needs to be expanded for all GDALDataTypes
   */
  if(GDT_Byte == myDataType)
  {
    if(minGrayDouble < 0.0)
      minGrayDouble = 0.0;
    if(maxGrayDouble > 255.0)
      maxGrayDouble = 255.0;
  }

  QgsDebugMsg("Starting main render loop");
  // print each point in myGdalScanData with equal parts R, G ,B o make it show as gray
  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      double myGrayValueDouble = readValue ( myGdalScanData, myDataType,
          myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );

      // If noDataValueDouble is 'nan', the comparison
      // against myGrayValDouble will always fail ( nan==nan always
      // returns false, by design), hence the slightly odd comparison
      // of myGrayValDouble against itself. 
      if ( myGrayValueDouble == noDataValueDouble ||
          myGrayValueDouble != myGrayValueDouble)
      {

        myQImage.setPixel(myRowInt, myColumnInt, qRgba(255,255,255,0 ));
        continue;
      }

      if(QgsRasterLayer::NO_STRETCH != getColorScalingAlgorithm())
      {
        if(QgsRasterLayer::CLIP_TO_MINMAX == getColorScalingAlgorithm() || QgsRasterLayer::STRETCH_AND_CLIP_TO_MINMAX == getColorScalingAlgorithm())
          if(myGrayValueDouble < minGrayDouble || myGrayValueDouble > maxGrayDouble) continue;

        if(QgsRasterLayer::STRETCH_TO_MINMAX == getColorScalingAlgorithm() || QgsRasterLayer::STRETCH_AND_CLIP_TO_MINMAX == getColorScalingAlgorithm())
          myGrayValueDouble = ((myGrayValueDouble - minGrayDouble)/(maxGrayDouble - minGrayDouble))*255;

        //Check for out of range pixel values
        if(myGrayValueDouble < 0.0)
          myGrayValueDouble = 0.0;
        else if(myGrayValueDouble > 255.0)
          myGrayValueDouble = 255.0;
      }

      int myGrayValInt = static_cast < int > ( myGrayValueDouble);
      //int myGrayValInt = static_cast < int >( (myGrayValDouble-myRasterBandStats.minValDouble) * (255/myRangeDouble));

      if (invertHistogramFlag)
      {
        myGrayValInt = 255 - myGrayValInt;
      }
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myGrayValInt, myGrayValInt, myGrayValInt, mTransparencyLevel));
    }
  }
  
  CPLFree ( myGdalScanData );

  QgsDebugMsg("Render done, preparing to copy to canvas");
  //render any inline filters
  filterLayer(&myQImage);

  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;

  if (theQgsMapToPixel)
  {
    paintXoffset = static_cast<int>( 
        (theRasterViewPort->rectXOffsetFloat - 
         theRasterViewPort->rectXOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[1])
        ); 

    paintYoffset = static_cast<int>( 
        (theRasterViewPort->rectYOffsetFloat - 
         theRasterViewPort->rectYOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[5])
        ); 
  }                                   

  QgsDebugMsg("QgsRasterLayer::drawSingleBandGray: painting image to canvas from "\
      + QString::number(paintXoffset) + ", " + QString::number(paintYoffset)\
      + " to "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5))\
      + ", "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5))\
      + ".");

  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5),
      static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5),
      myQImage,
      paintXoffset,
      paintYoffset);

} // QgsRasterLayer::drawSingleBandGray


void QgsRasterLayer::drawSingleBandPseudoColor(QPainter * theQPainter, 
    QgsRasterViewPort * theRasterViewPort,
    QgsMapToPixel * theQgsMapToPixel, 
    int theBandNoInt)
{
  QgsDebugMsg("QgsRasterLayer::drawSingleBandPseudoColor called");

  QgsRasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if (myGdalScanData == NULL)
  {
    return;
  }

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  //myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  myQImage.fill(qRgba(255,255,255,0 )); // fill transparent

  //calculate the adjusted matrix stats - which come into effect if the user has chosen
  QgsRasterBandStats myAdjustedRasterBandStats = getRasterBandStats(theBandNoInt);

  int myRedInt = 0;
  int myGreenInt = 0;
  int myBlueInt = 0;

  //to histogram stretch to a given number of std deviations
  //see if we are using histogram stretch using stddev and plot only within the selected
  //number of deviations if we are
  if (stdDevsToPlotDouble > 0)
  {
    //work out how far on either side of the mean we should include data
    float myTotalDeviationDouble = stdDevsToPlotDouble * myAdjustedRasterBandStats.stdDevDouble;
    //adjust min and max accordingly
    //only change min if it is less than mean  -  (n  x  deviations)
    if (noDataValueDouble < (myAdjustedRasterBandStats.meanDouble - myTotalDeviationDouble))
    {
      noDataValueDouble = (myAdjustedRasterBandStats.meanDouble - myTotalDeviationDouble);
    }
    //only change max if it is greater than mean  +  (n  x  deviations)
    if (myAdjustedRasterBandStats.maxValDouble > (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble))
    {
      myAdjustedRasterBandStats.maxValDouble = (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble);
    }
    //update the range
    myAdjustedRasterBandStats.rangeDouble = myAdjustedRasterBandStats.maxValDouble - noDataValueDouble;
  }

  //set up the three class breaks for pseudocolour mapping
  double myBreakSizeDouble = myAdjustedRasterBandStats.rangeDouble / 3;
  double myClassBreakMin1 = myRasterBandStats.minValDouble;
  double myClassBreakMax1 = myClassBreakMin1 + myBreakSizeDouble;
  double myClassBreakMin2 = myClassBreakMax1;
  double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
  double myClassBreakMin3 = myClassBreakMax2;

  myQImage.setAlphaBuffer(true);

  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      //hardcoding to white to start with
      myRedInt = 255;
      myBlueInt = 255;
      myGreenInt = 255;
      double myValDouble = readValue ( myGdalScanData, myDataType,
          myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );

      if ( myValDouble == noDataValueDouble || myValDouble != myValDouble ) continue;

      //double check that myInt >= min and <= max
      //this is relevant if we are plotting within stddevs
      if ( myValDouble < myAdjustedRasterBandStats.minValDouble )
      {
        myValDouble = myAdjustedRasterBandStats.minValDouble;
      }
      if ( myValDouble > myAdjustedRasterBandStats.maxValDouble)
      {
        myValDouble = myAdjustedRasterBandStats.maxValDouble;
      }

      if (!invertHistogramFlag)
      {
        //check if we are in the first class break
        if ((myValDouble >= myClassBreakMin1) && (myValDouble < myClassBreakMax1))
        {
          myRedInt = 0;
          myBlueInt = 255;
          myGreenInt = static_cast < int >( ( (255 / myAdjustedRasterBandStats.rangeDouble)
                * (myValDouble - myClassBreakMin1) ) * 3 );
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=255-myGreenInt;
          }
        }
        //check if we are in the second class break
        else if ( (myValDouble >= myClassBreakMin2) && (myValDouble < myClassBreakMax2) )
        {
          myRedInt = static_cast < int >( ( (255 / myAdjustedRasterBandStats.rangeDouble)
                * ((myValDouble - myClassBreakMin2) / 1)) * 3);
          myBlueInt = static_cast < int >(255 - ( ( (255 / myAdjustedRasterBandStats.rangeDouble)
                  * ((myValDouble - myClassBreakMin2) / 1)) * 3));
          myGreenInt = 255;
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myGreenInt=myBlueInt;
          }
        }
        //otherwise we must be in the third classbreak
        else
        {
          myRedInt = 255;
          myBlueInt = 0;
          myGreenInt = static_cast < int >(255 - ( ( (255 / myAdjustedRasterBandStats.rangeDouble) *
                  ((myValDouble - myClassBreakMin3) / 1) * 3)));
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=myGreenInt;
            myGreenInt=255-myGreenInt;
          }
        }
      }
      else            //invert histogram toggle is on
      {
        //check if we are in the first class break
        if ((myValDouble >= myClassBreakMin1) && (myValDouble < myClassBreakMax1))
        {
          myRedInt = 255;
          myBlueInt = 0;
          myGreenInt = static_cast < int >( ( (255 / myAdjustedRasterBandStats.rangeDouble)
                * ((myValDouble - myClassBreakMin1) / 1) * 3));
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=255-myGreenInt;
          }
        }
        //check if we are in the second class break
        else if ((myValDouble >= myClassBreakMin2) && (myValDouble < myClassBreakMax2))
        {
          myRedInt = static_cast < int >(255 - ( ( (255 / myAdjustedRasterBandStats.rangeDouble)
                  * ((myValDouble - myClassBreakMin2) / 1)) * 3));
          myBlueInt = static_cast < int >( ( (255 / myAdjustedRasterBandStats.rangeDouble)
                * ((myValDouble - myClassBreakMin2) / 1)) * 3);
          myGreenInt = 255;
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myGreenInt=myBlueInt;
          }
        }
        //otherwise we must be in the third classbreak
        else
        {
          myRedInt = 0;
          myBlueInt = 255;
          myGreenInt = static_cast < int >(255 - ( ( (255 / myAdjustedRasterBandStats.rangeDouble)
                  * (myValDouble - myClassBreakMin3)) * 3));
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=myGreenInt;
            myGreenInt=255-myGreenInt;
          }
        }
      }
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedInt, myGreenInt, myBlueInt, mTransparencyLevel));
    }                       //end of columnwise loop
  }                           //end of towwise loop

  CPLFree ( myGdalScanData );

  //render any inline filters
  filterLayer(&myQImage);

  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;

  if (theQgsMapToPixel)
  {
    paintXoffset = static_cast<int>( 
        (theRasterViewPort->rectXOffsetFloat - 
         theRasterViewPort->rectXOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[1])
        ); 

    paintYoffset = static_cast<int>( 
        (theRasterViewPort->rectYOffsetFloat - 
         theRasterViewPort->rectYOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[5])
        ); 
  }                                   

  QgsDebugMsg("QgsRasterLayer - painting image to canvas from "\
      + QString::number(paintXoffset) + ", " + QString::number(paintYoffset)\
      + " to "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5))\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5))\
      + ".");

  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5),
      static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5),
      myQImage,
      paintXoffset,
      paintYoffset);

}

/**
 * This method is used to render a paletted raster layer as a colour image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 */
void QgsRasterLayer::drawPalettedSingleBandColor(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort, 
    QgsMapToPixel * theQgsMapToPixel, int theBandNoInt)
{
  QgsDebugMsg("QgsRasterLayer::drawPalettedSingleBandColor called");

  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if (myGdalScanData == NULL)
  {
    return;
  }

  QgsColorTable *myColorTable = colorTable ( theBandNoInt );

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  //myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  myQImage.fill(qRgba(255,255,255,0 )); // fill transparent

  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      double myValDouble = readValue ( myGdalScanData, myDataType,
          myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );

      if ( myValDouble == noDataValueDouble || myValDouble != myValDouble ) continue; // NULL

      int c1, c2, c3;
      bool found = myColorTable->color ( myValDouble, &c1, &c2, &c3 );
      if ( !found ) continue;

      if (invertHistogramFlag)
      {
        c1 = 255 - c1;
        c2 = 255 - c2;
        c3 = 255 - c3;
      }
      //set the pixel based on the above color mappings
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(c1, c2, c3, mTransparencyLevel));
    }
  }

  CPLFree(myGdalScanData);

  //render any inline filters
  filterLayer(&myQImage);

  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;

  if (theQgsMapToPixel)
  {
    paintXoffset = static_cast<int>( 
        (theRasterViewPort->rectXOffsetFloat - 
         theRasterViewPort->rectXOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[1])
        ); 

    paintYoffset = static_cast<int>( 
        (theRasterViewPort->rectYOffsetFloat - 
         theRasterViewPort->rectYOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[5])
        ); 
  }                                   

  QgsDebugMsg("QgsRasterLayer - painting image to canvas from "\
      + QString::number(paintXoffset) + ", " + QString::number(paintYoffset)\
      + " to "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5))\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5))\
      + ".");

  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5),
      static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5),
      myQImage,
      paintXoffset,
      paintYoffset);

  CPLFree(myGdalScanData);
}


/**
 * This method is used to render a paletted raster layer as a gray image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 * @param theColorQString - QString containing either 'Red' 'Green' or 'Blue' indicating which part of the rgb triplet will be used to render gray.
 */
void QgsRasterLayer::drawPalettedSingleBandGray(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort, 
    QgsMapToPixel * theQgsMapToPixel, int theBandNoInt,
    QString const & theColorQString)
{
  QgsDebugMsg("QgsRasterLayer::drawPalettedSingleBandGray called");

  QgsRasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if (myGdalScanData == NULL)
  {
    return;
  }

  QgsColorTable *myColorTable = &(myRasterBandStats.colorTable);

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  //myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  myQImage.fill(qRgba(255,255,255,0 )); // fill transparent

  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      double myValDouble = readValue ( myGdalScanData, myDataType,
          myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );

      if ( myValDouble == noDataValueDouble || myValDouble != myValDouble ) continue; // NULL

      int c1, c2, c3;
      bool found = myColorTable->color ( myValDouble, &c1, &c2, &c3 );
      if ( !found ) continue;

      int myGrayValueInt = 0; //color 1 int

      if (theColorQString == redBandNameQString)
      {
        myGrayValueInt = c1;
      }
      else if (theColorQString == greenBandNameQString)
      {
        myGrayValueInt = c2;
      }
      else if (theColorQString == blueBandNameQString)
      {
        myGrayValueInt = c3;
      }

      if (invertHistogramFlag)
      {
        myGrayValueInt = 255 - myGrayValueInt;
      }

      //set the pixel based on the above color mappings
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myGrayValueInt, myGrayValueInt, myGrayValueInt, mTransparencyLevel));
    }
  }
  CPLFree ( myGdalScanData );

  //render any inline filters
  filterLayer(&myQImage);

  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;

  if (theQgsMapToPixel)
  {
    paintXoffset = static_cast<int>( 
        (theRasterViewPort->rectXOffsetFloat - 
         theRasterViewPort->rectXOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[1])
        ); 

    paintYoffset = static_cast<int>( 
        (theRasterViewPort->rectYOffsetFloat - 
         theRasterViewPort->rectYOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[5])
        ); 
  }                                   

  QgsDebugMsg("QgsRasterLayer - painting image to canvas from "\
      + QString::number(paintXoffset) + ", " + QString::number(paintYoffset)\
      + " to "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5))\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5))\
      + ".");

  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5),
      static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5),
      myQImage,
      paintXoffset,
      paintYoffset);

}


/**
 * This method is used to render a paletted raster layer as a pseudocolor image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 * @param theColorQString - QString containing either 'Red' 'Green' or 'Blue' indicating which part of the rgb triplet will be used to render gray.
 */
void QgsRasterLayer::drawPalettedSingleBandPseudoColor(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    QgsMapToPixel * theQgsMapToPixel, int theBandNoInt, 
    QString const & theColorQString)
{
  QgsDebugMsg("QgsRasterLayer::drawPalettedSingleBandPseudoColor called");
  QgsRasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if (myGdalScanData == NULL)
  {
    return;
  }

  QgsColorTable *myColorTable = &(myRasterBandStats.colorTable);

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  //myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  myQImage.fill(qRgba(255,255,255,0 )); // fill transparent

  int myRedInt = 0;
  int myGreenInt = 0;
  int myBlueInt = 0;

  //calculate the adjusted matrix stats - which come into affect if the user has chosen
  QgsRasterBandStats myAdjustedRasterBandStats = getRasterBandStats(theBandNoInt);

  //to histogram stretch to a given number of std deviations
  //see if we are using histogram stretch using stddev and plot only within the selected number of deviations if we are
  if (stdDevsToPlotDouble > 0)
  {
    //work out how far on either side of the mean we should include data
    float myTotalDeviationDouble = stdDevsToPlotDouble * myAdjustedRasterBandStats.stdDevDouble;
    //printf("myTotalDeviationDouble: %i\n" , myTotalDeviationDouble );
    //adjust min and max accordingly
    //only change min if it is less than mean  -  (n  x  deviations)
    if (noDataValueDouble < (myAdjustedRasterBandStats.meanDouble - myTotalDeviationDouble))
    {
      noDataValueDouble = (myAdjustedRasterBandStats.meanDouble - myTotalDeviationDouble);
    }
    //only change max if it is greater than mean  +  (n  x  deviations)
    if (myAdjustedRasterBandStats.maxValDouble > (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble))
    {
      myAdjustedRasterBandStats.maxValDouble = (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble);
    }
    //update the range
    myAdjustedRasterBandStats.rangeDouble = myAdjustedRasterBandStats.maxValDouble - noDataValueDouble;
  }
  //set up the three class breaks for pseudocolour mapping
  double myBreakSizeDouble = myAdjustedRasterBandStats.rangeDouble / 3;
  double myClassBreakMin1 = myRasterBandStats.minValDouble;
  double myClassBreakMax1 = myClassBreakMin1 + myBreakSizeDouble;
  double myClassBreakMin2 = myClassBreakMax1;
  double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
  double myClassBreakMin3 = myClassBreakMax2;


  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      double myValDouble = readValue ( myGdalScanData, myDataType,
          myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );

      if ( myValDouble == noDataValueDouble || myValDouble != myValDouble ) continue; // NULL

      int c1, c2, c3;
      bool found = myColorTable->color ( myValDouble, &c1, &c2, &c3 );
      if ( !found ) continue;

      int myInt=0;

      //check for alternate color mappings
      if (theColorQString == redBandNameQString)
      {
        myInt = c1;
      }
      else if (theColorQString == greenBandNameQString)
      {
        myInt = c2;
      }
      else if (theColorQString == blueBandNameQString)
      {
        myInt = c3;
      }
      //dont draw this point if it is no data !
      //double check that myInt >= min and <= max
      //this is relevant if we are plotting within stddevs

      if ( myInt < myAdjustedRasterBandStats.minValDouble )
      {
        myInt = static_cast < int >(myAdjustedRasterBandStats.minValDouble);
      }
      else if ( myInt > myAdjustedRasterBandStats.maxValDouble )
      {
        myInt = static_cast < int >(myAdjustedRasterBandStats.maxValDouble);
      }

      if (!invertHistogramFlag)
      {
        //check if we are in the first class break
        if ((myInt >= myClassBreakMin1) && (myInt < myClassBreakMax1))
        {
          myRedInt = 0;
          myBlueInt = 255;
          myGreenInt = static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble)
                * (myInt - myClassBreakMin1)) * 3);
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=255-myGreenInt;
          }
        }
        //check if we are in the second class break
        else if ((myInt >= myClassBreakMin2) && (myInt < myClassBreakMax2))
        {
          myRedInt = static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble)
                * ((myInt - myClassBreakMin2) / 1)) * 3);
          myBlueInt = static_cast < int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble)
                  * ((myInt - myClassBreakMin2) / 1)) * 3));
          myGreenInt = 255;
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myGreenInt=myBlueInt;
          }
        }
        //otherwise we must be in the third classbreak
        else
        {
          myRedInt = 255;
          myBlueInt = 0;
          myGreenInt = static_cast < int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble)
                  * ((myInt - myClassBreakMin3) / 1) * 3)));
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=myGreenInt;
            myGreenInt=255-myGreenInt;
          }
        }
      }
      else            //invert histogram toggle is on
      {
        //check if we are in the first class break
        if ((myInt >= myClassBreakMin1) && (myInt < myClassBreakMax1))
        {
          myRedInt = 255;
          myBlueInt = 0;
          myGreenInt =
            static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin1) / 1) * 3));
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=255-myGreenInt;
          }
        }
        //check if we are in the second class break
        else if ((myInt >= myClassBreakMin2) && (myInt < myClassBreakMax2))
        {
          myRedInt =
            static_cast <
            int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3));
          myBlueInt =
            static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3);
          myGreenInt = 255;
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myGreenInt=myBlueInt;
          }
        }
        //otherwise we must be in the third classbreak
        else
        {
          myRedInt = 0;
          myBlueInt = 255;
          myGreenInt =
            static_cast < int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * (myInt - myClassBreakMin3)) * 3));
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=255-myGreenInt;
            myGreenInt=myGreenInt;
          }
        }


      }
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedInt, myGreenInt, myBlueInt, mTransparencyLevel));
    }
  }
  CPLFree ( myGdalScanData );

  //render any inline filters
  filterLayer(&myQImage);

  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;

  if (theQgsMapToPixel)
  {
    paintXoffset = static_cast<int>( 
        (theRasterViewPort->rectXOffsetFloat - 
         theRasterViewPort->rectXOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[1])
        ); 

    paintYoffset = static_cast<int>( 
        (theRasterViewPort->rectYOffsetFloat - 
         theRasterViewPort->rectYOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[5])
        ); 
  }                                   

  QgsDebugMsg("QgsRasterLayer - painting image to canvas from "\
      + QString::number(paintXoffset) + ", " + QString::number(paintYoffset)\
      + " to "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5))\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5))\
      + ".");

  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5),
      static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5),
      myQImage,
      paintXoffset,
      paintYoffset);

}

/**
 * This method is used to render a paletted raster layer as a colour image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 */
void QgsRasterLayer::drawPalettedMultiBandColor(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort, 
    QgsMapToPixel * theQgsMapToPixel, int theBandNoInt)
{
  QgsDebugMsg("QgsRasterLayer::drawPalettedMultiBandColor called");

  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );

  /* Check for out of memory error */
  if (myGdalScanData == NULL)
  {
    return;
  }

  QgsColorTable *myColorTable = colorTable ( theBandNoInt );

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  //myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  myQImage.fill(qRgba(255,255,255,0 )); // fill transparent

  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      double myValDouble = readValue ( myGdalScanData, myDataType,
          myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );

      if ( myValDouble == noDataValueDouble || myValDouble != myValDouble ) continue; // NULL

      int c1, c2, c3;
      bool found = myColorTable->color ( myValDouble, &c1, &c2, &c3 );
      if ( !found ) continue;

      int myRedValueInt = 0;  //color 1 int
      int myGreenValueInt = 0;  //color 2 int
      int myBlueValueInt = 0; //color 3 int

      //check for alternate color mappings
      if (redBandNameQString == "Red")
        myRedValueInt = c1;
      else if (redBandNameQString == "Green")
        myRedValueInt = c2;
      else if (redBandNameQString == "Blue")
        myRedValueInt = c3;

      if (greenBandNameQString == "Red")
        myGreenValueInt = c1;
      else if (greenBandNameQString == "Green")
        myGreenValueInt = c2;
      else if (greenBandNameQString == "Blue")

        myGreenValueInt = c3;

      if (blueBandNameQString == "Red")
        myBlueValueInt = c1;
      else if (blueBandNameQString == "Green")
        myBlueValueInt = c2;
      else if (blueBandNameQString == "Blue")
        myBlueValueInt = c3;

      if (invertHistogramFlag)
      {
        myRedValueInt = 255 - myRedValueInt;
        myGreenValueInt = 255 - myGreenValueInt;
        myBlueValueInt = 255 - myBlueValueInt;

      }
      //set the pixel based on the above color mappings
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedValueInt, myGreenValueInt, myBlueValueInt, mTransparencyLevel));
    }
  }
  //render any inline filters
  filterLayer(&myQImage);

  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;

  if (theQgsMapToPixel)
  {
    paintXoffset = static_cast<int>( 
        (theRasterViewPort->rectXOffsetFloat - 
         theRasterViewPort->rectXOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[1])
        ); 

    paintYoffset = static_cast<int>( 
        (theRasterViewPort->rectYOffsetFloat - 
         theRasterViewPort->rectYOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[5])
        ); 
  }                                   

  QgsDebugMsg("QgsRasterLayer::drawSingleBandGray: painting image to canvas from "\
      + QString::number(paintXoffset) + ", " + QString::number(paintYoffset)\
      + " to "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5))\
      + ", "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5))\
      + ".");

  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5),
      static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5),
      myQImage,
      paintXoffset,
      paintYoffset);

  CPLFree(myGdalScanData);
}


void QgsRasterLayer::drawMultiBandSingleBandGray(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort, 
    QgsMapToPixel * theQgsMapToPixel, int theBandNoInt)
{
  //delegate to drawSingleBandGray!
  drawSingleBandGray(theQPainter, theRasterViewPort, theQgsMapToPixel, theBandNoInt);
}


void QgsRasterLayer::drawMultiBandSingleBandPseudoColor(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort, 
    QgsMapToPixel * theQgsMapToPixel, int theBandNoInt)
{
  //delegate to drawSinglePseudocolor!
  drawSingleBandPseudoColor(theQPainter, theRasterViewPort, theQgsMapToPixel, theBandNoInt);
}


void QgsRasterLayer::drawMultiBandColor(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,
    QgsMapToPixel * theQgsMapToPixel)
{
  QgsDebugMsg("QgsRasterLayer::drawMultiBandColor called");

  int myRedBandNoInt = getRasterBandNumber(redBandNameQString);
  int myGreenBandNoInt = getRasterBandNumber(greenBandNameQString);
  int myBlueBandNoInt = getRasterBandNumber(blueBandNameQString);
  GDALRasterBand *myGdalRedBand = gdalDataset->GetRasterBand(myRedBandNoInt);
  GDALRasterBand *myGdalGreenBand = gdalDataset->GetRasterBand(myGreenBandNoInt);
  GDALRasterBand *myGdalBlueBand = gdalDataset->GetRasterBand(myBlueBandNoInt);

  GDALDataType myRedType = myGdalRedBand->GetRasterDataType();
  GDALDataType myGreenType = myGdalGreenBand->GetRasterDataType();
  GDALDataType myBlueType = myGdalBlueBand->GetRasterDataType();

  void *myGdalRedData = readData ( myGdalRedBand, theRasterViewPort );
  void *myGdalGreenData = readData ( myGdalGreenBand, theRasterViewPort );
  void *myGdalBlueData = readData ( myGdalBlueBand, theRasterViewPort );

  /* Check for out of memory error */
  if (myGdalRedData == NULL || myGdalGreenData == NULL || myGdalBlueData == NULL)
  {
    // Safe to free NULL-pointer */
    VSIFree(myGdalRedData);
    VSIFree(myGdalGreenData);
    VSIFree(myGdalBlueData);
    return;
  }

  bool haveTransparencyBand(false);
  GDALRasterBand *myGdalTransparentBand = NULL;
  GDALDataType myTransparentType = GDT_Byte; //default to prevent uninitialised var warnings
  void *myGdalTransparentData = NULL;

  if (transparentBandNameQString != tr("Not Set"))
  {
    haveTransparencyBand = true;
    int myTransparentBandNoInt = getRasterBandNumber(transparentBandNameQString); 
    myGdalTransparentBand = gdalDataset->GetRasterBand(myTransparentBandNoInt);
    myTransparentType = myGdalTransparentBand->GetRasterDataType();
    myGdalTransparentData = readData ( myGdalTransparentBand, theRasterViewPort );
    if (myGdalTransparentData == NULL)
    {
      // Safe to free NULL-pointer */
      VSIFree(myGdalRedData);
      VSIFree(myGdalGreenData);
      VSIFree(myGdalBlueData);
      return;
    }
  }

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  //myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  myQImage.fill(qRgba(255,255,255,0 )); // fill transparent

  QgsRasterBandStats myRedBandStats;
  QgsRasterBandStats myGreenBandStats;
  QgsRasterBandStats myBlueBandStats;
  /*
   * If a stetch is requested and there are no user defined Min Max values
   * we need to get these values from the bands themselves.
   * If stDevsToPlotDouble is set it will override any user defined Min Max values
   *
   * NOTE: If the user only set minRedDouble this next block would be skipped 
   * and all the other variables could have garbage (they are not initialized in the constructor) 
   * This may want to be updated so that each Min Max is check to be sure it was set and if one is missing
   * the get the QgsRasterBandStats for that band.
   */
  if(QgsRasterLayer::NO_STRETCH != getColorScalingAlgorithm() && (!userDefinedColorMinMax || stdDevsToPlotDouble > 0))
  {
    myRedBandStats = getRasterBandStats(myRedBandNoInt);
    myGreenBandStats = getRasterBandStats(myGreenBandNoInt);
    myBlueBandStats = getRasterBandStats(myBlueBandNoInt);

    /*
     * This may upset some, but these values are set directly so that the userDefinedColorMinMax variable is not set, 
     * though it really does not matter at this point. Also insignificantly faster.
     */
    if(stdDevsToPlotDouble > 0)
    {
      minRedDouble = myRedBandStats.meanDouble - (stdDevsToPlotDouble * myRedBandStats.stdDevDouble);
      maxRedDouble = myRedBandStats.meanDouble + (stdDevsToPlotDouble * myRedBandStats.stdDevDouble);
      minGreenDouble = myGreenBandStats.meanDouble - (stdDevsToPlotDouble * myGreenBandStats.stdDevDouble);
      maxGreenDouble = myGreenBandStats.meanDouble + (stdDevsToPlotDouble * myGreenBandStats.stdDevDouble);
      minBlueDouble = myBlueBandStats.meanDouble - (stdDevsToPlotDouble * myBlueBandStats.stdDevDouble);
      maxBlueDouble = myBlueBandStats.meanDouble + (stdDevsToPlotDouble * myBlueBandStats.stdDevDouble);
    }
    else 
    {
      minRedDouble = myRedBandStats.minValDouble;
      maxRedDouble = myRedBandStats.maxValDouble;
      minGreenDouble = myGreenBandStats.minValDouble;
      maxGreenDouble = myGreenBandStats.maxValDouble;
      minBlueDouble = myBlueBandStats.minValDouble;
      maxBlueDouble = myBlueBandStats.maxValDouble;
    }
  }

  /*
   * Check for invalid min max value based on GDALDataType.
   * Invalid values can happen if the user uses stdDevs to set min and max
   * TODO:Needs to be expanded for all GDALDataTypes
   */
  if(GDT_Byte == myRedType)
  {
    if(minRedDouble < 0.0)
      minRedDouble = 0.0;
    if(maxRedDouble > 255.0)
      maxRedDouble = 255.0;
  }

  if(GDT_Byte == myGreenType)
  {
    if(minGreenDouble < 0.0)
      minGreenDouble = 0.0;
    if(maxGreenDouble > 255.0)
      maxGreenDouble = 255.0;
  }

  if(GDT_Byte == myBlueType)
  {
    if(minBlueDouble < 0.0)
      minBlueDouble = 0.0;
    if(maxBlueDouble > 255.0)
      maxBlueDouble = 255.0;
  }

  //Read and display pixels
  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      double myRedValueDouble   = readValue ( myGdalRedData, myRedType,
          myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );
      double myGreenValueDouble = readValue ( myGdalGreenData, myGreenType,
          myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );
      double myBlueValueDouble  = readValue ( myGdalBlueData, myBlueType,
          myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );
      if (haveTransparencyBand)
      {
        double myTransparentValueDouble  = readValue ( myGdalTransparentData, myTransparentType,
            myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );
        if (myTransparentValueDouble == 0.0)
          continue;
      }

	if((myRedValueDouble == noDataValueDouble || myRedValueDouble != myRedValueDouble) || (myGreenValueDouble == noDataValueDouble || myGreenValueDouble != myGreenValueDouble) || (myBlueValueDouble == noDataValueDouble || myBlueValueDouble != myBlueValueDouble))
      {
        continue;
      }

      /*
       * Stretch RBG values based on selected color scaling algoritm
       * NOTE: NO_STRETCH enum will help eliminte the need to call QgsRasterBandStats when an image is initially loaded
       */
      
      if(QgsRasterLayer::NO_STRETCH != getColorScalingAlgorithm())
      {
        /*
         * Currently if any one band is outside of min max range for the band the pixel is discarded,
         * this can easily be updated so that all band have to be ouside of the min max range
         */
        if(QgsRasterLayer::CLIP_TO_MINMAX == getColorScalingAlgorithm() || QgsRasterLayer::STRETCH_AND_CLIP_TO_MINMAX == getColorScalingAlgorithm())
        {
          if(myRedValueDouble < minRedDouble || myRedValueDouble > maxRedDouble) continue;
          if(myGreenValueDouble < minRedDouble || myGreenValueDouble > maxRedDouble) continue;
          if(myBlueValueDouble < minBlueDouble || myBlueValueDouble > maxBlueDouble) continue;
        }
        if(QgsRasterLayer::STRETCH_TO_MINMAX == getColorScalingAlgorithm() || QgsRasterLayer::STRETCH_AND_CLIP_TO_MINMAX == getColorScalingAlgorithm())
        {
          myRedValueDouble = ((myRedValueDouble - minRedDouble)/(maxRedDouble - minRedDouble))*255;
          myGreenValueDouble = ((myGreenValueDouble - minGreenDouble)/(maxGreenDouble - minGreenDouble))*255;
          myBlueValueDouble = ((myBlueValueDouble - minBlueDouble)/(maxBlueDouble - minBlueDouble))*255;
        }
        //Check for out of range pixel values
        if(myRedValueDouble < 0.0)
          myRedValueDouble = 0.0;
        else if(myRedValueDouble > 255.0)
          myRedValueDouble = 255.0;
        if(myGreenValueDouble < 0.0)
          myGreenValueDouble = 0.0;
        else if(myGreenValueDouble > 255.0)
          myGreenValueDouble = 255.0;
        if(myBlueValueDouble < 0.0)
          myBlueValueDouble = 0.0;
        else if(myBlueValueDouble > 255.0)
          myBlueValueDouble = 255.0;
      }

      int myRedValueInt   = static_cast < int > ( myRedValueDouble );
      int myGreenValueInt = static_cast < int > ( myGreenValueDouble );
      int myBlueValueInt  = static_cast < int > ( myBlueValueDouble );

      if (invertHistogramFlag)
      {
        myRedValueInt = 255 - myRedValueInt;
        myGreenValueInt = 255 - myGreenValueInt;
        myBlueValueInt = 255 - myBlueValueInt;
      }

      //set the pixel based on the above color mappings
      myQImage.setPixel ( myRowInt, myColumnInt,
          qRgba(myRedValueInt, myGreenValueInt, myBlueValueInt, mTransparencyLevel) );
    }
  }

  //render any inline filters
  filterLayer(&myQImage);

#ifdef QGISDEBUG
  QPixmap* pm = dynamic_cast<QPixmap*>(theQPainter->device());
  if(pm)
  {
    QgsDebugMsg("QgsRasterLayer::drawMultiBandColor: theQPainter stats: ");
    QgsDebugMsg("width = " + QString::number(pm->width()));
    QgsDebugMsg("height = " + QString::number(pm->height()));
    pm->save("/tmp/qgis-rasterlayer-drawmultibandcolor-test-a.png", "PNG");
  }
#endif

  // \/\/\/ - added to handle zoomed-in rasters

  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;

  if (theQgsMapToPixel)
  {
    paintXoffset = static_cast<int>( 
        (theRasterViewPort->rectXOffsetFloat - 
         theRasterViewPort->rectXOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[1])
        ); 

    paintYoffset = static_cast<int>( 
        (theRasterViewPort->rectYOffsetFloat - 
         theRasterViewPort->rectYOffsetInt)
        / theQgsMapToPixel->mapUnitsPerPixel() 
        * fabs(adfGeoTransform[5])
        ); 
  }

  QgsDebugMsg("QgsRasterLayer::drawSingleBandGray: painting image to canvas from source NW"\
      + QString::number(paintXoffset) + ", " + QString::number(paintYoffset)\
      + " to "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5))\
      + ", "\
      + QString::number(static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5))\
      + ".");

  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5),
      static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5),
      myQImage,
      paintXoffset,
      paintYoffset);

#ifdef QGISDEBUG
  QgsDebugMsg("QgsRasterLayer::drawMultiBandColor: theQPainter->drawImage.");
  if(pm)
  {
    pm->save("/tmp/qgis-rasterlayer-drawmultibandcolor-test-b.png", "PNG");
  }
#endif

  //free the scanline memory
  CPLFree(myGdalRedData);
  CPLFree(myGdalGreenData);
  CPLFree(myGdalBlueData);
  if (haveTransparencyBand)
    CPLFree(myGdalTransparentData);
}



/**
 * Call any inline filters
 */
void QgsRasterLayer::filterLayer(QImage * theQImage)
{
  //do stuff here....
  //return;
}

/**
  Print some debug info to the qpainter
  */

void QgsRasterLayer::showDebugOverlay(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort)
{


  QFont myQFont("arial", 10, QFont::Bold);
  theQPainter->setFont(myQFont);
  theQPainter->setPen(Qt::black);
  QBrush myQBrush(qRgba(128, 128, 164, 50), Qt::Dense6Pattern); //semi transparent
  theQPainter->setBrush(myQBrush);  // set the yellow brush
  theQPainter->drawRect(5, 5, theQPainter->window().width() - 10, 60);
  theQPainter->setBrush(Qt::NoBrush); // do not fill

  theQPainter->drawText(10, 20, "QPainter: "
      + QString::number(theQPainter->window().width()) + " x " + QString::number(theQPainter->window().height()));
  theQPainter->drawText(10, 32, tr("Raster Extent: ")
      + QString::number(theRasterViewPort->drawableAreaXDimInt)
      + "," + QString::number(theRasterViewPort->drawableAreaYDimInt));
  theQPainter->drawText(10, 44, tr("Clipped area: ")
      + QString::number(theRasterViewPort->clippedXMinDouble)
      + "," + QString::number(theRasterViewPort->clippedYMinDouble)
      + " - " + QString::number(theRasterViewPort->clippedXMaxDouble)
      + "," + QString::number(theRasterViewPort->clippedYMinDouble));

  return;


}                               //end of main draw method

/** Return the statistics for a given band name.
  WARDNING::: THERE IS NO GUARANTEE THAT BAND NAMES ARE UNIQE
  THE FIRST MATCH WILL BE RETURNED!!!!!!!!!!!!
  */
const QgsRasterBandStats QgsRasterLayer::getRasterBandStats(QString const & theBandNameQString)
{

  //we cant use a vector iterator because the iterator is astruct not a class
  //and the qvector model does not like this.
  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++)
  {
    QgsRasterBandStats myRasterBandStats = getRasterBandStats(i);
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      return myRasterBandStats;
    }
  }

  return QgsRasterBandStats();     // return a null one
  // XXX is this ok?  IS there a "null" one?
}

//get the number of a band given its name
//note this should be the rewritten name set up in the constructor,
//not the name retrieved directly from gdal!
//if no matching band is found zero will be returned!
const int QgsRasterLayer::getRasterBandNumber(QString const &  theBandNameQString)
{
  for (int myIteratorInt = 0; myIteratorInt <= rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    QgsRasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    QgsDebugMsg("myRasterBandStats.bandName: " + myRasterBandStats.bandName + "  :: theBandNameQString: "\
        + theBandNameQString);

    if (myRasterBandStats.bandName == theBandNameQString)
    {
      QgsDebugMsg("********** band " + QString::number(myRasterBandStats.bandNoInt) +\
          " was found in getRasterBandNumber " + theBandNameQString);

      return myRasterBandStats.bandNoInt;
    }
  }
  QgsDebugMsg("********** no band was found in getRasterBandNumber " + theBandNameQString);

  return 0;                     //no band was found
}



// get the name of a band given its number
const QString QgsRasterLayer::getRasterBandName(int theBandNoInt)
{

  if (theBandNoInt <= rasterStatsVector.size())
  {
    //vector starts at base 0, band counts at base1 !
    return rasterStatsVector[theBandNoInt - 1].bandName;
  }
  else
  {
    return QString("");
  }
}



/** Check whether a given band number has stats associated with it */
const bool QgsRasterLayer::hasStats(int theBandNoInt)
{
  if (theBandNoInt <= rasterStatsVector.size())
  {
    //vector starts at base 0, band counts at base1 !
    return rasterStatsVector[theBandNoInt - 1].statsGatheredFlag;
  }
  else
  {
    return false;
  }
}


/** Private method to calculate statistics for a band. Populates rasterStatsMemArray.

Calculates:

<ul>
<li>myRasterBandStats.elementCountInt
<li>myRasterBandStats.minValDouble
<li>myRasterBandStats.maxValDouble
<li>myRasterBandStats.sumDouble
<li>myRasterBandStats.rangeDouble
<li>myRasterBandStats.meanDouble
<li>myRasterBandStats.sumSqrDevDouble
<li>myRasterBandStats.stdDevDouble
<li>myRasterBandStats.colorTable
</ul>

@seealso RasterBandStats

@note

That this is a cpu intensive and slow task!

*/
const QgsRasterBandStats QgsRasterLayer::getRasterBandStats(int theBandNoInt)
{
  // check if we have received a valid band number
  if ((gdalDataset->GetRasterCount() < theBandNoInt) && rasterLayerType != PALETTE)
  {
    // invalid band id, return nothing
    QgsRasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }
  if (rasterLayerType == PALETTE && (theBandNoInt > 3))
  {
    // invalid band id, return nothing
    QgsRasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }
  // check if we have previously gathered stats for this band...

  QgsRasterBandStats myRasterBandStats = rasterStatsVector[theBandNoInt - 1];
  myRasterBandStats.bandNoInt = theBandNoInt;

  // don't bother with this if we already have stats
  if (myRasterBandStats.statsGatheredFlag)
  {
    return myRasterBandStats;
  }
  // only print message if we are actually gathering the stats
  emit setStatus(QString("Retrieving stats for ")+name());
  qApp->processEvents();
  QgsDebugMsg("QgsRasterLayer::retrieve stats for band " + QString::number(theBandNoInt));
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);


  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());

  //declare a colorTable to hold a palette - will only be used if the layer color interp is palette ???
  //get the palette colour table
  QgsColorTable *myColorTable = &(myRasterBandStats.colorTable);
  if (rasterLayerType == PALETTE)
  {

    //override the band name - palette images are really only one band
    //so we are faking three band behaviour
    switch (theBandNoInt)
    {
      // a "Red" layer
      case 1:
        myRasterBandStats.bandName = redBandNameQString;
        break;
      case 2:
        myRasterBandStats.bandName = blueBandNameQString;
        break;
      case 3:
        myRasterBandStats.bandName = greenBandNameQString;
        break;
      default:
        //invalid band id so return
        QgsRasterBandStats myNullReturnStats;
        return myNullReturnStats;
        break;
    }
  }
  else if (rasterLayerType==GRAY_OR_UNDEFINED)
  {
    myRasterBandStats.bandName = myColorInterpretation;
  }
  else //rasterLayerType is MULTIBAND
  {
    //do nothing
  }

  // XXX this sets the element count to a sensible value; but then you ADD to
  // XXX it later while iterating through all the pixels?
  //myRasterBandStats.elementCountInt = rasterXDimInt * rasterYDimInt;

  myRasterBandStats.elementCountInt = 0; // because we'll be counting only VALID pixels later

  emit setStatus(QString("Calculating stats for ")+name());
  //reset the main app progress bar
  emit drawingProgress(0,0);

  // let the user know we're going to possibly be taking a while
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  GDALDataType myDataType = myGdalBand->GetRasterDataType();

  int  myNXBlocks, myNYBlocks, myXBlockSize, myYBlockSize;
  myGdalBand->GetBlockSize( &myXBlockSize, &myYBlockSize );

  myNXBlocks = (myGdalBand->GetXSize() + myXBlockSize - 1) / myXBlockSize;
  myNYBlocks = (myGdalBand->GetYSize() + myYBlockSize - 1) / myYBlockSize;

  void *myData = CPLMalloc ( myXBlockSize * myYBlockSize * GDALGetDataTypeSize(myDataType)/8 );

  // unfortunately we need to make two passes through the data to calculate stddev
  bool myFirstIterationFlag = true;

  // Comparison value for equality; i.e., we shouldn't directly compare two
  // floats so it's better to take their difference and see if they're within
  // a certain range -- in this case twenty times the smallest value that
  // doubles can take for the current system.  (Yes, 20 was arbitrary.)
  double myPrecision = std::numeric_limits<double>::epsilon() * 20;
  int success;
  double GDALminimum = myGdalBand->GetMinimum( &success );

  if ( ! success )
  {
    QgsDebugMsg("myGdalBand->GetMinimum() failed");
  }

//ifdefs below to remove compiler warning about unused vars
#ifdef QGISDEBUG
  double GDALmaximum = myGdalBand->GetMaximum( &success );
#else
  myGdalBand->GetMaximum( &success );
#endif
  
  if ( ! success )
  {
    QgsDebugMsg("myGdalBand->GetMaximum() failed");
  }

//ifdefs below to remove compiler warning about unused vars
#ifdef QGISDEBUG
  double GDALnodata = myGdalBand->GetNoDataValue( &success );
#else
  myGdalBand->GetNoDataValue( &success );
#endif
  
  if ( ! success )
  {
    QgsDebugMsg("myGdalBand->GetNoDataValue() failed");
  }

#ifdef QGISDEBUG
  QgsLogger::debug("GDALminium: ", GDALminimum, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("GDALmaximum: ", GDALmaximum, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("GDALnodata: ", GDALnodata, __FILE__, __FUNCTION__, __LINE__);
#endif

  double GDALrange[2];          // calculated min/max, as opposed to the
  // dataset provided

  GDALComputeRasterMinMax( myGdalBand, 0, GDALrange );

#ifdef QGISDEBUG
  QgsLogger::debug("approximate computed GDALminium:", GDALrange[0], __FILE__, __FUNCTION__, __LINE__, 1);
  QgsLogger::debug("approximate computed GDALmaximum:", GDALrange[1], __FILE__, __FUNCTION__, __LINE__, 1);
#endif

  GDALComputeRasterMinMax( myGdalBand, 1, GDALrange );

#ifdef QGISDEBUG
  QgsLogger::debug("exactly computed GDALminium:", GDALrange[0]);
  QgsLogger::debug("exactly computed GDALmaximum:", GDALrange[1]);
#endif

  for( int iYBlock = 0; iYBlock < myNYBlocks; iYBlock++ )
  {
    emit drawingProgress ( iYBlock, myNYBlocks * 2 );

    for( int iXBlock = 0; iXBlock < myNXBlocks; iXBlock++ )
    {
      int  nXValid, nYValid;
      myGdalBand->ReadBlock( iXBlock, iYBlock, myData );

      // Compute the portion of the block that is valid
      // for partial edge blocks.
      if( (iXBlock+1) * myXBlockSize > myGdalBand->GetXSize() )
        nXValid = myGdalBand->GetXSize() - iXBlock * myXBlockSize;
      else
        nXValid = myXBlockSize;

      if( (iYBlock+1) * myYBlockSize > myGdalBand->GetYSize() )
        nYValid = myGdalBand->GetYSize() - iYBlock * myYBlockSize;
      else
        nYValid = myYBlockSize;

      // Collect the histogram counts.
      for( int iY = 0; iY < nYValid; iY++ )
      {
        for( int iX = 0; iX < nXValid; iX++ )
        {
          double myDouble = readValue ( myData, myDataType, iX + iY * myXBlockSize );

          if ( fabs(myDouble - noDataValueDouble) < myPrecision ||
              myDouble < GDALminimum || myDouble != myDouble)
          {
            continue; // NULL
          }

          //only use this element if we have a non null element
          if (myFirstIterationFlag)
          {
            //this is the first iteration so initialise vars
            myFirstIterationFlag = false;
            myRasterBandStats.minValDouble = myDouble;
            myRasterBandStats.maxValDouble = myDouble;
            ++myRasterBandStats.elementCountInt;
          }               //end of true part for first iteration check
          else
          {
            //this is done for all subsequent iterations
            if (myDouble < myRasterBandStats.minValDouble)
            {
              myRasterBandStats.minValDouble = myDouble;
            }
            if (myDouble > myRasterBandStats.maxValDouble)
            {
              myRasterBandStats.maxValDouble = myDouble;
            }
            //only increment the running total if it is not a nodata value
            if (myDouble != noDataValueDouble || myDouble != myDouble)
            {
              myRasterBandStats.sumDouble += myDouble;
              ++myRasterBandStats.elementCountInt;
            }
          }               //end of false part for first iteration check
        }
      }
    }                       //end of column wise loop
  }                           //end of row wise loop


  //end of first pass through data now calculate the range
  myRasterBandStats.rangeDouble = myRasterBandStats.maxValDouble - myRasterBandStats.minValDouble;
  //calculate the mean
  myRasterBandStats.meanDouble = myRasterBandStats.sumDouble / myRasterBandStats.elementCountInt;

  //for the second pass we will get the sum of the squares / mean
  for( int iYBlock = 0; iYBlock < myNYBlocks; iYBlock++ )
  {
    emit drawingProgress ( iYBlock+myNYBlocks, myNYBlocks * 2 );

    for( int iXBlock = 0; iXBlock < myNXBlocks; iXBlock++ )
    {
      int  nXValid, nYValid;

      myGdalBand->ReadBlock( iXBlock, iYBlock, myData );

      // Compute the portion of the block that is valid
      // for partial edge blocks.
      if( (iXBlock+1) * myXBlockSize > myGdalBand->GetXSize() )
        nXValid = myGdalBand->GetXSize() - iXBlock * myXBlockSize;
      else
        nXValid = myXBlockSize;

      if( (iYBlock+1) * myYBlockSize > myGdalBand->GetYSize() )
        nYValid = myGdalBand->GetYSize() - iYBlock * myYBlockSize;
      else
        nYValid = myYBlockSize;

      // Collect the histogram counts.
      for( int iY = 0; iY < nYValid; iY++ )
      {
        for( int iX = 0; iX < nXValid; iX++ )
        {
          double myDouble = readValue ( myData, myDataType, iX + iY * myXBlockSize );

          if ( fabs(myDouble - noDataValueDouble) < myPrecision ||
              myDouble < GDALminimum || myDouble != myDouble)
          {
            continue; // NULL
          }

          //get the nth element from the current row
          if (myColorInterpretation == "Palette") // dont translate this its a gdal string
          {
            //this is a palette layer so red / green / blue 'layers are 'virtual'
            //in that we need to obtain the palette entry and then get the r,g or g
            //component from that palette entry

            int c1, c2, c3;
            bool found = myColorTable->color ( myDouble, &c1, &c2, &c3 );
            if ( !found ) continue;

            //check for alternate color mappings
            switch (theBandNoInt)
            {
              case 1:
                myDouble = c1;
                break;
              case 2:
                myDouble = c2;
                break;
              case 3:
                myDouble = c3;
                break;
            }
          }

          myRasterBandStats.sumSqrDevDouble += static_cast < double >
            (pow(myDouble - myRasterBandStats.meanDouble, 2));
        }
      }
    }                       //end of column wise loop
  }                           //end of row wise loop

  //divide result by sample size - 1 and get square root to get stdev
  myRasterBandStats.stdDevDouble = static_cast < double >(sqrt(myRasterBandStats.sumSqrDevDouble /
        (myRasterBandStats.elementCountInt - 1)));

#ifdef QGISDEBUG
  QgsLogger::debug("************ STATS **************", 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("NULL", noDataValueDouble, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("MIN", myRasterBandStats.minValDouble, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("MAX", myRasterBandStats.maxValDouble, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("RANGE", myRasterBandStats.rangeDouble, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("MEAN", myRasterBandStats.meanDouble, 1, __FILE__, __FUNCTION__, __LINE__);
  QgsLogger::debug("STDDEV", myRasterBandStats.stdDevDouble, 1, __FILE__, __FUNCTION__, __LINE__);
#endif

  CPLFree(myData);
  myRasterBandStats.statsGatheredFlag = true;

  QgsDebugMsg("adding stats to stats collection at position " + QString::number(theBandNoInt - 1));
  //add this band to the class stats map
  rasterStatsVector[theBandNoInt - 1] = myRasterBandStats;
  emit drawingProgress(rasterYDimInt, rasterYDimInt); //reset progress
  QApplication::restoreOverrideCursor(); //restore the cursor
  QgsDebugMsg("Stats collection completed returning");
  return myRasterBandStats;

} // QgsRasterLayer::getRasterBandStats




//mutator for red band name (allows alternate mappings e.g. map blue as red colour)
void QgsRasterLayer::setRedBandName(QString const &  theBandNameQString)
{
  QgsDebugMsg("setRedBandName :  " + theBandNameQString);
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    redBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == "Red" || theBandNameQString == "Green" || theBandNameQString == "Blue"))
  {
    redBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    QgsRasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      redBandNameQString = theBandNameQString;
      return;
    }
  }

  //if no matches were found default to not set
  redBandNameQString = tr("Not Set");
  return;
}



//mutator for green band name
void QgsRasterLayer::setGreenBandName(QString const &  theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    greenBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == "Red" || theBandNameQString == "Green" || theBandNameQString == "Blue"))
  {
    greenBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    QgsRasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      greenBandNameQString = theBandNameQString;
      return;
    }
  }

  //if no matches were found default to not set
  greenBandNameQString = tr("Not Set");
  return;
}

//mutator for blue band name
void QgsRasterLayer::setBlueBandName(QString const &  theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    blueBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == "Red" || theBandNameQString == "Green" || theBandNameQString == "Blue"))
  {
    blueBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    QgsRasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      blueBandNameQString = theBandNameQString;
      return;
    }
  }

  //if no matches were found default to not set
  blueBandNameQString = tr("Not Set");
  return;
}

//mutator for transparent band name
void QgsRasterLayer::setTransparentBandName(QString const &  theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    transparentBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == "Red" || theBandNameQString == "Green" || theBandNameQString == "Blue"))
  {
    transparentBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    QgsRasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      transparentBandNameQString = theBandNameQString;
      return;
    }
  }

  //if no matches were found default to not set
  transparentBandNameQString = tr("Not Set");
  return;
}


//mutator for gray band name
void QgsRasterLayer::setGrayBandName(QString const &  theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    grayBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == redBandNameQString || theBandNameQString == greenBandNameQString || theBandNameQString == blueBandNameQString))
  {
    grayBandNameQString = theBandNameQString;
    return;
  }
  //otherwise check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    QgsRasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    QgsDebugMsg("Checking if " + myRasterBandStats.bandName + " == " 
        + grayBandNameQString);
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      grayBandNameQString = theBandNameQString;
      return;
    }
  }

  //if no matches were found default to not set
  grayBandNameQString = tr("Not Set");
  return;
}

/** Return a pixmap representing a legend image. This is an overloaded
 * version of the method below and assumes false for the legend name flag.
 */
QPixmap QgsRasterLayer::getLegendQPixmap()
{
  return getLegendQPixmap(false);
}

/** Return a pixmap representing a legend image
 * @param theWithNameFlag - boolena flag whether to overlay the legend name in the text
 */
QPixmap QgsRasterLayer::getLegendQPixmap(bool theWithNameFlag)
{
  QgsDebugMsg("QgsRasterLayer::getLegendQPixmap called (" + getDrawingStyleAsQString() + ")");

  QPixmap myLegendQPixmap;      //will be initialised once we know what drawing style is active
  QPainter myQPainter; 


  if (!mProviderKey.isEmpty())
  {
    QgsDebugMsg("QgsRasterLayer::getLegendQPixmap called with provider Key (" + mProviderKey + ")");
    myLegendQPixmap = QPixmap(3, 1);
    myQPainter.begin(&myLegendQPixmap);
    //draw legend red part
    myQPainter.setPen(QPen(QColor(255,   0,   0, QColor::Rgb), 0));
    myQPainter.drawPoint(0, 0);
    //draw legend green part
    myQPainter.setPen(QPen(QColor(  0, 255,   0, QColor::Rgb), 0));
    myQPainter.drawPoint(1, 0);
    //draw legend blue part
    myQPainter.setPen(QPen(QColor(  0,   0, 255, QColor::Rgb), 0));
    myQPainter.drawPoint(2, 0);

  }
  else
  { 
    // Legacy GDAL (non-provider)

    //
    // Get the adjusted matrix stats
    //
    GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(1);
    QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());



    //
    // Create the legend pixmap - note it is generated on the preadjusted stats
    //
    if (drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY)
    {

      myLegendQPixmap = QPixmap(100, 1);
      myQPainter.begin(&myLegendQPixmap);
      int myPosInt = 0;
      for (double myDouble = 0; myDouble < 255; myDouble += 2.55)
      {
        if (!invertHistogramFlag) //histogram is not inverted
        {
          //draw legend as grayscale
          int myGrayInt = static_cast < int >(myDouble);
          myQPainter.setPen(QPen(QColor(myGrayInt, myGrayInt, myGrayInt, QColor::Rgb), 0));
        } else                //histogram is inverted
        {
          //draw legend as inverted grayscale
          int myGrayInt = 255 - static_cast < int >(myDouble);
          myQPainter.setPen(QPen(QColor(myGrayInt, myGrayInt, myGrayInt, QColor::Rgb), 0));
        }                   //end of invert histogram  check
        myQPainter.drawPoint(myPosInt++, 0);
      }
    }                           //end of gray check
    else if (drawingStyle == MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR ||
        drawingStyle == PALETTED_SINGLE_BAND_PSEUDO_COLOR || drawingStyle == SINGLE_BAND_PSEUDO_COLOR)
    {

      //set up the three class breaks for pseudocolour mapping
      double myRangeSizeDouble = 90;  //hard coded for now
      double myBreakSizeDouble = myRangeSizeDouble / 3;
      double myClassBreakMin1 = 0;
      double myClassBreakMax1 = myClassBreakMin1 + myBreakSizeDouble;
      double myClassBreakMin2 = myClassBreakMax1;
      double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
      double myClassBreakMin3 = myClassBreakMax2;

      //
      // Create the legend pixmap - note it is generated on the preadjusted stats
      //
      myLegendQPixmap = QPixmap(100, 1);
      myQPainter.begin(&myLegendQPixmap);
      int myPosInt = 0;
      for (double myDouble = 0; myDouble < myRangeSizeDouble; myDouble += myRangeSizeDouble / 100.0)
      {
        //draw pseudocolor legend
        if (!invertHistogramFlag)
        {
          //check if we are in the first class break
          if ((myDouble >= myClassBreakMin1) && (myDouble < myClassBreakMax1))
          {
            int myRedInt = 0;
            int myBlueInt = 255;
            int myGreenInt = static_cast < int >(((255 / myRangeSizeDouble) * (myDouble - myClassBreakMin1)) * 3);
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
          //check if we are in the second class break
          else if ((myDouble >= myClassBreakMin2) && (myDouble < myClassBreakMax2))
          {
            int myRedInt = static_cast < int >(((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3);
            int myBlueInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3));
            int myGreenInt = 255;
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myGreenInt=myBlueInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRedInt = 255;
            int myBlueInt = 0;
            int myGreenInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin3) / 1) * 3)));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=myGreenInt;
              myGreenInt=255-myGreenInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
        }                   //end of invert histogram == false check
        else                  //invert histogram toggle is off
        {
          //check if we are in the first class break
          if ((myDouble >= myClassBreakMin1) && (myDouble < myClassBreakMax1))
          {
            int myRedInt = 255;
            int myBlueInt = 0;
            int myGreenInt = static_cast < int >(((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin1) / 1) * 3));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
          //check if we are in the second class break
          else if ((myDouble >= myClassBreakMin2) && (myDouble < myClassBreakMax2))
          {
            int myRedInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3));
            int myBlueInt = static_cast < int >(((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3);
            int myGreenInt = 255;
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myGreenInt=myBlueInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRedInt = 0;
            int myBlueInt = 255;
            int myGreenInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * (myDouble - myClassBreakMin3)) * 3));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }

        }                   //end of invert histogram check
        myQPainter.drawPoint(myPosInt++, 0);
      }

    }                           //end of pseudocolor check
    else if (drawingStyle == PALETTED_MULTI_BAND_COLOR || drawingStyle == MULTI_BAND_COLOR)
    {
      //
      // Create the legend pixmap showing red green and blue band mappings
      //
      // TODO update this so it actually shows the mappings for paletted images
      myLegendQPixmap = QPixmap(3, 1);
      myQPainter.begin(&myLegendQPixmap);
      //draw legend red part
      myQPainter.setPen(QPen(QColor(224, 103, 103, QColor::Rgb), 0));
      myQPainter.drawPoint(0, 0);
      //draw legend green part
      myQPainter.setPen(QPen(QColor(132, 224, 127, QColor::Rgb), 0));
      myQPainter.drawPoint(1, 0);
      //draw legend blue part
      myQPainter.setPen(QPen(QColor(127, 160, 224, QColor::Rgb), 0));
      myQPainter.drawPoint(2, 0);
    }
  }

  myQPainter.end();


  // see if the caller wants the name of the layer in the pixmap (used for legend bar)
  if (theWithNameFlag)
  {
    QFont myQFont("arial", 10, QFont::Normal);
    QFontMetrics myQFontMetrics(myQFont);

    int myHeightInt = (myQFontMetrics.height() + 10 > 35) ? myQFontMetrics.height() + 10 : 35;

    //create a matrix to
    QMatrix myQWMatrix;
    //scale the raster legend up a bit bigger to the legend item size
    //note that scaling parameters are factors, not absolute values,
    // so scale (0.25,1) scales the painter to a quarter of its size in the x direction
    //TODO We need to decide how much to scale by later especially for rgb images which are only 3x1 pix
    //hard coding thes values for now.
    if (myLegendQPixmap.width() == 3)
    {
      //scale width by factor of 50 (=150px wide)
      myQWMatrix.scale(60, myHeightInt);
    }
    else
    {
      //assume 100px so scale by factor of 1.5 (=150px wide)
      myQWMatrix.scale(1.8, myHeightInt);
    }
    //apply the matrix
    QPixmap myQPixmap2 = myLegendQPixmap.xForm(myQWMatrix);
    QPainter myQPainter(&myQPixmap2);

    //load  up the pyramid icons
    QString myThemePath = QgsApplication::themePath();
    QPixmap myPyramidPixmap(myThemePath + "/mIconPyramid.png");
    QPixmap myNoPyramidPixmap(myThemePath + "/mIconNoPyramid.png");

    //
    // Overlay a pyramid icon
    //
    if (hasPyramidsFlag)
    {
      myQPainter.drawPixmap(0,myHeightInt-myPyramidPixmap.height(),myPyramidPixmap);
    }
    else
    {
      myQPainter.drawPixmap(0,myHeightInt-myNoPyramidPixmap.height(),myNoPyramidPixmap);
    }
    //
    // Overlay the layername
    //
    if (drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY)
    {
      myQPainter.setPen(Qt::white);
    }
    else
    {
      myQPainter.setPen(Qt::black);
    }
    myQPainter.setFont(myQFont);
    myQPainter.drawText(25, myHeightInt - 10, this->name());
    //
    // finish up
    //
    myLegendQPixmap = myQPixmap2;
    myQPainter.end();
  }
  //finish up

  return myLegendQPixmap;

}                               //end of getLegendQPixmap function


QPixmap QgsRasterLayer::getDetailedLegendQPixmap(int theLabelCountInt=3)
{
  QgsDebugMsg("QgsRasterLayer::getDetailedLegendQPixmap called");
  QFont myQFont("arial", 10, QFont::Normal);
  QFontMetrics myQFontMetrics(myQFont);

  int myFontHeight = (myQFontMetrics.height() );
  const int myInterLabelSpacing = 5;
  int myImageHeightInt = ((myFontHeight + (myInterLabelSpacing*2)) * theLabelCountInt);
  //these next two vars are not used anywhere so commented out for now
  //int myLongestLabelWidthInt =  myQFontMetrics.width(this->name());
  //const int myHorizontalLabelSpacing = 5;
  const int myColourBarWidthInt = 10;
  //
  // Get the adjusted matrix stats
  //
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(1);
  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
  QPixmap myLegendQPixmap;      //will be initialised once we know what drawing style is active
  QPainter myQPainter;
  //
  // Create the legend pixmap - note it is generated on the preadjusted stats
  //
  if (drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY)
  {

    myLegendQPixmap = QPixmap(1, myImageHeightInt);
    const double myIncrementDouble = static_cast<double>(myImageHeightInt)/255.0;
    myQPainter.begin(&myLegendQPixmap);
    int myPosInt = 0;
    for (double myDouble = 0; myDouble < 255; myDouble += myIncrementDouble)
    {
      if (!invertHistogramFlag) //histogram is not inverted
      {
        //draw legend as grayscale
        int myGrayInt = static_cast < int >(myDouble);
        myQPainter.setPen(QPen(QColor(myGrayInt, myGrayInt, myGrayInt, QColor::Rgb), 0));
      } else                //histogram is inverted
      {
        //draw legend as inverted grayscale
        int myGrayInt = 255 - static_cast < int >(myDouble);
        myQPainter.setPen(QPen(QColor(myGrayInt, myGrayInt, myGrayInt, QColor::Rgb), 0));
      }                   //end of invert histogram  check
      myQPainter.drawPoint(0,myPosInt++);
    }
  }                           //end of gray check
  else if (drawingStyle == MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR ||
      drawingStyle == PALETTED_SINGLE_BAND_PSEUDO_COLOR || drawingStyle == SINGLE_BAND_PSEUDO_COLOR)
  {

    //set up the three class breaks for pseudocolour mapping
    double myRangeSizeDouble = 90;  //hard coded for now
    double myBreakSizeDouble = myRangeSizeDouble / 3;
    double myClassBreakMin1 = 0;
    double myClassBreakMax1 = myClassBreakMin1 + myBreakSizeDouble;
    double myClassBreakMin2 = myClassBreakMax1;
    double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
    double myClassBreakMin3 = myClassBreakMax2;

    //
    // Create the legend pixmap - note it is generated on the preadjusted stats
    //
    myLegendQPixmap = QPixmap(1, myImageHeightInt);
    const double myIncrementDouble = myImageHeightInt/myRangeSizeDouble;
    myQPainter.begin(&myLegendQPixmap);
    int myPosInt = 0;
    for (double myDouble = 0; myDouble < 255; myDouble += myIncrementDouble)
      for (double myDouble = 0; myDouble < myRangeSizeDouble; myDouble +=myIncrementDouble)
      {
        //draw pseudocolor legend
        if (!invertHistogramFlag)
        {
          //check if we are in the first class break
          if ((myDouble >= myClassBreakMin1) && (myDouble < myClassBreakMax1))
          {
            int myRedInt = 0;
            int myBlueInt = 255;
            int myGreenInt = static_cast < int >(((255 / myRangeSizeDouble) * (myDouble - myClassBreakMin1)) * 3);
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
          //check if we are in the second class break
          else if ((myDouble >= myClassBreakMin2) && (myDouble < myClassBreakMax2))
          {
            int myRedInt = static_cast < int >(((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3);
            int myBlueInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3));
            int myGreenInt = 255;
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myGreenInt=myBlueInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRedInt = 255;
            int myBlueInt = 0;
            int myGreenInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin3) / 1) * 3)));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=myGreenInt;
              myGreenInt=255-myGreenInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
        }                   //end of invert histogram == false check
        else                  //invert histogram toggle is off
        {
          //check if we are in the first class break
          if ((myDouble >= myClassBreakMin1) && (myDouble < myClassBreakMax1))
          {
            int myRedInt = 255;
            int myBlueInt = 0;
            int myGreenInt = static_cast < int >(((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin1) / 1) * 3));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
          //check if we are in the second class break
          else if ((myDouble >= myClassBreakMin2) && (myDouble < myClassBreakMax2))
          {
            int myRedInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3));
            int myBlueInt = static_cast < int >(((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3);
            int myGreenInt = 255;
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myGreenInt=myBlueInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRedInt = 0;
            int myBlueInt = 255;
            int myGreenInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * (myDouble - myClassBreakMin3)) * 3));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
            myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
          }

        }                   //end of invert histogram check
        myQPainter.drawPoint(0,myPosInt++);
      }

  }                           //end of pseudocolor check
  else if (drawingStyle == PALETTED_MULTI_BAND_COLOR || drawingStyle == MULTI_BAND_COLOR)
  {
    //
    // Create the legend pixmap showing red green and blue band mappings
    //
    // TODO update this so it actually shows the mappings for paletted images
    myLegendQPixmap = QPixmap(1, 3);
    myQPainter.begin(&myLegendQPixmap);
    //draw legend red part
    myQPainter.setPen(QPen(QColor(224, 103, 103, QColor::Rgb), 0));
    myQPainter.drawPoint(0, 0);
    //draw legend green part
    myQPainter.setPen(QPen(QColor(132, 224, 127, QColor::Rgb), 0));
    myQPainter.drawPoint(0,1);
    //draw legend blue part
    myQPainter.setPen(QPen(QColor(127, 160, 224, QColor::Rgb), 0));
    myQPainter.drawPoint(0,2);
  }


  myQPainter.end();



  //create a matrix to
  QMatrix myQWMatrix;
  //scale the raster legend up a bit bigger to the legend item size
  //note that scaling parameters are factors, not absolute values,
  // so scale (0.25,1) scales the painter to a quarter of its size in the x direction
  //TODO We need to decide how much to scale by later especially for rgb images which are only 3x1 pix
  //hard coding thes values for now.
  if (myLegendQPixmap.height() == 3)
  {
    myQWMatrix.scale(myColourBarWidthInt,2);
  }
  else
  {
    myQWMatrix.scale(myColourBarWidthInt,2);
  }
  //apply the matrix
  QPixmap myQPixmap2 = myLegendQPixmap.xForm(myQWMatrix);
  QPainter myQPainter2(&myQPixmap2);
  //
  // Overlay the layername
  //
  if (drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY)
  {
    myQPainter2.setPen(Qt::white);
  }
  else
  {
    myQPainter2.setPen(Qt::black);
  }
  myQPainter2.setFont(myQFont);
  myQPainter2.drawText(25, myImageHeightInt - 10, this->name());
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

  if (dataProvider)
  {
    return dataProvider->subLayers();
  }
  else
  {
    return QStringList();   // Empty
  }

}


// Useful for Provider mode

void QgsRasterLayer::setLayerOrder(QStringList const & layers)
{
  QgsDebugMsg("QgsRasterLayer::setLayerOrder: Entered.");

  if (dataProvider)
  {
    QgsDebugMsg("QgsRasterLayer::setLayerOrder: About to dataProvider->setLayerOrder(layers).");
    dataProvider->setLayerOrder(layers);
  }

}

// Useful for Provider mode

void QgsRasterLayer::setSubLayerVisibility(QString const &  name, bool vis)
{

  if (dataProvider)
  {
    QgsDebugMsg("QgsRasterLayer::setSubLayerVisibility: About to dataProvider->setSubLayerVisibility(name, vis).");
    dataProvider->setSubLayerVisibility(name, vis);
  }

}


void QgsRasterLayer::updateProgress(int theProgress, int theMax)
{
  //simply propogate it on!
  emit drawingProgress (theProgress,theMax);
}



// convenience function for building getMetadata() HTML table cells
static
  QString
makeTableCell_( QString const & value )
{
  return "<td>" + value + "</td>";
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
  QString myMetadataQString ;
  myMetadataQString += "<table class=\"wide\">";
  myMetadataQString += "<tr><td class=\"glossy\">";
  myMetadataQString += tr("Driver:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td>";
  if (mProviderKey.isEmpty())
  {
    myMetadataQString += QString(gdalDataset->GetDriver()->GetDescription());
    myMetadataQString += "<br>";
    myMetadataQString += QString(gdalDataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME));
  }
  else
  {
    myMetadataQString += dataProvider->description();
  }
  myMetadataQString += "</td></tr>";

  if (!mProviderKey.isEmpty())
  {
    // Insert provider-specific (e.g. WMS-specific) metadata
    myMetadataQString += dataProvider->getMetadata();
  }
  else
  {

    // my added code (MColetti)

    myMetadataQString += "<tr><td class=\"glossy\">";
    myMetadataQString += tr("Dataset Description");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td>";
    myMetadataQString += QFile::decodeName(gdalDataset->GetDescription());
    myMetadataQString += "</td></tr>";


    char ** GDALmetadata = gdalDataset->GetMetadata();

    if ( GDALmetadata )
    {
      QStringList metadata = cStringList2Q_( GDALmetadata );
      myMetadataQString += makeTableCells_( metadata ); 
    }
    else
    {
      QgsDebugMsg("dataset has no metadata");
    }

    for ( int i = 1; i <= gdalDataset->GetRasterCount(); ++i )
    {
      gdalDataset->GetRasterBand(i)->GetMetadata();

      if ( GDALmetadata )
      {
        QStringList metadata = cStringList2Q_( GDALmetadata );
        myMetadataQString += makeTableCells_( metadata ); 
      }
      else
      {
        QgsDebugMsg("band " + QString::number(i) + "has no metadata"); 
      }

      char ** GDALcategories = gdalDataset->GetRasterBand(i)->GetCategoryNames();

      if ( GDALcategories )
      {
        QStringList categories = cStringList2Q_( GDALcategories );
        myMetadataQString += makeTableCells_( categories ); 
      }
      else
      {
        QgsDebugMsg("band " + QString::number(i) + " has no categories");
      }

    }

    // end my added code

    myMetadataQString += "<tr><td class=\"glossy\">";
    myMetadataQString += tr("Dimensions:");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td>";
    myMetadataQString += tr("X: ") + QString::number(gdalDataset->GetRasterXSize()) +
      tr(" Y: ") + QString::number(gdalDataset->GetRasterYSize()) + tr(" Bands: ") + QString::number(gdalDataset->GetRasterCount());
    myMetadataQString += "</td></tr>";

    //just use the first band
    GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(1);

    myMetadataQString += "<tr><td class=\"glossy\">";
    myMetadataQString += tr("No Data Value");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td>";
    myMetadataQString += QString::number(noDataValueDouble);
    myMetadataQString += "</td></tr>";

    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td class=\"glossy\">";
    myMetadataQString += tr("Data Type:");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td>";
    switch (myGdalBand->GetRasterDataType())
    {
      case GDT_Byte:
        myMetadataQString += tr("GDT_Byte - Eight bit unsigned integer");
        break;
      case GDT_UInt16:
        myMetadataQString += tr("GDT_UInt16 - Sixteen bit unsigned integer ");
        break;
      case GDT_Int16:
        myMetadataQString += tr("GDT_Int16 - Sixteen bit signed integer ");
        break;
      case GDT_UInt32:
        myMetadataQString += tr("GDT_UInt32 - Thirty two bit unsigned integer ");
        break;
      case GDT_Int32:
        myMetadataQString += tr("GDT_Int32 - Thirty two bit signed integer ");
        break;
      case GDT_Float32:
        myMetadataQString += tr("GDT_Float32 - Thirty two bit floating point ");
        break;
      case GDT_Float64:
        myMetadataQString += tr("GDT_Float64 - Sixty four bit floating point ");
        break;
      case GDT_CInt16:
        myMetadataQString += tr("GDT_CInt16 - Complex Int16 ");
        break;
      case GDT_CInt32:
        myMetadataQString += tr("GDT_CInt32 - Complex Int32 ");
        break;
      case GDT_CFloat32:
        myMetadataQString += tr("GDT_CFloat32 - Complex Float32 ");
        break;
      case GDT_CFloat64:
        myMetadataQString += tr("GDT_CFloat64 - Complex Float64 ");
        break;
      default:
        myMetadataQString += tr("Could not determine raster data type.");
    }
    myMetadataQString += "</td></tr>";

    myMetadataQString += "<tr><td class=\"glossy\">";
    myMetadataQString += tr("Pyramid overviews:");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td>";

    if( GDALGetOverviewCount(myGdalBand) > 0 )
    {
      int myOverviewInt;
      for( myOverviewInt = 0;
          myOverviewInt < GDALGetOverviewCount(myGdalBand);
          myOverviewInt++ )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGdalBand, myOverviewInt );
        myMetadataQString += "<p>X : " + QString::number(GDALGetRasterBandXSize( myOverview ));
        myMetadataQString += ",Y " + QString::number(GDALGetRasterBandYSize( myOverview ) ) + "</p>";
      }
    }
    myMetadataQString += "</td></tr>";
  }  // if (mProviderKey.isEmpty())

  myMetadataQString += "<tr><td class=\"glossy\">";
  myMetadataQString += tr("Layer Spatial Reference System: ");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td>";
  myMetadataQString += mSRS->proj4String();
  myMetadataQString += "</td></tr>";

  // output coordinate system
  // TODO: this is not related to layer, to be removed? [MD]
  /*  
      myMetadataQString += "<tr><td class=\"glossy\">";
      myMetadataQString += tr("Project Spatial Reference System: ");
      myMetadataQString += "</td></tr>";
      myMetadataQString += "<tr><td>";
      myMetadataQString +=  mCoordinateTransform->destSRS().proj4String();
      myMetadataQString += "</td></tr>";
      */

  if (mProviderKey.isEmpty())
  {
    if (gdalDataset->GetGeoTransform(adfGeoTransform) != CE_None)
    {
      // if the raster does not have a valid transform we need to use
      // a pixel size of (1,-1), but GDAL returns (1,1)
      adfGeoTransform[5] = -1;
    }
    else
    {
      myMetadataQString += "<tr><td class=\"glossy\">";
      myMetadataQString += tr("Origin:");
      myMetadataQString += "</td></tr>";
      myMetadataQString += "<tr><td>";
      myMetadataQString += QString::number(adfGeoTransform[0]);
      myMetadataQString += ",";
      myMetadataQString += QString::number(adfGeoTransform[3]);
      myMetadataQString += "</td></tr>";

      myMetadataQString += "<tr><td class=\"glossy\">";
      myMetadataQString += tr("Pixel Size:");
      myMetadataQString += "</td></tr>";
      myMetadataQString += "<tr><td>";
      myMetadataQString += QString::number(adfGeoTransform[1]);
      myMetadataQString += ",";
      myMetadataQString += QString::number(adfGeoTransform[5]);
      myMetadataQString += "</td></tr>";
    }

    //
    // Add the stats for each band to the output table
    //
    myMetadataQString += "<tr><td>";

    // Start a nested table in this trow
    myMetadataQString += "<table class=\"wide\">";
    myMetadataQString += "<tr><td class=\"glossy\">";
    myMetadataQString += tr("Property") ;
    myMetadataQString += "</td>";
    myMetadataQString += "<td class=\"glossy\">";
    myMetadataQString += tr("Value");
    myMetadataQString += "</th><tr>";

    int myBandCountInt = getBandCount();
    for (int myIteratorInt = 1; myIteratorInt <= myBandCountInt; ++myIteratorInt)
    {
      QgsDebugMsg("Raster properties : checking if band " + QString::number(myIteratorInt) + " has stats? ");
      //band name
      myMetadataQString += "<tr><td class=\"cellHeader\">";
      myMetadataQString += tr("Band");
      myMetadataQString += "</td>";
      myMetadataQString += "<td class=\"cellHeader\">";
      myMetadataQString += getRasterBandName(myIteratorInt);
      myMetadataQString += "</td></tr>";
      //band number
      myMetadataQString += "<tr><td>";
      myMetadataQString += tr("Band No");
      myMetadataQString += "</td>";
      myMetadataQString += "<td>";
      myMetadataQString += QString::number(myIteratorInt);
      myMetadataQString += "</td></tr>";

      //check if full stats for this layer have already been collected
      if (!hasStats(myIteratorInt))  //not collected
      {
        QgsDebugMsg(".....no");

        myMetadataQString += "<tr><td>";
        myMetadataQString += tr("No Stats");
        myMetadataQString += "</td>";
        myMetadataQString += "<td>";
        myMetadataQString += tr("No stats collected yet");
        myMetadataQString += "</td></tr>";
      }
      else                    // collected - show full detail
      {
        QgsDebugMsg(".....yes");

        QgsRasterBandStats myRasterBandStats = getRasterBandStats(myIteratorInt);
        //Min Val
        myMetadataQString += "<tr><td>";
        myMetadataQString += tr("Min Val");
        myMetadataQString += "</td>";
        myMetadataQString += "<td>";
        myMetadataQString += QString::number(myRasterBandStats.minValDouble, 'f',10);
        myMetadataQString += "</td></tr>";

        // Max Val
        myMetadataQString += "<tr><td>";
        myMetadataQString += tr("Max Val");
        myMetadataQString += "</td>";
        myMetadataQString += "<td>";
        myMetadataQString += QString::number(myRasterBandStats.maxValDouble, 'f',10);
        myMetadataQString += "</td></tr>";

        // Range
        myMetadataQString += "<tr><td>";
        myMetadataQString += tr("Range");
        myMetadataQString += "</td>";
        myMetadataQString += "<td>";
        myMetadataQString += QString::number(myRasterBandStats.rangeDouble, 'f',10);
        myMetadataQString += "</td></tr>";

        // Mean
        myMetadataQString += "<tr><td>";
        myMetadataQString += tr("Mean");
        myMetadataQString += "</td>";
        myMetadataQString += "<td>";
        myMetadataQString += QString::number(myRasterBandStats.meanDouble, 'f',10);
        myMetadataQString += "</td></tr>";

        //sum of squares
        myMetadataQString += "<tr><td>";
        myMetadataQString += tr("Sum of squares");
        myMetadataQString += "</td>";
        myMetadataQString += "<td>";
        myMetadataQString += QString::number(myRasterBandStats.sumSqrDevDouble, 'f',10);
        myMetadataQString += "</td></tr>";

        //standard deviation
        myMetadataQString += "<tr><td>";
        myMetadataQString += tr("Standard Deviation");
        myMetadataQString += "</td>";
        myMetadataQString += "<td>";
        myMetadataQString += QString::number(myRasterBandStats.stdDevDouble, 'f',10);
        myMetadataQString += "</td></tr>";

        //sum of all cells
        myMetadataQString += "<tr><td>";
        myMetadataQString += tr("Sum of all cells");
        myMetadataQString += "</td>";
        myMetadataQString += "<td>";
        myMetadataQString += QString::number(myRasterBandStats.sumDouble, 'f',10);
        myMetadataQString += "</td></tr>";

        //number of cells
        myMetadataQString += "<tr><td>";
        myMetadataQString += tr("Cell Count");
        myMetadataQString += "</td>";
        myMetadataQString += "<td>";
        myMetadataQString += QString::number(myRasterBandStats.elementCountInt);
        myMetadataQString += "</td></tr>";
      }
    }
    myMetadataQString += "</table>"; //end of nested table
    myMetadataQString += "</td></tr>"; //end of stats container table row
  } // if (mProviderKey.isEmpty())

  //
  // Close the table
  //

  myMetadataQString += "</table>";
  return myMetadataQString;
}

QString QgsRasterLayer::buildPyramids(RasterPyramidList const & theRasterPyramidList, 
    QString const & theResamplingMethod)
{
  emit drawingProgress(0,0);
  //first test if the file is writeable
  QFileInfo myQFile(mDataSource);

  if (!myQFile.isWritable())
  {
    return "ERROR_WRITE_ACCESS";
  }

  GDALAllRegister();
  //close the gdal dataset and reopen it in read / write mode
  delete gdalDataset;
  gdalDataset = (GDALDataset *) GDALOpen(QFile::encodeName(mDataSource).constData(), GA_Update);

  // if the dataset couldn't be opened in read / write mode, tell the user
  if (!gdalDataset)
  {
    emit drawingProgress(0,0);
    gdalDataset = (GDALDataset *) GDALOpen(QFile::encodeName(mDataSource).constData(), GA_ReadOnly);
    return "ERROR_WRITE_FORMAT";
  }

  //
  // Iterate through the Raster Layer Pyramid Vector, building any pyramid
  // marked as exists in eaxh RasterPyramid struct.
  //
  CPLErr myError; //in case anything fails
  int myCountInt=1;
  int myTotalInt=theRasterPyramidList.count();
  RasterPyramidList::const_iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator = theRasterPyramidList.begin();
      myRasterPyramidIterator != theRasterPyramidList.end();
      ++myRasterPyramidIterator )
  {
#ifdef QGISDEBUG
    QgsLogger::debug("Build pyramids:: Level", (*myRasterPyramidIterator).levelInt, 1, __FILE__, __FUNCTION__, __LINE__);
    QgsLogger::debug("x", (*myRasterPyramidIterator).xDimInt, 1, __FILE__, __FUNCTION__, __LINE__);
    QgsLogger::debug("y", (*myRasterPyramidIterator).yDimInt, 1, __FILE__, __FUNCTION__, __LINE__);
    QgsLogger::debug("exists :", (*myRasterPyramidIterator).existsFlag,  1, __FILE__, __FUNCTION__, __LINE__);
#endif
    if ((*myRasterPyramidIterator).existsFlag)
    {
      QgsDebugMsg("Building.....");
      emit drawingProgress(myCountInt,myTotalInt);
      int myOverviewLevelsIntArray[1] = {(*myRasterPyramidIterator).levelInt };
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
        if(theResamplingMethod==tr("Average Magphase"))
        {
          myError = gdalDataset->BuildOverviews( "MODE", 1, myOverviewLevelsIntArray, 0, NULL,
              GDALDummyProgress, NULL );
        }
        else if(theResamplingMethod==tr("Average"))

        {
          myError = gdalDataset->BuildOverviews( "AVERAGE", 1, myOverviewLevelsIntArray, 0, NULL,
              GDALDummyProgress, NULL );
        }
        else // fall back to nearest neighbor
        {
          myError = gdalDataset->BuildOverviews( "NEAREST", 1, myOverviewLevelsIntArray, 0, NULL,
              GDALDummyProgress, NULL );
        }
        if (myError == CE_Failure || CPLGetLastErrorNo()==CPLE_NotSupported  )
        {
          //something bad happenend
          //QString myString = QString (CPLGetLastError());
          delete gdalDataset;
          gdalDataset = (GDALDataset *) GDALOpen(QFile::encodeName(mDataSource).constData(), GA_ReadOnly);
          emit drawingProgress(0,0);
          return "FAILED_NOT_SUPPORTED";
        }
        myCountInt++;
        //make sure the raster knows it has pyramids
        hasPyramidsFlag=true;
      }
      catch (CPLErr)
      {
        QgsLogger::warning("Pyramid overview building failed!");
      }
    }
  }
  QgsDebugMsg("Pyramid overviews built");
  //close the gdal dataset and reopen it in read only mode
  delete gdalDataset;
  gdalDataset = (GDALDataset *) GDALOpen(QFile::encodeName(mDataSource).constData(), GA_ReadOnly);
  emit drawingProgress(0,0);
  return NULL; // returning null on success
}

QgsRasterLayer::RasterPyramidList  QgsRasterLayer::buildRasterPyramidList()
{
  //
  // First we build up a list of potential pyramid layers
  //
  int myWidth=rasterXDimInt;
  int myHeight=rasterYDimInt;
  int myDivisorInt=2;
  GDALRasterBandH myGDALBand = GDALGetRasterBand( gdalDataset, 1 ); //just use the first band

  mPyramidList.clear();
  QgsDebugMsg("Building initial pyramid list");
  while((myWidth/myDivisorInt > 32) && ((myHeight/myDivisorInt)>32))
  {

    QgsRasterPyramid myRasterPyramid;
    myRasterPyramid.levelInt=myDivisorInt;
    myRasterPyramid.xDimInt = (int)(0.5 + (myWidth/(double)myDivisorInt));
    myRasterPyramid.yDimInt = (int)(0.5 + (myHeight/(double)myDivisorInt));
    myRasterPyramid.existsFlag=false;
#ifdef QGISDEBUG
    QgsLogger::debug("Pyramid", myRasterPyramid.levelInt, 1, __FILE__, __FUNCTION__, __LINE__);
    QgsLogger::debug("xDimInt", myRasterPyramid.xDimInt, 1, __FILE__, __FUNCTION__, __LINE__);
    QgsLogger::debug("yDimInt", myRasterPyramid.yDimInt, 1, __FILE__, __FUNCTION__, __LINE__);
#endif




    //
    // Now we check if it actually exists in the raster layer
    // and also adjust the dimensions if the dimensions calculated
    // above are only a near match.
    //
    const int myNearMatchLimitInt=5;
    if( GDALGetOverviewCount(myGDALBand) > 0 )
    {
      int myOverviewInt;
      for( myOverviewInt = 0;
          myOverviewInt < GDALGetOverviewCount(myGDALBand);
          myOverviewInt++ )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGDALBand, myOverviewInt );
        int myOverviewXDim = GDALGetRasterBandXSize( myOverview );
        int myOverviewYDim = GDALGetRasterBandYSize( myOverview );
        //
        // here is where we check if its a near match:
        // we will see if its within 5 cells either side of
        //
        QgsDebugMsg("Checking whether " + QString::number(myRasterPyramid.xDimInt) + " x " +\
            QString::number(myRasterPyramid.yDimInt) + " matches "+\
            QString::number(myOverviewXDim) + " x " + QString::number(myOverviewYDim));


        if ((myOverviewXDim <= (myRasterPyramid.xDimInt+ myNearMatchLimitInt)) &&
            (myOverviewXDim >= (myRasterPyramid.xDimInt- myNearMatchLimitInt)) &&
            (myOverviewYDim <= (myRasterPyramid.yDimInt+ myNearMatchLimitInt)) &&
            (myOverviewYDim >= (myRasterPyramid.yDimInt- myNearMatchLimitInt)))
        {
          //right we have a match so adjust the a / y before they get added to the list
          myRasterPyramid.xDimInt=myOverviewXDim;
          myRasterPyramid.yDimInt=myOverviewYDim;
          myRasterPyramid.existsFlag=true;
          QgsDebugMsg(".....YES!");
        }
        else
        {
          //no match
          QgsDebugMsg(".....no.");
        }
      }
    }
    mPyramidList.append(myRasterPyramid);
    //sqare the divisor each step
    myDivisorInt=(myDivisorInt *2);
  }

  return mPyramidList;
}


bool QgsRasterLayer::isEditable() const
{
  return false;
}




void QgsRasterLayer::readColorTable ( GDALRasterBand *gdalBand, QgsColorTable *theColorTable )
{
  QgsDebugMsg("QgsRasterLayer::readColorTable()");

  // First try to read color table from metadata
  char **metadata = gdalBand->GetMetadata();
  theColorTable->clear();
  bool found = false;
  while ( metadata && metadata[0] )
  {
    QStringList metadataTokens = QStringList::split("=", *metadata );

    if (metadataTokens.count() < 2 ) continue;

    if ( metadataTokens[0].contains("COLOR_TABLE_RULE_RGB_") )
    {
      double min, max;
      int min_c1, min_c2, min_c3, max_c1, max_c2, max_c3;

      if ( sscanf(metadataTokens[1].toLocal8Bit().data(), "%lf %lf %d %d %d %d %d %d",
            &min, &max, &min_c1, &min_c2, &min_c3, &max_c1, &max_c2, &max_c3) != 8 )
      {
        continue;
      }
      theColorTable->add ( min, max,
          (unsigned char)min_c1, (unsigned char)min_c2, (unsigned char)min_c3, 0,
          (unsigned char)max_c1, (unsigned char)max_c2, (unsigned char)max_c3, 0 );
      found = true;
    }
    ++metadata;
  }
  theColorTable->sort();

  // If no color table was found, try to read it from GDALColorTable
  if ( !found )
  {
    GDALColorTable *gdalColorTable = gdalBand->GetColorTable();

    if ( gdalColorTable )
    {
      int count = gdalColorTable->GetColorEntryCount();

      for ( int i = 0; i < count; i++ )
      {
        const GDALColorEntry *colorEntry = gdalColorTable->GetColorEntry ( i );

        if ( !colorEntry ) continue;

        theColorTable->add ( i, (unsigned char) colorEntry->c1, (unsigned char) colorEntry->c2,
            (unsigned char) colorEntry->c3 );
      }
    }
  }

#ifdef QGISDEBUG
  theColorTable->print();
#endif
}


QgsColorTable *QgsRasterLayer::colorTable ( int theBandNoInt )
{
  return &(rasterStatsVector[theBandNoInt-1].colorTable);
}


void *QgsRasterLayer::readData ( GDALRasterBand *gdalBand, QgsRasterViewPort *viewPort )
{
  GDALDataType type = gdalBand->GetRasterDataType();
  int size = GDALGetDataTypeSize ( type ) / 8;

  QgsDebugMsg("QgsRasterLayer::readData: calling RasterIO with " +\
      QString(", source NW corner: ") + QString::number(viewPort->rectXOffsetInt)+\
      ", " + QString::number(viewPort->rectYOffsetInt)+\
      ", source size: " + QString::number(viewPort->clippedWidthInt)+\
      ", " + QString::number(viewPort->clippedHeightInt)+\
      ", dest size: " + QString::number(viewPort->drawableAreaXDimInt)+\
      ", " + QString::number(viewPort->drawableAreaYDimInt));

  void *data = VSIMalloc ( size * viewPort->drawableAreaXDimInt * viewPort->drawableAreaYDimInt );
  
  /* Abort if out of memory */
  if (data == NULL) 
  {
    QgsDebugMsg("Layer " + this->name() + " couldn't allocate enough memory. Ignoring");
  }
  else
  {
    CPLErr myErr = gdalBand->RasterIO ( GF_Read,
                                        viewPort->rectXOffsetInt,
                                        viewPort->rectYOffsetInt,
                                        viewPort->clippedWidthInt,
                                        viewPort->clippedHeightInt,
                                        data,
                                        viewPort->drawableAreaXDimInt,
                                        viewPort->drawableAreaYDimInt,
                                        type, 0, 0 );
    if (myErr != CPLE_None)
    {
      QgsLogger::warning("RaterIO error: " + QString(CPLGetLastErrorMsg()));
    }
  }
  return data;
}


double QgsRasterLayer::readValue ( void *data, GDALDataType type, int index )
{
  double val;

  switch ( type )
  {
    case GDT_Byte:
      return (double) ((GByte *)data)[index];
      break;
    case GDT_UInt16:
      return (double) ((GUInt16 *)data)[index];
      break;
    case GDT_Int16:
      return (double) ((GInt16 *)data)[index];
      break;
    case GDT_UInt32:
      return (double) ((GUInt32 *)data)[index];
      break;
    case GDT_Int32:
      return (double) ((GInt32 *)data)[index];
      break;
    case GDT_Float32:
      return (double) ((float *)data)[index];
      break;
    case GDT_Float64:
      val = ((double *)data)[index];
      return (double) ((double *)data)[index];
      break;
    default:
      QgsLogger::warning("GDAL data type is not supported");
  }
  return 0.0;
}


/**

  Raster layer project file XML of form:

  <maplayer type="raster" visible="1" showInOverviewFlag="1">
  <layername>Wynoochee_dem</layername>
  <datasource>/home/mcoletti/mnt/MCOLETTIF8F9/c/Toolkit_Course/Answers/Training_Data/wynoochee_dem.img</datasource>
  <zorder>0</zorder>
  <rasterproperties>
  <showDebugOverlayFlag boolean="false"/>
  <drawingStyle>SINGLE_BAND_GRAY</drawingStyle>
  <invertHistogramFlag boolean="false"/>
  <stdDevsToPlotDouble>0</stdDevsToPlotDouble>
  <transparencyLevelInt>255</transparencyLevelInt>
  <redBandNameQString>Not Set</redBandNameQString>
  <greenBandNameQString>Not Set</greenBandNameQString>
  <blueBandNameQString>Not Set</blueBandNameQString>
  <grayBandNameQString>Undefined</grayBandNameQString>
  </rasterproperties>
  </maplayer>
  */
bool QgsRasterLayer::readXML_( QDomNode & layer_node )
{
  //! @NOTE Make sure to read the file first so stats etc are initialised properly!

  //process provider key
  QDomNode pkeyNode = layer_node.namedItem("provider");

  if (pkeyNode.isNull())
  {
    mProviderKey = "";
  }
  else
  {
    QDomElement pkeyElt = pkeyNode.toElement();
    mProviderKey = pkeyElt.text();
  }

  // Open the raster source based on provider and datasource

  if (!mProviderKey.isEmpty())
  {
    // Go down the raster-data-provider paradigm

    // Collect provider-specific information

    QDomNode rpNode = layer_node.namedItem("rasterproperties");

    // Collect sublayer names and styles
    QStringList layers;
    QStringList styles;
    QDomElement layerElement = rpNode.firstChildElement("wmsSublayer");
    while (!layerElement.isNull())
    {
      // TODO: sublayer visibility - post-0.8 release timeframe

      // collect name for the sublayer
      layers += layerElement.namedItem("name").toElement().text();

      // collect style for the sublayer
      styles += layerElement.namedItem("style").toElement().text();

      layerElement = layerElement.nextSiblingElement("wmsSublayer");
    }

    // Collect format
    QString format = rpNode.namedItem("wmsFormat").toElement().text();

    // Collect CRS
    QString crs = QString("EPSG:%1").arg(srs().epsg());

    // Collect proxy information
    QString proxyHost = rpNode.namedItem("wmsProxyHost").toElement().text();
    int     proxyPort = rpNode.namedItem("wmsProxyPort").toElement().text().toInt();
    QString proxyUser = rpNode.namedItem("wmsProxyUser").toElement().text();
    QString proxyPass = rpNode.namedItem("wmsProxyPass").toElement().text();

    setDataProvider( mProviderKey, layers, styles, format, crs,
        proxyHost, proxyPort, proxyUser, proxyPass );
  }
  else
  {
    // Go down the monolithic-gdal-provider paradigm

    if (!readFile(source()))   // Data source name set in
      // QgsMapLayer::readXML()
    {
      QgsLogger::warning(QString(__FILE__) + ":" + QString(__LINE__) + 
          " unable to read from raster file " + source());
      return false;
    }

  }

  QDomNode mnl = layer_node.namedItem("rasterproperties");

  QDomNode snode = mnl.namedItem("showDebugOverlayFlag");
  QDomElement myElement = snode.toElement();
  QVariant myQVariant = (QVariant) myElement.attribute("boolean");
  setShowDebugOverlayFlag(myQVariant.toBool());

  snode = mnl.namedItem("drawingStyle");
  myElement = snode.toElement();
  setDrawingStyle(myElement.text());

  snode = mnl.namedItem("invertHistogramFlag");
  myElement = snode.toElement();
  myQVariant = (QVariant) myElement.attribute("boolean");
  setInvertHistogramFlag(myQVariant.toBool());

  snode = mnl.namedItem("stdDevsToPlotDouble");
  myElement = snode.toElement();
  setStdDevsToPlot(myElement.text().toDouble());

  snode = mnl.namedItem("redBandNameQString");
  myElement = snode.toElement();
  setRedBandName(myElement.text());

  snode = mnl.namedItem("greenBandNameQString");
  myElement = snode.toElement();
  setGreenBandName(myElement.text());

  snode = mnl.namedItem("blueBandNameQString");
  myElement = snode.toElement();
  setBlueBandName(myElement.text());

  snode = mnl.namedItem("grayBandNameQString");
  myElement = snode.toElement();
  std::cout << __FILE__ << ":" << __LINE__<< " Setting gray band to : " << myElement.text().data() << std::endl;
  setGrayBandName(myElement.text());

  QgsDebugMsg("ReadXml: gray band name " + grayBandNameQString);
  QgsDebugMsg("ReadXml: red band name " + redBandNameQString);
  QgsDebugMsg("ReadXml: green band name  " + greenBandNameQString);
  QgsDebugMsg("Drawing style " + getDrawingStyleAsQString());

  return true;

} // QgsRasterLayer::readXML_( QDomNode & layer_node )



/* virtual */ bool QgsRasterLayer::writeXML_( QDomNode & layer_node,
    QDomDocument & document )
{
  // first get the layer element so that we can append the type attribute

  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || ("maplayer" != mapLayerNode.nodeName()) )
  {
    QgsLogger::warning("QgsRasterLayer::writeXML() can't find <maplayer>");
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

  if (!mProviderKey.isEmpty())
  {
    QStringList sl = subLayers();
    QStringList sls = dataProvider->subLayerStyles();

    QStringList::const_iterator layerStyle = sls.begin();

    // <rasterproperties><wmsSublayer>
    for ( QStringList::const_iterator layerName  = sl.begin();
        layerName != sl.end();
        ++layerName )
    {

#ifdef QGISDEBUG
      std::cout << "<rasterproperties><wmsSublayer> " << layerName->toLocal8Bit().data() << std::endl;
#endif

      QDomElement sublayerElement = document.createElement("wmsSublayer");

      // TODO: sublayer visibility - post-0.8 release timeframe

      // <rasterproperties><wmsSublayer><name>
      QDomElement sublayerNameElement = document.createElement("name");
      QDomText sublayerNameText = document.createTextNode(*layerName);
      sublayerNameElement.appendChild(sublayerNameText);
      sublayerElement.appendChild(sublayerNameElement);

      // <rasterproperties><wmsSublayer><style>
      QDomElement sublayerStyleElement = document.createElement("style");
      QDomText sublayerStyleText = document.createTextNode(*layerStyle);
      sublayerStyleElement.appendChild(sublayerStyleText);
      sublayerElement.appendChild(sublayerStyleElement);

      rasterPropertiesElement.appendChild(sublayerElement);

      // This assumes there are exactly the same number of "layerName"s as there are "layerStyle"s
      ++layerStyle;
    }

    // <rasterproperties><wmsFormat>
    QDomElement formatElement = document.createElement("wmsFormat");
    QDomText formatText =
      document.createTextNode(dataProvider->imageEncoding());
    formatElement.appendChild(formatText);
    rasterPropertiesElement.appendChild(formatElement);

    // <rasterproperties><wmsProxyHost>
    QDomElement proxyHostElement = document.createElement("wmsProxyHost");
    QDomText proxyHostText =
      document.createTextNode(dataProvider->proxyHost());
    proxyHostElement.appendChild(proxyHostText);
    rasterPropertiesElement.appendChild(proxyHostElement);

    // <rasterproperties><wmsProxyPort>
    QDomElement proxyPortElement = document.createElement("wmsProxyPort");
    QDomText proxyPortText =
      document.createTextNode( QString::number(dataProvider->proxyPort()) );
    proxyPortElement.appendChild(proxyPortText);
    rasterPropertiesElement.appendChild(proxyPortElement);

    // <rasterproperties><wmsProxyUser>
    QDomElement proxyUserElement = document.createElement("wmsProxyUser");
    QDomText proxyUserText =
      document.createTextNode(dataProvider->proxyUser());
    proxyUserElement.appendChild(proxyUserText);
    rasterPropertiesElement.appendChild(proxyUserElement);

    // <rasterproperties><wmsProxyPass>
    QDomElement proxyPassElement = document.createElement("wmsProxyPass");
    QDomText proxyPassText =
      document.createTextNode(dataProvider->proxyPass());
    proxyPassElement.appendChild(proxyPassText);
    rasterPropertiesElement.appendChild(proxyPassElement);
  }

  // <showDebugOverlayFlag>
  QDomElement showDebugOverlayFlagElement = document.createElement( "showDebugOverlayFlag" );

  if ( getShowDebugOverlayFlag() )
  {
    showDebugOverlayFlagElement.setAttribute( "boolean", "true" );
  }
  else
  {
    showDebugOverlayFlagElement.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( showDebugOverlayFlagElement );

  // <drawingStyle>
  QDomElement drawStyleElement = document.createElement( "drawingStyle" );
  QDomText    drawStyleText    = document.createTextNode( getDrawingStyleAsQString() );

  drawStyleElement.appendChild( drawStyleText );

  rasterPropertiesElement.appendChild( drawStyleElement );


  // <invertHistogramFlag>
  QDomElement invertHistogramFlagElement = document.createElement( "invertHistogramFlag" );

  if ( getInvertHistogramFlag() )
  {
    invertHistogramFlagElement.setAttribute( "boolean", "true" );
  }
  else
  {
    invertHistogramFlagElement.setAttribute( "boolean", "false" );
  }

  rasterPropertiesElement.appendChild( invertHistogramFlagElement );


  // <stdDevsToPlotDouble>
  QDomElement stdDevsToPlotDoubleElement = document.createElement( "stdDevsToPlotDouble" );
  QDomText    stdDevsToPlotDoubleText    = document.createTextNode( QString::number(getStdDevsToPlot()) );

  stdDevsToPlotDoubleElement.appendChild( stdDevsToPlotDoubleText );

  rasterPropertiesElement.appendChild( stdDevsToPlotDoubleElement );




  // <redBandNameQString>
  QDomElement redBandNameQStringElement = document.createElement( "redBandNameQString" );
  QDomText    redBandNameQStringText    = document.createTextNode( getRedBandName() );

  redBandNameQStringElement.appendChild( redBandNameQStringText );

  rasterPropertiesElement.appendChild( redBandNameQStringElement );


  // <greenBandNameQString>
  QDomElement greenBandNameQStringElement = document.createElement( "greenBandNameQString" );
  QDomText    greenBandNameQStringText    = document.createTextNode( getGreenBandName() );

  greenBandNameQStringElement.appendChild( greenBandNameQStringText );

  rasterPropertiesElement.appendChild( greenBandNameQStringElement );


  // <blueBandNameQString>
  QDomElement blueBandNameQStringElement = document.createElement( "blueBandNameQString" );
  QDomText    blueBandNameQStringText    = document.createTextNode( getBlueBandName() );

  blueBandNameQStringElement.appendChild( blueBandNameQStringText );

  rasterPropertiesElement.appendChild( blueBandNameQStringElement );


  // <grayBandNameQString>
  QDomElement grayBandNameQStringElement = document.createElement( "grayBandNameQString" );
  QDomText    grayBandNameQStringText    = document.createTextNode( getGrayBandName() );

  grayBandNameQStringElement.appendChild( grayBandNameQStringText );

  rasterPropertiesElement.appendChild( grayBandNameQStringElement );


  return true;
} // bool QgsRasterLayer::writeXML_



void QgsRasterLayer::identify(const QgsPoint& point, std::map<QString,QString>& results)
{
  double x = point.x();
  double y = point.y();

  QgsDebugMsg("QgsRasterLayer::identify: " + QString::number(x) + ", " + QString::number(y));

  if ( x < mLayerExtent.xMin() || x > mLayerExtent.xMax() || y < mLayerExtent.yMin() || y > mLayerExtent.yMax() )
  {
    // Outside the raster
    for ( int i = 1; i <= gdalDataset->GetRasterCount(); i++ )
    {
      results[tr("Band") + QString::number(i)] = tr("out of extent");
    }
  }
  else
  {
    /* Calculate the row / column where the point falls */
    double xres = (mLayerExtent.xMax() - mLayerExtent.xMin()) / rasterXDimInt;
    double yres = (mLayerExtent.yMax() - mLayerExtent.yMin()) / rasterYDimInt;

    // Offset, not the cell index -> flor
    int col = (int) floor ( (x - mLayerExtent.xMin()) / xres );
    int row = (int) floor ( (mLayerExtent.yMax() - y) / yres );

    QgsDebugMsg( "row = " + QString::number(row) + " col = " + QString::number(col));

    for ( int i = 1; i <= gdalDataset->GetRasterCount(); i++ )
    {
      GDALRasterBand *gdalBand = gdalDataset->GetRasterBand(i);
      GDALDataType type = gdalBand->GetRasterDataType();
      int size = GDALGetDataTypeSize ( type ) / 8;
      void *data = CPLMalloc ( size );

      CPLErr err = gdalBand->RasterIO ( GF_Read, col, row, 1, 1, data, 1, 1, type, 0, 0 );

      if (err != CPLE_None)
      {
        QgsLogger::warning("RaterIO error: " + QString(CPLGetLastErrorMsg()));
      }

      double value = readValue ( data, type, 0 );
#ifdef QGISDEBUG
      QgsLogger::debug("value", value, 1, __FILE__, __FUNCTION__, __LINE__);
#endif
      QString v;

      if ( noDataValueDouble == value || value != value )
      {
        v = tr("null (no data)");
      }
      else
      {
        v.setNum ( value );
      }
      results[tr("Band") + QString::number(i)] = v;

      free (data);
    }
  }

} // void QgsRasterLayer::identify


QString QgsRasterLayer::identifyAsText(const QgsPoint& point)
{
  if (mProviderKey != "wms")
  {
    // Currently no meaning for anything other than OGC WMS layers
    return QString();
  }

  return (dataProvider->identifyAsText(point));
}

void QgsRasterLayer::populateHistogram(int theBandNoInt, int theBinCountInt,bool theIgnoreOutOfRangeFlag,bool theHistogramEstimatedFlag)
{

  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  QgsRasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  //calculate the histogram for this band
  //we assume that it only needs to be calculated if the lenght of the histogram
  //vector is not equal to the number of bins
  //i.e if the histogram has never previously been generated or the user has
  //selected a new number of bins.
  if (myRasterBandStats.histogramVector->size()!=theBinCountInt ||
      theIgnoreOutOfRangeFlag != myRasterBandStats.histogramOutOfRangeFlag ||
      theHistogramEstimatedFlag != myRasterBandStats.histogramEstimatedFlag)
  {
    myRasterBandStats.histogramVector->clear();
    myRasterBandStats.histogramEstimatedFlag=theHistogramEstimatedFlag;
    myRasterBandStats.histogramOutOfRangeFlag=theIgnoreOutOfRangeFlag;
    int *myHistogramArray = new int[theBinCountInt];


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
    double myInterval = (myRasterBandStats.maxValDouble-myRasterBandStats.minValDouble)/theBinCountInt;
    myGdalBand->GetHistogram( myRasterBandStats.minValDouble-0.1*myInterval, myRasterBandStats.maxValDouble+0.1*myInterval, theBinCountInt, myHistogramArray ,theIgnoreOutOfRangeFlag ,theHistogramEstimatedFlag , GDALDummyProgress, NULL );

    for (int myBin = 0; myBin <theBinCountInt; myBin++)
    {
      myRasterBandStats.histogramVector->push_back( myHistogramArray[myBin]);
      QgsDebugMsg("Added " + QString::number(myHistogramArray[myBin]) + " to histogram vector");
    }

  }
  QgsDebugMsg(">>>>> Histogram vector now contains " + QString::number(myRasterBandStats.histogramVector->size()) +\
      " elements");
}


/*
 * 
 * New functions that will convert this class to a data provider interface
 * (B Morley)
 *
 */ 



QgsRasterLayer::QgsRasterLayer(int dummy,
    QString const & rasterLayerPath,
    QString const & baseName,
    QString const & providerKey,
    QStringList const & layers,
    QStringList const & styles,
    QString const & format,
    QString const & crs,
    QString const & proxyHost,
    int proxyPort,
    QString const & proxyUser,
    QString const & proxyPass )
: QgsMapLayer(RASTER, baseName, rasterLayerPath),
  rasterXDimInt( std::numeric_limits<int>::max() ),
  rasterYDimInt( std::numeric_limits<int>::max() ),
  showDebugOverlayFlag(false),
  invertHistogramFlag(false),
  stdDevsToPlotDouble(0),
  mProviderKey(providerKey),
  dataProvider(0),
  mEditable(false),
mModified(false)

{
  QgsDebugMsg("QgsRasterLayer::QgsRasterLayer(4 arguments): starting. with layer list of "+\
      layers.join(", ") +  " and style list of "+ styles.join(", ") + " and format of " +\
      format +  " and CRS of " + crs);

  // Initialise the affine transform matrix
  adfGeoTransform[0] =  0;
  adfGeoTransform[1] =  1;
  adfGeoTransform[2] =  0;
  adfGeoTransform[3] =  0;
  adfGeoTransform[4] =  0;
  adfGeoTransform[5] = -1;

  // if we're given a provider type, try to create and bind one to this layer
  if ( ! providerKey.isEmpty() )
  {
    setDataProvider( providerKey, layers, styles, format, crs,
        proxyHost, proxyPort, proxyUser, proxyPass );
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
      dataProvider, SIGNAL(setStatus        (QString)),
      this,           SLOT(showStatusMessage(QString))
      );
  QgsDebugMsg("QgsRasterLayer::QgsRasterLayer(4 arguments): exiting.");

  emit setStatus("QgsRasterLayer created");
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
    QString const & crs,
    QString const & proxyHost,
    int proxyPort,
    QString const & proxyUser,
    QString const & proxyPass )
{
  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  mProviderKey = provider;     // XXX is this necessary?  Usually already set
  // XXX when execution gets here.

  // load the plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QString ogrlib = pReg->library(provider);

  //QString ogrlib = libDir + "/libpostgresprovider.so";

#ifdef TESTPROVIDERLIB
  const char *cOgrLib = (const char *) ogrlib;
  // test code to help debug provider loading problems
  //  void *handle = dlopen(cOgrLib, RTLD_LAZY);
  void *handle = dlopen(cOgrLib, RTLD_LAZY | RTLD_GLOBAL);
  if (!handle)
  {
    QgsLogger::warning("Error in dlopen: ");
  }
  else
  {
    QgsDebugMsg("dlopen suceeded");
    dlclose(handle);
  }

#endif

  // load the data provider
  myLib = new QLibrary((const char *) ogrlib);
  QgsDebugMsg("QgsRasterLayer::setDataProvider: Library name is " + myLib->library());
  bool loaded = myLib->load();

  if (loaded)
  {
    QgsDebugMsg("QgsRasterLayer::setDataProvider: Loaded data provider library");
    QgsDebugMsg("QgsRasterLayer::setDataProvider: Attempting to resolve the classFactory function");
    classFactoryFunction_t * classFactory = (classFactoryFunction_t *) myLib->resolve("classFactory");

    mValid = false;            // assume the layer is invalid until we
    // determine otherwise
    if (classFactory)
    {
      QgsDebugMsg("QgsRasterLayer::setDataProvider: Getting pointer to a dataProvider object from the library");
      //XXX - This was a dynamic cast but that kills the Windows
      //      version big-time with an abnormal termination error
      //      dataProvider = (QgsRasterDataProvider*)(classFactory((const
      //                                              char*)(dataSource.utf8())));

      // Copied from qgsproviderregistry in preference to the above.
      dataProvider = (QgsRasterDataProvider*)(*classFactory)(&mDataSource);

      if (dataProvider)
      {
        QgsDebugMsg("QgsRasterLayer::setDataProvider: Instantiated the data provider plugin" +\
            QString(" with layer list of ") + layers.join(", ") + " and style list of " + styles.join(", ")+\
            " and format of " + format +  " and CRS of " + crs);
        if (dataProvider->isValid())
        {
          mValid = true;

          dataProvider->addLayers(layers, styles);
          dataProvider->setImageEncoding(format);
          dataProvider->setImageCrs(crs);
          dataProvider->setProxy(proxyHost, proxyPort, proxyUser, proxyPass);

          // get the extent
          QgsRect mbr = dataProvider->extent();

          // show the extent
          QString s = mbr.stringRep();
          QgsDebugMsg("QgsRasterLayer::setDataProvider: Extent of layer: " + s);
          // store the extent
          mLayerExtent.setXmax(mbr.xMax());
          mLayerExtent.setXmin(mbr.xMin());
          mLayerExtent.setYmax(mbr.yMax());
          mLayerExtent.setYmin(mbr.yMin());

          // upper case the first letter of the layer name
          QgsDebugMsg("QgsRasterLayer::setDataProvider: mLayerName: " + name());

          // set up the raster drawing style
          drawingStyle = MULTI_BAND_COLOR;  //sensible default

          // Setup source SRS
          *mSRS = QgsSpatialRefSys();
          mSRS->createFromOgcWmsCrs(crs);
        }
      }
      else
      {
        QgsLogger::warning("QgsRasterLayer::setDataProvider: Unable to instantiate the data provider plugin");
        mValid = false;
      }
    }
  }
  else
  {
    mValid = false;
    QgsLogger::warning("QgsRasterLayer::setDataProvider: Failed to load ../providers/libproviders.so");

  }
  QgsDebugMsg("QgsRasterLayer::setDataProvider: exiting.");

} // QgsRasterLayer::setDataProvider


bool QgsRasterLayer::setProxy(QString const & host,
    int port,
    QString const & user,
    QString const & pass)
{
  if (!dataProvider)
  {
    return FALSE;
  }
  else
  {
#ifdef QGISDEBUG
    std::cout << "  QgsRasterLayer::setProxy: host = " << host.toLocal8Bit().data() << "." << std::endl;
    std::cout << "  QgsRasterLayer::setProxy: port = " << port << "." << std::endl;
    std::cout << "  QgsRasterLayer::setProxy: user = " << user.toLocal8Bit().data() << "." << std::endl;
    std::cout << "  QgsRasterLayer::setProxy: pass = " << pass.toLocal8Bit().data() << "." << std::endl;
#endif
    return dataProvider->setProxy(host, port, user, pass);
  }
}



bool QgsRasterLayer::usesProvider()
{
  if (mProviderKey.isEmpty())
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
  if (mProviderKey.isEmpty())
  {
    return QString();
  }
  else
  {
    return mProviderKey;
  }
}


void QgsRasterLayer::showStatusMessage(QString const & theMessage)
{
#ifdef QGISDEBUG
  //  std::cout << "QgsRasterLayer::showStatusMessage: entered with '" << theMessage << "'." << std::endl;
#endif
  // Pass-through
  // TODO: See if we can connect signal-to-signal.  This is a kludge according to the Qt doc.
  emit setStatus(theMessage);
}


QString QgsRasterLayer::errorCaptionString()
{
  return mErrorCaption;
}


QString QgsRasterLayer::errorString()
{
  return mError;
}


QgsRasterDataProvider* QgsRasterLayer::getDataProvider()
{
  return dataProvider;
}

const QgsRasterDataProvider* QgsRasterLayer::getDataProvider() const
{
  return dataProvider;
}

const unsigned int QgsRasterLayer::getBandCount()
{
  return rasterStatsVector.size();
};

double QgsRasterLayer::rasterUnitsPerPixel()
{
 
  // We return one raster pixel per map unit pixel
  // One raster pixel can have several raster units...

  // We can only use one of the adfGeoTransform[], so go with the
   // horisontal one.
  
  return fabs(adfGeoTransform[1]);
}
 

// ENDS
