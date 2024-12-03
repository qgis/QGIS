/***************************************************************************
                         testqgslayoutpicture.cpp
                         ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
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
#include "qgslayoutitempicture.h"
#include "qgsproject.h"
#include "qgsproperty.h"
#include "qgslayout.h"
#include "qgsfontutils.h"

#include <QObject>
#include "qgstest.h"
#include <QColor>
#include <QPainter>

class TestQgsLayoutPicture : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutPicture()
      : QgsTest( QStringLiteral( "Layout Picture Tests" ), QStringLiteral( "composer_picture" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void pictureRender();
    void pictureRaster();
    void pictureSvg();
    void pictureRotation();     //test if picture pictureRotation is functioning
    void pictureItemRotation(); //test if composer picture item rotation is functioning

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

    void svgParameters();
    void dynamicSvgParameters();
    void issue_14644();

    void pictureExpression();
    void pictureInvalidExpression();
    void valid();


  private:
    QgsLayout *mLayout = nullptr;
    QgsLayoutItemPicture *mPicture = nullptr;
    QString mPngImage;
    QString mSvgImage;
    QString mSvgParamsImage;
    QString mDynamicSvgParamsImage;
};

void TestQgsLayoutPicture::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QgsFontUtils::loadStandardTestFonts( { QStringLiteral( "Roman" ), QStringLiteral( "Bold" ) } );

  mPngImage = QStringLiteral( TEST_DATA_DIR ) + "/sample_image.png";
  mSvgImage = QStringLiteral( TEST_DATA_DIR ) + "/sample_svg.svg";
  mSvgParamsImage = QStringLiteral( TEST_DATA_DIR ) + "/svg_params.svg";
  mDynamicSvgParamsImage = QStringLiteral( TEST_DATA_DIR ) + "/svg/test_dynamic_svg.svg";

  mLayout = new QgsLayout( QgsProject::instance() );
  mLayout->initializeDefaults();

  mPicture = new QgsLayoutItemPicture( mLayout );
  mPicture->setPicturePath( mPngImage );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
  mPicture->setFrameEnabled( true );
}

void TestQgsLayoutPicture::cleanupTestCase()
{
  delete mPicture;
  delete mLayout;

  QgsApplication::exitQgis();
}

void TestQgsLayoutPicture::init()
{
}

void TestQgsLayoutPicture::cleanup()
{
}

void TestQgsLayoutPicture::pictureRender()
{
  //test picture rotation
  mLayout->addLayoutItem( mPicture );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_render" ), mLayout );

  mLayout->removeItem( mPicture );
}

void TestQgsLayoutPicture::pictureRaster()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemPicture *p = new QgsLayoutItemPicture( &l );
  p->setPicturePath( mPngImage, Qgis::PictureFormat::Raster );
  p->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
  p->setFrameEnabled( true );

  l.addLayoutItem( p );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_render" ), &l );
}

void TestQgsLayoutPicture::pictureSvg()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemPicture *p = new QgsLayoutItemPicture( &l );
  p->setResizeMode( QgsLayoutItemPicture::Zoom );
  p->setPicturePath( mSvgImage, Qgis::PictureFormat::SVG );
  p->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
  p->setFrameEnabled( true );

  l.addLayoutItem( p );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_svg_zoom" ), &l );
}

void TestQgsLayoutPicture::pictureRotation()
{
  //test picture rotation
  mLayout->addLayoutItem( mPicture );
  mPicture->setPictureRotation( 45 );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_rotation" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setPictureRotation( 0 );

  // Set picture rotation on uninitialized picture should not create an invalid size (NaN)
  std::unique_ptr<QgsLayoutItemPicture> uninitialized( new QgsLayoutItemPicture( mLayout ) );
  uninitialized->setResizeMode( QgsLayoutItemPicture::ZoomResizeFrame );
  QCOMPARE( uninitialized->sizeWithUnits().toQSizeF(), QSizeF( 0, 0 ) );
  uninitialized->setPictureRotation( 10 );
  QCOMPARE( uninitialized->sizeWithUnits().toQSizeF(), QSizeF( 0, 0 ) );
}

void TestQgsLayoutPicture::pictureItemRotation()
{
  //test picture item rotation
  mLayout->addLayoutItem( mPicture );
  mPicture->setItemRotation( 45, true );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_itemrotation" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setItemRotation( 0, true );
}

void TestQgsLayoutPicture::pictureResizeZoom()
{
  //test picture resize Zoom mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_resize_zoom" ), mLayout );

  mLayout->removeItem( mPicture );
}

void TestQgsLayoutPicture::pictureResizeStretch()
{
  //test picture resize Stretch mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Stretch );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_resize_stretch" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
}

void TestQgsLayoutPicture::pictureResizeClip()
{
  //test picture resize Clip mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Clip );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 30, 50 ) );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_resize_clip" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsLayoutPicture::pictureResizeZoomAndResize()
{
  //test picture resize ZoomResizeFrame mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::ZoomResizeFrame );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 50, 300 ) );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_resize_zoomresize" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsLayoutPicture::pictureResizeFrameToImage()
{
  //test picture resize FrameToImageSize mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::FrameToImageSize );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 50, 300 ) );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_resize_frametoimage" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsLayoutPicture::pictureClipAnchor()
{
  //test picture anchor in Clip mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Clip );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 30, 50 ) );
  mPicture->setPictureAnchor( QgsLayoutItem::LowerRight );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_clip_anchor" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->setPictureAnchor( QgsLayoutItem::UpperLeft );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsLayoutPicture::pictureClipAnchorOversize()
{
  //test picture anchor in Clip mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Clip );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 150, 120 ) );
  mPicture->setPictureAnchor( QgsLayoutItem::LowerMiddle );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_clip_anchoroversize" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->setPictureAnchor( QgsLayoutItem::UpperLeft );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsLayoutPicture::pictureZoomAnchor()
{
  //test picture anchor in Zoom mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->attemptSetSceneRect( QRectF( 70, 10, 30, 100 ) );
  mPicture->setPictureAnchor( QgsLayoutItem::LowerMiddle );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_zoom_anchor" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setPictureAnchor( QgsLayoutItem::UpperLeft );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsLayoutPicture::pictureSvgZoom()
{
  //test picture resize Zoom mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->setPicturePath( mSvgImage );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_svg_zoom" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setPicturePath( mPngImage );
}

void TestQgsLayoutPicture::pictureSvgStretch()
{
  //test picture resize Stretch mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Stretch );
  mPicture->setPicturePath( mSvgImage );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 20, 100 ) );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_svg_stretch" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->setPicturePath( mPngImage );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
}

void TestQgsLayoutPicture::pictureSvgZoomAndResize()
{
  //test picture resize ZoomResizeFrame mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::ZoomResizeFrame );
  mPicture->setPicturePath( mSvgImage );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 50, 300 ) );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_svg_zoomresize" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
  mPicture->setPicturePath( mPngImage );
}

void TestQgsLayoutPicture::pictureSvgFrameToImage()
{
  //test picture resize FrameToImageSize mode
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::FrameToImageSize );
  mPicture->setPicturePath( mSvgImage );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_svg_frametoimage" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
  mPicture->setPicturePath( mPngImage );
}

void TestQgsLayoutPicture::svgParameters()
{
  //test rendering an SVG file with parameters
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->setPicturePath( mSvgParamsImage );
  mPicture->setSvgFillColor( QColor( 30, 90, 200, 100 ) );
  mPicture->setSvgStrokeColor( QColor( 255, 45, 20, 200 ) );
  mPicture->setSvgStrokeWidth( 2.2 );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_svg_params" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
  mPicture->setPicturePath( mPngImage );
}

void TestQgsLayoutPicture::dynamicSvgParameters()
{
  //test rendering an SVG file with parameters
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->setPicturePath( mDynamicSvgParamsImage );

  QMap<QString, QgsProperty> parametersProperties;
  parametersProperties.insert( QStringLiteral( "text1" ), QgsProperty::fromExpression( QStringLiteral( "'green?'" ) ) );
  parametersProperties.insert( QStringLiteral( "text2" ), QgsProperty::fromExpression( QStringLiteral( "'supergreen'" ) ) );
  parametersProperties.insert( QStringLiteral( "align" ), QgsProperty::fromExpression( QStringLiteral( "'middle'" ) ) );

  mPicture->setSvgDynamicParameters( parametersProperties );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_svg_dynamic_params" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
  mPicture->setPicturePath( mPngImage );
}

void TestQgsLayoutPicture::issue_14644()
{
  //test rendering SVG file with text
  mLayout->addLayoutItem( mPicture );
  mPicture->setResizeMode( QgsLayoutItemPicture::Zoom );
  mPicture->setPicturePath( QStringLiteral( TEST_DATA_DIR ) + "/svg/issue_14644.svg" );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_issue_14644" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->attemptSetSceneRect( QRectF( 70, 70, 100, 100 ) );
  mPicture->setPicturePath( mPngImage );
}

void TestQgsLayoutPicture::pictureExpression()
{
  //test picture source via expression
  mLayout->addLayoutItem( mPicture );

  const QString expr = QStringLiteral( "'%1' || '/sample_svg.svg'" ).arg( TEST_DATA_DIR );
  mPicture->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::PictureSource, QgsProperty::fromExpression( expr ) );
  mPicture->refreshPicture();
  QVERIFY( !mPicture->isMissingImage() );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composerpicture_expression" ), mLayout );

  mLayout->removeItem( mPicture );
  mPicture->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::PictureSource, QgsProperty() );
}

void TestQgsLayoutPicture::pictureInvalidExpression()
{
  //test picture source via bad expression
  mLayout->addLayoutItem( mPicture );

  const QString expr = QStringLiteral( "bad expression" );
  mPicture->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::PictureSource, QgsProperty::fromExpression( expr ) );
  mPicture->refreshPicture();
  QVERIFY( mPicture->isMissingImage() );

  mPicture->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::PictureSource, QgsProperty::fromValue( QString() ) );
  mPicture->refreshPicture();
  QVERIFY( !mPicture->isMissingImage() );

  mLayout->removeItem( mPicture );
  mPicture->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::PictureSource, QgsProperty() );
}

void TestQgsLayoutPicture::valid()
{
  QgsProject p;
  QgsLayout l( &p );

  QgsLayoutItemPicture *picture = new QgsLayoutItemPicture( &l );
  l.addItem( picture );

  picture->setPicturePath( mPngImage );
  QVERIFY( !picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), mPngImage );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::Raster );
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::Unknown );

  picture->setPicturePath( QStringLiteral( "bad" ) );
  QVERIFY( picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), QStringLiteral( "bad" ) );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::Unknown );
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::Unknown );

  picture->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::PictureSource, QgsProperty::fromExpression( QStringLiteral( "'%1'" ).arg( mSvgImage ) ) );
  picture->refreshPicture();
  QVERIFY( !picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), mSvgImage );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::SVG );
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::Unknown );

  picture->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::PictureSource, QgsProperty::fromExpression( QStringLiteral( "'bad'" ) ) );
  picture->refreshPicture();
  QVERIFY( picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), QStringLiteral( "bad" ) );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::Unknown );
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::Unknown );

  // same tests with a given format

  picture->dataDefinedProperties().clear();

  picture->setPicturePath( mPngImage, Qgis::PictureFormat::Raster );
  QVERIFY( !picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), mPngImage );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::Raster );
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::Raster );

  picture->setPicturePath( mPngImage, Qgis::PictureFormat::Unknown );
  QVERIFY( !picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), mPngImage );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::Raster );
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::Unknown );

  picture->setPicturePath( QStringLiteral( "bad" ), Qgis::PictureFormat::Unknown );
  QVERIFY( picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), QStringLiteral( "bad" ) );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::Unknown );
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::Unknown );

  picture->setPicturePath( QStringLiteral( "bad" ), Qgis::PictureFormat::Raster );
  QVERIFY( picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), QStringLiteral( "bad" ) );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::Raster ); // cross image for missing image
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::Raster );

  picture->setPicturePath( QStringLiteral( "bad" ), Qgis::PictureFormat::SVG );
  picture->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::PictureSource, QgsProperty::fromExpression( QStringLiteral( "'%1'" ).arg( mSvgImage ) ) );
  picture->refreshPicture();
  QVERIFY( !picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), mSvgImage );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::SVG );
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::SVG );

  picture->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::PictureSource, QgsProperty::fromExpression( QStringLiteral( "'bad'" ) ) );
  picture->refreshPicture();
  QVERIFY( picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), QStringLiteral( "bad" ) );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::SVG ); // cross image for missing picture
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::SVG );

  picture->setPicturePath( QStringLiteral( "bad" ), Qgis::PictureFormat::Unknown );
  picture->refreshPicture();
  QVERIFY( picture->isMissingImage() );
  QCOMPARE( picture->evaluatedPath(), QStringLiteral( "bad" ) );
  QCOMPARE( picture->mode(), Qgis::PictureFormat::Unknown );
  QCOMPARE( picture->originalMode(), Qgis::PictureFormat::Unknown );
}

QGSTEST_MAIN( TestQgsLayoutPicture )
#include "testqgslayoutpicture.moc"
