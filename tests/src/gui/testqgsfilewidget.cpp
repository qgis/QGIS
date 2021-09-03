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
#include <memory>

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
};

void TestQgsFileWidget::initTestCase()
{
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

QGSTEST_MAIN( TestQgsFileWidget )
#include "testqgsfilewidget.moc"
