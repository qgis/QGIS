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

#ifdef WITH_QTWEBKIT
#include <QWebFrame>
#include <QWebView>
#endif

#define SAMPLE_IMAGE QStringLiteral( "%1/sample_image.png" ).arg( TEST_DATA_DIR )

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
    void testBlankAfterValue();

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
  vl = qgis::make_unique<QgsVectorLayer>( QStringLiteral( "NoGeometry?field=link:string" ),
                                          QStringLiteral( "myvl" ),
                                          QLatin1String( "memory" ) );

  QgsFeature feat1( vl->fields(),  1 );
  feat1.setAttribute( QStringLiteral( "type" ), QStringLiteral( "type1" ) );
  vl->dataProvider()->addFeature( feat1 );

  QgsFeature feat2( vl->fields(),  2 );
  feat2.setAttribute( QStringLiteral( "type" ), QStringLiteral( "type2" ) );
  vl->dataProvider()->addFeature( feat2 );
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

void TestQgsExternalResourceWidgetWrapper::testBlankAfterValue()
{
  // test that application doesn't crash when we set a blank page in web preview
  // after an item have been set

  QgsExternalResourceWidgetWrapper ww( vl.get(), 0, nullptr, nullptr );
  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "DocumentViewer" ), QgsExternalResourceWidget::Web );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  widget->show();

  QEventLoop loop;
  connect( ww.mQgsWidget->mWebView, &QWebView::loadFinished, &loop, &QEventLoop::quit );

  ww.setValues( QString( "file://%1" ).arg( SAMPLE_IMAGE ), QVariantList() );

  QVERIFY( ww.mQgsWidget->mWebView->isVisible() );

  loop.exec();

  ww.setValues( QString(), QVariantList() );

  QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
  QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
}

QGSTEST_MAIN( TestQgsExternalResourceWidgetWrapper )
#include "testqgsexternalresourcewidgetwrapper.moc"
