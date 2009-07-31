/***************************************************************************
    osmrenderer.h - Quantum GIS renderer for OpenStreetMap vector layers.
    ------------------
    begin                : April 2009
    copyright            : (C) 2009 by Lukas Berka
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFile>
#include <QString>
#include <sqlite3.h>
#include <iostream>

#include "qgsrenderer.h"
#include "qgsfeature.h"
#include "osmstyle.h"

using namespace std;


/**
 * Quantum GIS renderer for OpenStreetMap vector layers.
 */
class OsmRenderer : public QgsRenderer
{
  public:
    // Object construction
    OsmRenderer( QGis::GeometryType geometryType, QString styleFileName );
//    ~OsmRenderer();

    QMap<QString, QString> parse_tags( QString tags );

    // ??? Determines if a feature will be rendered or not.
    bool willRenderFeature( QgsFeature *f );

    // A vector layer passes features to a renderer object to change the brush and pen of the qpainter.
    void renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* pic, bool selected );

    // Reads the renderer configuration from an XML file.
    int readXML( const QDomNode &rnode, QgsVectorLayer &vl );

    // Writes the contents of the renderer to a configuration file @ return true in case of success.
    bool writeXML( QDomNode &layer_node, QDomDocument &document, const QgsVectorLayer &vl ) const;

    // Returns true, if attribute values are used by the renderer and false otherwise.
    bool needsAttributes() const;

    // Returns a list with indexes of classification attributes.
    QgsAttributeList classificationAttributes() const;

    // Returns the renderers name.
    QString name() const;

    // Return symbology items.
    const QList< QgsSymbol * > symbols() const;

    //Returns a copy of the renderer (a deep copy on the heap).
    QgsRenderer *clone() const;

    // ??? Returns true if this renderer returns a pixmap in the render method
    bool containsPixmap() const;

    // ??? Returns true if this renderer uses its own transparency settings
    bool usesTransparency() const;

  protected:
// member variables
    OsmStyle osmstyle;
    QGis::GeometryType mGeomType;
};

