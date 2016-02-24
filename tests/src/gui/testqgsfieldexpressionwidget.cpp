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


#include <QtTest/QtTest>
#include <QObject>

//qgis includes...
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsvectorlayerjoinbuffer.h>
#include <qgsmaplayerregistry.h>
#include <qgsfieldexpressionwidget.h>
#include <qgsproject.h>

/** @ingroup UnitTests
 * This is a unit test for the field expression widget
 *
 * @see QgsFieldExpressionWidget
 */
class TestQgsFieldExpressionWidget : public QObject
{
    Q_OBJECT

  public:
    TestQgsFieldExpressionWidget()
        : mWidget( nullptr )
        , mLayerA( nullptr )
        , mLayerB( nullptr )
    {}

  private slots:
    void initTestCase();      // will be called before the first testfunction is executed.
    void cleanupTestCase();   // will be called after the last testfunction was executed.
    void init();              // will be called before each testfunction is executed.
    void cleanup();           // will be called after every testfunction.

    void testRemoveJoin();
    void asExpression();

  private:
    QgsFieldExpressionWidget* mWidget;
    QgsVectorLayer* mLayerA;
    QgsVectorLayer* mLayerB;
};

// runs before all tests
void TestQgsFieldExpressionWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  // Create memory layers
  // LAYER A //
  mLayerA = new QgsVectorLayer( "Point?field=id_a:integer", "A", "memory" );
  QVERIFY( mLayerA->isValid() );
  QVERIFY( mLayerA->fields().count() == 1 );
  QgsMapLayerRegistry::instance()->addMapLayer( mLayerA );
  // LAYER B //
  mLayerB = new QgsVectorLayer( "Point?field=id_b:integer&field=value_b", "B", "memory" );
  QVERIFY( mLayerB->isValid() );
  QVERIFY( mLayerB->fields().count() == 2 );
  QgsMapLayerRegistry::instance()->addMapLayer( mLayerB );

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

  QgsVectorJoinInfo joinInfo;
  joinInfo.targetFieldName = "id_a";
  joinInfo.joinLayerId = mLayerB->id();
  joinInfo.joinFieldName = "id_b";
  joinInfo.memoryCache = false;
  joinInfo.prefix = "B_";
  mLayerA->addJoin( joinInfo );

  QVERIFY( mLayerA->fields().count() == 2 );

  const QString expr = "'hello '|| B_value_b";
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
  QgsVectorLayer* layer = new QgsVectorLayer( "point?field=fld:int&field=fld2:int&field=fld3:int", "x", "memory" );
  QgsMapLayerRegistry::instance()->addMapLayer( layer );

  QScopedPointer< QgsFieldExpressionWidget > widget( new QgsFieldExpressionWidget() );
  widget->setLayer( layer );

  // check with field set
  widget->setField( "fld" );
  QCOMPARE( widget->asExpression(), QString( "\"fld\"" ) );

  // check with expressions set
  widget->setField( "fld + 1" );
  QCOMPARE( widget->asExpression(), QString( "fld + 1" ) );
  widget->setField( "1" );
  QCOMPARE( widget->asExpression(), QString( "1" ) );
  widget->setField( "\"fld2\"" );
  QCOMPARE( widget->asExpression(), QString( "\"fld2\"" ) );

  // check switching back to a field
  widget->setField( "fld3" );
  QCOMPARE( widget->asExpression(), QString( "\"fld3\"" ) );

  QgsMapLayerRegistry::instance()->removeMapLayer( layer );
}


QTEST_MAIN( TestQgsFieldExpressionWidget )
#include "testqgsfieldexpressionwidget.moc"


