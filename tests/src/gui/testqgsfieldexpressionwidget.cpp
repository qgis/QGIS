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
 * @ingroup UnitTests
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
    void initTestCase();      // will be called before the first testfunction is executed.
    void cleanupTestCase();   // will be called after the last testfunction was executed.
    void init();              // will be called before each testfunction is executed.
    void cleanup();           // will be called after every testfunction.

    void testRemoveJoin();
    void asExpression();
    void testIsValid();
    void testFilters();
    void setNull();

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
  layer->dataProvider()->addAttributes( QList< QgsField >() << QgsField( QStringLiteral( "a space" ), QVariant::String ) );
  layer->updateFields();
  QgsProject::instance()->addMapLayer( layer );

  std::unique_ptr< QgsFieldExpressionWidget > widget( new QgsFieldExpressionWidget() );
  widget->setLayer( layer );

  const QSignalSpy spy( widget.get(), static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ) );
  const QSignalSpy spy2( widget.get(), static_cast < void ( QgsFieldExpressionWidget::* )( const QString &, bool ) >( &QgsFieldExpressionWidget::fieldChanged ) );

  // check with field set
  widget->setField( QStringLiteral( "fld" ) );
  QCOMPARE( widget->asExpression(), QStringLiteral( "\"fld\"" ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), QStringLiteral( "fld" ) );
  QCOMPARE( spy2.count(), 1 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), QStringLiteral( "fld" ) );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  // check with expressions set
  widget->setField( QStringLiteral( "fld + 1" ) );
  QCOMPARE( widget->asExpression(), QStringLiteral( "fld + 1" ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), QStringLiteral( "fld + 1" ) );
  QCOMPARE( spy2.count(), 2 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), QStringLiteral( "fld + 1" ) );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  widget->setField( QStringLiteral( "1" ) );
  QCOMPARE( widget->asExpression(), QStringLiteral( "1" ) );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), QStringLiteral( "1" ) );
  QCOMPARE( spy2.count(), 3 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), QStringLiteral( "1" ) );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  widget->setField( QStringLiteral( "\"fld2\"" ) );
  QCOMPARE( widget->asExpression(), QStringLiteral( "\"fld2\"" ) );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), QStringLiteral( "fld2" ) );
  QCOMPARE( spy2.count(), 4 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), QStringLiteral( "fld2" ) );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  // check switching back to a field
  widget->setField( QStringLiteral( "fld3" ) );
  QCOMPARE( widget->asExpression(), QStringLiteral( "\"fld3\"" ) );
  QCOMPARE( spy.count(), 5 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), QStringLiteral( "fld3" ) );
  QCOMPARE( spy2.count(), 5 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), QStringLiteral( "fld3" ) );
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
  widget->setField( QStringLiteral( "a space" ) );
  QCOMPARE( widget->asExpression(), QStringLiteral( "\"a space\"" ) );
  bool isExpression = true;
  QCOMPARE( widget->currentField( &isExpression ), QStringLiteral( "a space" ) );
  QVERIFY( !isExpression );
  QCOMPARE( spy.count(), 7 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), QStringLiteral( "a space" ) );
  QCOMPARE( spy2.count(), 7 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), QStringLiteral( "a space" ) );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

  widget->setField( QString() );
  QVERIFY( widget->asExpression().isEmpty() );
  widget->setExpression( QStringLiteral( "\"a space\"" ) );
  QCOMPARE( widget->asExpression(), QStringLiteral( "\"a space\"" ) );
  isExpression = true;
  QCOMPARE( widget->currentField( &isExpression ), QStringLiteral( "a space" ) );
  QVERIFY( !isExpression );
  QCOMPARE( spy.count(), 9 );
  QCOMPARE( spy.constLast().at( 0 ).toString(), QStringLiteral( "a space" ) );
  QCOMPARE( spy2.count(), 9 );
  QCOMPARE( spy2.constLast().at( 0 ).toString(), QStringLiteral( "a space" ) );
  QVERIFY( spy2.constLast().at( 1 ).toBool() );

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
  QCOMPARE( widget->currentField( &isExpression, &isValid ), QStringLiteral( "fld" ) );
  QVERIFY( !isExpression );
  QVERIFY( isValid );
  QVERIFY( widget->isValidExpression() );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.last().at( 0 ).toString(), QStringLiteral( "fld" ) );
  QVERIFY( spy.last().at( 1 ).toBool() );


  //check with complex field name set
  widget->setField( QStringLiteral( "name with space" ) );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), QStringLiteral( "name with space" ) );
  QVERIFY( !isExpression );
  QVERIFY( isValid );
  QVERIFY( !widget->isValidExpression() );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.last().at( 0 ).toString(), QStringLiteral( "name with space" ) );
  QVERIFY( spy.last().at( 1 ).toBool() );

  //check with valid expression set
  widget->setField( QStringLiteral( "2 * 4" ) );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), QStringLiteral( "2 * 4" ) );
  QVERIFY( isExpression );
  QVERIFY( isValid );
  QVERIFY( widget->isValidExpression() );
  QCOMPARE( spy.count(), 3 );
  QCOMPARE( spy.last().at( 0 ).toString(), QStringLiteral( "2 * 4" ) );
  QVERIFY( spy.last().at( 1 ).toBool() );

  //check with invalid expression set
  widget->setField( QStringLiteral( "2 *" ) );
  QCOMPARE( widget->currentField( &isExpression, &isValid ), QStringLiteral( "2 *" ) );
  QVERIFY( isExpression );
  QVERIFY( !isValid );
  QVERIFY( !widget->isValidExpression() );
  QCOMPARE( spy.count(), 4 );
  QCOMPARE( spy.last().at( 0 ).toString(), QStringLiteral( "2 *" ) );
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
  QCOMPARE( widget->mCombo->itemText( 0 ), QStringLiteral( "intfld" ) );
  QCOMPARE( widget->mCombo->itemText( 1 ), QStringLiteral( "stringfld" ) );
  QCOMPARE( widget->mCombo->itemText( 2 ), QStringLiteral( "string2fld" ) );
  QCOMPARE( widget->mCombo->itemText( 3 ), QStringLiteral( "longfld" ) );
  QCOMPARE( widget->mCombo->itemText( 4 ), QStringLiteral( "doublefld" ) );
  QCOMPARE( widget->mCombo->itemText( 5 ), QStringLiteral( "datefld" ) );
  QCOMPARE( widget->mCombo->itemText( 6 ), QStringLiteral( "timefld" ) );
  QCOMPARE( widget->mCombo->itemText( 7 ), QStringLiteral( "datetimefld" ) );

  widget->setFilters( QgsFieldProxyModel::String );
  QCOMPARE( widget->mCombo->count(), 2 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QStringLiteral( "stringfld" ) );
  QCOMPARE( widget->mCombo->itemText( 1 ), QStringLiteral( "string2fld" ) );

  widget->setFilters( QgsFieldProxyModel::Int );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QStringLiteral( "intfld" ) );

  widget->setFilters( QgsFieldProxyModel::LongLong );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QStringLiteral( "longfld" ) );

  widget->setFilters( QgsFieldProxyModel::Double );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QStringLiteral( "doublefld" ) );

  widget->setFilters( QgsFieldProxyModel::Numeric );
  QCOMPARE( widget->mCombo->count(), 3 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QStringLiteral( "intfld" ) );
  QCOMPARE( widget->mCombo->itemText( 1 ), QStringLiteral( "longfld" ) );
  QCOMPARE( widget->mCombo->itemText( 2 ), QStringLiteral( "doublefld" ) );

  widget->setFilters( QgsFieldProxyModel::Date );
  QCOMPARE( widget->mCombo->count(), 2 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QStringLiteral( "datefld" ) );
  QCOMPARE( widget->mCombo->itemText( 1 ), QStringLiteral( "datetimefld" ) );

  widget->setFilters( QgsFieldProxyModel::Time );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QStringLiteral( "timefld" ) );

  widget->setFilters( QgsFieldProxyModel::DateTime );
  QCOMPARE( widget->mCombo->count(), 1 );
  QCOMPARE( widget->mCombo->itemText( 0 ), QStringLiteral( "datetimefld" ) );

  QgsProject::instance()->removeMapLayer( layer );
}

void TestQgsFieldExpressionWidget::setNull()
{
  // test that QgsFieldExpressionWidget can be set to an empty value
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "point?field=fld:int&field=fld2:int&field=fld3:int" ), QStringLiteral( "x" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( layer );

  std::unique_ptr< QgsFieldExpressionWidget > widget( new QgsFieldExpressionWidget() );
  widget->setLayer( layer );

  widget->setField( QString() );
  QVERIFY( widget->currentField().isEmpty() );

  widget->setField( QStringLiteral( "fld2" ) );
  QCOMPARE( widget->currentField(), QStringLiteral( "fld2" ) );

  widget->setField( QString() );
  QVERIFY( widget->currentField().isEmpty() );

  QgsProject::instance()->removeMapLayer( layer );
}

QGSTEST_MAIN( TestQgsFieldExpressionWidget )
#include "testqgsfieldexpressionwidget.moc"
