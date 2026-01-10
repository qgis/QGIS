/***************************************************************************
     testqgs25drenderer.cpp
     --------------------------------------
    Date                 : April 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QStringList>

//qgis includes...
#include <qgsmapsettings.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgs25drenderer.h>
#include "qgslayoutitemmap.h"
#include "qgsexpressioncontextutils.h"
#include "qgslayout.h"

/**
 * \ingroup UnitTests
 * This is a unit test for 25d renderer.
 */
class TestQgs25DRenderer : public QgsTest
{
    Q_OBJECT
  public:
    TestQgs25DRenderer()
      : QgsTest( u"25D Renderer Tests"_s, u"25d_renderer"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void render();
    void renderLayout();

  private:
    QgsVectorLayer *mpPolysLayer = nullptr;
    QString mTestDataDir;
};


void TestQgs25DRenderer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  const QString myPolysFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(), myPolyFileInfo.completeBaseName(), u"ogr"_s );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlags() );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  //need a very high height to check for stacking
  QgsExpressionContextUtils::setLayerVariable( mpPolysLayer, u"qgis_25d_height"_s, 8 );
  QgsExpressionContextUtils::setLayerVariable( mpPolysLayer, u"qgis_25d_angle"_s, 45 );
}

void TestQgs25DRenderer::cleanupTestCase()
{
  delete mpPolysLayer;

  QgsApplication::exitQgis();
}

void TestQgs25DRenderer::render()
{
  //setup 25d renderer
  Qgs25DRenderer *renderer = new Qgs25DRenderer();
  renderer->setShadowEnabled( false );
  renderer->setWallShadingEnabled( false );
  renderer->setRoofColor( QColor( 253, 191, 111 ) );
  mpPolysLayer->setRenderer( renderer );

  QgsMapSettings mapSettings;
  mapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mapSettings.setExtent( mpPolysLayer->extent() );
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::mapSettingsScope( mapSettings );
  mapSettings.setExpressionContext( context );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "25d_render", "25d_render", mapSettings, 500, 20 );
}

void TestQgs25DRenderer::renderLayout()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  l.addLayoutItem( map );

  map->setExtent( mpPolysLayer->extent() );

  QGSVERIFYLAYOUTCHECK( u"25d_composer"_s, &l, 0, 100 );
}

QGSTEST_MAIN( TestQgs25DRenderer )
#include "testqgs25drenderer.moc"
