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
    getProjList();
    setProjectionWKT(projectionWKT());
    
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

/** Set the projection passing only its 'friendly name'. If it doesnt exist in the 
  *  projections list ( as simple text file ) and error will occur */
bool QgsProjectProperties::setProjectionWKT(QString theName)
{
  QgsProject::instance()->writeEntry("SpatialRefSys","/WKT",GEOWKT);
}
/** Set the projection passing only its 'friendly name'. If it doesnt exist in the 
 *  projections list ( as simple text file ) it will be added to the list */
bool QgsProjectProperties::setProjectionWKT(QString theName, QString theWKT)
{
  QgsProject::instance()->writeEntry("SpatialRefSys","/WKT",theWKT);
}

//when user clicks apply button
void QgsProjectProperties::apply()
{
#ifdef QGISDEBUG
  std::cout << "Projection changed, notifying all layers" << std::endl;
#endif      
    //notify all layers the output projection has changed
    emit setDestWKT(mProjectionsMap[cboProjection->currentText()]);
    //update the project props
    QgsProject::instance()->writeEntry("SpatialRefSys","/WKT",mProjectionsMap[cboProjection->currentText()]);
    
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
    while ( !myQTextStream.atEnd() ) 
    {
      myCurrentLineQString = myQTextStream.readLine(); // line of text excluding '\n'
      if (myCurrentLineQString.left(4)!="PROJ")
      {
        QString myNextLineQString = myQTextStream.readLine(); // lthis is the actual wkt string
        if (myNextLineQString.left(4)!="PROJ") //the line shoue start with PROJ
        {
          continue;
        }
#ifdef QGISDEBUG
        //generates a lot of output to stdout!
        //std::cout << " Match found:" << myCurrentLineQString.ascii() << std::endl;
#endif
        mProjectionsMap[myCurrentLineQString]=myNextLineQString;
      }
    }
    myQFile.close();
    //no add each key to our combo
    ProjectionWKTMap::Iterator myIterator;
    for ( myIterator = mProjectionsMap.begin(); myIterator != mProjectionsMap.end(); ++myIterator ) 
    {
      //std::cout << "Widget map has: " <<myIterator.key().ascii() << std::endl;
      cboProjection->insertItem(myIterator.key());
    }
  }
  else
  {
    QMessageBox::warning( this,QString("QGIS Error"),QString("The projections file is not readable. Check you have the neccessary file permissions and try again. Only a small list of projectsion is now availiable."));      
    ProjectionWKTMap::Iterator myIterator;
    for ( myIterator = mProjectionsMap.begin(); myIterator != mProjectionsMap.end(); ++myIterator ) 
    {
      //std::cout << "Widget map has: " <<myIterator.key().ascii() << std::endl;
      cboProjection->insertItem(myIterator.key());
    }
  }   

}