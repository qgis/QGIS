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

/*
Please observe the following variable naming guidelines when editing this class:
---------------------------------------------------------------------------------
In my opinion, clarity of code is more important than brevity, so variables should be
given clear, unambiguous names. Variables names should be written in mixed case, with
a lowercase first letter. Each variable name should include a scope resolution
indicator and a type indicator, in the form:
 
[scope]+[name]+[type]
 
Where scope resolution indicators are:
 
- global vars and class members : [none]
- variables passed as parameters to a function/method: the
- variables declared locally in a method or function: my
 
For example:
 
class FooClass {
int fooInt;  //class var has no prefix
 
void FooClass::fooMethod (int theBarInt)  //function parameter prefixed by 'the'
{
fooInt=1;
int myLocalInt=0; //function members prefixed by 'my'
myLocalInt=fooInt+theBarInt;
}
}
 
Using this scope resolution naming scheme makes the origin of each variable unambiguous
and the code easy to read (especially by people who did not write it!).
 
The [name] part of the variable should be short and descriptive, usually a noun.
 
The [type] part of the variable should be the type class of the variable written out in full.
 
 
DEBUG DIRECTIVES:
 
When compiling you can make sure DEBUG is defined by including -DDEBUG in the gcc command (e.g. gcc -DDEBUG myprog.c ) if you
wish to see edbug messages printed to stdout.
 
*/
#include "qgsrasterlayer.h"

#include <cstdio>
#include <cmath>
#include <limits>
#include <iostream>

#include <qapplication.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qimage.h>
#include <qfont.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qfontmetrics.h>
#include <qwmatrix.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qslider.h>
#include <qlabel.h>
#include <qdom.h>
#include <qlistview.h>
#include <qwidget.h>
#include <qwidgetlist.h>


/*
 * 
 * New includes that will convert this class to a data provider interface
 * (B Morley)
 *
 */ 
 
#include <qlibrary.h>

/*
 * END
 */


#include "qgsrect.h"
#include "qgisapp.h"
#include "qgscolortable.h"
#include "qgsrasterlayerproperties.h"
#include "qgsproject.h"
#include "qgsidentifyresults.h"
#include "qgsattributeaction.h"

#include "qgsspatialrefsys.h"

/*
 * 
 * New includes that will convert this class to a data provider interface
 * (B Morley)
 *
 */ 

#include "qgsproviderregistry.h"

/*
 * END
 */
 

#include <gdal_priv.h>

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
    std::cerr << "unable to get GDALDriverManager\n";
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

  QString catchallFilter;       // for Any file(*.*), but also for those
  // drivers with no specific file
  // filter

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
      qWarning("unable to get driver %d", i);
      continue;
    }
    // now we need to see if the driver is for something currently
    // supported; if not, we give it a miss for the next driver

    myGdalDriverDescription = myGdalDriver->GetDescription();

    if (!isSupportedRasterDriver(myGdalDriverDescription))
    {
      // not supported, therefore skip
#ifdef QGISDEBUG
      qWarning("skipping unsupported driver %s", myGdalDriver->GetDescription());
#endif
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
        catchallFilter += QString(myGdalDriver->GetDescription()) + " ";
      }
    }
    
    // A number of drivers support JPEG 2000. Add it in for those.
    if (  myGdalDriverDescription.startsWith("MrSID") 
          || myGdalDriverDescription.startsWith("ECW")
          || myGdalDriverDescription.startsWith("JPEG2000")
          || myGdalDriverDescription.startsWith("JP2KAK") )
    {
      QString glob = "*.jp2 *.j2k";
      theFileFiltersString += "JPEG 2000 (" + glob.lower() + " " + glob.upper() + ");;";
    }

    // A number of drivers support JPEG 2000. Add it in for those.
    if (  myGdalDriverDescription.startsWith("MrSID")
          || myGdalDriverDescription.startsWith("ECW")
          || myGdalDriverDescription.startsWith("JPEG2000")
          || myGdalDriverDescription.startsWith("JP2KAK") )
    {
      QString glob = "*.jp2 *.j2k";
      theFileFiltersString += "JPEG 2000 (" + glob.lower() + " " + glob.upper() + ");;";
    }

    myGdalDriverExtension = myGdalDriverLongName = "";  // reset for next driver

  }                           // each loaded GDAL driver

  // can't forget the default case
  theFileFiltersString += catchallFilter + "All other files (*)";
#ifdef QGISDEBUG
  std::cout << "Raster filter list built: " << theFileFiltersString << std::endl;
