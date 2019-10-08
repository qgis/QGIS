/***************************************************************************
    testqgsdatetimeedit.cpp
     --------------------------------------
    Date                 : September 2019
    Copyright            : (C) 2019 Etienne Trimaille
    Email                : etienne dot trimaille at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"
#include "qdatetime.h"

#include <qgsdatetimeedit.h>
#include <qgsdatetimeeditwrapper.h>
#include <qgsdatetimefieldformatter.h>

class TestQgsDateTimeEdit: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void nullValues();
    void focus();

  private:
    std::unique_ptr<QgsDateTimeEditWrapper> widget1; // For field 0
    std::unique_ptr<QgsDateTimeEditWrapper> widget2; // For field 1
    std::unique_ptr<QgsDateTimeEditWrapper> widget3; // For field 2
    std::unique_ptr<QgsVectorLayer> vl;

};

void TestQgsDateTimeEdit::initTestCase()
{
}

void TestQgsDateTimeEdit::cleanupTestCase()
{
}

void TestQgsDateTimeEdit::init()
{

  vl = qgis::make_unique<QgsVectorLayer>( QStringLiteral( "Point?crs=epsg:4326" ),
                                          QStringLiteral( "myvl" ),
                                          QLatin1Literal( "memory" ) );

  // add fields
  QList<QgsField> fields;
  fields.append( QgsField( "date1", QVariant::Date ) );
  fields.append( QgsField( "date2", QVariant::Date ) );
  fields.append( QgsField( "date3", QVariant::Date ) );
  vl->dataProvider()->addAttributes( fields );
  vl->updateFields();
  QVERIFY( vl.get() );
  QVERIFY( vl->isValid() );

  widget1 = qgis::make_unique<QgsDateTimeEditWrapper>( vl.get(), 0, nullptr, nullptr );
  widget2 = qgis::make_unique<QgsDateTimeEditWrapper>( vl.get(), 1, nullptr, nullptr );
  widget3 = qgis::make_unique<QgsDateTimeEditWrapper>( vl.get(), 2, nullptr, nullptr );
  QVERIFY( widget1.get() );
  QVERIFY( widget2.get() );
  QVERIFY( widget3.get() );
}

void TestQgsDateTimeEdit::cleanup()
{
}

void TestQgsDateTimeEdit::nullValues()
{
  QgsDateTimeEdit *timeEdit = new QgsDateTimeEdit();

  // Allow null with a null datetime
  QVERIFY( timeEdit->allowNull() );
  timeEdit->setDateTime( QDateTime() );
  QCOMPARE( timeEdit->dateTime(), QDateTime() );
  QCOMPARE( timeEdit->time(), QTime() );
  QCOMPARE( timeEdit->date(), QDate() );

  // Not null with not null datetime
  QDateTime date( QDate( 2019, 7, 6 ), QTime( 8, 30, 0 ) );
  timeEdit->setAllowNull( false );
  QVERIFY( !timeEdit->allowNull() );
  timeEdit->setDateTime( date );
  QCOMPARE( timeEdit->dateTime(), date );
  QCOMPARE( timeEdit->time(), date.time() );
  QCOMPARE( timeEdit->date(), date.date() );

  // Not null with null date
  timeEdit->setAllowNull( false );
  QVERIFY( !timeEdit->allowNull() );
  timeEdit->setDateTime( QDateTime() );
  QCOMPARE( timeEdit->dateTime(), date );
  QCOMPARE( timeEdit->time(), date.time() );
  QCOMPARE( timeEdit->date(), date.date() );

  delete timeEdit;
}

void TestQgsDateTimeEdit::focus()
{
  QgsApplication::setNullRepresentation( QString( "nope" ) );
  QWidget *w = new QWidget(); //required for focus events
  QApplication::setActiveWindow( w );

  QVariantMap cfg;
  cfg.insert( QStringLiteral( "AllowNull" ), true );

  widget1->setConfig( cfg );
  QgsDateTimeEdit *dateedit1 = qobject_cast<QgsDateTimeEdit *>( widget1->createWidget( w ) );
  QVERIFY( dateedit1 );
  widget1->initWidget( dateedit1 );
  widget1->setValue( QVariant::Date );

  widget2->setConfig( cfg );
  QgsDateTimeEdit *dateedit2 = qobject_cast<QgsDateTimeEdit *>( widget2->createWidget( w ) );
  QVERIFY( dateedit2 );
  widget2->initWidget( dateedit2 );
  widget2->setValue( QVariant::Date );

  widget3->setConfig( cfg );
  QgsDateTimeEdit *dateedit3 = qobject_cast<QgsDateTimeEdit *>( widget3->createWidget( w ) );
  QVERIFY( dateedit3 );
  widget3->initWidget( dateedit3 );
  widget3->setValue( QVariant::Date );

  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( !dateedit1->hasFocus() );
  QVERIFY( !dateedit2->hasFocus() );
  QVERIFY( !dateedit3->hasFocus() );
  QCOMPARE( dateedit1->text(), QStringLiteral( "nope" ) );
  QCOMPARE( dateedit2->text(), QStringLiteral( "nope" ) );
  QCOMPARE( dateedit3->text(), QStringLiteral( "nope" ) );

  dateedit1->setFocus();
  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( dateedit1->hasFocus() );
  QVERIFY( !dateedit2->hasFocus() );
  QVERIFY( !dateedit3->hasFocus() );
  QCOMPARE( dateedit1->text(), QDateTime::currentDateTime().toString( QgsDateTimeFieldFormatter::DATE_FORMAT ) );
  QCOMPARE( dateedit2->text(), QStringLiteral( "nope" ) );
  QCOMPARE( dateedit3->text(), QStringLiteral( "nope" ) );

  dateedit2->setFocus();
  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( !dateedit1->hasFocus() );
  QVERIFY( dateedit2->hasFocus() );
  QVERIFY( !dateedit3->hasFocus() );
  QCOMPARE( dateedit1->text(), QStringLiteral( "nope" ) );
  QCOMPARE( dateedit2->text(), QDateTime::currentDateTime().toString( QgsDateTimeFieldFormatter::DATE_FORMAT ) );
  QCOMPARE( dateedit3->text(), QStringLiteral( "nope" ) );

  dateedit3->setFocus();
  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( !dateedit1->hasFocus() );
  QVERIFY( !dateedit2->hasFocus() );
  QVERIFY( dateedit3->hasFocus() );
  QCOMPARE( dateedit1->text(), QStringLiteral( "nope" ) );
  QCOMPARE( dateedit2->text(), QStringLiteral( "nope" ) );
  QCOMPARE( dateedit3->text(), QDateTime::currentDateTime().toString( QgsDateTimeFieldFormatter::DATE_FORMAT ) );

  dateedit1->setFocus();
  dateedit1->setDateTime( QDateTime::fromString( QStringLiteral( "1955-11-12" ), QgsDateTimeFieldFormatter::DATE_FORMAT ) );
  QVERIFY( !widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( dateedit1->hasFocus() );
  QVERIFY( !dateedit2->hasFocus() );
  QVERIFY( !dateedit3->hasFocus() );
  QCOMPARE( dateedit1->text(), QStringLiteral( "1955-11-12" ) );
  QCOMPARE( dateedit2->text(), QStringLiteral( "nope" ) );
  QCOMPARE( dateedit3->text(), QStringLiteral( "nope" ) );

  dateedit2->setFocus();
  QVERIFY( !widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( !dateedit1->hasFocus() );
  QVERIFY( dateedit2->hasFocus() );
  QVERIFY( !dateedit3->hasFocus() );
  QCOMPARE( dateedit1->text(), QStringLiteral( "1955-11-12" ) );
  QCOMPARE( dateedit2->text(), QDateTime::currentDateTime().toString( QgsDateTimeFieldFormatter::DATE_FORMAT ) );
  QCOMPARE( dateedit3->text(), QStringLiteral( "nope" ) );

  dateedit1->setFocus();
  dateedit1->clear();
  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( dateedit1->hasFocus() );
  QVERIFY( !dateedit2->hasFocus() );
  QVERIFY( !dateedit3->hasFocus() );
  QCOMPARE( dateedit1->text(), QDateTime::currentDateTime().toString( QgsDateTimeFieldFormatter::DATE_FORMAT ) );
  QCOMPARE( dateedit2->text(), QStringLiteral( "nope" ) );
  QCOMPARE( dateedit3->text(), QStringLiteral( "nope" ) );

  dateedit2->setFocus();
  QVERIFY( widget1->value().isNull() );
  QVERIFY( widget2->value().isNull() );
  QVERIFY( widget3->value().isNull() );
  QVERIFY( !dateedit1->hasFocus() );
  QVERIFY( dateedit2->hasFocus() );
  QVERIFY( !dateedit3->hasFocus() );
  QCOMPARE( dateedit1->text(), QStringLiteral( "nope" ) );
  QCOMPARE( dateedit2->text(), QDateTime::currentDateTime().toString( QgsDateTimeFieldFormatter::DATE_FORMAT ) );
  QCOMPARE( dateedit3->text(), QStringLiteral( "nope" ) );
}

QGSTEST_MAIN( TestQgsDateTimeEdit )
#include "testqgsdatetimeedit.moc"
