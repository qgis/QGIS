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

//qt includes
#include <qcombobox.h>
#include <qfile.h>
#include <qtextedit.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qmessagebox.h>

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

//when user picks a new proj in cmbo
void QgsProjectProperties::projectionChange(QString theQString)
{

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
  theFileNameQString += "resources/wkt_defs.txt";

  
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
        std::cout << " Match found:" << myCurrentLineQString.ascii() << std::endl;
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