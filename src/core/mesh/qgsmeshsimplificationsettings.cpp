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
  QDomElement elem = doc.createElement( u"mesh-simplify-settings"_s );
  elem.setAttribute( u"enabled"_s, mEnabled ? u"1"_s : u"0"_s );
  elem.setAttribute( u"reduction-factor"_s, mReductionFactor );
  elem.setAttribute( u"mesh-resolution"_s, mMeshResolution );
  return elem;
}

void QgsMeshSimplificationSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mEnabled = elem.attribute( u"enabled"_s ).toInt();
  mReductionFactor = elem.attribute( u"reduction-factor"_s ).toDouble();
  mMeshResolution = elem.attribute( u"mesh-resolution"_s ).toInt();
}

int QgsMeshSimplificationSettings::meshResolution() const
{
  return mMeshResolution;
}

void QgsMeshSimplificationSettings::setMeshResolution( int meshResolution )
{
  mMeshResolution = meshResolution;
}
