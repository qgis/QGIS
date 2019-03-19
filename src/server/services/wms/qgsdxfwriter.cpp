/***************************************************************************
                        qgsdxfexport.cpp
  -------------------------------------------------------------------
Date                 : 20 December 2016
Copyright            : (C) 2015 by
email                : marco.hugentobler at sourcepole dot com (original code)
Copyright            : (C) 2016 by
email                : david dot marteau at 3liz dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmodule.h"
#include "qgsdxfwriter.h"
#include "qgsdxfexport.h"
#include "qgswmsrenderer.h"

namespace QgsWms
{

  namespace
  {

    QMap<QString, QString> parseFormatOptions( const QString &optionString )
    {
      QMap<QString, QString> options;

      QStringList optionsList = optionString.split( ';' );
      for ( auto optionsIt = optionsList.constBegin(); optionsIt != optionsList.constEnd(); ++optionsIt )
      {
        int equalIdx = optionsIt->indexOf( ':' );
        if ( equalIdx > 0 && equalIdx < ( optionsIt->length() - 1 ) )
        {
          options.insert( optionsIt->left( equalIdx ).toUpper(),
                          optionsIt->right( optionsIt->length() - equalIdx - 1 ).toUpper() );
        }
      }
      return options;
    }

  }

  void writeAsDxf( QgsServerInterface *serverIface, const QgsProject *project,
                   const QString &version,  const QgsServerRequest &request,
                   QgsServerResponse &response )
  {
    Q_UNUSED( version );

    QgsWmsParameters wmsParameters( QUrlQuery( request.url() ) );
    QgsRenderer renderer( serverIface, project, wmsParameters );

    // Write output
    QgsDxfExport dxf = renderer.getDxf();
    response.setHeader( "Content-Type", "application/dxf" );
    dxf.writeToFile( response.io(), wmsParameters.dxfCodec() );
  }
} // namespace QgsWms
