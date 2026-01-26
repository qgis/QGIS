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

#include <memory>

#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsdoublespinbox.h"
#include "qgsfilterlineedit.h"
#include "qgslogger.h"
#include "qgsrangeconfigdlg.h"
#include "qgsrangewidgetwrapper.h"
#include "qgsspinbox.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QLineEdit>
#include <QObject>
#include <QtTest/QSignalSpy>

#define SPECIAL_TEXT_WHEN_EMPTY QString( QChar( 0x2063 ) )

/**
 * This is a unit test for the range widget
 *
 * \see QgsRangeWidgetWrapper
 */
class TestQgsRangeWidgetWrapper : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void test_setDoubleRange();
    void test_setDoubleSmallerRange();
    void test_setDoubleLimits();
    void test_doubleExplicitPrecision();
    void test_nulls();
    void test_negativeIntegers(); // see GH issue #32149
    void test_focus();
    void testLongLong();

  private:
    std::unique_ptr<QgsRangeWidgetWrapper> widget0; // For field 0
    std::unique_ptr<QgsRangeWidgetWrapper> widget1; // For field 1
    std::unique_ptr<QgsRangeWidgetWrapper> widget2; // For field 2
    std::unique_ptr<QgsRangeWidgetWrapper> widget3; // For field 3
    std::unique_ptr<QgsVectorLayer> vl;
};

void TestQgsRangeWidgetWrapper::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST-RANGE-WIDGET"_s );

  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsRangeWidgetWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRangeWidgetWrapper::init()
{
  vl = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326"_s, u"myvl"_s, "memory"_L1 );

  // add fields
  QList<QgsField> fields;
  fields.append( QgsField( "id", QMetaType::Type::Int ) );
  // precision = 9
  QgsField dfield( "number", QMetaType::Type::Double );
  dfield.setPrecision( 9 );
  fields.append( dfield );
  // default precision = 0
  const QgsField dfield2( "number_def", QMetaType::Type::Double );
  fields.append( dfield2 );
  // simple int
  fields.append( QgsField( "simplenumber", QMetaType::Type::Int ) );
  fields.append( QgsField( "longlong", QMetaType::Type::LongLong ) );
  vl->dataProvider()->addAttributes( fields );
  vl->updateFields();
  QVERIFY( vl.get() );
  QVERIFY( vl->isValid() );
  // Add feature 1:1:123.123456789:123.123456789:NULL:POINT( 1 1 )
  QgsFeature feat1( vl->fields(), 1 );
  feat1.setGeometry( QgsGeometry::fromWkt( u"POINT( 1 1 )"_s ) );
  feat1.setAttribute( u"id"_s, 1 );
  feat1.setAttribute( u"number"_s, 123.123456789 );
  feat1.setAttribute( u"number_def"_s, 123.123456789 );
  vl->dataProvider()->addFeature( feat1 );
  // Add feature 2:2:NULL:NULL:NULL:POINT( 2 2 )
  QgsFeature feat2( vl->fields(), 2 );
  feat2.setGeometry( QgsGeometry::fromWkt( u"POINT( 2 2 )"_s ) );
  feat2.setAttribute( u"id"_s, 2 );
  vl->dataProvider()->addFeature( feat2 );
  // Add feature 3:3:-123.123456789:-123.123456789:NULL:POINT( 3 3 )
  QgsFeature feat3( vl->fields(), 3 );
  feat3.setGeometry( QgsGeometry::fromWkt( u"POINT( 3 3 )"_s ) );
  feat3.setAttribute( u"number"_s, -123.123456789 );
  feat3.setAttribute( u"number_def"_s, -123.123456789 );
  feat3.setAttribute( u"id"_s, 3 );
  vl->dataProvider()->addFeature( feat3 );
  // Verify feat 1 was added
  QCOMPARE( vl->featureCount(), ( long ) 3 );
  const QgsFeature _feat1( vl->getFeature( 1 ) );
  QCOMPARE( _feat1, feat1 );
  widget0 = std::make_unique<QgsRangeWidgetWrapper>( vl.get(), 0, nullptr, nullptr );
  widget1 = std::make_unique<QgsRangeWidgetWrapper>( vl.get(), 1, nullptr, nullptr );
  widget2 = std::make_unique<QgsRangeWidgetWrapper>( vl.get(), 2, nullptr, nullptr );
  widget3 = std::make_unique<QgsRangeWidgetWrapper>( vl.get(), 3, nullptr, nullptr );
  QVERIFY( widget1.get() );
}

