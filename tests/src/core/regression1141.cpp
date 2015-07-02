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
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QPainter>
#include <QTime>

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
#include <qgslogger.h>

#if defined(linux)
#include <langinfo.h>
#endif


/** \ingroup UnitTests
 * This is a regression test ticket 1141.
 * broken Polish characters support since r8592
 * http://hub.qgis.org/issues/1141
 *
 */
class Regression1141: public QObject
{
    Q_OBJECT

  public:
    Regression1141();

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
    QgsFields mFields;
    QString mFileName;
};

Regression1141::Regression1141()
    : mError( QgsVectorFileWriter::NoError )
{

}

void Regression1141::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  // compute our test file name:
  QString myTmpDir = QDir::tempPath() + "/";
  mFileName = myTmpDir +  "ąęćń.shp";
}


void Regression1141::cleanupTestCase()
{
  //
  // Runs after all tests are done
  //
  QgsApplication::exitQgis();
}



void Regression1141::diacriticalTest()
{
#if defined(linux)
  const char *cs = nl_langinfo( CODESET );
  QgsDebugMsg( QString( "CODESET:%1" ).arg( cs ? cs : "unset" ) );
  if ( !cs || strcmp( cs, "UTF-8" ) != 0 )
  {
    QSKIP( "This test requires a UTF-8 locale", SkipSingle );
    return;
  }
#endif

  //create some objects that will be used in all tests...
  mEncoding = "UTF-8";
  QgsField myField( "ąęćń", QVariant::Int, "int", 10, 0, "Value on lon" );
  mFields.append( myField );
  mCRS = QgsCoordinateReferenceSystem( GEOWKT );

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
    myFeature.setGeometry( mypPointGeometry );
    myFeature.initAttributes( 1 );
    myFeature.setAttribute( 0, 10 );
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
#include "regression1141.moc"
