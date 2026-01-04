/***************************************************************************
     test_qgsserver_wms_vector_tile.cpp
     ---------------------------------
    Date                 : 06 Oct 2022
    Copyright            : (C) 2022 by Paul Blottiere
    Email                : paul dot blottiere @ gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproject.h"
#include "qgsserverinterfaceimpl.h"
#include "qgstest.h"
#include "qgsvectortilelayer.h"
#include "qgswmsparameters.h"
#include "qgswmsrendercontext.h"
#include "qgswmsrenderer.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the vector tile management in WMS
 */
class TestQgsServerWmsVectorTile : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void opacity();
};

void TestQgsServerWmsVectorTile::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsServerWmsVectorTile::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsServerWmsVectorTile::opacity()
{
  // init vector tile layers
  const QString path = QStringLiteral( TEST_DATA_DIR ) + u"/vector_tile/{z}-{x}-{y}.pbf"_s;

  QgsDataSourceUri uri;
  uri.setParam( u"type"_s, u"xyz"_s );
  uri.setParam( u"url"_s, QUrl::fromLocalFile( path ).toString() );

  QgsVectorTileLayer *layer0 = new QgsVectorTileLayer( uri.encodedUri(), u"layer0"_s );
  QgsVectorTileLayer *layer1 = new QgsVectorTileLayer( uri.encodedUri(), u"layer1"_s );

  // init project with vector tile
  QgsProject project;
  project.addMapLayer( layer0 );
  project.addMapLayer( layer1 );

  // init wms parameters
  QUrlQuery query;
  query.addQueryItem( "LAYERS", "layer0,layer1" );
  query.addQueryItem( "OPACITIES", "0,255" );
  const QgsWms::QgsWmsParameters parameters( query );

  // init wms renderer
  QgsServiceRegistry registry;
  QgsServerSettings settings;
  QgsCapabilitiesCache cache( settings.capabilitiesCacheSize() );
  QgsServerInterfaceImpl interface( &cache, &registry, &settings );

  QgsWms::QgsWmsRenderContext context( &project, &interface );
  context.setFlag( QgsWms::QgsWmsRenderContext::UseOpacity );
  context.setParameters( parameters );

  QgsWms::QgsRenderer renderer( context );

  // configure layers according to WMS renderer
  QList<QgsMapLayer *> layers = context.layersToRender();

  QgsMapSettings mapSettings;
  renderer.configureLayers( layers, &mapSettings );

  // check opacity
  for ( auto layer : layers )
  {
    float opacity = 0.;
    if ( layer->name() == "layer1" )
      opacity = 1.;
    QCOMPARE( layer->opacity(), opacity );
  }
}

QGSTEST_MAIN( TestQgsServerWmsVectorTile )
#include "test_qgsserver_wms_vector_tile.moc"
