/* **************************************************************************
                qgscontrastenhancement.cpp -  description
                       -------------------
begin                : Mon Oct 22 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

This class contains code that was originally part of the larger QgsRasterLayer
class originally created circa 2004 by T.Sutton, Gary E.Sherman, Steve Halasz
****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"

#include "qgscontrastenhancement.h"
#include "qgscontrastenhancementfunction.h"
#include "qgslinearminmaxenhancement.h"
#include "qgslinearminmaxenhancementwithclip.h"
#include "qgscliptominmaxenhancement.h"
#include <QDomDocument>
#include <QDomElement>

QgsContrastEnhancement::QgsContrastEnhancement( QgsRasterDataType theDataType )
{
  mLookupTable = 0;
  mContrastEnhancementFunction = 0;
  mEnhancementDirty = false;
  mContrastEnhancementAlgorithm = NoEnhancement;
  mRasterDataType = theDataType;

  mMinimumValue = minimumValuePossible( mRasterDataType );
  mMaximumValue = maximumValuePossible( mRasterDataType );
  mRasterDataTypeRange = mMaximumValue - mMinimumValue;

  mLookupTableOffset = mMinimumValue * -1;

  mContrastEnhancementFunction = new QgsContrastEnhancementFunction( mRasterDataType, mMinimumValue, mMaximumValue );

  //If the data type is larger than 16-bit do not generate a lookup table
  if ( mRasterDataTypeRange <= 65535.0 )
  {
    mLookupTable = new int[static_cast <int>( mRasterDataTypeRange+1 )];
  }

}

QgsContrastEnhancement::~QgsContrastEnhancement()
{
}
/*
 *
 * Static methods
 *
 */

/**
    Simple function to compute the maximum possible value for a data types.
*/
double QgsContrastEnhancement::maximumValuePossible( QgsRasterDataType theDataType )
{
  switch ( theDataType )
  {
    case QGS_Byte:
      return std::numeric_limits<unsigned char>::max();
      break;
    case QGS_UInt16:
      return std::numeric_limits<unsigned short>::max();
      break;
    case QGS_Int16:
      return std::numeric_limits<short>::max();
      break;
    case QGS_UInt32:
      return std::numeric_limits<unsigned int>::max();
      break;
    case QGS_Int32:
      return std::numeric_limits<int>::max();
      break;
    case QGS_Float32:
      return std::numeric_limits<float>::max();
      break;
    case QGS_Float64:
      return std::numeric_limits<double>::max();
      break;
    case QGS_CInt16:
      return std::numeric_limits<short>::max();
      break;
    case QGS_CInt32:
      return std::numeric_limits<int>::max();
      break;
    case QGS_CFloat32:
      return std::numeric_limits<float>::max();
      break;
    case QGS_CFloat64:
      return std::numeric_limits<double>::max();
      break;
    case QGS_Unknown:
    case QGS_TypeCount:
      // XXX - mloskot: not handled?
      break;
  }

  return std::numeric_limits<double>::max();
}
/**
    Simple function to compute the minimum possible value for a data type.
*/
double QgsContrastEnhancement::minimumValuePossible( QgsRasterDataType theDataType )
{
  switch ( theDataType )
  {
    case QGS_Byte:
      return std::numeric_limits<unsigned char>::min();
      break;
    case QGS_UInt16:
      return std::numeric_limits<unsigned short>::min();
      break;
    case QGS_Int16:
      return std::numeric_limits<short>::min();
      break;
    case QGS_UInt32:
      return std::numeric_limits<unsigned int>::min();
      break;
    case QGS_Int32:
      return std::numeric_limits<int>::min();
      break;
    case QGS_Float32:
      return std::numeric_limits<float>::max() * -1.0;
      break;
    case QGS_Float64:
      return std::numeric_limits<double>::max() * -1.0;
      break;
    case QGS_CInt16:
      return std::numeric_limits<short>::min();
      break;
    case QGS_CInt32:
      return std::numeric_limits<int>::min();
      break;
    case QGS_CFloat32:
      return std::numeric_limits<float>::max() * -1.0;
      break;
    case QGS_CFloat64:
      return std::numeric_limits<double>::max() * -1.0;
      break;
    case QGS_Unknown:
    case QGS_TypeCount:
      // XXX - mloskot: not handled?
      break;
  }

  return std::numeric_limits<double>::max() * -1.0;
}

/*
 *
 * Non-Static methods
 *
 */
