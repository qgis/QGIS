/***************************************************************************
                         qgsrenderer.h  -  description
                             -------------------
    begin                : Sat Jan 4 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */
#ifndef QGSRENDERER_H
#define QGSRENDERER_H

class QgsFeature;
class QgsCoordinateTransform;
class QgsVectorLayer;
class QPainter;
class QgsDlgVectorLayerProperties;



/**Abstract base class for renderers. A renderer holds all the information necessary to draw the contents of a vector layer to a map canvas. The vector layer then passes each feature to paint to the renderer*/
class QgsRenderer
{
 public:
    /**Sets the initial symbology configuration for a layer. An instance of the corresponding renderer dialog is created and associated with the layer. Finally, a pixmap for the legend is drawn. If pr is not 0, the renderer dialog is copied to the pointer in the layer properties instead of directly to the vector layer and no image is rendered 
     @param layer the vector layer associated with the renderer
     @param pr the layer properties class involved
    */
    virtual void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0)=0;
    /**Renders a feature. A vector layer passes features to a renderer object for display*/
    virtual void renderFeature(QPainter* p, QgsFeature* f, QgsCoordinateTransform* t)=0;
    /** Returns true, if attribute values are used by the renderer and false otherwise*/
    virtual bool needsAttributes()=0;
};

#endif // QGSRENDERER_H
