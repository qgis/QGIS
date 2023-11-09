/***************************************************************************
                          qgsrastermatrix.cpp
                          -------------------
    begin                : 2010-10-23
    copyright            : (C) 20010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastermatrix.h"
#include <cstring>
#include <cmath>
#include <algorithm>

QgsRasterMatrix::QgsRasterMatrix( int nCols, int nRows, double *data, double nodataValue )
  : mColumns( nCols )
  , mRows( nRows )
  , mData( data )
  , mNodataValue( nodataValue )
{
}

QgsRasterMatrix::QgsRasterMatrix( const QgsRasterMatrix &m )
{
  operator=( m );
}

QgsRasterMatrix::~QgsRasterMatrix()
{
  delete[] mData;
}

QgsRasterMatrix &QgsRasterMatrix::operator=( const QgsRasterMatrix &m )
{
  if ( this != &m )
  {
    delete[] mData;
    mColumns = m.nColumns();
    mRows = m.nRows();
    const int nEntries = mColumns * mRows;
    mData = new double[nEntries];
    memcpy( mData, m.mData, sizeof( double ) * nEntries );
    mNodataValue = m.nodataValue();
  }
  return *this;
}

void QgsRasterMatrix::setData( int cols, int rows, double *data, double nodataValue )
{
  delete[] mData;
  mColumns = cols;
  mRows = rows;
  mData = data;
  mNodataValue = nodataValue;
}

double *QgsRasterMatrix::takeData()
{
  double *data = mData;
  mData = nullptr;
  mColumns = 0;
  mRows = 0;
  return data;
}

bool QgsRasterMatrix::add( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opPLUS, other );
}

bool QgsRasterMatrix::subtract( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opMINUS, other );
}

bool QgsRasterMatrix::multiply( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opMUL, other );
}

bool QgsRasterMatrix::divide( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opDIV, other );
}

bool QgsRasterMatrix::power( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opPOW, other );
}

bool QgsRasterMatrix::equal( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opEQ, other );
}

bool QgsRasterMatrix::notEqual( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opNE, other );
}

bool QgsRasterMatrix::greaterThan( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opGT, other );
}

bool QgsRasterMatrix::lesserThan( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opLT, other );
}

bool QgsRasterMatrix::greaterEqual( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opGE, other );
}

bool QgsRasterMatrix::lesserEqual( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opLE, other );
}

bool QgsRasterMatrix::logicalAnd( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opAND, other );
}

bool QgsRasterMatrix::logicalOr( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opOR, other );
}

bool QgsRasterMatrix::max( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opMAX, other );
}

bool QgsRasterMatrix::min( const QgsRasterMatrix &other )
{
  return twoArgumentOperation( opMIN, other );
}

bool QgsRasterMatrix::squareRoot()
{
  return oneArgumentOperation( opSQRT );
}

bool QgsRasterMatrix::sinus()
{
  return oneArgumentOperation( opSIN );
}

bool QgsRasterMatrix::asinus()
{
  return oneArgumentOperation( opASIN );
}

bool QgsRasterMatrix::cosinus()
{
  return oneArgumentOperation( opCOS );
}

bool QgsRasterMatrix::acosinus()
{
  return oneArgumentOperation( opACOS );
}

bool QgsRasterMatrix::tangens()
{
  return oneArgumentOperation( opTAN );
}

bool QgsRasterMatrix::atangens()
{
  return oneArgumentOperation( opATAN );
}

bool QgsRasterMatrix::changeSign()
{
  return oneArgumentOperation( opSIGN );
}

bool QgsRasterMatrix::log()
{
  return oneArgumentOperation( opLOG );
}

bool QgsRasterMatrix::log10()
{
  return oneArgumentOperation( opLOG10 );
}

bool QgsRasterMatrix::absoluteValue()
{
  return oneArgumentOperation( opABS );
}

bool QgsRasterMatrix::oneArgumentOperation( OneArgOperator op )
{
  if ( !mData )
  {
    return false;
  }

  const int nEntries = mColumns * mRows;
  double value;
  for ( int i = 0; i < nEntries; ++i )
  {
    value = mData[i];
    if ( value != mNodataValue )
    {
      switch ( op )
      {
        case opSQRT:
          if ( value < 0 ) //no complex numbers
          {
            mData[i] = mNodataValue;
          }
          else
          {
            mData[i] = std::sqrt( value );
          }
          break;
        case opSIN:
          mData[i] = std::sin( value );
          break;
        case opCOS:
          mData[i] = std::cos( value );
          break;
        case opTAN:
          mData[i] = std::tan( value );
          break;
        case opASIN:
          mData[i] = std::asin( value );
          break;
        case opACOS:
          mData[i] = std::acos( value );
          break;
        case opATAN:
          mData[i] = std::atan( value );
          break;
        case opSIGN:
          mData[i] = -value;
          break;
        case opLOG:
          if ( value <= 0 )
          {
            mData[i] = mNodataValue;
          }
          else
          {
            mData[i] = ::log( value );
          }
          break;
        case opLOG10:
          if ( value <= 0 )
          {
            mData[i] = mNodataValue;
          }
          else
          {
            mData[i] = ::log10( value );
          }
          break;
        case opABS:
          mData[i] = ::fabs( value );
          break;
      }
    }
  }
  return true;
}

double QgsRasterMatrix::calculateTwoArgumentOp( TwoArgOperator op, double arg1, double arg2 ) const
{
  switch ( op )
  {
    case opPLUS:
      return arg1 + arg2;
    case opMINUS:
      return arg1 - arg2;
    case opMUL:
      return arg1 * arg2;
    case opDIV:
      if ( arg2 == 0 )
      {
        return mNodataValue;
      }
      else
      {
        return arg1 / arg2;
      }
    case opPOW:
      if ( !testPowerValidity( arg1, arg2 ) )
      {
        return mNodataValue;
      }
      else
      {
        return std::pow( arg1, arg2 );
      }
    case opEQ:
      return ( arg1 == arg2 ? 1.0 : 0.0 );
    case opNE:
      return ( arg1 == arg2 ? 0.0 : 1.0 );
    case opGT:
      return ( arg1 > arg2 ? 1.0 : 0.0 );
    case opLT:
      return ( arg1 < arg2 ? 1.0 : 0.0 );
    case opGE:
      return ( arg1 >= arg2 ? 1.0 : 0.0 );
    case opLE:
      return ( arg1 <= arg2 ? 1.0 : 0.0 );
    case opAND:
      return ( arg1 && arg2 ? 1.0 : 0.0 );
    case opOR:
      return ( arg1 || arg2 ? 1.0 : 0.0 );
    case opMAX:
      return std::max( arg1, arg2 );
    case opMIN:
      return std::min( arg1, arg2 );
  }
  return mNodataValue;
}

bool QgsRasterMatrix::twoArgumentOperation( TwoArgOperator op, const QgsRasterMatrix &other )
{
  if ( isNumber() && other.isNumber() ) //operation on two 1x1 matrices
  {
    //operations with nodata values always generate nodata
    if ( mData[0] == mNodataValue || other.number() == other.nodataValue() )
    {
      mData[0] = mNodataValue;
    }
    else
    {
      mData[0] = calculateTwoArgumentOp( op, mData[0], other.number() );
    }
    return true;
  }

  //two matrices
  if ( !isNumber() && !other.isNumber() )
  {
    double *matrix = other.mData;
    const int nEntries = mColumns * mRows;
    double value1, value2;

    for ( int i = 0; i < nEntries; ++i )
    {
      value1 = mData[i];
      value2 = matrix[i];
      if ( value1 == mNodataValue || value2 == other.mNodataValue )
      {
        mData[i] = mNodataValue;
      }
      else
      {
        mData[i] = calculateTwoArgumentOp( op, value1, value2 );
      }
    }
    return true;
  }

  //this matrix is a single number and the other one a real matrix
  if ( isNumber() )
  {
    double *matrix = other.mData;
    const int nEntries = other.nColumns() * other.nRows();
    const double value = mData[0];
    delete[] mData;
    mData = new double[nEntries];
    mColumns = other.nColumns();
    mRows = other.nRows();
    mNodataValue = other.nodataValue();

    if ( value == mNodataValue )
    {
      for ( int i = 0; i < nEntries; ++i )
      {
        mData[i] = mNodataValue;
      }
      return true;
    }

    for ( int i = 0; i < nEntries; ++i )
    {
      if ( matrix[i] == other.mNodataValue )
      {
        mData[i] = mNodataValue;
        continue;
      }

      mData[i] = calculateTwoArgumentOp( op, value, matrix[i] );
    }
    return true;
  }
  else //this matrix is a real matrix and the other a number
  {
    const double value = other.number();
    const int nEntries = mColumns * mRows;

    if ( other.number() == other.mNodataValue )
    {
      for ( int i = 0; i < nEntries; ++i )
      {
        mData[i] = mNodataValue;
      }
      return true;
    }

    for ( int i = 0; i < nEntries; ++i )
    {
      if ( mData[i] == mNodataValue )
      {
        continue;
      }

      mData[i] = calculateTwoArgumentOp( op, mData[i], value );
    }
    return true;
  }
}

bool QgsRasterMatrix::testPowerValidity( double base, double power ) const
{
  return !( ( base == 0 && power < 0 ) || ( base < 0 && ( power - std::floor( power ) ) > 0 ) );
}