void TestQgsRangeWidgetWrapper::cleanup()
{
}

void TestQgsRangeWidgetWrapper::test_setDoubleRange()
{
  // Test setting scale range with doubles and NULL values, default range
  // See https://github.com/qgis/QGIS/issues/25773
  // QGIS 3 Vector Layer Fields Garbled when Clicking the Toggle Editing Icon

  QgsDoubleSpinBox *editor = qobject_cast<QgsDoubleSpinBox *>( widget1->createWidget( nullptr ) );
  QVERIFY( editor );
  widget1->initWidget( editor );
  QgsDoubleSpinBox *editor2 = qobject_cast<QgsDoubleSpinBox *>( widget2->createWidget( nullptr ) );
  QVERIFY( editor2 );
  widget2->initWidget( editor2 );

  const QgsFeature feat( vl->getFeature( 1 ) );
  QVERIFY( feat.isValid() );
  QCOMPARE( feat.attribute( 1 ).toDouble(), 123.123456789 );
  widget1->setFeature( vl->getFeature( 1 ) );
  widget2->setFeature( vl->getFeature( 1 ) );
  QCOMPARE( vl->fields().at( 1 ).precision(), 9 );
  // Field precision is 0, which from the QgsField::precision docs indicates "not applicable" to this field.
  // That is correct, as memory layers do not support field precision for real
  QCOMPARE( vl->fields().at( 2 ).precision(), 0 );
  QCOMPARE( editor->decimals(), 9 );
  // for double fields without explicit precision, we should default to a reasonable, non-zero value (currently hardcoded to 4)
  QCOMPARE( editor2->decimals(), 4 );
  QCOMPARE( editor->valueFromText( feat.attribute( 1 ).toString() ), 123.123456789 );
  QCOMPARE( feat.attribute( 1 ).toString(), u"123.123456789"_s );
  QCOMPARE( editor2->valueFromText( feat.attribute( 1 ).toString() ), 123.123456789 );
  QCOMPARE( editor->value(), 123.123456789 );
  QCOMPARE( editor2->value(), 123.1235 );
  QCOMPARE( editor->minimum(), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor2->minimum(), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor->maximum(), std::numeric_limits<double>::max() );
  QCOMPARE( editor2->maximum(), std::numeric_limits<double>::max() );

  widget1->setFeature( vl->getFeature( 2 ) );
  widget2->setFeature( vl->getFeature( 2 ) );
  QCOMPARE( editor->value(), editor->minimum() );
  QCOMPARE( editor2->value(), editor->minimum() );

  widget1->setFeature( vl->getFeature( 3 ) );
  widget2->setFeature( vl->getFeature( 3 ) );
  QCOMPARE( editor->value(), -123.123456789 );
  QCOMPARE( editor2->value(), -123.1235 );
}

