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

//qt includes
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
#include <qcheckbox.h>
#include <qregexp.h>
#include <qlistview.h>

//stdc++ includes
#include <iostream>

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
   QgsProject::instance()->mapUnits(
       static_cast<QgsScaleCalculator::units>(unit));
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
    //XXX setup the nodes for the list view
    geoList = new QListViewItem(lstCoordinateSystems,"Geographic Coordinate System");
    projList = new QListViewItem(lstCoordinateSystems,"Projected Coordinate System");

    while ( !myQTextStream.atEnd() ) 
    {
      myCurrentLineQString = myQTextStream.readLine(); // line of text excluding '\n'
#ifdef QGISDEBUG
      //generates a lot of output to stdout!
      //std::cout << " Match found:" << myCurrentLineQString.ascii() << std::endl;
#endif
      //get the user friendly name for the WKT
      QString myShortName = getWKTShortName(myCurrentLineQString);
      mProjectionsMap[myShortName]=myCurrentLineQString;
    }
    myQFile.close();
    
    
    //determine the current project projection so we can select the correct entry in the combo
    QString myProjectionName = QgsProject::instance()->readEntry("SpatialRefSys","/WKT",GEOWKT);
    QString mySelectedKey = getWKTShortName(myProjectionName);
    QListViewItem * mySelectedItem = NULL;
    //make sure we dont allow duplicate entries into the combo
    //cboProjection->setDuplicatesEnabled(false);
    //no add each key to our list view
    ProjectionWKTMap::Iterator myIterator;
    for ( myIterator = mProjectionsMap.begin(); myIterator != mProjectionsMap.end(); ++myIterator ) 
    {
      //std::cout << "Widget map has: " <<myIterator.key().ascii() << std::endl;
      //cboProjection->insertItem(myIterator.key());
      
      //XXX Add to the tree view
      if(myIterator.key().find("LatLong") > -1)
      {
        if (myIterator.key()==mySelectedKey)
        {
          mySelectedItem = new QListViewItem(geoList, myIterator.key());
        }
        else
        {
          new QListViewItem(geoList, myIterator.key());
        }        
      }
      else
      {
        if (myIterator.key()==mySelectedKey)
        {
          mySelectedItem = new QListViewItem(projList, myIterator.key());
        }
        else
        {
          new QListViewItem(projList, myIterator.key());
        }        
      }
      

    }
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
  }
  else
  {
    QMessageBox::warning( this,tr("QGIS Error"),tr("The projections file is not readable. Check you have the neccessary file permissions and try again. Only a small list of projections is currently availiable."));      
    
    ProjectionWKTMap::Iterator myIterator;
    for ( myIterator = mProjectionsMap.begin(); myIterator != mProjectionsMap.end(); ++myIterator ) 
    {
      //std::cout << "Widget map has: " <<myIterator.key().ascii() << std::endl;
      //cboProjection->insertItem(myIterator.key());
      if(myIterator.key().find("LatLong") > -1)
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
    myProjection = "LatLong";
    myCoordinateSystem = mySpatialRefSys.GetAttrValue("GEOGCS",0);
    myName = myProjection + " - " + myCoordinateSystem;
  }  
  else
  {    
  
    myProjection = mySpatialRefSys.GetAttrValue("PROJCS",0);
    myCoordinateSystem = mySpatialRefSys.GetAttrValue("PROJECTION",0);
    myName = myProjection + " - " + myCoordinateSystem;
  } 
  //std::cout << "Projection short name " << myName << std::endl;
  return myName; 
}
