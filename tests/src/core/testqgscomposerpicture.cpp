/***************************************************************************
                         testqgscomposerpicture.cpp
                         ----------------------
    begin                : April 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include "qgscomposerpicture.h"
#include <QObject>
#include <QtTest>
#include <QColor>
#include <QPainter>

class TestQgsComposerPicture: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void pictureRotation(); //test if picture pictureRotation is functioning
    void pictureItemRotation(); //test if composer picture item rotation is functioning
    //void oldPictureRotationApi(); //test if old deprectated composer picture rotation api is functioning

  private:
    QgsComposition* mComposition;
    QgsComposerPicture* mComposerPicture;
    QgsMapSettings mMapSettings;
    QString mReport;
};

void TestQgsComposerPicture::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mComposerPicture = new QgsComposerPicture( mComposition );
  mComposerPicture->setPictureFile( QString( TEST_DATA_DIR ) + QDir::separator() +  "sample_image.png" );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
  mComposerPicture->setFrameEnabled( true );

  mReport = "<h1>Composer Picture Tests</h1>\n";
}

void TestQgsComposerPicture::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerPicture::init()
{

}

void TestQgsComposerPicture::cleanup()
{

}

void TestQgsComposerPicture::pictureRotation()
{
  //test picture rotation
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setPictureRotation( 45 );

  QgsCompositionChecker checker( "composerpicture_rotation", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 100 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setPictureRotation( 0 );
}

void TestQgsComposerPicture::pictureItemRotation()
{
  //test picture item rotation
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setItemRotation( 45, true );

  QgsCompositionChecker checker( "composerpicture_itemrotation", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 100 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setItemRotation( 0, true );
}

#if 0
void TestQgsComposerPicture::oldPictureRotationApi()
{
  //test old style deprecated rotation api - remove test after 2.0 series
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setRotation( 45 );

  QgsCompositionChecker checker( "composerpicture_rotation_oldapi", mComposition );
  QVERIFY( checker.testComposition( mReport ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setRotation( 0 );
}
#endif

QTEST_MAIN( TestQgsComposerPicture )
#include "moc_testqgscomposerpicture.cxx"
