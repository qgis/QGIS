/***************************************************************************
                          qgslightsource.cpp
                          -----------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslightsource.h"
#include "qgspointlightsettings.h"
#include "qgsdirectionallightsettings.h"

QgsLightSource::~QgsLightSource() = default;

void QgsLightSource::resolveReferences( const QgsProject & )
{

}

QgsLightSource *QgsLightSource::createFromXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsLightSource > res;
  if ( element.nodeName() == QLatin1String( "point-light" ) )
  {
    res = std::make_unique< QgsPointLightSettings >();
  }
  else if ( element.nodeName() == QLatin1String( "directional-light" ) )
  {
    res = std::make_unique< QgsDirectionalLightSettings >();
  }

  if ( res )
    res->readXml( element, context );

  return res.release();
}