/**
    Public function to generate the enhanced for enhanceContrasted value for a given input.

    @param theValue The pixel value to enhance
*/
int QgsContrastEnhancement::enhanceContrast( double theValue )
{
  if ( mEnhancementDirty )
  {
    generateLookupTable();
  }

  if ( mLookupTable && NoEnhancement != mContrastEnhancementAlgorithm )
  {
    return mLookupTable[static_cast <int>( theValue + mLookupTableOffset )];
  }
  else
  {
    // Even if the contrast enhancement algorithms is set to NoEnhancement
    // The input values will still have to be scaled for all data types
    // greater than 1 byte.
    return mContrastEnhancementFunction->enhance( theValue );
  }
}

/**
    Generate a new lookup table
*/
bool QgsContrastEnhancement::generateLookupTable()
{
  mEnhancementDirty = false;

  if ( !mContrastEnhancementFunction )
    return false;
  if ( NoEnhancement == mContrastEnhancementAlgorithm )
    return false;
  if ( QGS_Byte != mRasterDataType && QGS_UInt16 != mRasterDataType && QGS_Int16 != mRasterDataType )
    return false;
  if ( !mLookupTable )
    return false;

  QgsDebugMsg( "building lookup table" );
  QgsDebugMsg( "***MinimumValue : " + QString::number( mMinimumValue ) );
  QgsDebugMsg( "***MaximumValue : " + QString::number( mMaximumValue ) );
  QgsDebugMsg( "***mLookupTableOffset : " + QString::number( mLookupTableOffset ) );
  QgsDebugMsg( "***mRasterDataTypeRange : " + QString::number( mRasterDataTypeRange ) );

  for ( int myIterator = 0; myIterator <= mRasterDataTypeRange; myIterator++ )
  {
    mLookupTable[myIterator] = mContrastEnhancementFunction->enhance(( double )myIterator - mLookupTableOffset );
  }

  return true;
}

/**
    Determine if a pixel is within in the displayable range.

    @param theValue The pixel value to examine
*/
bool QgsContrastEnhancement::isValueInDisplayableRange( double theValue )
{

  if ( 0 != mContrastEnhancementFunction )
  {
    return mContrastEnhancementFunction->isValueInDisplayableRange( theValue );
  }

  return false;
}

/**
    Set the contrast enhancement algorithm. The second parameter is optional and is for performace improvements. If you know you are immediately going to set the Minimum or Maximum value, you can elect to not generate the lookup tale. By default it will be generated.

    @param theAlgorithm The new contrast enhancement algorithm
    @param generateTable Flag to overide automatic look up table generation
*/
void QgsContrastEnhancement::setContrastEnhancementAlgorithm( ContrastEnhancementAlgorithm theAlgorithm, bool generateTable )
{
  QgsDebugMsg( "called algorithm: " + QString::number(( int )theAlgorithm ) + " generate lookup table: " + QString::number(( int )generateTable ) );

  if ( theAlgorithm != mContrastEnhancementAlgorithm )
  {
    switch ( theAlgorithm )
    {
      case StretchToMinimumMaximum :
        mContrastEnhancementFunction = new QgsLinearMinMaxEnhancement( mRasterDataType, mMinimumValue, mMaximumValue );
        break;
      case StretchAndClipToMinimumMaximum :
        mContrastEnhancementFunction = new QgsLinearMinMaxEnhancementWithClip( mRasterDataType, mMinimumValue, mMaximumValue );
        break;
      case ClipToMinimumMaximum :
        mContrastEnhancementFunction = new QgsClipToMinMaxEnhancement( mRasterDataType, mMinimumValue, mMaximumValue );
        break;
      case UserDefinedEnhancement :
        //Do nothing
        break;
      default:
        mContrastEnhancementFunction = new QgsContrastEnhancementFunction( mRasterDataType, mMinimumValue, mMaximumValue );
        break;
    }

    mEnhancementDirty = true;
    mContrastEnhancementAlgorithm = theAlgorithm;

    if ( generateTable )
    {
      generateLookupTable();
    }
  }
}

/**
    A public function that allows the user to set their own custom contrast enhancement function.

    @param theFunction The new contrast enhancement function
*/
void QgsContrastEnhancement::setContrastEnhancementFunction( QgsContrastEnhancementFunction* theFunction )
{
  QgsDebugMsg( "called" );

  if ( 0 != theFunction )
  {
    mContrastEnhancementFunction = theFunction;
    mContrastEnhancementAlgorithm = UserDefinedEnhancement;
    generateLookupTable();
  }
}

