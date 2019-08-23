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
    void nullRepresentation();

  private:
    void waitForLoaded( QgsFeatureListComboBox *cb );

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
  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );

  Q_NOWARN_DEPRECATED_PUSH
  QVERIFY( cb->identifierValue().isNull() );

  cb->setSourceLayer( mLayer.get() );
  cb->setDisplayExpression( "\"material\"" );
  cb->lineEdit()->setText( "ro" );
  emit cb->lineEdit()->textChanged( "ro" );
  QVERIFY( cb->identifierValue().isNull() );

  waitForLoaded( cb.get() );

  QVERIFY( cb->identifierValue().isNull() );

  cb->setIdentifierValue( 20 );
  QCOMPARE( cb->identifierValue(), QVariant( 20 ) );
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
}

void TestQgsFeatureListComboBox::testAllowNull()
{
  //QVERIFY( false );
  // Note to self: implement this!
}

void TestQgsFeatureListComboBox::testValuesAndSelection()
{
  QgsApplication::setNullRepresentation( QStringLiteral( "nope" ) );
  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );

  cb->setSourceLayer( mLayer.get() );
  cb->setDisplayExpression( QStringLiteral( "\"raccord\"" ) );
  cb->setAllowNull( true );

  //check if everything is fine:
  waitForLoaded( cb.get() );
  QCOMPARE( cb->currentIndex(), cb->nullIndex() );
  QCOMPARE( cb->currentText(), QStringLiteral( "nope" ) );

  //check if text correct, selected and if the clear button disappeared:
  cb->mLineEdit->clearValue();
  waitForLoaded( cb.get() );
  QCOMPARE( cb->currentIndex(), cb->nullIndex() );
  QCOMPARE( cb->currentText(), QStringLiteral( "nope" ) );
  QCOMPARE( cb->lineEdit()->selectedText(), QStringLiteral( "nope" ) );
  QVERIFY( ! cb->mLineEdit->mClearAction );

  //check if text is selected after receiving focus
  cb->setFocus();
  waitForLoaded( cb.get() );
  QCOMPARE( cb->currentIndex(), cb->nullIndex() );
  QCOMPARE( cb->currentText(), QStringLiteral( "nope" ) );
  QCOMPARE( cb->lineEdit()->selectedText(), QStringLiteral( "nope" ) );
  QVERIFY( ! cb->mLineEdit->mClearAction );

  //check with another entry, clear button needs to be there then:
  QTest::keyClicks( cb.get(), QStringLiteral( "sleeve" ) );
  //QTest::keyClick(cb.get(), Qt::Key_Enter );
  waitForLoaded( cb.get() );
  QCOMPARE( cb->currentText(), QStringLiteral( "sleeve" ) );
  QVERIFY( cb->mLineEdit->mClearAction );
  //QVERIFY( cb->currentIndex() != cb->nullIndex());
  //QCOMPARE( cb->model()->data( cb->currentModelIndex() ).toString(), QStringLiteral( "sleeve" )  );
}

void TestQgsFeatureListComboBox::nullRepresentation()
{

  QgsApplication::setNullRepresentation( QStringLiteral( "nope" ) );
  std::unique_ptr<QgsFeatureListComboBox> cb( new QgsFeatureListComboBox() );
  cb->setAllowNull( true );

  QCOMPARE( cb->lineEdit()->text(), QStringLiteral( "nope" ) );
  QCOMPARE( cb->nullIndex(), 0 );

}

void TestQgsFeatureListComboBox::waitForLoaded( QgsFeatureListComboBox *cb )
{
  QgsFeatureFilterModel *model = qobject_cast<QgsFeatureFilterModel *>( cb->model() );

  // Wait
  while ( model->isLoading() )
  {}
}

QGSTEST_MAIN( TestQgsFeatureListComboBox )
#include "testqgsfeaturelistcombobox.moc"
