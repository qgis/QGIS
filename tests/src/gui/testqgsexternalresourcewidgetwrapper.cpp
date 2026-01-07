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

#include "qgsexternalresourcewidget.h"
#include "qgsexternalresourcewidgetwrapper.h"
#include "qgsexternalstorage.h"
#include "qgsexternalstoragefilewidget.h"
#include "qgsexternalstorageregistry.h"
#include "qgsmediawidget.h"
#include "qgsmessagebar.h"
#include "qgspixmaplabel.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QLineEdit>
#include <QMovie>
#include <QSignalSpy>

#define SAMPLE_IMAGE u"%1/sample_image.png"_s.arg( TEST_DATA_DIR )
#define OTHER_SAMPLE_IMAGE u"%1/sample_image2.png"_s.arg( TEST_DATA_DIR )

/**
 * This is a unit test for the external resource widget wrapper
 *
 * \see QgsExternalResourceWidgetWrapper
 */
class TestQgsExternalResourceWidgetWrapper : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
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
      , mCached( url.endsWith( "cached.txt"_L1 ) )
      , mUrl( mCached ? SAMPLE_IMAGE : url )
    {
    }

    void fetch() override
    {
      if ( mCached )
      {
        setStatus( Qgis::ContentStatus::Finished );
        emit fetched();
      }
      else
        setStatus( Qgis::ContentStatus::Running );
    }


    QString filePath() const override
    {
      return mUrl;
    }

    void emitFetched()
    {
      setStatus( Qgis::ContentStatus::Finished );
      emit fetched();
    }

    void emitErrorOccurred()
    {
      setStatus( Qgis::ContentStatus::Failed );
      mErrorString = u"an error"_s;
      emit errorOccurred( mErrorString );
    }

    void cancel() override
    {
      setStatus( Qgis::ContentStatus::Canceled );
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
      : QgsExternalStorageStoredContent(), mUrl( url )
    {
    }

    void store() override
    {
      setStatus( Qgis::ContentStatus::Running );
    }

    void emitStored()
    {
      setStatus( Qgis::ContentStatus::Finished );
      emit stored();
    }

    void emitErrorOccurred()
    {
      setStatus( Qgis::ContentStatus::Failed );
      mErrorString = "an error";
      emit errorOccurred( mErrorString );
    }

    void cancel() override
    {
      setStatus( Qgis::ContentStatus::Canceled );
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
    QString type() const override { return u"test"_s; }

    QString displayName() const override { return u"Test"_s; }

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
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST-EXTERNAL-RESOURCE-WIDGET-WRAPPER"_s );

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
  vl = std::make_unique<QgsVectorLayer>( u"NoGeometry?field=type:string&field=url:string"_s, u"myvl"_s, "memory"_L1 );

  QgsFeature feat1( vl->fields(), 1 );
  feat1.setAttribute( u"type"_s, u"type1"_s );
  vl->dataProvider()->addFeature( feat1 );

  QgsFeature feat2( vl->fields(), 2 );
  feat2.setAttribute( u"type"_s, u"type2"_s );
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

  ww.updateValues( u"test"_s );
  QCOMPARE( ww.mLineEdit->text(), u"test"_s );
  QCOMPARE( ww.mQgsWidget->documentPath(), QVariant( "test" ) );
  QCOMPARE( spy.count(), 1 );

  ww.updateValues( QVariant() );
  QCOMPARE( ww.mLineEdit->text(), QgsApplication::nullRepresentation() );
  QCOMPARE( ww.mQgsWidget->documentPath(), QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
  QCOMPARE( spy.count(), 2 );

  ww.updateValues( QgsApplication::nullRepresentation() );
  QCOMPARE( ww.mLineEdit->text(), QgsApplication::nullRepresentation() );
  QCOMPARE( ww.mQgsWidget->documentPath(), QgsVariantUtils::createNullVariant( QMetaType::Type::QString ) );
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
  config.insert( u"StorageType"_s, u"test"_s );
  QgsPropertyCollection propertyCollection;
  propertyCollection.setProperty( QgsWidgetWrapper::Property::StorageUrl, QgsProperty::fromExpression( "@myurl || @layer_name || '/' || \"type\" || '/' "
                                                                                                       "|| attribute( @current_feature, 'type' ) "
                                                                                                       "|| '/' || $id || '/test'",
                                                                                                       true ) );
  config.insert( u"PropertyCollection"_s, propertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QVERIFY( ww.mQgsWidget->fileWidget() );

  QCOMPARE( ww.mQgsWidget->fileWidget()->storageType(), u"test"_s );
  QgsExpression *expression = ww.mQgsWidget->fileWidget()->storageUrlExpression();
  QVERIFY( expression );

  QgsExpressionContext expressionContext = ww.mQgsWidget->fileWidget()->expressionContext();
  QVERIFY( expression->prepare( &expressionContext ) );
  QCOMPARE( expression->evaluate( &expressionContext ).toString(), u"http://url.test.com/myvl/type1/type1/1/test"_s );

  feat = vl->getFeature( 2 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  expressionContext = ww.mQgsWidget->fileWidget()->expressionContext();
  QVERIFY( expression->prepare( &expressionContext ) );
  QCOMPARE( expression->evaluate( &expressionContext ).toString(), u"http://url.test.com/myvl/type2/type2/2/test"_s );
}

void TestQgsExternalResourceWidgetWrapper::testLoadExternalDocument_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
  QTest::newRow( "audio" ) << static_cast<int>( QgsExternalResourceWidget::Audio );
  QTest::newRow( "video" ) << static_cast<int>( QgsExternalResourceWidget::Video );
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
  config.insert( u"StorageType"_s, u"test"_s );
  config.insert( u"DocumentViewer"_s, documentType );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QVERIFY( ww.mQgsWidget->fileWidget() );
  QCOMPARE( ww.mQgsWidget->fileWidget()->storageType(), u"test"_s );

  widget->show();
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }
  else if ( documentType == QgsExternalResourceWidget::Audio )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }
  else if ( documentType == QgsExternalResourceWidget::Video )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // ----------------------------------------------------
  // load url
  // ----------------------------------------------------
  ww.setValues( SAMPLE_IMAGE, QVariantList() );

  // content still null, fetching in progress...
  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
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
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }

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
  ww.setValues( u"/home/test/cached.txt"_s, QVariantList() );

  // cached, no fetching, content is OK
  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  QVERIFY( QgsTestExternalStorage::sFetchContent );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }

  // wait for the fetch content object to be destroyed
  connect( QgsTestExternalStorage::sFetchContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sFetchContent );

  // ----------------------------------------------------
  // load an error url
  // ----------------------------------------------------
  ww.setValues( u"/home/test/error.txt"_s, QVariantList() );

  // still no error, fetching in progress...
  if ( documentType == QgsExternalResourceWidget::Image )
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );

  QVERIFY( ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  QVERIFY( QgsTestExternalStorage::sFetchContent );

  QgsTestExternalStorage::sFetchContent->emitErrorOccurred();
  QCoreApplication::processEvents();

  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );

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
  QTest::newRow( "audio" ) << static_cast<int>( QgsExternalResourceWidget::Audio );
  QTest::newRow( "video" ) << static_cast<int>( QgsExternalResourceWidget::Video );
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
  config.insert( u"StorageType"_s, u"test"_s );
  config.insert( u"DocumentViewer"_s, documentType );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QVERIFY( ww.mQgsWidget->fileWidget() );
  QCOMPARE( ww.mQgsWidget->fileWidget()->storageType(), u"test"_s );

  widget->show();
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }
  else if ( documentType == QgsExternalResourceWidget::Audio )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }
  else if ( documentType == QgsExternalResourceWidget::Video )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }

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
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() != QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );
}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocument_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
  QTest::newRow( "audio" ) << static_cast<int>( QgsExternalResourceWidget::Audio );
  QTest::newRow( "video" ) << static_cast<int>( QgsExternalResourceWidget::Video );
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
  config.insert( u"StorageType"_s, u"test"_s );
  config.insert( u"DocumentViewer"_s, documentType );

  QgsPropertyCollection propertyCollection;
  propertyCollection.setProperty( QgsWidgetWrapper::Property::StorageUrl, QgsProperty::fromExpression( "'http://mytest.com/' || $id || '/' "
                                                                                                       " || file_name(@selected_file_path)",
                                                                                                       true ) );
  config.insert( u"PropertyCollection"_s, propertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), u"test"_s );

  widget->show();
  ww.mQgsWidget->setReadOnly( false );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }
  else if ( documentType == QgsExternalResourceWidget::Audio )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }
  else if ( documentType == QgsExternalResourceWidget::Video )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // ----------------------------------------------------
  // store one document
  // ----------------------------------------------------
  fileWidget->setSelectedFileNames( QStringList() << u"/home/test/myfile.txt"_s );

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

  QCOMPARE( ww.value().toString(), u"http://mytest.com/1/myfile.txt"_s );

  delete widget;
  delete messageBar;
}

