/***************************************************************************
                            qgsprojectproperties.cpp
       Set various project properties (inherits qgsprojectpropertiesbase)
                              -------------------
  begin                : May 18, 2004
  copyright            : (C) 2004 by Gary E.Sherman
  email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsprojectproperties.h"

//qgis includes
#include "qgsconfig.h"
#include "qgsproject.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrenderer.h"

//qt includes
#include <qapplication.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qtextedit.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qstring.h>
#include <qspinbox.h>
#include <qcolor.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qlistview.h>


//stdc++ includes
#include <iostream>
#include <cstdlib>

    const QString GEOWKT =    "GEOGCS[\"WGS 84\", "
    "  DATUM[\"WGS_1984\", "
    "    SPHEROID[\"WGS 84\",6378137,298.257223563, "
    "      AUTHORITY[\"EPSG\",7030]], "
    "    TOWGS84[0,0,0,0,0,0,0], "
    "    AUTHORITY[\"EPSG\",6326]], "
    "  PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",8901]], "
    "  UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",9108]], "
    "  AXIS[\"Lat\",NORTH], "
    "  AXIS[\"Long\",EAST], "
    "  AUTHORITY[\"EPSG\",4326]]";
    
QgsProjectProperties::QgsProjectProperties(QWidget *parent, const char *name)
    : QgsProjectPropertiesBase(parent, name)
{

    //for now we are disabling the use of projection related widgets until they are
    //ready for production use
    cbxProjectionEnabled->setEnabled(false);
    lstCoordinateSystems->setEnabled(false);
    teProjection->setEnabled(false);
    
    //    out with the old
    //    QgsProject::instance()->mapUnits( QgsScaleCalculator::METERS );
    //    in with the new...
    QgsScaleCalculator::units myUnit = QgsProject::instance()->mapUnits();
    setMapUnits(myUnit);
    title(QgsProject::instance()->title());
    
    //see if the user wants on the fly projection enabled
    int myProjectionEnabledFlag = 
       QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectionsEnabled",0);
    if (myProjectionEnabledFlag==0)
    {
      cbxProjectionEnabled->setChecked(false);
    }
    else
    {
      cbxProjectionEnabled->setChecked(true);
    }
    
    getProjList();
    
    //if the user changes the projection for the project, we need to 
    //fire a signal to each layer telling it to change its coordinateTransform
    //member's output projection. These connects I'm setting up should be
    //automatically cleaned up when this project props dialog closes
    std::map<QString, QgsMapLayer *> myMapLayers = QgsMapLayerRegistry::instance()->mapLayers();
    std::map<QString, QgsMapLayer *>::iterator myMapIterator;
    for ( myMapIterator = myMapLayers.begin(); myMapIterator != myMapLayers.end(); ++myMapIterator )
    {
        QgsMapLayer * myMapLayer = myMapIterator->second;
         connect(this,
                    SIGNAL(setDestWKT(QString)),
                     myMapLayer->coordinateTransform(),
                     SLOT(setDestWKT(QString)));   
    }

    // get the manner in which the number of decimal places in the mouse
    // position display is set (manual or automatic)
    bool automaticPrecision = QgsProject::instance()->readBoolEntry("PositionPrecision","/Automatic");
    if (automaticPrecision)
      btnGrpPrecision->setButton(0);
    else
      btnGrpPrecision->setButton(1);

    int dp = QgsProject::instance()->readNumEntry("PositionPrecision", "/DecimalPlaces");
    spinBoxDP->setValue(dp);

    //get the snapping tolerance for digitising and set the control accordingly
    double mySnapTolerance = QgsProject::instance()->readDoubleEntry("Digitizing","/Tolerance",0);
    //leSnappingTolerance->setInputMask("000000.000000");
    leSnappingTolerance->setText(QString::number(mySnapTolerance));

    //get the line width for digitised lines and set the control accordingly
    int myLineWidth = QgsProject::instance()->readNumEntry("Digitizing","/LineWidth",0);
    spinDigitisedLineWidth->setValue(myLineWidth);    
    
    //get the colour of digitising lines and set the button colour accordingly
    int myRedInt = QgsProject::instance()->readNumEntry("Digitizing","/LineColorRedPart",255);
    int myGreenInt = QgsProject::instance()->readNumEntry("Digitizing","/LineColorGreenPart",0);
    int myBlueInt = QgsProject::instance()->readNumEntry("Digitizing","/LineColorBluePart",0);
    QColor myColour = QColor(myRedInt,myGreenInt,myBlueInt);
    pbnDigitisedLineColour->setPaletteBackgroundColor (myColour);

    //get the colour selections and set the button colour accordingly
    myRedInt = QgsProject::instance()->readNumEntry("Gui","/SelectionColorRedPart",255);
    myGreenInt = QgsProject::instance()->readNumEntry("Gui","/SelectionColorGreenPart",255);
    myBlueInt = QgsProject::instance()->readNumEntry("Gui","/SelectionColorBluePart",0);
    myColour = QColor(myRedInt,myGreenInt,myBlueInt);
    pbnSelectionColour->setPaletteBackgroundColor (myColour);    
    

}

QgsProjectProperties::~QgsProjectProperties()
{}


QgsScaleCalculator::units QgsProjectProperties::mapUnits() const
{
  return QgsProject::instance()->mapUnits();
}


void QgsProjectProperties::mapUnitChange(int unit)
{
  /*
   QgsProject::instance()->mapUnits(
       static_cast<QgsScaleCalculator::units>(unit));
  */
}


