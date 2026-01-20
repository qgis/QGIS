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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsmergeattributesdialog.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QObject>

class TestQgsMergeattributesDialog : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMergeattributesDialog()
      : QgsTest( u"Merge attributes dialog"_s )
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
      QVERIFY( tmpFile.open() );
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
      QgsVectorLayer layer( u"%1|layername=test"_s.arg( newFilename ), "src_test", "ogr" );
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

      QgsField uniqueField( u"unique"_s, QMetaType::Type::Int );
      QgsFieldConstraints constraints;
      constraints.setConstraint(
        QgsFieldConstraints::ConstraintUnique
      );
      uniqueField.setConstraints(
        constraints
      );

      QgsField notUniqueField( u"not_unique"_s, QMetaType::Type::Int );
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

      QgsField notHiddenField( u"not_hidden"_s, QMetaType::Type::Int );
      QgsField hiddenField( u"hidden"_s, QMetaType::Type::Int );
      // hide the field
      ml.setEditorWidgetSetup( 1, QgsEditorWidgetSetup( u"Hidden"_s, QVariantMap() ) );
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

      QgsField notHiddenField( u"not_hidden"_s, QMetaType::Type::Int );
      QgsField hiddenField( u"hidden"_s, QMetaType::Type::Int );
      QVERIFY( ml.dataProvider()->addAttributes( { notHiddenField, hiddenField } ) );
      ml.updateFields();

      // hide the field
      ml.setEditorWidgetSetup( 1, QgsEditorWidgetSetup( u"Hidden"_s, QVariantMap() ) );


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

    void testMergePolicies()
    {
      // Create test layer
      QgsVectorFileWriter::SaveVectorOptions options;
      QgsVectorLayer ml( "LineString", "test", "memory" );
      QVERIFY( ml.isValid() );

      QgsField defaultValueField( u"defaultValue"_s, QMetaType::Type::Int );
      QgsField sumField( u"sum"_s, QMetaType::Type::Int );
      QgsField geometryWeightedField( u"geometryWeighted"_s, QMetaType::Type::Double );
      QgsField largestGeometryField( u"largestGeometry"_s, QMetaType::Type::QString );
      QgsField minimumValueField( u"minimumValue"_s, QMetaType::Type::Int );
      QgsField maximumValueField( u"maximumValue"_s, QMetaType::Type::Int );
      QgsField skipAttributeField( u"skipAttribute"_s, QMetaType::Type::Int );
      QgsField unsetField( u"unsetField"_s, QMetaType::Type::Int );

      QVERIFY( ml.dataProvider()->addAttributes( { defaultValueField, sumField, geometryWeightedField, largestGeometryField, minimumValueField, maximumValueField, skipAttributeField, unsetField } ) );
      ml.updateFields();

      // set policies
      ml.setFieldMergePolicy( 0, Qgis::FieldDomainMergePolicy::DefaultValue );
      ml.setFieldMergePolicy( 1, Qgis::FieldDomainMergePolicy::Sum );
      ml.setFieldMergePolicy( 2, Qgis::FieldDomainMergePolicy::GeometryWeighted );
      ml.setFieldMergePolicy( 3, Qgis::FieldDomainMergePolicy::LargestGeometry );
      ml.setFieldMergePolicy( 4, Qgis::FieldDomainMergePolicy::MinimumValue );
      ml.setFieldMergePolicy( 5, Qgis::FieldDomainMergePolicy::MaximumValue );
      ml.setFieldMergePolicy( 6, Qgis::FieldDomainMergePolicy::SetToNull );
      ml.setFieldMergePolicy( 7, Qgis::FieldDomainMergePolicy::UnsetField );

      // verify that policies have been correctly set

      QCOMPARE( ml.fields().field( 0 ).mergePolicy(), Qgis::FieldDomainMergePolicy::DefaultValue );
      QCOMPARE( ml.fields().field( 1 ).mergePolicy(), Qgis::FieldDomainMergePolicy::Sum );
      QCOMPARE( ml.fields().field( 2 ).mergePolicy(), Qgis::FieldDomainMergePolicy::GeometryWeighted );
      QCOMPARE( ml.fields().field( 3 ).mergePolicy(), Qgis::FieldDomainMergePolicy::LargestGeometry );
      QCOMPARE( ml.fields().field( 4 ).mergePolicy(), Qgis::FieldDomainMergePolicy::MinimumValue );
      QCOMPARE( ml.fields().field( 5 ).mergePolicy(), Qgis::FieldDomainMergePolicy::MaximumValue );
      QCOMPARE( ml.fields().field( 6 ).mergePolicy(), Qgis::FieldDomainMergePolicy::SetToNull );
      QCOMPARE( ml.fields().field( 7 ).mergePolicy(), Qgis::FieldDomainMergePolicy::UnsetField );

      // Create features
      QgsFeature f1( ml.fields(), 1 );
      f1.setAttributes( QVector<QVariant>() << 10 << 200 << 7.5 << u"smaller"_s << 10 << -10 << 0 << 20 );
      f1.setGeometry( QgsGeometry::fromWkt( "LINESTRING(10 0, 15 0)" ) );
      QVERIFY( ml.dataProvider()->addFeature( f1 ) );
      QCOMPARE( ml.featureCount(), 1 );

      QgsFeature f2( ml.fields(), 2 );
      f2.setAttributes( QVector<QVariant>() << 15 << 100 << 5 << u"bigger"_s << -10 << 10 << 5 << 12 );
      f2.setGeometry( QgsGeometry::fromWkt( "LINESTRING(0 0, 10 0)" ) );
      QVERIFY( ml.dataProvider()->addFeature( f2 ) );
      QCOMPARE( ml.featureCount(), 2 );

      QgsMergeAttributesDialog dialog1( QgsFeatureList() << f1 << f2, &ml, mQgisApp->mapCanvas() );

      QCOMPARE( dialog1.mergedAttributes().at( 0 ).toInt(), 10 );
      QCOMPARE( dialog1.mergedAttributes().at( 1 ).toInt(), 300 );
      QVERIFY( qgsDoubleNear( dialog1.mergedAttributes().at( 2 ).toDouble(), 5.83333, 0.00001 ) );
      QCOMPARE( dialog1.mergedAttributes().at( 3 ).toString(), u"bigger"_s );
      QCOMPARE( dialog1.mergedAttributes().at( 4 ).toInt(), -10 );
      QCOMPARE( dialog1.mergedAttributes().at( 5 ).toInt(), 10 );
      QVERIFY( !dialog1.mergedAttributes().at( 6 ).isValid() );
      QCOMPARE( dialog1.mergedAttributes().at( 7 ).toInt(), 20 );
    }
};

QGSTEST_MAIN( TestQgsMergeattributesDialog )
#include "testqgsmergeattributesdialog.moc"
