/***************************************************************************
    qgsrastercalcnode.cpp
    ---------------------
    begin                : October 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrastercalcnode.h"
#include "qgsrasterblock.h"
#include "qgsrastermatrix.h"
#include <cfloat>

QgsRasterCalcNode::QgsRasterCalcNode( double number )
  : mNumber( number )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( QgsRasterMatrix *matrix )
  : mType( tMatrix )
  , mMatrix( matrix )
{

}

QgsRasterCalcNode::QgsRasterCalcNode( Operator op, QgsRasterCalcNode *left, QgsRasterCalcNode *right )
  : mType( tOperator )
  , mLeft( left )
  , mRight( right )
  , mOperator( op )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( const QString &rasterName )
  : mType( tRasterRef )
  , mRasterName( rasterName )
{
  if ( mRasterName.startsWith( '"' ) && mRasterName.endsWith( '"' ) )
    mRasterName = mRasterName.mid( 1, mRasterName.size() - 2 );
}

QgsRasterCalcNode::~QgsRasterCalcNode()
{
  delete mLeft;
  delete mRight;
}

bool QgsRasterCalcNode::calculate( QMap<QString, QgsRasterBlock * > &rasterData, QgsRasterMatrix &result, int row ) const
{
  //if type is raster ref: return a copy of the corresponding matrix

  //if type is operator, call the proper matrix operations
  if ( mType == tRasterRef )
  {
    QMap<QString, QgsRasterBlock *>::iterator it = rasterData.find( mRasterName );
    if ( it == rasterData.end() )
    {
      return false;
    }

    int nRows = ( row >= 0 ? 1 : ( *it )->height() );
    int startRow = ( row >= 0 ? row : 0 );
    int endRow = startRow + nRows;
    int nCols = ( *it )->width();
    int nEntries = nCols * nRows;
    double *data = new double[nEntries];

    //convert input raster values to double, also convert input no data to result no data

    int outRow = 0;
    for ( int dataRow = startRow; dataRow < endRow ; ++dataRow, ++outRow )
    {
      for ( int dataCol = 0; dataCol < nCols; ++dataCol )
      {
        data[ dataCol + nCols * outRow] = ( *it )->isNoData( dataRow, dataCol ) ? result.nodataValue() : ( *it )->value( dataRow, dataCol );
      }
    }
    result.setData( nCols, nRows, data, result.nodataValue() );
    return true;
  }
  else if ( mType == tOperator )
  {
    QgsRasterMatrix leftMatrix, rightMatrix;
    leftMatrix.setNodataValue( result.nodataValue() );
    rightMatrix.setNodataValue( result.nodataValue() );

    if ( !mLeft || !mLeft->calculate( rasterData, leftMatrix, row ) )
    {
      return false;
    }
    if ( mRight && !mRight->calculate( rasterData, rightMatrix, row ) )
    {
      return false;
    }

    switch ( mOperator )
    {
      case opPLUS:
        leftMatrix.add( rightMatrix );
        break;
      case opMINUS:
        leftMatrix.subtract( rightMatrix );
        break;
      case opMUL:
        leftMatrix.multiply( rightMatrix );
        break;
      case opDIV:
        leftMatrix.divide( rightMatrix );
        break;
      case opPOW:
        leftMatrix.power( rightMatrix );
        break;
      case opEQ:
        leftMatrix.equal( rightMatrix );
        break;
      case opNE:
        leftMatrix.notEqual( rightMatrix );
        break;
      case opGT:
        leftMatrix.greaterThan( rightMatrix );
        break;
      case opLT:
        leftMatrix.lesserThan( rightMatrix );
        break;
      case opGE:
        leftMatrix.greaterEqual( rightMatrix );
        break;
      case opLE:
        leftMatrix.lesserEqual( rightMatrix );
        break;
      case opAND:
        leftMatrix.logicalAnd( rightMatrix );
        break;
      case opOR:
        leftMatrix.logicalOr( rightMatrix );
        break;
      case opSQRT:
        leftMatrix.squareRoot();
        break;
      case opSIN:
        leftMatrix.sinus();
        break;
      case opCOS:
        leftMatrix.cosinus();
        break;
      case opTAN:
        leftMatrix.tangens();
        break;
      case opASIN:
        leftMatrix.asinus();
        break;
      case opACOS:
        leftMatrix.acosinus();
        break;
      case opATAN:
        leftMatrix.atangens();
        break;
      case opSIGN:
        leftMatrix.changeSign();
        break;
      case opLOG:
        leftMatrix.log();
        break;
      case opLOG10:
        leftMatrix.log10();
        break;
      default:
        return false;
    }
    int newNColumns = leftMatrix.nColumns();
    int newNRows = leftMatrix.nRows();
    result.setData( newNColumns, newNRows, leftMatrix.takeData(), leftMatrix.nodataValue() );
    return true;
  }
  else if ( mType == tNumber )
  {
    double *data = new double[1];
    data[0] = mNumber;
    result.setData( 1, 1, data, result.nodataValue() );
    return true;
  }
  else if ( mType == tMatrix )
  {
    int nEntries = mMatrix->nColumns() * mMatrix->nRows();
    double *data = new double[nEntries];
    for ( int i = 0; i < nEntries; ++i )
    {
      data[i] = mMatrix->data()[i] == mMatrix->nodataValue() ? result.nodataValue() : mMatrix->data()[i];
    }
    result.setData( mMatrix->nColumns(), mMatrix->nRows(), data, result.nodataValue() );
    return true;
  }
  return false;
}

QgsRasterCalcNode *QgsRasterCalcNode::parseRasterCalcString( const QString &str, QString &parserErrorMsg )
{
  extern QgsRasterCalcNode *localParseRasterCalcString( const QString & str, QString & parserErrorMsg );
  return localParseRasterCalcString( str, parserErrorMsg );
}

