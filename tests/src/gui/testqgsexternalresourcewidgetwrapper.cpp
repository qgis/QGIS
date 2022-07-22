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
#include "qgsexternalstorage.h"
#include "qgsexternalstorageregistry.h"
#include "qgspixmaplabel.h"
#include "qgsmessagebar.h"
#include "qgsexternalstoragefilewidget.h"

#include <QLineEdit>
#include <QSignalSpy>
#include <QMovie>

#ifdef WITH_QTWEBKIT
#include <QWebFrame>
#include <QWebView>
#endif

#define SAMPLE_IMAGE QStringLiteral( "%1/sample_image.png" ).arg( TEST_DATA_DIR )
#define OTHER_SAMPLE_IMAGE QStringLiteral( "%1/sample_image2.png" ).arg( TEST_DATA_DIR )

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
    void testSetNullValues();

    void testUrlStorageExpression();
    void testLoadExternalDocument_data();
    void testLoadExternalDocument();
    void testLoadNullExternalDocument_data();
    void testLoadNullExternalDocument();
    void testStoreExternalDocument_data();
    void testStoreExternalDocument();
    void testStoreExternalDocumentError_data();
    void testStoreExternalDocumentError();
    void testStoreExternalDocumentCancel_data();
    void testStoreExternalDocumentCancel();
    void testStoreExternalDocumentNoExpression_data();
    void testStoreExternalDocumentNoExpression();
    void testChangeValueBeforeLoaded();
    void testChangeValueBeforeLoaded_data();
    void testBlankAfterValue();
    void testChangeValueToNullBeforeLoaded();
    void testChangeValueToNullBeforeLoaded_data();

  private:
    std::unique_ptr<QgsVectorLayer> vl;
};

class QgsTestExternalStorageFetchedContent
  : public QgsExternalStorageFetchedContent
{
    Q_OBJECT

  public:

    QgsTestExternalStorageFetchedContent( QString url )
      : QgsExternalStorageFetchedContent()
      , mCached( url.endsWith( QLatin1String( "cached.txt" ) ) )
      , mUrl( mCached ? SAMPLE_IMAGE : url )
    {
    }

    void fetch() override
    {
      if ( mCached )
      {
        mStatus = Qgis::ContentStatus::Finished;
        emit fetched();
      }
      else
        mStatus = Qgis::ContentStatus::Running;
    }


    QString filePath() const override
    {
      return mUrl;
    }

    void emitFetched()
    {
      mStatus = Qgis::ContentStatus::Finished;
      emit fetched();
    }

    void emitErrorOccurred()
    {
      mStatus = Qgis::ContentStatus::Failed;
      mErrorString = QStringLiteral( "an error" );
      emit errorOccurred( mErrorString );
    }

    void cancel() override
    {
      mStatus = Qgis::ContentStatus::Canceled;
      emit canceled();
    }

  private:

    bool mCached = false;
    QString mUrl;
};

class QgsTestExternalStorageStoredContent
  : public QgsExternalStorageStoredContent
{
    Q_OBJECT

  public:

    QgsTestExternalStorageStoredContent( const QString &url )
      : QgsExternalStorageStoredContent(),
        mUrl( url )
    {
    }

    void store() override
    {
      mStatus = Qgis::ContentStatus::Running;
    }

    void emitStored()
    {
      mStatus = Qgis::ContentStatus::Finished;
      emit stored();
    }

    void emitErrorOccurred()
    {
      mStatus = Qgis::ContentStatus::Failed;
      mErrorString = "an error";
      emit errorOccurred( mErrorString );
    }

    void cancel() override
    {
      mStatus = Qgis::ContentStatus::Canceled;
      emit canceled();
    };

    QString url() const override
    {
      return mUrl.endsWith( "/" ) ? QString( "http://www.test.com/here/myfile.txt" )
             : mUrl;
    }

  private:

    QString mUrl;
};


class QgsTestExternalStorage : public QgsExternalStorage
{
  public:

    QString type() const override { return QStringLiteral( "test" ); }

