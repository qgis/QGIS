/***************************************************************************
    qgsprojectgpssettings.cpp
    ---------------------------
    begin                : November 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprojectgpssettings.h"

#include <QDomElement>

QgsProjectGpsSettings::QgsProjectGpsSettings( QObject *parent )
  : QObject( parent )
{

}

QgsProjectGpsSettings::~QgsProjectGpsSettings() = default;

void QgsProjectGpsSettings::reset()
{
  mAutoAddTrackVertices = false;
  mAutoCommitFeatures = false;

  emit automaticallyAddTrackVerticesChanged( false );
  emit automaticallyCommitFeaturesChanged( false );
}

bool QgsProjectGpsSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  mAutoAddTrackVertices = element.attribute( QStringLiteral( "autoAddTrackVertices" ), "0" ).toInt();
  mAutoCommitFeatures = element.attribute( QStringLiteral( "autoCommitFeatures" ), "0" ).toInt();

  emit automaticallyAddTrackVerticesChanged( mAutoAddTrackVertices );
  emit automaticallyCommitFeaturesChanged( mAutoCommitFeatures );
  return true;
}

QDomElement QgsProjectGpsSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement element = doc.createElement( QStringLiteral( "ProjectGpsSettings" ) );

  element.setAttribute( QStringLiteral( "autoAddTrackVertices" ),  mAutoAddTrackVertices ? 1 : 0 );
  element.setAttribute( QStringLiteral( "autoCommitFeatures" ),  mAutoCommitFeatures ? 1 : 0 );

  return element;
}

bool QgsProjectGpsSettings::automaticallyAddTrackVertices() const
{
  return mAutoAddTrackVertices;
}

bool QgsProjectGpsSettings::automaticallyCommitFeatures() const
{
  return mAutoCommitFeatures;
}

void QgsProjectGpsSettings::setAutomaticallyAddTrackVertices( bool enabled )
{
  if ( enabled == mAutoAddTrackVertices )
    return;

  mAutoAddTrackVertices = enabled;
  emit automaticallyAddTrackVerticesChanged( enabled );
}

void QgsProjectGpsSettings::setAutomaticallyCommitFeatures( bool enabled )
{
  if ( enabled == mAutoCommitFeatures )
    return;

  mAutoCommitFeatures = enabled;
  emit automaticallyCommitFeaturesChanged( enabled );
}
