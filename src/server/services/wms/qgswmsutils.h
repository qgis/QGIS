/***************************************************************************
                              qgswms.h

  Define WMS service utility functions
  ------------------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts from qgswmshandler)
                         (C) 2014 by Alessandro Pasotti ( parts from qgswmshandler)
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
#ifndef QGSWMSUTILS_H
#define QGSWMSUTILS_H

#include "qgsmodule.h"
#include "qgsserversettings.h"

class QgsRectangle;

/**
 * \ingroup server
 * \brief WMS implementation
 */

//! WMS implementation
namespace QgsWms
{
  //! Supported image output format
  enum ImageOutputFormat
  {
    UNKN,
    PNG,
    PNG8,
    PNG16,
    PNG1,
    JPEG,
    WEBP
  };

  /**
   * Returns WMS service URL
   */
  QUrl serviceUrl( const QgsServerRequest &request, const QgsProject *project, const QgsServerSettings &settings );

  /**
   * Parse image format parameter
   *  \returns OutputFormat
   */
  ImageOutputFormat parseImageFormat( const QString &format );

  /**
   * Write image response
   */
  void writeImage( QgsServerResponse &response, QImage &img, const QString &formatStr,
                   int imageQuality = -1 );
} // namespace QgsWms

#endif


