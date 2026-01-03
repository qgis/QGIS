/***************************************************************************
    testqgsfieldexpressionwidget.cpp
     --------------------------------------
    Date                 : January 2016
    Copyright            : (C) 2016 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
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
#include <QSignalSpy>

//qgis includes...
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsvectorlayerjoinbuffer.h>
#include <qgsfieldexpressionwidget.h>
#include <qgsproject.h>

/**
 * This is a unit test for the field expression widget
 *
 * \see QgsFieldExpressionWidget
 */
class TestQgsFieldExpressionWidget : public QObject
{
    Q_OBJECT

  public:
    TestQgsFieldExpressionWidget() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testRemoveJoin();
    void asExpression();
    void testIsValid();
    void testFilters();
    void setNull();
    void testVeryLongExpression();

  private:
    QgsFieldExpressionWidget *mWidget = nullptr;
    QgsVectorLayer *mLayerA = nullptr;
    QgsVectorLayer *mLayerB = nullptr;
};

// runs before all tests
void TestQgsFieldExpressionWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  // Create memory layers
  // LAYER A //
  mLayerA = new QgsVectorLayer( u"Point?field=id_a:integer"_s, u"A"_s, u"memory"_s );
  QVERIFY( mLayerA->isValid() );
  QVERIFY( mLayerA->fields().count() == 1 );
  QgsProject::instance()->addMapLayer( mLayerA );
  // LAYER B //
  mLayerB = new QgsVectorLayer( u"Point?field=id_b:integer&field=value_b"_s, u"B"_s, u"memory"_s );
  QVERIFY( mLayerB->isValid() );
  QVERIFY( mLayerB->fields().count() == 2 );
  QgsProject::instance()->addMapLayer( mLayerB );

  // init widget
  mWidget = new QgsFieldExpressionWidget();
  mWidget->setLayer( mLayerA );
}

void TestQgsFieldExpressionWidget::init()
{
}

void TestQgsFieldExpressionWidget::cleanup()
{
}

void TestQgsFieldExpressionWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsFieldExpressionWidget::testRemoveJoin()
{
  QVERIFY( mLayerA->fields().count() == 1 );

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( u"id_a"_s );
  joinInfo.setJoinLayer( mLayerB );
  joinInfo.setJoinFieldName( u"id_b"_s );
  joinInfo.setUsingMemoryCache( false );
  joinInfo.setPrefix( u"B_"_s );
  mLayerA->addJoin( joinInfo );

  QVERIFY( mLayerA->fields().count() == 2 );

  const QString expr = u"'hello '|| B_value_b"_s;
  mWidget->setField( expr );

  bool isExpression, isValid;
  QVERIFY( mWidget->isValidExpression() );
  QCOMPARE( mWidget->currentField( &isExpression, &isValid ), expr );
  QVERIFY( isExpression );
  QVERIFY( isValid );

  QVERIFY( mLayerA->removeJoin( mLayerB->id() ) );

  QVERIFY( mLayerA->fields().count() == 1 );

  QCOMPARE( mWidget->mCombo->currentText(), expr );

  QCOMPARE( mWidget->currentField( &isExpression, &isValid ), expr );
  QVERIFY( isExpression );
  // QVERIFY( !isValid ); TODO: the expression should not be valid anymore since the field doesn't exist anymore. Maybe we need a new expression method to get more details.
}