void TestQgsRangeWidgetWrapper::test_setDoubleSmallerRange()
{
  // Same test but we set a smaller validity range for the widget
  QVariantMap cfg;
  cfg.insert( u"Min"_s, -100.0 );
  cfg.insert( u"Max"_s, 100.0 );
  cfg.insert( u"Step"_s, 1 );
  widget1->setConfig( cfg );
  QgsDoubleSpinBox *editor = qobject_cast<QgsDoubleSpinBox *>( widget1->createWidget( nullptr ) );
  QVERIFY( editor );
  widget1->initWidget( editor );

  widget2->setConfig( cfg );
  QgsDoubleSpinBox *editor2 = qobject_cast<QgsDoubleSpinBox *>( widget2->createWidget( nullptr ) );
  QVERIFY( editor2 );
  widget2->initWidget( editor2 );

  const QgsFeature feat( vl->getFeature( 1 ) );
  QVERIFY( feat.isValid() );
  QCOMPARE( feat.attribute( 1 ).toDouble(), 123.123456789 );
  widget1->setFeature( vl->getFeature( 1 ) );
  widget2->setFeature( vl->getFeature( 1 ) );

  QCOMPARE( vl->fields().at( 1 ).precision(), 9 );
  // Field precision is 0, which from the QgsField::precision docs indicates "not applicable" to this field.
  // That is correct, as memory layers do not support field precision for real
  QCOMPARE( vl->fields().at( 2 ).precision(), 0 );
  QCOMPARE( editor->decimals(), vl->fields().at( 1 ).precision() );
  // for double fields without explicit precision, we should default to a reasonable, non-zero value (currently hardcoded to 4)
  QCOMPARE( editor2->decimals(), 4 );
  // value was changed to the maximum (not NULL) accepted value
  QCOMPARE( editor->value(), 100.0 );
  // value was changed to the maximum (not NULL) accepted value
  QCOMPARE( editor2->value(), 100.0 );
  // minimum was lowered by the precision (10e-9)
  QCOMPARE( editor->minimum(), -100.000000001 );
  // minimum was lowered by step (1)
  QCOMPARE( editor2->minimum(), ( double ) -100.0001 );
  QCOMPARE( editor->maximum(), ( double ) 100 );
  QCOMPARE( editor2->maximum(), ( double ) 100 );

  // NULL, NULL
  widget1->setFeature( vl->getFeature( 2 ) );
  widget2->setFeature( vl->getFeature( 2 ) );
  QCOMPARE( editor->value(), editor->minimum() );
  QCOMPARE( editor2->value(), editor2->minimum() );

  // negative, negative
  widget1->setFeature( vl->getFeature( 3 ) );
  widget2->setFeature( vl->getFeature( 3 ) );
  // value was changed to the minimum
  QCOMPARE( editor->value(), editor->minimum() );
  QCOMPARE( editor2->value(), editor2->minimum() );
}

void TestQgsRangeWidgetWrapper::test_setDoubleLimits()
{
  // Same test but check double numeric limits
  QVariantMap cfg;
  cfg.insert( u"Min"_s, std::numeric_limits<double>::lowest() );
  cfg.insert( u"Max"_s, std::numeric_limits<double>::max() );
  cfg.insert( u"Step"_s, 1 );
  widget1->setConfig( cfg );
  QgsDoubleSpinBox *editor = qobject_cast<QgsDoubleSpinBox *>( widget1->createWidget( nullptr ) );
  QVERIFY( editor );
  widget1->initWidget( editor );

  widget2->setConfig( cfg );
  QgsDoubleSpinBox *editor2 = qobject_cast<QgsDoubleSpinBox *>( widget2->createWidget( nullptr ) );
  QVERIFY( editor2 );
  widget2->initWidget( editor2 );

  QCOMPARE( editor->minimum(), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor2->minimum(), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor->maximum(), std::numeric_limits<double>::max() );
  QCOMPARE( editor2->maximum(), std::numeric_limits<double>::max() );

  const QgsFeature feat( vl->getFeature( 1 ) );
  QVERIFY( feat.isValid() );
  QCOMPARE( feat.attribute( 1 ).toDouble(), 123.123456789 );
  widget1->setFeature( vl->getFeature( 1 ) );
  widget2->setFeature( vl->getFeature( 1 ) );

  QCOMPARE( vl->fields().at( 1 ).precision(), 9 );
  // Field precision is 0, which from the QgsField::precision docs indicates "not applicable" to this field.
  // That is correct, as memory layers do not support field precision for real
  QCOMPARE( vl->fields().at( 2 ).precision(), 0 );
  QCOMPARE( editor->decimals(), vl->fields().at( 1 ).precision() );
  // for double fields without explicit precision, we should default to a reasonable, non-zero value (currently hardcoded to 4)
  QCOMPARE( editor2->decimals(), 4 );
  QCOMPARE( editor->value(), 123.123456789 );
  QCOMPARE( editor2->value(), 123.1235 );

  // NULL, NULL
  widget1->setFeature( vl->getFeature( 2 ) );
  widget2->setFeature( vl->getFeature( 2 ) );
  QCOMPARE( editor->value(), editor->minimum() );
  QCOMPARE( editor2->value(), editor2->minimum() );

  // negative, negative
  widget1->setFeature( vl->getFeature( 3 ) );
  widget2->setFeature( vl->getFeature( 3 ) );
  // value was changed to the minimum
  QCOMPARE( editor->value(), -123.123456789 );
  QCOMPARE( editor2->value(), -123.1235 );
}

