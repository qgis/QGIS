/***************************************************************************
     test_qgsserver_wms_restorer.cpp
     -------------------------------
    Date                 : 01 May 2020
    Copyright            : (C) 2020 by Paul Blottiere
    Email                : blottiere.paul@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsvectorlayer.h"

#include "qgswmsrestorer.h"
#include "qgsserverinterfaceimpl.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS restorer class
 */
class TestQgsServerWmsRestorer : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void restorer_layer();
};

void TestQgsServerWmsRestorer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsServerWmsRestorer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsServerWmsRestorer::restorer_layer()
{
  // read project
  const QString filename = QString( "%1/qgis_server/test_project.qgs" ).arg( TEST_DATA_DIR );
  QgsProject project;
  project.read( filename );

  // init wms parameters
  const QUrlQuery query;
  const QgsWms::QgsWmsParameters parameters( query );

  // init context
  QgsCapabilitiesCache cache;
  QgsServiceRegistry registry;
  QgsServerSettings settings;
  QgsServerInterfaceImpl interface( &cache, &registry, &settings );

  QgsWms::QgsWmsRenderContext context( &project, &interface );
  context.setFlag( QgsWms::QgsWmsRenderContext::UseOpacity );
  context.setParameters( parameters );

  // retrieve a specific vector layer
  const QString name = "testlayer";
  QgsVectorLayer *vl = nullptr;
  for ( QgsMapLayer *l : context.layers() )
  {
    if ( l->name().compare( name ) == 0 )
      vl = qobject_cast<QgsVectorLayer *>( l );
  }

  QCOMPARE( vl->name(), name );
  const double opacity = vl->opacity();

  // call restorer
  {
    // destructor is called once out of scope
    std::unique_ptr<QgsWms::QgsWmsRestorer> restorer;
    restorer.reset( new QgsWms::QgsWmsRestorer( context ) );

    const QString new_name = "new_name";
    vl->setName( new_name );
    QCOMPARE( vl->name(), new_name );

    vl->setOpacity( opacity + 10 );
    QCOMPARE( vl->opacity(), opacity + 10 );
  }

  // check that initial values are well restored
  QCOMPARE( vl->name(), name );
  QCOMPARE( vl->opacity(), opacity );
}

QGSTEST_MAIN( TestQgsServerWmsRestorer )
#include "test_qgsserver_wms_restorer.moc"
