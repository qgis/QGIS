/* **************************************************************************
                qgsrastertransparency.cpp -  description
                       -------------------
begin                : Mon Nov 30 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QList>

#include "qgsrastertransparency.h"
#include "qgis.h"

QgsRasterTransparency::QgsRasterTransparency()
{

}

/**
  Accessor for transparentSingleValuePixelList
*/
QList<QgsRasterTransparency::TransparentSingleValuePixel> QgsRasterTransparency::transparentSingleValuePixelList() const
{
  return mTransparentSingleValuePixelList;
}

/**
  Accessor for transparentThreeValuePixelList
*/
QList<QgsRasterTransparency::TransparentThreeValuePixel> QgsRasterTransparency::transparentThreeValuePixelList() const
{
  return mTransparentThreeValuePixelList;
}

/**
  Reset to the transparency list to a single value
*/
void QgsRasterTransparency::initializeTransparentPixelList( double theValue )
{
  //clear the existing list
  mTransparentSingleValuePixelList.clear();

  //add the initial value
  TransparentSingleValuePixel myTransparentSingleValuePixel;
  myTransparentSingleValuePixel.pixelValue = theValue;
  myTransparentSingleValuePixel.percentTransparent = 100.0;
  mTransparentSingleValuePixelList.append( myTransparentSingleValuePixel );
}

/**
  Reset to the transparency list to a single value
*/
void QgsRasterTransparency::initializeTransparentPixelList( double theRedValue, double theGreenValue, double theBlueValue )
{
  //clearn the existing list
  mTransparentThreeValuePixelList.clear();

  //add the initial values
  TransparentThreeValuePixel myTransparentThreeValuePixel;
  myTransparentThreeValuePixel.red = theRedValue;
  myTransparentThreeValuePixel.green = theGreenValue;
  myTransparentThreeValuePixel.blue = theBlueValue;
  myTransparentThreeValuePixel.percentTransparent = 100.0;
  mTransparentThreeValuePixelList.append( myTransparentThreeValuePixel );
}


/**
  Mutator for transparentSingleValuePixelList, replaces the whole list
*/
void QgsRasterTransparency::setTransparentSingleValuePixelList( QList<QgsRasterTransparency::TransparentSingleValuePixel> theNewList )
{
  mTransparentSingleValuePixelList = theNewList;
}

/**
  Mutator for transparentThreeValuePixelList, replaces the whole list
*/
void QgsRasterTransparency::setTransparentThreeValuePixelList( QList<QgsRasterTransparency::TransparentThreeValuePixel> theNewList )
{
  mTransparentThreeValuePixelList = theNewList;
}

/**
  Searches through the transparency list, if a match is found, the global transparency value is scaled
  by the stored transparency value.
  @param theValue the needle to search for in the transparency hay stack
  @param theGlobalTransparency  the overal transparency level for the layer
*/
int QgsRasterTransparency::alphaValue( double theValue, int theGlobalTransparency ) const
{
  //if NaN return 0, transparent
  if ( theValue != theValue )
  {
    return 0;
  }

  //Search through the transparency list looking for a match
  bool myTransparentPixelFound = false;
  TransparentSingleValuePixel myTransparentPixel = {0, 100};
  for ( int myListRunner = 0; myListRunner < mTransparentSingleValuePixelList.count(); myListRunner++ )
  {
    myTransparentPixel = mTransparentSingleValuePixelList[myListRunner];
    if ( myTransparentPixel.pixelValue == theValue )
    {
      myTransparentPixelFound = true;
      break;
    }
  }

  //if a match was found use the stored transparency percentage
  if ( myTransparentPixelFound )
  {
    return ( int )(( float )theGlobalTransparency *( 1.0 - ( myTransparentPixel.percentTransparent / 100.0 ) ) );
  }

  return theGlobalTransparency;
}

/**
  Searches through the transparency list, if a match is found, the global transparency value is scaled
  by the stored transparency value.
  @param theRedValue the red portion of the needle to search for in the transparency hay stack
  @param theGreenValue  the green portion of the needle to search for in the transparency hay stack
  @param theBlueValue the green portion of the needle to search for in the transparency hay stack
  @param theGlobalTransparency  the overal transparency level for the layer
*/
int QgsRasterTransparency::alphaValue( double theRedValue, double theGreenValue, double theBlueValue, int theGlobalTransparency ) const
{
  //if NaN return 0, transparent
  if ( theRedValue != theRedValue || theGreenValue != theGreenValue || theBlueValue != theBlueValue )
  {
    return 0;
  }

  //Search through the transparency list looking for a match
  bool myTransparentPixelFound = false;
  TransparentThreeValuePixel myTransparentPixel = {0, 0, 0, 100};
  for ( int myListRunner = 0; myListRunner < mTransparentThreeValuePixelList.count(); myListRunner++ )
  {
    myTransparentPixel = mTransparentThreeValuePixelList[myListRunner];
    if ( myTransparentPixel.red == theRedValue )
    {
      if ( myTransparentPixel.green == theGreenValue )
      {
        if ( myTransparentPixel.blue == theBlueValue )
        {
          myTransparentPixelFound = true;
          break;
        }
      }
    }
  }

  //if a match was found use the stored transparency percentage
  if ( myTransparentPixelFound )
  {
    return ( int )(( float )theGlobalTransparency *( 1.0 - ( myTransparentPixel.percentTransparent / 100.0 ) ) );
  }

  return theGlobalTransparency;
}

bool QgsRasterTransparency::isEmpty( double nodataValue ) const
{
  return (
           ( mTransparentSingleValuePixelList.isEmpty() ||
             ( mTransparentSingleValuePixelList.size() == 1 && doubleNear( mTransparentSingleValuePixelList.at( 0 ).pixelValue, nodataValue ) ) )
           &&
           ( mTransparentThreeValuePixelList.isEmpty() ||
             ( mTransparentThreeValuePixelList.size() < 4 && doubleNear( mTransparentThreeValuePixelList.at( 0 ).red, nodataValue ) &&
               doubleNear( mTransparentThreeValuePixelList.at( 0 ).green, nodataValue ) && doubleNear( mTransparentThreeValuePixelList.at( 0 ).blue, nodataValue ) ) ) );
}
