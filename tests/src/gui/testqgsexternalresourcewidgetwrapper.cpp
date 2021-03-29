/***************************************************************************
                         testqgsexternalresourcewidgetwrapper.cpp
                         ---------------------------
    begin                : Oct 2020
    copyright            : (C) 2020 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
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
#include "qgsvectorlayer.h"
#include "qgsexternalresourcewidgetwrapper.h"
#include "qgsexternalresourcewidget.h"

#include <QLineEdit>
#include <QSignalSpy>

/**
 * @ingroup UnitTests
 * This is a unit test for the external resource widget wrapper
 *
 * \see QgsExternalResourceWidgetWrapper
 */
class TestQgsExternalResourceWidgetWrapper : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void test_setNullValues();

  private:
    std::unique_ptr<QgsVectorLayer> vl;
};

void TestQgsExternalResourceWidgetWrapper::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST-EXTERNAL-RESOURCE-WIDGET-WRAPPER" ) );

  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsExternalResourceWidgetWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsExternalResourceWidgetWrapper::init()
{
  vl = std::make_unique<QgsVectorLayer>( QStringLiteral( "NoGeometry?field=link:string" ),
                                         QStringLiteral( "myvl" ),
                                         QLatin1String( "memory" ) );
}

void TestQgsExternalResourceWidgetWrapper::cleanup()
{
}

void TestQgsExternalResourceWidgetWrapper::test_setNullValues()
{
  QgsExternalResourceWidgetWrapper ww( vl.get(), 0, nullptr, nullptr );
  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QVERIFY( ww.mLineEdit );

  QSignalSpy spy( &ww, &QgsExternalResourceWidgetWrapper::valuesChanged );

  ww.updateValues( QStringLiteral( "test" ) );
  QCOMPARE( ww.mLineEdit->text(), QStringLiteral( "test" ) );
  QCOMPARE( ww.mQgsWidget->documentPath(), QVariant( "test" ) );
  QCOMPARE( spy.count(), 1 );

  ww.updateValues( QVariant() );
  QCOMPARE( ww.mLineEdit->text(), QgsApplication::nullRepresentation() );
  QCOMPARE( ww.mQgsWidget->documentPath(), QVariant( QVariant::String ) );
  QCOMPARE( spy.count(), 2 );

  ww.updateValues( QgsApplication::nullRepresentation() );
  QCOMPARE( ww.mLineEdit->text(), QgsApplication::nullRepresentation() );
  QCOMPARE( ww.mQgsWidget->documentPath(), QVariant( QVariant::String ) );
  QCOMPARE( spy.count(), 2 );

  delete widget;
}

QGSTEST_MAIN( TestQgsExternalResourceWidgetWrapper )
#include "testqgsexternalresourcewidgetwrapper.moc"
