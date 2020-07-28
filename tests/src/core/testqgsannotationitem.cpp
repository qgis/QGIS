/***************************************************************************
                         testqgsannotationitem.cpp
                         -----------------------
    begin                : October 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsannotationitem.h"
#include "qgsannotationitemregistry.h"
#include "qgsmultirenderchecker.h"
#include "qgstest.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"

#include <QObject>
#include <QPainter>
#include <QImage>
#include <QtTest/QSignalSpy>


//simple item for testing, since some methods in QgsAnnotationItem are pure virtual
class TestItem : public QgsAnnotationItem
{

  public:

    TestItem() : QgsAnnotationItem( QgsCoordinateReferenceSystem() )
    {
    }

    //implement pure virtual methods
    QString type() const override { return QStringLiteral( "test_item" ); }

    void render( QgsRenderContext &, QgsFeedback * ) override
    {
    }

    TestItem *clone() override
    {
      return new TestItem();
    }

    bool writeXml( QDomElement &, QDomDocument &, const QgsReadWriteContext & ) const override
    {
      return true;
    }

    bool readXml( const QDomElement &, const QgsReadWriteContext & ) override
    {
      return true;
    }
};


class TestQgsAnnotationItem: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

  private:

    QString mReport;

    bool renderCheck( QString testName, QImage &image, int mismatchCount );

};

void TestQgsAnnotationItem::initTestCase()
{
  mReport = QStringLiteral( "<h1>Annotation Item Tests</h1>\n" );
}

void TestQgsAnnotationItem::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsAnnotationItem::init()
{

}

void TestQgsAnnotationItem::cleanup()
{

}

bool TestQgsAnnotationItem::renderCheck( QString testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + QDir::separator();
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "annotations" ) );
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}


QGSTEST_MAIN( TestQgsAnnotationItem )
#include "testqgsannotationitem.moc"
