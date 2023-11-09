/***************************************************************************
                         qgsmeshsimplificationsettings.cpp
                         ---------------------
    begin                : February 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshsimplificationsettings.h"

bool QgsMeshSimplificationSettings::isEnabled() const
{
  return mEnabled;
}

void QgsMeshSimplificationSettings::setEnabled( bool active )
{
  mEnabled = active;
}

double QgsMeshSimplificationSettings::reductionFactor() const
{
  return mReductionFactor;
}

void QgsMeshSimplificationSettings::setReductionFactor( double value )
{
  mReductionFactor = value;
}

QDomElement QgsMeshSimplificationSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  QDomElement elem = doc.createElement( QStringLiteral( "mesh-simplify-settings" ) );
  elem.setAttribute( QStringLiteral( "enabled" ), mEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.setAttribute( QStringLiteral( "reduction-factor" ), mReductionFactor );
  elem.setAttribute( QStringLiteral( "mesh-resolution" ), mMeshResolution );
  return elem;
}

void QgsMeshSimplificationSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mEnabled = elem.attribute( QStringLiteral( "enabled" ) ).toInt();
  mReductionFactor = elem.attribute( QStringLiteral( "reduction-factor" ) ).toDouble();
  mMeshResolution = elem.attribute( QStringLiteral( "mesh-resolution" ) ).toInt();
}

int QgsMeshSimplificationSettings::meshResolution() const
{
  return mMeshResolution;
}

void QgsMeshSimplificationSettings::setMeshResolution( int meshResolution )
{
  mMeshResolution = meshResolution;
}
