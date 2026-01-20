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

#include "qgsrasterchecker.h"

#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"

#include <QBuffer>
#include <QByteArray>
#include <QColor>
#include <QCryptographicHash>
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QTime>

QgsRasterChecker::QgsRasterChecker()
{
  mTabStyle = u"border-spacing: 0px; border-width: 1px 1px 0 0; border-style: solid;"_s;
  mCellStyle = u"border-width: 0 0 1px 1px; border-style: solid; font-size: smaller; text-align: center;"_s;
  mOkStyle = u"background: #00ff00;"_s;
  mErrStyle = u"background: #ff0000;"_s;
  mErrMsgStyle = u"color: #ff0000;"_s;
}

bool QgsRasterChecker::runTest( const QString &verifiedKey, QString verifiedUri,
                                const QString &expectedKey, QString expectedUri )
{
  bool ok = true;
  mReport += "\n\n"_L1;

  //QgsRasterDataProvider* verifiedProvider = QgsRasterLayer::loadProvider( verifiedKey, verifiedUri );
  const QgsDataProvider::ProviderOptions options;
  QgsRasterDataProvider *verifiedProvider = qobject_cast< QgsRasterDataProvider * >( QgsProviderRegistry::instance()->createProvider( verifiedKey, verifiedUri, options ) );
  if ( !verifiedProvider || !verifiedProvider->isValid() )
  {
    error( u"Cannot load provider %1 with URI: %2"_s.arg( verifiedKey, verifiedUri ), mReport );
    return false;
  }

  //QgsRasterDataProvider* expectedProvider = QgsRasterLayer::loadProvider( expectedKey, expectedUri );
  QgsRasterDataProvider *expectedProvider = qobject_cast< QgsRasterDataProvider * >( QgsProviderRegistry::instance()->createProvider( expectedKey, expectedUri, options ) );
  if ( !expectedProvider || !expectedProvider->isValid() )
  {
    error( u"Cannot load provider %1 with URI: %2"_s.arg( expectedKey, expectedUri ), mReport );
    return false;
  }

  mReport += u"Verified URI: %1<br>"_s.arg( verifiedUri.replace( '&', "&amp;"_L1 ) );
  mReport += u"Expected URI: %1<br>"_s.arg( expectedUri.replace( '&', "&amp;"_L1 ) );

  mReport += "<br>"_L1;
  mReport += u"<table style='%1'>\n"_s.arg( mTabStyle );
  mReport += compareHead();

  compare( u"Band count"_s, verifiedProvider->bandCount(), expectedProvider->bandCount(), mReport, ok );

  compare( u"Width"_s, verifiedProvider->xSize(), expectedProvider->xSize(), mReport, ok );
  compare( u"Height"_s, verifiedProvider->ySize(), expectedProvider->ySize(), mReport, ok );

  compareRow( u"Extent"_s, verifiedProvider->extent().toString(), expectedProvider->extent().toString(), mReport, verifiedProvider->extent() == expectedProvider->extent() );

  if ( verifiedProvider->extent() != expectedProvider->extent() ) ok = false;


  mReport += "</table>\n"_L1;

  if ( !ok ) return false;

  bool allOk = true;
  for ( int band = 1; band <= expectedProvider->bandCount(); band++ )
  {
    mReport += u"<h3>Band %1</h3>\n"_s.arg( band );
    mReport += u"<table style='%1'>\n"_s.arg( mTabStyle );
    mReport += compareHead();

    // Data types may differ (?)
    bool typesOk = true;
    compare( u"Source data type"_s, verifiedProvider->sourceDataType( band ), expectedProvider->sourceDataType( band ), mReport, typesOk );
    compare( u"Data type"_s, verifiedProvider->dataType( band ), expectedProvider->dataType( band ), mReport, typesOk );

    // Check nodata
    bool noDataOk = true;
    compare( u"No data (NULL) value existence flag"_s, verifiedProvider->sourceHasNoDataValue( band ), expectedProvider->sourceHasNoDataValue( band ), mReport, noDataOk );
    if ( verifiedProvider->sourceHasNoDataValue( band ) && expectedProvider->sourceHasNoDataValue( band ) )
    {
      compare( u"No data (NULL) value"_s, verifiedProvider->sourceNoDataValue( band ), expectedProvider->sourceNoDataValue( band ), mReport, noDataOk );
    }

    bool statsOk = true;
    const QgsRasterBandStats verifiedStats = verifiedProvider->bandStatistics( band );
    const QgsRasterBandStats expectedStats = expectedProvider->bandStatistics( band );

    // Min/max may 'slightly' differ, for big numbers however, the difference may
    // be quite big, for example for Float32 with max -3.332e+38, the difference is 1.47338e+24
    double tol = tolerance( expectedStats.minimumValue );
    compare( u"Minimum value"_s, verifiedStats.minimumValue, expectedStats.minimumValue, mReport, statsOk, tol );
    tol = tolerance( expectedStats.maximumValue );
    compare( u"Maximum value"_s, verifiedStats.maximumValue, expectedStats.maximumValue, mReport, statsOk, tol );

    // TODO: enable once fixed (WCS excludes nulls but GDAL does not)
    //compare( "Cells count", verifiedStats.elementCount, expectedStats.elementCount, mReport, statsOk );

    tol = tolerance( expectedStats.mean );
    compare( u"Mean"_s, verifiedStats.mean, expectedStats.mean, mReport, statsOk, tol );

    // stdDev usually differ significantly
    tol = tolerance( expectedStats.stdDev, 1 );
    compare( u"Standard deviation"_s, verifiedStats.stdDev, expectedStats.stdDev, mReport, statsOk, tol );

    mReport += "</table>"_L1;
    mReport += "<br>"_L1;

    if ( !statsOk || !typesOk || !noDataOk )
    {
      allOk = false;
      // create values table anyway so that values are available
    }

    mReport += "<table><tr>"_L1;
    mReport += "<td>Data comparison</td>"_L1;
    mReport += u"<td style='%1 %2 border: 1px solid'>correct&nbsp;value</td>"_s.arg( mCellStyle, mOkStyle );
    mReport += "<td></td>"_L1;
    mReport += u"<td style='%1 %2 border: 1px solid'>wrong&nbsp;value<br>expected value</td></tr>"_s.arg( mCellStyle, mErrStyle );
    mReport += "</tr></table>"_L1;
    mReport += "<br>"_L1;

    const int width = expectedProvider->xSize();
    const int height = expectedProvider->ySize();
    std::unique_ptr< QgsRasterBlock > expectedBlock( expectedProvider->block( band, expectedProvider->extent(), width, height ) );
    std::unique_ptr< QgsRasterBlock > verifiedBlock( verifiedProvider->block( band, expectedProvider->extent(), width, height ) );

    if ( !expectedBlock || !expectedBlock->isValid() ||
         !verifiedBlock || !verifiedBlock->isValid() )
    {
      allOk = false;
      mReport += "cannot read raster block"_L1;
      continue;
    }

    // compare data values
    QString htmlTable = u"<table style='%1'>"_s.arg( mTabStyle );
    for ( int row = 0; row < height; row ++ )
    {
      htmlTable += "<tr>"_L1;
      for ( int col = 0; col < width; col ++ )
      {
        bool cellOk = true;
        const double verifiedVal = verifiedBlock->value( row, col );
        const double expectedVal = expectedBlock->value( row, col );

        QString valStr;
        if ( compare( verifiedVal, expectedVal, 0 ) )
        {
          valStr = QString::number( verifiedVal );
        }
        else
        {
          cellOk = false;
          allOk = false;
          valStr = u"%1<br>%2"_s.arg( verifiedVal ).arg( expectedVal );
        }
        htmlTable += u"<td style='%1 %2'>%3</td>"_s.arg( mCellStyle, cellOk ? mOkStyle : mErrStyle, valStr );
      }
      htmlTable += "</tr>"_L1;
    }
    htmlTable += "</table>"_L1;

    mReport += htmlTable;
  }
  delete verifiedProvider;
  delete expectedProvider;
  return allOk;
}