void TestQgsFieldExpressionWidget::asExpression()
{
  QgsVectorLayer *layer = new QgsVectorLayer( u"point?field=fld:int&field=fld2:int&field=fld3:int"_s, u"x"_s, u"memory"_s );
  layer->dataProvider()->addAttributes( QList<QgsField>() << QgsField( u"a space"_s, QMetaType::Type::QString ) );
  layer->updateFields();
  QgsProject::instance()->addMapLayer( layer );

  auto widget = std::make_unique<QgsFieldExpressionWidget>();
  widget->setLayer( layer );

  const QSignalSpy spy( widget.get(), static_cast<void ( QgsFieldExpressionWidget::* )( const QString & )>( &QgsFieldExpressionWidget::fieldChanged ) );
  const QSignalSpy spy2( widget.get(), static_cast<void ( QgsFieldExpressionWidget::* )( const QString &, bool )>( &QgsFieldExpressionWidget::fieldChanged ) );

  // check with field set
  widget->setField( u"fld"_s );
  QCOMPARE( widget->asExpression(), u"\"fld\""_s );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), u"fld"_s );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), u"fld"_s );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  // check with expressions set
  widget->setField( u"fld + 1"_s );
  QCOMPARE( widget->asExpression(), u"fld + 1"_s );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), u"fld + 1"_s );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), u"fld + 1"_s );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  widget->setField( u"1"_s );
  QCOMPARE( widget->asExpression(), u"1"_s );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), u"1"_s );
  QCOMPARE( spy2.count(), 3 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), u"1"_s );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  widget->setField( u"\"fld2\""_s );
  QCOMPARE( widget->asExpression(), u"\"fld2\""_s );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), u"fld2"_s );
  QCOMPARE( spy2.count(), 4 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), u"fld2"_s );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  // check switching back to a field
  widget->setField( u"fld3"_s );
  QCOMPARE( widget->asExpression(), u"\"fld3\""_s );
  QCOMPARE( spy.count(), 5 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), u"fld3"_s );
  QCOMPARE( spy2.count(), 5 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), u"fld3"_s );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  // and back to null
  widget->setField( QString() );
  QVERIFY( widget->asExpression().isEmpty() );
  QCOMPARE( spy.count(), 6 );
  QVERIFY( spy.constLast().at( 0 ).toString().isEmpty() );
  QCOMPARE( spy2.count(), 6 );
  QVERIFY( spy2.constLast().at( 0 ).toString().isEmpty() );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  // field name with space
  widget->setField( u"a space"_s );
  QCOMPARE( widget->asExpression(), u"\"a space\""_s );
  bool isExpression = true;
  QCOMPARE( widget->currentField( &isExpression ), u"a space"_s );
  QVERIFY( !isExpression );
  QCOMPARE( spy.count(), 7 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), u"a space"_s );
  QCOMPARE( spy2.count(), 7 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), u"a space"_s );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  widget->setField( QString() );
  QVERIFY( widget->asExpression().isEmpty() );
  widget->setExpression( u"\"a space\""_s );
  QCOMPARE( widget->asExpression(), u"\"a space\""_s );
  isExpression = true;
  QCOMPARE( widget->currentField( &isExpression ), u"a space"_s );
  QVERIFY( !isExpression );
  QCOMPARE( spy.count(), 9 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), u"a space"_s );
  QCOMPARE( spy2.count(), 9 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), u"a space"_s );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  QgsProject::instance()->removeMapLayer( layer );
}

void TestQgsFieldExpressionWidget::testIsValid()
{
  QgsVectorLayer *layer = new QgsVectorLayer( u"point?field=fld:int&field=name%20with%20space:string"_s, u"x"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( layer );

  auto widget = std::make_unique<QgsFieldExpressionWidget>();
  widget->setLayer( layer );

  // also check the fieldChanged signal to ensure that the emitted bool isValid value is correct
  QSignalSpy spy( widget.get(), SIGNAL( fieldChanged( QString, bool ) ) );

  // check with simple field name set
  bool isExpression = false;
  bool isValid = false;
  widget->setField( u"fld"_s );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), u"fld"_s );
  QVERIFY( !isExpression );
  QVERIFY( isValid );
  QVERIFY( widget->isValidExpression() );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toString(), u"fld"_s );
  QVERIFY( spy.last().at( 1 ).toBool() );


  //check with complex field name set
  widget->setField( u"name with space"_s );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), u"name with space"_s );
  QVERIFY( !isExpression );
  QVERIFY( isValid );
  QVERIFY( !widget->isValidExpression() );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toString(), u"name with space"_s );
  QVERIFY( spy.last().at( 1 ).toBool() );

  //check with valid expression set
  widget->setField( u"2 * 4"_s );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), u"2 * 4"_s );
  QVERIFY( isExpression );
  QVERIFY( isValid );
  QVERIFY( widget->isValidExpression() );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.last().at( 0 ).toString(), u"2 * 4"_s );
  QVERIFY( spy.last().at( 1 ).toBool() );

  //check with invalid expression set
  widget->setField( u"2 *"_s );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), u"2 *"_s );
  QVERIFY( isExpression );
  QVERIFY( !isValid );
  QVERIFY( !widget->isValidExpression() );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( spy.last().at( 0 ).toString(), u"2 *"_s );
  QVERIFY( !spy.last().at( 1 ).toBool() );

  QgsProject::instance()->removeMapLayer( layer );
}

