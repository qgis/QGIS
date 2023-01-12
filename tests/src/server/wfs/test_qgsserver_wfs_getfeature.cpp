/***************************************************************************
     test_qgsserver_wfs_getfeature.cpp
     ---------------------------------
    Date                 : 12 Jan 2023
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

#include "qgstest.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsfeaturerequest.h"

#include "qgsbufferserverresponse.h"
#include "qgsserverinterfaceimpl.h"
#include "qgswfsgetfeature.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS parameters class
 */
class TestQgsServerWfsGetFeature : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void nullValueProperty();
};

void TestQgsServerWfsGetFeature::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsServerWfsGetFeature::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsServerWfsGetFeature::nullValueProperty()
{
  // init server iface
  QgsCapabilitiesCache cache;
  QgsServiceRegistry registry;
  QgsServerSettings settings;
  QgsServerInterfaceImpl interface( &cache, &registry, &settings );

  // init project
  QgsProject project;

  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=name0:int&field=name1:int&field=name2:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );

  QgsFields fields;
  const QgsField field0( QStringLiteral( "name0" ) );
  fields.append( field0 );
  const QgsField field1( QStringLiteral( "name1" ) );
  fields.append( field1 );
  const QgsField field2( QStringLiteral( "name2" ) );
  fields.append( field2 );

  QgsFeature f;
  f.setFields( fields, true );
  f.setAttribute( 0, QVariant( 0 ) );
  f.setAttribute( 1, QVariant() );
  f.setAttribute( 2, QVariant() );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 0.50, 0.50 ) ) );
  std::cout << "feature is valid: " << f.isValid() << std::endl;

  vl->startEditing();
  vl->addFeature( f );
  vl->commitChanges();
  std::cout << "feature count: " << vl->featureCount() << std::endl;

  project.addMapLayer( vl );

  QStringList wfsLayers;
  wfsLayers << vl->id();
  project.writeEntry( "WFSLayers", "/", wfsLayers );

  // build response
  QgsServerRequest request;
  request.setParameter( "VERSION", "1.0.0" );
  request.setParameter( "TYPENAME", "vl" );

  QgsBufferServerResponse response;
  QgsWfs::writeGetFeature( &interface, &project, "", request, response );

  // check response
  QDomDocument xml;
  xml.setContent( response.body() );

  const QDomElement elem = xml.documentElement();
  const QDomNode featureNode = elem.elementsByTagName( "gml:featureMember" ).at( 0 ).firstChild();
  const QDomNodeList childs = featureNode.childNodes();

  // attributes for fields name1 and name2 are NULL (QVariant()) for the
  // feature f, so it's not added in the resulting document. This way, the XML
  // only contains 3 childs:
  //   - <gml:boundedBy>
  //   - <qgs:geometry>
  //   - <qgs:name0>0</qgs:name0>
  QCOMPARE( childs.count(), 3 );
}

QGSTEST_MAIN( TestQgsServerWfsGetFeature )
#include "test_qgsserver_wfs_getfeature.moc"
