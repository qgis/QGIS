/***************************************************************************
                         qgssvgdiagramfactory.h  -  description
                         ----------------------
    begin                : November 2007
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

#ifndef QGSSVGDIAGRAMFACTORY_H
#define QGSSVGDIAGRAMFACTORY_H

#include "qgsdiagramfactory.h"
#include <QSvgRenderer>

class QgsSVGDiagramFactory: public QgsDiagramFactory
{
  public:
    QgsSVGDiagramFactory();
    ~QgsSVGDiagramFactory();

    /**Creates the diagram image for a feature in order to be placed on the map
    @param size diagram size (may be height, diameter, squaresize depending on diagram type
    @param f feature that is symbolized by the diagram
    @param renderContext rendering parameters*/
    QImage* createDiagram( int size, const QgsFeature& f, const QgsRenderContext& renderContext ) const;
    /**Creates the text/images for the legend items. The caller takes ownership of the generated
     image objects.
    @param size diagram size that should be represented in the legend
    @param renderContext rendering parameters
    @param value diagram value that should be represented in the legend
    @param items generated items
    @return 0 in case of success*/
    int createLegendContent( int size, const QgsRenderContext& renderContext, QString value, QMap<QString, QImage*>& items ) const
    { Q_UNUSED( size ); Q_UNUSED( renderContext ); Q_UNUSED( value ); Q_UNUSED( items ); return 1; } //later...

    /**Gets the width and height (in pixels) of the diagram image. Considers different width, height values, the maximum width of the drawing pen and the conversion from mm size to pixels according to render context.
    @param size diagram size calculated by diagram renderer
    @param f reference to the feature associated with the diagram
    @param the render context (contains mm scale factor and raster scale factor)
    @param width out: the width of the diagram image in pixels
    @param height out: the height of the diagram image in pixels*/
    int getDiagramDimensions( int size, const QgsFeature& f, const QgsRenderContext& context, int& width, int& height ) const;

    bool writeXML( QDomNode& overlay_node, QDomDocument& doc ) const;

    /**Sets the SVG data to be rendered.
     @return true in case of success*/
    bool setSVGData( const QByteArray& data, const QString& filePath = "" );

    QString svgFilePath() const {return mSvgFilePath;}

    /**Read settings from project file*/
    bool readXML( const QDomNode& factoryNode );

  private:
    mutable QSvgRenderer mRenderer;

    /**Path to the current svg file. Mainly for the purpose of inserting the path into a newly created dialog*/
    QString mSvgFilePath;
};

#endif
