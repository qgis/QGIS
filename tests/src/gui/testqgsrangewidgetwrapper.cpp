/***************************************************************************
                         testqgsrangewidget.cpp
                         ---------------------------
    begin                : Jan 2018
    copyright            : (C) 2018 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsrangewidgetwrapper.h"
#include "qgsrangeconfigdlg.h"
#include "qgsdoublespinbox.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsdataprovider.h"


#include <QObject>
#include <QtTest/QSignalSpy>

#include <memory>

/**
 * @ingroup UnitTests
 * This is a unit test for the range widget
 *
 * \see QgsRangeWidgetWrapper
 */
class TestQgsRangeWidgetWrapper : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void test_setDoubleRange();
    void test_setDoubleSmallerRange();
    void test_setDoubleLimits();
  private:
    std::unique_ptr<QgsRangeWidgetWrapper> widget; // For field 1
    std::unique_ptr<QgsRangeWidgetWrapper> widget2; // For field 2
    std::unique_ptr<QgsVectorLayer> vl;
};

void TestQgsRangeWidgetWrapper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsRangeWidgetWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRangeWidgetWrapper::init()
{
  vl = qgis::make_unique<QgsVectorLayer>( QStringLiteral( "Point?crs=epsg:4326" ),
                                          QStringLiteral( "myvl" ),
                                          QLatin1Literal( "memory" ) );

  // add fields
  QList<QgsField> fields;
  fields.append( QgsField( "id", QVariant::Int ) );
  // precision = 9
  QgsField dfield( "number",  QVariant::Double );
  dfield.setPrecision( 9 );
  fields.append( dfield );
  // default precision = 0
  QgsField dfield2( "number_def",  QVariant::Double );
  fields.append( dfield2 );
  vl->dataProvider()->addAttributes( fields );
  vl->updateFields();
  QVERIFY( vl.get() );
  QVERIFY( vl->isValid() );
  // Add feature 1:1:123.123456789:123.123456789:POINT( 1 1 )
  QgsFeature feat1( vl->fields(),  1 );
  feat1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 1 1 )" ) ) );
  feat1.setAttribute( QStringLiteral( "id" ), 1 );
  feat1.setAttribute( QStringLiteral( "number" ), 123.123456789 );
  feat1.setAttribute( QStringLiteral( "number_def" ), 123.123456789 );
  vl->dataProvider()->addFeature( feat1 );
  // Add feature 2:2:NULL:NULL:POINT( 2 2 )
  QgsFeature feat2( vl->fields(),  2 );
  feat2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 2 2 )" ) ) );
  feat2.setAttribute( QStringLiteral( "id" ), 2 );
  vl->dataProvider()->addFeature( feat2 );
  // Add feature 3:3:-123.123456789:-123.123456789:POINT( 3 3 )
  QgsFeature feat3( vl->fields(),  3 );
  feat3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 3 3 )" ) ) );
  feat3.setAttribute( QStringLiteral( "number" ), -123.123456789 );
  feat3.setAttribute( QStringLiteral( "number_def" ), -123.123456789 );
  feat3.setAttribute( QStringLiteral( "id" ), 3 );
  vl->dataProvider()->addFeature( feat3 );
  // Verify feat 1 was added
  QCOMPARE( vl->featureCount( ), ( long )3 );
  QgsFeature _feat1( vl->getFeature( 1 ) );
  QCOMPARE( _feat1, feat1 );
  widget = qgis::make_unique<QgsRangeWidgetWrapper>( vl.get(), 1, nullptr, nullptr );
  widget2 = qgis::make_unique<QgsRangeWidgetWrapper>( vl.get(), 2, nullptr, nullptr );
  QVERIFY( widget.get() );
}

void TestQgsRangeWidgetWrapper::cleanup()
{
}

