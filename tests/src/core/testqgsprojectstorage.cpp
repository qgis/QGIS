/***************************************************************************
     testqgsprojectstorage.cpp
     --------------------------------------
    Date                 : March 2018
    Copyright            : (C) 2018 by Martin Dobias
    Email                : wonder.sk at gmail dot com
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
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgsvectorlayer.h"


class TestQgsProjectStorage : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void testMemoryStorage();
};

void TestQgsProjectStorage::init()
{
}

void TestQgsProjectStorage::cleanup()
{
  // will be called after every testfunction.
}

void TestQgsProjectStorage::initTestCase()
{
  // Runs once before any tests are run

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();
}


void TestQgsProjectStorage::cleanupTestCase()
{
  // Runs once after all tests are run
  QgsApplication::exitQgis();
}


//! A simple storage implementation that stores projects in memory
class MemoryStorage : public QgsProjectStorage
{
  public:
    virtual QString type() override
    {
      return QStringLiteral( "memory" );
    }

    virtual QStringList listProjects( const QString &uri ) override
    {
      Q_UNUSED( uri );
      return mProjects.keys();
    }

    virtual bool readProject( const QString &uri, QIODevice *ioDevice, QgsReadWriteContext &context ) override
    {
      QgsReadWriteContextCategoryPopper popper( context.enterCategory( "memory storage" ) );

      QStringList lst = uri.split( ":" );
      Q_ASSERT( lst.count() == 2 );
      QString projectName = lst[1];

      if ( !mProjects.contains( projectName ) )
      {
        context.pushMessage( "project not found", Qgis::Critical );
        return false;
      }

      QByteArray content = mProjects[projectName];
      ioDevice->write( content );
      ioDevice->seek( 0 );
      return true;
    }

    virtual bool writeProject( const QString &uri, QIODevice *ioDevice, QgsReadWriteContext &context ) override
    {
      Q_UNUSED( context );
      QStringList lst = uri.split( ":" );
      Q_ASSERT( lst.count() == 2 );
      QString projectName = lst[1];

      mProjects[projectName] = ioDevice->readAll();

      QgsProjectStorage::Metadata meta;
      meta.name = projectName;
      meta.lastModified = QDateTime::currentDateTime();
      mProjectsMetadata[projectName] = meta;
      return true;
    }

    virtual bool removeProject( const QString &uri ) override
    {
      QStringList lst = uri.split( ":" );
      Q_ASSERT( lst.count() == 2 );
      QString projectName = lst[1];

      if ( !mProjects.contains( projectName ) )
        return false;

      mProjects.remove( projectName );
      mProjectsMetadata.remove( projectName );
      return true;
    }

    virtual bool readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata ) override
    {
      QStringList lst = uri.split( ":" );
      Q_ASSERT( lst.count() == 2 );
      QString projectName = lst[1];
      if ( !mProjects.contains( projectName ) )
        return false;

      metadata = mProjectsMetadata[projectName];
      return true;
    }

  private:
    QHash<QString, QByteArray> mProjects;
    QHash<QString, QgsProjectStorage::Metadata> mProjectsMetadata;
};


void TestQgsProjectStorage::testMemoryStorage()
{
  QString dataDir( TEST_DATA_DIR ); // defined in CmakeLists.txt
  QString layerPath = dataDir + "/points.shp";
  QgsVectorLayer *layer1 = new QgsVectorLayer( layerPath, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer1->isValid() );

  MemoryStorage *memStorage = new MemoryStorage;

  QgsApplication::projectStorageRegistry()->registerProjectStorage( memStorage );

  QVERIFY( QgsApplication::projectStorageRegistry()->projectStorages().contains( memStorage ) );

  QCOMPARE( memStorage->listProjects( QString() ).count(), 0 );

  QgsProject prj1;
  prj1.setTitle( "best project ever" );
  prj1.addMapLayer( layer1 );
  prj1.setFileName( "memory:project1" );
  bool writeOk = prj1.write();

  QVERIFY( writeOk );
  QCOMPARE( memStorage->listProjects( QString() ).count(), 1 );

  QVERIFY( prj1.absoluteFilePath().isEmpty() );
  QCOMPARE( prj1.baseName(), QString( "project1" ) );
  QVERIFY( prj1.lastModified().secsTo( QDateTime::currentDateTime() ) < 1 );

  // read the project back

  QgsProject prj2;
  prj2.setFileName( "memory:project1" );
  bool readOk = prj2.read();

  QVERIFY( readOk );
  QCOMPARE( prj2.mapLayers().count(), 1 );
  QCOMPARE( prj2.title(), QString( "best project ever" ) );

  // test access of non-existent project

  QgsProject prj3;
  prj3.setFileName( "memory:nooooooooo!" );
  bool readInvalidOk = prj3.read();
  QVERIFY( !readInvalidOk );

  // test metadata access

  QgsProjectStorage::Metadata meta1;
  bool readMetaOk = memStorage->readProjectStorageMetadata( "memory:project1", meta1 );
  QVERIFY( readMetaOk );
  QCOMPARE( meta1.name, QString( "project1" ) );
  QVERIFY( meta1.lastModified.secsTo( QDateTime::currentDateTime() ) < 1 );

  QgsProjectStorage::Metadata metaX;
  bool readMetaInvalidOk = memStorage->readProjectStorageMetadata( "memory:projectXYZ", metaX );
  QVERIFY( !readMetaInvalidOk );

  // test removal

  bool removeInvalidOk = memStorage->removeProject( "memory:projectXYZ" );
  QVERIFY( !removeInvalidOk );
  QCOMPARE( memStorage->listProjects( QString() ).count(), 1 );

  bool removeOk = memStorage->removeProject( "memory:project1" );
  QVERIFY( removeOk );
  QCOMPARE( memStorage->listProjects( QString() ).count(), 0 );

  QgsApplication::projectStorageRegistry()->unregisterProjectStorage( memStorage );
}


QGSTEST_MAIN( TestQgsProjectStorage )
#include "testqgsprojectstorage.moc"
