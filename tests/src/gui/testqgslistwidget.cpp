/***************************************************************************
    testqgslistwidget.cpp
     --------------------------------------
    Date                 : 08 09 2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick dot valsecchi at camptocamp dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include <editorwidgets/qgslistwidgetfactory.h>
#include <qgslistwidget.h>
#include <editorwidgets/core/qgseditorwidgetwrapper.h>
#include <qgsapplication.h>

class TestQgsListWidget : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase() // will be called before the first testfunction is executed.
    {
      QgsApplication::init();
      QgsApplication::initQgis();
    }

    void cleanupTestCase() // will be called after the last testfunction was executed.
    {
      QgsApplication::exitQgis();
    }

    void testStringUpdate()
    {
      const QgsListWidgetFactory factory( QStringLiteral( "testList" ) );
      QgsVectorLayer vl( QStringLiteral( "Point?field=fld:string[]" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
      QgsEditorWidgetWrapper *wrapper = factory.create( &vl, 0, nullptr, nullptr );
      QVERIFY( wrapper );
      QSignalSpy spy( wrapper, SIGNAL( valueChanged( const QVariant & ) ) );

      QgsListWidget *widget = qobject_cast< QgsListWidget * >( wrapper->widget() );
      QVERIFY( widget );

      QStringList initial;
      initial << QStringLiteral( "one" ) << QStringLiteral( "two" );
      wrapper->setValues( initial, QVariantList() );

      const QVariant value = wrapper->value();
      QCOMPARE( int( value.type() ), int( QVariant::StringList ) );
      QCOMPARE( value.toStringList(), initial );
      QCOMPARE( spy.count(), 0 );

      QAbstractItemModel *model = widget->tableView->model();
      model->setData( model->index( 0, 0 ), "hello" );
      QCOMPARE( spy.count(), 1 );
      QVERIFY( widget->valid() );

      QStringList expected = initial;
      expected[0] = QStringLiteral( "hello" );
      QVariant eventValue = spy.at( 0 ).at( 0 ).value<QVariant>();
      QCOMPARE( int( eventValue.type() ), int( QVariant::StringList ) );
      QCOMPARE( eventValue.toStringList(), expected );
      QCOMPARE( wrapper->value().toStringList(), expected );
      QCOMPARE( spy.count(), 1 );
      QVERIFY( widget->valid() );
    }

    void testIntUpdate()
    {
      const QgsListWidgetFactory factory( QStringLiteral( "testList" ) );
      QgsVectorLayer vl( QStringLiteral( "Point?field=fld:int[]" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
      QgsEditorWidgetWrapper *wrapper = factory.create( &vl, 0, nullptr, nullptr );
      QVERIFY( wrapper );
      QSignalSpy spy( wrapper, SIGNAL( valueChanged( const QVariant & ) ) );

      QgsListWidget *widget = qobject_cast< QgsListWidget * >( wrapper->widget() );
      QVERIFY( widget );

      QVariantList initial;
      initial << 1 << -2;
      wrapper->setValues( initial, QVariantList() );

      const QVariant value = wrapper->value();
      QCOMPARE( int( value.type() ), int( QVariant::List ) );
      QCOMPARE( value.toList(), initial );
      QCOMPARE( spy.count(), 0 );

      QAbstractItemModel *model = widget->tableView->model();
      model->setData( model->index( 0, 0 ), 3 );
      QCOMPARE( spy.count(), 1 );

      QVariantList expected = initial;
      expected[0] = 3;
      QCOMPARE( spy.count(), 1 );
      QVariant eventValue = spy.at( 0 ).at( 0 ).value<QVariant>();
      QCOMPARE( int( eventValue.type() ), int( QVariant::List ) );
      QCOMPARE( eventValue.toList(), expected );
      QCOMPARE( wrapper->value().toList(), expected );
      QVERIFY( widget->valid() );

      model->setData( model->index( 0, 0 ), "a" );
      expected = initial;
      expected.removeAt( 0 );
      QVERIFY( !widget->valid() );
      QCOMPARE( wrapper->value().toList(), expected );

      spy.clear();
      model->setData( model->index( 0, 0 ), 56 );
      expected = initial;
      expected[0] = 56;
      QCOMPARE( spy.count(), 1 );
      eventValue = spy.at( 0 ).at( 0 ).value<QVariant>();
      QCOMPARE( eventValue.toList(), expected );
      QCOMPARE( wrapper->value().toList(), expected );
      QVERIFY( widget->valid() );
    }
};

QGSTEST_MAIN( TestQgsListWidget )
#include "testqgslistwidget.moc"