#endif
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
bool QgsRasterLayer::isValidRasterFileName(QString theFileNameQString)
{

  GDALDatasetH myDataset;
  GDALAllRegister();

  //open the file using gdal making sure we have handled locale properly
  myDataset = GDALOpen( (const char*)(theFileNameQString.local8Bit()), GA_ReadOnly );
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



/** Overloaded of the above function provided for convenience that takes a qstring pointer */
bool QgsRasterLayer::isValidRasterFileName(QString * theFileNameQString)
{
  //dereference and delegate
  return isValidRasterFileName(*theFileNameQString);
}



//////////////////////////////////////////////////////////
//
// Non Static methods now....
//
/////////////////////////////////////////////////////////
QgsRasterLayer::QgsRasterLayer(QString path, QString baseName)
    : QgsMapLayer(RASTER, baseName, path),
    // XXX where is this? popMenu(0), //popMenu is the contextmenu obtained by right clicking on the legend
    invertHistogramFlag(false),
    stdDevsToPlotDouble(0),
    transparencyLevelInt(255), // 0 is completely transparent
    showDebugOverlayFlag(false),
    mLayerProperties(0x0),
    mTransparencySlider(0x0),
    mIdentifyResults(0),
    dataProvider(0)
{
  // we need to do the tr() stuff outside of the loop becauses tr() is a time
  // consuming operation nd we dont want to do it in the loop!

  redTranslatedQString   = tr("Red");
  greenTranslatedQString = tr("Green");
  blueTranslatedQString  = tr("Blue");

  // set the layer name (uppercase first character)

  if ( ! baseName.isEmpty() )   // XXX shouldn't this happen in parent?
  {
    QString layerTitle = baseName;
    std::cout << "layertitle length" << layerTitle.length() << std::endl;
    layerTitle = layerTitle.left(1).upper() + layerTitle.mid(1);
    setLayerName(layerTitle);
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

  if (!(providerKey))
  {
    GDALClose(gdalDataset);
  }  
}



bool
QgsRasterLayer::readFile( QString const & fileName )
{
  GDALAllRegister();

  //open the dataset making sure we handle char encoding of locale properly
  gdalDataset = (GDALDataset *) GDALOpen((const char*)(fileName.local8Bit()), GA_ReadOnly);

  if (gdalDataset == NULL)
  {
    valid = FALSE;
    return false;
  }

  // Store timestamp
  mLastModified = lastModified ( fileName.local8Bit() );

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

  //load  up the pyramid icons
#if defined(WIN32) || defined(Q_OS_MACX)
  QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif

  mPyramidPixmap.load(QString(PKGDATAPATH) + QString("/images/icons/pyramid.png"));
  mNoPyramidPixmap.load(QString(PKGDATAPATH) + QString("/images/icons/no_pyramid.png"));

  //just testing remove this later
  getMetadata();

  // Use the affine transform to get geo coordinates for
  // the corners of the raster
  double myXMaxDouble = adfGeoTransform[0] +
                        gdalDataset->GetRasterXSize() * adfGeoTransform[1] +
                        gdalDataset->GetRasterYSize() * adfGeoTransform[2];
  double myYMinDouble = adfGeoTransform[3] +
                        gdalDataset->GetRasterXSize() * adfGeoTransform[4] +
                        gdalDataset->GetRasterYSize() * adfGeoTransform[5];

  layerExtent.setXmax(myXMaxDouble);
  // The affine transform reduces to these values at the
  // top-left corner of the raster
  layerExtent.setXmin(adfGeoTransform[0]);
  layerExtent.setYmax(adfGeoTransform[3]);
  layerExtent.setYmin(myYMinDouble);

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
    RasterBandStats myRasterBandStats;
    myRasterBandStats.bandName = myColorQString + " (" + QString::number(i) + ")";
    //myRasterBandStats.bandName=QString::number(i) + " : " + myColorQString;
    myRasterBandStats.bandNoInt = i;
    myRasterBandStats.statsGatheredFlag = false;
    myRasterBandStats.histogramVector = new RasterBandStats::HistogramVector();
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
    redBandNameQString = redTranslatedQString; // sensible default
    greenBandNameQString = greenTranslatedQString; // sensible default
    blueBandNameQString = blueTranslatedQString; // sensible default
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
    grayBandNameQString = tr("Not Set");  //sensible default
    drawingStyle = MULTI_BAND_COLOR;  //sensible default
  }
  else                        //GRAY_OR_UNDEFINED
  {
    getRasterBandStats(1);
    redBandNameQString = tr("Not Set"); //sensible default
    greenBandNameQString = tr("Not Set"); //sensible default
    blueBandNameQString = tr("Not Set");  //sensible default
    drawingStyle = SINGLE_BAND_GRAY;  //sensible default
    if (hasBand("Gray"))
    {
      grayBandNameQString = "Gray"; // sensible default //dont tr() this its a gdal word!
    }
    else if (hasBand("Undefined")) //dont tr() this its a gdal word!
    {
      grayBandNameQString = "Undefined";  // sensible default
    }
  }

  // Get the layer's projection info and set up the
  // QgsCoordinateTransform for this layer

  QString mySourceWKT = getProjectionWKT();
  //get the project projection, defaulting to this layer's projection
  //if none exists....
  QString myDestWKT = QgsProject::instance()->readEntry("SpatialRefSys","/WKT",mySourceWKT);
  //set up the coordinat transform - in the case of raster this is mainly used to convert
  //the inverese projection of the map extents of the canvas when zzooming in etc. so
  //that they match the coordinate system of this layer
  mCoordinateTransform = new QgsCoordinateTransform(mySourceWKT,myDestWKT);
  //validate the source coordinate system!
  mCoordinateTransform->sourceSRS().validate();
  

  //mark the layer as valid
  valid=TRUE;
  return true;

} // QgsRasterLayer::readFile

void QgsRasterLayer::closeDataset()
{
  if ( !valid  ) return;
  valid = FALSE;

  GDALClose(gdalDataset);
  gdalDataset = 0;

  hasPyramidsFlag=false;
  mPyramidList.clear();

  rasterStatsVector.clear();
}

bool QgsRasterLayer::update()
{
#ifdef QGISDEBUG
  std::cerr << "QgsRasterLayer::update" << std::endl;
#endif

  if ( mLastModified < QgsRasterLayer::lastModified ( source() ) )
  {
#ifdef QGISDEBUG
    std::cerr << "Outdated -> reload" << std::endl;
#endif
    closeDataset();
    return readFile ( source() );
  }
  return true;
}

QDateTime QgsRasterLayer::lastModified ( QString name )
{
#ifdef QGISDEBUG
  std::cerr << "QgsRasterLayer::lastModified: " << name << std::endl;
#endif
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
      if ( f.open ( IO_ReadOnly ) )
      {
        QString ln;
        QString dir = fi.dirPath() + "/../../../";
        while ( f.readLine(ln,100) != -1 )
        {
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
#ifdef QGISDEBUG
  std::cerr << "last modified = " << t.toString() << std::endl;
#endif

  return t;
}


void QgsRasterLayer::showLayerProperties()
{
  std::cerr << "SHOWING RASTER LAYER PROPERTIES DIALOG\n";
  qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
  if ( ! mLayerProperties )
  {
    mLayerProperties = new QgsRasterLayerProperties(this);
#ifdef QGISDEBUG
    std::cout << "Creating new raster properties dialog instance" << std::endl;
#endif
  }

  mLayerProperties->sync();
  mLayerProperties->raise();
  mLayerProperties->show();
  qApp->restoreOverrideCursor();
} // QgsRasterLayer::showLayerProperties()



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

void QgsRasterLayer::setDrawingStyle(QString theDrawingStyleQString)
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
bool QgsRasterLayer::hasBand(QString theBandName)
{
#ifdef QGISDEBUG
  std::cout << "Looking for band : " << theBandName << std::endl;
#endif

  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++)
  {
    GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(i);
    QString myColorQString = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
#ifdef QGISDEBUG

    std::cout << "band : " << i << std::endl;
#endif

    if (myColorQString == theBandName)
    {
#ifdef QGISDEBUG
      std::cout << "band : " << i << std::endl;
      std::cout << "Found band : " << theBandName << std::endl;
#endif

      return true;
    }
#ifdef QGISDEBUG
    std::cout << "Found unmatched band : " << i << " " << myColorQString << std::endl;
#endif

  }
  return false;
}

void QgsRasterLayer::drawThumbnail(QPixmap * theQPixmap)
{
  theQPixmap->fill(); //defaults to white
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



QPixmap QgsRasterLayer::getPaletteAsPixmap()
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::getPaletteAsPixmap" << std::endl;
#endif
  if (hasBand("Palette") ) //dont tr() this its a gdal word!
  {
#ifdef QGISDEBUG
    std::cout << "....found paletted image" << std::endl;
#endif
    QgsColorTable *myColorTable = colorTable ( 1 );
    GDALRasterBandH myGdalBand = gdalDataset->GetRasterBand(1);
    if( GDALGetRasterColorInterpretation(myGdalBand) == GCI_PaletteIndex && myColorTable->defined() )
    {
#ifdef QGISDEBUG
      std::cout << "....found GCI_PaletteIndex" << std::endl;
#endif
      double myMinDouble = myColorTable->min();
      double myMaxDouble = myColorTable->max();

#ifdef QGISDEBUG
      std::cout << "myMinDouble = " << myMinDouble << " myMaxDouble = " << myMaxDouble << std::endl;
#endif

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



void QgsRasterLayer::draw(QPainter * theQPainter,
                          QgsRect * theViewExtent,
                          QgsMapToPixel * theQgsMapToPixel,
                          QPaintDevice* dst)
{

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::draw(4 arguments): entered." << std::endl;
#endif

  //Dont waste time drawing if transparency is at 0 (completely transparent)
  if (transparencyLevelInt == 0)
    return;

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::draw(4 arguments): checking timestamp." << std::endl;
#endif

/* TODO: Re-enable this for providers    
  // Check timestamp
  if ( !providerKey )
  {

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::draw(4 arguments): checking timestamp with no providerKey." << std::endl;
#endif
    
    if ( !update() )
    {
      return;
    }
  }    
*/
  
  // clip raster extent to view extent
  QgsRect myRasterExtent = theViewExtent->intersect(&layerExtent);
  if (myRasterExtent.isEmpty())
  {
    // nothing to do
    return;
  }
  
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::draw(4 arguments): myRasterExtent is '" << myRasterExtent << "'." << std::endl;
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
  myRasterViewPort->rectXOffsetFloat = (theViewExtent->xMin() - layerExtent.xMin()) / fabs(adfGeoTransform[1]);
  myRasterViewPort->rectYOffsetFloat = (layerExtent.yMax() - theViewExtent->yMax()) / fabs(adfGeoTransform[5]);
  
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


  // get dimensions of clipped raster image in raster pixel space/ RasterIO will do the scaling for us.
  // So for example, if the user is zoomed in a long way, there may only be e.g. 5x5 pixels retrieved from
  // the raw raster data, but rasterio will seamlessly scale the up to whatever the screen coordinats are
  // e.g. a 600x800 display window (see next section below)
  myRasterViewPort->clippedXMinDouble = (myRasterExtent.xMin() - adfGeoTransform[0]) / adfGeoTransform[1];
  myRasterViewPort->clippedXMaxDouble = (myRasterExtent.xMax() - adfGeoTransform[0]) / adfGeoTransform[1];
  myRasterViewPort->clippedYMinDouble = (myRasterExtent.yMin() - adfGeoTransform[3]) / adfGeoTransform[5];
  myRasterViewPort->clippedYMaxDouble = (myRasterExtent.yMax() - adfGeoTransform[3]) / adfGeoTransform[5];
  myRasterViewPort->clippedWidthInt =
    abs(static_cast < int >(myRasterViewPort->clippedXMaxDouble - myRasterViewPort->clippedXMinDouble));
  myRasterViewPort->clippedHeightInt =
    abs(static_cast < int >(myRasterViewPort->clippedYMaxDouble - myRasterViewPort->clippedYMinDouble));
  
  // Add one to the raster dimensions to guard against the integer truncation
  // effects of static_cast<int>
  myRasterViewPort->clippedWidthInt++;
  myRasterViewPort->clippedHeightInt++;
  
  // make sure we don't exceed size of raster
  if (myRasterViewPort->clippedWidthInt > rasterXDimInt)
  {
    myRasterViewPort->clippedWidthInt = rasterXDimInt;
  }
  if (myRasterViewPort->clippedHeightInt > rasterYDimInt)
  {
    myRasterViewPort->clippedHeightInt = rasterYDimInt;
  }

  // get dimensions of clipped raster image in device coordinate space (this is the size of the viewport)
  myRasterViewPort->topLeftPoint = theQgsMapToPixel->transform(myRasterExtent.xMin(), myRasterExtent.yMax());
  myRasterViewPort->bottomRightPoint = theQgsMapToPixel->transform(myRasterExtent.xMax(), myRasterExtent.yMin());

  myRasterViewPort->drawableAreaXDimInt = 
    static_cast<int>( myRasterViewPort->clippedWidthInt  
                    / theQgsMapToPixel->mapUnitsPerPixel() 
                    * fabs(adfGeoTransform[1]) );
                    
  myRasterViewPort->drawableAreaYDimInt = 
    static_cast<int>( myRasterViewPort->clippedHeightInt 
                    / theQgsMapToPixel->mapUnitsPerPixel() 
                    * fabs(adfGeoTransform[5]) );

#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::draw: mapUnitsPerPixel      = " << theQgsMapToPixel->mapUnitsPerPixel() << std::endl; 
    
    std::cout << "QgsRasterLayer::draw: rectXOffsetFloat      = " << myRasterViewPort->rectXOffsetFloat << std::endl; 
    std::cout << "QgsRasterLayer::draw: rectXOffsetInt        = " << myRasterViewPort->rectXOffsetInt << std::endl; 
    std::cout << "QgsRasterLayer::draw: rectYOffsetFloat      = " << myRasterViewPort->rectYOffsetFloat << std::endl; 
    std::cout << "QgsRasterLayer::draw: rectYOffsetInt        = " << myRasterViewPort->rectYOffsetInt << std::endl; 

    std::cout << "QgsRasterLayer::draw: myRasterExtent.xMin() = " << myRasterExtent.xMin() << std::endl; 
    std::cout << "QgsRasterLayer::draw: myRasterExtent.xMax() = " << myRasterExtent.xMax() << std::endl; 
    std::cout << "QgsRasterLayer::draw: myRasterExtent.yMin() = " << myRasterExtent.yMin() << std::endl; 
    std::cout << "QgsRasterLayer::draw: myRasterExtent.yMax() = " << myRasterExtent.yMax() << std::endl; 
    
    std::cout << "QgsRasterLayer::draw: topLeftPoint.x()      = " << myRasterViewPort->topLeftPoint.x() << std::endl; 
    std::cout << "QgsRasterLayer::draw: bottomRightPoint.x()  = " << myRasterViewPort->bottomRightPoint.x() << std::endl; 
    std::cout << "QgsRasterLayer::draw: topLeftPoint.y()      = " << myRasterViewPort->topLeftPoint.y() << std::endl; 
    std::cout << "QgsRasterLayer::draw: bottomRightPoint.y()  = " << myRasterViewPort->bottomRightPoint.y() << std::endl; 
       
    std::cout << "QgsRasterLayer::draw: clippedXMinDouble     = " << myRasterViewPort->clippedXMinDouble << std::endl; 
    std::cout << "QgsRasterLayer::draw: clippedXMaxDouble     = " << myRasterViewPort->clippedXMaxDouble << std::endl; 
    std::cout << "QgsRasterLayer::draw: clippedYMinDouble     = " << myRasterViewPort->clippedYMinDouble << std::endl; 
    std::cout << "QgsRasterLayer::draw: clippedYMaxDouble     = " << myRasterViewPort->clippedYMaxDouble << std::endl; 
    
    std::cout << "QgsRasterLayer::draw: clippedWidthInt       = " << myRasterViewPort->clippedWidthInt << std::endl; 
    std::cout << "QgsRasterLayer::draw: clippedHeightInt      = " << myRasterViewPort->clippedHeightInt << std::endl; 
    std::cout << "QgsRasterLayer::draw: drawableAreaXDimInt   = " << myRasterViewPort->drawableAreaXDimInt << std::endl; 
    std::cout << "QgsRasterLayer::draw: drawableAreaYDimInt   = " << myRasterViewPort->drawableAreaYDimInt << std::endl; 
#endif
  
  // /\/\/\ - added to handle zoomed-in rasters

  
    // Provider mode: See if a provider key is specified, and if so use the provider instead
  
#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::draw: Checking for provider key." << std::endl; 
#endif
  
  if (providerKey)
  {
#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::draw: Wanting a '" << providerKey << "' provider to draw this." << std::endl; 
#endif

  emit setStatus(QString("Retrieving using ")+providerKey);
    
    QImage* image = 
      dataProvider->draw(
                         myRasterExtent, 
                         myRasterViewPort->drawableAreaXDimInt,
                         myRasterViewPort->drawableAreaYDimInt
                        );

#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::draw: Done dataProvider->draw." << std::endl; 
#endif


#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::draw: image stats: "
              << "depth = " << image->depth() << ", "
              << "bytes = " << image->numBytes() << ", "
              << "width = " << image->width() << ", "
              << "height = " << image->height()
              << "." << std::endl; 
 

#endif


#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::draw: Want to theQPainter->drawImage with "
              << "origin x = " << myRasterViewPort->topLeftPoint.x() 
              << " (" << static_cast<int>(myRasterViewPort->topLeftPoint.x()) << "), "
              << "origin y = " << myRasterViewPort->topLeftPoint.y()
              << " (" << static_cast<int>(myRasterViewPort->topLeftPoint.y()) << ")"
              << "." << std::endl; 
#endif
  

                          
    theQPainter->drawImage(static_cast<int>(myRasterViewPort->topLeftPoint.x()),
                           static_cast<int>(myRasterViewPort->topLeftPoint.y()),
                           *image);

  }
  else
  {
    // Otherwise use the old-fashioned GDAL direct-drawing style
    // TODO: Move into its own GDAL provider.

    // \/\/\/ - commented-out to handle zoomed-in rasters
//    draw(theQPainter,myRasterViewPort);
    // /\/\/\ - commented-out to handle zoomed-in rasters
    // \/\/\/ - added to handle zoomed-in rasters
    draw(theQPainter, myRasterViewPort, theQgsMapToPixel);
    // /\/\/\ - added to handle zoomed-in rasters
    
  }
  
  delete myRasterViewPort;

#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::draw: exiting." << std::endl; 
#endif

}

void QgsRasterLayer::draw (QPainter * theQPainter, QgsRasterViewPort * myRasterViewPort,
                           QgsMapToPixel * theQgsMapToPixel)
{
#ifdef QGISDEBUG
  std::cerr << "QgsRasterLayer::draw" << std::endl;
#endif
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
      drawSingleBandGray(theQPainter, myRasterViewPort,
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
      drawSingleBandPseudoColor(theQPainter, myRasterViewPort, 
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
#ifdef QGISDEBUG
      std::cout << "PALETTED_SINGLE_BAND_GRAY drawing type detected..." << std::endl;
#endif

      int myBandNoInt = 1;
      drawPalettedSingleBandGray(theQPainter, myRasterViewPort, 
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
      drawPalettedSingleBandPseudoColor(theQPainter, myRasterViewPort,
                                        theQgsMapToPixel, myBandNoInt, grayBandNameQString);
      break;
    }
    //a "Palette" image where the bands contains 24bit color info and 8 bits is pulled out per color
  case PALETTED_MULTI_BAND_COLOR:
    drawPalettedMultiBandColor(theQPainter, myRasterViewPort,
                               theQgsMapToPixel, 1);
    break;
    // a layer containing 2 or more bands, but using only one band to produce a grayscale image
  case MULTI_BAND_SINGLE_BAND_GRAY:
#ifdef QGISDEBUG

    std::cout << "MULTI_BAND_SINGLE_BAND_GRAY drawing type detected..." << std::endl;
#endif
    //check the band is set!
    if (grayBandNameQString == tr("Not Set"))
    {
#ifdef QGISDEBUG
      std::cout << "MULTI_BAND_SINGLE_BAND_GRAY Not Set detected..." << grayBandNameQString << std::endl;
#endif

      break;
    }
    else
    {

      //get the band number for the mapped gray band
      drawMultiBandSingleBandGray(theQPainter, myRasterViewPort,
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

      drawMultiBandSingleBandPseudoColor(theQPainter, myRasterViewPort,
                                         theQgsMapToPixel, getRasterBandNumber(grayBandNameQString));
      break;
    }
    //a layer containing 2 or more bands, mapped to the three RGBcolors.
    //In the case of a multiband with only two bands, one band will have to be mapped to more than one color
  case MULTI_BAND_COLOR:
    drawMultiBandColor(theQPainter, myRasterViewPort,
                       theQgsMapToPixel);
    break;

  default:
    break;

  }

  //see if debug info is wanted
  if (showDebugOverlayFlag)
  {
    showDebugOverlay(theQPainter, myRasterViewPort);
  };

}                               //end of draw method


void QgsRasterLayer::drawSingleBandGray(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,                                                                             QgsMapToPixel * theQgsMapToPixel, int theBandNoInt)
{
#ifdef QGISDEBUG
  std::cerr << "QgsRasterLayer::drawSingleBandGray called for layer " << theBandNoInt << std::endl;
#endif
  RasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);

  double myRangeDouble = myRasterBandStats.rangeDouble;

  // print each point in myGdalScanData with equal parts R, G ,B o make it show as gray
  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      double myGrayValDouble = readValue ( myGdalScanData, myDataType,
                                           myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );

      if ( myGrayValDouble == noDataValueDouble || myGrayValDouble != myGrayValDouble ) continue;

      int myGrayValInt = static_cast < int >( (myGrayValDouble-myRasterBandStats.minValDouble)
                                              * (255/myRangeDouble));

      if (invertHistogramFlag)
      {
        myGrayValDouble = 255 - myGrayValDouble;
      }
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myGrayValInt, myGrayValInt, myGrayValInt, transparencyLevelInt));
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

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer - painting image to canvas at " 
            << paintXoffset << ", " << paintYoffset << "." << std::endl;
#endif
  
  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x()),
                         static_cast<int>(theRasterViewPort->topLeftPoint.y()),
                         myQImage,
                         paintXoffset,
                         paintYoffset);

} // QgsRasterLayer::drawSingleBandGray


void QgsRasterLayer::drawSingleBandPseudoColor(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort,                                                                             QgsMapToPixel * theQgsMapToPixel, int theBandNoInt)
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawSingleBandPseudoColor called" << std::endl;
#endif

  RasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);

  //calculate the adjusted matrix stats - which come into affect if the user has chosen
  RasterBandStats myAdjustedRasterBandStats = getRasterBandStats(theBandNoInt);

  double myRangeDouble = myAdjustedRasterBandStats.rangeDouble;
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
  double myClassBreakMax3 = myAdjustedRasterBandStats.maxValDouble;

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
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedInt, myGreenInt, myBlueInt, transparencyLevelInt));
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

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer - painting image to canvas at " 
            << paintXoffset << ", " << paintYoffset << "." << std::endl;
