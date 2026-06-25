/***************************************************************************
  qgsfixedgradientbackgroundsettings.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfixedgradientbackgroundsettings.h"

#include "qgscolorutils.h"
#include "qgsreadwritecontext.h"

#include <QDomElement>
#include <QString>

using namespace Qt::StringLiterals;

QgsFixedGradientBackgroundSettings *QgsFixedGradientBackgroundSettings::clone() const
{
  return new QgsFixedGradientBackgroundSettings( *this );
}


void QgsFixedGradientBackgroundSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mTopColor = QgsColorUtils::colorFromString( element.attribute( u"top-color"_s ) );
  mBottomColor = QgsColorUtils::colorFromString( element.attribute( u"bottom-color"_s ) );
}

void QgsFixedGradientBackgroundSettings::writeXml( QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( u"top-color"_s, QgsColorUtils::colorToString( mTopColor ) );
  element.setAttribute( u"bottom-color"_s, QgsColorUtils::colorToString( mBottomColor ) );
}
