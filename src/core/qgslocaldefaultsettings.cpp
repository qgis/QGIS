/***************************************************************************
    qgslocaldefaultsettings.cpp
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

#include "qgslocaldefaultsettings.h"

#include <memory>

#include "qgis.h"
#include "qgsbearingnumericformat.h"
#include "qgscoordinatenumericformat.h"
#include "qgsreadwritecontext.h"

#include <QSettings>

void QgsLocalDefaultSettings::setBearingFormat( const QgsBearingNumericFormat *format )
{
  const QVariantMap config = format->configuration( QgsReadWriteContext() );

  QSettings s;
  s.beginGroup( u"defaults/bearing_format"_s );
  for ( auto it = config.constBegin(); it != config.constEnd(); ++it )
  {
    s.setValue( it.key(), it.value() );
  }
  s.endGroup();
}

QgsBearingNumericFormat *QgsLocalDefaultSettings::bearingFormat()
{
  QVariantMap config;
  QSettings s;
  s.beginGroup( u"defaults/bearing_format"_s );
  const QStringList keys = s.childKeys();
  for ( const QString &key : keys )
  {
    const QVariant value = s.value( key );
    config.insert( key, value );
  }
  s.endGroup();

  auto res = std::make_unique< QgsBearingNumericFormat >();
  res->setConfiguration( config, QgsReadWriteContext() );
  return res.release();
}

void QgsLocalDefaultSettings::setGeographicCoordinateFormat( const QgsGeographicCoordinateNumericFormat *format )
{
  const QVariantMap config = format->configuration( QgsReadWriteContext() );

  QSettings s;
  s.beginGroup( u"defaults/coordinate_format"_s );
  for ( auto it = config.constBegin(); it != config.constEnd(); ++it )
  {
    s.setValue( it.key(), it.value() );
  }
  s.endGroup();
}

QgsGeographicCoordinateNumericFormat *QgsLocalDefaultSettings::geographicCoordinateFormat()
{
  QVariantMap config;
  QSettings s;
  s.beginGroup( u"defaults/coordinate_format"_s );
  const QStringList keys = s.childKeys();
  for ( const QString &key : keys )
  {
    const QVariant value = s.value( key );
    config.insert( key, value );
  }
  s.endGroup();

  auto res = std::make_unique< QgsGeographicCoordinateNumericFormat >();
  res->setConfiguration( config, QgsReadWriteContext() );
  return res.release();
}
