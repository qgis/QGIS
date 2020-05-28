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

namespace QgsWms
{

  /**
   * Output GetStyles response
   */
  void writeGetStyles( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                       const QgsServerRequest &request, QgsServerResponse &response );


  /**
   * Returns an SLD file with the styles of the requested layers. Exception is raised in case of troubles :-)
   */
  QDomDocument getStyles( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                          const QgsServerRequest &request );

  //GetStyle for compatibility with earlier QGIS versions

  /**
   * Output GetStyle response
   */
  void writeGetStyle( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                      const QgsServerRequest &request, QgsServerResponse &response );

  /**
   * Returns an SLD file with the style of the requested layer
   */
  QDomDocument getStyle( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                         const QgsServerRequest &request );



} // namespace QgsWms