void TestQgsRangeWidgetWrapper::test_doubleExplicitPrecision()
{
  // Same test but check double numeric limits
  QVariantMap cfg;
  cfg.insert( u"Min"_s, std::numeric_limits<double>::lowest() );
  cfg.insert( u"Max"_s, std::numeric_limits<double>::max() );
  cfg.insert( u"Step"_s, 1 );
  // explicitly set precision for field
  cfg.insert( u"Precision"_s, 3 );
  widget1->setConfig( cfg );
  QgsDoubleSpinBox *editor = qobject_cast<QgsDoubleSpinBox *>( widget1->createWidget( nullptr ) );
  QVERIFY( editor );
  widget1->initWidget( editor );

  widget2->setConfig( cfg );
  QgsDoubleSpinBox *editor2 = qobject_cast<QgsDoubleSpinBox *>( widget2->createWidget( nullptr ) );
  QVERIFY( editor2 );
  widget2->initWidget( editor2 );

  QCOMPARE( editor->minimum(), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor2->minimum(), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor->maximum(), std::numeric_limits<double>::max() );
  QCOMPARE( editor2->maximum(), std::numeric_limits<double>::max() );

  const QgsFeature feat( vl->getFeature( 1 ) );
  QVERIFY( feat.isValid() );
  QCOMPARE( feat.attribute( 1 ).toDouble(), 123.123456789 );
  widget1->setFeature( vl->getFeature( 1 ) );
  widget2->setFeature( vl->getFeature( 1 ) );

  QCOMPARE( vl->fields().at( 1 ).precision(), 9 );
  // Field precision is 0, which from the QgsField::precision docs indicates "not applicable" to this field.
  // That is correct, as memory layers do not support field precision for real
  QCOMPARE( vl->fields().at( 2 ).precision(), 0 );
  QCOMPARE( editor->decimals(), 3 );
  QCOMPARE( editor2->decimals(), 3 );
  QCOMPARE( editor->value(), 123.123 );
  QCOMPARE( editor2->value(), 123.123 );

  // NULL, NULL
  widget1->setFeature( vl->getFeature( 2 ) );
  widget2->setFeature( vl->getFeature( 2 ) );
  QCOMPARE( editor->value(), editor->minimum() );
  QCOMPARE( editor2->value(), editor2->minimum() );

  // negative, negative
  widget1->setFeature( vl->getFeature( 3 ) );
  widget2->setFeature( vl->getFeature( 3 ) );
  // value was changed to the minimum
  QCOMPARE( editor->value(), -123.123 );
  QCOMPARE( editor2->value(), -123.123 );
}

