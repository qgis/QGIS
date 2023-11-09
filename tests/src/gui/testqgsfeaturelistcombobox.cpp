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

#include "qgstest.h"

#include "qgsapplication.h"
#include "qgsfeaturelistcombobox.h"
#include "qgsfilterlineedit.h"
#include "qgsvectorlayer.h"
#include "qgsfeaturefiltermodel.h"
#include "qgsgui.h"

#include <memory>

#include <QLineEdit>
#include <QSignalSpy>

class QgsFilterLineEdit;

class TestQgsFeatureListComboBox : public QObject
{
    Q_OBJECT
  public:
    TestQgsFeatureListComboBox() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

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
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST-FEATURELIST-COMBOBOX" ) );

}

void TestQgsFeatureListComboBox::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsFeatureListComboBox::init()
{
  // create layer
  mLayer.reset( new QgsVectorLayer( QStringLiteral( "LineString?field=pk:int&field=material:string&field=diameter:int&field=raccord:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) ) );
  mLayer->setDisplayExpression( QStringLiteral( "pk" ) );

  // add features
  mLayer->startEditing();

  QgsFeature ft1( mLayer->fields() );
  ft1.setAttribute( QStringLiteral( "pk" ), 10 );
  ft1.setAttribute( QStringLiteral( "material" ), "iron" );
  ft1.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft1.setAttribute( QStringLiteral( "raccord" ), "brides" );
  mLayer->addFeature( ft1 );

  QgsFeature ft2( mLayer->fields() );
  ft2.setAttribute( QStringLiteral( "pk" ), 11 );
  ft2.setAttribute( QStringLiteral( "material" ), "iron" );
  ft2.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft2.setAttribute( QStringLiteral( "raccord" ), "sleeve" );
  mLayer->addFeature( ft2 );

  QgsFeature ft3( mLayer->fields() );
  ft3.setAttribute( QStringLiteral( "pk" ), 12 );
  ft3.setAttribute( QStringLiteral( "material" ), "steel" );
  ft3.setAttribute( QStringLiteral( "diameter" ), 120 );
  ft3.setAttribute( QStringLiteral( "raccord" ), "collar" );
  mLayer->addFeature( ft3 );

  QgsFeatureList flist;
  for ( int i = 13; i < 40; i++ )
  {
    QgsFeature f( mLayer->fields() );
    f.setAttribute( QStringLiteral( "pk" ), i );
    f.setAttribute( QStringLiteral( "material" ), QStringLiteral( "material_%1" ).arg( i ) );
    f.setAttribute( QStringLiteral( "diameter" ), i );
    f.setAttribute( QStringLiteral( "raccord" ), QStringLiteral( "raccord_%1" ).arg( i ) );
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
  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );

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

  QCOMPARE( cb.identifierValue().toString(), QStringLiteral( "iron" ) );

  Q_NOWARN_DEPRECATED_POP
}

void TestQgsFeatureListComboBox::testMultipleForeignKeys()
{
  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );

  QgsApplication::setNullRepresentation( QStringLiteral( "nope" ) );

  QVERIFY( cb->identifierValues().isEmpty() );

  cb->setSourceLayer( mLayer.get() );
  cb->setIdentifierFields( QStringList() << "material" << "diameter" << "raccord" );
  cb->setDisplayExpression( "\"material\" || ' ' || \"diameter\" || ' ' || \"raccord\"" );
  cb->setAllowNull( true );

  cb->setIdentifierValues( QVariantList() << "gold" << 777 << "rush" );
  QCOMPARE( cb->identifierValues(), QVariantList() << "gold" << 777 << "rush" );

  cb->setIdentifierValuesToNull();
  QCOMPARE( cb->identifierValues().count(), 3 );
  QCOMPARE( cb->identifierValues(), QVariantList() << QVariant( QVariant::Int ) << QVariant( QVariant::Int ) << QVariant( QVariant::Int ) );

  cb->setIdentifierValues( QVariantList() << "silver" << 888 << "fish" );
  QCOMPARE( cb->identifierValues(), QVariantList() << "silver" << 888 << "fish" );

  cb->setIdentifierValuesToNull();
  QCOMPARE( cb->identifierValues().count(), 3 );
  QCOMPARE( cb->identifierValues(), QVariantList() << QVariant( QVariant::Int ) << QVariant( QVariant::Int ) << QVariant( QVariant::Int ) );

  cb->setIdentifierFields( QStringList() << "material" << "raccord" );
  cb->setDisplayExpression( "\"material\" || ' ' || \"raccord\"" );
  cb->setAllowNull( true );

  cb->setIdentifierValues( QVariantList() << "gold" << "fish" );
  QCOMPARE( cb->identifierValues().count(), 2 );
  QCOMPARE( cb->identifierValues(), QVariantList() << "gold" << "fish" );

  cb->setIdentifierValuesToNull();
  QCOMPARE( cb->identifierValues().count(), 2 );
  QCOMPARE( cb->identifierValues(), QVariantList() << QVariant( QVariant::Int ) << QVariant( QVariant::Int ) );
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

  QgsApplication::setNullRepresentation( QStringLiteral( "nope" ) );
  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );

  QSignalSpy spy( cb.get(), &QgsFeatureListComboBox::identifierValueChanged );

  cb->setSourceLayer( mLayer.get() );
  cb->setAllowNull( allowNull );
  cb->setIdentifierFields( {QStringLiteral( "raccord" )} );
  cb->setDisplayExpression( QStringLiteral( "\"raccord\"" ) );

  //check if everything is fine:
  spy.wait();
  QCOMPARE( cb->currentIndex(), allowNull ? cb->nullIndex() : 0 );
  QCOMPARE( cb->currentText(), allowNull ? QStringLiteral( "nope" ) : QStringLiteral( "brides" ) );

  //check if text correct, selected and if the clear button disappeared:
  cb->mLineEdit->clearValue();
  QCOMPARE( cb->currentIndex(), allowNull ? cb->nullIndex() : 0 );
  QCOMPARE( cb->currentText(), allowNull ? QStringLiteral( "nope" ) : QString() );
  QCOMPARE( cb->lineEdit()->selectedText(), allowNull ? QStringLiteral( "nope" ) : QString() );
  QVERIFY( ! cb->mLineEdit->mClearAction );

  //check if text is selected after receiving focus
  cb->setFocus();
  QCOMPARE( cb->currentIndex(), allowNull ? cb->nullIndex() : 0 );
  QCOMPARE( cb->currentText(), allowNull ? QStringLiteral( "nope" ) : QString() );
  QCOMPARE( cb->lineEdit()->selectedText(), allowNull ? QStringLiteral( "nope" ) : QString() );
  QVERIFY( ! cb->mLineEdit->mClearAction );

  //check with another entry, clear button needs to be there then:
  QTest::keyClicks( cb.get(), QStringLiteral( "sleeve" ) );
  spy.wait();
  QCOMPARE( cb->currentText(), QStringLiteral( "sleeve" ) );
  QVERIFY( cb->mLineEdit->mClearAction );
}

void TestQgsFeatureListComboBox::nullRepresentation()
{
  QgsApplication::setNullRepresentation( QStringLiteral( "nope" ) );
  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );

  QgsFeatureFilterModel *model = qobject_cast<QgsFeatureFilterModel *>( cb->model() );
  QEventLoop loop;
  connect( model, &QgsFeatureFilterModel::filterJobCompleted, &loop, &QEventLoop::quit );

  cb->setAllowNull( true );
  cb->setSourceLayer( mLayer.get() );

  loop.exec();
  QCOMPARE( cb->lineEdit()->text(), QStringLiteral( "nope" ) );
  QCOMPARE( cb->nullIndex(), 0 );
}


