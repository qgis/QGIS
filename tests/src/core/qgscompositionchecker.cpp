/***************************************************************************
     qgscompositionchecker.cpp - check rendering of Qgscomposition against an expected image
                     --------------------------------------
               Date                 : 5 Juli 2012
               Copyright            : (C) 2012 by Marco Hugentobler
               email                : marco@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscompositionchecker.h"
#include "qgscomposition.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QPainter>

QgsCompositionChecker::QgsCompositionChecker( const QString& testName, QgsComposition* composition )
    : QgsRenderChecker( ),
    mTestName( testName ),
    mComposition( composition )
{
}

QgsCompositionChecker::QgsCompositionChecker()
{
}

QgsCompositionChecker::~QgsCompositionChecker()
{
}

bool QgsCompositionChecker::testComposition( QString &report, int page, int pixelDiff )
{
  if ( !mComposition )
  {
    return false;
  }

#if 0
  //fake mode to generate expected image
  //assume 300 dpi and size of the control image 3507 * 2480
  QImage outputImage( QSize( 3507, 2480 ), QImage::Format_ARGB32 );
  mComposition->setPlotStyle( QgsComposition::Print );
  outputImage.setDotsPerMeterX( 300 / 25.4 * 1000 );
  outputImage.setDotsPerMeterY( 300 / 25.4 * 1000 );
  outputImage.fill( 0 );
  QPainter p( &outputImage );
  //QRectF sourceArea( 0, 0, mComposition->paperWidth(), mComposition->paperHeight() );
  //QRectF targetArea( 0, 0, 3507, 2480 );
  mComposition->renderPage( &p, page );
  p.end();
  outputImage.save( "/tmp/composerhtml_table_control.png", "PNG" );
  return false;
#endif //0

  //load expected image
  setControlName( "expected_" + mTestName );
  QImage expectedImage( mExpectedImageFile );

  //get width/height, create image and render the composition to it
  int width = expectedImage.width();
  int height = expectedImage.height();
  QImage outputImage( QSize( width, height ), QImage::Format_ARGB32 );

  mComposition->setPlotStyle( QgsComposition::Print );
  outputImage.setDotsPerMeterX( expectedImage.dotsPerMeterX() );
  outputImage.setDotsPerMeterY( expectedImage.dotsPerMeterX() );
  outputImage.fill( 0 );
  QPainter p( &outputImage );
  mComposition->renderPage( &p, page );
  p.end();

  QString renderedFilePath = QDir::tempPath() + QDir::separator() + QFileInfo( mTestName ).baseName() + "_rendered.png";
  outputImage.save( renderedFilePath, "PNG" );

  QString diffFilePath = QDir::tempPath() + QDir::separator() + QFileInfo( mTestName ).baseName() + "_result_diff.png";

  bool testResult = compareImages( mTestName, pixelDiff, renderedFilePath );

  QString myDashMessage = "<DartMeasurementFile name=\"Rendered Image " + mTestName + "\""
                          " type=\"image/png\">" + renderedFilePath +
                          "</DartMeasurementFile>"
                          "<DartMeasurementFile name=\"Expected Image " + mTestName + "\" type=\"image/png\">" +
                          mExpectedImageFile + "</DartMeasurementFile>"
                          "<DartMeasurementFile name=\"Difference Image " + mTestName + "\" type=\"image/png\">" +
                          diffFilePath + "</DartMeasurementFile>";
  qDebug( ) << myDashMessage;

  report += mReport;
  return testResult;
}