void QgsProjectProperties::setMapUnits(QgsScaleCalculator::units unit)
{
  // select the button
  btnGrpMapUnits->setButton(static_cast<int>(unit));
  QgsProject::instance()->mapUnits(unit);
}


QString QgsProjectProperties::title() const
{
    return titleEdit->text();
} //  QgsProjectPropertires::title() const


void QgsProjectProperties::title( QString const & title )
{
    titleEdit->setText( title );
    QgsProject::instance()->title( title );
} // QgsProjectProperties::title( QString const & title )

QString QgsProjectProperties::projectionWKT()
{
  return QgsProject::instance()->readEntry("SpatialRefSys","/WKT",GEOWKT);
}  


//when user clicks apply button
void QgsProjectProperties::apply()
{
    // Set the map units
    // Note. Qt 3.2.3 and greater have a function selectedId() that
    // can be used instead of the two part technique here
    QgsProject::instance()->mapUnits(
       static_cast<QgsScaleCalculator::units>(btnGrpMapUnits->id(btnGrpMapUnits->selected())));

    // Set the project title
    QgsProject::instance()->title( title() );

#ifdef QGISDEBUG
  std::cout << "Projection changed, notifying all layers" << std::endl;
#endif      
    //tell the project if projections are to be used or not...      
    if (cbxProjectionEnabled->isChecked())
    {
      QgsProject::instance()->writeEntry("SpatialRefSys","/ProjectionsEnabled",1);
    }
    else
    {
      QgsProject::instance()->writeEntry("SpatialRefSys","/ProjectionsEnabled",0);
    }
    //notify all layers the output projection has changed
    //emit setDestWKT(mProjectionsMap[cboProjection->currentText()]);
    emit setDestWKT(mProjectionsMap[lstCoordinateSystems->currentItem()->text(0)]);
    //update the project props
    //QgsProject::instance()->writeEntry("SpatialRefSys","/WKT",mProjectionsMap[cboProjection->currentText()]);
    QgsProject::instance()->writeEntry("SpatialRefSys","/WKT",mProjectionsMap[lstCoordinateSystems->currentItem()->text(0)]);
    
    // set the mouse display precision method and the
    // number of decimal places for the manual option
    // Note. Qt 3.2.3 and greater have a function selectedId() that
    // can be used instead of the two part technique here
    if (btnGrpPrecision->id(btnGrpPrecision->selected()) == 0)
      QgsProject::instance()->writeEntry("PositionPrecision","/Automatic", true);
    else
      QgsProject::instance()->writeEntry("PositionPrecision","/Automatic", false);
    QgsProject::instance()->writeEntry("PositionPrecision","/DecimalPlaces", spinBoxDP->value());
    // Announce that we may have a new display precision setting
    emit displayPrecisionChanged();

    //set the snapping tolerance for digitising (we write as text but read will convert to a num
    QgsProject::instance()->writeEntry("Digitizing","/Tolerance",leSnappingTolerance->text());

    //set the line width for digitised lines and set the control accordingly
    QgsProject::instance()->writeEntry("Digitizing","/LineWidth",spinDigitisedLineWidth->value());
        
    //set the colour of digitising lines
    QColor myColour = pbnDigitisedLineColour->paletteBackgroundColor();
    QgsProject::instance()->writeEntry("Digitizing","/LineColorRedPart",myColour.red());
    QgsProject::instance()->writeEntry("Digitizing","/LineColorGreenPart",myColour.green());
    QgsProject::instance()->writeEntry("Digitizing","/LineColorBluePart",myColour.blue());

    //set the colour for selections
    myColour = pbnSelectionColour->paletteBackgroundColor();
    QgsProject::instance()->writeEntry("Gui","/SelectionColorRedPart",myColour.red());
    QgsProject::instance()->writeEntry("Gui","/SelectionColorGreenPart",myColour.green());
    QgsProject::instance()->writeEntry("Gui","/SelectionColorBluePart",myColour.blue()); 
    QgsRenderer::mSelectionColor=myColour;
            
}

