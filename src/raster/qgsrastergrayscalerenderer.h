/***************************************************************************
   qgsrastergrayscalerenderer.h -  abstract base class for all raster renderers
                              -------------------
	begin                : Fri Jun 28 2002
	copyright            : (C) 2005 by T.Sutton
	email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgsrasterlayer.h 4380 2005-12-26 23:37:50Z timlinux $ */
#ifndef QGSRASTERGRAYSCALERENDERER_H
#define QGSRASTERGRAYSCALERENDERER_H

#include <qgsrasterrendererif.h>
#include <QPainter>
#include <qgsrasterviewport.h>
#include <qgsmaptopixel.h>
#include <qgsrasterlayer.h>
/** \file qgsrastergrayscalerenderer.h
 *  \brief This is a renderer for drawing single band grayscale images..
 */


class QgsRasterGrayscaleRenderer : public QgsRasterRendererIF
{
  QgsRasterGrayscaleRenderer(const * QgsRasterLayer);
  ~QgsRasterGrayscaleRenderer();
  
  Q_OBJECT;
public:
    /** \brief Drawing routine. 
     * This reimplements the method defined in the abstract base class. */
   void draw(QPainter * theQPainter, 
              const QgsRasterViewPort * theRasterViewPort,
              const QgsMapToPixel * theQgsMapToPixel,
              ) ;

};

#endif //QGSRASTERGRAYSCALERENDERER_H
