/***************************************************************************
                              qgsfssutils.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts fron qgswmshandler)
                         (C) 2014 by Alessandro Pasotti ( parts from qgswmshandler)
                         (C) 2017 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                  *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsutils.h"
#include "qgsconfigcache.h"
#include "qgsserverprojectutils.h"

namespace QgsWfs
{
  QString implementationVersion()
  {
    return QStringLiteral( "1.0.0" );
  }

  // Return the wms config parser (Transitional)
  QgsWfsProjectParser *getConfigParser( QgsServerInterface *serverIface )
  {
    QString configFilePath = serverIface->configFilePath();

    QgsWfsProjectParser *parser  = QgsConfigCache::instance()->wfsConfiguration( configFilePath, serverIface->accessControls() );
    if ( !parser )
    {
      throw QgsServiceException(
        QStringLiteral( "WFS configuration error" ),
        QStringLiteral( "There was an error reading the project file or the SLD configuration" ) );
    }
    return parser;
  }

  QString serviceUrl( const QgsServerRequest &request, const QgsProject *project )
  {
    QString href;
    if ( project )
    {
      href = QgsServerProjectUtils::wfsServiceUrl( *project );
    }

    // Build default url
    if ( href.isEmpty() )
    {
      QUrl url = request.url();
      QUrlQuery q( url );

      q.removeAllQueryItems( QStringLiteral( "REQUEST" ) );
      q.removeAllQueryItems( QStringLiteral( "VERSION" ) );
      q.removeAllQueryItems( QStringLiteral( "SERVICE" ) );
      q.removeAllQueryItems( QStringLiteral( "_DC" ) );

      url.setQuery( q );
      href = url.toString( QUrl::FullyDecoded );
    }

    return  href;
  }

} // namespace QgsWfs


