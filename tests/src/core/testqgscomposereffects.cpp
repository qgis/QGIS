/***************************************************************************
                         testqgscomposereffects.cpp
                         ----------------------
    begin                : April 2013
    copyright            : (C) 2013 by Marco Hugentobler, Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgsmultirenderchecker.h"
#include "qgscomposershape.h"
#include "qgsproject.h"
#include <QObject>
#include "qgstest.h"
#include <QColor>
#include <QPainter>

class TestQgsComposerEffects : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerEffects() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void blend_modes(); //test if composer item blending is functioning
    void transparency(); //test if composer transparency is functioning

  private:
    QgsComposition *mComposition = nullptr;
    QgsComposerShape *mComposerRect1 = nullptr;
    QgsComposerShape *mComposerRect2 = nullptr;
    QString mReport;
};

void TestQgsComposerEffects::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create composition with two rectangles

  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerRect1 = new QgsComposerShape( 20, 20, 150, 100, mComposition );
  mComposerRect1->setShapeType( QgsComposerShape::Rectangle );
  mComposerRect1->setBackgroundColor( QColor::fromRgb( 255, 150, 0 ) );
  mComposition->addComposerShape( mComposerRect1 );
  mComposerRect2 = new QgsComposerShape( 50, 50, 150, 100, mComposition );
  mComposerRect2->setBackgroundColor( QColor::fromRgb( 0, 100, 150 ) );
  mComposerRect2->setShapeType( QgsComposerShape::Rectangle );
  mComposition->addComposerShape( mComposerRect2 );

  mReport = QStringLiteral( "<h1>Composer Effects Tests</h1>\n" );
}

void TestQgsComposerEffects::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsComposerEffects::init()
{

}

void TestQgsComposerEffects::cleanup()
{

}

void TestQgsComposerEffects::blend_modes()
{
  mComposerRect2->setBlendMode( QPainter::CompositionMode_Multiply );

  QgsCompositionChecker checker( QStringLiteral( "composereffects_blend" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_effects" ) );
  QVERIFY( checker.testComposition( mReport ) );
  // reset blending
  mComposerRect2->setBlendMode( QPainter::CompositionMode_SourceOver );
}

void TestQgsComposerEffects::transparency()
{
  mComposerRect2->setItemOpacity( 0.5 );

  QgsCompositionChecker checker( QStringLiteral( "composereffects_transparency" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_effects" ) );
  QVERIFY( checker.testComposition( mReport ) );
}

QGSTEST_MAIN( TestQgsComposerEffects )
#include "testqgscomposereffects.moc"
