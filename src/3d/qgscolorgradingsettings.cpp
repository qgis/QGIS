/***************************************************************************
  qgscolorgradingsettings.cpp
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorgradingsettings.h"

#include "qgsreadwritecontext.h"

#include <QDomDocument>
#include <QString>

using namespace Qt::StringLiterals;

QgsColorGradingSettings::QgsColorGradingSettings( const QgsColorGradingSettings &other )
  : mExposureAdjustment( other.mExposureAdjustment )
  , mToneMapping( other.mToneMapping )
{}

QgsColorGradingSettings &QgsColorGradingSettings::operator=( QgsColorGradingSettings const &rhs )
{
  if ( &rhs == this )
    return *this;

  mExposureAdjustment = rhs.mExposureAdjustment;
  mToneMapping = rhs.mToneMapping;
  return *this;
}

void QgsColorGradingSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mExposureAdjustment = element.attribute( u"exposure"_s, u"0.0"_s ).toDouble();
  mToneMapping = qgsEnumKeyToValue( element.attribute( u"tone-mapping"_s ), Qgis::ToneMappingMethod::Clamp );
}

void QgsColorGradingSettings::writeXml( QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( u"exposure"_s, mExposureAdjustment );
  element.setAttribute( u"tone-mapping"_s, qgsEnumValueToKey( mToneMapping ) );
}