    QString displayName() const override { return QStringLiteral( "Test" ); }

    QgsExternalStorageStoredContent *doStore( const QString &filePath, const QString &url, const QString &authcfg = QString() ) const override
    {
      Q_UNUSED( authcfg );
      Q_UNUSED( filePath );

      sStoreContent = new QgsTestExternalStorageStoredContent( url );
      return sStoreContent;
    }

    QgsExternalStorageFetchedContent *doFetch( const QString &url, const QString &authcfg = QString() ) const override
    {
      Q_UNUSED( authcfg );

      sFetchContent = new QgsTestExternalStorageFetchedContent( url );

      return sFetchContent;
    }

    static QPointer<QgsTestExternalStorageFetchedContent> sFetchContent;
    static QPointer<QgsTestExternalStorageStoredContent> sStoreContent;
};

QPointer<QgsTestExternalStorageFetchedContent> QgsTestExternalStorage::sFetchContent;
QPointer<QgsTestExternalStorageStoredContent> QgsTestExternalStorage::sStoreContent;

void TestQgsExternalResourceWidgetWrapper::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST-EXTERNAL-RESOURCE-WIDGET-WRAPPER" ) );

  QgsApplication::init();
  QgsApplication::initQgis();

  QgsApplication::externalStorageRegistry()->registerExternalStorage( new QgsTestExternalStorage() );
}

void TestQgsExternalResourceWidgetWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsExternalResourceWidgetWrapper::init()
{
  vl = std::make_unique<QgsVectorLayer>( QStringLiteral( "NoGeometry?field=type:string&field=url:string" ),
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

void TestQgsExternalResourceWidgetWrapper::testSetNullValues()
{
  QgsExternalResourceWidgetWrapper ww( vl.get(), 0, nullptr, nullptr );
  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QVERIFY( ww.mLineEdit );

  const QSignalSpy spy( &ww, &QgsExternalResourceWidgetWrapper::valuesChanged );

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

void TestQgsExternalResourceWidgetWrapper::testUrlStorageExpression()
{
  // test that everything related to Url storage expression is correctly set
  // according to configuration

  QVariantMap globalVariables;
  globalVariables.insert( "myurl", "http://url.test.com/" );
  QgsApplication::setCustomVariables( globalVariables );

  QgsExternalResourceWidgetWrapper ww( vl.get(), 0, nullptr, nullptr );
  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "StorageType" ), QStringLiteral( "test" ) );
  QgsPropertyCollection propertyCollection;
  propertyCollection.setProperty( QgsWidgetWrapper::StorageUrl, QgsProperty::fromExpression(
                                    "@myurl || @layer_name || '/' || \"type\" || '/' "
                                    "|| attribute( @current_feature, 'type' ) "
                                    "|| '/' || $id || '/test'", true ) );
  config.insert( QStringLiteral( "PropertyCollection" ), propertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QVERIFY( ww.mQgsWidget->fileWidget() );

  QCOMPARE( ww.mQgsWidget->fileWidget()->storageType(), QStringLiteral( "test" ) );
  QgsExpression *expression = ww.mQgsWidget->fileWidget()->storageUrlExpression();
  QVERIFY( expression );

  QgsExpressionContext expressionContext = ww.mQgsWidget->fileWidget()->expressionContext();
  QVERIFY( expression->prepare( &expressionContext ) );
  QCOMPARE( expression->evaluate( &expressionContext ).toString(),
            QStringLiteral( "http://url.test.com/myvl/type1/type1/1/test" ) );

  feat = vl->getFeature( 2 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  expressionContext = ww.mQgsWidget->fileWidget()->expressionContext();
  QVERIFY( expression->prepare( &expressionContext ) );
  QCOMPARE( expression->evaluate( &expressionContext ).toString(),
            QStringLiteral( "http://url.test.com/myvl/type2/type2/2/test" ) );
}

void TestQgsExternalResourceWidgetWrapper::testLoadExternalDocument_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
#ifdef WITH_QTWEBKIT
  QTest::newRow( "webview" ) << static_cast<int>( QgsExternalResourceWidget::Web );
#endif
}

void TestQgsExternalResourceWidgetWrapper::testLoadExternalDocument()
{
  // test to load a document to be accessed with an external storage
  QEventLoop loop;

  QFETCH( int, documentType );

  QgsMessageBar *messageBar = new QgsMessageBar;
  QgsExternalResourceWidgetWrapper ww( vl.get(), 0, nullptr, messageBar, nullptr );

  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "StorageType" ), QStringLiteral( "test" ) );
  config.insert( QStringLiteral( "DocumentViewer" ), documentType );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QVERIFY( ww.mQgsWidget->fileWidget() );
  QCOMPARE( ww.mQgsWidget->fileWidget()->storageType(), QStringLiteral( "test" ) );

  widget->show();
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )

  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // ----------------------------------------------------
  // load url
  // ----------------------------------------------------
  ww.setValues( SAMPLE_IMAGE, QVariantList() );

  // content still null, fetching in progress...
  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
