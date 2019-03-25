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