void QgsRasterChecker::error( const QString &message, QString &report )
{
  report += u"<font style='%1'>Error: "_s.arg( mErrMsgStyle );
  report += message;
  report += "</font>"_L1;
}

double QgsRasterChecker::tolerance( double val, int places )
{
  // float precision is about 7 decimal digits, double about 16
  // default places = 6
  return 1. * std::pow( 10, std::round( std::log10( std::fabs( val ) ) - places ) );
}

QString QgsRasterChecker::compareHead()
{
  QString html;
  html += u"<tr><th style='%1'>Param name</th><th style='%1'>Verified value</th><th style='%1'>Expected value</th><th style='%1'>Difference</th><th style='%1'>Tolerance</th></tr>"_s.arg( mCellStyle );
  return html;
}

void QgsRasterChecker::compare( const QString &paramName, int verifiedVal, int expectedVal, QString &report, bool &ok )
{
  const bool isEqual = verifiedVal == expectedVal;
  compareRow( paramName, QString::number( verifiedVal ), QString::number( expectedVal ), report, isEqual, QString::number( verifiedVal - expectedVal ) );
  if ( !isEqual )
    ok = false;
}

void QgsRasterChecker::compare( const QString &paramName, Qgis::DataType verifiedVal, Qgis::DataType expectedVal, QString &report, bool &ok )
{
  const bool isEqual = verifiedVal == expectedVal;
  compareRow( paramName, QString::number( static_cast< int>( verifiedVal ) ), QString::number( static_cast< int >( expectedVal ) ), report, isEqual, QString::number( static_cast< int >( verifiedVal ) - static_cast< int>( expectedVal ) ) );
  if ( !isEqual )
    ok = false;
}

bool QgsRasterChecker::compare( double verifiedVal, double expectedVal, double tolerance )
{
  // values may be nan
  return ( std::isnan( verifiedVal ) && std::isnan( expectedVal ) ) || ( std::fabs( verifiedVal - expectedVal ) <= tolerance );
}

void QgsRasterChecker::compare( const QString &paramName, double verifiedVal, double expectedVal, QString &report, bool &ok, double tolerance )
{
  const bool isNearEqual = compare( verifiedVal, expectedVal, tolerance );
  compareRow( paramName, QString::number( verifiedVal ), QString::number( expectedVal ), report, isNearEqual, QString::number( verifiedVal - expectedVal ), QString::number( tolerance ) );
  if ( !isNearEqual )
    ok = false;
}

void QgsRasterChecker::compareRow( const QString &paramName, const QString &verifiedVal, const QString &expectedVal, QString &report, bool ok, const QString &difference, const QString &tolerance )
{
  report += "<tr>\n"_L1;
  report += u"<td style='%1'>%2</td><td style='%1 %3'>%4</td><td style='%1'>%5</td>\n"_s.arg( mCellStyle, paramName, ok ? mOkStyle : mErrStyle, verifiedVal, expectedVal );
  report += u"<td style='%1'>%2</td>\n"_s.arg( mCellStyle, difference );
  report += u"<td style='%1'>%2</td>\n"_s.arg( mCellStyle, tolerance );
  report += "</tr>"_L1;
}
