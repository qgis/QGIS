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

QgsContrastEnhancement::QgsContrastEnhancement( Qgis::DataType dataType )
  : mRasterDataType( dataType )
{
  mMinimumValue = minimumValuePossible( mRasterDataType );
  mMaximumValue = maximumValuePossible( mRasterDataType );
  mRasterDataTypeRange = mMaximumValue - mMinimumValue;

  mLookupTableOffset = mMinimumValue * -1;

  mContrastEnhancementFunction.reset( new QgsContrastEnhancementFunction( mRasterDataType, mMinimumValue, mMaximumValue ) );

  //If the data type is larger than 16-bit do not generate a lookup table
  if ( mRasterDataTypeRange <= 65535.0 )
  {
    mLookupTable = new int[static_cast <int>( mRasterDataTypeRange + 1 )];
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
    mLookupTable = new int[static_cast <int>( mRasterDataTypeRange + 1 )];
  }
}

QgsContrastEnhancement::~QgsContrastEnhancement()
{
  delete [] mLookupTable;
}
/*
 *
 * Static methods
 *
 */

double QgsContrastEnhancement::maximumValuePossible( Qgis::DataType dataType )
{
  switch ( dataType )
  {
    case Qgis::Byte:
      return std::numeric_limits<unsigned char>::max();
    case Qgis::UInt16:
      return std::numeric_limits<unsigned short>::max();
    case Qgis::Int16:
      return std::numeric_limits<short>::max();
    case Qgis::UInt32:
      return std::numeric_limits<unsigned int>::max();
    case Qgis::Int32:
      return std::numeric_limits<int>::max();
    case Qgis::Float32:
      return std::numeric_limits<float>::max();
    case Qgis::Float64:
      return std::numeric_limits<double>::max();
    case Qgis::CInt16:
      return std::numeric_limits<short>::max();
    case Qgis::CInt32:
      return std::numeric_limits<int>::max();
    case Qgis::CFloat32:
      return std::numeric_limits<float>::max();
    case Qgis::CFloat64:
      return std::numeric_limits<double>::max();
    case Qgis::ARGB32:
    case Qgis::ARGB32_Premultiplied:
    case Qgis::UnknownDataType:
      // XXX - mloskot: not handled?
      break;
  }

  return std::numeric_limits<double>::max();
}

