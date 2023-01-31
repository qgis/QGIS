/***************************************************************************
     testqgsmergeattributesdialog.cpp
     --------------------------------
    Date                 : Feb 2023
    Copyright            : (C) 2023 by Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsmergeattributesdialog.h"

class TestQgsMergeattributesDialog : public QObject
{
    Q_OBJECT

  public:
    TestQgsMergeattributesDialog() = default;

  private:
    QgisApp *mQgisApp = nullptr;

  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
      mQgisApp = new QgisApp();
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void test()
    {
      // Create test layer
      QgsVectorFileWriter::SaveVectorOptions options;
      QgsVectorLayer ml( "Polygon", "test", "memory" );
      QVERIFY( ml.isValid() );
      QTemporaryFile tmpFile( QDir::tempPath() +  "/TestQgsMergeattributesDialog" );
      tmpFile.open();
      const QString fileName( tmpFile.fileName( ) );
      options.driverName = "GPKG";
      options.layerName = "test";
      QString newFilename;
      const QgsVectorFileWriter::WriterError error( QgsVectorFileWriter::writeAsVectorFormatV3(
            &ml,
            fileName,
            ml.transformContext(),
            options, nullptr,
            &newFilename ) );

      QCOMPARE( error, QgsVectorFileWriter::WriterError::NoError );
      QgsVectorLayer layer( QStringLiteral( "%1|layername=test" ).arg( newFilename ), "src_test", "ogr" );
      QVERIFY( layer.startEditing() );
      QgsVectorDataProvider *pr = layer.dataProvider();

      // Create a feature
      QgsFeature f1( layer.fields(), 1 );
      f1.setGeometry( QgsGeometry::fromWkt( "POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))" ) );
      QVERIFY( pr->addFeature( f1 ) );
      QCOMPARE( layer.featureCount(), 1 );

      // And a bigger feature
      QgsFeature f2( layer.fields(), 2 );
      f2.setGeometry( QgsGeometry::fromWkt( "POLYGON((3 3, 10 3, 10 10, 3 10, 3 3))" ) );
      QVERIFY( pr->addFeature( f2 ) );
      QCOMPARE( layer.featureCount(), 2 );

      // Merge the attributes together
      QgsMergeAttributesDialog dialog( QgsFeatureList() << f1 << f2, &layer, mQgisApp->mapCanvas() );

      // At beginnning the first feature of the list is the target
      QCOMPARE( dialog.targetFeatureId(), f1.id() );

      // Check after taking feature with largest geometry
      QVERIFY( QMetaObject::invokeMethod( &dialog, "mFromLargestPushButton_clicked" ) );
      QCOMPARE( dialog.targetFeatureId(), f2.id() );
    }
};

QGSTEST_MAIN( TestQgsMergeattributesDialog )
#include "testqgsmergeattributesdialog.moc"
