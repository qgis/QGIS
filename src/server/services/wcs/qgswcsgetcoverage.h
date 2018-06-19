/***************************************************************************
                              qgswcsgetcoverage.h
                              -------------------------
  begin                : January 16 , 2017
  copyright            : (C) 2013 by René-Luc D'Hont  ( parts from qgswcsserver )
                         (C) 2017 by David Marteau
  email                : rldhont at 3liz dot com
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
#ifndef QGSWCSGETCOVERAGE_H
#define QGSWCSGETCOVERAGE_H

#include <QByteArray>

namespace QgsWcs
{

  /**
   * Output WCS GetCoverage response
   */
  void writeGetCoverage( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                         const QgsServerRequest &request, QgsServerResponse &response );

  /**
   * Compute coverage data
   */
  QByteArray getCoverageData( QgsServerInterface *serverIface, const QgsProject *project, const QgsServerRequest &request );

} // namespace QgsWcs

#endif
