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
#include "qgsnumericformat.h"
#include "qgsbasicnumericformat.h"
#include "qgsapplication.h"
#include "qgsnumericformatregistry.h"
#include "qgis.h"

QgsColorRampLegendNodeSettings::QgsColorRampLegendNodeSettings()
  : mNumericFormat( std::make_unique< QgsBasicNumericFormat >() )
{
}

QgsColorRampLegendNodeSettings::QgsColorRampLegendNodeSettings( const QgsColorRampLegendNodeSettings &other )
  : mUseContinuousLegend( other.mUseContinuousLegend )
  , mMinimumLabel( other.mMinimumLabel )
  , mMaximumLabel( other.mMaximumLabel )
  , mPrefix( other.mPrefix )
  , mSuffix( other.mSuffix )
  , mDirection( other.mDirection )
  , mNumericFormat( other.numericFormat()->clone() )
  , mTextFormat( other.textFormat() )
  , mOrientation( other.mOrientation )
{

}

QgsColorRampLegendNodeSettings &QgsColorRampLegendNodeSettings::operator=( const QgsColorRampLegendNodeSettings &other )
{
  mUseContinuousLegend = other.mUseContinuousLegend;
  mMinimumLabel = other.mMinimumLabel;
  mMaximumLabel = other.mMaximumLabel;
  mPrefix = other.mPrefix;
  mSuffix = other.mSuffix;
  mDirection = other.mDirection;
  mNumericFormat.reset( other.numericFormat()->clone() );
  mTextFormat = other.mTextFormat;
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
  QDomElement settingsElement = doc.createElement( QStringLiteral( "rampLegendSettings" ) );

  settingsElement.setAttribute( QStringLiteral( "useContinuousLegend" ),  mUseContinuousLegend );
  settingsElement.setAttribute( QStringLiteral( "minimumLabel" ), mMinimumLabel );
  settingsElement.setAttribute( QStringLiteral( "maximumLabel" ), mMaximumLabel );
  settingsElement.setAttribute( QStringLiteral( "prefix" ), mPrefix );
  settingsElement.setAttribute( QStringLiteral( "suffix" ), mSuffix );
  settingsElement.setAttribute( QStringLiteral( "direction" ), static_cast< int >( mDirection ) );
  settingsElement.setAttribute( QStringLiteral( "orientation" ), static_cast< int >( mOrientation ) );

  QDomElement numericFormatElem = doc.createElement( QStringLiteral( "numericFormat" ) );
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
  const QDomElement settingsElement = element.firstChildElement( QStringLiteral( "rampLegendSettings" ) );
  if ( !settingsElement.isNull() )
  {
    mUseContinuousLegend = settingsElement.attribute( QStringLiteral( "useContinuousLegend" ), QStringLiteral( "1" ) ).toInt( );
    mMinimumLabel = settingsElement.attribute( QStringLiteral( "minimumLabel" ) );
    mMaximumLabel = settingsElement.attribute( QStringLiteral( "maximumLabel" ) );
    mPrefix = settingsElement.attribute( QStringLiteral( "prefix" ) );
    mSuffix = settingsElement.attribute( QStringLiteral( "suffix" ) );
    mDirection = static_cast<  QgsColorRampLegendNodeSettings::Direction >( settingsElement.attribute( QStringLiteral( "direction" ) ).toInt() );
    mOrientation = static_cast<  Qt::Orientation >( settingsElement.attribute( QStringLiteral( "orientation" ), QString::number( Qt::Vertical ) ).toInt() );

    const QDomNodeList numericFormatNodeList = settingsElement.elementsByTagName( QStringLiteral( "numericFormat" ) );
    if ( !numericFormatNodeList.isEmpty() )
    {
      const QDomElement numericFormatElem = numericFormatNodeList.at( 0 ).toElement();
      mNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElem, context ) );
    }

    if ( !settingsElement.firstChildElement( QStringLiteral( "text-style" ) ).isNull() )
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