#endif
  
  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x()),
                         static_cast<int>(theRasterViewPort->topLeftPoint.y()),
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
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawPalettedSingleBandColor called" << std::endl;
#endif

  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );
  QgsColorTable *myColorTable = colorTable ( theBandNoInt );

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);

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
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(c1, c2, c3, transparencyLevelInt));
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

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer - painting image to canvas at " 
            << paintXoffset << ", " << paintYoffset << "." << std::endl;
#endif
  
  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x()),
                         static_cast<int>(theRasterViewPort->topLeftPoint.y()),
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
                                                QString theColorQString)
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawPalettedSingleBandGray called" << std::endl;
#endif
  RasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );
  QgsColorTable *myColorTable = &(myRasterBandStats.colorTable);

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);

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

      if (theColorQString == redTranslatedQString)
      {
        myGrayValueInt = c1;
      }
      else if (theColorQString == greenTranslatedQString)
      {
        myGrayValueInt = c2;
      }
      else if (theColorQString == blueTranslatedQString)
      {
        myGrayValueInt = c3;
      }

      if (invertHistogramFlag)
      {
        myGrayValueInt = 255 - myGrayValueInt;
      }

      //set the pixel based on the above color mappings
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myGrayValueInt, myGrayValueInt, myGrayValueInt, transparencyLevelInt));
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

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer - painting image to canvas at " 
            << paintXoffset << ", " << paintYoffset << "." << std::endl;
