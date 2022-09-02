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
#include "qgssettings.h"

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

  mCoordinateType = Qgis::CoordinateDisplayType::MapCrs;
  mCoordinateCustomCrs = QgsCoordinateReferenceSystem( "EPSG:4326" );

  emit bearingFormatChanged();
  emit geographicCoordinateFormatChanged();
  emit coordinateTypeChanged();
  emit coordinateCustomCrsChanged();
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

void QgsProjectDisplaySettings::setCoordinateType( Qgis::CoordinateDisplayType type )
{
  if ( mCoordinateType == type )
    return;

  mCoordinateType = type;

  emit coordinateTypeChanged();
}

void QgsProjectDisplaySettings::setCoordinateCustomCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( mCoordinateCustomCrs == crs )
    return;

  mCoordinateCustomCrs = crs;

  emit coordinateCustomCrsChanged();
}

bool QgsProjectDisplaySettings::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  {
    const QDomElement bearingElement = element.firstChildElement( QStringLiteral( "BearingFormat" ) );
    mBearingFormat.reset( static_cast< QgsBearingNumericFormat * >( QgsApplication::numericFormatRegistry()->createFromXml( bearingElement, context ) ) );
    emit bearingFormatChanged();
  }

  QgsProject *project = qobject_cast< QgsProject * >( parent() );

  {
    const QDomElement geographicElement = element.firstChildElement( QStringLiteral( "GeographicCoordinateFormat" ) );
    if ( !geographicElement.isNull() )
    {
      mGeographicCoordinateFormat.reset( static_cast< QgsGeographicCoordinateNumericFormat * >( QgsApplication::numericFormatRegistry()->createFromXml( geographicElement, context ) ) );
    }
    else if ( project )
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

  {
    if ( element.hasAttribute( QStringLiteral( "CoordinateType" ) ) )
    {
      mCoordinateType = static_cast<Qgis::CoordinateDisplayType>( element.attribute( QStringLiteral( "CoordinateType" ) ).toInt() );
    }
    else if ( project )
    {
      const QString format = project->readEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DegreeFormat" ), QString() );
      if ( !format.isEmpty() )
      {
        if ( format != QStringLiteral( "MU" ) && !project->crs().isGeographic() )
        {
          mCoordinateType = Qgis::CoordinateDisplayType::CustomCrs;
        }
        else
        {
          mCoordinateType = Qgis::CoordinateDisplayType::MapCrs;
        }
      }
    }
    emit coordinateTypeChanged();

    QDomNodeList crsNodeList = element.elementsByTagName( QStringLiteral( "CoordinateCustomCrs" ) );
    if ( !crsNodeList.isEmpty() )
    {
      QDomElement crsElem = crsNodeList.at( 0 ).toElement();
      mCoordinateCustomCrs.readXml( crsElem );
    }
    emit coordinateCustomCrsChanged();
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

  element.setAttribute( QStringLiteral( "CoordinateType" ), static_cast<int>( mCoordinateType ) );
  if ( mCoordinateCustomCrs.isValid() )
  {
    QDomElement crsElem = doc.createElement( QStringLiteral( "CoordinateCustomCrs" ) );
    mCoordinateCustomCrs.writeXml( crsElem, doc );
    element.appendChild( crsElem );
  }

  return element;
}
