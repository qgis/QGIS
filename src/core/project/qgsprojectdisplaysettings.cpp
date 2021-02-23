/***************************************************************************
    qgsprojectdisplaysettings.cpp
    -----------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectdisplaysettings.h"
#include "qgis.h"
#include "qgsbearingnumericformat.h"
#include "qgsnumericformatregistry.h"
#include "qgsapplication.h"
#include "qgslocaldefaultsettings.h"

#include <QDomElement>

QgsProjectDisplaySettings::QgsProjectDisplaySettings( QObject *parent )
  : QObject( parent )
  , mBearingFormat( std::make_unique< QgsBearingNumericFormat >() )
{

}

QgsProjectDisplaySettings::~QgsProjectDisplaySettings() = default;

void QgsProjectDisplaySettings::reset()
{
  // inherit local default settings
  mBearingFormat.reset( QgsLocalDefaultSettings::bearingFormat() );

  emit bearingFormatChanged();
}

void QgsProjectDisplaySettings::setBearingFormat( QgsBearingNumericFormat *format )
{
  mBearingFormat.reset( format );
  emit bearingFormatChanged();
}

const QgsBearingNumericFormat *QgsProjectDisplaySettings::bearingFormat() const
{
  return mBearingFormat.get();
}

bool QgsProjectDisplaySettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  QDomElement bearingElement = element.firstChildElement( QStringLiteral( "BearingFormat" ) );
  mBearingFormat.reset( static_cast< QgsBearingNumericFormat * >( QgsApplication::numericFormatRegistry()->createFromXml( bearingElement, context ) ) );
  emit bearingFormatChanged();

  return true;
}

QDomElement QgsProjectDisplaySettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectDisplaySettings" ) );

  QDomElement bearingElement =  doc.createElement( QStringLiteral( "BearingFormat" ) );
  mBearingFormat->writeXml( bearingElement, doc, context );
  element.appendChild( bearingElement );

  return element;
}
