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
  : mNumericFormat( qgis::make_unique< QgsBasicNumericFormat >() )
{
}

QgsColorRampLegendNodeSettings::QgsColorRampLegendNodeSettings( const QgsColorRampLegendNodeSettings &other )
  : mMinimumLabel( other.mMinimumLabel )
  , mMaximumLabel( other.mMaximumLabel )
  , mDirection( other.mDirection )
  , mNumericFormat( other.numericFormat()->clone() )
{

}

QgsColorRampLegendNodeSettings &QgsColorRampLegendNodeSettings::operator=( const QgsColorRampLegendNodeSettings &other )
{
  mMinimumLabel = other.mMinimumLabel;
  mMaximumLabel = other.mMaximumLabel;
  mDirection = other.mDirection;
  mNumericFormat.reset( other.numericFormat()->clone() );
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

  settingsElement.setAttribute( QStringLiteral( "minimumLabel" ), mMinimumLabel );
  settingsElement.setAttribute( QStringLiteral( "maximumLabel" ), mMaximumLabel );
  settingsElement.setAttribute( QStringLiteral( "direction" ), static_cast< int >( mDirection ) );

  QDomElement numericFormatElem = doc.createElement( QStringLiteral( "numericFormat" ) );
  mNumericFormat->writeXml( numericFormatElem, doc, context );
  settingsElement.appendChild( numericFormatElem );

  element.appendChild( settingsElement );
}

void QgsColorRampLegendNodeSettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement settingsElement = element.firstChildElement( QStringLiteral( "rampLegendSettings" ) );
  if ( !settingsElement.isNull() )
  {
    mMinimumLabel = settingsElement.attribute( QStringLiteral( "minimumLabel" ) );
    mMaximumLabel = settingsElement.attribute( QStringLiteral( "maximumLabel" ) );
    mDirection = static_cast<  QgsColorRampLegendNodeSettings::Direction >( settingsElement.attribute( QStringLiteral( "direction" ) ).toInt() );

    QDomNodeList numericFormatNodeList = settingsElement.elementsByTagName( QStringLiteral( "numericFormat" ) );
    if ( !numericFormatNodeList.isEmpty() )
    {
      QDomElement numericFormatElem = numericFormatNodeList.at( 0 ).toElement();
      mNumericFormat.reset( QgsApplication::numericFormatRegistry()->createFromXml( numericFormatElem, context ) );
    }
  }
}