void TestQgsFieldExpressionWidget::testFilters()
{
  QgsVectorLayer *layer = new QgsVectorLayer( u"point?field=intfld:int&field=stringfld:string&field=string2fld:string&field=longfld:long&field=doublefld:double&field=datefld:date&field=timefld:time&field=datetimefld:datetime&field=binaryfld:binary&field=booleanfld:boolean"_s, u"x"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( layer );

  auto widget = std::make_unique<QgsFieldExpressionWidget>();
  widget->setLayer( layer );

  QCOMPARE( widget->mCombo->count(), 10 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"intfld"_s );
  QCOMPARE( widget->mCombo->itemText( 1 ), u"stringfld"_s );
  QCOMPARE( widget->mCombo->itemText( 2 ), u"string2fld"_s );
  QCOMPARE( widget->mCombo->itemText( 3 ), u"longfld"_s );
  QCOMPARE( widget->mCombo->itemText( 4 ), u"doublefld"_s );
  QCOMPARE( widget->mCombo->itemText( 5 ), u"datefld"_s );
  QCOMPARE( widget->mCombo->itemText( 6 ), u"timefld"_s );
  QCOMPARE( widget->mCombo->itemText( 7 ), u"datetimefld"_s );
  QCOMPARE( widget->mCombo->itemText( 8 ), u"binaryfld"_s );
  QCOMPARE( widget->mCombo->itemText( 9 ), u"booleanfld"_s );

  widget->setFilters( QgsFieldProxyModel::String );
  QCOMPARE( widget->mCombo->count(), 2 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"stringfld"_s );
  QCOMPARE( widget->mCombo->itemText( 1 ), u"string2fld"_s );

  widget->setFilters( QgsFieldProxyModel::Int );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"intfld"_s );

  widget->setFilters( QgsFieldProxyModel::LongLong );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"longfld"_s );

  widget->setFilters( QgsFieldProxyModel::Double );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"doublefld"_s );

  widget->setFilters( QgsFieldProxyModel::Numeric );
  QCOMPARE( widget->mCombo->count(), 3 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"intfld"_s );
  QCOMPARE( widget->mCombo->itemText( 1 ), u"longfld"_s );
  QCOMPARE( widget->mCombo->itemText( 2 ), u"doublefld"_s );

  widget->setFilters( QgsFieldProxyModel::Date );
  QCOMPARE( widget->mCombo->count(), 2 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"datefld"_s );
  QCOMPARE( widget->mCombo->itemText( 1 ), u"datetimefld"_s );

  widget->setFilters( QgsFieldProxyModel::Time );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"timefld"_s );

  widget->setFilters( QgsFieldProxyModel::DateTime );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"datetimefld"_s );

  widget->setFilters( QgsFieldProxyModel::Binary );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"binaryfld"_s );

  widget->setFilters( QgsFieldProxyModel::Boolean );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), u"booleanfld"_s );

  QgsProject::instance()->removeMapLayer( layer );
}

void TestQgsFieldExpressionWidget::setNull()
{
  // test that QgsFieldExpressionWidget can be set to an empty value
  QgsVectorLayer *layer = new QgsVectorLayer( u"point?field=fld:int&field=fld2:int&field=fld3:int"_s, u"x"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( layer );

  auto widget = std::make_unique<QgsFieldExpressionWidget>();
  widget->setLayer( layer );

  widget->setField( QString() );
  QVERIFY( widget->currentField().isEmpty() );

  widget->setField( u"fld2"_s );
  QCOMPARE( widget->currentField(), u"fld2"_s );

  widget->setField( QString() );
  QVERIFY( widget->currentField().isEmpty() );

  QgsProject::instance()->removeMapLayer( layer );
}

void TestQgsFieldExpressionWidget::testVeryLongExpression()
{
  QString veryLongExpression;
  for ( int i = 0; i < 32770; i++ )
  {
    veryLongExpression += "a";
  }

  mWidget->setExpression( veryLongExpression );
  QCOMPARE( veryLongExpression.size(), mWidget->currentText().size() );
};


QGSTEST_MAIN( TestQgsFieldExpressionWidget )
#include "testqgsfieldexpressionwidget.moc"
