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
#include <string.h>

#ifndef Q_OS_MACX
#include <cmath>
#else
#include <math.h>
#endif

QgsRasterMatrix::QgsRasterMatrix(): mColumns( 0 ), mRows( 0 ), mData( 0 )
{
}

QgsRasterMatrix::QgsRasterMatrix( int nCols, int nRows, float* data ): mColumns( nCols ), mRows( nRows ), mData( data )
{
}

QgsRasterMatrix::QgsRasterMatrix( const QgsRasterMatrix& m ): mColumns( 0 ), mRows( 0 ), mData( 0 )
{
  operator=( m );
}

QgsRasterMatrix::~QgsRasterMatrix()
{
  delete[] mData;
}

QgsRasterMatrix& QgsRasterMatrix::operator=( const QgsRasterMatrix& m )
{
  delete[] mData;
  mColumns = m.nColumns();
  mRows = m.nRows();
  int nEntries = mColumns * mRows;
  mData = new float[nEntries];
  memcpy( mData, m.mData, sizeof( float ) * nEntries );
  return *this;
}

void QgsRasterMatrix::setData( int cols, int rows, float* data )
{
  delete[] mData;
  mColumns = cols;
  mRows = rows;
  mData = data;
}

float* QgsRasterMatrix::takeData()
{
  float* data = mData;
  mData = 0; mColumns = 0; mRows = 0;
  return data;
}

bool QgsRasterMatrix::add( const QgsRasterMatrix& other )
{
  return twoArgumentOperation( opPLUS, other );
}

bool QgsRasterMatrix::subtract( const QgsRasterMatrix& other )
{
  return twoArgumentOperation( opMINUS, other );
}

bool QgsRasterMatrix::multiply( const QgsRasterMatrix& other )
{
  return twoArgumentOperation( opMUL, other );
}

bool QgsRasterMatrix::divide( const QgsRasterMatrix& other )
{
  return twoArgumentOperation( opDIV, other );
}

bool QgsRasterMatrix::power( const QgsRasterMatrix& other )
{
  return twoArgumentOperation( opPOW, other );
}

bool QgsRasterMatrix::squareRoot()
{
  if( !mData )
  {
    return false;
  }

  int nEntries = mColumns * mRows;
  for( int i = 0; i < nEntries; ++i )
  {
    double value = mData[i];
    if( value >= 0 )
    {
      mData[i] = sqrt( value );
    }
    else
    {
      mData[i] = -10000; //no complex numbers
    }
  }
  return true;
}

bool QgsRasterMatrix::sinus()
{
  if( !mData )
  {
    return false;
  }

  int nEntries = mColumns * mRows;
  for( int i = 0; i < nEntries; ++i )
  {
    mData[i] = sin( mData[i] );
  }
  return true;
}

bool QgsRasterMatrix::asinus()
{
  if( !mData )
  {
    return false;
  }

  int nEntries = mColumns * mRows;
  for( int i = 0; i < nEntries; ++i )
  {
    mData[i] = asin( mData[i] );
  }
  return true;
}

bool QgsRasterMatrix::cosinus()
{
  if( !mData )
  {
    return false;
  }

  int nEntries = mColumns * mRows;
  for( int i = 0; i < nEntries; ++i )
  {
    mData[i] = cos( mData[i] );
  }
  return true;
}

bool QgsRasterMatrix::acosinus()
{
  if( !mData )
  {
    return false;
  }

  int nEntries = mColumns * mRows;
  for( int i = 0; i < nEntries; ++i )
  {
    mData[i] = acos( mData[i] );
  }
  return true;
}

bool QgsRasterMatrix::tangens()
{
  if( !mData )
  {
    return false;
  }

  int nEntries = mColumns * mRows;
  for( int i = 0; i < nEntries; ++i )
  {
    mData[i] = tan( mData[i] );
  }
  return true;
}

bool QgsRasterMatrix::atangens()
{
  if( !mData )
  {
    return false;
  }

  int nEntries = mColumns * mRows;
  for( int i = 0; i < nEntries; ++i )
  {
    mData[i] = atan( mData[i] );
  }
  return true;
}