#endif
  
  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x()),
                         static_cast<int>(theRasterViewPort->topLeftPoint.y()),
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
                                                       QString theColorQString)
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawPalettedSingleBandPseudoColor called" << std::endl;
#endif
  RasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );
  QgsColorTable *myColorTable = &(myRasterBandStats.colorTable);

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);

  double myRangeDouble = myRasterBandStats.rangeDouble;
  int myRedInt = 0;
  int myGreenInt = 0;
  int myBlueInt = 0;

  //calculate the adjusted matrix stats - which come into affect if the user has chosen
  RasterBandStats myAdjustedRasterBandStats = getRasterBandStats(theBandNoInt);

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
  double myClassBreakMax3 = myAdjustedRasterBandStats.maxValDouble;


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

      int myInt;

      //check for alternate color mappings
      if (theColorQString == redTranslatedQString)
      {
        myInt = c1;
      }
      else if (theColorQString == ("Green"))
      {
        myInt = c2;
      }
      else if (theColorQString == ("Blue"))
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
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedInt, myGreenInt, myBlueInt, transparencyLevelInt));
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

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer - painting image to canvas at " 
            << paintXoffset << ", " << paintYoffset << "." << std::endl;
#endif
  
  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x()),
                         static_cast<int>(theRasterViewPort->topLeftPoint.y()),
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
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawPalettedMultiBandColor called" << std::endl;
#endif

  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );
  QgsColorTable *myColorTable = colorTable ( theBandNoInt );

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);

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
      if (redBandNameQString == redTranslatedQString)
        myRedValueInt = c1;
      else if (redBandNameQString == greenTranslatedQString)
        myRedValueInt = c2;
      else if (redBandNameQString == blueTranslatedQString)
        myRedValueInt = c3;

      if (greenBandNameQString == redTranslatedQString)
        myGreenValueInt = c1;
      else if (greenBandNameQString == greenTranslatedQString)
        myGreenValueInt = c2;
      else if (greenBandNameQString == blueTranslatedQString)
        myGreenValueInt = c3;

      if (blueBandNameQString == redTranslatedQString)
        myBlueValueInt = c1;
      else if (blueBandNameQString == greenTranslatedQString)
        myBlueValueInt = c2;
      else if (blueBandNameQString == blueTranslatedQString)
        myBlueValueInt = c3;

      if (invertHistogramFlag)
      {
        myRedValueInt = 255 - myRedValueInt;
        myGreenValueInt = 255 - myGreenValueInt;
        myBlueValueInt = 255 - myBlueValueInt;

      }
      //set the pixel based on the above color mappings
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedValueInt, myGreenValueInt, myBlueValueInt, transparencyLevelInt));
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

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer - painting image to canvas at " 
            << paintXoffset << ", " << paintYoffset << "." << std::endl;
#endif
  
  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x()),
                         static_cast<int>(theRasterViewPort->topLeftPoint.y()),
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
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawMultiBandColor called" << std::endl;
#endif

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

  int myRedInt, myGreenInt, myBlueInt;
  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
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

      // TODO: check all channels ?
      if ( myRedValueDouble == noDataValueDouble || myRedValueDouble != myRedValueDouble )
      {
#ifdef QGISDEBUG
        std::cout << "myRedValueDouble = " << myRedValueDouble << std::endl;
        std::cout << "noDataValueDouble = " << noDataValueDouble << std::endl;
#endif
        continue;
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
                          qRgba(myRedValueInt, myGreenValueInt, myBlueValueInt, transparencyLevelInt) );
    }
  }

  //render any inline filters
  filterLayer(&myQImage);
  
#ifdef QGISDEBUG
    QPixmap* pm = dynamic_cast<QPixmap*>(theQPainter->device());
   
    std::cout << "QgsRasterLayer::drawMultiBandColor: theQPainter stats: "
              << "width = " << pm->width() << ", "
              << "height = " << pm->height()
              << "." << std::endl; 

    pm->save("/tmp/qgis-rasterlayer-drawmultibandcolor-test-a.png", "PNG");

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

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer - painting image to canvas at " 
            << paintXoffset << ", " << paintYoffset << "." << std::endl;
#endif
  
  //part of the experimental transparency support
  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x()),
                         static_cast<int>(theRasterViewPort->topLeftPoint.y()),
                         myQImage,
                         paintXoffset,
                         paintYoffset);

#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::drawMultiBandColor: theQPainter->drawImage." << std::endl; 
    
    pm->save("/tmp/qgis-rasterlayer-drawmultibandcolor-test-b.png", "PNG");

#endif
                         
  //free the scanline memory
  CPLFree(myGdalRedData);
  CPLFree(myGdalGreenData);
  CPLFree(myGdalBlueData);
}



/**
* Call any inline filters
*/
void QgsRasterLayer::filterLayer(QImage * theQImage)
{
  //do stuff here....
  return;
}

/**
Print some debug info to the qpainter
*/

void QgsRasterLayer::showDebugOverlay(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort)
{


  QFont myQFont("arial", 10, QFont::Bold);
  theQPainter->setFont(myQFont);
  theQPainter->setPen(Qt::black);
  QBrush myQBrush(qRgba(128, 128, 164, 50), Dense6Pattern); //semi transparent
  theQPainter->setBrush(myQBrush);  // set the yellow brush
  theQPainter->drawRect(5, 5, theQPainter->window().width() - 10, 60);
  theQPainter->setBrush(NoBrush); // do not fill

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
const RasterBandStats QgsRasterLayer::getRasterBandStats(QString theBandNameQString)
{

  //we cant use a vector iterator because the iterator is astruct not a class
  //and the qvector model does not like this.
  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++)
  {
    RasterBandStats myRasterBandStats = getRasterBandStats(i);
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      return myRasterBandStats;
    }
  }

  return RasterBandStats();     // return a null one
  // XXX is this ok?  IS there a "null" one?
}

