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
#include "qgswmsconfigparser.h"
#include "qgswmsserviceexception.h"

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

  /** Return the highest version supported by this implementation
   */
  QString ImplementationVersion();

  /** Parse image format parameter
   *  @return OutputFormat
   */
  ImageOutputFormat parseImageFormat( const QString& format );

  /** Write image response
   */
  void writeImage( QgsServerResponse& response, QImage& img, const QString& formatStr,
                   int imageQuality = -1 );


  /**
   * Parse bbox paramater
   * @param bboxstr the bbox string as comma separated values
   * @return QgsRectangle
   *
   * If the parsing fail then an empty bbox is returned
   */
  QgsRectangle parseBbox( const QString& bboxstr );

  /**
   * Return the wms config parser (Transitional)
   *
   * XXX This is needed in the current implementation.
   * This should disappear as soon we get rid of singleton.
   */
  QgsWmsConfigParser* getConfigParser( QgsServerInterface* serverIface );

} // namespace QgsWms

#endif


