/***************************************************************************
  testqgsfeaturelistcombobox.cpp

 ---------------------
 begin                : 3.10.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "qgsapplication.h"
#include "qgsfeaturefiltermodel.h"
#include "qgsfeaturelistcombobox.h"
#include "qgsfilterlineedit.h"
#include "qgsgui.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QLineEdit>
#include <QSignalSpy>

class QgsFilterLineEdit;

class TestQgsFeatureListComboBox : public QObject
{
    Q_OBJECT
  public:
    TestQgsFeatureListComboBox() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testSetGetLayer();
    void testSetGetForeignKey();
    void testMultipleForeignKeys();
    void testAllowNull();
    void testValuesAndSelection();
    void testValuesAndSelection_data();
    void nullRepresentation();
    void testNotExistingYetFeature();
    void testFeatureFurtherThanFetchLimit();

  private:
    std::unique_ptr<QgsVectorLayer> mLayer;

    friend class QgsFeatureListComboBox;
};

void TestQgsFeatureListComboBox::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST-FEATURELIST-COMBOBOX"_s );
}

void TestQgsFeatureListComboBox::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsFeatureListComboBox::init()
{
  // create layer
  mLayer = std::make_unique<QgsVectorLayer>( u"LineString?field=pk:int&field=material:string&field=diameter:int&field=raccord:string"_s, u"vl2"_s, u"memory"_s );
  mLayer->setDisplayExpression( u"pk"_s );

  // add features
  mLayer->startEditing();

  QgsFeature ft1( mLayer->fields() );
  ft1.setAttribute( u"pk"_s, 10 );
  ft1.setAttribute( u"material"_s, "iron" );
  ft1.setAttribute( u"diameter"_s, 120 );
  ft1.setAttribute( u"raccord"_s, "brides" );
  mLayer->addFeature( ft1 );

  QgsFeature ft2( mLayer->fields() );
  ft2.setAttribute( u"pk"_s, 11 );
  ft2.setAttribute( u"material"_s, "iron" );
  ft2.setAttribute( u"diameter"_s, 120 );
  ft2.setAttribute( u"raccord"_s, "sleeve" );
  mLayer->addFeature( ft2 );

  QgsFeature ft3( mLayer->fields() );
  ft3.setAttribute( u"pk"_s, 12 );
  ft3.setAttribute( u"material"_s, "steel" );
  ft3.setAttribute( u"diameter"_s, 120 );
  ft3.setAttribute( u"raccord"_s, "collar" );
  mLayer->addFeature( ft3 );

  QgsFeatureList flist;
  for ( int i = 13; i < 40; i++ )
  {
    QgsFeature f( mLayer->fields() );
    f.setAttribute( u"pk"_s, i );
    f.setAttribute( u"material"_s, u"material_%1"_s.arg( i ) );
    f.setAttribute( u"diameter"_s, i );
    f.setAttribute( u"raccord"_s, u"raccord_%1"_s.arg( i ) );
    flist << f;
  }
  mLayer->addFeatures( flist );

  mLayer->commitChanges();
}

void TestQgsFeatureListComboBox::cleanup()
{
}

void TestQgsFeatureListComboBox::testSetGetLayer()
{
  auto cb = std::make_unique<QgsFeatureListComboBox>();

  QVERIFY( cb->sourceLayer() == nullptr );
  cb->setSourceLayer( mLayer.get() );
  QCOMPARE( cb->sourceLayer(), mLayer.get() );
}

void TestQgsFeatureListComboBox::testSetGetForeignKey()
{
  QgsFeatureListComboBox cb;

  Q_NOWARN_DEPRECATED_PUSH
  QVERIFY( cb.identifierValue().isNull() );

  cb.setSourceLayer( mLayer.get() );
  cb.setDisplayExpression( "\"material\"" );
  cb.setIdentifierField( "material" );

  QSignalSpy spy( &cb, &QgsFeatureListComboBox::identifierValueChanged );
  QTest::keyClicks( cb.lineEdit(), "ro" );
  QTest::keyClick( cb.lineEdit(), Qt::Key_Enter );

  spy.wait();

  QCOMPARE( cb.identifierValue().toString(), u"iron"_s );

  Q_NOWARN_DEPRECATED_POP
}

void TestQgsFeatureListComboBox::testMultipleForeignKeys()
{
  auto cb = std::make_unique<QgsFeatureListComboBox>();

  QgsApplication::setNullRepresentation( u"nope"_s );

  QVERIFY( cb->identifierValues().isEmpty() );

  cb->setSourceLayer( mLayer.get() );
  cb->setIdentifierFields( QStringList() << "material" << "diameter" << "raccord" );
  cb->setDisplayExpression( "\"material\" || ' ' || \"diameter\" || ' ' || \"raccord\"" );
  cb->setAllowNull( true );

  cb->setIdentifierValues( QVariantList() << "gold" << 777 << "rush" );
  QCOMPARE( cb->identifierValues(), QVariantList() << "gold" << 777 << "rush" );

  cb->setIdentifierValuesToNull();
  QCOMPARE( cb->identifierValues().count(), 3 );
  QCOMPARE( cb->identifierValues(), QVariantList() << QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) << QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) << QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );

  cb->setIdentifierValues( QVariantList() << "silver" << 888 << "fish" );
  QCOMPARE( cb->identifierValues(), QVariantList() << "silver" << 888 << "fish" );

  cb->setIdentifierValuesToNull();
  QCOMPARE( cb->identifierValues().count(), 3 );
  QCOMPARE( cb->identifierValues(), QVariantList() << QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) << QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) << QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );

  cb->setIdentifierFields( QStringList() << "material" << "raccord" );
  cb->setDisplayExpression( "\"material\" || ' ' || \"raccord\"" );
  cb->setAllowNull( true );

  cb->setIdentifierValues( QVariantList() << "gold" << "fish" );
  QCOMPARE( cb->identifierValues().count(), 2 );
  QCOMPARE( cb->identifierValues(), QVariantList() << "gold" << "fish" );

  cb->setIdentifierValuesToNull();
  QCOMPARE( cb->identifierValues().count(), 2 );
  QCOMPARE( cb->identifierValues(), QVariantList() << QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) << QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );
}

void TestQgsFeatureListComboBox::testAllowNull()
{
  //QVERIFY( false );
  // Note to self: implement this!
}

void TestQgsFeatureListComboBox::testValuesAndSelection_data()
{
  QTest::addColumn<bool>( "allowNull" );

  QTest::newRow( "allowNull=true" ) << true;
  QTest::newRow( "allowNull=false" ) << false;
}

void TestQgsFeatureListComboBox::testValuesAndSelection()
{
  QFETCH( bool, allowNull );

  QgsApplication::setNullRepresentation( u"nope"_s );
  auto cb = std::make_unique<QgsFeatureListComboBox>();

  QSignalSpy spy( cb.get(), &QgsFeatureListComboBox::identifierValueChanged );

  cb->setSourceLayer( mLayer.get() );
  cb->setAllowNull( allowNull );
  cb->setIdentifierFields( { u"raccord"_s } );
  cb->setDisplayExpression( u"\"raccord\""_s );

  //check if everything is fine:
  spy.wait();
  QCOMPARE( cb->currentIndex(), allowNull ? cb->nullIndex() : 0 );
  QCOMPARE( cb->currentText(), allowNull ? u"nope"_s : u"brides"_s );

  //check if text correct, selected and if the clear button disappeared:
  cb->mLineEdit->clearValue();
  QCOMPARE( cb->currentIndex(), allowNull ? cb->nullIndex() : 0 );
  QCOMPARE( cb->currentText(), allowNull ? u"nope"_s : QString() );
  QCOMPARE( cb->lineEdit()->selectedText(), allowNull ? u"nope"_s : QString() );
  QVERIFY( !cb->mLineEdit->mClearAction );

  //check if text is selected after receiving focus
  cb->setFocus();
  QCOMPARE( cb->currentIndex(), allowNull ? cb->nullIndex() : 0 );
  QCOMPARE( cb->currentText(), allowNull ? u"nope"_s : QString() );
  QCOMPARE( cb->lineEdit()->selectedText(), allowNull ? u"nope"_s : QString() );
  QVERIFY( !cb->mLineEdit->mClearAction );

  //check with another entry, clear button needs to be there then:
  QTest::keyClicks( cb.get(), u"sleeve"_s );
  spy.wait();
  QCOMPARE( cb->currentText(), u"sleeve"_s );
  QVERIFY( cb->mLineEdit->mClearAction );
}

void TestQgsFeatureListComboBox::nullRepresentation()
{
  QgsApplication::setNullRepresentation( u"nope"_s );
  auto cb = std::make_unique<QgsFeatureListComboBox>();

  QgsFeatureFilterModel *model = qobject_cast<QgsFeatureFilterModel *>( cb->model() );
  QEventLoop loop;
  connect( model, &QgsFeatureFilterModel::filterJobCompleted, &loop, &QEventLoop::quit );

  cb->setAllowNull( true );
  cb->setSourceLayer( mLayer.get() );

  loop.exec();
  QCOMPARE( cb->lineEdit()->text(), u"nope"_s );
  QCOMPARE( cb->nullIndex(), 0 );
}


void TestQgsFeatureListComboBox::testNotExistingYetFeature()
{
  // test behavior when feature list combo box identifier values references a
  // not existing yet feature (created but not saved for instance)

  auto cb = std::make_unique<QgsFeatureListComboBox>();
  QgsFeatureFilterModel *model = qobject_cast<QgsFeatureFilterModel *>( cb->model() );
  QEventLoop loop;
  connect( model, &QgsFeatureFilterModel::filterJobCompleted, &loop, &QEventLoop::quit );

  QgsApplication::setNullRepresentation( u"nope"_s );

  QVERIFY( cb->identifierValues().isEmpty() );

  cb->setSourceLayer( mLayer.get() );
  cb->setAllowNull( true );

  cb->setIdentifierValues( QVariantList() << 42 );

  loop.exec();
  QCOMPARE( cb->currentText(), u"(42)"_s );
}

void TestQgsFeatureListComboBox::testFeatureFurtherThanFetchLimit()
{
  const int fetchLimit = 20;
  QVERIFY( fetchLimit < mLayer->featureCount() );
  auto cb = std::make_unique<QgsFeatureListComboBox>();
  QgsFeatureFilterModel *model = qobject_cast<QgsFeatureFilterModel *>( cb->model() );
  QSignalSpy spy( cb.get(), &QgsFeatureListComboBox::identifierValueChanged );
  model->setFetchLimit( 20 );
  model->setAllowNull( false );
  cb->setSourceLayer( mLayer.get() );
  cb->setIdentifierFields( { u"pk"_s } );
  spy.wait();
  QCOMPARE( model->mEntries.count(), 20 );
  for ( int i = 0; i < 20; i++ )
    QCOMPARE( model->mEntries.at( i ).identifierFields.at( 0 ).toInt(), i + 10 );
  cb->setIdentifierValues( { 33 } );
  spy.wait();
  QCOMPARE( cb->lineEdit()->text(), u"33"_s );
  QCOMPARE( model->mEntries.count(), 21 );
  QCOMPARE( model->mEntries.at( 0 ).identifierFields.at( 0 ).toInt(), 33 );
}

QGSTEST_MAIN( TestQgsFeatureListComboBox )
#include "testqgsfeaturelistcombobox.moc"
