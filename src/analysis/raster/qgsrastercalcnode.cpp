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
#include <cfloat>

QgsRasterCalcNode::QgsRasterCalcNode()
    : mType( tNumber )
    , mLeft( 0 )
    , mRight( 0 )
    , mNumber( 0 )
    , mOperator( opNONE )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( double number )
    : mType( tNumber )
    , mLeft( 0 )
    , mRight( 0 )
    , mNumber( number )
    , mOperator( opNONE )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( Operator op, QgsRasterCalcNode* left, QgsRasterCalcNode* right )
    : mType( tOperator )
    , mLeft( left )
    , mRight( right )
    , mNumber( 0 )
    , mOperator( op )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( const QString& rasterName )
    : mType( tRasterRef )
    , mLeft( 0 )
    , mRight( 0 )
    , mNumber( 0 )
    , mRasterName( rasterName )
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
  //if type is raster ref: return a copy of the corresponding matrix

  //if type is operator, call the proper matrix operations
  if ( mType == tRasterRef )
  {
    QMap<QString, QgsRasterMatrix*>::iterator it = rasterData.find( mRasterName );
    if ( it == rasterData.end() )
    {
      return false;
    }

    int nEntries = ( *it )->nColumns() * ( *it )->nRows();
    float* data = new float[nEntries];
    memcpy( data, ( *it )->data(), nEntries * sizeof( float ) );
    result.setData(( *it )->nColumns(), ( *it )->nRows(), data, ( *it )->nodataValue() );
    return true;
  }
  else if ( mType == tOperator )
  {
    QgsRasterMatrix leftMatrix, rightMatrix;
    QgsRasterMatrix resultMatrix;
    if ( !mLeft || !mLeft->calculate( rasterData, leftMatrix ) )
    {
      return false;
    }
    if ( mRight && !mRight->calculate( rasterData, rightMatrix ) )
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
    float* data = new float[1];
    data[0] = mNumber;
    result.setData( 1, 1, data, -FLT_MAX );
    return true;
  }
  return false;
}

QgsRasterCalcNode* QgsRasterCalcNode::parseRasterCalcString( const QString& str, QString& parserErrorMsg )
{
  extern QgsRasterCalcNode* localParseRasterCalcString( const QString & str, QString & parserErrorMsg );
  return localParseRasterCalcString( str, parserErrorMsg );
}