/**
    Set the maximum value for the contrast enhancement. The second parameter is option an is for performace improvements. If you know you are immediately going to set the Minimum value or the contrast enhancement algorithm, you can elect to not generate the lookup tale. By default it will be generated.

    @param theValue The new maximum value for the band
    @param generateTable Flag to overide automatic look up table generation
*/
void QgsContrastEnhancement::setMaximumValue( double theValue, bool generateTable )
{
  QgsDebugMsg( "called value: " + QString::number( theValue ) + " generate lookup table: " + QString::number(( int )generateTable ) );

  if ( theValue > maximumValuePossible( mRasterDataType ) )
  {
    mMaximumValue = maximumValuePossible( mRasterDataType );
  }
  else
  {
    mMaximumValue = theValue;
  }

  if ( 0 != mContrastEnhancementFunction )
  {
    mContrastEnhancementFunction->setMaximumValue( theValue );
  }

  mEnhancementDirty = true;

  if ( generateTable )
  {
    generateLookupTable();
  }
}

/**
    Set the maximum value for the contrast enhancement. The second parameter is option an is for performace improvements. If you know you are immediately going to set the Maximum value or the contrast enhancement algorithm, you can elect to not generate the lookup tale. By default it will be generated.

    @param theValue The new minimum value for the band
    @param generateTable Flag to overide automatic look up table generation
*/
void QgsContrastEnhancement::setMinimumValue( double theValue, bool generateTable )
{
  QgsDebugMsg( "called value: " + QString::number( theValue ) + " generate lookup table: " + QString::number(( int )generateTable ) );

  if ( theValue < minimumValuePossible( mRasterDataType ) )
  {
    mMinimumValue = minimumValuePossible( mRasterDataType );
  }
  else
  {
    mMinimumValue = theValue;
  }

  if ( 0 != mContrastEnhancementFunction )
  {
    mContrastEnhancementFunction->setMinimumValue( theValue );
  }

  mEnhancementDirty = true;

  if ( generateTable )
  {
    generateLookupTable();
  }
}

void QgsContrastEnhancement::writeXML( QDomDocument& doc, QDomElement& parentElem ) const
{
  //minimum value
  QDomElement minElem = doc.createElement( "minValue" );
  QDomText minText = doc.createTextNode( QString::number( mMinimumValue ) );
  minElem.appendChild( minText );
  parentElem.appendChild( minElem );

  //maximum value
  QDomElement maxElem = doc.createElement( "maxValue" );
  QDomText maxText = doc.createTextNode( QString::number( mMaximumValue ) );
  maxElem.appendChild( maxText );
  parentElem.appendChild( maxElem );

  //algorithm
  QDomElement algorithmElem = doc.createElement( "algorithm" );
  QDomText algorithmText = doc.createTextNode( QString::number( mContrastEnhancementAlgorithm ) );
  algorithmElem.appendChild( algorithmText );
  parentElem.appendChild( algorithmElem );
}

void QgsContrastEnhancement::readXML( const QDomElement& elem )
{
  QDomElement minValueElem = elem.firstChildElement( "minValue" );
  if ( !minValueElem.isNull() )
  {
    mMinimumValue = minValueElem.text().toDouble();
  }
  QDomElement maxValueElem = elem.firstChildElement( "maxValue" );
  if ( !maxValueElem.isNull() )
  {
    mMaximumValue = maxValueElem.text().toDouble();
  }
  QDomElement algorithmElem = elem.firstChildElement( "algorithm" );
  if ( !algorithmElem.isNull() )
  {
    setContrastEnhancementAlgorithm(( ContrastEnhancementAlgorithm )( algorithmElem.text().toInt() ) );
  }
}

QgsContrastEnhancement::ContrastEnhancementAlgorithm QgsContrastEnhancement::contrastEnhancementAlgorithmFromString( const QString& contrastEnhancementString )
{
  if ( contrastEnhancementString == "StretchToMinimumMaximum" )
  {
    return StretchToMinimumMaximum;
  }
  else if ( contrastEnhancementString == "StretchAndClipToMinimumMaximum" )
  {
    return StretchAndClipToMinimumMaximum;
  }
  else if ( contrastEnhancementString == "ClipToMinimumMaximum" )
  {
    return ClipToMinimumMaximum;
  }
  else
  {
    return NoEnhancement;
  }
}
