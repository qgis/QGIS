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
  : mMinimumValue( minimumValuePossible( dataType ) )
  , mMaximumValue( maximumValuePossible( dataType ) )
  , mRasterDataType( dataType )
  , mRasterDataTypeRange( mMaximumValue - mMinimumValue )
  , mLookupTableOffset( mMinimumValue * -1 )
{
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
  if ( Qgis::DataType::Byte != mRasterDataType && Qgis::DataType::UInt16 != mRasterDataType && Qgis::DataType::Int16 != mRasterDataType )
    return false;
  if ( !mLookupTable )
    return false;

  QgsDebugMsgLevel( QStringLiteral( "building lookup table" ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "***MinimumValue : %1" ).arg( mMinimumValue ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "***MaximumValue : %1" ).arg( mMaximumValue ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "***mLookupTableOffset : %1" ).arg( mLookupTableOffset ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "***mRasterDataTypeRange : %1" ).arg( mRasterDataTypeRange ), 4 );

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
  const QDomText minText = doc.createTextNode( QgsRasterBlock::printValue( mMinimumValue ) );
  minElem.appendChild( minText );
  parentElem.appendChild( minElem );

  //maximum value
  QDomElement maxElem = doc.createElement( QStringLiteral( "maxValue" ) );
  const QDomText maxText = doc.createTextNode( QgsRasterBlock::printValue( mMaximumValue ) );
  maxElem.appendChild( maxText );
  parentElem.appendChild( maxElem );

  //algorithm
  QDomElement algorithmElem = doc.createElement( QStringLiteral( "algorithm" ) );
  const QDomText algorithmText = doc.createTextNode( contrastEnhancementAlgorithmString( mContrastEnhancementAlgorithm ) );
  algorithmElem.appendChild( algorithmText );
  parentElem.appendChild( algorithmElem );
}

void QgsContrastEnhancement::readXml( const QDomElement &elem )
{
  const QDomElement minValueElem = elem.firstChildElement( QStringLiteral( "minValue" ) );
  if ( !minValueElem.isNull() )
  {
    mMinimumValue = minValueElem.text().toDouble();
  }
  const QDomElement maxValueElem = elem.firstChildElement( QStringLiteral( "maxValue" ) );
  if ( !maxValueElem.isNull() )
  {
    mMaximumValue = maxValueElem.text().toDouble();
  }
  const QDomElement algorithmElem = elem.firstChildElement( QStringLiteral( "algorithm" ) );
  if ( !algorithmElem.isNull() )
  {
    const QString algorithmString = algorithmElem.text();
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

void QgsContrastEnhancement::toSld( QDomDocument &doc, QDomElement &element ) const
{
  if ( doc.isNull() || element.isNull() )
    return;

  QString algName;
  switch ( contrastEnhancementAlgorithm() )
  {
    case StretchToMinimumMaximum:
      algName = QStringLiteral( "StretchToMinimumMaximum" );
      break;
    /* TODO: check if ClipToZero => StretchAndClipToMinimumMaximum
     * because value outside min/max ar considered as NoData instead of 0 */
    case StretchAndClipToMinimumMaximum:
      algName = QStringLiteral( "ClipToMinimumMaximum" );
      break;
    case ClipToMinimumMaximum:
      algName = QStringLiteral( "ClipToMinimumMaximum" );
      break;
    case NoEnhancement:
      return;
    case UserDefinedEnhancement:
      algName = contrastEnhancementAlgorithmString( contrastEnhancementAlgorithm() );
      QgsDebugMsg( QObject::tr( "No SLD1.0 conversion yet for stretch algorithm %1" ).arg( algName ) );
      return;
  }

  // Only <Normalize> is supported
  // minValue and maxValue are that values as set depending on "Min /Max value settings"
  // parameters
  QDomElement normalizeElem = doc.createElement( QStringLiteral( "sld:Normalize" ) );
  element.appendChild( normalizeElem );

  QDomElement vendorOptionAlgorithmElem = doc.createElement( QStringLiteral( "sld:VendorOption" ) );
  vendorOptionAlgorithmElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "algorithm" ) );
  vendorOptionAlgorithmElem.appendChild( doc.createTextNode( algName ) );
  normalizeElem.appendChild( vendorOptionAlgorithmElem );

  QDomElement vendorOptionMinValueElem = doc.createElement( QStringLiteral( "sld:VendorOption" ) );
  vendorOptionMinValueElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "minValue" ) );
  vendorOptionMinValueElem.appendChild( doc.createTextNode( QString::number( minimumValue() ) ) );
  normalizeElem.appendChild( vendorOptionMinValueElem );

  QDomElement vendorOptionMaxValueElem = doc.createElement( QStringLiteral( "sld:VendorOption" ) );
  vendorOptionMaxValueElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "maxValue" ) );
  vendorOptionMaxValueElem.appendChild( doc.createTextNode( QString::number( maximumValue() ) ) );
  normalizeElem.appendChild( vendorOptionMaxValueElem );
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