#ifdef WITH_QTWEBKIT
  QVERIFY( !ww.mQgsWidget->mWebView->isVisible() );
#endif
  QVERIFY( ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  QVERIFY( QgsTestExternalStorage::sFetchContent );

  QgsTestExternalStorage::sFetchContent->emitFetched();
  QCoreApplication::processEvents();

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QVERIFY( ww.mQgsWidget->mWebView->url().isValid() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "file://%1/sample_image.png" ).arg( TEST_DATA_DIR ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  // wait for the fetch content object to be destroyed
  connect( QgsTestExternalStorage::sFetchContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sFetchContent );

  // ----------------------------------------------------
  // load a cached url
  // ----------------------------------------------------
  ww.setValues( QStringLiteral( "/home/test/cached.txt" ), QVariantList() );

  // cached, no fetching, content is OK
  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  QVERIFY( QgsTestExternalStorage::sFetchContent );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QVERIFY( ww.mQgsWidget->mWebView->url().isValid() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "file://%1/sample_image.png" ).arg( TEST_DATA_DIR ) );
  }
#endif

  // wait for the fetch content object to be destroyed
  connect( QgsTestExternalStorage::sFetchContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sFetchContent );

  // ----------------------------------------------------
  // load an error url
  // ----------------------------------------------------
  ww.setValues( QStringLiteral( "/home/test/error.txt" ), QVariantList() );

  // still no error, fetching in progress...
  if ( documentType == QgsExternalResourceWidget::Image )
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );

#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
    QVERIFY( !ww.mQgsWidget->mWebView->isVisible() );

#endif

  QVERIFY( ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  QVERIFY( QgsTestExternalStorage::sFetchContent );

  QgsTestExternalStorage::sFetchContent->emitErrorOccurred();
  QCoreApplication::processEvents();

  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
#ifdef WITH_QTWEBKIT
  QVERIFY( !ww.mQgsWidget->mWebView->isVisible() );
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( messageBar->currentItem() );

  // wait for the fetch content object to be destroyed
  connect( QgsTestExternalStorage::sFetchContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sFetchContent );

  delete widget;
  delete messageBar;
}

void TestQgsExternalResourceWidgetWrapper::testLoadNullExternalDocument_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
#ifdef WITH_QTWEBKIT
  QTest::newRow( "webview" ) << static_cast<int>( QgsExternalResourceWidget::Web );
#endif
}

void TestQgsExternalResourceWidgetWrapper::testLoadNullExternalDocument()
{
  // Check that widget doesn't try to load a document which value is NULL
  QFETCH( int, documentType );

  QgsMessageBar *messageBar = new QgsMessageBar;
  QgsExternalResourceWidgetWrapper ww( vl.get(), 0, nullptr, messageBar, nullptr );

  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "StorageType" ), QStringLiteral( "test" ) );
  config.insert( QStringLiteral( "DocumentViewer" ), documentType );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QVERIFY( ww.mQgsWidget->fileWidget() );
  QCOMPARE( ww.mQgsWidget->fileWidget()->storageType(), QStringLiteral( "test" ) );

  widget->show();
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )

  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // load null url
  ww.setValues( QString(), QVariantList() );

  QVERIFY( !QgsTestExternalStorage::sFetchContent );

  // content is null
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )

  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() != QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocument_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
