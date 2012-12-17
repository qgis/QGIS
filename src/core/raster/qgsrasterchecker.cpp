/***************************************************************************
     qgsrasterchecker.cpp
     --------------------------------------
    Date                 : 5 Sep 2012
    Copyright            : (C) 2012 by Radim Blazek
    Email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderregistry.h"
#include "qgsrasterchecker.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"

#include <qmath.h>
#include <QColor>
#include <QPainter>
#include <QImage>
#include <QTime>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDebug>
#include <QBuffer>

QgsRasterChecker::QgsRasterChecker( ) :
    mReport( "" )
{
  mTabStyle = "border-spacing: 0px; border-width: 1px 1px 0 0; border-style: solid;";
  mCellStyle = "border-width: 0 0 1px 1px; border-style: solid; font-size: smaller; text-align: center;";
  mOkStyle = "background: #00ff00;";
  mErrStyle = "background: #ff0000;";
  mErrMsgStyle = "color: #ff0000;";
}

bool QgsRasterChecker::runTest( QString theVerifiedKey, QString theVerifiedUri,
                                QString theExpectedKey, QString theExpectedUri )
{
  bool ok = true;
  mReport += "\n\n";

  //QgsRasterDataProvider* verifiedProvider = QgsRasterLayer::loadProvider( theVerifiedKey, theVerifiedUri );
  QgsRasterDataProvider* verifiedProvider = ( QgsRasterDataProvider* ) QgsProviderRegistry::instance()->provider( theVerifiedKey, theVerifiedUri );
  if ( !verifiedProvider || !verifiedProvider->isValid() )
  {
    error( QString( "Cannot load provider %1 with URI: %2" ).arg( theVerifiedKey ).arg( theVerifiedUri ), mReport );
    ok = false;
  }

  //QgsRasterDataProvider* expectedProvider = QgsRasterLayer::loadProvider( theExpectedKey, theExpectedUri );
  QgsRasterDataProvider* expectedProvider = ( QgsRasterDataProvider* ) QgsProviderRegistry::instance()->provider( theExpectedKey, theExpectedUri );
  if ( !expectedProvider || !expectedProvider->isValid() )
  {
    error( QString( "Cannot load provider %1 with URI: %2" ).arg( theExpectedKey ).arg( theExpectedUri ), mReport );
    ok = false;
  }

  if ( !ok ) return false;

  mReport += QString( "Verified URI: %1<br>" ).arg( theVerifiedUri.replace( "&", "&amp;" ) );
  mReport += QString( "Expected URI: %1<br>" ).arg( theExpectedUri.replace( "&", "&amp;" ) );

  mReport += "<br>";
  mReport += QString( "<table style='%1'>\n" ).arg( mTabStyle );
  mReport += compareHead();

  compare( "Band count", verifiedProvider->bandCount(), expectedProvider->bandCount(), mReport, ok );

  compare( "Width", verifiedProvider->xSize(), expectedProvider->xSize(), mReport, ok );
  compare( "Height", verifiedProvider->ySize(), expectedProvider->ySize(), mReport, ok );

  compareRow( "Extent", verifiedProvider->extent().toString(), expectedProvider->extent().toString(), mReport, verifiedProvider->extent() == expectedProvider->extent() );

  if ( verifiedProvider->extent() != expectedProvider->extent() ) ok = false;


  mReport += "</table>\n";

  if ( !ok ) return false;

  bool allOk = true;
  for ( int band = 1; band <= expectedProvider->bandCount(); band++ )
  {
    bool bandOk = true;
    mReport += QString( "<h3>Band %1</h3>\n" ).arg( band );
    mReport += QString( "<table style='%1'>\n" ).arg( mTabStyle );
    mReport += compareHead();

    // Data types may differ (?)
    bool typesOk = true;
    compare( "Source data type", verifiedProvider->srcDataType( band ), expectedProvider->srcDataType( band ), mReport, typesOk );
    compare( "Data type", verifiedProvider->dataType( band ), expectedProvider->dataType( band ), mReport, typesOk ) ;

    compare( "No data (NULL) value", verifiedProvider->noDataValue( band ), expectedProvider->noDataValue( band ), mReport, typesOk );

    bool statsOk = true;
    QgsRasterBandStats verifiedStats =  verifiedProvider->bandStatistics( band );
    QgsRasterBandStats expectedStats =  expectedProvider->bandStatistics( band );

    // Min/max may 'slightly' differ, for big numbers however, the difference may
    // be quite big, for example for Float32 with max -3.332e+38, the difference is 1.47338e+24
    double tol = tolerance( expectedStats.minimumValue );
    compare( "Minimum value", verifiedStats.minimumValue, expectedStats.minimumValue, mReport, statsOk, tol );
    tol = tolerance( expectedStats.maximumValue );
    compare( "Maximum value", verifiedStats.maximumValue, expectedStats.maximumValue, mReport, statsOk, tol );

    // TODO: enable once fixed (WCS excludes nulls but GDAL does not)
    //compare( "Cells count", verifiedStats.elementCount, expectedStats.elementCount, mReport, statsOk );

    tol = tolerance( expectedStats.mean );
    compare( "Mean", verifiedStats.mean, expectedStats.mean, mReport, statsOk, tol );

    // stdDev usually differ significantly
    tol = tolerance( expectedStats.stdDev, 1 );
    compare( "Standard deviation", verifiedStats.stdDev, expectedStats.stdDev, mReport, statsOk, tol );

    mReport += "</table>";
    mReport += "<br>";

    if ( !bandOk )
    {
      allOk = false;
      continue;
    }

    if ( !statsOk || !typesOk )
    {
      allOk = false;
      // create values table anyway so that values are available
    }

    mReport += "<table><tr>";
    mReport += "<td>Data comparison</td>";
    mReport += QString( "<td style='%1 %2 border: 1px solid'>correct&nbsp;value</td>" ).arg( mCellStyle ).arg( mOkStyle );
    mReport += "<td></td>";
    mReport += QString( "<td style='%1 %2 border: 1px solid'>wrong&nbsp;value<br>expected value</td></tr>" ).arg( mCellStyle ).arg( mErrStyle );
    mReport += "</tr></table>";
    mReport += "<br>";

    int width = expectedProvider->xSize();
    int height = expectedProvider->ySize();
    int blockSize =  width * height * QgsRasterBlock::typeSize( expectedProvider->dataType( band ) ) ;
    void * expectedData = malloc( blockSize );
    void * verifiedData = malloc( blockSize );

    expectedProvider->readBlock( band, expectedProvider->extent(), width, height, expectedData );
    verifiedProvider->readBlock( band, expectedProvider->extent(), width, height, verifiedData );

    // compare data values
    QString htmlTable = QString( "<table style='%1'>" ).arg( mTabStyle );
    for ( int row = 0; row < height; row ++ )
    {
      htmlTable += "<tr>";
      for ( int col = 0; col < width; col ++ )
      {
        bool cellOk = true;
        double verifiedVal =  QgsRasterBlock::readValue( verifiedData,  verifiedProvider->dataType( band ), row * width + col );
        double expectedVal =  QgsRasterBlock::readValue( expectedData,  expectedProvider->dataType( band ), row * width + col );

        QString valStr;
        if ( compare( verifiedVal, expectedVal, 0 ) )
        {
          valStr = QString( "%1" ).arg( verifiedVal );
        }
        else
        {
          cellOk = false;
          allOk = false;
          valStr = QString( "%1<br>%2" ).arg( verifiedVal ).arg( expectedVal );
        }
        htmlTable += QString( "<td style='%1 %2'>%3</td>" ).arg( mCellStyle ).arg( cellOk ? mOkStyle : mErrStyle ).arg( valStr );
      }
      htmlTable += "</tr>";
    }
    htmlTable += "</table>";

    mReport += htmlTable;

    free( expectedData );
    free( verifiedData );
  }
  delete verifiedProvider;
  delete expectedProvider;
  return allOk;
}

void QgsRasterChecker::error( QString theMessage, QString &theReport )
{
  theReport += QString( "<font style='%1'>Error: " ).arg( mErrMsgStyle );
  theReport += theMessage;
  theReport += "</font>";
}

double QgsRasterChecker::tolerance( double val, int places )
{
  // float precision is about 7 decimal digits, double about 16
  // default places = 6
  return 1. * qPow( 10, qRound( log10( qAbs( val ) ) - places ) );
}

QString QgsRasterChecker::compareHead()
{
  QString html;
  html += QString( "<tr><th style='%1'>Param name</th><th style='%1'>Verified value</th><th style='%1'>Eexpected value</th><th style='%1'>Difference</th><th style='%1'>Tolerance</th></tr>" ).arg( mCellStyle );
  return html;
}

void QgsRasterChecker::compare( QString theParamName, int verifiedVal, int expectedVal, QString &theReport, bool &theOk )
{
  bool ok = verifiedVal == expectedVal;
  compareRow( theParamName, QString::number( verifiedVal ), QString::number( expectedVal ), theReport, ok, QString::number( verifiedVal - expectedVal ) );
  if ( !ok ) theOk = false;
}

bool QgsRasterChecker::compare( double verifiedVal, double expectedVal, double theTolerance )
{
  // values may be nan
  return ( qIsNaN( verifiedVal ) && qIsNaN( expectedVal ) ) || ( qAbs( verifiedVal - expectedVal ) <= theTolerance );
}

void QgsRasterChecker::compare( QString theParamName, double verifiedVal, double expectedVal, QString &theReport, bool &theOk, double theTolerance )
{
  bool ok = compare( verifiedVal, expectedVal, theTolerance );
  compareRow( theParamName, QString::number( verifiedVal ), QString::number( expectedVal ), theReport, ok, QString::number( verifiedVal - expectedVal ), QString::number( theTolerance ) );
  if ( !ok ) theOk = false;
}

void QgsRasterChecker::compareRow( QString theParamName, QString verifiedVal, QString expectedVal, QString &theReport, bool theOk, QString theDifference, QString theTolerance )
{
  theReport += "<tr>\n";
  theReport += QString( "<td style='%1'>%2</td><td style='%1 %3'>%4</td><td style='%1'>%5</td>\n" ).arg( mCellStyle ).arg( theParamName ).arg( theOk ? mOkStyle : mErrStyle ).arg( verifiedVal ).arg( expectedVal );
  theReport += QString( "<td style='%1'>%2</td>\n" ).arg( mCellStyle ).arg( theDifference );
  theReport += QString( "<td style='%1'>%2</td>\n" ).arg( mCellStyle ).arg( theTolerance );
  theReport += "</tr>";
}
