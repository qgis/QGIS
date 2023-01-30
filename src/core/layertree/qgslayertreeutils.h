/***************************************************************************
  qgslayertreeutils.h
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEUTILS_H
#define QGSLAYERTREEUTILS_H

#include <qnamespace.h>
#include <QList>
#include <QPair>
#include <QDomNodeList>
#include "qgis_core.h"

class QDomElement;
class QDomDocument;

class QgsLayerTreeNode;
class QgsLayerTreeGroup;
class QgsLayerTreeLayer;
class QgsMapLayer;
class QgsProject;

/**
 * \ingroup core
 * \brief Assorted functions for dealing with layer trees.
 *
 * \since QGIS 2.4
 */
class CORE_EXPORT QgsLayerTreeUtils
{
  public:

    //! Try to load layer tree from \verbatim <legend> \endverbatim tag from project files from QGIS 2.2 and below
    static bool readOldLegend( QgsLayerTreeGroup *root, const QDomElement &legendElem );
    //! Try to load custom layer order from \verbatim <legend> \endverbatim tag from project files from QGIS 2.2 and below
    static bool readOldLegendLayerOrder( const QDomElement &legendElem, bool &hasCustomOrder, QStringList &order );
    //! Returns \verbatim <legend> \endverbatim tag used in QGIS 2.2 and below
    static QDomElement writeOldLegend( QDomDocument &doc, QgsLayerTreeGroup *root, bool hasCustomOrder, const QList<QgsMapLayer *> &order );

    //! Convert Qt::CheckState to QString
    static QString checkStateToXml( Qt::CheckState state );
    //! Convert QString to Qt::CheckState
    static Qt::CheckState checkStateFromXml( const QString &txt );

    /**
     * Returns TRUE if any of the specified layers is editable.
     *
     * The \a ignoreLayersWhichCannotBeToggled argument can be used to control whether layers which cannot have their
     * edit states toggled by users should be ignored or not (since QGIS 3.22).
     */
    static bool layersEditable( const QList<QgsLayerTreeLayer *> &layerNodes, bool ignoreLayersWhichCannotBeToggled = false );

    //! Returns TRUE if any of the layers is modified
    static bool layersModified( const QList<QgsLayerTreeLayer *> &layerNodes );

    //! Removes layer nodes that refer to invalid layers
    static void removeInvalidLayers( QgsLayerTreeGroup *group );

    /**
     * Stores in a layer's originalXmlProperties the layer properties information
     * \since 3.6
     */
    static void storeOriginalLayersProperties( QgsLayerTreeGroup *group, const QDomDocument *doc );

    //! Remove subtree of embedded groups and replaces it with a custom property embedded-visible-layers
    static void replaceChildrenOfEmbeddedGroups( QgsLayerTreeGroup *group );

    /**
     * Updates an embedded \a group from a \a project.
     */
    static void updateEmbeddedGroupsProjectPath( QgsLayerTreeGroup *group, const QgsProject *project );

    //! Gets invisible layers
    static QStringList invisibleLayerList( QgsLayerTreeNode *node );

    //! Sets the expression filter of a legend layer
    static void setLegendFilterByExpression( QgsLayerTreeLayer &layer, const QString &expr, bool enabled = true );
    //! Returns the expression filter of a legend layer
    static QString legendFilterByExpression( const QgsLayerTreeLayer &layer, bool *enabled = nullptr );
    //! Test if one of the layers in a group has an expression filter
    static bool hasLegendFilterExpression( const QgsLayerTreeGroup &group );

    /**
     * Insert a QgsMapLayer just below another one
     * \param group the tree group where layers are (can be the root group)
     * \param refLayer the reference layer
     * \param layerToInsert the new layer to insert just below the reference layer
     * \returns the new tree layer
     */
    static QgsLayerTreeLayer *insertLayerBelow( QgsLayerTreeGroup *group, const QgsMapLayer *refLayer, QgsMapLayer *layerToInsert );

    /**
     * Returns map layers from the given list of layer tree nodes. Also recursively visits
     * child nodes of groups.
     * \since QGIS 3.4
     */
    static QSet<QgsMapLayer *> collectMapLayersRecursive( const QList<QgsLayerTreeNode *> &nodes );

    /**
     * Returns how many occurrences of a map layer are there in a layer tree.
     * In normal situations there is at most one occurrence, but sometimes there
     * may be temporarily more: for example, during drag&drop, upon drop a new layer
     * node is created while the original dragged node is still in the tree, resulting
     * in two occurrences.
     *
     * This is useful when deciding whether to start or stop listening to a signal
     * of a map layer within a layer tree and only connecting/disconnecting when
     * there is only one occurrence of that layer.
     * \since QGIS 3.4
     */
    static int countMapLayerInTree( QgsLayerTreeNode *tree, QgsMapLayer *layer );

    /**
     * Returns the first parent which doesn't have the given custom property
     * or the group itself if it doesn't hold the property
     * \param group the layer tree group
     * \param property the property
     * \since QGIS 3.8
     */
    static QgsLayerTreeGroup *firstGroupWithoutCustomProperty( QgsLayerTreeGroup *group, const QString &property );

    /**
     * Inserts a \a layer within a given \a group at an optimal index position by insuring a given layer
     * type will always sit on top of or below other types. From top to bottom, the stacking logic is
     * as follow:
     *
     * - vector points
     * - vector lines
     * - vector polygons
     * - point clouds
     * - meshes
     * - rasters
     * - base maps
     *
     * A base map is defined as a non-gdal provider raster layer (e.g. XYZ raster layer, vector tile layer, etc.)
     * \since QGIS 3.28
     */
    static QgsLayerTreeLayer *insertLayerAtOptimalPlacement( QgsLayerTreeGroup *group, QgsMapLayer *layer );
};

#endif // QGSLAYERTREEUTILS_H
