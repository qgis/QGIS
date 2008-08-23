/***************************************************************************
     graticulecreator.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:06:45 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "graticulecreator.h"
#include <qgsfeature.h> //we will need to pass a bunch of these for each rec
#include <qgsgeometry.h> //each feature needs a geometry
#include <qgspoint.h> //we will use point geometry
#include <qgis.h> //defines GEOWKT
#include <qgslogger.h>

#include <QFileInfo>
#include "qgslogger.h"
GraticuleCreator::GraticuleCreator( QString theOutputFileName )
{
  QgsLogger::debug( "GraticuleCreator constructor called with " +
                    theOutputFileName +  " for output file " );
  mEncoding = "UTF-8";
  QgsField myField1( "LabelX", QVariant::Double, "Double", 10, 4, "X Position for label" );
  QgsField myField2( "LabelY", QVariant::Double, "Double", 10, 4, "Y Position for label" );
  QgsField myField3( "LblOffsetX", QVariant::Int, "Int", 5, 0, "X Offset for label" );
  QgsField myField4( "LblOffsetY", QVariant::Int, "int", 5, 0, "Y Offset for label" );
  QgsField myField5( "Label", QVariant::String, "String", 10, 0, "Label text" );
  QgsField myField6( "Row", QVariant::String, "String", 10, 0, "Row" );
  QgsField myField7( "Column", QVariant::String, "String", 10, 0, "Col" );
  QgsField myField8( "RowCol", QVariant::String, "String", 10, 0, "Row and col" );
  mFields.insert( 0, myField1 );
  mFields.insert( 1, myField2 );
  mFields.insert( 2, myField3 );
  mFields.insert( 3, myField4 );
  mFields.insert( 4, myField5 );
  mFields.insert( 5, myField6 );
  mFields.insert( 6, myField7 );
  mFields.insert( 7, myField8 );
  mCRS = QgsCoordinateReferenceSystem( GEOWKT );
  mFileName = theOutputFileName;

}
GraticuleCreator::~GraticuleCreator()
{
}

//TODO: check for rediculous intervals!
void GraticuleCreator::generatePointGraticule(
  double theXInterval,
  double theYInterval,
  double theXOrigin,
  double theYOrigin,
  double theXEndPoint,
  double theYEndPoint )
{
  //
  // Remove old copies that may be lying around
  //
  QgsVectorFileWriter::deleteShapeFile( mFileName );
  QgsVectorFileWriter myWriter( mFileName,
                                mEncoding,
                                mFields,
                                QGis::WKBPoint,
                                &mCRS );
  //
  // Order our loop so that it goes from smallest to biggest
  //
  if ( theXEndPoint < theXOrigin )
  {
    double myBuffer = theXOrigin;
    theXOrigin = theXEndPoint;
    theXEndPoint = myBuffer;
  }
  if ( theYEndPoint < theYOrigin )
  {
    double myBuffer = theYOrigin;
    theYOrigin = theYEndPoint;
    theYEndPoint = myBuffer;
  }

  int myColumn = 0;
  int myRow = 0;
  for ( double i = theXOrigin;
        i <= theXEndPoint;
        i += theXInterval )
  {
    for ( double j = theYOrigin;
          j <= theYEndPoint;
          j += theYInterval )
    {
      //
      // Create a polygon feature
      //
      QgsPolyline myPolyline;
      QgsPoint myPoint = QgsPoint( i, j );
      //
      // NOTE: dont delete this pointer again -
      // ownership is passed to the feature which will
      // delete it in its dtor!
      QgsGeometry * mypPointGeometry = QgsGeometry::fromPoint( myPoint );
      QgsFeature myFeature;
      myFeature.setTypeName( "WKBPoint" );
      myFeature.setGeometry( mypPointGeometry );
      if ( i == theXOrigin && j == theYEndPoint ) //labels for bottom right corner
      {
        myFeature.addAttribute( 0, i );//"LabelX"
        myFeature.addAttribute( 1, j );//"LabelY"
        myFeature.addAttribute( 2, -20 );//"LabelOffsetX"
        myFeature.addAttribute( 3, -20 );//"LabelOffsetY"
        myFeature.addAttribute( 4, QString::number( i ) + "," + QString::number( j ) );//"Label"
      }
      else if ( i == theXEndPoint && j == theYOrigin ) //labels for top left corner
      {
        myFeature.addAttribute( 0, i );//"LabelX"
        myFeature.addAttribute( 1, j );//"LabelY"
        myFeature.addAttribute( 2, 20 );//"LabelOffsetX"
        myFeature.addAttribute( 3, 20 );//"LabelOffsetY"
        myFeature.addAttribute( 4, QString::number( i ) + "," +  QString::number( j ) );//"Label"
      }
      if ( i == theXOrigin && j == theYOrigin ) //labels for bottom left corner
      {
        myFeature.addAttribute( 0, i );//"LabelX"
        myFeature.addAttribute( 1, j );//"LabelY"
        myFeature.addAttribute( 2, -20 );//"LabelOffsetX"
        myFeature.addAttribute( 3, -20 );//"LabelOffsetY"
        myFeature.addAttribute( 4, QString::number( i ) + "," + QString::number( j ) );//"Label"
      }
      else if ( i == theXEndPoint && j == theYEndPoint ) //labels for top right corner
      {
        myFeature.addAttribute( 0, i );//"LabelX"
        myFeature.addAttribute( 1, j );//"LabelY"
        myFeature.addAttribute( 2, 20 );//"LabelOffsetX"
        myFeature.addAttribute( 3, 20 );//"LabelOffsetY"
        myFeature.addAttribute( 4, QString::number( i ) + "," +  QString::number( j ) );//"Label"
      }
      else if ( i == theXOrigin ) //labels for left edge
      {
        myFeature.addAttribute( 0, i );//"LabelX"
        myFeature.addAttribute( 1, j );//"LabelY"
        myFeature.addAttribute( 2, -20 );//"LabelOffsetX"
        myFeature.addAttribute( 3, 0 );//"LabelOffsetY"
        myFeature.addAttribute( 4, QString::number( j ) );//"Label"
      }
      else if ( i == theXEndPoint ) //labels for right edge
      {
        myFeature.addAttribute( 0, i );//"LabelX"
        myFeature.addAttribute( 1, j );//"LabelY"
        myFeature.addAttribute( 2, 20 );//"LabelOffsetX"
        myFeature.addAttribute( 3, 0 );//"LabelOffsetY"
        myFeature.addAttribute( 4, QString::number( j ) );//"Label"
      }
      else if ( j == theYOrigin ) //labels for bottom edge
      {
        myFeature.addAttribute( 0, i );//"LabelX"
        myFeature.addAttribute( 1, j );//"LabelY"
        myFeature.addAttribute( 2, 0 );//"LabelOffsetX"
        myFeature.addAttribute( 3, -20 );//"LabelOffsetY"
        myFeature.addAttribute( 4, QString::number( i ) );//"Label"
      }
      else if ( j == theYEndPoint ) //labels for top edge
      {
        myFeature.addAttribute( 0, i );//"LabelX"
        myFeature.addAttribute( 1, j );//"LabelY"
        myFeature.addAttribute( 2, 0 );//"LabelOffsetX"
        myFeature.addAttribute( 3, 20 );//"LabelOffsetY"
        myFeature.addAttribute( 4, QString::number( i ) );//"Label"
      }
      //
      // Set column and row attributes
      //
      myFeature.addAttribute( 6, QString::number( myRow ) );
      myFeature.addAttribute( 7, QString::number( myColumn ) );
      myFeature.addAttribute( 8, QString::number( myRow ) + "," +
                              QString::number( myColumn ) );
      ++myRow;
      ++myColumn;
      //
      // Write the feature to the filewriter
      // and check for errors
      //
      myWriter.addFeature( myFeature );
      mError = myWriter.hasError();
      if ( mError == QgsVectorFileWriter::ErrDriverNotFound )
      {
        QgsDebugMsg( "Driver not found error" );
      }
      else if ( mError == QgsVectorFileWriter::ErrCreateDataSource )
      {
        QgsDebugMsg( "Create data source error" );
      }
      else if ( mError == QgsVectorFileWriter::ErrCreateLayer )
      {
        QgsDebugMsg( "Create layer error" );
      }
      if ( mError != QgsVectorFileWriter::NoError )
      {
        return;
      }
    }
  }
}

//TODO: check for rediculous intervals!
void GraticuleCreator::generatePolygonGraticule(
  double theXInterval,
  double theYInterval,
  double theXOrigin,
  double theYOrigin,
  double theXEndPoint,
  double theYEndPoint )
{
  //
  // Remove old copies that may be lying around
  //
  QgsVectorFileWriter::deleteShapeFile( mFileName );
  QgsVectorFileWriter myWriter( mFileName,
                                mEncoding,
                                mFields,
                                QGis::WKBPolygon,
                                &mCRS );
  //
  // Order our loop so that it goes from smallest to biggest
  //
  if ( theXEndPoint < theXOrigin )
  {
    double myBuffer = theXOrigin;
    theXOrigin = theXEndPoint;
    theXEndPoint = myBuffer;
  }
  if ( theYEndPoint < theYOrigin )
  {
    double myBuffer = theYOrigin;
    theYOrigin = theYEndPoint;
    theYEndPoint = myBuffer;
  }
  for ( double i = theXOrigin;
        i <= theXEndPoint;
        i += theXInterval )
  {
    for ( double j = theYOrigin;
          j <= theYEndPoint;
          j += theYInterval )
    {
      //
      // Create a polygon feature
      //
      QgsPolyline myPolyline;
      QgsPoint myPoint1 = QgsPoint( i, j );
      QgsPoint myPoint2 = QgsPoint( i + theXInterval, j );
      QgsPoint myPoint3 = QgsPoint( i + theXInterval, j + theYInterval );
      QgsPoint myPoint4 = QgsPoint( i, j + theYInterval );
      myPolyline << myPoint1 << myPoint2 << myPoint3 << myPoint4 << myPoint1;
      QgsPolygon myPolygon;
      myPolygon << myPolyline;
      //polygon: first item of the list is outer ring,
      // inner rings (if any) start from second item
      //
      // NOTE: dont delete this pointer again -
      // ownership is passed to the feature which will
      // delete it in its dtor!
      QgsGeometry * mypPolygonGeometry = QgsGeometry::fromPolygon( myPolygon );
      QgsFeature myFeature;
      myFeature.setTypeName( "WKBPolygon" );
      myFeature.setGeometry( mypPolygonGeometry );
      myFeature.addAttribute( 0, i );//"LabelX");
      myFeature.addAttribute( 1, j );//"LabelY");
      myFeature.addAttribute( 2, -20 );//"LabelOffsetX");
      myFeature.addAttribute( 3, -20 );//"LabelOffsetY");
      myFeature.addAttribute( 4, QString::number( i ) );//"Label");
      //
      // Write the feature to the filewriter
      // and check for errors
      //
      myWriter.addFeature( myFeature );
      mError = myWriter.hasError();
      if ( mError == QgsVectorFileWriter::ErrDriverNotFound )
      {
        QgsDebugMsg( "Driver not found error" );
      }
      else if ( mError == QgsVectorFileWriter::ErrCreateDataSource )
      {
        QgsDebugMsg( "Create data source error" );
      }
      else if ( mError == QgsVectorFileWriter::ErrCreateLayer )
      {
        QgsDebugMsg( "Create layer error" );
      }
      if ( mError != QgsVectorFileWriter::NoError )
      {
        return;
      }
    }
  }
}





