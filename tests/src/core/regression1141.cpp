/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:54 AKDT 2007
    Copyright            : (C) 2007 by Tim Sutton
    Email                : tim @ linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QPainter>
#include <QSettings>
#include <QTime>
#include <iostream>

#include <QApplication>
#include <QDesktopServices>

//qgis includes...
#include <qgsvectorlayer.h> //defines QgsFieldMap 
#include <qgsvectorfilewriter.h> //logic for writing shpfiles
#include <qgsfeature.h> //we will need to pass a bunch of these for each rec
#include <qgsgeometry.h> //each feature needs a geometry
#include <qgspoint.h> //we will use point geometry
#include <qgscoordinatereferencesystem.h> //needed for creating a srs
#include <qgsapplication.h> //search path for srs.db
#include <qgsfield.h>
#include <qgis.h> //defines GEOWkt
#include <qgsproviderregistry.h>


/** \ingroup UnitTests
 * This is a regression test ticket 1141.
 * broken Polish characters support since r8592
 * https://trac.osgeo.org/qgis/ticket/1141
 *
 */
class Regression1141: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    /** This method tests that we can create a shpfile with diacriticals in its name
     *    and with fields that have diacriticals in their names*/
    void diacriticalTest();

  private:
    QString mEncoding;
    QgsVectorFileWriter::WriterError mError;
    QgsCoordinateReferenceSystem mCRS;
    QgsFieldMap mFields;
    QString mFileName;
};

void Regression1141::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath();
  QgsApplication::setPrefixPath( INSTALL_PREFIX, true );
  QgsApplication::showSettings();
  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );
  // compute our test file name:
  QString myTmpDir = QDir::tempPath() + QDir::separator() ;
  mFileName = myTmpDir +  "ąęćń.shp";
}


void Regression1141::cleanupTestCase()
{
  //
  // Runs after all tests are done
  //
}



void Regression1141::diacriticalTest()
{
  //create some objects that will be used in all tests...
  mEncoding = "UTF-8";
  QgsField myField( "ąęćń", QVariant::Int, "int", 10, 0, "Value on lon" );
  mFields.insert( 0, myField );
  mCRS = QgsCoordinateReferenceSystem( GEOWkt );

  qDebug( "Checking test dataset exists...\n%s", mFileName.toLocal8Bit().constData() );

  if ( !QFile::exists( mFileName ) )
  {
    qDebug( "Creating test dataset: " );

    QgsVectorFileWriter myWriter( mFileName,
                                  mEncoding,
                                  mFields,
                                  QGis::WKBPoint,
                                  &mCRS );

    QgsPoint myPoint = QgsPoint( 10.0, 10.0 );
    // NOTE: don't delete this pointer again -
    // ownership is passed to the feature which will
    // delete it in its dtor!
    QgsGeometry * mypPointGeometry = QgsGeometry::fromPoint( myPoint );
    QgsFeature myFeature;
    myFeature.setTypeName( "WKBPoint" );
    myFeature.setGeometry( mypPointGeometry );
    myFeature.addAttribute( 0, 10 );
    //
    // Write the feature to the filewriter
    // and check for errors
    //
    QVERIFY( myWriter.addFeature( myFeature ) );
    mError = myWriter.hasError();

    if ( mError == QgsVectorFileWriter::ErrDriverNotFound )
    {
      std::cout << "Driver not found error" << std::endl;
    }
    else if ( mError == QgsVectorFileWriter::ErrCreateDataSource )
    {
      std::cout << "Create data source error" << std::endl;
    }
    else if ( mError == QgsVectorFileWriter::ErrCreateLayer )
    {
      std::cout << "Create layer error" << std::endl;
    }

    QVERIFY( mError == QgsVectorFileWriter::NoError );
    // Now check we can delete it again ok
    QVERIFY( QgsVectorFileWriter::deleteShapeFile( mFileName ) );

  } //file exists
}


QTEST_MAIN( Regression1141 )

#include "moc_regression1141.cxx"

