/***************************************************************************
  testqgslayerdropclassifier.cpp
  --------------------------------------
  Date                 : July 2026
  Copyright            : (C) 2026 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscustomdrophandler.h"
#include "qgslayerdropclassifier.h"
#include "qgsmimedatautils.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgstest.h"

#include <QDir>
#include <QMimeData>
#include <QString>
#include <QTemporaryFile>
#include <QUrl>

using namespace Qt::StringLiterals;

//! Test drop handler which declares (via canHandleMimeData) local files with a given suffix
class TestDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT
  public:
    explicit TestDropHandler( const QString &suffix )
      : mSuffix( suffix )
    {}

    bool canHandleMimeData( const QMimeData *data ) override
    {
      const QList<QUrl> urls = data->urls();
      for ( const QUrl &url : urls )
      {
        if ( url.toLocalFile().endsWith( '.' + mSuffix, Qt::CaseInsensitive ) )
          return true;
      }
      return false;
    }

  private:
    QString mSuffix;
};

//! Test drop handler mimicking a legacy handler which only implements handleFileDrop()
class LegacyDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT
    // does not reimplement canHandleMimeData(), so it relies on the base default (TRUE)
    // and claims any payload, for backward compatibility
};

//! Test drop handler which claims a custom uri provider key (browser custom uri drops)
class CustomUriDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT
  public:
    QString customUriProviderKey() const override { return u"test_custom"_s; }
};

class TestQgsLayerDropClassifier : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testIsDatasetDrag();
    void testClassifyDragPayload();
};

void TestQgsLayerDropClassifier::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsLayerDropClassifier::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayerDropClassifier::testIsDatasetDrag()
{
  // file urls dragged from the desktop
  QMimeData fileMime;
  fileMime.setUrls( { QUrl::fromLocalFile( u"/home/me/points.shp"_s ) } );
  QVERIFY( QgsLayerDropClassifier::isDatasetDrag( &fileMime ) );

  // a uri list originating from within QGIS, e.g. the browser panel
  QgsMimeDataUtils::Uri layerUri;
  layerUri.layerType = u"vector"_s;
  layerUri.providerKey = u"memory"_s;
  layerUri.uri = u"Point?field=fld:integer"_s;
  const std::unique_ptr<QMimeData> uriMime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << layerUri ) );
  QVERIFY( QgsLayerDropClassifier::isDatasetDrag( uriMime.get() ) );

  // an internal layer tree reordering is not a dataset drag, even though the layer tree
  // model also exposes its nodes as a QGIS uri list
  uriMime->setData( u"application/qgis.layertreemodeldata"_s, QByteArray() );
  QVERIFY( !QgsLayerDropClassifier::isDatasetDrag( uriMime.get() ) );

  // unrelated payloads
  QMimeData textMime;
  textMime.setText( u"some text"_s );
  QVERIFY( !QgsLayerDropClassifier::isDatasetDrag( &textMime ) );
}

void TestQgsLayerDropClassifier::testClassifyDragPayload()
{
  using PayloadType = Qgis::LayerDropPayloadType;

  QVector<QPointer<QgsCustomDropHandler>> noHandlers;

  // project file url: the file does not need to exist, the extension is enough
  QMimeData projectFileMime;
  projectFileMime.setUrls( { QUrl::fromLocalFile( u"/home/me/project.qgz"_s ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &projectFileMime, noHandlers ), PayloadType::Project );

  // dataset file url: providers recognize the extension of an existing file
  const QString pointsPath = QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s;
  QVERIFY( QFile::exists( pointsPath ) );
  QMimeData datasetFileMime;
  datasetFileMime.setUrls( { QUrl::fromLocalFile( pointsPath ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &datasetFileMime, noHandlers ), PayloadType::Layers );

  // file with an extension no provider recognizes
  QTemporaryFile docFile( QDir::tempPath() + u"/XXXXXX.docx"_s );
  QVERIFY( docFile.open() );
  QMimeData invalidFileMime;
  invalidFileMime.setUrls( { QUrl::fromLocalFile( docFile.fileName() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &invalidFileMime, noHandlers ), PayloadType::Invalid );

  // the fast, extension based check cannot judge extensionless files and directories:
  // they get the benefit of the doubt, the full check happens on drop
  QTemporaryFile extensionlessFile( QDir::tempPath() + u"/XXXXXX"_s );
  QVERIFY( extensionlessFile.open() );
  QMimeData extensionlessFileMime;
  extensionlessFileMime.setUrls( { QUrl::fromLocalFile( extensionlessFile.fileName() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &extensionlessFileMime, noHandlers ), PayloadType::Layers );

  QMimeData directoryMime;
  directoryMime.setUrls( { QUrl::fromLocalFile( QDir::tempPath() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &directoryMime, noHandlers ), PayloadType::Layers );

  // mixed payload: the loadable dataset wins over the unknown file
  QMimeData mixedFileMime;
  mixedFileMime.setUrls( { QUrl::fromLocalFile( docFile.fileName() ), QUrl::fromLocalFile( pointsPath ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &mixedFileMime, noHandlers ), PayloadType::Layers );

  // a project anywhere in the payload takes precedence
  QMimeData projectAndDatasetMime;
  projectAndDatasetMime.setUrls( { QUrl::fromLocalFile( pointsPath ), QUrl::fromLocalFile( u"/home/me/project.qgs"_s ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &projectAndDatasetMime, noHandlers ), PayloadType::Project );

  // project uri (e.g. a project stored in a geopackage, dragged from the browser)
  QgsMimeDataUtils::Uri projectUri;
  projectUri.layerType = u"project"_s;
  projectUri.uri = u"/home/me/db.gpkg"_s;
  const std::unique_ptr<QMimeData> projectUriMime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << projectUri ) );
  QCOMPARE( QgsLayerDropClassifier::classify( projectUriMime.get(), noHandlers ), PayloadType::Project );

  // layer uri: any non-project uri from QGIS is a loadable dataset
  QgsMimeDataUtils::Uri layerUri;
  layerUri.layerType = u"vector"_s;
  layerUri.providerKey = u"memory"_s;
  layerUri.uri = u"Point?field=fld:integer"_s;
  const std::unique_ptr<QMimeData> layerUriMime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << layerUri ) );
  QCOMPARE( QgsLayerDropClassifier::classify( layerUriMime.get(), noHandlers ), PayloadType::Layers );

  // layer definition files insert layers into the tree: they classify as layers
  // and get the insertion indicator
  QTemporaryFile qlrFile( QDir::tempPath() + u"/XXXXXX.qlr"_s );
  QVERIFY( qlrFile.open() );
  QMimeData qlrFileMime;
  qlrFileMime.setUrls( { QUrl::fromLocalFile( qlrFile.fileName() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &qlrFileMime, noHandlers ), PayloadType::Layers );

  // payloads no provider can load, but which the application handles through custom
  // drop handlers, classify as CustomHandler instead of Invalid
  QTemporaryFile qptFile( QDir::tempPath() + u"/XXXXXX.qpt"_s );
  QVERIFY( qptFile.open() );
  QMimeData qptFileMime;
  qptFileMime.setUrls( { QUrl::fromLocalFile( qptFile.fileName() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &qptFileMime, noHandlers ), PayloadType::Invalid );

  TestDropHandler qptHandler( u"qpt"_s );
  const QVector<QPointer<QgsCustomDropHandler>> handlers { QPointer<QgsCustomDropHandler>( &qptHandler ) };
  QCOMPARE( QgsLayerDropClassifier::classify( &qptFileMime, handlers ), PayloadType::CustomHandler );
  // other unloadable payloads are still refused: a modern handler declares its
  // capabilities via canHandleMimeData() and opts out of unrecognized file drops
  QCOMPARE( QgsLayerDropClassifier::classify( &invalidFileMime, handlers ), PayloadType::Invalid );

  // a legacy handler which does not reimplement canHandleMimeData() relies on the base
  // default (TRUE): for backward compatibility it claims any payload, so an otherwise
  // unloadable drop is accepted as a custom handler payload rather than refused
  LegacyDropHandler legacyHandler;
  const QVector<QPointer<QgsCustomDropHandler>> legacyHandlers { QPointer<QgsCustomDropHandler>( &legacyHandler ) };
  QCOMPARE( QgsLayerDropClassifier::classify( &invalidFileMime, legacyHandlers ), PayloadType::CustomHandler );

  // a custom uri (e.g. a Processing model dragged from the browser) is dispatched to a
  // matching custom drop handler via handleCustomUriDrop(); it must not be classified as
  // a layer (which would show a misleading insertion indicator) but as CustomHandler
  QgsMimeDataUtils::Uri customUri;
  customUri.layerType = u"custom"_s;
  customUri.providerKey = u"test_custom"_s;
  customUri.uri = u"some_model"_s;
  const std::unique_ptr<QMimeData> customUriMime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << customUri ) );

  // without a handler claiming its provider key, the custom uri cannot be handled at all
  QCOMPARE( QgsLayerDropClassifier::classify( customUriMime.get(), noHandlers ), PayloadType::Invalid );

  CustomUriDropHandler customUriHandler;
  const QVector<QPointer<QgsCustomDropHandler>> customUriHandlers { QPointer<QgsCustomDropHandler>( &customUriHandler ) };
  QCOMPARE( QgsLayerDropClassifier::classify( customUriMime.get(), customUriHandlers ), PayloadType::CustomHandler );
}

QGSTEST_MAIN( TestQgsLayerDropClassifier )
#include "testqgslayerdropclassifier.moc"
