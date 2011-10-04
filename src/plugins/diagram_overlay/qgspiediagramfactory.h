/***************************************************************************
                         qgspiediagramfactory.h  -  description
                         ----------------------
    begin                : December 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPIEDIAGRAMFACTORY_H
#define QGSPIEDIAGRAMFACTORY_H

#include "qgswkndiagramfactory.h"
#include <QPainter>
#include <cmath>

/**A class that creates pie charts*/

class QgsPieDiagramFactory: public QgsWKNDiagramFactory
{
  public:
    QgsPieDiagramFactory();
    ~QgsPieDiagramFactory();

    /**Creates a diagram for a feature and a given size. The size is usually determined by QgsDiagramRenderer. The calling method takes ownership of the generated image*/
    QImage* createDiagram( int size, const QgsFeature& f, const QgsRenderContext& renderContext ) const;

    /**Creates items to show in the legend*/
    int createLegendContent( int size, const QgsRenderContext& renderContext, QString value, QMap<QString, QImage*>& items ) const
    { Q_UNUSED( size ); Q_UNUSED( renderContext ); Q_UNUSED( value ); Q_UNUSED( items ); return 1; } //soon

    /**Gets the width and height (in pixels) of the diagram image. Considers different width, height values, the maximum width of the drawing pen and the conversion from mm size to pixels according to render context.
    @param size diagram size calculated by diagram renderer (in mm)
    @param f reference to the feature associated with the diagram
    @param the render context (contains mm scale factor and raster scale factor)
    @param width out: the width of the diagram image in pixels
    @param height out: the height of the diagram image in pixels*/
    int getDiagramDimensions( int size, const QgsFeature& f, const QgsRenderContext& context, int& width, int& height ) const;

    /**Returns the property described by the size (e.g. diameter or height). This can be important to
     know if e.g. size has to be calculated proportional to pie area*/
    QgsDiagramFactory::SizeType sizeType() const {return QgsDiagramFactory::DIAMETER;}

  private:
    /**Calculates the x- and y-offsets for pie gaps
       @param gap the gap (in pixels)
       @param angle azimut of pie slice (mean azimut of both pie angles), 0 - 5760 (degrees * 16) with 0 in 3 o'clock position and counterclockwise direction
       @param xOffset out offset (in pixel) for x-direction
       @param yOffset out offset (in pixel) for y-direction
    @return 0 in case of success*/
    int gapOffsetsForPieSlice( int gap, int angle, int& xOffset, int& yOffset ) const;
};

#endif
