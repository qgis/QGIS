/***************************************************************************
                              qgswcsutils.cpp
                              -------------------------
  begin                : December 9, 2013
  copyright            : (C) 2013 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                  *
 *                                                                         *
 ***************************************************************************/

#include "qgswcsutils.h"
#include "qgsconfigcache.h"

namespace QgsWcs
{
  QString implementationVersion()
  {
    return QStringLiteral( "1.0.0" );
  }

  // Return the wms config parser (Transitional)
  QgsWCSProjectParser* getConfigParser( QgsServerInterface* serverIface )
  {
    QString configFilePath = serverIface->configFilePath();

    QgsWCSProjectParser* parser  = QgsConfigCache::instance()->wcsConfiguration( configFilePath, serverIface->accessControls() );
    if ( !parser )
    {
      throw QgsServiceException(
        QStringLiteral( "WFS configuration error" ),
        QStringLiteral( "There was an error reading the project file or the SLD configuration" ) );
    }
    return parser;
  }

  QString serviceUrl( const QgsServerRequest& request, QgsWCSProjectParser* parser )
  {
    QString href;
    if ( parser )
    {
      href = parser->wcsServiceUrl();
      if ( href.isEmpty() )
      {
        href = parser->serviceUrl();
      }
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

  QgsRectangle parseBbox( const QString& bboxStr )
  {
    QStringList lst = bboxStr.split( QStringLiteral( "," ) );
    if ( lst.count() != 4 )
      return QgsRectangle();

    double d[4];
    bool ok;
    for ( int i = 0; i < 4; i++ )
    {
      lst[i].replace( QLatin1String( " " ), QLatin1String( "+" ) );
      d[i] = lst[i].toDouble( &ok );
      if ( !ok )
        return QgsRectangle();
    }
    return QgsRectangle( d[0], d[1], d[2], d[3] );
  }

} // namespace QgsWfs


