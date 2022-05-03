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
#include "qgscoordinatenumericformat.h"
#include "qgsnumericformatregistry.h"
#include "qgsapplication.h"
#include "qgslocaldefaultsettings.h"
#include "qgsproject.h"

#include <QDomElement>

QgsProjectDisplaySettings::QgsProjectDisplaySettings( QObject *parent )
  : QObject( parent )
  , mBearingFormat( std::make_unique< QgsBearingNumericFormat >() )
  , mGeographicCoordinateFormat( std::make_unique< QgsGeographicCoordinateNumericFormat >() )
{

}

QgsProjectDisplaySettings::~QgsProjectDisplaySettings() = default;

void QgsProjectDisplaySettings::reset()
{
  // inherit local default settings
  mBearingFormat.reset( QgsLocalDefaultSettings::bearingFormat() );
  mGeographicCoordinateFormat.reset( QgsLocalDefaultSettings::geographicCoordinateFormat() );

  emit bearingFormatChanged();
  emit geographicCoordinateFormatChanged();
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

void QgsProjectDisplaySettings::setGeographicCoordinateFormat( QgsGeographicCoordinateNumericFormat *format )
{
  mGeographicCoordinateFormat.reset( format );
  emit geographicCoordinateFormatChanged();
}

const QgsGeographicCoordinateNumericFormat *QgsProjectDisplaySettings::geographicCoordinateFormat() const
{
  return mGeographicCoordinateFormat.get();
}

bool QgsProjectDisplaySettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  {
    const QDomElement bearingElement = element.firstChildElement( QStringLiteral( "BearingFormat" ) );
    mBearingFormat.reset( static_cast< QgsBearingNumericFormat * >( QgsApplication::numericFormatRegistry()->createFromXml( bearingElement, context ) ) );
    emit bearingFormatChanged();
  }

  {
    const QDomElement geographicElement = element.firstChildElement( QStringLiteral( "GeographicCoordinateFormat" ) );
    if ( !geographicElement.isNull() )
    {
      mGeographicCoordinateFormat.reset( static_cast< QgsGeographicCoordinateNumericFormat * >( QgsApplication::numericFormatRegistry()->createFromXml( geographicElement, context ) ) );
    }
    else if ( QgsProject *project = qobject_cast< QgsProject * >( parent() ) )
    {
      // upgrade old project setting
      bool ok = false;
      const QString format = project->readEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DegreeFormat" ), QString(), &ok );
      if ( ok )
      {
        mGeographicCoordinateFormat = std::make_unique< QgsGeographicCoordinateNumericFormat >();
        mGeographicCoordinateFormat->setShowDirectionalSuffix( true );
        if ( format == QLatin1String( "DM" ) )
          mGeographicCoordinateFormat->setAngleFormat( QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutes );
        else if ( format == QLatin1String( "DMS" ) )
          mGeographicCoordinateFormat->setAngleFormat( QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutesSeconds );
        else
          mGeographicCoordinateFormat->setAngleFormat( QgsGeographicCoordinateNumericFormat::AngleFormat::DecimalDegrees );
      }
      else
      {
        mGeographicCoordinateFormat.reset( QgsLocalDefaultSettings::geographicCoordinateFormat() );
      }
    }
    else
    {
      mGeographicCoordinateFormat.reset( QgsLocalDefaultSettings::geographicCoordinateFormat() );
    }
    emit geographicCoordinateFormatChanged();
  }

  return true;
}

QDomElement QgsProjectDisplaySettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectDisplaySettings" ) );

  {
    QDomElement bearingElement =  doc.createElement( QStringLiteral( "BearingFormat" ) );
    mBearingFormat->writeXml( bearingElement, doc, context );
    element.appendChild( bearingElement );
  }

  {
    QDomElement geographicElement =  doc.createElement( QStringLiteral( "GeographicCoordinateFormat" ) );
    mGeographicCoordinateFormat->writeXml( geographicElement, doc, context );
    element.appendChild( geographicElement );
  }

  return element;
}