//get the number of a band given its name
//note this should be the rewritten name set up in the constructor,
//not the name retrieved directly from gdal!
//if no matching band is found zero will be returned!
const int QgsRasterLayer::getRasterBandNumber(QString theBandNameQString)
{
  for (int myIteratorInt = 0; myIteratorInt <= rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
#ifdef QGISDEBUG

    std::cout << "myRasterBandStats.bandName: " << myRasterBandStats.bandName << "  :: theBandNameQString: " << theBandNameQString << std::endl;
#endif

    if (myRasterBandStats.bandName == theBandNameQString)
    {
#ifdef QGISDEBUG
      std::cerr << "********** band " << myRasterBandStats.bandNoInt << " was found in getRasterBandNumber " << theBandNameQString << std::endl;
#endif

      return myRasterBandStats.bandNoInt;
    }
  }
#ifdef QGISDEBUG
  std::cerr << "********** no band was found in getRasterBandNumber " << theBandNameQString << std::endl;
#endif

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
const RasterBandStats QgsRasterLayer::getRasterBandStats(int theBandNoInt)
{
  // check if we have received a valid band number
  if ((gdalDataset->GetRasterCount() < theBandNoInt) && rasterLayerType != PALETTE)
  {
    // invalid band id, return nothing
    RasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }
  if (rasterLayerType == PALETTE && (theBandNoInt > 3))
  {
    // invalid band id, return nothing
    RasterBandStats myNullReturnStats;
    return myNullReturnStats;
  }
  // check if we have previously gathered stats for this band...

  RasterBandStats myRasterBandStats = rasterStatsVector[theBandNoInt - 1];
  myRasterBandStats.bandNoInt = theBandNoInt;

  // don't bother with this if we already have stats
  if (myRasterBandStats.statsGatheredFlag)
  {
    return myRasterBandStats;
  }
  // only print message if we are actually gathering the stats
  emit setStatus(QString("Retrieving stats for ")+layerName);
  qApp->processEvents();
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::retrieve stats for band " << theBandNoInt << std::endl;
#endif


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
      myRasterBandStats.bandName = redTranslatedQString;
      break;
    case 2:
      myRasterBandStats.bandName = blueTranslatedQString;
      break;
    case 3:
      myRasterBandStats.bandName = greenTranslatedQString;
      break;
    default:
      //invalid band id so return
      RasterBandStats myNullReturnStats;
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

  emit setStatus(QString("Calculating stats for ")+layerName);
  //reset the main app progress bar
  emit setProgress(0,0);

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
    std::cerr << __FILE__ << " : " << __LINE__
    << " myGdalBand->GetMinimum() failed\n";
  }

  double GDALmaximum = myGdalBand->GetMaximum( &success );

  if ( ! success )
  {
    std::cerr << __FILE__ << " : " << __LINE__
    << " myGdalBand->GetMaximum() failed\n";
  }

  double GDALnodata = myGdalBand->GetNoDataValue( &success );

  if ( ! success )
  {
    std::cerr << __FILE__ << " : " << __LINE__
    << " myGdalBand->GetNoDataValue() failed\n";
  }

  std::cerr << "GDALminium:\t" << GDALminimum << "\n";
  std::cerr << "GDALmaxium:\t" << GDALmaximum << "\n";
  std::cerr << "GDALnodata:\t" << GDALnodata  << "\n";

  double GDALrange[2];          // calculated min/max, as opposed to the
  // dataset provided

  GDALComputeRasterMinMax( myGdalBand, 0, GDALrange );

  std::cerr << "approximate computed GDALminium:\t" << GDALrange[0] << "\n";
  std::cerr << "approximate computed GDALmaxium:\t" << GDALrange[1] << "\n";

  GDALComputeRasterMinMax( myGdalBand, 1, GDALrange );

  std::cerr << "exactly computed GDALminium:\t" << GDALrange[0] << "\n";
  std::cerr << "exactly computed GDALmaxium:\t" << GDALrange[1] << "\n";


  for( int iYBlock = 0; iYBlock < myNYBlocks; iYBlock++ )
  {
    emit setProgress ( iYBlock, myNYBlocks * 2 );

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
               myDouble < GDALminimum )
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
            if (myDouble != noDataValueDouble)
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
    emit setProgress ( iYBlock+myNYBlocks, myNYBlocks * 2 );

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
               myDouble < GDALminimum )
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
  std::cout << "************ STATS **************" << std::endl;
  std::cout << "NULL   = " << noDataValueDouble << std::endl;
  std::cout << "MIN    = " << myRasterBandStats.minValDouble << std::endl;
  std::cout << "MAX    = " << myRasterBandStats.maxValDouble << std::endl;
  std::cout << "RANGE  = " << myRasterBandStats.rangeDouble << std::endl;
  std::cout << "MEAN   = " << myRasterBandStats.meanDouble << std::endl;
  std::cout << "STDDEV = " << myRasterBandStats.stdDevDouble << std::endl;
#endif

  CPLFree(myData);
  myRasterBandStats.statsGatheredFlag = true;

#ifdef QGISDEBUG
  std::cout << "adding stats to stats collection at position " << theBandNoInt - 1<< std::endl;
#endif
  //add this band to the class stats map
  rasterStatsVector[theBandNoInt - 1] = myRasterBandStats;
  emit setProgress(rasterYDimInt, rasterYDimInt); //reset progress
  QApplication::restoreOverrideCursor(); //restore the cursor

#ifdef QGISDEBUG
  std::cout << "Stats collection completed returning " << std::endl;
#endif
  return myRasterBandStats;

} // QgsRasterLayer::getRasterBandStats



