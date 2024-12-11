/***************************************************************************
                        qgsdxfexport.h
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

#include "qgswmsrequest.h"

namespace QgsWms
{

  /**
   * Output GetMap response in DXF format
   */
  void writeAsDxf( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response );

} // namespace QgsWms
