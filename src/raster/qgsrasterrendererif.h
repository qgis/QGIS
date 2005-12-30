/***************************************************************************
   qgsrasterrendererif.h  -  abstract base class for all raster renderers
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
#ifndef QGSRASTERRENDERERIF_H
#define QGSRASTERRENDERERIF_H
class QPainter;
class QgsRasterViewPort;
class QgsMapToPixel;
class QgsRasterLayer;
/** \file qgsrasterrendererif.h
 *  \brief This interface provides the abstract base class that all renderers
 *  must implement.
 */


class QgsRasterRendererIF 
{
  virtual ~QgsRasterRendererIF();
  
public:
    /** \brief Drawing routine. 
     * This is a pure virtual method - all subclasses must reimplement*/
    virtual void draw(QPainter * theQPainter, 
              const QgsRasterViewPort * theRasterViewPort,
              const QgsMapToPixel * theQgsMapToPixel
              ) =0;
protected:
    /** get the raster layer member (only available to derived classes */
    //QgsRasterLayer * getRaster() { return mpQgsRasterLayer; };
private:
    /** Typically interfaces dont have data members (Meyer pp 145)
     *  but Im'm using one here anyway.... */
    //QgsRasterLayer * mpQgsRasterLayer;

}

#endif //QGSRASTERRENDERERIF_H
