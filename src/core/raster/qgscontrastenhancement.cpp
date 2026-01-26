/***************************************************************************
                qgscontrastenhancement.cpp -  description
                       -------------------
begin                : Mon Oct 22 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

This class contains code that was originally part of the larger QgsRasterLayer
class originally created circa 2004 by T.Sutton, Gary E.Sherman, Steve Halasz
****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscontrastenhancement.h"

#include <memory>

#include "qgscliptominmaxenhancement.h"
#include "qgscontrastenhancementfunction.h"
#include "qgslinearminmaxenhancement.h"
#include "qgslinearminmaxenhancementwithclip.h"
#include "qgslogger.h"
#include "qgsrasterblock.h"

#include <QDomDocument>
#include <QDomElement>

QgsContrastEnhancement::QgsContrastEnhancement( Qgis::DataType dataType )
  : mMinimumValue( minimumValuePossible( dataType ) )
  , mMaximumValue( maximumValuePossible( dataType ) )
  , mRasterDataType( dataType )
  , mRasterDataTypeRange( mMaximumValue - mMinimumValue )
  , mLookupTableOffset( mMinimumValue * -1 )
{
  mContrastEnhancementFunction = std::make_unique<QgsContrastEnhancementFunction>( mRasterDataType, mMinimumValue, mMaximumValue );

  //If the data type is larger than 16-bit do not generate a lookup table
  if ( mRasterDataTypeRange <= 65535.0 )
  {
    mLookupTable = std::make_unique<int[]>( static_cast <int>( mRasterDataTypeRange + 1 ) );
  }
}

QgsContrastEnhancement::QgsContrastEnhancement( const QgsContrastEnhancement &ce )
  : mEnhancementDirty( true )
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
    mLookupTable = std::make_unique<int[]>( static_cast <int>( mRasterDataTypeRange + 1 ) );
  }
}

QgsContrastEnhancement::~QgsContrastEnhancement()
{

}

int QgsContrastEnhancement::enhanceContrast( double value )
{
  if ( mEnhancementDirty )
  {
    generateLookupTable();
  }

  if ( mLookupTable && NoEnhancement != mContrastEnhancementAlgorithm )
  {
    const double shiftedValue = value + mLookupTableOffset;
    if ( shiftedValue >= 0 && shiftedValue < mRasterDataTypeRange + 1 )
      return mLookupTable[static_cast <int>( shiftedValue )];
    return 0;
  }
  else
  {
    // Even if the contrast enhancement algorithms is set to NoEnhancement
    // The input values will still have to be scaled for all data types
    // greater than 1 byte.
    return mContrastEnhancementFunction->enhance( value );
  }
}

bool QgsContrastEnhancement::generateLookupTable()
{
  mEnhancementDirty = false;

  if ( !mContrastEnhancementFunction )
    return false;
  if ( NoEnhancement == mContrastEnhancementAlgorithm )
    return false;

  switch ( mRasterDataType )
  {
    case Qgis::DataType::Byte:
    case Qgis::DataType::UInt16:
    case Qgis::DataType::Int16:
    case Qgis::DataType::Int8:
      break;

    case Qgis::DataType::UnknownDataType:
    case Qgis::DataType::UInt32:
    case Qgis::DataType::Int32:
    case Qgis::DataType::Float32:
    case Qgis::DataType::Float64:
    case Qgis::DataType::CInt16:
    case Qgis::DataType::CInt32:
    case Qgis::DataType::CFloat32:
    case Qgis::DataType::CFloat64:
    case Qgis::DataType::ARGB32:
    case Qgis::DataType::ARGB32_Premultiplied:
      return false;
  }

  if ( !mLookupTable )
    return false;

  QgsDebugMsgLevel( u"building lookup table"_s, 4 );
  QgsDebugMsgLevel( u"***MinimumValue : %1"_s.arg( mMinimumValue ), 4 );
  QgsDebugMsgLevel( u"***MaximumValue : %1"_s.arg( mMaximumValue ), 4 );
  QgsDebugMsgLevel( u"***mLookupTableOffset : %1"_s.arg( mLookupTableOffset ), 4 );
  QgsDebugMsgLevel( u"***mRasterDataTypeRange : %1"_s.arg( mRasterDataTypeRange ), 4 );

  for ( int myIterator = 0; myIterator <= mRasterDataTypeRange; myIterator++ )
  {
    mLookupTable[myIterator] = mContrastEnhancementFunction->enhance( static_cast< double >( myIterator ) - mLookupTableOffset );
  }

  return true;
}

bool QgsContrastEnhancement::isValueInDisplayableRange( double value )
{
  if ( mContrastEnhancementFunction )
  {
    return mContrastEnhancementFunction->isValueInDisplayableRange( value );
  }

  return false;
}

void QgsContrastEnhancement::setContrastEnhancementAlgorithm( ContrastEnhancementAlgorithm algorithm, bool generateTable )
{
  switch ( algorithm )
  {
    case StretchToMinimumMaximum :
      mContrastEnhancementFunction = std::make_unique<QgsLinearMinMaxEnhancement>( mRasterDataType, mMinimumValue, mMaximumValue );
      break;
    case StretchAndClipToMinimumMaximum :
      mContrastEnhancementFunction = std::make_unique<QgsLinearMinMaxEnhancementWithClip>( mRasterDataType, mMinimumValue, mMaximumValue );
      break;
    case ClipToMinimumMaximum :
      mContrastEnhancementFunction = std::make_unique<QgsClipToMinMaxEnhancement>( mRasterDataType, mMinimumValue, mMaximumValue );
      break;
    case UserDefinedEnhancement :
      //Do nothing
      break;
    default:
      mContrastEnhancementFunction = std::make_unique<QgsContrastEnhancementFunction>( mRasterDataType, mMinimumValue, mMaximumValue );
      break;
  }

  mEnhancementDirty = true;
  mContrastEnhancementAlgorithm = algorithm;

  if ( generateTable )
  {
    generateLookupTable();
  }
}

void QgsContrastEnhancement::setContrastEnhancementFunction( QgsContrastEnhancementFunction *function )
{
  QgsDebugMsgLevel( u"called"_s, 4 );

  if ( function )
  {
    mContrastEnhancementFunction.reset( function );
    mContrastEnhancementAlgorithm = UserDefinedEnhancement;
    generateLookupTable();
  }
}

void QgsContrastEnhancement::setMaximumValue( double value, bool generateTable )
{
  QgsDebugMsgLevel( "called value: " + QString::number( value ) + " generate lookup table: " + QString::number( static_cast< int >( generateTable ) ), 4 );

  if ( value > maximumValuePossible( mRasterDataType ) )
  {
    mMaximumValue = maximumValuePossible( mRasterDataType );
  }
  else
  {
    mMaximumValue = value;
  }

  if ( mContrastEnhancementFunction )
  {
    mContrastEnhancementFunction->setMaximumValue( value );
  }

  mEnhancementDirty = true;

  if ( generateTable )
  {
    generateLookupTable();
  }
}

void QgsContrastEnhancement::setMinimumValue( double value, bool generateTable )
{
  QgsDebugMsgLevel( "called value: " + QString::number( value ) + " generate lookup table: " + QString::number( static_cast< int >( generateTable ) ), 4 );

  if ( value < minimumValuePossible( mRasterDataType ) )
  {
    mMinimumValue = minimumValuePossible( mRasterDataType );
  }
  else
  {
    mMinimumValue = value;
  }

  if ( mContrastEnhancementFunction )
  {
    mContrastEnhancementFunction->setMinimumValue( value );
  }

  mEnhancementDirty = true;

  if ( generateTable )
  {
    generateLookupTable();
  }
}

void QgsContrastEnhancement::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  //minimum value
  QDomElement minElem = doc.createElement( u"minValue"_s );
  const QDomText minText = doc.createTextNode( QgsRasterBlock::printValue( mMinimumValue ) );
  minElem.appendChild( minText );
  parentElem.appendChild( minElem );

  //maximum value
  QDomElement maxElem = doc.createElement( u"maxValue"_s );
  const QDomText maxText = doc.createTextNode( QgsRasterBlock::printValue( mMaximumValue ) );
  maxElem.appendChild( maxText );
  parentElem.appendChild( maxElem );

  //algorithm
  QDomElement algorithmElem = doc.createElement( u"algorithm"_s );
  const QDomText algorithmText = doc.createTextNode( contrastEnhancementAlgorithmString( mContrastEnhancementAlgorithm ) );
  algorithmElem.appendChild( algorithmText );
  parentElem.appendChild( algorithmElem );
}

void QgsContrastEnhancement::readXml( const QDomElement &elem )
{
  const QDomElement minValueElem = elem.firstChildElement( u"minValue"_s );
  if ( !minValueElem.isNull() )
  {
    mMinimumValue = minValueElem.text().toDouble();
  }
  const QDomElement maxValueElem = elem.firstChildElement( u"maxValue"_s );
  if ( !maxValueElem.isNull() )
  {
    mMaximumValue = maxValueElem.text().toDouble();
  }
  const QDomElement algorithmElem = elem.firstChildElement( u"algorithm"_s );
  if ( !algorithmElem.isNull() )
  {
    const QString algorithmString = algorithmElem.text();
    ContrastEnhancementAlgorithm algorithm = NoEnhancement;
    // old version ( < 19 Apr 2013) was using enum directly -> for backward compatibility
    if ( algorithmString == "0"_L1 )
    {
      algorithm = NoEnhancement;
    }
    else if ( algorithmString == "1"_L1 )
    {
      algorithm = StretchToMinimumMaximum;
    }
    else if ( algorithmString == "2"_L1 )
    {
      algorithm = StretchAndClipToMinimumMaximum;
    }
    else if ( algorithmString == "3"_L1 )
    {
      algorithm = ClipToMinimumMaximum;
    }
    else if ( algorithmString == "4"_L1 )
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

void QgsContrastEnhancement::toSld( QDomDocument &doc, QDomElement &element ) const
{
  if ( doc.isNull() || element.isNull() )
    return;

  QString algName;
  switch ( contrastEnhancementAlgorithm() )
  {
    case StretchToMinimumMaximum:
      algName = u"StretchToMinimumMaximum"_s;
      break;
    /* TODO: check if ClipToZero => StretchAndClipToMinimumMaximum
     * because value outside min/max ar considered as NoData instead of 0 */
    case StretchAndClipToMinimumMaximum:
      algName = u"ClipToMinimumMaximum"_s;
      break;
    case ClipToMinimumMaximum:
      algName = u"ClipToMinimumMaximum"_s;
      break;
    case NoEnhancement:
      return;
    case UserDefinedEnhancement:
      algName = contrastEnhancementAlgorithmString( contrastEnhancementAlgorithm() );
      QgsDebugError( u"No SLD1.0 conversion yet for stretch algorithm %1"_s.arg( algName ) );
      return;
  }

  // Only <Normalize> is supported
  // minValue and maxValue are that values as set depending on "Min /Max value settings"
  // parameters
  QDomElement normalizeElem = doc.createElement( u"sld:Normalize"_s );
  element.appendChild( normalizeElem );

  QDomElement vendorOptionAlgorithmElem = doc.createElement( u"sld:VendorOption"_s );
  vendorOptionAlgorithmElem.setAttribute( u"name"_s, u"algorithm"_s );
  vendorOptionAlgorithmElem.appendChild( doc.createTextNode( algName ) );
  normalizeElem.appendChild( vendorOptionAlgorithmElem );

  QDomElement vendorOptionMinValueElem = doc.createElement( u"sld:VendorOption"_s );
  vendorOptionMinValueElem.setAttribute( u"name"_s, u"minValue"_s );
  vendorOptionMinValueElem.appendChild( doc.createTextNode( QString::number( minimumValue() ) ) );
  normalizeElem.appendChild( vendorOptionMinValueElem );

  QDomElement vendorOptionMaxValueElem = doc.createElement( u"sld:VendorOption"_s );
  vendorOptionMaxValueElem.setAttribute( u"name"_s, u"maxValue"_s );
  vendorOptionMaxValueElem.appendChild( doc.createTextNode( QString::number( maximumValue() ) ) );
  normalizeElem.appendChild( vendorOptionMaxValueElem );
}

