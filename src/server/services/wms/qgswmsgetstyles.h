/***************************************************************************
                              qgswmsgetstyles.h
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2016 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

class QgsServerInterface;
class QgsProject;
class QgsWmsRequest;
class QgsServerResponse;
class QDomDocument;

namespace QgsWms
{

  /**
   * Output GetStyles response
   */
  void writeGetStyles( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response );


  /**
   * Returns an SLD file with the styles of the requested layers. Exception is raised in case of troubles :-)
   */
  QDomDocument getStyles( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request );

} // namespace QgsWms