void TestQgsExternalResourceWidgetWrapper::testStoreExternalDocumentError_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
  QTest::newRow( "audio" ) << static_cast<int>( QgsExternalResourceWidget::Audio );
  QTest::newRow( "video" ) << static_cast<int>( QgsExternalResourceWidget::Video );
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
  config.insert( u"StorageType"_s, u"test"_s );
  config.insert( u"DocumentViewer"_s, documentType );
  QgsPropertyCollection propertyCollection;
  propertyCollection.setProperty( QgsWidgetWrapper::Property::StorageUrl, QgsProperty::fromExpression( "'http://mytest.com/' || $id || '/' "
                                                                                                       " || file_name(@selected_file_path)",
                                                                                                       true ) );
  config.insert( u"PropertyCollection"_s, propertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), u"test"_s );

  widget->show();
  ww.mQgsWidget->setReadOnly( false );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }
  else if ( documentType == QgsExternalResourceWidget::Audio )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }
  else if ( documentType == QgsExternalResourceWidget::Video )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  fileWidget->setSelectedFileNames( QStringList() << u"/home/test/error.txt"_s );

  QVERIFY( QgsTestExternalStorage::sStoreContent );

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  QgsTestExternalStorage::sStoreContent->emitErrorOccurred();
  QCoreApplication::processEvents();

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }

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
  QTest::newRow( "audio" ) << static_cast<int>( QgsExternalResourceWidget::Audio );
  QTest::newRow( "video" ) << static_cast<int>( QgsExternalResourceWidget::Video );
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
  config.insert( u"StorageType"_s, u"test"_s );
  config.insert( u"DocumentViewer"_s, documentType );
  QgsPropertyCollection propertyCollection;
  propertyCollection.setProperty( QgsWidgetWrapper::Property::StorageUrl, QgsProperty::fromExpression( "'http://mytest.com/' || $id || '/' "
                                                                                                       " || file_name(@selected_file_path)",
                                                                                                       true ) );
  config.insert( u"PropertyCollection"_s, propertyCollection.toVariant( QgsWidgetWrapper::propertyDefinitions() ) );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), u"test"_s );

  widget->show();
  ww.mQgsWidget->setReadOnly( false );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }
  else if ( documentType == QgsExternalResourceWidget::Audio )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }
  else if ( documentType == QgsExternalResourceWidget::Video )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // ----------------------------------------------------
  // store one document and cancel it
  // ----------------------------------------------------
  fileWidget->setSelectedFileNames( QStringList() << u"/home/test/myfile.txt"_s );

  QVERIFY( QgsTestExternalStorage::sStoreContent );

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  QgsTestExternalStorage::sStoreContent->cancel();
  QCoreApplication::processEvents();

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }

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
  QTest::newRow( "audio" ) << static_cast<int>( QgsExternalResourceWidget::Audio );
  QTest::newRow( "video" ) << static_cast<int>( QgsExternalResourceWidget::Video );
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
  config.insert( u"StorageType"_s, u"test"_s );
  config.insert( u"DocumentViewer"_s, documentType );
  config.insert( u"StorageUrl"_s, u"http://www.test.com/here/"_s );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), u"test"_s );

  widget->show();
  ww.mQgsWidget->setReadOnly( false );

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }
  else if ( documentType == QgsExternalResourceWidget::Audio )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }
  else if ( documentType == QgsExternalResourceWidget::Video )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // store one document
  fileWidget->setSelectedFileNames( QStringList() << u"/home/test/myfile.txt"_s );

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

  QCOMPARE( ww.value().toString(), u"http://www.test.com/here/myfile.txt"_s );

  delete widget;
  delete messageBar;
}