//mutator for red band name (allows alternate mappings e.g. map blue as red colour)
void QgsRasterLayer::setRedBandName(QString theBandNameQString)
{
#ifdef QGISDEBUG
  std::cout << "setRedBandName :  " << theBandNameQString << std::endl;
#endif
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    redBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == redTranslatedQString || theBandNameQString == greenTranslatedQString || theBandNameQString == blueTranslatedQString))
  {
    redBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
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
void QgsRasterLayer::setGreenBandName(QString theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    greenBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == redTranslatedQString || theBandNameQString == greenTranslatedQString || theBandNameQString == blueTranslatedQString))
  {
    greenBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
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
void QgsRasterLayer::setBlueBandName(QString theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    blueBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == redTranslatedQString || theBandNameQString == greenTranslatedQString || theBandNameQString == blueTranslatedQString))
  {
    blueBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
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

//mutator for gray band name
void QgsRasterLayer::setGrayBandName(QString theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    grayBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == redTranslatedQString || theBandNameQString == greenTranslatedQString || theBandNameQString == blueTranslatedQString))
  {
    grayBandNameQString = theBandNameQString;
    return;
  }
  //otherwise check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
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
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::getLegendQPixmap called (" << getDrawingStyleAsQString() << ")" << std::endl;
#endif
  //
  // Get the adjusted matrix stats
  //
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(1);
  double noDataDouble = myGdalBand->GetNoDataValue();
  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
  QPixmap myLegendQPixmap;      //will be initialised once we know what drawing style is active
  QPainter myQPainter;
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
    double myClassBreakMax3 = myClassBreakMin3 + myBreakSizeDouble;

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


  myQPainter.end();


  // see if the caller wants the name of the layer in the pixmap (used for legend bar)
  if (theWithNameFlag)
  {
    QFont myQFont("arial", 10, QFont::Normal);
    QFontMetrics myQFontMetrics(myQFont);

    int myWidthInt = 40 + myQFontMetrics.width(this->name());
    int myHeightInt = (myQFontMetrics.height() + 10 > 35) ? myQFontMetrics.height() + 10 : 35;

    //create a matrix to
    QWMatrix myQWMatrix;
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
    //
    // Overlay a pyramid icon
    //
    if (hasPyramidsFlag)
    {
      myQPainter.drawPixmap(0,myHeightInt-mPyramidPixmap.height(),mPyramidPixmap);
    }
    else
    {
      myQPainter.drawPixmap(0,myHeightInt-mNoPyramidPixmap.height(),mNoPyramidPixmap);
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
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::getDetailedLegendQPixmap called" << std::endl;
#endif
  QFont myQFont("arial", 10, QFont::Normal);
  QFontMetrics myQFontMetrics(myQFont);

  int myFontHeight = (myQFontMetrics.height() );
  const int myInterLabelSpacing = 5;
  int myImageHeightInt = ((myFontHeight + (myInterLabelSpacing*2)) * theLabelCountInt);
  int myLongestLabelWidthInt =  myQFontMetrics.width(this->name());
  const int myHorizontalLabelSpacing = 5;
  int myImageWidthInt = (myHorizontalLabelSpacing*2) + myLongestLabelWidthInt;
  const int myColourBarWidthInt = 10;
  //
  // Get the adjusted matrix stats
  //
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(1);
  double noDataDouble = myGdalBand->GetNoDataValue();
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
    double myClassBreakMax3 = myClassBreakMin3 + myBreakSizeDouble;

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
  QWMatrix myQWMatrix;
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

//similar to above but returns a pointer. Implemented for qgsmaplayer interface
QPixmap *QgsRasterLayer::legendPixmap()
{
  m_legendPixmap=getLegendQPixmap(true);
  //m_legendPixmap=getDetailedLegendQPixmap();
  return &m_legendPixmap;
}


// Useful for Provider mode

QStringList QgsRasterLayer::subLayers()
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

void QgsRasterLayer::setLayerOrder(QStringList layers)
{

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::setLayerOrder: Entered." << std::endl;
#endif


  if (dataProvider)
  {
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::setLayerOrder: About to dataProvider->setLayerOrder(layers)." << std::endl;
#endif
    dataProvider->setLayerOrder(layers);
  }

}

// Useful for Provider mode

void QgsRasterLayer::setSubLayerVisibility(QString name, bool vis)
{
  
  if (dataProvider)
  {
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::setSubLayerVisibility: About to dataProvider->setSubLayerVisibility(name, vis)." << std::endl;
#endif
    dataProvider->setSubLayerVisibility(name, vis);
  }

}


/** Accessor for the superclass popmenu var*/
QPopupMenu *QgsRasterLayer::contextMenu()
{
  return popMenu;
}

void QgsRasterLayer::initContextMenu_(QgisApp * theApp)
{
  popMenu->setCheckable ( true );

  myPopupLabel->setText( tr("<center><b>Raster Layer</b></center>") );

  QLabel * myTransparencyLabel = new QLabel( popMenu );

  myTransparencyLabel->setFrameStyle( QFrame::Panel | QFrame::Raised );
  myTransparencyLabel->setText( tr("<center><b>Transparency</b></center>") );

  popMenu->insertItem(myTransparencyLabel);

  // XXX why GUI element here?
  mTransparencySlider = new QSlider(0,255,5,0,QSlider::Horizontal,popMenu);
  mTransparencySlider->setTickmarks(QSlider::Both);
  mTransparencySlider->setTickInterval(25);
  mTransparencySlider->setTracking(false); //stop slider emmitting a signal until mouse released

  connect(mTransparencySlider, SIGNAL(valueChanged(int)), this, SLOT(popupTransparencySliderMoved(int)));

  popMenu->insertItem(mTransparencySlider);

} // QgsRasterLayer::initContextMenu



void QgsRasterLayer::popupTransparencySliderMoved(int theInt)
{
#ifdef QGISDEBUG
  std::cout << "popupTransparencySliderMoved called with : " << theInt << std::endl;
#endif
  if (theInt > 255)
  {
    transparencyLevelInt = 255;
  }
  else if (theInt < 0)
  {
    transparencyLevelInt = 0;
  }
  else
  {
    transparencyLevelInt = 255-theInt;
  }
  triggerRepaint();
}


//should be between 0 and 255
void QgsRasterLayer::setTransparency(int theInt)
{
#ifdef QGISDEBUG
  std::cout << "Set transparency called with : " << theInt << std::endl;
#endif
  // XXX bad to have GUI elements in this class mTransparencySlider->setValue(255-theInt);
  //delegate rest to transparency slider
  if ( mTransparencySlider )
  {
    mTransparencySlider->setValue(255-theInt);
  }
} //  QgsRasterLayer::setTransparency(int theInt)



unsigned int QgsRasterLayer::getTransparency()
{
  return transparencyLevelInt;
}

QString QgsRasterLayer::getMetadata()
{
  QString myMetadataQString = "<html><body>";
  myMetadataQString += "<table width=\"100%\">";
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Driver:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += QString(gdalDataset->GetDriver()->GetDescription());
  myMetadataQString += "<br>";
  myMetadataQString += QString(gdalDataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME));
  myMetadataQString += "</td></tr>";

  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Dimensions:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += tr("X: ") + QString::number(gdalDataset->GetRasterXSize()) +
                       tr(" Y: ") + QString::number(gdalDataset->GetRasterYSize()) + tr(" Bands: ") + QString::number(gdalDataset->GetRasterCount());
  myMetadataQString += "</td></tr>";

  //just use the first band
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(1);

  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Data Type:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";
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

  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Pyramid overviews:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";

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

  //layer coordinate system
  if (gdalDataset->GetProjectionRef() != NULL)
  {
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Layer Spatial Reference System: ");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    //note we inject some white space so the projection wraps better in the <td>
    QString myProjectionQString = QString(gdalDataset->GetProjectionRef());
    myProjectionQString = myProjectionQString.replace(QRegExp("\"")," \"");
    myMetadataQString += myProjectionQString ;
    myMetadataQString += "</td></tr>";
  }
  // output coordinate system
  QString myDestWKT = QgsProject::instance()->readEntry("SpatialRefSys","/WKT","");
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Project Spatial Reference System: ");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  //note we inject some white space so the projection wraps better in the <td>
  myDestWKT = myDestWKT.replace(QRegExp("\"")," \"");
  myMetadataQString += myDestWKT ;
  myMetadataQString += "</td></tr>";

  if (gdalDataset->GetGeoTransform(adfGeoTransform) == CE_None)
  {
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Origin:");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += QString::number(adfGeoTransform[0]);
    myMetadataQString += ",";
    myMetadataQString += QString::number(adfGeoTransform[3]);
    myMetadataQString += "</td></tr>";

    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Pixel Size:");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += QString::number(adfGeoTransform[1]);
    myMetadataQString += ",";
    myMetadataQString += QString::number(adfGeoTransform[5]);
    myMetadataQString += "</td></tr>";
  }
  else
  {
    adfGeoTransform[5] = -1;
  }
  //
  // Add the stats for each band to the output table
  //
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Band Statistics (if gathered):");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";

  // Start a nested table in this trow
  myMetadataQString += "<table width=\"100%\">";
  myMetadataQString += "<tr><th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Property") + "</font>";
  myMetadataQString += "</th>";
  myMetadataQString += "<th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Value") + "</font>";
  myMetadataQString += "</th><tr>";

  int myRowInt = 0;
  int myBandCountInt = getBandCount();
  for (int myIteratorInt = 1; myIteratorInt <= myBandCountInt; ++myIteratorInt)
  {
#ifdef QGISDEBUG
    std::cout << "Raster properties : checking if band " << myIteratorInt << " has stats? ";
#endif
    //band name
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Band");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += getRasterBandName(myIteratorInt);
    myMetadataQString += "</td></tr>";
    //band number
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += tr("Band No");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"white\">";
    myMetadataQString += QString::number(myIteratorInt);
    myMetadataQString += "</td></tr>";

    //check if full stats for this layer have already been collected
    if (!hasStats(myIteratorInt))  //not collected
    {
#ifdef QGISDEBUG
      std::cout << ".....no" << std::endl;
#endif

      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("No Stats");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += tr("No stats collected yet");
      myMetadataQString += "</td></tr>";
    }
    else                    // collected - show full detail
    {
#ifdef QGISDEBUG
      std::cout << ".....yes" << std::endl;
#endif

      RasterBandStats myRasterBandStats = getRasterBandStats(myIteratorInt);
      //Min Val
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Min Val");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.minValDouble, 'f',10);
      myMetadataQString += "</td></tr>";

      // Max Val
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Max Val");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.maxValDouble, 'f',10);
      myMetadataQString += "</td></tr>";

      // Range
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Range");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.rangeDouble, 'f',10);
      myMetadataQString += "</td></tr>";

      // Mean
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Mean");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.meanDouble, 'f',10);
      myMetadataQString += "</td></tr>";

      //sum of squares
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Sum of squares");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.sumSqrDevDouble, 'f',10);
      myMetadataQString += "</td></tr>";

      //standard deviation
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Standard Deviation");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.stdDevDouble, 'f',10);
      myMetadataQString += "</td></tr>";

      //sum of all cells
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Sum of all cells");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.sumDouble, 'f',10);
      myMetadataQString += "</td></tr>";

      //number of cells
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Cell Count");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.elementCountInt);
      myMetadataQString += "</td></tr>";
    }
  }
  myMetadataQString += "</table>"; //end of nested table
  myMetadataQString += "</td></tr>"; //end of stats container table row


  //
  // Close the table
  //

  myMetadataQString += "</table>";
  myMetadataQString += "</body></html>";
  return myMetadataQString;
}

