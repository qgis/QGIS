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
#include <QtTest/QtTest>
#include <QColor>
#include <QPainter>

class TestQgsComposerPicture : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerPicture();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void pictureRotation(); //test if picture pictureRotation is functioning
    void pictureItemRotation(); //test if composer picture item rotation is functioning
    //void oldPictureRotationApi(); //test if old deprectated composer picture rotation api is functioning

    void pictureResizeZoom();
    void pictureResizeStretch();
    void pictureResizeClip();
    void pictureResizeZoomAndResize();
    void pictureResizeFrameToImage();

    void pictureClipAnchor();
    void pictureClipAnchorOversize();
    void pictureZoomAnchor();

    void pictureSvgZoom();
    void pictureSvgStretch();
    void pictureSvgZoomAndResize();
    void pictureSvgFrameToImage();

    void pictureExpression();
    void pictureInvalidExpression();

  private:
    QgsComposition* mComposition;
    QgsComposerPicture* mComposerPicture;
    QgsMapSettings *mMapSettings;
    QString mReport;
    QString mPngImage;
    QString mSvgImage;
};

TestQgsComposerPicture::TestQgsComposerPicture()
    : mComposition( 0 )
    , mComposerPicture( 0 )
    , mMapSettings( 0 )
{

}

void TestQgsComposerPicture::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();

  mPngImage = QString( TEST_DATA_DIR ) + "/sample_image.png";
  mSvgImage = QString( TEST_DATA_DIR ) + "/sample_svg.svg";

  mComposition = new QgsComposition( *mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mComposerPicture = new QgsComposerPicture( mComposition );
  mComposerPicture->setPicturePath( mPngImage );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
  mComposerPicture->setFrameEnabled( true );

  mReport = "<h1>Composer Picture Tests</h1>\n";
}

void TestQgsComposerPicture::cleanupTestCase()
{
  delete mComposerPicture;
  delete mComposition;
  delete mMapSettings;

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
  QgsApplication::exitQgis();
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
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setPictureRotation( 0 );
}

void TestQgsComposerPicture::pictureItemRotation()
{
  //test picture item rotation
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setItemRotation( 45, true );

  QgsCompositionChecker checker( "composerpicture_itemrotation", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

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

void TestQgsComposerPicture::pictureResizeZoom()
{
  //test picture resize Zoom mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );

  QgsCompositionChecker checker( "composerpicture_resize_zoom", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
}

void TestQgsComposerPicture::pictureResizeStretch()
{
  //test picture resize Stretch mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Stretch );

  QgsCompositionChecker checker( "composerpicture_resize_stretch", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
}

void TestQgsComposerPicture::pictureResizeClip()
{
  //test picture resize Clip mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Clip );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 30, 50 ) );

  QgsCompositionChecker checker( "composerpicture_resize_clip", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsComposerPicture::pictureResizeZoomAndResize()
{
  //test picture resize ZoomResizeFrame mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::ZoomResizeFrame );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 50, 300 ) );

  QgsCompositionChecker checker( "composerpicture_resize_zoomresize", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsComposerPicture::pictureResizeFrameToImage()
{
  //test picture resize FrameToImageSize mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::FrameToImageSize );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 50, 300 ) );

  QgsCompositionChecker checker( "composerpicture_resize_frametoimage", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsComposerPicture::pictureClipAnchor()
{
  //test picture anchor in Clip mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Clip );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 30, 50 ) );
  mComposerPicture->setPictureAnchor( QgsComposerItem::LowerRight );

  QgsCompositionChecker checker( "composerpicture_clip_anchor", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setPictureAnchor( QgsComposerItem::UpperLeft );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsComposerPicture::pictureClipAnchorOversize()
{
  //test picture anchor in Clip mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Clip );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 150, 120 ) );
  mComposerPicture->setPictureAnchor( QgsComposerItem::LowerMiddle );

  QgsCompositionChecker checker( "composerpicture_clip_anchoroversize", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setPictureAnchor( QgsComposerItem::UpperLeft );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsComposerPicture::pictureZoomAnchor()
{
  //test picture anchor in Zoom mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setSceneRect( QRectF( 70, 10, 30, 100 ) );
  mComposerPicture->setPictureAnchor( QgsComposerItem::LowerMiddle );

  QgsCompositionChecker checker( "composerpicture_zoom_anchor", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setPictureAnchor( QgsComposerItem::UpperLeft );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsComposerPicture::pictureSvgZoom()
{
  //test picture resize Zoom mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setPicturePath( mSvgImage );

  QgsCompositionChecker checker( "composerpicture_svg_zoom", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setPicturePath( mPngImage );
}

void TestQgsComposerPicture::pictureSvgStretch()
{
  //test picture resize Stretch mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Stretch );
  mComposerPicture->setPicturePath( mSvgImage );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 20, 100 ) );

  QgsCompositionChecker checker( "composerpicture_svg_stretch", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setPicturePath( mPngImage );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsComposerPicture::pictureSvgZoomAndResize()
{
  //test picture resize ZoomResizeFrame mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::ZoomResizeFrame );
  mComposerPicture->setPicturePath( mSvgImage );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 50, 300 ) );

  QgsCompositionChecker checker( "composerpicture_svg_zoomresize", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
  mComposerPicture->setPicturePath( mPngImage );
}

void TestQgsComposerPicture::pictureSvgFrameToImage()
{
  //test picture resize FrameToImageSize mode
  mComposition->addComposerPicture( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::FrameToImageSize );
  mComposerPicture->setPicturePath( mSvgImage );

  QgsCompositionChecker checker( "composerpicture_svg_frametoimage", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setResizeMode( QgsComposerPicture::Zoom );
  mComposerPicture->setSceneRect( QRectF( 70, 70, 100, 100 ) );
  mComposerPicture->setPicturePath( mPngImage );
}

void TestQgsComposerPicture::pictureExpression()
{
  //test picture source via expression
  mComposition->addComposerPicture( mComposerPicture );

  QString expr = QString( "'%1' || '/sample_svg.svg'" ).arg( TEST_DATA_DIR );
  mComposerPicture->setPictureExpression( expr );
  mComposerPicture->setUsePictureExpression( true );

  QgsCompositionChecker checker( "composerpicture_expression", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setUsePictureExpression( false );
}

void TestQgsComposerPicture::pictureInvalidExpression()
{
  //test picture source via bad expression
  mComposition->addComposerPicture( mComposerPicture );

  QString expr = QString( "bad expression" );
  mComposerPicture->setPictureExpression( expr );
  mComposerPicture->setUsePictureExpression( true );

  QgsCompositionChecker checker( "composerpicture_badexpression", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );

  mComposition->removeItem( mComposerPicture );
  mComposerPicture->setUsePictureExpression( false );
}

QTEST_MAIN( TestQgsComposerPicture )
#include "testqgscomposerpicture.moc"
