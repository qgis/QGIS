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

#include <QString>
#include <QPen>

#include "qgsrenderer.h"
#include "qgsfeature.h"
#include "osmstyle.h"


/**
 * Quantum GIS renderer for OpenStreetMap vector layers.
 */
class OsmRenderer : public QgsRenderer
{
  public:
    /**
     * Object construction.
     * @param geometryType supported geometry type; point, line or polygon
     * @param styleFileName name of file with OSM stylesheet
     */
    OsmRenderer( QGis::GeometryType geometryType, QString styleFileName );

    /**
     * Function parses concatenated tags from string to map.
     * @param tags concatenation of tags in form: "key1"="val1","key2"="val2","key3"="val3"
     * @return map of tag pairs from given concatenation
     */
    QMap<QString, QString> parse_tags( QString tags );

    /**
     * Determines if a feature will be rendered or not.
     * @param f feature object
     * @return true if a feature will be rendered; false otherwise; in this implementation function always returns True
     */
    bool willRenderFeature( QgsFeature *f );

    /**
     * Through calling of this function a vector layer passes features to a renderer object
     * to change the brush and pen of the qpainter.
     * @param p pointer to QPainter object that should be changed
     * @param f feature object
     * @param pic pointer to QImage object - image that will be displayed on map in place of the feature
     * @param selected true if feature is selected in qgis - ignored here (osm plugin uses its own way of feature identification and selection)
     * @param widthScale ignored here
     * @param rasterScaleFactor ignored here
     */
    void renderFeature( QgsRenderContext& renderContext, QgsFeature& f, QImage* pic, bool selected, double opacity = 1.0 );

    /**
     * Reads the renderer configuration from an XML file.
     * Not implemented!
     * @param rnode
     * @param vl
     */
    int readXML( const QDomNode &rnode, QgsVectorLayer &vl );

    /**
     * Writes the contents of the renderer to a configuration file.
     * Not implemented!
     * @param layer_node
     * @param document
     * @param vl
     * @return true in case of success.
     */
    bool writeXML( QDomNode &layer_node, QDomDocument &document, const QgsVectorLayer &vl ) const;

    /**
     * Returns true, if attribute values are used by the renderer and false otherwise.
     * In case of OSM renderer attributes are always used for feature rendering!
     * @return true anytime
     */
    bool needsAttributes() const;

    /**
     * Returns a list with indexes of classification attributes.
     * @return list with indexes of classification attributes
     */
    QgsAttributeList classificationAttributes() const;

    /**
     * Returns the name of this renderer.
     * @return the name of this renderer
     */
    QString name() const;

    /**
     * Return symbology items.
     * Not implemented!
     * @return symbology items
     */
    const QList< QgsSymbol * > symbols() const;

    /**
     * Returns a copy of the renderer (a deep copy on the heap).
     * Not implemented!
     * @return a copy of the renderer (a deep copy on the heap)
     */
    QgsRenderer *clone() const;

    /**
     * Returns true if this renderer returns a pixmap in the render method.
     * @return false anytime
     */
    bool containsPixmap() const;

    /**
     * Returns true if this renderer uses its own transparency settings.
     * @return false anytime
     */
    bool usesTransparency() const;

  protected:
    //! OsmStyle object with info on what type of object will be painted in what style.
    OsmStyle osmstyle;

    //! geometry type of features this renderer handles - points, lines or polygons.
    QGis::GeometryType mGeomType;
};