void TestQgsFeatureListComboBox::testNotExistingYetFeature()
{
  // test behavior when feature list combo box identifier values references a
  // not existing yet feature (created but not saved for instance)

  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );
  QgsFeatureFilterModel *model = qobject_cast<QgsFeatureFilterModel *>( cb->model() );
  QEventLoop loop;
  connect( model, &QgsFeatureFilterModel::filterJobCompleted, &loop, &QEventLoop::quit );

  QgsApplication::setNullRepresentation( QStringLiteral( "nope" ) );

  QVERIFY( cb->identifierValues().isEmpty() );

  cb->setSourceLayer( mLayer.get() );
  cb->setAllowNull( true );

  cb->setIdentifierValues( QVariantList() << 42 );

  loop.exec();
  QCOMPARE( cb->currentText(), QStringLiteral( "(42)" ) );
}

void TestQgsFeatureListComboBox::testFeatureFurtherThanFetchLimit()
{
  const int fetchLimit = 20;
  QVERIFY( fetchLimit < mLayer->featureCount() );
  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );
  QgsFeatureFilterModel *model = qobject_cast<QgsFeatureFilterModel *>( cb->model() );
  QSignalSpy spy( cb.get(), &QgsFeatureListComboBox::identifierValueChanged );
  model->setFetchLimit( 20 );
  model->setAllowNull( false );
  cb->setSourceLayer( mLayer.get() );
  cb->setIdentifierFields( {QStringLiteral( "pk" )} );
  spy.wait();
  QCOMPARE( model->mEntries.count(), 20 );
  for ( int i = 0; i < 20; i++ )
    QCOMPARE( model->mEntries.at( i ).identifierFields.at( 0 ).toInt(), i + 10 );
  cb->setIdentifierValues( {33} );
  spy.wait();
  QCOMPARE( cb->lineEdit()->text(), QStringLiteral( "33" ) );
  QCOMPARE( model->mEntries.count(), 21 );
  QCOMPARE( model->mEntries.at( 0 ).identifierFields.at( 0 ).toInt(), 33 );
}

QGSTEST_MAIN( TestQgsFeatureListComboBox )
#include "testqgsfeaturelistcombobox.moc"
