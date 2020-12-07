/***************************************************************************
                              qgsmediancut.h

  Median cut color reduction implementation
  -----------------------------------------
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
#ifndef QGSMEDIANCUT_H
#define QGSMEDIANCUT_H

#include <QVector>
#include <QImage>

/**
 * \ingroup server
 * Median cut implementation
 */

namespace QgsWms
{

  /**
   * Median cut implementation used when reducing RGB colors to palletized colors
   */
  void medianCut( QVector<QRgb> &colorTable, int nColors, const QImage &inputImage );

} // namespace QgsWms

#endif