void QgsRasterLayer::buildPyramids(RasterPyramidList theRasterPyramidList, QString theResamplingMethod)
{
  emit setProgress(0,0);
  //first test if the file is writeable
  QFile myQFile(dataSource);
  if (!myQFile.open(IO_WriteOnly| IO_Append))
  {

    QMessageBox myMessageBox( tr("Write access denied"),
                              tr("Write access denied. Adjust the file permissions and try again.\n\n"),
                              QMessageBox::Warning,
                              QMessageBox::Ok,
                              QMessageBox::NoButton,
                              QMessageBox::NoButton );
    myMessageBox.exec();

    return;
  }
  myQFile.close();
  // let the user know we're going to possibly be taking a while
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  GDALAllRegister();
  //close the gdal dataset and reopen it in read / write mode
  delete gdalDataset;
  gdalDataset = (GDALDataset *) GDALOpen(dataSource, GA_Update);
  //
  // Iterate through the Raster Layer Pyramid Vector, building any pyramid
  // marked as exists in eaxh RasterPyramid struct.
  //
  int myCountInt=1;
  int myTotalInt=theRasterPyramidList.count();
  RasterPyramidList::iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator=theRasterPyramidList.begin();
        myRasterPyramidIterator != theRasterPyramidList.end();
        ++myRasterPyramidIterator )
  {
    std::cout << "Buld pyramids:: Level " << (*myRasterPyramidIterator).levelInt
    << "x :" << (*myRasterPyramidIterator).xDimInt
    << "y :" << (*myRasterPyramidIterator).yDimInt
    << "exists :" << (*myRasterPyramidIterator).existsFlag
    << std::endl;
    if ((*myRasterPyramidIterator).existsFlag)
    {
      std::cout << "Building....." << std::endl;
      emit setProgress(myCountInt,myTotalInt);
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
#ifdef QGISDEBUG
      //build the pyramid and show progress to console
      if(theResamplingMethod==tr("Average Magphase"))
      {
        gdalDataset->BuildOverviews( "MODE", 1, myOverviewLevelsIntArray, 0, NULL,
                                     GDALTermProgress, NULL );
      }
      else if(theResamplingMethod==tr("Average"))

      {
        gdalDataset->BuildOverviews( "AVERAGE", 1, myOverviewLevelsIntArray, 0, NULL,
                                     GDALTermProgress, NULL );
      }
      else // fall back to nearest neighbor
      {
        gdalDataset->BuildOverviews( "NEAREST", 1, myOverviewLevelsIntArray, 0, NULL,
                                     GDALTermProgress, NULL );
      }
#else
      //build the pyramid and show progress to console
      if(theResamplingMethod==tr("Average Magphase"))
      {
        gdalDataset->BuildOverviews( "MODE", 1, myOverviewLevelsIntArray, 0, NULL,
                                     GDALDummyProgress, NULL );
      }
      else if(theResamplingMethod==tr("Average"))

      {
        gdalDataset->BuildOverviews( "AVERAGE", 1, myOverviewLevelsIntArray, 0, NULL,
                                     GDALDummyProgress, NULL );
      }
      else // fall back to nearest neighbor
      {
        gdalDataset->BuildOverviews( "NEAREST", 1, myOverviewLevelsIntArray, 0, NULL,
                                     GDALDummyProgress, NULL );
      }
#endif
      myCountInt++;
      //make sure the raster knows it has pyramids
      hasPyramidsFlag=true;
    }

  }
  std::cout << "Pyramid overviews built" << std::endl;
  //close the gdal dataset and reopen it in read only mode
  delete gdalDataset;
  gdalDataset = (GDALDataset *) GDALOpen(dataSource, GA_ReadOnly);
  emit setProgress(0,0);
  QApplication::restoreOverrideCursor();
}