double QgsContrastEnhancement::minimumValuePossible( Qgis::DataType dataType )
{
  switch ( dataType )
  {
    case Qgis::Byte:
      return std::numeric_limits<unsigned char>::min();
    case Qgis::UInt16:
      return std::numeric_limits<unsigned short>::min();
    case Qgis::Int16:
      return std::numeric_limits<short>::min();
    case Qgis::UInt32:
      return std::numeric_limits<unsigned int>::min();
    case Qgis::Int32:
      return std::numeric_limits<int>::min();
    case Qgis::Float32:
      return std::numeric_limits<float>::max() * -1.0;
    case Qgis::Float64:
      return std::numeric_limits<double>::max() * -1.0;
    case Qgis::CInt16:
      return std::numeric_limits<short>::min();
    case Qgis::CInt32:
      return std::numeric_limits<int>::min();
    case Qgis::CFloat32:
      return std::numeric_limits<float>::max() * -1.0;
    case Qgis::CFloat64:
      return std::numeric_limits<double>::max() * -1.0;
    case Qgis::ARGB32:
    case Qgis::ARGB32_Premultiplied:
    case Qgis::UnknownDataType:
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

int QgsContrastEnhancement::enhanceContrast( double value )
{
  if ( mEnhancementDirty )
  {
    generateLookupTable();
  }

  if ( mLookupTable && NoEnhancement != mContrastEnhancementAlgorithm )
  {
    double shiftedValue = value + mLookupTableOffset;
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
  if ( Qgis::Byte != mRasterDataType && Qgis::UInt16 != mRasterDataType && Qgis::Int16 != mRasterDataType )
    return false;
  if ( !mLookupTable )
    return false;

  QgsDebugMsg( QStringLiteral( "building lookup table" ) );
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
      mContrastEnhancementFunction.reset( new QgsLinearMinMaxEnhancement( mRasterDataType, mMinimumValue, mMaximumValue ) );
      break;
    case StretchAndClipToMinimumMaximum :
      mContrastEnhancementFunction.reset( new QgsLinearMinMaxEnhancementWithClip( mRasterDataType, mMinimumValue, mMaximumValue ) );
      break;
    case ClipToMinimumMaximum :
      mContrastEnhancementFunction.reset( new QgsClipToMinMaxEnhancement( mRasterDataType, mMinimumValue, mMaximumValue ) );
      break;
    case UserDefinedEnhancement :
      //Do nothing
      break;
    default:
      mContrastEnhancementFunction.reset( new QgsContrastEnhancementFunction( mRasterDataType, mMinimumValue, mMaximumValue ) );
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
  QgsDebugMsgLevel( QStringLiteral( "called" ), 4 );

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
  QDomElement minElem = doc.createElement( QStringLiteral( "minValue" ) );
  QDomText minText = doc.createTextNode( QgsRasterBlock::printValue( mMinimumValue ) );
  minElem.appendChild( minText );
  parentElem.appendChild( minElem );

  //maximum value
  QDomElement maxElem = doc.createElement( QStringLiteral( "maxValue" ) );
  QDomText maxText = doc.createTextNode( QgsRasterBlock::printValue( mMaximumValue ) );
  maxElem.appendChild( maxText );
  parentElem.appendChild( maxElem );

  //algorithm
  QDomElement algorithmElem = doc.createElement( QStringLiteral( "algorithm" ) );
  QDomText algorithmText = doc.createTextNode( contrastEnhancementAlgorithmString( mContrastEnhancementAlgorithm ) );
  algorithmElem.appendChild( algorithmText );
  parentElem.appendChild( algorithmElem );
}

void QgsContrastEnhancement::readXml( const QDomElement &elem )
{
  QDomElement minValueElem = elem.firstChildElement( QStringLiteral( "minValue" ) );
  if ( !minValueElem.isNull() )
  {
    mMinimumValue = minValueElem.text().toDouble();
  }
  QDomElement maxValueElem = elem.firstChildElement( QStringLiteral( "maxValue" ) );
  if ( !maxValueElem.isNull() )
  {
    mMaximumValue = maxValueElem.text().toDouble();
  }
  QDomElement algorithmElem = elem.firstChildElement( QStringLiteral( "algorithm" ) );
  if ( !algorithmElem.isNull() )
  {
    QString algorithmString = algorithmElem.text();
    ContrastEnhancementAlgorithm algorithm = NoEnhancement;
    // old version ( < 19 Apr 2013) was using enum directly -> for backward compatibility
    if ( algorithmString == QLatin1String( "0" ) )
    {
      algorithm = NoEnhancement;
    }
    else if ( algorithmString == QLatin1String( "1" ) )
    {
      algorithm = StretchToMinimumMaximum;
    }
    else if ( algorithmString == QLatin1String( "2" ) )
    {
      algorithm = StretchAndClipToMinimumMaximum;
    }
    else if ( algorithmString == QLatin1String( "3" ) )
    {
      algorithm = ClipToMinimumMaximum;
    }
    else if ( algorithmString == QLatin1String( "4" ) )
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
      return QStringLiteral( "NoEnhancement" );
    case StretchToMinimumMaximum:
      return QStringLiteral( "StretchToMinimumMaximum" );
    case StretchAndClipToMinimumMaximum:
      return QStringLiteral( "StretchAndClipToMinimumMaximum" );
    case ClipToMinimumMaximum:
      return QStringLiteral( "ClipToMinimumMaximum" );
    case UserDefinedEnhancement:
      return QStringLiteral( "UserDefinedEnhancement" );
  }
  return QStringLiteral( "NoEnhancement" );
}

QgsContrastEnhancement::ContrastEnhancementAlgorithm QgsContrastEnhancement::contrastEnhancementAlgorithmFromString( const QString &contrastEnhancementString )
{
  if ( contrastEnhancementString == QLatin1String( "StretchToMinimumMaximum" ) )
  {
    return StretchToMinimumMaximum;
  }
  else if ( contrastEnhancementString == QLatin1String( "StretchAndClipToMinimumMaximum" ) )
  {
    return StretchAndClipToMinimumMaximum;
  }
  else if ( contrastEnhancementString == QLatin1String( "ClipToMinimumMaximum" ) )
  {
    return ClipToMinimumMaximum;
  }
  else if ( contrastEnhancementString == QLatin1String( "UserDefinedEnhancement" ) )
  {
    return UserDefinedEnhancement;
  }
  else
  {
    return NoEnhancement;
  }
}
