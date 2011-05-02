/***************************************************************************
                         qgsdiagramoverlay.h  -  description
                         -------------------
    begin                : January 2007
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

#ifndef QGSDIAGRAMOVERLAY_H
#define QGSDIAGRAMOVERLAY_H

#include "qgsvectoroverlay.h"

class QgsDiagramRenderer;

/**An overlay class that places diagrams on a vectorlayer*/
class QgsDiagramOverlay: public QgsVectorOverlay
{
  public:
    QgsDiagramOverlay( QgsVectorLayer* vl );
    ~QgsDiagramOverlay();
    void createOverlayObjects( const QgsRenderContext& renderContext );
    void drawOverlayObjects( QgsRenderContext& context ) const;

    //setters and getters
    QString typeName() const {return "diagram";}
    void setDiagramRenderer( QgsDiagramRenderer* r );
    const QgsDiagramRenderer* diagramRenderer() const;

    bool readXML( const QDomNode& overlayNode );
    bool writeXML( QDomNode& layer_node, QDomDocument& doc ) const;
    int createLegendContent( std::list<std::pair<QString, QImage*> >& content ) const;

    /**Helper function that returns the attribute index from an attribute name. Returns -1 in case of error*/
    static int indexFromAttributeName( const QString& name, const QgsVectorLayer* vl );
    /**Helper function that returns the attribute name from an attribute index. Returns an empty string
     in case of error.*/
    static QString attributeNameFromIndex( int index, const QgsVectorLayer* vl );


  protected:
    int getOverlayObjectSize( int& width, int& height, double value, const QgsFeature& f, const QgsRenderContext& renderContext ) const;

  private:
    /**Does the classification and manages the diagram generation*/
    QgsDiagramRenderer* mDiagramRenderer;
};

#endif
