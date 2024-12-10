/***************************************************************************
                              qgswcsdescribecoverage.h
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
#ifndef QGSWCSDESCRIBECOVERAGE_H
#define QGSWCSDESCRIBECOVERAGE_H


#include <QDomDocument>

namespace QgsWcs
{

  /**
   * Create describe coverage document
   */
  QDomDocument createDescribeCoverageDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request );

  /**
   * Output WCS DescribeCoverage response
   */
  void writeDescribeCoverage( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response );

} // namespace QgsWcs

#endif