void TestQgsRangeWidgetWrapper::test_nulls()
{
  QgsApplication::setNullRepresentation( QString( "" ) );

  QVariantMap cfg;
  cfg.insert( u"Min"_s, 100.00 );
  cfg.insert( u"Max"_s, 200.00 );
  cfg.insert( u"Step"_s, 1 );
  cfg.insert( u"Precision"_s, 0 );
  widget1->setConfig( cfg );
  QgsDoubleSpinBox *editor1 = qobject_cast<QgsDoubleSpinBox *>( widget1->createWidget( nullptr ) );
  QVERIFY( editor1 );
  widget1->initWidget( editor1 );
  // Out of range
  widget1->setFeature( vl->getFeature( 3 ) );
  QCOMPARE( editor1->value(), editor1->minimum() );
  QCOMPARE( widget1->value(), QgsVariantUtils::createNullVariant( QMetaType::Type::Double ) );
  widget1->setFeature( QgsFeature( vl->fields() ) );
  // Null
  QCOMPARE( editor1->value(), editor1->minimum() );
  QCOMPARE( widget1->value(), QgsVariantUtils::createNullVariant( QMetaType::Type::Double ) );
  QCOMPARE( editor1->mLineEdit->text(), SPECIAL_TEXT_WHEN_EMPTY );
  editor1->mLineEdit->setText( QString( "151%1" ).arg( SPECIAL_TEXT_WHEN_EMPTY ) );
  QCOMPARE( widget1->value().toInt(), 151 );
  editor1->mLineEdit->setText( QString( SPECIAL_TEXT_WHEN_EMPTY ).append( u"161"_s ) );
  QCOMPARE( widget1->value().toInt(), 161 );


  QgsSpinBox *editor0 = qobject_cast<QgsSpinBox *>( widget0->createWidget( nullptr ) );
  QVERIFY( editor0 );
  widget0->setConfig( cfg );
  widget0->initWidget( editor0 );
  // Out of range
  widget0->setFeature( vl->getFeature( 3 ) );
  QCOMPARE( editor0->value(), editor0->minimum() );
  QCOMPARE( widget0->value(), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );
  widget0->setFeature( QgsFeature( vl->fields() ) );
  // Null
  QCOMPARE( editor0->value(), editor0->minimum() );
  QCOMPARE( widget0->value(), QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );
  QCOMPARE( editor0->mLineEdit->text(), SPECIAL_TEXT_WHEN_EMPTY );

  editor0->mLineEdit->setText( QString( "150%1" ).arg( SPECIAL_TEXT_WHEN_EMPTY ) );
  QCOMPARE( widget0->value().toInt(), 150 );
  editor0->mLineEdit->setText( QString( SPECIAL_TEXT_WHEN_EMPTY ).append( u"160"_s ) );
  QCOMPARE( widget0->value().toInt(), 160 );
}

void TestQgsRangeWidgetWrapper::test_negativeIntegers()
{
  QgsApplication::setNullRepresentation( QString( "" ) );

  QVariantMap cfg;
  widget3->setConfig( cfg );

  QgsSpinBox *editor3 = qobject_cast<QgsSpinBox *>( widget3->createWidget( nullptr ) );
  QVERIFY( editor3 );
  widget3->initWidget( editor3 );

  QgsFeature feature { vl->getFeature( 3 ) };
  feature.setAttribute( 3, -12345 );

  widget3->setFeature( feature );
  QCOMPARE( widget3->value().toInt(), -12345 );

  cfg.insert( u"Min"_s, 10 );
  widget3->setConfig( cfg );
  widget3->initWidget( editor3 );
  widget3->setFeature( feature );
  QVERIFY( widget3->value().isNull() );
  QCOMPARE( widget3->value().toInt(), 0 );

  cfg.clear();
  cfg.insert( u"Min"_s, -12346 );
  widget3->setConfig( cfg );
  widget3->initWidget( editor3 );
  widget3->setFeature( feature );
  QCOMPARE( widget3->value().toInt(), -12345 );
}

