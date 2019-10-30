/***************************************************************************
    qgsprojectviewsettings.cpp
    -----------------------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectviewsettings.h"
#include "qgis.h"
#include <QDomElement>

QgsProjectViewSettings::QgsProjectViewSettings( QObject *parent )
  : QObject( parent )
{

}

void QgsProjectViewSettings::reset()
{
  if ( mUseProjectScales || !mMapScales.empty() )
  {
    mUseProjectScales = false;
    mMapScales.clear();
    emit mapScalesChanged();
  }
}

void QgsProjectViewSettings::setMapScales( const QVector<double> &scales )
{
  // sort scales in descending order
  QVector< double > sorted = scales;
  std::sort( sorted.begin(), sorted.end(), std::greater<double>() );

  if ( sorted == mapScales() )
    return;

  mMapScales = sorted;

  emit mapScalesChanged();
}

QVector<double> QgsProjectViewSettings::mapScales() const
{
  return mMapScales;
}

void QgsProjectViewSettings::setUseProjectScales( bool enabled )
{
  if ( enabled == useProjectScales() )
    return;

  mUseProjectScales = enabled;
  emit mapScalesChanged();
}

bool QgsProjectViewSettings::useProjectScales() const
{
  return mUseProjectScales;
}

bool QgsProjectViewSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  bool useProjectScale = element.attribute( QStringLiteral( "UseProjectScales" ), QStringLiteral( "0" ) ).toInt();

  QDomNodeList scalesNodes = element.elementsByTagName( QStringLiteral( "Scales" ) );
  QVector< double > newScales;
  if ( !scalesNodes.isEmpty() )
  {
    QDomElement scalesElement = scalesNodes.at( 0 ).toElement();
    QDomNodeList scalesNodes = scalesElement.elementsByTagName( QStringLiteral( "Scale" ) );
    for ( int i = 0; i < scalesNodes.count(); i++ )
    {
      QDomElement scaleElement = scalesNodes.at( i ).toElement();
      newScales.append( scaleElement.attribute( QStringLiteral( "Value" ) ).toDouble() );
    }
  }
  if ( useProjectScale != mUseProjectScales || newScales != mMapScales )
  {
    mMapScales = newScales;
    mUseProjectScales = useProjectScale;
    emit mapScalesChanged();
  }

  return true;
}

QDomElement QgsProjectViewSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectViewSettings" ) );
  element.setAttribute( QStringLiteral( "UseProjectScales" ), mUseProjectScales ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  QDomElement scales = doc.createElement( QStringLiteral( "Scales" ) );
  for ( double scale : mMapScales )
  {
    QDomElement scaleElement = doc.createElement( QStringLiteral( "Scale" ) );
    scaleElement.setAttribute( QStringLiteral( "Value" ), qgsDoubleToString( scale ) );
    scales.appendChild( scaleElement );
  }
  element.appendChild( scales );
  return element;
}