bool QgsRasterMatrix::twoArgumentOperation( TwoArgOperator op, const QgsRasterMatrix& other )
{
  if( isNumber() && other.isNumber() )
  {
    switch( op )
    {
      case opPLUS:
        mData[0] = number() + other.number();
        break;
      case opMINUS:
        mData[0] = number() - other.number();
        break;
      case opMUL:
        mData[0] = number() * other.number();
        break;
      case opDIV:
        if( other.number() == 0 )
        {
          mData[0] = -10000;
        }
        else
        {
          mData[0] = number() / other.number();
        }
        break;
      case opPOW:
        if( !testPowerValidity( mData[0], other.number() ) )
        {
          mData[0] = -10000;
        }
        else
        {
          mData[0] = pow( mData[0], other.number() );
        }
        break;
    }
    return true;
  }
  //two matrices
  if( !isNumber() && !other.isNumber() )
  {
    float* matrix = other.mData;
    int nEntries = mColumns * mRows;
    switch( op )
    {
      case opPLUS:
        for( int i = 0; i < nEntries; ++i )
        {
          mData[i] = mData[i] + matrix[i];
        }
        break;
      case opMINUS:
        for( int i = 0; i < nEntries; ++i )
        {
          mData[i] = mData[i] - matrix[i];
        }
        break;
      case opMUL:
        for( int i = 0; i < nEntries; ++i )
        {
          mData[i] = mData[i] * matrix[i];
        }
        break;
      case opDIV:
        for( int i = 0; i < nEntries; ++i )
        {
          if( matrix[i] == 0 )
          {
            mData[i] = -10000;
          }
          else
          {
            mData[i] = mData[i] / matrix[i];
          }
        }
        break;
      case opPOW:
        for( int i = 0; i < nEntries; ++i )
        {
          if( !testPowerValidity( mData[i], matrix[i] ) )
          {
            mData[i] = -10000;
          }
          else
          {
            mData[i] = pow( mData[i], matrix[i] );
          }
        }
        break;
    }
    return true;
  }
  double value = 0;
  if( isNumber() )
  {
    float* matrix = other.mData;
    int nEntries = other.nColumns() * other.nRows();
    value = mData[0];
    delete[] mData;
    mData = new float[nEntries]; mColumns = other.nColumns(); mRows = other.nRows();

    switch( op )
    {
      case opPLUS:
        for( int i = 0; i < nEntries; ++i )
        {
          mData[i] = value + matrix[i];
        }
        break;
      case opMINUS:
        for( int i = 0; i < nEntries; ++i )
        {
          mData[i] = value - matrix[i];
        }
        break;
      case opMUL:
        for( int i = 0; i < nEntries; ++i )
        {
          mData[i] = value * matrix[i];
        }
        break;
      case opDIV:
        for( int i = 0; i < nEntries; ++i )
        {
          if( matrix[i] == 0 )
          {
            mData[i] = -10000;
          }
          else
          {
            mData[i] = value / matrix[i];
          }
        }
        break;
      case opPOW:
        for( int i = 0; i < nEntries; ++i )
        {
          if( !testPowerValidity( value, matrix[i] ) )
          {
            mData[i] = -10000;
          }
          else
          {
            mData[i] = pow( value, matrix[i] );
          }
        }
        break;
    }
  }
  else
  {
    value = other.number();
    int nEntries = mColumns * mRows;
    switch( op )
    {
      case opPLUS:
        for( int i = 0; i < nEntries; ++i )
        {
          mData[i] = mData[i] + value;
        }
        break;
      case opMINUS:
        for( int i = 0; i < nEntries; ++i )
        {
          mData[i] = mData[i] - value;
        }
        break;
      case opMUL:
        for( int i = 0; i < nEntries; ++i )
        {
          mData[i] = mData[i] * value;
        }
        break;
      case opDIV:
        if( value == 0 )
        {
          for( int i = 0; i < nEntries; ++i )
          {
            mData[i] = -10000;
          }
        }
        else
        {
          for( int i = 0; i < nEntries; ++i )
          {
            mData[i] = mData[i] / value;
          }
        }
        break;
      case opPOW:
        for( int i = 0; i < nEntries; ++i )
        {
          if( !testPowerValidity( mData[i], value ) )
          {
            mData[i] = -10000;
          }
          else
          {
            mData[i] = pow( mData[i], value );
          }
        }
        break;
    }
  }
  return true;
}

bool QgsRasterMatrix::testPowerValidity( double base, double power )
{
  if(( base == 0 && power < 0 ) || ( power < 0 && ( power - floor( power ) ) > 0 ) )
  {
    return false;
  }
  return true;
}
