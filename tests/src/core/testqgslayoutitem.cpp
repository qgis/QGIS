/***************************************************************************
                         testqgslayoutitem.cpp
                         -----------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitem.h"
#include "qgslayout.h"
#include "qgsmultirenderchecker.h"
#include "qgstest.h"
#include <QObject>
#include <QPainter>
#include <QImage>

class TestQgsLayoutItem: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //test creation of QgsLayoutItem

  private:

    //simple item for testing, since some methods in QgsLayoutItem are pure virtual
    class TestItem : public QgsLayoutItem
    {
      public:

        TestItem( QgsLayout *layout ) : QgsLayoutItem( layout ) {}
        ~TestItem() {}

        //implement pure virtual methods
        int type() const { return QgsLayoutItemRegistry::LayoutItem + 101; }
        void draw( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
        {
          Q_UNUSED( itemStyle );
          Q_UNUSED( pWidget );
          painter->save();
          painter->setRenderHint( QPainter::Antialiasing, false );
          painter->setPen( Qt::NoPen );
          painter->setBrush( QColor( 255, 100, 100, 200 ) );
          painter->drawRect( rect() );
          painter->restore();
        }
    };

    QgsLayout *mLayout = nullptr;
    QString mReport;

    bool renderCheck( QString testName, QImage &image, int mismatchCount );

};

void TestQgsLayoutItem::initTestCase()
{
  mLayout = new QgsLayout();
  mReport = "<h1>Layout Item Tests</h1>\n";
}

void TestQgsLayoutItem::cleanupTestCase()
{
  delete mLayout;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsLayoutItem::init()
{

}

void TestQgsLayoutItem::cleanup()
{

}

void TestQgsLayoutItem::creation()
{
  TestItem *item = new TestItem( mLayout );
  QVERIFY( item );
  delete item;
}

bool TestQgsLayoutItem::renderCheck( QString testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + QDir::separator();
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsLayoutItem )
#include "testqgslayoutitem.moc"
