/***************************************************************************
    testqgsfilewidget.cpp
     --------------------------------------
    Date                 : December 2014
    Copyright            : (C) 2014 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include "qgsfilewidget.h"
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

class TestQgsFileWidget: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void relativePath();
    void toUrl();
    void testDroppedFiles();
    void testMultipleFiles();
    void testSplitFilePaths();
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
};

class QgsTestExternalStorageStoredContent : public QgsExternalStorageStoredContent
{
    Q_OBJECT

  public:

    QgsTestExternalStorageStoredContent( const QString &filePath, const QString &url )
      : QgsExternalStorageStoredContent(),
        mUrl( filePath.endsWith( QStringLiteral( "mydir" ) ) ? url + "mydir/" : url )
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

void TestQgsFileWidget::initTestCase()
{
  QgsApplication::externalStorageRegistry()->registerExternalStorage( new QgsTestExternalStorage() );
}

void TestQgsFileWidget::cleanupTestCase()
{
}

void TestQgsFileWidget::init()
{
}

void TestQgsFileWidget::cleanup()
{
}

void TestQgsFileWidget::relativePath()
{
  QgsFileWidget *w = new QgsFileWidget();
  w->setDefaultRoot( QStringLiteral( "/home/test" ) );
  w->setRelativeStorage( QgsFileWidget::Absolute );
  QCOMPARE( w->relativePath( "/home/test2/file1.ext", true ), QString( "/home/test2/file1.ext" ) );
  QCOMPARE( w->relativePath( "/home/test2/file2.ext", false ), QString( "/home/test2/file2.ext" ) );
  w->setRelativeStorage( QgsFileWidget::RelativeDefaultPath );
  QCOMPARE( w->relativePath( "/home/test2/file3.ext", true ), QString( "../test2/file3.ext" ) );
  QCOMPARE( w->relativePath( "../test2/file4.ext", true ), QString( "../test2/file4.ext" ) );
  QCOMPARE( w->relativePath( "/home/test2/file5.ext", false ), QString( "/home/test2/file5.ext" ) );
  QCOMPARE( w->relativePath( "../test2/file6.ext", false ), QString( "/home/test2/file6.ext" ) );
}

void TestQgsFileWidget::toUrl()
{
  QgsFileWidget *w = new QgsFileWidget();
  w->setDefaultRoot( QStringLiteral( "/home/test" ) );
  w->setRelativeStorage( QgsFileWidget::Absolute );
  w->setFullUrl( true );
  QCOMPARE( w->toUrl( "/home/test2/file1.ext" ), QString( "<a href=\"file:///home/test2/file1.ext\">/home/test2/file1.ext</a>" ) );
  w->setFullUrl( false );
  QCOMPARE( w->toUrl( "/home/test2/file2.ext" ), QString( "<a href=\"file:///home/test2/file2.ext\">file2.ext</a>" ) );
  w->setRelativeStorage( QgsFileWidget::RelativeDefaultPath );
  w->setFullUrl( true );
  QCOMPARE( w->toUrl( "/home/test2/file3.ext" ), QString( "<a href=\"file:///home/test2/file3.ext\">/home/test2/file3.ext</a>" ) );
  QCOMPARE( w->toUrl( "../test2/file4.ext" ), QString( "<a href=\"file:///home/test2/file4.ext\">../test2/file4.ext</a>" ) );
  w->setFullUrl( false );
  QCOMPARE( w->toUrl( "/home/test2/file5.ext" ), QString( "<a href=\"file:///home/test2/file5.ext\">file5.ext</a>" ) );
  QCOMPARE( w->toUrl( "../test2/file6.ext" ), QString( "<a href=\"file:///home/test2/file6.ext\">file6.ext</a>" ) );
}

void TestQgsFileWidget::testDroppedFiles()
{
  QgsFileWidget *w = new QgsFileWidget();
  w->setStorageMode( QgsFileWidget::GetFile );

  // should not accept dropped folders
  std::unique_ptr< QMimeData > mime( new QMimeData() );
  mime->setUrls( QList<QUrl>() << QUrl::fromLocalFile( TEST_DATA_DIR ) );
  std::unique_ptr< QDropEvent > event( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier ) );

  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QVERIFY( w->lineEdit()->text().isEmpty() );

  // but dropped files should be fine
  mime->setUrls( QList<QUrl>() << QUrl::fromLocalFile( TEST_DATA_DIR + QStringLiteral( "/bug5598.shp" ) ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton,  Qt::NoModifier ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( TEST_DATA_DIR + QStringLiteral( "/bug5598.shp" ) ) );

  // also should support files dragged from browser
  mime->setUrls( QList<QUrl>() );
  QgsMimeDataUtils::Uri uri;
  uri.uri = TEST_DATA_DIR + QStringLiteral( "/mesh/quad_and_triangle.2dm" );
  QgsMimeDataUtils::UriList uriList;
  uriList << uri;
  mime.reset( QgsMimeDataUtils::encodeUriList( uriList ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton,  Qt::NoModifier ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( TEST_DATA_DIR + QStringLiteral( "/mesh/quad_and_triangle.2dm" ) ) );

  QgsBrowserModel m;
  m.initialize();
  QgsLayerItem *layerItem = new QgsLayerItem( nullptr, QStringLiteral( "Test" ), QString(), TEST_DATA_DIR + QStringLiteral( "/mesh/quad_and_triangle.txt" ), Qgis::BrowserLayerType::Mesh, "mdal" );
  m.driveItems().first()->addChild( layerItem );
  mime.reset( m.mimeData( QModelIndexList() << m.findItem( layerItem ) ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( QString( TEST_DATA_DIR ) + QStringLiteral( "/mesh/quad_and_triangle.txt" ) ) );

  // plain text should also be permitted
  mime = std::make_unique< QMimeData >();
  mime->setText( TEST_DATA_DIR + QStringLiteral( "/mesh/quad_and_triangle.2dm" ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton,  Qt::NoModifier ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( TEST_DATA_DIR + QStringLiteral( "/mesh/quad_and_triangle.2dm" ) ) );

  mime.reset( new QMimeData() );
  mime->setUrls( QList<QUrl>() << QUrl::fromLocalFile( TEST_DATA_DIR + QStringLiteral( "/bug5598.shp" ) ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton,  Qt::NoModifier ) );
  // with file filter
  w->setFilter( QStringLiteral( "Data (*.shp)" ) );
  w->setFilePath( QString() );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( TEST_DATA_DIR + QStringLiteral( "/bug5598.shp" ) ) );
  w->setFilePath( QString() );
  // should be rejected, not compatible with filter
  mime->setUrls( QList<QUrl>() << QUrl::fromLocalFile( TEST_DATA_DIR + QStringLiteral( "/encoded_html.html" ) ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton,  Qt::NoModifier ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QVERIFY( w->lineEdit()->text().isEmpty() );
  // new filter, should be allowed now
  w->setFilter( QStringLiteral( "Data (*.shp);;HTML (*.HTML)" ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( TEST_DATA_DIR + QStringLiteral( "/encoded_html.html" ) ) );

  //try with wildcard filter
  w->setFilter( QStringLiteral( "All files (*.*);;Data (*.shp);;HTML (*.HTML)" ) );
  mime->setUrls( QList<QUrl>() << QUrl::fromLocalFile( TEST_DATA_DIR + QStringLiteral( "/bug5598.prj" ) ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton,  Qt::NoModifier ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( TEST_DATA_DIR + QStringLiteral( "/bug5598.prj" ) ) );

  // try with folders
  w->setStorageMode( QgsFileWidget::GetDirectory );
  w->setFilePath( QString() );
  // dropping a file should accept only the folder containing that file
  mime->setUrls( QList<QUrl>() << QUrl::fromLocalFile( TEST_DATA_DIR + QStringLiteral( "/mesh/quad_and_triangle.2dm" ) ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton,  Qt::NoModifier ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( QString( TEST_DATA_DIR ) +  QStringLiteral( "/mesh" ) ) );

  // but dropping a folder should work
  mime->setUrls( QList<QUrl>() << QUrl::fromLocalFile( TEST_DATA_DIR ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( TEST_DATA_DIR ) );

  // integration test - dropping a directory item's mime data
  QgsDirectoryItem *dirItem = new QgsDirectoryItem( nullptr, QStringLiteral( "Test" ), TEST_DATA_DIR + QStringLiteral( "/mesh" ) );
  m.driveItems().first()->addChild( dirItem );
  mime.reset( m.mimeData( QModelIndexList() << m.findItem( dirItem ) ) );
  event.reset( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier ) );
  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QString( QString( TEST_DATA_DIR ) + QStringLiteral( "/mesh" ) ) );
}

void TestQgsFileWidget::testMultipleFiles()
{
  QgsFileWidget *w = new QgsFileWidget();
  w->setStorageMode( QgsFileWidget::GetMultipleFiles );

  std::unique_ptr< QMimeData > mime( new QMimeData() );
  mime->setUrls( QList<QUrl>() << QUrl::fromLocalFile( TEST_DATA_DIR + QStringLiteral( "/bug5598.shp" ) )
                 << QUrl::fromLocalFile( TEST_DATA_DIR + QStringLiteral( "/elev.gpx" ) ) );
  const std::unique_ptr< QDropEvent > event( new QDropEvent( QPointF( 1, 1 ), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier ) );

  qobject_cast< QgsFileDropEdit * >( w->lineEdit() )->dropEvent( event.get() );
  QCOMPARE( w->lineEdit()->text(), QStringLiteral( "\"%1\" \"%2\"" ).arg( TEST_DATA_DIR + QStringLiteral( "/bug5598.shp" ) )
            .arg( TEST_DATA_DIR + QStringLiteral( "/elev.gpx" ) ) );
}


void TestQgsFileWidget::testSplitFilePaths()
{
  const QString path = QString( TEST_DATA_DIR + QStringLiteral( "/bug5598.shp" ) );
  QCOMPARE( QgsFileWidget::splitFilePaths( QStringLiteral( "\"%1\" \"%1\"" ).arg( path ) ), QStringList() << path << path );
  QCOMPARE( QgsFileWidget::splitFilePaths( QStringLiteral( "\"%1\"   \"%1\"" ).arg( path ) ), QStringList() << path << path );
  QCOMPARE( QgsFileWidget::splitFilePaths( QStringLiteral( " \"%1\"   \"%1\"" ).arg( path ) ), QStringList() << path << path );
  QCOMPARE( QgsFileWidget::splitFilePaths( QStringLiteral( " \"%1\"   \"%1\" " ).arg( path ) ), QStringList() << path << path );
  QCOMPARE( QgsFileWidget::splitFilePaths( QStringLiteral( "\"%1\"   \"%1\" " ).arg( path ) ), QStringList() << path << path );
  QCOMPARE( QgsFileWidget::splitFilePaths( path ), QStringList() << path );
}

void TestQgsFileWidget::testLayout_data()
{
  QTest::addColumn<QString>( "storageType" );

  QTest::newRow( "without external storage" ) << QString();
  QTest::newRow( "with external storage" ) << QStringLiteral( "test" );
}

void TestQgsFileWidget::testLayout()
{
  // test correct buttons are displayed according to different mode and interactions

  QFETCH( QString, storageType );

  QgsFileWidget w;
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

void TestQgsFileWidget::testStoring_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsFileWidget::testStoring()
{
  // test widget when an external storage is used

  QFETCH( bool, useLink );

  QgsFileWidget w;
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

  w.setSelectedFileNames( QStringList() << QStringLiteral( "myfile" ) );

  QVERIFY( QgsTestExternalStorage::sCurrentStoredContent );

  QVERIFY( !w.mLinkLabel->isVisible() );
  QVERIFY( !w.mLinkEditButton->isVisible() );
  QVERIFY( !w.mLineEdit->isVisible() );
  QVERIFY( !w.mFileWidgetButton->isVisible() );
  QVERIFY( w.mProgressLabel->isVisible() );
  QVERIFY( w.mProgressBar->isVisible() );
  QVERIFY( w.mCancelButton->isVisible() );

  // link is not yet updated
  QVERIFY( w.mLinkLabel->text().isEmpty() );

  QgsTestExternalStorage::sCurrentStoredContent->finish();
  QCoreApplication::processEvents();

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
    QCOMPARE( w.mLinkLabel->text(), QStringLiteral( "<a href=\"http://test.url.com/test/myfile\">myfile</a>" ) );
  else
    QCOMPARE( w.mLineEdit->text(), QStringLiteral( "http://test.url.com/test/myfile" ) );
}


void TestQgsFileWidget::testStoringSeveralFiles_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsFileWidget::testStoringSeveralFiles()
{
  // test widget when storing several files with an external storage
  QEventLoop loop;
  QFETCH( bool, useLink );

  QgsFileWidget w;
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
  QCoreApplication::processEvents();

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
  QCoreApplication::processEvents();

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

void TestQgsFileWidget::testStoringSeveralFilesError_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsFileWidget::testStoringSeveralFilesError()
{
  // test widget when storing several files with an external storage and an error occured
  QEventLoop loop;
  QFETCH( bool, useLink );

  QgsFileWidget w;
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
  QCoreApplication::processEvents();

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
  QCoreApplication::processEvents();

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


void TestQgsFileWidget::testStoringSeveralFilesCancel_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsFileWidget::testStoringSeveralFilesCancel()
{
  // test widget when storing several files with an external storage and user cancel operation
  QEventLoop loop;
  QFETCH( bool, useLink );

  QgsFileWidget w;
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
  QCoreApplication::processEvents();

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
  QCoreApplication::processEvents();

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


void TestQgsFileWidget::testStoringChangeFeature()
{
  // test widget with external storage to store files with different features

  QgsFileWidget w;
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

void TestQgsFileWidget::testStoringBadExpression_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsFileWidget::testStoringBadExpression()
{
  // test widget when an external storage is used and the given expression if incorrect

  QFETCH( bool, useLink );

  QgsFileWidget w;
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

void TestQgsFileWidget::testStoringDirectory_data()
{
  QTest::addColumn<bool>( "useLink" );

  QTest::newRow( "use link" ) << true;
  QTest::newRow( "don't use link" ) << false;
}

void TestQgsFileWidget::testStoringDirectory()
{
  // test widget when storing a directory with an external storage
  QEventLoop loop;
  QFETCH( bool, useLink );

  QgsFileWidget w;
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
  QCoreApplication::processEvents();

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

QGSTEST_MAIN( TestQgsFileWidget )
#include "testqgsfilewidget.moc"