QString QgsContrastEnhancement::contrastEnhancementAlgorithmString( ContrastEnhancementAlgorithm algorithm )
{
  switch ( algorithm )
  {
    case NoEnhancement:
      return u"NoEnhancement"_s;
    case StretchToMinimumMaximum:
      return u"StretchToMinimumMaximum"_s;
    case StretchAndClipToMinimumMaximum:
      return u"StretchAndClipToMinimumMaximum"_s;
    case ClipToMinimumMaximum:
      return u"ClipToMinimumMaximum"_s;
    case UserDefinedEnhancement:
      return u"UserDefinedEnhancement"_s;
  }
  return u"NoEnhancement"_s;
}

QgsContrastEnhancement::ContrastEnhancementAlgorithm QgsContrastEnhancement::contrastEnhancementAlgorithmFromString( const QString &contrastEnhancementString )
{
  if ( contrastEnhancementString == "StretchToMinimumMaximum"_L1 )
  {
    return StretchToMinimumMaximum;
  }
  else if ( contrastEnhancementString == "StretchAndClipToMinimumMaximum"_L1 )
  {
    return StretchAndClipToMinimumMaximum;
  }
  else if ( contrastEnhancementString == "ClipToMinimumMaximum"_L1 )
  {
    return ClipToMinimumMaximum;
  }
  else if ( contrastEnhancementString == "UserDefinedEnhancement"_L1 )
  {
    return UserDefinedEnhancement;
  }
  else
  {
    return NoEnhancement;
  }
}
