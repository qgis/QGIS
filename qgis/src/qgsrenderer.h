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
/* $Id$ */
#ifndef QGSRENDERER_H
#define QGSRENDERER_H

class QgsFeature;
class QgsMapToPixel;
class QgsVectorLayer;
class QPainter;
class QgsDlgVectorLayerProperties;
class QPicture;
class QDomNode;
class QColor;

#include <fstream>
#include <list>
#include <qlistview.h>
#include <qstring.h>
#include <qdom.h>

#include "qgis.h"
#include "qgsproject.h"
#include <qcolor.h>

class QgsRenderItem;
class QgsSymbol;

/**Abstract base class for renderers. A renderer holds all the information necessary to draw the contents of a vector layer to a map canvas. The vector layer then passes each feature to paint to the renderer*/
class QgsRenderer
{
 public:
    /** Default ctor sets up selection colour from project properties */
    QgsRenderer();
    /**A vector layer passes features to a renderer object to change the brush and pen of the qpainter
     @param p the painter storing brush and pen
     @param f a pointer to the feature to be rendered
     @param pic pointer to a marker from SVG (is only used by marker renderers)
     @param scalefactor pointer to the scale factor for the marker image*/
    virtual void renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, double* scalefactor, bool selected, int oversampling = 1, double widthScale = 1.)=0;
    /**Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual void readXML(const QDomNode& rnode, QgsVectorLayer& vl)=0;
    /**Writes the contents of the renderer to a configuration file*/
    // virtual void writeXML(std::ostream& xml)=0;
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document )=0;
    /** Returns true, if attribute values are used by the renderer and false otherwise*/
    virtual bool needsAttributes()=0;
    /**Returns a list with indexes of classification attributes*/
    virtual std::list<int> classificationAttributes()=0;
    /**Returns the renderers name*/
    virtual QString name()=0;    
    /** Set up the selection color by reading approriate values from project props */
    void initialiseSelectionColor();
    /**Return symbology items*/
    virtual const std::list<QgsSymbol*> symbols() const=0;
    /**Color to draw selected features - static so we can change it in proj props and automatically 
       all renderers are updated*/
    static QColor mSelectionColor;
    /**Layer type*/
 protected:
    QGis::VectorType mVectorType;
    
};

inline void QgsRenderer::initialiseSelectionColor()
{
    int myRedInt = QgsProject::instance()->readNumEntry("Gui","/SelectionColorRedPart",255);
    int myGreenInt = QgsProject::instance()->readNumEntry("Gui","/SelectionColorGreenPart",255);
    int myBlueInt = QgsProject::instance()->readNumEntry("Gui","/SelectionColorBluePart",0);
    mSelectionColor = QColor(myRedInt,myGreenInt,myBlueInt);
}
#endif // QGSRENDERER_H
