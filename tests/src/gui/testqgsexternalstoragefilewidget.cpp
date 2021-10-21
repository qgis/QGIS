/***************************************************************************
    testqgsexternalstoragefilewidget.cpp
     --------------------------------------
    Date                 : August 2021
    Copyright            : (C) 2021 Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include "qgsexternalstoragefilewidget.h"
#include "qgsmimedatautils.h"
#include "qgsdataitem.h"
#include "qgsbrowsermodel.h"
#include "qgslayeritem.h"
#include "qgsdirectoryitem.h"
#include "qgsexternalstorage.h"
#include "qgsexternalstorageregistry.h"
#include "qgsmessagebar.h"
#include "qgsexpressioncontextutils.h"
#include <memory>

#include <QLabel>
#include <QToolButton>
#include <QProgressBar>

class TestQgsExternalStorageFileWidget: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void testLayout_data();
    void testLayout();
    void testStoring();
    void testStoring_data();
    void testStoringSeveralFiles_data();
    void testStoringSeveralFiles();
    void testStoringSeveralFilesError_data();
    void testStoringSeveralFilesError();
    void testStoringSeveralFilesCancel_data();
    void testStoringSeveralFilesCancel();
    void testStoringDirectory_data();
    void testStoringDirectory();
    void testStoringChangeFeature();
    void testStoringBadExpression_data();
    void testStoringBadExpression();
    void testDragAndDrop();
};

class QgsTestExternalStorageStoredContent : public QgsExternalStorageStoredContent
{
    Q_OBJECT

  public:

    QgsTestExternalStorageStoredContent( const QString &filePath, const QString &url )
      : QgsExternalStorageStoredContent(),
        mUrl( filePath.endsWith( QLatin1String( "mydir" ) ) ? url + "mydir/" : url )
    {}

    void store() override
    {
      mStatus = Qgis::ContentStatus::Running;
    }

    void cancel() override
    {
      mStatus = Qgis::ContentStatus::Canceled;
      emit canceled();
    };

    void error()
    {
      mStatus = Qgis::ContentStatus::Failed;
      mErrorString = QStringLiteral( "error" );
      emit errorOccurred( mErrorString );
    }

    void setProgress( double progress )
    {
      emit progressChanged( progress );
    }

    void finish()
    {
      mStatus = Qgis::ContentStatus::Finished;
      emit stored();
    }

    QString url() const override
    {
      return mUrl;
    }

  private:

    QString mUrl;

};

class QgsTestExternalStorage : public QgsExternalStorage
{
  public:

    QString type() const override { return QStringLiteral( "test" ); }

    QString displayName() const override { return QStringLiteral( "Test" ); }

    static QPointer<QgsTestExternalStorageStoredContent> sCurrentStoredContent;

  protected:

    QgsExternalStorageStoredContent *doStore( const QString &filePath, const QString &url, const QString &authcfg = QString() ) const override
    {
      Q_UNUSED( authcfg );
      sCurrentStoredContent = new QgsTestExternalStorageStoredContent( filePath, url );
      return sCurrentStoredContent;
    }

    QgsExternalStorageFetchedContent *doFetch( const QString &url, const QString &authcfg = QString() ) const override
    {
      Q_UNUSED( url );
      Q_UNUSED( authcfg );
      return nullptr;
    }
};

QPointer<QgsTestExternalStorageStoredContent> QgsTestExternalStorage::sCurrentStoredContent;

void TestQgsExternalStorageFileWidget::initTestCase()
{
  QgsApplication::externalStorageRegistry()->registerExternalStorage( new QgsTestExternalStorage() );
}

void TestQgsExternalStorageFileWidget::cleanupTestCase()
{
}

void TestQgsExternalStorageFileWidget::init()
{
}

void TestQgsExternalStorageFileWidget::cleanup()
{
}

void TestQgsExternalStorageFileWidget::testLayout_data()
{
  QTest::addColumn<QString>( "storageType" );

  QTest::newRow( "without external storage" ) << QString();
  QTest::newRow( "with external storage" ) << QStringLiteral( "test" );
}

void TestQgsExternalStorageFileWidget::testLayout()
{
  // test correct buttons are displayed according to different mode and interactions

  QFETCH( QString, storageType );

  QgsExternalStorageFileWidget w;
  w.setStorageType( storageType );
  w.show();

  QIcon editIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) );
  QIcon saveIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveEdits.svg" ) );

  // with link, read-only
  w.setReadOnly( true );
  w.setUseLink( true );

  QVERIFY( w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  // with link, edit mode
  w.setReadOnly( false );

  QVERIFY( w.mLinkLabel->isVisible() );
  QVERIFY( w.mLinkEditButton->isVisible() );
  QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  // with link, edit mode, we edit the link
  w.editLink();

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( w.mLinkEditButton->isVisible() );
  QCOMPARE( w.mLinkEditButton->icon(), saveIcon );
  QVERIFY( w.mLineEdit->isVisible() );
  QVERIFY( w.mLineEdit->isEnabled() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  // with link, edit mode, we finish editing the link
  w.editLink();

  QVERIFY( w.mLinkLabel->isVisible() );
  QVERIFY( w.mLinkEditButton->isVisible() );
  QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  // without link, read-only
  w.setUseLink( false );
  w.setReadOnly( true );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( w.mLineEdit->isVisible() );
  QVERIFY( !w.mLineEdit->isEnabled() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  // without link, edit mode
  w.setReadOnly( false );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( w.mLineEdit->isVisible() );
  QVERIFY( w.mLineEdit->isEnabled() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );
}

void TestQgsExternalStorageFileWidget::testStoring_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsExternalStorageFileWidget::testStoring()
{
  // test widget when an external storage is used

  QFETCH( bool, useLink );

  QgsExternalStorageFileWidget w;
  w.show();

  QIcon editIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) );

  w.setStorageType( "test" );
  w.setStorageUrlExpression( "'http://test.url.com/test/' || file_name(@selected_file_path)" );

  // start edit mode
  w.setUseLink( useLink );
  w.setReadOnly( false );

  const QStringList fileNames = QStringList() << "myfile1.txt" << "myfile2.txt" ;
  for ( QString fileName : fileNames )
  {
    QVERIFY( useLink == w.mLinkLabel->isVisible() );
    QVERIFY( useLink == w.mLinkEditButton->isVisible() );
    if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
    QVERIFY( useLink != w.mLineEdit->isVisible() );
    QVERIFY( w.mFileWidgetButton->isVisible() );
    QVERIFY( w.mFileWidgetButton->isEnabled() );
    QVERIFY( !w.mProgressLabel->isVisible() );
    QVERIFY( !w.mProgressBar->isVisible() );
    QVERIFY( !w.mCancelButton->isVisible() );

    QString currentLabel = useLink ? w.mLinkLabel->text() : w.mLineEdit->text();

    w.setSelectedFileNames( QStringList() << fileName );

    QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );

    QVERIFY( !w.mLinkLabel->isVisible() );
    QVERIFY( !w.mLinkEditButton->isVisible() );
    QVERIFY( !w.mLineEdit->isVisible() );
    QVERIFY( !w.mFileWidgetButton->isVisible() );
    QVERIFY( w.mProgressLabel->isVisible() );
    QVERIFY( w.mProgressBar->isVisible() );
    QCOMPARE( w.mProgressBar->value(), 0 );
    QVERIFY( w.mCancelButton->isVisible() );

    // link is not yet updated
    if ( useLink )
      QCOMPARE( currentLabel, w.mLinkLabel->text() );
    else
      QCOMPARE( currentLabel, w.mLineEdit->text() );

    QgsTestExternalStorage::sCurrentStoredContent->setProgress( 50 );
    QVERIFY( w.mProgressBar->isVisible() );
    QCOMPARE( w.mProgressBar->value(), 50 );

    QgsTestExternalStorage::sCurrentStoredContent->finish();

    QVERIFY( useLink == w.mLinkLabel->isVisible() );
    QVERIFY( useLink == w.mLinkEditButton->isVisible() );
    if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
    QVERIFY( useLink != w.mLineEdit->isVisible() );
    QVERIFY( w.mFileWidgetButton->isVisible() );
    QVERIFY( w.mFileWidgetButton->isEnabled() );
    QVERIFY( !w.mProgressLabel->isVisible() );
    QVERIFY( !w.mProgressBar->isVisible() );
    QVERIFY( !w.mCancelButton->isVisible() );
    if ( useLink )
      QCOMPARE( w.mLinkLabel->text(), QStringLiteral( "<a href=\"http://test.url.com/test/%1\">%1</a>" ).arg( fileName ) );
    else
      QCOMPARE( w.mLineEdit->text(), QStringLiteral( "http://test.url.com/test/%1" ).arg( fileName ) );
  }
}


void TestQgsExternalStorageFileWidget::testStoringSeveralFiles_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsExternalStorageFileWidget::testStoringSeveralFiles()
{
  // test widget when storing several files with an external storage
  QEventLoop loop;
  QFETCH( bool, useLink );

  QgsExternalStorageFileWidget w;
  w.show();

  QIcon editIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) );

  w.setStorageType( "test" );
  w.setStorageUrlExpression( "'http://test.url.com/test/' || file_name(@selected_file_path)" );
  w.setStorageMode( QgsFileWidget::GetMultipleFiles );

  // start edit mode
  w.setUseLink( useLink );
  w.setReadOnly( false );

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  w.setSelectedFileNames( QStringList() << QStringLiteral( "myfile1" ) << QStringLiteral( "myfile2" ) );

  QPointer<QgsTestExternalStorageStoredContent> content1 = QgsTestExternalStorage::sCurrentStoredContent;
  QVERIFY( content1 );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mProgressLabel->isVisible() );
  QVERIFY( w.mProgressBar->isVisible() );
  QVERIFY( w.mCancelButton->isVisible() );
  QVERIFY( w.mLinkLabel->text().isEmpty() );

  QgsTestExternalStorage::sCurrentStoredContent->finish();

  // second file is being stored
  QVERIFY( content1 );
  QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );
  QVERIFY( content1 != QgsTestExternalStorage::sCurrentStoredContent );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mProgressLabel->isVisible() );
  QVERIFY( w.mProgressBar->isVisible() );
  QVERIFY( w.mCancelButton->isVisible() );
  QVERIFY( w.mLinkLabel->text().isEmpty() );

  // wait for first file content to be destroyed
  connect( content1, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !content1 );
  QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );

  // end second store
  QgsTestExternalStorage::sCurrentStoredContent->finish();

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );
  if ( useLink )
    QCOMPARE( w.mLinkLabel->text(), QStringLiteral( "<a>\"http://test.url.com/test/myfile1\" \"http://test.url.com/test/myfile2\"</a>" ) );
  else
    QCOMPARE( w.mLineEdit->text(), QStringLiteral( "\"http://test.url.com/test/myfile1\" \"http://test.url.com/test/myfile2\"" ) );

  // wait for second file content to be destroyed
  connect( QgsTestExternalStorage::sCurrentStoredContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sCurrentStoredContent );
}

void TestQgsExternalStorageFileWidget::testStoringSeveralFilesError_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsExternalStorageFileWidget::testStoringSeveralFilesError()
{
  // test widget when storing several files with an external storage and an error occurred
  QEventLoop loop;
  QFETCH( bool, useLink );

  QgsExternalStorageFileWidget w;
  QgsMessageBar messageBar;
  w.show();

  QIcon editIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) );

  w.setStorageType( "test" );
  w.setStorageUrlExpression( "'http://test.url.com/test/' || file_name(@selected_file_path)" );
  w.setMessageBar( &messageBar );

  // start edit mode
  w.setUseLink( useLink );
  w.setReadOnly( false );

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  w.setSelectedFileNames( QStringList() << QStringLiteral( "myfile1" ) << QStringLiteral( "error.txt" ) );

  QPointer<QgsTestExternalStorageStoredContent> content1 = QgsTestExternalStorage::sCurrentStoredContent;
  QVERIFY( content1 );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mProgressLabel->isVisible() );
  QVERIFY( w.mProgressBar->isVisible() );
  QVERIFY( w.mCancelButton->isVisible() );
  QVERIFY( w.mLinkLabel->text().isEmpty() );
  QVERIFY( !messageBar.currentItem() );

  QgsTestExternalStorage::sCurrentStoredContent->finish();

  // second file is being stored
  QVERIFY( content1 );
  QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );
  QVERIFY( content1 != QgsTestExternalStorage::sCurrentStoredContent );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mProgressLabel->isVisible() );
  QVERIFY( w.mProgressBar->isVisible() );
  QVERIFY( w.mCancelButton->isVisible() );
  QVERIFY( w.mLinkLabel->text().isEmpty() );

  // wait for first file content to be destroyed
  connect( content1, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !content1 );
  QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );

  // error while storing second file
  QgsTestExternalStorage::sCurrentStoredContent->error();

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );
  if ( useLink )
    QVERIFY( w.mLinkLabel->text().isEmpty() );
  else
    QVERIFY( w.mLineEdit->text().isEmpty() );
  QVERIFY( messageBar.currentItem() );

  // wait for second file content to be destroyed
  connect( QgsTestExternalStorage::sCurrentStoredContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sCurrentStoredContent );
}


void TestQgsExternalStorageFileWidget::testStoringSeveralFilesCancel_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsExternalStorageFileWidget::testStoringSeveralFilesCancel()
{
  // test widget when storing several files with an external storage and user cancel operation
  QEventLoop loop;
  QFETCH( bool, useLink );

  QgsExternalStorageFileWidget w;
  w.show();

  QIcon editIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) );

  w.setStorageType( "test" );
  w.setStorageUrlExpression( "'http://test.url.com/test/' || file_name(@selected_file_path)" );

  // start edit mode
  w.setUseLink( useLink );
  w.setReadOnly( false );

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  w.setSelectedFileNames( QStringList() << QStringLiteral( "myfile1" ) << QStringLiteral( "error.txt" ) );

  QPointer<QgsTestExternalStorageStoredContent> content1 = QgsTestExternalStorage::sCurrentStoredContent;
  QVERIFY( content1 );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mProgressLabel->isVisible() );
  QVERIFY( w.mProgressBar->isVisible() );
  QVERIFY( w.mCancelButton->isVisible() );
  QVERIFY( w.mLinkLabel->text().isEmpty() );

  QgsTestExternalStorage::sCurrentStoredContent->finish();

  // second file is being stored
  QVERIFY( content1 );
  QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );
  QVERIFY( content1 != QgsTestExternalStorage::sCurrentStoredContent );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mProgressLabel->isVisible() );
  QVERIFY( w.mProgressBar->isVisible() );
  QVERIFY( w.mCancelButton->isVisible() );
  QVERIFY( w.mLinkLabel->text().isEmpty() );

  // wait for first file content to be destroyed
  connect( content1, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !content1 );
  QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );

  // cancel while storing second file
  QgsTestExternalStorage::sCurrentStoredContent->cancel();

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );
  if ( useLink )
    QVERIFY( w.mLinkLabel->text().isEmpty() );
  else
    QVERIFY( w.mLineEdit->text().isEmpty() );

  // wait for second file content to be destroyed
  connect( QgsTestExternalStorage::sCurrentStoredContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sCurrentStoredContent );
}


void TestQgsExternalStorageFileWidget::testStoringChangeFeature()
{
  // test widget with external storage to store files with different features

  QgsExternalStorageFileWidget w;
  w.show();

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "myfield" ), QVariant::String ) );

  QgsFeature f1( fields );
  f1.setAttribute( QStringLiteral( "myfield" ), QStringLiteral( "val1" ) );

  w.setStorageType( "test" );
  w.setStorageUrlExpression( "'http://test.url.com/' || attribute( @current_feature, 'myfield' )" );

  QgsExpressionContext expressionContext;
  expressionContext.appendScope( QgsExpressionContextUtils::formScope( f1 ) );
  w.setExpressionContext( expressionContext );

  w.setUseLink( false );
  w.setReadOnly( false );

  w.setSelectedFileNames( QStringList() << QStringLiteral( "blank" ) );

  QgsTestExternalStorage::sCurrentStoredContent->finish();

  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "http://test.url.com/val1" ) );

  QgsFeature f2( fields );
  f2.setAttribute( QStringLiteral( "myfield" ), QStringLiteral( "val2" ) );

  QgsExpressionContext expressionContext2;
  expressionContext2.appendScope( QgsExpressionContextUtils::formScope( f2 ) );
  w.setExpressionContext( expressionContext2 );

  w.setSelectedFileNames( QStringList() << QStringLiteral( "blank" ) );

  QgsTestExternalStorage::sCurrentStoredContent->finish();

  QCOMPARE( w.mLineEdit->text(), QStringLiteral( "http://test.url.com/val2" ) );
}

void TestQgsExternalStorageFileWidget::testStoringBadExpression_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsExternalStorageFileWidget::testStoringBadExpression()
{
  // test widget when an external storage is used and the given expression if incorrect

  QFETCH( bool, useLink );

  QgsExternalStorageFileWidget w;
  w.show();

  QIcon editIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) );

  w.setStorageType( "test" );
  w.setStorageUrlExpression( "'http://test.url.com/test/' || file_name(@not_existing_variable)" );

  // start edit mode
  w.setUseLink( useLink );
  w.setReadOnly( false );

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  w.setSelectedFileNames( QStringList() << QStringLiteral( "myfile" ) );

  QVERIFY( !QgsTestExternalStorage::sCurrentStoredContent );

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  // link is not updated
  QVERIFY( w.mLinkLabel->text().isEmpty() );
}

void TestQgsExternalStorageFileWidget::testStoringDirectory_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsExternalStorageFileWidget::testStoringDirectory()
{
  // test widget when storing a directory with an external storage
  QEventLoop loop;
  QFETCH( bool, useLink );

  QgsExternalStorageFileWidget w;
  w.show();

  QIcon editIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) );

  w.setStorageType( "test" );
  w.setStorageUrlExpression( "'http://test.url.com/test/'" );
  w.setStorageMode( QgsFileWidget::GetDirectory );

  // start edit mode
  w.setUseLink( useLink );
  w.setReadOnly( false );

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );

  w.setSelectedFileNames( QStringList() << "/tmp/mydir" );

  QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mProgressLabel->isVisible() );
  QVERIFY( w.mProgressBar->isVisible() );
  QVERIFY( w.mCancelButton->isVisible() );
  QVERIFY( w.mLinkLabel->text().isEmpty() );

  QgsTestExternalStorage::sCurrentStoredContent->finish();

  QVERIFY( useLink == w.mLinkLabel->isVisible() );
  QVERIFY( useLink == w.mLinkEditButton->isVisible() );
  if ( useLink ) QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( useLink != w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );
  if ( useLink )
    QCOMPARE( w.mLinkLabel->text(), QStringLiteral( "<a href=\"http://test.url.com/test/mydir/\"></a>" ) );
  else
    QCOMPARE( w.mLineEdit->text(), QStringLiteral( "http://test.url.com/test/mydir/" ) );

  // wait for file content to be destroyed
  connect( QgsTestExternalStorage::sCurrentStoredContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sCurrentStoredContent );
}

void TestQgsExternalStorageFileWidget::testDragAndDrop()
{
  // test widget when drag and droping urls with an external storage
  QEventLoop loop;

  QgsExternalStorageFileWidget w;
  w.show();

  std::unique_ptr<QMimeData> mime( new QMimeData() );
  mime->setUrls( QList<QUrl>() << QUrl::fromLocalFile( TEST_DATA_DIR + QStringLiteral( "/bug5598.shp" ) ) );

  QIcon editIcon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) );

  QVERIFY( !w.acceptDrops() );

  w.setReadOnly( true );
  w.setStorageType( "test" );
  w.setStorageUrlExpression( "'http://test.url.com/test/' || file_name(@selected_file_path)" );
  w.setStorageMode( QgsFileWidget::GetFile );
  QVERIFY( !w.acceptDrops() );

  // start edit mode
  w.setUseLink( true );
  w.setReadOnly( false );

  QVERIFY( w.mLinkLabel->isVisible() );
  QVERIFY( w.mLinkEditButton->isVisible() );
  QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );
  QVERIFY( w.acceptDrops() );

  std::unique_ptr<QDragEnterEvent> dragEvent( new QDragEnterEvent( QPoint( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton,  Qt::NoModifier ) );
  w.dragEnterEvent( dragEvent.get() );
  QVERIFY( dragEvent->isAccepted() );

  std::unique_ptr<QDropEvent> dropEvent( new QDropEvent( QPoint( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton,  Qt::NoModifier ) );
  w.dropEvent( dropEvent.get() );
  QVERIFY( dropEvent->isAccepted() );

  QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mProgressLabel->isVisible() );
  QVERIFY( w.mProgressBar->isVisible() );
  QVERIFY( w.mCancelButton->isVisible() );
  QVERIFY( w.mLinkLabel->text().isEmpty() );

  QgsTestExternalStorage::sCurrentStoredContent->finish();

  QVERIFY( w.mLinkLabel->isVisible() );
  QVERIFY( w.mLinkEditButton->isVisible() );
  QCOMPARE( w.mLinkEditButton->icon(), editIcon );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mFileWidgetButton->isEnabled() );
  QVERIFY( !w.mProgressLabel->isVisible() );
  QVERIFY( !w.mProgressBar->isVisible() );
  QVERIFY( !w.mCancelButton->isVisible() );
  QCOMPARE( w.mLinkLabel->text(), QStringLiteral( "<a href=\"http://test.url.com/test/bug5598.shp\">bug5598.shp</a>" ) );

  // wait for file content to be destroyed
  connect( QgsTestExternalStorage::sCurrentStoredContent, &QObject::destroyed, &loop, &QEventLoop::quit );
  loop.exec();
  QVERIFY( !QgsTestExternalStorage::sCurrentStoredContent );
}


QGSTEST_MAIN( TestQgsExternalStorageFileWidget )
#include "testqgsexternalstoragefilewidget.moc"
