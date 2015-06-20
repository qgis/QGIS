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
#include <cfloat>

QgsRasterCalcNode::QgsRasterCalcNode()
    : mType( tNumber )
    , mLeft( 0 )
    , mRight( 0 )
    , mNumber( 0 )
    , mMatrix( 0 )
    , mOperator( opNONE )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( double number )
    : mType( tNumber )
    , mLeft( 0 )
    , mRight( 0 )
    , mNumber( number )
    , mMatrix( 0 )
    , mOperator( opNONE )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( QgsRasterMatrix* matrix )
    : mType( tMatrix )
    , mLeft( 0 )
    , mRight( 0 )
    , mNumber( 0 )
    , mMatrix( matrix )
    , mOperator( opNONE )
{

}

QgsRasterCalcNode::QgsRasterCalcNode( Operator op, QgsRasterCalcNode* left, QgsRasterCalcNode* right )
    : mType( tOperator )
    , mLeft( left )
    , mRight( right )
    , mNumber( 0 )
    , mMatrix( 0 )
    , mOperator( op )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( const QString& rasterName )
    : mType( tRasterRef )
    , mLeft( 0 )
    , mRight( 0 )
    , mNumber( 0 )
    , mRasterName( rasterName )
    , mMatrix( 0 )
    , mOperator( opNONE )
{
  if ( mRasterName.startsWith( '"' ) && mRasterName.endsWith( '"' ) )
    mRasterName = mRasterName.mid( 1, mRasterName.size() - 2 );
}

QgsRasterCalcNode::~QgsRasterCalcNode()
{
  if ( mLeft )
  {
    delete mLeft;
  }
  if ( mRight )
  {
    delete mRight;
  }
}

bool QgsRasterCalcNode::calculate( QMap<QString, QgsRasterMatrix*>& rasterData, QgsRasterMatrix& result ) const
{
  //deprecated method
  //convert QgsRasterMatrix to QgsRasterBlock and call replacement method
  QMap<QString, QgsRasterBlock* > rasterBlockData;
  QMap<QString, QgsRasterMatrix*>::const_iterator it = rasterData.constBegin();
  for ( ; it != rasterData.constEnd(); ++it )
  {
    QgsRasterBlock* block = new QgsRasterBlock( QGis::Float32, it.value()->nColumns(), it.value()->nRows(), it.value()->nodataValue() );
    for ( int row = 0; row < it.value()->nRows(); ++row )
    {
      for ( int col = 0; col < it.value()->nColumns(); ++col )
      {
        block->setValue( row, col, it.value()->data()[ row * it.value()->nColumns() + col ] );
      }
    }
    rasterBlockData.insert( it.key(), block );
  }

  return calculate( rasterBlockData, result );
}

bool QgsRasterCalcNode::calculate( QMap<QString, QgsRasterBlock* >& rasterData, QgsRasterMatrix& result, int row ) const
{
  //if type is raster ref: return a copy of the corresponding matrix

  //if type is operator, call the proper matrix operations
  if ( mType == tRasterRef )
  {
    QMap<QString, QgsRasterBlock*>::iterator it = rasterData.find( mRasterName );
    if ( it == rasterData.end() )
    {
      return false;
    }

    int nRows = ( row >= 0 ? 1 : ( *it )->height() );
    int startRow = ( row >= 0 ? row : 0 );
    int endRow = startRow + nRows;
    int nCols = ( *it )->width();
    int nEntries = nCols * nRows;
    double* data = new double[nEntries];

    //convert input raster values to double, also convert input no data to result no data

    int outRow = 0;
    for ( int dataRow = startRow; dataRow < endRow ; ++dataRow, ++outRow )
    {
      for ( int dataCol = 0; dataCol < nCols; ++dataCol )
      {
        data[ dataCol + nCols * outRow] = ( *it )->isNoData( dataRow , dataCol ) ? result.nodataValue() : ( *it )->value( dataRow, dataCol );
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
    double* data = new double[1];
    data[0] = mNumber;
    result.setData( 1, 1, data, result.nodataValue() );
    return true;
  }
  else if ( mType == tMatrix )
  {
    int nEntries = mMatrix->nColumns() * mMatrix->nRows();
    double* data = new double[nEntries];
    for ( int i = 0; i < nEntries; ++i )
    {
      data[i] = mMatrix->data()[i] == mMatrix->nodataValue() ? result.nodataValue() : mMatrix->data()[i];
    }
    result.setData( mMatrix->nColumns(), mMatrix->nRows(), data, result.nodataValue() );
    return true;
  }
  return false;
}

QgsRasterCalcNode* QgsRasterCalcNode::parseRasterCalcString( const QString& str, QString& parserErrorMsg )
{
  extern QgsRasterCalcNode* localParseRasterCalcString( const QString & str, QString & parserErrorMsg );
  return localParseRasterCalcString( str, parserErrorMsg );
}

