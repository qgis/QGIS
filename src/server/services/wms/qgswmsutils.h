/***************************************************************************
                              qgswms.h

  Define WMS service utility functions
  ------------------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts fron qgswmshandler)
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
#include "qgswmsserviceexception.h"

class QgsRectangle;

/**
 * \ingroup server
 * WMS implementation
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
    JPEG
  };

  /**
   * Return the highest version supported by this implementation
   */
  QString ImplementationVersion();

  /**
   * Return WMS service URL
   */
  QUrl serviceUrl( const QgsServerRequest &request, const QgsProject *project );

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

  /**
   * Parse bbox parameter
   * \param bboxstr the bbox string as comma separated values
   * \returns QgsRectangle
   *
   * If the parsing fail then an empty bbox is returned
   */
  QgsRectangle parseBbox( const QString &bboxstr );

  /**
   * Reads the layers and style lists from the parameters LAYERS and STYLES
   */
  void readLayersAndStyles( const QgsServerRequest::Parameters &parameters, QStringList &layersList, QStringList &stylesList );

} // namespace QgsWms

#endif


