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
class QPicture;



/**Abstract base class for renderers. A renderer holds all the information necessary to draw the contents of a vector layer to a map canvas. The vector layer then passes each feature to paint to the renderer*/
class QgsRenderer
{
 public:
    /**Sets the initial symbology configuration for a layer. Besides of applying default symbology settings, an instance of the corresponding renderer dialog is created and associated with the layer (or with the property dialog, if pr is not 0). Finally, a pixmap for the legend is drawn (or, if pr is not 0, it is stored in the property dialog, until the settings are applied).
     @param layer the vector layer associated with the renderer
     @param pr the property dialog. This is only needed if the renderer is created from the property dialog and not yet associated with the vector layer, otherwise 0*/
    virtual void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0)=0;
    /**A vector layer passes features to a renderer object to change the brush and pen of the qpainter
     @param p the painter storing brush and pen
     @param f a pointer to the feature to be rendered
     @param pic pointer to a marker from SVG (is only used by marker renderers
     @param scalefactor pointer to the scale factor for the marker image*/
    virtual void renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, double* scalefactor)=0;
    /** Returns true, if attribute values are used by the renderer and false otherwise*/
    virtual bool needsAttributes()=0;
};

#endif // QGSRENDERER_H