void TestQgsRangeWidgetWrapper::test_setDoubleRange()
{
  // Test setting scale range with doubles and NULL values, default range
  // See https://issues.qgis.org/issues/17878
  // QGIS 3 Vector Layer Fields Garbled when Clicking the Toggle Editing Icon

  QgsDoubleSpinBox *editor = qobject_cast<QgsDoubleSpinBox *>( widget->createWidget( nullptr ) );
  QVERIFY( editor );
  widget->initWidget( editor );
  QgsDoubleSpinBox *editor2 = qobject_cast<QgsDoubleSpinBox *>( widget2->createWidget( nullptr ) );
  QVERIFY( editor2 );
  widget2->initWidget( editor2 );

  QgsFeature feat( vl->getFeature( 1 ) );
  QVERIFY( feat.isValid() );
  QCOMPARE( feat.attribute( 1 ).toDouble(), 123.123456789 );
  widget->setFeature( vl->getFeature( 1 ) );
  widget2->setFeature( vl->getFeature( 1 ) );
  QCOMPARE( vl->fields().at( 1 ).precision(), 9 );
  // Default is 0 !!! for double, really ?
  QCOMPARE( vl->fields().at( 2 ).precision(), 0 );
  QCOMPARE( editor->decimals(), vl->fields().at( 1 ).precision() );
  QCOMPARE( editor->decimals(), 9 );
  QCOMPARE( editor2->decimals(), vl->fields().at( 2 ).precision() );
  QCOMPARE( editor->valueFromText( feat.attribute( 1 ).toString() ),  123.123456789 );
  QCOMPARE( feat.attribute( 1 ).toString(), QStringLiteral( "123.123456789" ) );
  QCOMPARE( editor2->valueFromText( feat.attribute( 1 ).toString() ), 123.123456789 );
  QCOMPARE( editor->value( ), 123.123456789 );
  QCOMPARE( editor2->value( ), 123.0 );
  QCOMPARE( editor->minimum( ), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor2->minimum( ), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor->maximum( ), std::numeric_limits<double>::max() );
  QCOMPARE( editor2->maximum( ), std::numeric_limits<double>::max() );

  widget->setFeature( vl->getFeature( 2 ) );
  widget2->setFeature( vl->getFeature( 2 ) );
  QCOMPARE( editor->value( ), editor->minimum() );
  QCOMPARE( editor2->value( ), editor->minimum() );

  widget->setFeature( vl->getFeature( 3 ) );
  widget2->setFeature( vl->getFeature( 3 ) );
  QCOMPARE( editor->value( ), -123.123456789 );
  QCOMPARE( editor2->value( ), -123.0 );
}

void TestQgsRangeWidgetWrapper::test_setDoubleSmallerRange()
{
  // Same test but we set a smaller validity range for the widget
  QVariantMap cfg;
  cfg.insert( QStringLiteral( "Min" ), -100.0 );
  cfg.insert( QStringLiteral( "Max" ), 100.0 );
  cfg.insert( QStringLiteral( "Step" ), 1 );
  widget->setConfig( cfg );
  QgsDoubleSpinBox *editor = qobject_cast<QgsDoubleSpinBox *>( widget->createWidget( nullptr ) );
  QVERIFY( editor );
  widget->initWidget( editor );

  widget2->setConfig( cfg );
  QgsDoubleSpinBox *editor2 = qobject_cast<QgsDoubleSpinBox *>( widget2->createWidget( nullptr ) );
  QVERIFY( editor2 );
  widget2->initWidget( editor2 );

  QgsFeature feat( vl->getFeature( 1 ) );
  QVERIFY( feat.isValid() );
  QCOMPARE( feat.attribute( 1 ).toDouble(), 123.123456789 );
  widget->setFeature( vl->getFeature( 1 ) );
  widget2->setFeature( vl->getFeature( 1 ) );

  QCOMPARE( vl->fields().at( 1 ).precision(), 9 );
  // Default is 0 !!! for double, really ?
  QCOMPARE( vl->fields().at( 2 ).precision(), 0 );
  QCOMPARE( editor->decimals(), vl->fields().at( 1 ).precision() );
  QCOMPARE( editor2->decimals(), vl->fields().at( 2 ).precision() );
  // value was changed to the maximum (not NULL) accepted value
  QCOMPARE( editor->value( ), 100.0 );
  // value was changed to the maximum (not NULL) accepted value
  QCOMPARE( editor2->value( ), 100.0 );
  // minimum was lowered by the precision (10e-9)
  QCOMPARE( editor->minimum( ), -100.000000001 );
  // minimum was lowered by step (1)
  QCOMPARE( editor2->minimum( ), ( double ) - 101 );
  QCOMPARE( editor->maximum( ), ( double )100 );
  QCOMPARE( editor2->maximum( ), ( double )100 );

  // NULL, NULL
  widget->setFeature( vl->getFeature( 2 ) );
  widget2->setFeature( vl->getFeature( 2 ) );
  QCOMPARE( editor->value( ), editor->minimum() );
  QCOMPARE( editor2->value( ), editor2->minimum() );

  // negative, negative
  widget->setFeature( vl->getFeature( 3 ) );
  widget2->setFeature( vl->getFeature( 3 ) );
  // value was changed to the minimum
  QCOMPARE( editor->value( ), editor->minimum() );
  QCOMPARE( editor2->value( ), editor2->minimum() );

}

