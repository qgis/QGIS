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
#include "qgsrasterblock.h"
#include <QDomDocument>
#include <QDomElement>

QgsContrastEnhancement::QgsContrastEnhancement( QGis::DataType theDataType )
    : mContrastEnhancementAlgorithm( NoEnhancement )
    , mContrastEnhancementFunction( nullptr )
    , mEnhancementDirty( false )
    , mLookupTable( nullptr )
    , mRasterDataType( theDataType )
{
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

QgsContrastEnhancement::QgsContrastEnhancement( const QgsContrastEnhancement& ce )
    : mContrastEnhancementFunction( nullptr )
    , mEnhancementDirty( true )
    , mLookupTable( nullptr )
    , mMinimumValue( ce.mMinimumValue )
    , mMaximumValue( ce.mMaximumValue )
    , mRasterDataType( ce.mRasterDataType )
    , mRasterDataTypeRange( ce.mRasterDataTypeRange )
{
  mLookupTableOffset = minimumValuePossible( mRasterDataType ) * -1;

  // setContrastEnhancementAlgorithm sets also QgsContrastEnhancementFunction
  setContrastEnhancementAlgorithm( ce.mContrastEnhancementAlgorithm, false );

  //If the data type is larger than 16-bit do not generate a lookup table
  if ( mRasterDataTypeRange <= 65535.0 )
  {
    mLookupTable = new int[static_cast <int>( mRasterDataTypeRange+1 )];
  }
}

QgsContrastEnhancement::~QgsContrastEnhancement()
{
  delete [] mLookupTable;
  delete mContrastEnhancementFunction;
}
/*
 *
 * Static methods
 *
 */

/**
    Simple function to compute the maximum possible value for a data types.
*/
double QgsContrastEnhancement::maximumValuePossible( QGis::DataType theDataType )
{
  switch ( theDataType )
  {
    case QGis::Byte:
      return std::numeric_limits<unsigned char>::max();
    case QGis::UInt16:
      return std::numeric_limits<unsigned short>::max();
    case QGis::Int16:
      return std::numeric_limits<short>::max();
    case QGis::UInt32:
      return std::numeric_limits<unsigned int>::max();
    case QGis::Int32:
      return std::numeric_limits<int>::max();
    case QGis::Float32:
      return std::numeric_limits<float>::max();
    case QGis::Float64:
      return std::numeric_limits<double>::max();
    case QGis::CInt16:
      return std::numeric_limits<short>::max();
    case QGis::CInt32:
      return std::numeric_limits<int>::max();
    case QGis::CFloat32:
      return std::numeric_limits<float>::max();
    case QGis::CFloat64:
      return std::numeric_limits<double>::max();
    case QGis::ARGB32:
    case QGis::ARGB32_Premultiplied:
    case QGis::UnknownDataType:
      // XXX - mloskot: not handled?
      break;
  }

  return std::numeric_limits<double>::max();
}
/**
    Simple function to compute the minimum possible value for a data type.
*/
double QgsContrastEnhancement::minimumValuePossible( QGis::DataType theDataType )
{
  switch ( theDataType )
  {
    case QGis::Byte:
      return std::numeric_limits<unsigned char>::min();
    case QGis::UInt16:
      return std::numeric_limits<unsigned short>::min();
    case QGis::Int16:
      return std::numeric_limits<short>::min();
    case QGis::UInt32:
      return std::numeric_limits<unsigned int>::min();
    case QGis::Int32:
      return std::numeric_limits<int>::min();
    case QGis::Float32:
      return std::numeric_limits<float>::max() * -1.0;
    case QGis::Float64:
      return std::numeric_limits<double>::max() * -1.0;
    case QGis::CInt16:
      return std::numeric_limits<short>::min();
    case QGis::CInt32:
      return std::numeric_limits<int>::min();
    case QGis::CFloat32:
      return std::numeric_limits<float>::max() * -1.0;
    case QGis::CFloat64:
      return std::numeric_limits<double>::max() * -1.0;
    case QGis::ARGB32:
    case QGis::ARGB32_Premultiplied:
    case QGis::UnknownDataType:
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
  if ( QGis::Byte != mRasterDataType && QGis::UInt16 != mRasterDataType && QGis::Int16 != mRasterDataType )
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
    mLookupTable[myIterator] = mContrastEnhancementFunction->enhance( static_cast< double >( myIterator ) - mLookupTableOffset );
  }

  return true;
}

/**
    Determine if a pixel is within in the displayable range.

    @param theValue The pixel value to examine
*/
bool QgsContrastEnhancement::isValueInDisplayableRange( double theValue )
{
  if ( mContrastEnhancementFunction )
  {
    return mContrastEnhancementFunction->isValueInDisplayableRange( theValue );
  }

  return false;
}

/**
    Set the contrast enhancement algorithm. The second parameter is optional and is for performace improvements. If you know you are immediately going to set the Minimum or Maximum value, you can elect to not generate the lookup tale. By default it will be generated.

    @param theAlgorithm The new contrast enhancement algorithm
    @param generateTable Flag to override automatic look up table generation
*/
void QgsContrastEnhancement::setContrastEnhancementAlgorithm( ContrastEnhancementAlgorithm theAlgorithm, bool generateTable )
{
  switch ( theAlgorithm )
  {
    case StretchToMinimumMaximum :
      delete mContrastEnhancementFunction;
      mContrastEnhancementFunction = new QgsLinearMinMaxEnhancement( mRasterDataType, mMinimumValue, mMaximumValue );
      break;
    case StretchAndClipToMinimumMaximum :
      delete mContrastEnhancementFunction;
      mContrastEnhancementFunction = new QgsLinearMinMaxEnhancementWithClip( mRasterDataType, mMinimumValue, mMaximumValue );
      break;
    case ClipToMinimumMaximum :
      delete mContrastEnhancementFunction;
      mContrastEnhancementFunction = new QgsClipToMinMaxEnhancement( mRasterDataType, mMinimumValue, mMaximumValue );
      break;
    case UserDefinedEnhancement :
      //Do nothing
      break;
    default:
      delete mContrastEnhancementFunction;
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

/**
    A public function that allows the user to set their own custom contrast enhancement function.

    @param theFunction The new contrast enhancement function
*/
void QgsContrastEnhancement::setContrastEnhancementFunction( QgsContrastEnhancementFunction* theFunction )
{
  QgsDebugMsgLevel( "called", 4 );

  if ( theFunction )
  {
    delete mContrastEnhancementFunction;
    mContrastEnhancementFunction = theFunction;
    mContrastEnhancementAlgorithm = UserDefinedEnhancement;
    generateLookupTable();
  }
}

/**
    Set the maximum value for the contrast enhancement. The second parameter is option an is for performace improvements. If you know you are immediately going to set the Minimum value or the contrast enhancement algorithm, you can elect to not generate the lookup tale. By default it will be generated.

    @param theValue The new maximum value for the band
    @param generateTable Flag to override automatic look up table generation
*/
void QgsContrastEnhancement::setMaximumValue( double theValue, bool generateTable )
{
  QgsDebugMsgLevel( "called value: " + QString::number( theValue ) + " generate lookup table: " + QString::number( static_cast< int >( generateTable ) ), 4 );

  if ( theValue > maximumValuePossible( mRasterDataType ) )
  {
    mMaximumValue = maximumValuePossible( mRasterDataType );
  }
  else
  {
    mMaximumValue = theValue;
  }

  if ( mContrastEnhancementFunction )
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
    @param generateTable Flag to override automatic look up table generation
*/
void QgsContrastEnhancement::setMinimumValue( double theValue, bool generateTable )
{
  QgsDebugMsgLevel( "called value: " + QString::number( theValue ) + " generate lookup table: " + QString::number( static_cast< int >( generateTable ) ), 4 );

  if ( theValue < minimumValuePossible( mRasterDataType ) )
  {
    mMinimumValue = minimumValuePossible( mRasterDataType );
  }
  else
  {
    mMinimumValue = theValue;
  }

  if ( mContrastEnhancementFunction )
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
  QDomText minText = doc.createTextNode( QgsRasterBlock::printValue( mMinimumValue ) );
  minElem.appendChild( minText );
  parentElem.appendChild( minElem );

  //maximum value
  QDomElement maxElem = doc.createElement( "maxValue" );
  QDomText maxText = doc.createTextNode( QgsRasterBlock::printValue( mMaximumValue ) );
  maxElem.appendChild( maxText );
  parentElem.appendChild( maxElem );

  //algorithm
  QDomElement algorithmElem = doc.createElement( "algorithm" );
  QDomText algorithmText = doc.createTextNode( contrastEnhancementAlgorithmString( mContrastEnhancementAlgorithm ) );
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
    QString algorithmString = algorithmElem.text();
    ContrastEnhancementAlgorithm algorithm = NoEnhancement;
    // old version ( < 19 Apr 2013) was using enum directly -> for backward compatibility
    if ( algorithmString == "0" )
    {
      algorithm = NoEnhancement;
    }
    else if ( algorithmString == "1" )
    {
      algorithm = StretchToMinimumMaximum;
    }
    else if ( algorithmString == "2" )
    {
      algorithm = StretchAndClipToMinimumMaximum;
    }
    else if ( algorithmString == "3" )
    {
      algorithm = ClipToMinimumMaximum;
    }
    else if ( algorithmString == "4" )
    {
      algorithm = UserDefinedEnhancement;
    }
    else
    {
      algorithm = contrastEnhancementAlgorithmFromString( algorithmString );
    }

    setContrastEnhancementAlgorithm( algorithm );
  }
}

QString QgsContrastEnhancement::contrastEnhancementAlgorithmString( ContrastEnhancementAlgorithm algorithm )
{
  switch ( algorithm )
  {
    case NoEnhancement:
      return "NoEnhancement";
    case StretchToMinimumMaximum:
      return "StretchToMinimumMaximum";
    case StretchAndClipToMinimumMaximum:
      return "StretchAndClipToMinimumMaximum";
    case ClipToMinimumMaximum:
      return "ClipToMinimumMaximum";
    case UserDefinedEnhancement:
      return "UserDefinedEnhancement";
  }
  return "NoEnhancement";
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
  else if ( contrastEnhancementString == "UserDefinedEnhancement" )
  {
    return UserDefinedEnhancement;
  }
  else
  {
    return NoEnhancement;
  }
}