#ifdef WITH_QTWEBKIT
  QTest::newRow( "webview" ) << static_cast<int>( QgsExternalResourceWidget::Web );
#endif
}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocument()
{
  QFETCH( int, documentType );

  QEventLoop loop;
  QgsMessageBar *messageBar = new QgsMessageBar;
  QgsExternalResourceWidgetWrapper ww( vl.get(), 1, nullptr, messageBar, nullptr );

  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "StorageType" ), QStringLiteral( "test" ) );
  config.insert( QStringLiteral( "DocumentViewer" ), documentType );

  QgsPropertyCollection propertyCollection;
  propertyCollection.setProperty( QgsWidgetWrapper::StorageUrl, QgsProperty::fromExpression(
                                    "'http://mytest.com/' || $id || '/' "
                                    " || file_name(@selected_file_path)", true ) );
  config.insert( QStringLiteral( "PropertyCollection" ), propertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), QStringLiteral( "test" ) );

  widget->show();
  ww.mQgsWidget->setReadOnly( false );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // ----------------------------------------------------
  // store one document
  // ----------------------------------------------------
  fileWidget->setSelectedFileNames( QStringList() << QStringLiteral( "/home/test/myfile.txt" ) );

  QVERIFY( QgsTestExternalStorage::sStoreContent );

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  QgsTestExternalStorage::sStoreContent->emitStored();
  QCoreApplication::processEvents();

  QVERIFY( !messageBar->currentItem() );

  // wait for the store content object to be destroyed
  connect( QgsTestExternalStorage::sStoreContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sStoreContent );

  QCOMPARE( ww.value().toString(), QStringLiteral( "http://mytest.com/1/myfile.txt" ) );

  delete widget;
  delete messageBar;
}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocumentError_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
#ifdef WITH_QTWEBKIT
  QTest::newRow( "webview" ) << static_cast<int>( QgsExternalResourceWidget::Web );
#endif
}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocumentError()
{
  QFETCH( int, documentType );

  QEventLoop loop;
  QgsMessageBar *messageBar = new QgsMessageBar;
  QgsExternalResourceWidgetWrapper ww( vl.get(), 1, nullptr, messageBar, nullptr );

  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "StorageType" ), QStringLiteral( "test" ) );
  config.insert( QStringLiteral( "DocumentViewer" ), documentType );
  QgsPropertyCollection propertyCollection;
  propertyCollection.setProperty( QgsWidgetWrapper::StorageUrl, QgsProperty::fromExpression(
                                    "'http://mytest.com/' || $id || '/' "
                                    " || file_name(@selected_file_path)", true ) );
  config.insert( QStringLiteral( "PropertyCollection" ), propertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), QStringLiteral( "test" ) );

  widget->show();
  ww.mQgsWidget->setReadOnly( false );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  fileWidget->setSelectedFileNames( QStringList() << QStringLiteral( "/home/test/error.txt" ) );

  QVERIFY( QgsTestExternalStorage::sStoreContent );

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  QgsTestExternalStorage::sStoreContent->emitErrorOccurred();
  QCoreApplication::processEvents();

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( messageBar->currentItem() );

  // wait for the store content object to be destroyed
  connect( QgsTestExternalStorage::sStoreContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sStoreContent );

  // value hasn't changed, same as before we try to store
  QVERIFY( ww.value().toString().isEmpty() );

  delete widget;
  delete messageBar;
}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocumentCancel_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
#ifdef WITH_QTWEBKIT
  QTest::newRow( "webview" ) << static_cast<int>( QgsExternalResourceWidget::Web );
