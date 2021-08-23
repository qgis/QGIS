/***************************************************************************
    testqgskeyvaluewidget.cpp
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
#include <QSignalSpy>

#include <editorwidgets/qgskeyvaluewidgetfactory.h>
#include <qgskeyvaluewidget.h>
#include <editorwidgets/core/qgseditorwidgetwrapper.h>
#include <qgsapplication.h>

class TestQgsKeyValueWidget : public QObject
{
    Q_OBJECT
  public:

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

    void testUpdate()
    {
      const QgsKeyValueWidgetFactory factory( QStringLiteral( "testKeyValue" ) );
      QgsEditorWidgetWrapper *wrapper = factory.create( nullptr, 0, nullptr, nullptr );
      QVERIFY( wrapper );
      const QSignalSpy spy( wrapper, SIGNAL( valueChanged( const QVariant & ) ) );

      QgsKeyValueWidget *widget = qobject_cast< QgsKeyValueWidget * >( wrapper->widget() );
      QVERIFY( widget );

      QVariantMap initial;
      initial[QStringLiteral( "1" )] = "one";
      initial[QStringLiteral( "2" )] = "two";
      wrapper->setValues( initial, QVariantList() );

      const QVariant value = wrapper->value();
      QCOMPARE( int( value.type() ), int( QVariant::Map ) );
      QCOMPARE( value.toMap(), initial );
      QCOMPARE( spy.count(), 0 );

      QAbstractItemModel *model = widget->tableView->model();
      model->setData( model->index( 0, 1 ), "hello" );
      QCOMPARE( spy.count(), 1 );

      QVariantMap expected = initial;
      expected[QStringLiteral( "1" )] = "hello";
      const QVariant eventValue = spy.at( 0 ).at( 0 ).value<QVariant>();
      QCOMPARE( int( eventValue.type() ), int( QVariant::Map ) );
      QCOMPARE( eventValue.toMap(), expected );
      QCOMPARE( wrapper->value().toMap(), expected );
      QCOMPARE( spy.count(), 1 );
    }
};

QGSTEST_MAIN( TestQgsKeyValueWidget )
#include "testqgskeyvaluewidget.moc"
