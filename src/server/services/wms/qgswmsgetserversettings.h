/***************************************************************************
                              qgswmsgetserversettings.h
                              -------------------------
  begin                : Nov 7, 2019
  copyright            : (C) 2019 by Jorge Gustavo Rocha
  email                : jgr at geomaster dot pt
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
   * Creates Json object with the available fonts
   */
  QJsonArray availableFonts();

  /**
   * Creates Json version of the about
   */
  QJsonObject about();

  /**
   * Output GetPrint response
   */
  void writeServerSettings( QgsServerInterface *serverIface, QgsServerResponse &response );

} // namespace QgsWms




