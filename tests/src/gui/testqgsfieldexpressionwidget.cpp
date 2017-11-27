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

//qgis includes...
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsvectorlayerjoinbuffer.h>
#include <qgsfieldexpressionwidget.h>
#include <qgsproject.h>

/**
 * @ingroup UnitTests
 * This is a unit test for the field expression widget
 *
 * @see QgsFieldExpressionWidget
 */
class TestQgsFieldExpressionWidget : public QObject
{
    Q_OBJECT

  public:
    TestQgsFieldExpressionWidget() = default;

  private slots:
    void initTestCase();      // will be called before the first testfunction is executed.
    void cleanupTestCase();   // will be called after the last testfunction was executed.
    void init();              // will be called before each testfunction is executed.
    void cleanup();           // will be called after every testfunction.

    void testRemoveJoin();
    void asExpression();
    void testIsValid();
    void testFilters();

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
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  // Create memory layers
  // LAYER A //
  mLayerA = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "A" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerA->isValid() );
  QVERIFY( mLayerA->fields().count() == 1 );
  QgsProject::instance()->addMapLayer( mLayerA );
  // LAYER B //
  mLayerB = new QgsVectorLayer( QStringLiteral( "Point?field=id_b:integer&field=value_b" ), QStringLiteral( "B" ), QStringLiteral( "memory" ) );
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
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( mLayerB );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setUsingMemoryCache( false );
  joinInfo.setPrefix( QStringLiteral( "B_" ) );
  mLayerA->addJoin( joinInfo );

  QVERIFY( mLayerA->fields().count() == 2 );

  const QString expr = QStringLiteral( "'hello '|| B_value_b" );
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
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "point?field=fld:int&field=fld2:int&field=fld3:int" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( layer );

  std::unique_ptr< QgsFieldExpressionWidget > widget( new QgsFieldExpressionWidget() );
  widget->setLayer( layer );

  // check with field set
  widget->setField( QStringLiteral( "fld" ) );
  QCOMPARE( widget->asExpression(), QString( "\"fld\"" ) );

  // check with expressions set
  widget->setField( QStringLiteral( "fld + 1" ) );
  QCOMPARE( widget->asExpression(), QString( "fld + 1" ) );
  widget->setField( QStringLiteral( "1" ) );
  QCOMPARE( widget->asExpression(), QString( "1" ) );
  widget->setField( QStringLiteral( "\"fld2\"" ) );
  QCOMPARE( widget->asExpression(), QString( "\"fld2\"" ) );

  // check switching back to a field
  widget->setField( QStringLiteral( "fld3" ) );
  QCOMPARE( widget->asExpression(), QString( "\"fld3\"" ) );

  QgsProject::instance()->removeMapLayer( layer );
}

void TestQgsFieldExpressionWidget::testIsValid()
{
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "point?field=fld:int&field=name%20with%20space:string" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( layer );

  std::unique_ptr< QgsFieldExpressionWidget > widget( new QgsFieldExpressionWidget() );
  widget->setLayer( layer );

  // also check the fieldChanged signal to ensure that the emitted bool isValid value is correct
  QSignalSpy spy( widget.get(), SIGNAL( fieldChanged( QString, bool ) ) );

  // check with simple field name set
  bool isExpression = false;
  bool isValid = false;
  widget->setField( QStringLiteral( "fld" ) );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), QString( "fld" ) );
  QVERIFY( !isExpression );
  QVERIFY( isValid );
  QVERIFY( widget->isValidExpression() );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toString(), QString( "fld" ) );
  QVERIFY( spy.last().at( 1 ).toBool() );


  //check with complex field name set
  widget->setField( QStringLiteral( "name with space" ) );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), QString( "name with space" ) );
  QVERIFY( !isExpression );
  QVERIFY( isValid );
  QVERIFY( !widget->isValidExpression() );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toString(), QString( "name with space" ) );
  QVERIFY( spy.last().at( 1 ).toBool() );

  //check with valid expression set
  widget->setField( QStringLiteral( "2 * 4" ) );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), QString( "2 * 4" ) );
  QVERIFY( isExpression );
  QVERIFY( isValid );
  QVERIFY( widget->isValidExpression() );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.last().at( 0 ).toString(), QString( "2 * 4" ) );
  QVERIFY( spy.last().at( 1 ).toBool() );

  //check with invalid expression set
  widget->setField( QStringLiteral( "2 *" ) );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), QString( "2 *" ) );
  QVERIFY( isExpression );
  QVERIFY( !isValid );
  QVERIFY( !widget->isValidExpression() );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( spy.last().at( 0 ).toString(), QString( "2 *" ) );
  QVERIFY( !spy.last().at( 1 ).toBool() );

  QgsProject::instance()->removeMapLayer( layer );
}

void TestQgsFieldExpressionWidget::testFilters()
{
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "point?field=intfld:int&field=stringfld:string&field=string2fld:string&field=longfld:long&field=doublefld:double&field=datefld:date&field=timefld:time&field=datetimefld:datetime" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( layer );

  std::unique_ptr< QgsFieldExpressionWidget > widget( new QgsFieldExpressionWidget() );
  widget->setLayer( layer );

  QCOMPARE( widget->mCombo->count(), 8 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QString( "intfld" ) );
  QCOMPARE( widget->mCombo->itemText( 1 ), QString( "stringfld" ) );
  QCOMPARE( widget->mCombo->itemText( 2 ), QString( "string2fld" ) );
  QCOMPARE( widget->mCombo->itemText( 3 ), QString( "longfld" ) );
  QCOMPARE( widget->mCombo->itemText( 4 ), QString( "doublefld" ) );
  QCOMPARE( widget->mCombo->itemText( 5 ), QString( "datefld" ) );
  QCOMPARE( widget->mCombo->itemText( 6 ), QString( "timefld" ) );
  QCOMPARE( widget->mCombo->itemText( 7 ), QString( "datetimefld" ) );

  widget->setFilters( QgsFieldProxyModel::String );
  QCOMPARE( widget->mCombo->count(), 2 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QString( "stringfld" ) );
  QCOMPARE( widget->mCombo->itemText( 1 ), QString( "string2fld" ) );

  widget->setFilters( QgsFieldProxyModel::Int );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QString( "intfld" ) );

  widget->setFilters( QgsFieldProxyModel::LongLong );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QString( "longfld" ) );

  widget->setFilters( QgsFieldProxyModel::Double );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QString( "doublefld" ) );

  widget->setFilters( QgsFieldProxyModel::Numeric );
  QCOMPARE( widget->mCombo->count(), 3 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QString( "intfld" ) );
  QCOMPARE( widget->mCombo->itemText( 1 ), QString( "longfld" ) );
  QCOMPARE( widget->mCombo->itemText( 2 ), QString( "doublefld" ) );

  widget->setFilters( QgsFieldProxyModel::Date );
  QCOMPARE( widget->mCombo->count(), 2 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QString( "datefld" ) );
  QCOMPARE( widget->mCombo->itemText( 1 ), QString( "datetimefld" ) );

  widget->setFilters( QgsFieldProxyModel::Time );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QString( "timefld" ) );

  QgsProject::instance()->removeMapLayer( layer );
}

QGSTEST_MAIN( TestQgsFieldExpressionWidget )
#include "testqgsfieldexpressionwidget.moc"