//when user clicks ok
void QgsProjectProperties::accept()
{
    apply();
    close();
}
void QgsProjectProperties::getProjList()
{
  //first some hard coded options in case we cant open the wkt_defs file
  mProjectionsMap["Lat/Long WGS84"] = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]";
  mProjectionsMap["Lat/Long 1924 Brazil"] =  "GEOGCS[\"1924 ellipsoid\", DATUM[\"Not_specified\", SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG\",\"7022\"]], AUTHORITY[\"EPSG","6022\"]], PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]], UNIT[\"degree\",0.0174532925199433, AUTHORITY[\"EPSG","9108\"]], AUTHORITY[\"EPSG","4022\"]]";
  //...etc

  
  
  std::cout << "Getting proj list " << std::endl;
#if defined(Q_OS_MACX) || defined(WIN32)
  QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
  QString theFileNameQString = PKGDATAPATH;
  theFileNameQString += "/resources/wkt_defs.txt";

  
  QFile myQFile( theFileNameQString );
  if ( myQFile.open( IO_ReadOnly ) ) 
  {
    //clear the existing entries in the taxon combo first
    //cboCoordinateSystem->clear();     
    //now we parse the loc file, checking each line for its taxon
    QTextStream myQTextStream( &myQFile );
    QString myCurrentLineQString;
    QStringList myQStringList;
    // setup the nodes for the list view
    geoList = new QListViewItem(lstCoordinateSystems,"Geographic Coordinate System");
    projList = new QListViewItem(lstCoordinateSystems,"Projected Coordinate System");

    // Read the QGIS-supplied CS file 
    while ( !myQTextStream.atEnd() ) 
    {
      myCurrentLineQString = myQTextStream.readLine(); // line of text excluding '\n'
#ifdef QGISDEBUG
      //generates a lot of output to stdout!
      //std::cout << " Match found:" << myCurrentLineQString.ascii() << std::endl;
#endif
      //get the user friendly name for the WKT
      QString myShortName = getWKTShortName(myCurrentLineQString);
      if (!myShortName) continue;
      mProjectionsMap[myShortName]=myCurrentLineQString;
    }
    myQFile.close();

    // Read the users custom coordinate system (CS) file
    // Get the user home dir. On Unix, this is $HOME. On Windows and MacOSX we
    // will use the application directory.
    //
    // Note that the global cs file must exist or the user file will never
    // be read.
    //
    // XXX Check to make sure this works on OSX

    // construct the path to the users custom CS file
    #if defined(WIN32) || defined(Q_OS_MACX)
    customCsFile = PKGDATAPATH + "/share/qgis/user_defined_cs.txt";
#else

    customCsFile = getenv("HOME");
    customCsFile += "/.qgis/user_defined_cs.txt";
#endif
    QFile csQFile( customCsFile );
    if ( csQFile.open( IO_ReadOnly ) ) 
    {
      QTextStream userCsTextStream( &csQFile );

      // Read the user-supplied CS file 
      while ( !userCsTextStream.atEnd() ) 
      {
        myCurrentLineQString = userCsTextStream.readLine(); // line of text excluding '\n'
        //get the user friendly name for the WKT
        QString myShortName = getWKTShortName(myCurrentLineQString);
        mProjectionsMap[myShortName]=myCurrentLineQString;
      }
      csQFile.close();
    }

    // end of processing users custom CS file

    //determine the current project projection so we can select the correct entry in the combo
    QString myProjectionName = QgsProject::instance()->readEntry("SpatialRefSys","/WKT",GEOWKT);
    QString mySelectedKey = getWKTShortName(myProjectionName);
    QListViewItem * mySelectedItem = 0;
    //make sure we dont allow duplicate entries into the combo
    //cboProjection->setDuplicatesEnabled(false);
    //no add each key to our list view
    ProjectionWKTMap::Iterator myIterator;
    QListViewItem *newItem;
    for ( myIterator = mProjectionsMap.begin(); myIterator != mProjectionsMap.end(); ++myIterator ) 
    {
      //std::cout << "Widget map has: " <<myIterator.key().ascii() << std::endl;
      //cboProjection->insertItem(myIterator.key());

      //XXX Add to the tree view
      if(myIterator.key().find("Lat/Long") > -1)
      {
        // this is a geographic coordinate system
        // Add it to the tree
        newItem = new QListViewItem(geoList, myIterator.key());
        if (myIterator.key()==mySelectedKey)
        {
          // this is the selected item -- store it for future use
          mySelectedItem = newItem;
        }
      }
      else
      {
        // coordinate system is projected...
        QListViewItem *node; // node that we will add this cs to...

        // projected coordinate systems are stored by projection type
        QStringList projectionInfo = QStringList::split(" - ", myIterator.key());
        if(projectionInfo.size() == 2)
        {
          // Get the projection name and strip white space from it so we
          // don't add empty nodes to the tree
          QString projName = projectionInfo[1].stripWhiteSpace();
          if(projName.length() == 0)
          {
            // If the projection name is blank, set the node to 
            // 0 so it won't be inserted
            node = projList;
          }
          else
          {

            // Replace the underscores with blanks
            projName = projName.replace('_', ' ');
            // Get the node for this type and add the projection to it
            // If the node doesn't exist, create it
            node = lstCoordinateSystems->findItem(projName, 0);
            if(node == 0)
            {
              // the node doesn't exist -- create it
              node = new QListViewItem(projList, projName);
            }
          }
        }
        else
        {
          // No projection type is specified so add it to the top-level
          // projection node
          //XXX This should never happen
          node = projList;
        }

        // now add the coordinate system to the appropriate node

        newItem = new QListViewItem(node, myIterator.key());
        if (myIterator.key()==mySelectedKey)
          mySelectedItem = newItem;
      }
    } //else = proj coord sys        


    /**
    //make sure all the loaded layer WKT's and the active project projection exist in the 
    //combo box too....
    std::map<QString, QgsMapLayer *> myMapLayers = QgsMapLayerRegistry::instance()->mapLayers();
    std::map<QString, QgsMapLayer *>::iterator myMapIterator;
    for ( myMapIterator = myMapLayers.begin(); myMapIterator != myMapLayers.end(); ++myMapIterator )
    {
    QgsMapLayer * myMapLayer = myMapIterator->second;
    QString myWKT = myMapLayer->getProjectionWKT();
    QString myWKTShortName = getWKTShortName(myWKT);
    //TODO add check here that CS is not already in the projections map
    //and if not append to wkt_defs file
    cboProjection->insertItem(myIterator.key());
    mProjectionsMap[myWKTShortName]=myWKT;
    }    

    //set the combo entry to the current entry for the project
    cboProjection->setCurrentText(mySelectedKey);
    */
    lstCoordinateSystems->setCurrentItem(mySelectedItem);
    lstCoordinateSystems->ensureItemVisible(mySelectedItem);
  }
  else
  {
    QMessageBox::warning( this,tr("QGIS Error"),tr("The projections file is not readable. Check you have the neccessary file permissions and try again. Only a small list of projections is currently availiable."));      
    
    ProjectionWKTMap::Iterator myIterator;
    for ( myIterator = mProjectionsMap.begin(); myIterator != mProjectionsMap.end(); ++myIterator ) 
    {
      //std::cout << "Widget map has: " <<myIterator.key().ascii() << std::endl;
      //cboProjection->insertItem(myIterator.key());
      if(myIterator.key().find("Lat/Long") > -1)
      {
        new QListViewItem(geoList, myIterator.key());
      }
      else
      {
        new QListViewItem(projList, myIterator.key());
      }

    }
  }   

}
void QgsProjectProperties::coordinateSystemSelected( QListViewItem * theItem)
{
    //set the text box to show the full proection spec
    std::cout << "Item selected : " << theItem->text(0) << std::endl;
    std::cout << "Item selected full wkt : " << mProjectionsMap[theItem->text(0)] << std::endl;
    QString myKey = mProjectionsMap[lstCoordinateSystems->currentItem()->text(0)];
    if (!myKey.isEmpty())
    { 
      QString myFullWKT = mProjectionsMap[theItem->text(0)];
      if (!myFullWKT.isEmpty())
      {
           teProjection->setText(myFullWKT);
      }
    }
}
QString QgsProjectProperties::getWKTShortName(QString theWKT)
{
    if (!theWKT) return NULL;
    if (theWKT.isEmpty()) return NULL;
    /* for example 
    PROJCS["Kertau / Singapore Grid",GEOGCS["Kertau",DATUM["Kertau",SPHEROID["Everest 1830 Modified",6377304.063,300.8017]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]],PROJECTION["Cassini_Soldner"],PARAMETER["latitude_of_origin",1.28764666666667],PARAMETER["central_meridian",103.853002222222],PARAMETER["false_easting",30000],PARAMETER["false_northing",30000],UNIT["metre",1]]
    
    We want to pull out 
    Kertau / Singapore Grid
    and
    Cassini_Soldner
    */
  OGRSpatialReference mySpatialRefSys;
  //this is really ugly but we need to get a QString to a char**
  char * mySourceCharArrayPointer = (char*) theWKT.ascii();
  
  /* Here are the possible OGR error codes :
     typedef int OGRErr;

    #define OGRERR_NONE                0
    #define OGRERR_NOT_ENOUGH_DATA     1    --> not enough data to deserialize 
    #define OGRERR_NOT_ENOUGH_MEMORY   2
    #define OGRERR_UNSUPPORTED_GEOMETRY_TYPE 3
    #define OGRERR_UNSUPPORTED_OPERATION 4
    #define OGRERR_CORRUPT_DATA        5
    #define OGRERR_FAILURE             6
    #define OGRERR_UNSUPPORTED_SRS     7 */
    
  OGRErr myInputResult = mySpatialRefSys.importFromWkt( & mySourceCharArrayPointer );
  if (myInputResult != OGRERR_NONE)
  {
    return NULL;
  }
  //std::cout << theWKT << std::endl;
  //check if the coordinate system is projected or not

  // if the spatial ref sys starts with GEOGCS, the coordinate
  // system is not projected
  QString myProjection,myDatum,myCoordinateSystem,myName;
  if(theWKT.find(QRegExp("^GEOGCS")) == 0)
  {
    myProjection = "Lat/Long";
    myCoordinateSystem = mySpatialRefSys.GetAttrValue("GEOGCS",0);
    myName = myProjection + " - " + myCoordinateSystem.replace('_', ' ');
  }  
  else
  {    
  
    myProjection = mySpatialRefSys.GetAttrValue("PROJCS",0);
    myCoordinateSystem = mySpatialRefSys.GetAttrValue("PROJECTION",0);
    myName = myProjection + " - " + myCoordinateSystem.replace('_', ' ');
  } 
  //std::cout << "Projection short name " << myName << std::endl;
  return myName; 
}