RasterPyramidList  QgsRasterLayer::buildRasterPyramidList()
{
  //
  // First we build up a list of potential pyramid layers
  //
  int myWidth=rasterXDimInt;
  int myHeight=rasterYDimInt;
  int myDivisorInt=2;
  GDALRasterBandH myGDALBand = GDALGetRasterBand( gdalDataset, 1 ); //just use the first band

  mPyramidList.clear();
  std::cout << "Building initial pyramid list" << std::endl;
  while((myWidth/myDivisorInt > 32) && ((myHeight/myDivisorInt)>32))
  {

    RasterPyramid myRasterPyramid;
    myRasterPyramid.levelInt=myDivisorInt;
    myRasterPyramid.xDimInt = (int)(0.5 + (myWidth/(double)myDivisorInt));
    myRasterPyramid.yDimInt = (int)(0.5 + (myHeight/(double)myDivisorInt));
    myRasterPyramid.existsFlag=false;
    std::cout << "Pyramid:  " << myRasterPyramid.levelInt << " "
    << myRasterPyramid.xDimInt << " "
    << myRasterPyramid.yDimInt << " "
    << std::endl;




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
        std::cout << "Checking whether " <<
        myRasterPyramid.xDimInt << " x " <<
        myRasterPyramid.yDimInt << " matches " <<
        myOverviewXDim << " x " << myOverviewYDim ;

        if ((myOverviewXDim <= (myRasterPyramid.xDimInt+ myNearMatchLimitInt)) &&
            (myOverviewXDim >= (myRasterPyramid.xDimInt- myNearMatchLimitInt)) &&
            (myOverviewYDim <= (myRasterPyramid.yDimInt+ myNearMatchLimitInt)) &&
            (myOverviewYDim >= (myRasterPyramid.yDimInt- myNearMatchLimitInt)))
        {
          //right we have a match so adjust the a / y before they get added to the list
          myRasterPyramid.xDimInt=myOverviewXDim;
          myRasterPyramid.yDimInt=myOverviewYDim;
          myRasterPyramid.existsFlag=true;
          std::cout << ".....YES!" << std::endl;
        }
        else
        {
          //no match
          std::cout << ".....no." << std::endl;
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
#ifdef QGISDEBUG
  std::cerr << "QgsRasterLayer::readColorTable()" << std::endl;
#endif

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

      if ( sscanf((char*)metadataTokens[1].ascii(),"%lf %lf %d %d %d %d %d %d",
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

  void *data = CPLMalloc ( size * viewPort->drawableAreaXDimInt * viewPort->drawableAreaYDimInt );

  CPLErr myErr = gdalBand->RasterIO ( GF_Read,
                                      viewPort->rectXOffsetInt,
                                      viewPort->rectYOffsetInt,
                                      viewPort->clippedWidthInt,
                                      viewPort->clippedHeightInt,
                                      data,
                                      viewPort->drawableAreaXDimInt,
                                      viewPort->drawableAreaYDimInt,
                                      type, 0, 0 );
                                      
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
    qWarning("Data type %d is not supported", type);
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

  snode = mnl.namedItem("transparencyLevelInt");
  myElement = snode.toElement();
  setTransparency(myElement.text().toInt());

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
  setGrayBandName(myElement.text());

  const char * sourceNameStr = source(); // debugger probe

  if ( ! readFile( source() ) )   // Data source name set in
    // QgsMapLayer::readXML()
  {
    std::cerr << __FILE__ << ":" << __LINE__
    << " unable to read from raster file "
    << source() << "\n";

    return false;
  }

  return true;

} // QgsRasterLayer::readXML_( QDomNode & layer_node )



/* virtual */ bool QgsRasterLayer::writeXML_( QDomNode & layer_node,
    QDomDocument & document )
{
  // first get the layer element so that we can append the type attribute

  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || ("maplayer" != mapLayerNode.nodeName()) )
  {
    const char * nn = mapLayerNode.nodeName(); // debugger probe

    qDebug( "QgsRasterLayer::writeXML() can't find <maplayer>" );

    return false;
  }

  mapLayerNode.setAttribute( "type", "raster" );

  // <rasterproperties>
  QDomElement rasterPropertiesElement = document.createElement( "rasterproperties" );
  mapLayerNode.appendChild( rasterPropertiesElement );

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


  // <transparencyLevelInt>
  QDomElement transparencyLevelIntElement = document.createElement( "transparencyLevelInt" );
  QDomText    transparencyLevelIntText    = document.createTextNode( QString::number(getTransparency()) );

  transparencyLevelIntElement.appendChild( transparencyLevelIntText );

  rasterPropertiesElement.appendChild( transparencyLevelIntElement );


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



/** we wouldn't have to do this if slots were inherited
 
XXX Actually this <I>should</I> be inherited.
*/
void QgsRasterLayer::inOverview( bool b )
{
  QgsMapLayer::inOverview( b );
} // QgsRasterLayer::inOverview( bool )

void QgsRasterLayer::identify(QgsRect * r)
{
  if( !mIdentifyResults)
  {

    // TODO it is necessary to pass topLevelWidget()as parent, but there is no QWidget availabl
    QWidgetList *list = QApplication::topLevelWidgets ();
    QWidgetListIt it( *list );
    QWidget *w;
    QWidget *top = 0;
    while ( (w=it.current()) != 0 )
    {
      ++it;
      if ( typeid(*w) == typeid(QgisApp) )
      {
        top = w;
        break;
      }
    }
    delete list;
    QgsAttributeAction aa;
    mIdentifyResults = new QgsIdentifyResults(aa, top);
    mIdentifyResults->restorePosition();
  }
  else
  {
    mIdentifyResults->clear();
  }

  mIdentifyResults->setTitle( name() );
  mIdentifyResults->setColumnText ( 0, tr("Band") );

  double x = ( r->xMin() + r->xMax() ) / 2;
  double y = ( r->yMin() + r->yMax() ) / 2;

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::identify: " << x << ", " << y << std::endl;
#endif

  if ( x < layerExtent.xMin() || x > layerExtent.xMax() || y < layerExtent.yMin() || y > layerExtent.yMax() )
  {
    // Outside the raster
    for ( int i = 1; i <= gdalDataset->GetRasterCount(); i++ )
    {
      mIdentifyResults->addAttribute ( tr("Band") + QString::number(i), tr("out of extent") );
    }
  }
  else
  {
    /* Calculate the row / column where the point falls */
    double xres = (layerExtent.xMax() - layerExtent.xMin()) / rasterXDimInt;
    double yres = (layerExtent.yMax() - layerExtent.yMin()) / rasterYDimInt;

    // Offset, not the cell index -> flor
    int col = (int) floor ( (x - layerExtent.xMin()) / xres );
    int row = (int) floor ( (layerExtent.yMax() - y) / yres );

#ifdef QGISDEBUG
    std::cout << "row = " << row << " col = " << col << std::endl;
#endif

    for ( int i = 1; i <= gdalDataset->GetRasterCount(); i++ )
    {
      GDALRasterBand *gdalBand = gdalDataset->GetRasterBand(i);
      GDALDataType type = gdalBand->GetRasterDataType();
      int size = GDALGetDataTypeSize ( type ) / 8;
      void *data = CPLMalloc ( size );

      CPLErr err = gdalBand->RasterIO ( GF_Read, col, row, 1, 1, data, 1, 1, type, 0, 0 );

      double value = readValue ( data, type, 0 );
#ifdef QGISDEBUG
      std::cout << "value = " << value << std::endl;
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
      mIdentifyResults->addAttribute ( tr("Band") + QString::number(i), v );

      free (data);
    }
  }

  mIdentifyResults->showAllAttributes();
  mIdentifyResults->show();

} // void QgsRasterLayer::identify(QgsRect * r)


void QgsRasterLayer::populateHistogram(int theBandNoInt, int theBinCountInt,bool theIgnoreOutOfRangeFlag,bool theHistogramEstimatedFlag)
{

  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  RasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
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
    int myHistogramArray[theBinCountInt];


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
#ifdef QGISDEBUG
      std::cout << "Added " <<  myHistogramArray[myBin] << " to histogram vector" << std::endl;
#endif
    }

  }
#ifdef QGISDEBUG
  std::cout << ">>>>>>>>>>> Histogram vector now contains " <<  myRasterBandStats.histogramVector->size() << " elements" << std::endl;
#endif
}


/*
 * 
 * New functions that will convert this class to a data provider interface
 * (B Morley)
 *
 */ 
 
 
 
QgsRasterLayer::QgsRasterLayer(
                               int dummy,
                               QString rasterLayerPath,
                               QString baseName,
                               QString providerKey,
                               QStringList layers
                               )
    : QgsMapLayer(RASTER, baseName, rasterLayerPath),
    providerKey(providerKey),
    mEditable(false),
    mModified(false),
    invertHistogramFlag(false),
    stdDevsToPlotDouble(0),
    transparencyLevelInt(255), // 0 is completely transparent
    showDebugOverlayFlag(false),
    mLayerProperties(0x0),
    mTransparencySlider(0x0),
    mIdentifyResults(0),
    dataProvider(0)
{

#ifdef QGISDEBUG
      std::cout << "QgsRasterLayer::QgsRasterLayer(4 arguments): starting." <<
                  " with layer list of " << layers.join(", ") <<
                  std::endl;
#endif

  // if we're given a provider type, try to create and bind one to this layer
  if ( ! providerKey.isEmpty() )
  {
    setDataProvider( providerKey, layers );
  }

  // Default for the popup menu
  // TODO: popMenu = 0;

  // Get the update threshold from user settings. We
  // do this only on construction to avoid the penality of
  // fetching this each time the layer is drawn. If the user
  // changes the threshold from the preferences dialog, it will
  // have no effect on existing layers
  // TODO: QSettings settings;
  // updateThreshold = settings.readNumEntry("qgis/map/updateThreshold", 1000);
  
  
  // TODO: Connect signals from the dataprovider to the qgisapp
  
  // Do a passthrough for the status bar text
  connect(
          dataProvider, SIGNAL(setStatus        (QString)),
          this,           SLOT(showStatusMessage(QString))
         );
  


#ifdef QGISDEBUG
      std::cout << "QgsRasterLayer::QgsRasterLayer(4 arguments): exiting." << std::endl;
#endif
  
  emit setStatus("QgsRasterLayer created");
} // QgsRasterLayer ctor


 
// typedef for the QgsDataProvider class factory
typedef QgsDataProvider * create_it(const char * uri);
 

/** Copied from QgsVectorLayer::setDataProvider 
 *  TODO: Make it work in the raster environment
 */
void QgsRasterLayer::setDataProvider( QString const & provider, QStringList layers )
{
  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  providerKey = provider;     // XXX is this necessary?  Usually already set
  // XXX when execution gets here.

  // load the plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();
  QString ogrlib = pReg->library(provider);

  //QString ogrlib = libDir + "/libpostgresprovider.so";

  const char *cOgrLib = (const char *) ogrlib;

#ifdef TESTPROVIDERLIB
  // test code to help debug provider loading problems
  //  void *handle = dlopen(cOgrLib, RTLD_LAZY);
  void *handle = dlopen(cOgrLib, RTLD_LAZY | RTLD_GLOBAL);
  if (!handle)
  {
    std::cout << "Error in dlopen: " << dlerror() << std::endl;

  }
  else
  {
    std::cout << "dlopen suceeded" << std::endl;
    dlclose(handle);
  }

#endif

  // load the data provider
  myLib = new QLibrary((const char *) ogrlib);
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::setDataProvider: Library name is " << myLib->library() << std::endl;
#endif
  bool loaded = myLib->load();

  if (loaded)
  {
#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::setDataProvider: Loaded data provider library" << std::endl;
    std::cout << "QgsRasterLayer::setDataProvider: Attempting to resolve the classFactory function" << std::endl;
#endif
    create_it * classFactory = (create_it *) myLib->resolve("classFactory");

    valid = false;            // assume the layer is invalid until we
    // determine otherwise
    if (classFactory)
    {
#ifdef QGISDEBUG
      std::cout << "QgsRasterLayer::setDataProvider: Getting pointer to a dataProvider object from the library\n";
#endif
      //XXX - This was a dynamic cast but that kills the Windows
      //      version big-time with an abnormal termination error
      dataProvider = (QgsRasterDataProvider*)(classFactory((const
                                              char*)(dataSource.utf8())));

      if (dataProvider)
      {
#ifdef QGISDEBUG
        std::cout << "QgsRasterLayer::setDataProvider: Instantiated the data provider plugin" <<
                  " with layer list of " << layers.join(", ") << std::endl;
#endif
        if (dataProvider->isValid())
        {
          valid = true;
          
          dataProvider->addLayers(layers);
          
          // get the extent
          QgsRect *mbr = dataProvider->extent();

          // show the extent
          QString s = mbr->stringRep();
#ifdef QGISDEBUG
          std::cout << "QgsRasterLayer::setDataProvider: Extent of layer: " << s << std::endl;
#endif
          // store the extent
          layerExtent.setXmax(mbr->xMax());
          layerExtent.setXmin(mbr->xMin());
          layerExtent.setYmax(mbr->yMax());
          layerExtent.setYmin(mbr->yMin());

          // upper case the first letter of the layer name
          layerName = layerName.left(1).upper() + layerName.mid(1);
#ifdef QGISDEBUG
          std::cout << "QgsRasterLayer::setDataProvider: layerName: " << layerName << std::endl;
#endif


          //
          // Get the layers project info and set up the QgsCoordinateTransform for this layer
          //
          QString mySourceWKT = getProjectionWKT();
          //get the project projection, defaulting to this layer's projection
          //if none exists....
#ifdef QGISDEBUG
          std::cout << "QgsRasterLayer::setDataProvider: mySourceWKT: " << mySourceWKT << std::endl;
#endif
          QString myDestWKT = QgsProject::instance()->readEntry("SpatialRefSys","/WKT",mySourceWKT);
          //set up the coordinat transform - in the case of raster this is mainly used to convert
          //the inverese projection of the map extents of the canvas when zzooming in etc. so
          //that they match the coordinate system of this layer
#ifdef QGISDEBUG
          std::cout << "QgsRasterLayer::setDataProvider: mySourceWKT: " << myDestWKT << std::endl;
#endif
          mCoordinateTransform = new QgsCoordinateTransform(mySourceWKT,myDestWKT);
        }
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "QgsRasterLayer::setDataProvider: Unable to instantiate the data provider plugin\n";
#endif
        valid = false;
      }
    }
  }
  else
  {
    valid = false;
#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::setDataProvider: Failed to load " << "../providers/libproviders.so" << "\n";
#endif

  }
  
#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::setDataProvider: exiting." << "\n";
#endif

} // QgsRasterLayer::setDataProvider


void QgsRasterLayer::showStatusMessage(QString theMessage)
{
#ifdef QGISDEBUG
//  std::cout << "QgsRasterLayer::showStatusMessage: entered with '" << theMessage << "'." << std::endl;
#endif
    // Pass-through
    // TODO: See if we can connect signal-to-signal.  This is a kludge according to the Qt doc.
    emit setStatus(theMessage);
}



// ENDS
