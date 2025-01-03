/***************************************************************************
                        qgspdfwriter.h
  -------------------------------------------------------------------
Date                 : 09 October 2023
Copyright            : (C) 2023
email                : marco.hugentobler at sourcepole dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPDFWRITER_H
#define QGSPDFWRITER_H

#include "qgswmsrequest.h"

namespace QgsWms
{

  /**
   * Output GetMap response in PDF format
   * \since QGIS 3.36
   */
  void writeAsPdf( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response );

} // namespace QgsWms

#endif // QGSPDFWRITER_H
