/***************************************************************************
  qgscolorramplegendnode.h
  --------------------------------------
  Date                 : December 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorramplegendnodesettings.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsbasicnumericformat.h"
#include "qgsnumericformat.h"
#include "qgsnumericformatregistry.h"

QgsColorRampLegendNodeSettings::QgsColorRampLegendNodeSettings()
  : mNumericFormat( std::make_unique< QgsBasicNumericFormat >() )
{
}

QgsColorRampLegendNodeSettings::QgsColorRampLegendNodeSettings( const QgsColorRampLegendNodeSettings &other )
//****** IMPORTANT! editing this? make sure you update the move constructor too! *****
  : mUseContinuousLegend( other.mUseContinuousLegend )
  , mMinimumLabel( other.mMinimumLabel )
  , mMaximumLabel( other.mMaximumLabel )
  , mPrefix( other.mPrefix )
  , mSuffix( other.mSuffix )
  , mDirection( other.mDirection )
  , mNumericFormat( other.numericFormat()->clone() )
  , mTextFormat( other.textFormat() )
  , mOrientation( other.mOrientation )
    //****** IMPORTANT! editing this? make sure you update the move constructor too! *****
{

}

QgsColorRampLegendNodeSettings::QgsColorRampLegendNodeSettings( QgsColorRampLegendNodeSettings &&other )
  : mUseContinuousLegend( other.mUseContinuousLegend )
  , mMinimumLabel( std::move( other.mMinimumLabel ) )
  , mMaximumLabel( std::move( other.mMaximumLabel ) )
  , mPrefix( std::move( other.mPrefix ) )
  , mSuffix( std::move( other.mSuffix ) )
  , mDirection( other.mDirection )
  , mNumericFormat( std::move( other.mNumericFormat ) )
  , mTextFormat( std::move( other.mTextFormat ) )
  , mOrientation( other.mOrientation )
{

}

QgsColorRampLegendNodeSettings &QgsColorRampLegendNodeSettings::operator=( const QgsColorRampLegendNodeSettings &other )
{
  if ( &other == this )
    return *this;

  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  mUseContinuousLegend = other.mUseContinuousLegend;
  mMinimumLabel = other.mMinimumLabel;
  mMaximumLabel = other.mMaximumLabel;
  mPrefix = other.mPrefix;
  mSuffix = other.mSuffix;
  mDirection = other.mDirection;
  mNumericFormat.reset( other.numericFormat()->clone() );
  mTextFormat = other.mTextFormat;
  mOrientation = other.mOrientation;
  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  return *this;
}

QgsColorRampLegendNodeSettings &QgsColorRampLegendNodeSettings::operator=( QgsColorRampLegendNodeSettings &&other )
{
  if ( &other == this )
    return *this;

  mUseContinuousLegend = other.mUseContinuousLegend;
  mMinimumLabel = std::move( other.mMinimumLabel );
  mMaximumLabel = std::move( other.mMaximumLabel );
  mPrefix = std::move( other.mPrefix );
  mSuffix = std::move( other.mSuffix );
  mDirection = other.mDirection;
  mNumericFormat = std::move( other.mNumericFormat );
  mTextFormat = std::move( other.mTextFormat );
  mOrientation = other.mOrientation;
  return *this;
}

QgsColorRampLegendNodeSettings::~QgsColorRampLegendNodeSettings() = default;

QgsColorRampLegendNodeSettings::Direction QgsColorRampLegendNodeSettings::direction() const
{
  return mDirection;
}

void QgsColorRampLegendNodeSettings::setDirection( QgsColorRampLegendNodeSettings::Direction direction )
{
  mDirection = direction;
}

QString QgsColorRampLegendNodeSettings::minimumLabel() const
{
  return mMinimumLabel;
}

void QgsColorRampLegendNodeSettings::setMinimumLabel( const QString &label )
{
  mMinimumLabel = label;
}

QString QgsColorRampLegendNodeSettings::maximumLabel() const
{
  return mMaximumLabel;
}

void QgsColorRampLegendNodeSettings::setMaximumLabel( const QString &label )
{
  mMaximumLabel = label;
}

const QgsNumericFormat *QgsColorRampLegendNodeSettings::numericFormat() const
{
  return mNumericFormat.get();
}

void QgsColorRampLegendNodeSettings::setNumericFormat( QgsNumericFormat *format )
{
  mNumericFormat.reset( format );
}

void QgsColorRampLegendNodeSettings::writeXml( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const
{
  QDomElement settingsElement = doc.createElement( u"rampLegendSettings"_s );

  settingsElement.setAttribute( u"useContinuousLegend"_s,  mUseContinuousLegend );
  settingsElement.setAttribute( u"minimumLabel"_s, mMinimumLabel );
  settingsElement.setAttribute( u"maximumLabel"_s, mMaximumLabel );
  settingsElement.setAttribute( u"prefix"_s, mPrefix );
  settingsElement.setAttribute( u"suffix"_s, mSuffix );
  settingsElement.setAttribute( u"direction"_s, static_cast< int >( mDirection ) );
  settingsElement.setAttribute( u"orientation"_s, static_cast< int >( mOrientation ) );

  QDomElement numericFormatElem = doc.createElement( u"numericFormat"_s );
  mNumericFormat->writeXml( numericFormatElem, doc, context );
  settingsElement.appendChild( numericFormatElem );

  if ( mTextFormat.isValid() )
  {
    settingsElement.appendChild( mTextFormat.writeXml( doc, context ) );
  }

  element.appendChild( settingsElement );
}

void QgsColorRampLegendNodeSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement settingsElement = element.firstChildElement( u"rampLegendSettings"_s );
  if ( !settingsElement.isNull() )
  {
    mUseContinuousLegend = settingsElement.attribute( u"useContinuousLegend"_s, u"1"_s ).toInt( );
    mMinimumLabel = settingsElement.attribute( u"minimumLabel"_s );
    mMaximumLabel = settingsElement.attribute( u"maximumLabel"_s );
    mPrefix = settingsElement.attribute( u"prefix"_s );
    mSuffix = settingsElement.attribute( u"suffix"_s );
    mDirection = static_cast<  QgsColorRampLegendNodeSettings::Direction >( settingsElement.attribute( u"direction"_s ).toInt() );
    mOrientation = static_cast<  Qt::Orientation >( settingsElement.attribute( u"orientation"_s, QString::number( Qt::Vertical ) ).toInt() );

    const QDomNodeList numericFormatNodeList = settingsElement.elementsByTagName( u"numericFormat"_s );
    if ( !numericFormatNodeList.isEmpty() )
    {
      const QDomElement numericFormatElem = numericFormatNodeList.at( 0 ).toElement();
      mNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElem, context ) );
    }

    if ( !settingsElement.firstChildElement( u"text-style"_s ).isNull() )
    {
      mTextFormat.readXml( settingsElement, context );
    }
    else
    {
      mTextFormat = QgsTextFormat();
    }
  }
}

QString QgsColorRampLegendNodeSettings::prefix() const
{
  return mPrefix;
}

void QgsColorRampLegendNodeSettings::setPrefix( const QString &prefix )
{
  mPrefix = prefix;
}

QString QgsColorRampLegendNodeSettings::suffix() const
{
  return mSuffix;
}

void QgsColorRampLegendNodeSettings::setSuffix( const QString &suffix )
{
  mSuffix = suffix;
}

QgsTextFormat QgsColorRampLegendNodeSettings::textFormat() const
{
  return mTextFormat;
}

void QgsColorRampLegendNodeSettings::setTextFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

Qt::Orientation QgsColorRampLegendNodeSettings::orientation() const
{
  return mOrientation;
}

void QgsColorRampLegendNodeSettings::setOrientation( Qt::Orientation orientation )
{
  mOrientation = orientation;
}

bool QgsColorRampLegendNodeSettings::useContinuousLegend() const
{
  return mUseContinuousLegend;
}

void QgsColorRampLegendNodeSettings::setUseContinuousLegend( bool useContinuousLegend )
{
  mUseContinuousLegend = useContinuousLegend;
}