#endif
}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocumentCancel()
{
  QFETCH( int, documentType );

  QEventLoop loop;
  QgsMessageBar *messageBar = new QgsMessageBar;
  QgsExternalResourceWidgetWrapper ww( vl.get(), 1, nullptr, messageBar, nullptr );

  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "StorageType" ), QStringLiteral( "test" ) );
  config.insert( QStringLiteral( "DocumentViewer" ), documentType );
  QgsPropertyCollection propertyCollection;
  propertyCollection.setProperty( QgsWidgetWrapper::StorageUrl, QgsProperty::fromExpression(
                                    "'http://mytest.com/' || $id || '/' "
                                    " || file_name(@selected_file_path)", true ) );
  config.insert( QStringLiteral( "PropertyCollection" ), propertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), QStringLiteral( "test" ) );

  widget->show();
  ww.mQgsWidget->setReadOnly( false );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // ----------------------------------------------------
  // store one document and cancel it
  // ----------------------------------------------------
  fileWidget->setSelectedFileNames( QStringList() << QStringLiteral( "/home/test/myfile.txt" ) );

  QVERIFY( QgsTestExternalStorage::sStoreContent );

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  QgsTestExternalStorage::sStoreContent->cancel();
  QCoreApplication::processEvents();

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  // wait for the store content object to be destroyed
  connect( QgsTestExternalStorage::sStoreContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sStoreContent );

  QVERIFY( ww.value().toString().isEmpty() );

  delete widget;
  delete messageBar;
}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocumentNoExpression_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
#ifdef WITH_QTWEBKIT
  QTest::newRow( "webview" ) << static_cast<int>( QgsExternalResourceWidget::Web );
#endif
}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocumentNoExpression()
{
  // store a document using a raw URL, not an expression

  QFETCH( int, documentType );

  QEventLoop loop;
  QgsMessageBar *messageBar = new QgsMessageBar;
  QgsExternalResourceWidgetWrapper ww( vl.get(), 1, nullptr, messageBar, nullptr );

  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "StorageType" ), QStringLiteral( "test" ) );
  config.insert( QStringLiteral( "DocumentViewer" ), documentType );
  config.insert( QStringLiteral( "StorageUrl" ), QStringLiteral( "http://www.test.com/here/" ) );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), QStringLiteral( "test" ) );

  widget->show();
  ww.mQgsWidget->setReadOnly( false );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // store one document
  fileWidget->setSelectedFileNames( QStringList() << QStringLiteral( "/home/test/myfile.txt" ) );

  QVERIFY( QgsTestExternalStorage::sStoreContent );

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  QgsTestExternalStorage::sStoreContent->emitStored();
  QCoreApplication::processEvents();

  QVERIFY( !messageBar->currentItem() );

  // wait for the store content object to be destroyed
  connect( QgsTestExternalStorage::sStoreContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sStoreContent );

  QCOMPARE( ww.value().toString(), QStringLiteral( "http://www.test.com/here/myfile.txt" ) );

  delete widget;
  delete messageBar;
}

void TestQgsExternalResourceWidgetWrapper::testChangeValueBeforeLoaded_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
#ifdef WITH_QTWEBKIT
  QTest::newRow( "webview" ) << static_cast<int>( QgsExternalResourceWidget::Web );
#endif
}

void TestQgsExternalResourceWidgetWrapper::testChangeValueBeforeLoaded()
{
  // test to change value before loading of a previous value document has finished
  QEventLoop loop;

  QFETCH( int, documentType );

  QgsMessageBar *messageBar = new QgsMessageBar;
  QgsExternalResourceWidgetWrapper ww( vl.get(), 0, nullptr, messageBar, nullptr );

  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "StorageType" ), QStringLiteral( "test" ) );
  config.insert( QStringLiteral( "DocumentViewer" ), documentType );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), QStringLiteral( "test" ) );

  widget->show();
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )

  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // load url
  ww.setValues( SAMPLE_IMAGE, QVariantList() );

  // content still null, fetching in progress...
  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
#ifdef WITH_QTWEBKIT
  QVERIFY( !ww.mQgsWidget->mWebView->isVisible() );
