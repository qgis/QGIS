/* **************************************************************************
                qgscolorrampshader.cpp -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

This class is based off of code that was originally written by Marco Hugentobler and
originally part of the larger QgsRasterLayer class
****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define DOUBLE_DIFF_THRESHOLD 0.0000001

#include "qgslogger.h"

#include "qgscolorrampshader.h"

#include <cmath>

QgsColorRampShader::QgsColorRampShader( double theMinimumValue, double theMaximumValue ) : QgsRasterShaderFunction( theMinimumValue, theMaximumValue )
{
  QgsDebugMsg( "called." );
  mMaximumColorCacheSize = 1024; //good starting value
  mCurrentColorRampItemIndex = 0;
}

QString QgsColorRampShader::colorRampTypeAsQString()
{
  switch ( mColorRampType )
  {
    case INTERPOLATED:
      return QString( "INTERPOLATED" );
      break;
    case DISCRETE:
      return QString( "DISCRETE" );
      break;
    case EXACT:
      return QString( "EXACT" );
      break;
  }
  return QString( "Unknown" );
}

bool QgsColorRampShader::discreteColor( double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue )
{
  int myColorRampItemCount = mColorRampItemList.count();
  if ( myColorRampItemCount <= 0 )
  {
    return false;
  }

  double myTinyDiff = 0.0;
  QgsColorRampShader::ColorRampItem myColorRampItem;
  while ( mCurrentColorRampItemIndex >= 0 && mCurrentColorRampItemIndex < myColorRampItemCount )
  {
    //Start searching from the last index - assumtion is that neighboring pixels tend to be similar values
    myColorRampItem = mColorRampItemList.value( mCurrentColorRampItemIndex );
    myTinyDiff = qAbs( theValue - myColorRampItem.value );
    //If the previous entry is less, then search closer to the top of the list (assumes mColorRampItemList is sorted)
    if ( mCurrentColorRampItemIndex != 0 && theValue <= mColorRampItemList.at( mCurrentColorRampItemIndex - 1 ).value )
    {
      mCurrentColorRampItemIndex--;
    }
    else if ( theValue <= myColorRampItem.value || myTinyDiff <= DOUBLE_DIFF_THRESHOLD )
    {
      *theReturnRedValue = myColorRampItem.color.red();
      *theReturnGreenValue = myColorRampItem.color.green();
      *theReturnBlueValue = myColorRampItem.color.blue();
      //Cache the shaded value
      if ( mMaximumColorCacheSize >= mColorCache.size() )
      {
        mColorCache.insert( theValue, myColorRampItem.color );
      }
      return true;
    }
    //Search deeper into the color ramp list
    else
    {
      mCurrentColorRampItemIndex++;
    }
  }

  return false; // value not found
}

bool QgsColorRampShader::exactColor( double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue )
{
  int myColorRampItemCount = mColorRampItemList.count();
  if ( myColorRampItemCount <= 0 )
  {
    return false;
  }

  double myTinyDiff = 0.0;
  QgsColorRampShader::ColorRampItem myColorRampItem;
  while ( mCurrentColorRampItemIndex >= 0 && mCurrentColorRampItemIndex < myColorRampItemCount )
  {
    //Start searching from the last index - assumtion is that neighboring pixels tend to be similar values
    myColorRampItem = mColorRampItemList.value( mCurrentColorRampItemIndex );
    myTinyDiff = qAbs( theValue - myColorRampItem.value );
    if ( theValue == myColorRampItem.value || myTinyDiff <= DOUBLE_DIFF_THRESHOLD )
    {
      *theReturnRedValue = myColorRampItem.color.red();
      *theReturnGreenValue = myColorRampItem.color.green();
      *theReturnBlueValue = myColorRampItem.color.blue();
      //Cache the shaded value
      if ( mMaximumColorCacheSize >= mColorCache.size() )
      {
        mColorCache.insert( theValue, myColorRampItem.color );
      }
      return true;
    }
    //pixel value sits between ramp entries so bail
    else if ( mCurrentColorRampItemIndex != myColorRampItemCount - 1 && theValue > myColorRampItem.value && theValue < mColorRampItemList.at( mCurrentColorRampItemIndex + 1 ).value )
    {
      return false;
    }
    //Search deeper into the color ramp list
    else if ( theValue > myColorRampItem.value )
    {
      mCurrentColorRampItemIndex++;
    }
    //Search back toward the beginning of the list
    else
    {
      mCurrentColorRampItemIndex--;
    }
  }

  return false; // value not found
}

bool QgsColorRampShader::interpolatedColor( double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue )
{
  int myColorRampItemCount = mColorRampItemList.count();
  if ( myColorRampItemCount <= 0 )
  {
    return false;
  }

  double myTinyDiff = 0.0;
  double myCurrentRampRange; //difference between two consecutive entry values
  double myOffsetInRange; //difference between the previous entry value and value
  QgsColorRampShader::ColorRampItem myColorRampItem;
  while ( mCurrentColorRampItemIndex >= 0 && mCurrentColorRampItemIndex < myColorRampItemCount )
  {
    //Start searching from the last index - assumtion is that neighboring pixels tend to be similar values
    myColorRampItem = mColorRampItemList.value( mCurrentColorRampItemIndex );
    myTinyDiff = qAbs( theValue - myColorRampItem.value );
    //If the previous entry is less, then search closer to the top of the list (assumes mColorRampItemList is sorted)
    if ( mCurrentColorRampItemIndex != 0 && theValue <= mColorRampItemList.at( mCurrentColorRampItemIndex - 1 ).value )
    {
      mCurrentColorRampItemIndex--;
    }
    else if ( mCurrentColorRampItemIndex != 0 && ( theValue <= myColorRampItem.value || myTinyDiff <= DOUBLE_DIFF_THRESHOLD ) )
    {
      QgsColorRampShader::ColorRampItem myPreviousColorRampItem = mColorRampItemList.value( mCurrentColorRampItemIndex - 1 );
      myCurrentRampRange = myColorRampItem.value - myPreviousColorRampItem.value;
      myOffsetInRange = theValue - myPreviousColorRampItem.value;

      *theReturnRedValue = ( int )(( double ) myPreviousColorRampItem.color.red() + ((( double )( myColorRampItem.color.red() - myPreviousColorRampItem.color.red() ) / myCurrentRampRange ) * myOffsetInRange ) );
      *theReturnGreenValue = ( int )(( double ) myPreviousColorRampItem.color.green() + ((( double )( myColorRampItem.color.green() - myPreviousColorRampItem.color.green() ) / myCurrentRampRange ) * myOffsetInRange ) );
      *theReturnBlueValue = ( int )(( double ) myPreviousColorRampItem.color.blue() + ((( double )( myColorRampItem.color.blue() - myPreviousColorRampItem.color.blue() ) / myCurrentRampRange ) * myOffsetInRange ) );
      if ( mMaximumColorCacheSize >= mColorCache.size() )
      {
        QColor myNewColor( *theReturnRedValue, *theReturnGreenValue, *theReturnBlueValue );
        mColorCache.insert( theValue, myNewColor );
      }
      return true;
    }
    else if ( mCurrentColorRampItemIndex == 0 && theValue <= myColorRampItem.value )
    {
      QgsColorRampShader::ColorRampItem myPreviousColorRampItem = mColorRampItemList.value( mCurrentColorRampItemIndex - 1 );
      myCurrentRampRange = myColorRampItem.value - myPreviousColorRampItem.value;
      myOffsetInRange = theValue - myPreviousColorRampItem.value;

      *theReturnRedValue = myColorRampItem.color.red();
      *theReturnGreenValue = myColorRampItem.color.green();
      *theReturnBlueValue = myColorRampItem.color.blue();
      if ( mMaximumColorCacheSize >= mColorCache.size() )
      {
        QColor myNewColor( *theReturnRedValue, *theReturnGreenValue, *theReturnBlueValue );
        mColorCache.insert( theValue, myNewColor );
      }
      return true;
    }
    //Search deeper into the color ramp list
    else if ( theValue > myColorRampItem.value )
    {
      mCurrentColorRampItemIndex++;
    }
    else
    {
      return false;
    }
  }

  return false;
}

void QgsColorRampShader::setColorRampItemList( const QList<QgsColorRampShader::ColorRampItem>& theList )
{
  mColorRampItemList = theList;
  //Clear the cache
  mColorCache.clear();
}

void QgsColorRampShader::setColorRampType( QgsColorRampShader::ColorRamp_TYPE theColorRampType )
{
  //When the ramp type changes we need to clear out the cache
  mColorCache.clear();
  mColorRampType = theColorRampType;
}

void QgsColorRampShader::setColorRampType( QString theType )
{
  //When the type of the ramp changes we need to clear out the cache
  mColorCache.clear();
  if ( theType == "INTERPOLATED" )
  {
    mColorRampType = INTERPOLATED;
  }
  else if ( theType == "DISCRETE" )
  {
    mColorRampType = DISCRETE;
  }
  else
  {
    mColorRampType = EXACT;
  }
}

bool QgsColorRampShader::shade( double theValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue )
{

  //Get the shaded value from the cache if it exists already
  QColor myColor = mColorCache.value( theValue );
  if ( myColor.isValid() )
  {
    *theReturnRedValue = myColor.red();
    *theReturnGreenValue = myColor.green();
    *theReturnBlueValue = myColor.blue();
    return true;
  }

  //pixel value not in cache so generate new value

  //Check to be sure mCurrentColorRampItemIndex is within the valid range.
  if ( mCurrentColorRampItemIndex < 0 )
  {
    mCurrentColorRampItemIndex = 0;
  }
  else if ( mCurrentColorRampItemIndex >= mColorRampItemList.size() )
  {
    mCurrentColorRampItemIndex = mColorRampItemList.size() - 1;
  }

  if ( QgsColorRampShader::EXACT == mColorRampType )
  {
    return exactColor( theValue, theReturnRedValue, theReturnGreenValue, theReturnBlueValue );
  }
  else if ( QgsColorRampShader::INTERPOLATED == mColorRampType )
  {
    return interpolatedColor( theValue, theReturnRedValue, theReturnGreenValue, theReturnBlueValue );
  }

  return discreteColor( theValue, theReturnRedValue, theReturnGreenValue, theReturnBlueValue );
}

bool QgsColorRampShader::shade( double theRedValue, double theGreenValue, double theBlueValue, int* theReturnRedValue, int* theReturnGreenValue, int* theReturnBlueValue )
{
  Q_UNUSED( theRedValue );
  Q_UNUSED( theGreenValue );
  Q_UNUSED( theBlueValue );

  *theReturnRedValue = 0;
  *theReturnGreenValue = 0;
  *theReturnBlueValue = 0;

  return false;
}