void TestQgsRangeWidgetWrapper::test_focus()
{
  QgsApplication::setNullRepresentation( QString( "nope" ) );

  QWidget *w = new QWidget(); //required for focus events
  QApplication::setActiveWindow( w );

  QVariantMap cfg;
  cfg.insert( u"AllowNull"_s, true );

  //QgsDoubleSpinBox
  widget1->setConfig( cfg );
  QgsDoubleSpinBox *editor1 = qobject_cast<QgsDoubleSpinBox *>( widget1->createWidget( w ) );
  QVERIFY( editor1 );
  widget1->initWidget( editor1 );
  widget1->setValue( QgsVariantUtils::createNullVariant( QMetaType::Type::Double ) );

  //QgsDoubleSpinBox
  widget2->setConfig( cfg );
  QgsDoubleSpinBox *editor2 = qobject_cast<QgsDoubleSpinBox *>( widget2->createWidget( w ) );
  QVERIFY( editor2 );
  widget2->initWidget( editor2 );
  widget2->setValue( QgsVariantUtils::createNullVariant( QMetaType::Type::Double ) );

  //QgsSpinBox
  widget3->setConfig( cfg );
  QgsSpinBox *editor3 = qobject_cast<QgsSpinBox *>( widget3->createWidget( w ) );
  QVERIFY( editor3 );
  widget3->initWidget( editor3 );
  widget3->setValue( QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );

  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( !editor1->mLineEdit->hasFocus() );
  QVERIFY( !editor2->mLineEdit->hasFocus() );
  QVERIFY( !editor3->mLineEdit->hasFocus() );
  QCOMPARE( editor1->mLineEdit->text(), u"nope"_s );
  QCOMPARE( editor2->mLineEdit->text(), u"nope"_s );
  QCOMPARE( editor3->mLineEdit->text(), u"nope"_s );

  editor1->mLineEdit->setFocus();
  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( editor1->mLineEdit->hasFocus() );
  QVERIFY( !editor2->mLineEdit->hasFocus() );
  QVERIFY( !editor3->mLineEdit->hasFocus() );
  QCOMPARE( editor1->mLineEdit->text(), QString() );
  QCOMPARE( editor2->mLineEdit->text(), u"nope"_s );
  QCOMPARE( editor3->mLineEdit->text(), u"nope"_s );

  editor2->mLineEdit->setFocus();
  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( !editor1->mLineEdit->hasFocus() );
  QVERIFY( editor2->mLineEdit->hasFocus() );
  QVERIFY( !editor3->mLineEdit->hasFocus() );
  QCOMPARE( editor1->mLineEdit->text(), u"nope"_s );
  QCOMPARE( editor2->mLineEdit->text(), QString() );
  QCOMPARE( editor3->mLineEdit->text(), u"nope"_s );

  editor3->mLineEdit->setFocus();
  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( !editor1->mLineEdit->hasFocus() );
  QVERIFY( !editor2->mLineEdit->hasFocus() );
  QVERIFY( editor3->mLineEdit->hasFocus() );
  QCOMPARE( editor1->mLineEdit->text(), u"nope"_s );
  QCOMPARE( editor2->mLineEdit->text(), u"nope"_s );
  QCOMPARE( editor3->mLineEdit->text(), QString() );

  editor1->mLineEdit->setFocus();
  editor1->mLineEdit->setText( QString( "151.000000000" ) );
  QVERIFY( !widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( editor1->mLineEdit->hasFocus() );
  QVERIFY( !editor2->mLineEdit->hasFocus() );
  QVERIFY( !editor3->mLineEdit->hasFocus() );
  QCOMPARE( editor1->mLineEdit->text(), u"151.000000000"_s );
  QCOMPARE( editor2->mLineEdit->text(), u"nope"_s );
  QCOMPARE( editor3->mLineEdit->text(), u"nope"_s );

  editor2->mLineEdit->setFocus();
  QVERIFY( widget0->value().isNull() );
  QVERIFY( !widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( !editor1->mLineEdit->hasFocus() );
  QVERIFY( editor2->mLineEdit->hasFocus() );
  QVERIFY( !editor3->mLineEdit->hasFocus() );
  QCOMPARE( editor1->mLineEdit->text(), u"151.000000000"_s );
  QCOMPARE( editor2->mLineEdit->text(), QString() );
  QCOMPARE( editor3->mLineEdit->text(), u"nope"_s );
}

void TestQgsRangeWidgetWrapper::testLongLong()
{
  // test range widget with a long long field type
  auto wrapper = std::make_unique<QgsRangeWidgetWrapper>( vl.get(), 4, nullptr, nullptr );

  // should use a double spin box, as a integer spin box does not have sufficient range
  QgsDoubleSpinBox *editor = qobject_cast<QgsDoubleSpinBox *>( wrapper->createWidget( nullptr ) );
  QVERIFY( editor );
  wrapper->initWidget( editor );
  // no decimals, it's for long long value editing!
  QCOMPARE( editor->decimals(), 0 );
  QCOMPARE( editor->minimum(), std::numeric_limits<double>::lowest() );
  QCOMPARE( editor->maximum(), std::numeric_limits<double>::max() );

  wrapper->setValue( 1234567890123LL );

  // double spin box value should be lossless
  QCOMPARE( editor->value(), 1234567890123.0 );

  // wrapper value must be a long long type, not double
  QCOMPARE( wrapper->value(), 1234567890123LL );
}

QGSTEST_MAIN( TestQgsRangeWidgetWrapper )
#include "testqgsrangewidgetwrapper.moc"