#endif
  QVERIFY( ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  QVERIFY( QgsTestExternalStorage::sFetchContent );

  QPointer firstFetchContent = QgsTestExternalStorage::sFetchContent;

  // first fetch is not over, load another file
  ww.setValues( OTHER_SAMPLE_IMAGE, QVariantList() );
  QVERIFY( firstFetchContent != QgsTestExternalStorage::sFetchContent );

  // content still null, fetching in progress...
  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
#ifdef WITH_QTWEBKIT
  QVERIFY( !ww.mQgsWidget->mWebView->isVisible() );
#endif
  QVERIFY( ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  // first fetch has been canceled and should be destroyed
  connect( firstFetchContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();

  // content still null, fetching in progress...
  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
#ifdef WITH_QTWEBKIT
  QVERIFY( !ww.mQgsWidget->mWebView->isVisible() );
#endif
  QVERIFY( ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  QgsTestExternalStorage::sFetchContent->emitFetched();
  QCoreApplication::processEvents();

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )
  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QVERIFY( ww.mQgsWidget->mWebView->url().isValid() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "file://%1" ).arg( OTHER_SAMPLE_IMAGE ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  // wait for the fetch content object to be destroyed
  connect( QgsTestExternalStorage::sFetchContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
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

#ifdef WITH_QTWEBKIT
  QEventLoop loop;
  connect( ww.mQgsWidget->mWebView, &QWebView::loadFinished, &loop, &QEventLoop::quit );

  ww.setValues( QString( "file://%1" ).arg( SAMPLE_IMAGE ), QVariantList() );

  QVERIFY( ww.mQgsWidget->mWebView->isVisible() );

  loop.exec();

  ww.setValues( QString(), QVariantList() );

  QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
  QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
#endif
}

void TestQgsExternalResourceWidgetWrapper::testChangeValueToNullBeforeLoaded_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
#ifdef WITH_QTWEBKIT
  QTest::newRow( "webview" ) << static_cast<int>( QgsExternalResourceWidget::Web );
#endif
}

void TestQgsExternalResourceWidgetWrapper::testChangeValueToNullBeforeLoaded()
{
  // test to change value to Null before loading of a previous value document has finished
  QEventLoop loop;

  QFETCH( int, documentType );

  QgsMessageBar *messageBar = new QgsMessageBar;
  QgsExternalResourceWidgetWrapper ww( vl.get(), 0, nullptr, messageBar, nullptr );

  QWidget *widget = ww.createWidget( nullptr );
  QVERIFY( widget );

  QVariantMap config;
  config.insert( QStringLiteral( "StorageType" ), QStringLiteral( "test" ) );
  config.insert( QStringLiteral( "DocumentViewer" ), documentType );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), QStringLiteral( "test" ) );

  widget->show();
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )

  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // load url
  ww.setValues( SAMPLE_IMAGE, QVariantList() );

  // content still null, fetching in progress...
  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
#ifdef WITH_QTWEBKIT
  QVERIFY( !ww.mQgsWidget->mWebView->isVisible() );
#endif
  QVERIFY( ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  QVERIFY( QgsTestExternalStorage::sFetchContent );

  QPointer firstFetchContent = QgsTestExternalStorage::sFetchContent;

  // first fetch is not over, load another file
  ww.setValues( QString(), QVariantList() );

  // content null fetching over
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap() );
#else
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
#endif
  }
#ifdef WITH_QTWEBKIT
  else if ( documentType == QgsExternalResourceWidget::Web )

  {
    QVERIFY( ww.mQgsWidget->mWebView->isVisible() );
    QCOMPARE( ww.mQgsWidget->mWebView->url().toString(), QStringLiteral( "about:blank" ) );
  }
#endif

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );
  QVERIFY( !ww.mQgsWidget->mContent );

  connect( QgsTestExternalStorage::sFetchContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();

  QVERIFY( !QgsTestExternalStorage::sFetchContent );

  delete messageBar;
  delete widget;
}



QGSTEST_MAIN( TestQgsExternalResourceWidgetWrapper )
#include "testqgsexternalresourcewidgetwrapper.moc"