void TestQgsRangeWidgetWrapper::test_setDoubleLimits()
{
  // Same test but check double numeric limits
  QVariantMap cfg;
  cfg.insert( QStringLiteral( "Min" ), std::numeric_limits<double>::lowest() );
  cfg.insert( QStringLiteral( "Max" ), std::numeric_limits<double>::max() );
  cfg.insert( QStringLiteral( "Step" ), 1 );
  widget->setConfig( cfg );
  QgsDoubleSpinBox *editor = qobject_cast<QgsDoubleSpinBox *>( widget->createWidget( nullptr ) );
  QVERIFY( editor );
  widget->initWidget( editor );

  widget2->setConfig( cfg );
  QgsDoubleSpinBox *editor2 = qobject_cast<QgsDoubleSpinBox *>( widget2->createWidget( nullptr ) );
  QVERIFY( editor2 );
  widget2->initWidget( editor2 );

  QCOMPARE( editor->minimum( ), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor2->minimum( ), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor->maximum( ), std::numeric_limits<double>::max() );
  QCOMPARE( editor2->maximum( ), std::numeric_limits<double>::max() );

  QgsFeature feat( vl->getFeature( 1 ) );
  QVERIFY( feat.isValid() );
  QCOMPARE( feat.attribute( 1 ).toDouble(), 123.123456789 );
  widget->setFeature( vl->getFeature( 1 ) );
  widget2->setFeature( vl->getFeature( 1 ) );

  QCOMPARE( vl->fields().at( 1 ).precision(), 9 );
  // Default is 0 !!! for double, really ?
  QCOMPARE( vl->fields().at( 2 ).precision(), 0 );
  QCOMPARE( editor->decimals(), vl->fields().at( 1 ).precision() );
  QCOMPARE( editor2->decimals(), vl->fields().at( 2 ).precision() );
  QCOMPARE( editor->value( ), 123.123456789 );
  QCOMPARE( editor2->value( ), 123.0 );

  // NULL, NULL
  widget->setFeature( vl->getFeature( 2 ) );
  widget2->setFeature( vl->getFeature( 2 ) );
  QCOMPARE( editor->value( ), editor->minimum() );
  QCOMPARE( editor2->value( ), editor2->minimum() );

  // negative, negative
  widget->setFeature( vl->getFeature( 3 ) );
  widget2->setFeature( vl->getFeature( 3 ) );
  // value was changed to the minimum
  QCOMPARE( editor->value( ), -123.123456789 );
  QCOMPARE( editor2->value( ), -123.0 );

}



QGSTEST_MAIN( TestQgsRangeWidgetWrapper )
#include "testqgsrangewidgetwrapper.moc"
