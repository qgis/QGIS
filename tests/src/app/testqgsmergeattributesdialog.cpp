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

class TestQgsMergeattributesDialog : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMergeattributesDialog()
      : QgsTest( QStringLiteral( "Merge attributes dialog" ) )
    {}

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
      QTemporaryFile tmpFile( QDir::tempPath() + "/TestQgsMergeattributesDialog" );
      tmpFile.open();
      const QString fileName( tmpFile.fileName() );
      options.driverName = "GPKG";
      options.layerName = "test";
      QString newFilename;
      const QgsVectorFileWriter::WriterError error( QgsVectorFileWriter::writeAsVectorFormatV3(
        &ml,
        fileName,
        ml.transformContext(),
        options, nullptr,
        &newFilename
      ) );

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

      // At beginning the first feature of the list is the target
      QCOMPARE( dialog.targetFeatureId(), FID_NULL );
      QCOMPARE( dialog.mergedAttributes().at( 0 ), "Autogenerate" );

      // Check after taking feature with largest geometry
      QVERIFY( QMetaObject::invokeMethod( &dialog, "mFromLargestPushButton_clicked" ) );
      QCOMPARE( dialog.targetFeatureId(), f2.id() );
      QCOMPARE( dialog.mergedAttributes().at( 0 ), f2.id() );
    }

    void testWithUniqueConstraint()
    {
      // Create test layer
      QgsVectorFileWriter::SaveVectorOptions options;
      QgsVectorLayer ml( "Polygon", "test", "memory" );
      QVERIFY( ml.isValid() );

      QgsField uniqueField( QStringLiteral( "unique" ), QMetaType::Type::Int );
      QgsFieldConstraints constraints;
      constraints.setConstraint(
        QgsFieldConstraints::ConstraintUnique
      );
      uniqueField.setConstraints(
        constraints
      );

      QgsField notUniqueField( QStringLiteral( "not_unique" ), QMetaType::Type::Int );
      QVERIFY( ml.dataProvider()->addAttributes(
        { uniqueField, notUniqueField }
      ) );

      ml.updateFields();
      QCOMPARE( ml.fields().at( 0 ).constraints().constraints(), QgsFieldConstraints::ConstraintUnique );
      QCOMPARE( ml.fields().at( 1 ).constraints().constraints(), QgsFieldConstraints::Constraints() );

      // Create a feature
      QgsFeature f1( ml.fields(), 1 );
      f1.setAttributes( { 11, 12 } );
      f1.setGeometry( QgsGeometry::fromWkt( "POLYGON((0 0, 5 0, 5 5, 0 5, 0 0))" ) );
      QVERIFY( ml.dataProvider()->addFeature( f1 ) );
      QCOMPARE( ml.featureCount(), 1 );

      // And a bigger feature
      QgsFeature f2( ml.fields(), 2 );
      f2.setAttributes( { 21, 22 } );
      f2.setGeometry( QgsGeometry::fromWkt( "POLYGON((3 3, 10 3, 10 10, 3 10, 3 3))" ) );
      QVERIFY( ml.dataProvider()->addFeature( f2 ) );
      QCOMPARE( ml.featureCount(), 2 );

      // Merge the attributes together
      QgsMergeAttributesDialog dialog( QgsFeatureList() << f1 << f2, &ml, mQgisApp->mapCanvas() );

      // At beginning the first feature of the list is the target
      QCOMPARE( dialog.targetFeatureId(), FID_NULL );
      // the first field has a unique constraint, so it should not have any value copied from the source feature
      QVERIFY( !dialog.mergedAttributes().at( 0 ).isValid() );
      QCOMPARE( dialog.mergedAttributes().at( 1 ), 12 );

      // Check after taking feature with largest geometry
      QVERIFY( QMetaObject::invokeMethod( &dialog, "mFromLargestPushButton_clicked" ) );
      QCOMPARE( dialog.targetFeatureId(), f2.id() );
      // the first field has a unique constraint, so it should not have any value copied from the source feature
      QVERIFY( !dialog.mergedAttributes().at( 0 ).isValid() );
      QCOMPARE( dialog.mergedAttributes().at( 1 ), 22 );
    }

    void testWithHiddenField()
    {
      // Create test layer
      QgsVectorFileWriter::SaveVectorOptions options;
      QgsVectorLayer ml( "LineString", "test", "memory" );
      QVERIFY( ml.isValid() );

      QgsField notHiddenField( QStringLiteral( "not_hidden" ), QMetaType::Type::Int );
      QgsField hiddenField( QStringLiteral( "hidden" ), QMetaType::Type::Int );
      // hide the field
      ml.setEditorWidgetSetup( 1, QgsEditorWidgetSetup( QStringLiteral( "Hidden" ), QVariantMap() ) );
      QVERIFY( ml.dataProvider()->addAttributes( { notHiddenField, hiddenField } ) );
      ml.updateFields();

      // Create features
      QgsFeature f1( ml.fields(), 1 );
      f1.setAttributes( QVector<QVariant>() << 1 << 2 );
      f1.setGeometry( QgsGeometry::fromWkt( "LINESTRING(0 0, 10 0)" ) );
      QVERIFY( ml.dataProvider()->addFeature( f1 ) );
      QCOMPARE( ml.featureCount(), 1 );

      QgsFeature f2( ml.fields(), 2 );
      f2.setAttributes( QVector<QVariant>() << 3 << 4 );
      f2.setGeometry( QgsGeometry::fromWkt( "LINESTRING(10 0, 15 0)" ) );
      QVERIFY( ml.dataProvider()->addFeature( f2 ) );
      QCOMPARE( ml.featureCount(), 2 );

      // Merge the attributes together
      QgsMergeAttributesDialog dialog( QgsFeatureList() << f1 << f2, &ml, mQgisApp->mapCanvas() );
      QVERIFY( QMetaObject::invokeMethod( &dialog, "mFromLargestPushButton_clicked" ) );
      QCOMPARE( dialog.mergedAttributes(), QgsAttributes() << 1 << 2 );
    }

    void testWithHiddenFieldDefaultsToEmpty()
    {
      // Create test layer
      QgsVectorFileWriter::SaveVectorOptions options;
      QgsVectorLayer ml( "LineString", "test", "memory" );
      QVERIFY( ml.isValid() );

      QgsField notHiddenField( QStringLiteral( "not_hidden" ), QMetaType::Type::Int );
      QgsField hiddenField( QStringLiteral( "hidden" ), QMetaType::Type::Int );
      QVERIFY( ml.dataProvider()->addAttributes( { notHiddenField, hiddenField } ) );
      ml.updateFields();

      // hide the field
      ml.setEditorWidgetSetup( 1, QgsEditorWidgetSetup( QStringLiteral( "Hidden" ), QVariantMap() ) );


      // Create features
      QgsFeature f1( ml.fields(), 1 );
      f1.setAttributes( QVector<QVariant>() << 1 << 2 );
      f1.setGeometry( QgsGeometry::fromWkt( "LINESTRING(0 0, 10 0)" ) );
      QVERIFY( ml.dataProvider()->addFeature( f1 ) );
      QCOMPARE( ml.featureCount(), 1 );

      QgsFeature f2( ml.fields(), 2 );
      f2.setAttributes( QVector<QVariant>() << 3 << 4 );
      f2.setGeometry( QgsGeometry::fromWkt( "LINESTRING(10 0, 15 0)" ) );
      QVERIFY( ml.dataProvider()->addFeature( f2 ) );
      QCOMPARE( ml.featureCount(), 2 );

      // Merge the attributes together
      QgsMergeAttributesDialog dialog( QgsFeatureList() << f1 << f2, &ml, mQgisApp->mapCanvas() );
      // QVariant gets turned into default value while saving the layer
      QCOMPARE( dialog.mergedAttributes(), QgsAttributes() << 1 << QVariant() );
    }
};

QGSTEST_MAIN( TestQgsMergeattributesDialog )
#include "testqgsmergeattributesdialog.moc"