void TestQgsExternalResourceWidgetWrapper::testChangeValueBeforeLoaded_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
  QTest::newRow( "audio" ) << static_cast<int>( QgsExternalResourceWidget::Audio );
  QTest::newRow( "video" ) << static_cast<int>( QgsExternalResourceWidget::Video );
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
  config.insert( u"StorageType"_s, u"test"_s );
  config.insert( u"DocumentViewer"_s, documentType );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), u"test"_s );

  widget->show();
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }
  else if ( documentType == QgsExternalResourceWidget::Audio )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }
  else if ( documentType == QgsExternalResourceWidget::Video )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // load url
  ww.setValues( SAMPLE_IMAGE, QVariantList() );

  // content still null, fetching in progress...
  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
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
  QVERIFY( ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  // first fetch has been canceled and should be destroyed
  connect( firstFetchContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();

  // content still null, fetching in progress...
  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::Running );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );
  QVERIFY( !messageBar->currentItem() );

  QgsTestExternalStorage::sFetchContent->emitFetched();
  QCoreApplication::processEvents();

  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( !ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }

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
  config.insert( u"DocumentViewer"_s, QgsExternalResourceWidget::Web );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );

  widget->show();
}

void TestQgsExternalResourceWidgetWrapper::testChangeValueToNullBeforeLoaded_data()
{
  QTest::addColumn<int>( "documentType" );

  QTest::newRow( "image" ) << static_cast<int>( QgsExternalResourceWidget::Image );
  QTest::newRow( "audio" ) << static_cast<int>( QgsExternalResourceWidget::Audio );
  QTest::newRow( "video" ) << static_cast<int>( QgsExternalResourceWidget::Video );
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
  config.insert( u"StorageType"_s, u"test"_s );
  config.insert( u"DocumentViewer"_s, documentType );
  ww.setConfig( config );

  QgsFeature feat = vl->getFeature( 1 );
  QVERIFY( feat.isValid() );
  ww.setFeature( feat );

  ww.initWidget( widget );
  QVERIFY( ww.mQgsWidget );
  QgsExternalStorageFileWidget *fileWidget = ww.mQgsWidget->fileWidget();
  QVERIFY( fileWidget );
  QCOMPARE( fileWidget->storageType(), u"test"_s );

  widget->show();
  if ( documentType == QgsExternalResourceWidget::Image )
  {
    QVERIFY( ww.mQgsWidget->mPixmapLabel->isVisible() );
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }
  else if ( documentType == QgsExternalResourceWidget::Audio )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }
  else if ( documentType == QgsExternalResourceWidget::Video )
  {
    QVERIFY( ww.mQgsWidget->mMediaWidget->isVisible() );
  }

  QVERIFY( !ww.mQgsWidget->mLoadingLabel->isVisible() );
  QVERIFY( ww.mQgsWidget->mLoadingMovie->state() == QMovie::NotRunning );
  QVERIFY( !ww.mQgsWidget->mErrorLabel->isVisible() );

  // load url
  ww.setValues( SAMPLE_IMAGE, QVariantList() );

  // content still null, fetching in progress...
  QVERIFY( !ww.mQgsWidget->mPixmapLabel->isVisible() );
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
    QVERIFY( ww.mQgsWidget->mPixmapLabel->pixmap( Qt::ReturnByValue ).isNull() );
  }

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
