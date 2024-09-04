/***************************************************************************
  qgsmapthemecollection.h
  --------------------------------------
  Date                 : September 2014
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

#ifndef QGSMAPTHEMECOLLECTION_H
#define QGSMAPTHEMECOLLECTION_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QStringList>

#include "qgsmaplayer.h"

class QDomDocument;
class QgsLayerTreeModel;
class QgsLayerTreeNode;
class QgsLayerTreeGroup;
class QgsLayerTreeLayer;
class QgsProject;

/**
 * \class QgsMapThemeCollection
 * \ingroup core
 * \brief Container class that allows storage of map themes consisting of visible
 *  map layers and layer styles.
*/

class CORE_EXPORT QgsMapThemeCollection : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QStringList mapThemes READ mapThemes NOTIFY mapThemesChanged )
    Q_PROPERTY( QgsProject *project READ project WRITE setProject NOTIFY projectChanged )

  public:

    /**
     * \ingroup core
     * \brief Individual record of a visible layer in a map theme record.
     */
    class CORE_EXPORT MapThemeLayerRecord
    {
      public:
        //! Initialize layer record with a map layer - it will be stored as a weak pointer
        MapThemeLayerRecord( QgsMapLayer *l = nullptr ): mLayer( l ) {}

        // TODO c++20 - replace with = default
        bool operator==( const QgsMapThemeCollection::MapThemeLayerRecord &other ) const
        {
          return mLayer == other.mLayer && isVisible == other.isVisible &&
                 usingCurrentStyle == other.usingCurrentStyle && currentStyle == other.currentStyle &&
                 usingLegendItems == other.usingLegendItems && checkedLegendItems == other.checkedLegendItems &&
                 expandedLegendItems == other.expandedLegendItems && expandedLayerNode == other.expandedLayerNode;
        }
        bool operator!=( const QgsMapThemeCollection::MapThemeLayerRecord &other ) const
        {
          return !( *this == other );
        }

        //! Returns map layer or NULLPTR if the layer does not exist anymore
        QgsMapLayer *layer() const { return mLayer; }

        //! Sets the map layer for this record
        void setLayer( QgsMapLayer *layer );

        /**
         * TRUE if the layer is visible in the associated theme.
         * \since QGIS 3.14
         */
        bool isVisible = true;

        //! Whether current style is valid and should be applied
        bool usingCurrentStyle = false;
        //! Name of the current style of the layer
        QString currentStyle;
        //! Whether checkedLegendItems should be applied
        bool usingLegendItems = false;
        //! Rule keys of check legend items in layer tree model
        QSet<QString> checkedLegendItems;

        /**
         * Rule keys of expanded legend items in layer tree view.
         * \since QGIS 3.2
         */
        QSet<QString> expandedLegendItems;

        /**
         * Whether the layer's tree node is expanded
         * (only to be applied if the parent MapThemeRecord has the information about expanded nodes stored)
         * \since QGIS 3.2
         */
        bool expandedLayerNode = false;
      private:
        //! Weak pointer to the layer
        QgsWeakMapLayerPointer mLayer;
    };

    /**
     * \ingroup core
     * \brief Individual map theme record of visible layers and styles.
     *
     */
    class CORE_EXPORT MapThemeRecord
    {
      public:

        bool operator==( const QgsMapThemeCollection::MapThemeRecord &other ) const
        {
          return validLayerRecords() == other.validLayerRecords() &&
                 mHasExpandedStateInfo == other.mHasExpandedStateInfo &&
                 mExpandedGroupNodes == other.mExpandedGroupNodes && mCheckedGroupNodes == other.mCheckedGroupNodes;
        }
        bool operator!=( const QgsMapThemeCollection::MapThemeRecord &other ) const
        {
          return !( *this == other );
        }

        //! Returns a list of records for all visible layer belonging to the theme.
        QList<QgsMapThemeCollection::MapThemeLayerRecord> layerRecords() const { return mLayerRecords; }

        //! Sets layer records for the theme.
        void setLayerRecords( const QList<QgsMapThemeCollection::MapThemeLayerRecord> &records ) { mLayerRecords = records; }

        //! Removes a record for \a layer if present.
        void removeLayerRecord( QgsMapLayer *layer );

        //! Add a new record for a layer.
        void addLayerRecord( const QgsMapThemeCollection::MapThemeLayerRecord &record );

        /**
         * Returns set with only records for valid layers
         */
        QHash<QgsMapLayer *, QgsMapThemeCollection::MapThemeLayerRecord> validLayerRecords() const SIP_SKIP;

        /**
         * Returns whether information about expanded/collapsed state of nodes has been recorded
         * and thus whether expandedGroupNodes() and expandedLegendItems + expandedLayerNode from layer records are valid.
         * \since QGIS 3.2
         */
        bool hasExpandedStateInfo() const { return mHasExpandedStateInfo; }

        /**
         * Returns whether information about checked/unchecked state of groups has been recorded
         * and thus whether checkedGroupNodes() is valid.
         * \since QGIS 3.10.1
         */
        bool hasCheckedStateInfo() const { return mHasCheckedStateInfo; };

        /**
         * Sets whether the map theme contains valid expanded/collapsed state of nodes
         * \since QGIS 3.2
         */
        void setHasExpandedStateInfo( bool hasInfo ) { mHasExpandedStateInfo = hasInfo; }

        /**
         * Sets whether the map theme contains valid checked/unchecked state of group nodes
         * \since QGIS 3.10.1
         */
        void setHasCheckedStateInfo( bool hasInfo ) { mHasCheckedStateInfo = hasInfo; };

        /**
         * Returns a set of group identifiers for group nodes that should have expanded state (other group nodes should be collapsed).
         * The returned value is valid only when hasExpandedStateInfo() returns TRUE.
         * Group identifiers are built using group names, a sub-group name is prepended by parent group's identifier
         * and a forward slash, e.g. "level1/level2"
         * \since QGIS 3.2
         */
        QSet<QString> expandedGroupNodes() const { return mExpandedGroupNodes; }

        /**
         * Returns a set of group identifiers for group nodes that should have checked state (other group nodes should be unchecked).
         * The returned value is valid only when hasCheckedStateInfo() returns TRUE.
         * Group identifiers are built using group names, a sub-group name is prepended by parent group's identifier
         * and a forward slash, e.g. "level1/level2"
         * \since QGIS 3.10.1
         */
        QSet<QString> checkedGroupNodes() const { return mCheckedGroupNodes; }

        /**
         * Sets a set of group identifiers for group nodes that should have expanded state. See expandedGroupNodes().
         * \since QGIS 3.2
         */
        void setExpandedGroupNodes( const QSet<QString> &expandedGroupNodes ) { mExpandedGroupNodes = expandedGroupNodes; }

        /**
         * Sets a set of group identifiers for group nodes that should have checked state. See checkedGroupNodes().
         * \since QGIS 3.10.1
         */
        void setCheckedGroupNodes( const QSet<QString> &checkedGroupNodes ) { mCheckedGroupNodes = checkedGroupNodes; }

      private:
        //! Layer-specific records for the theme. Only visible layers are listed.
        QList<MapThemeLayerRecord> mLayerRecords;

        //! Whether the information about expanded/collapsed state of groups, layers and legend items has been stored
        bool mHasExpandedStateInfo = false;
        //! Whether the information about checked/unchecked state of groups, layers and legend items has been stored
        bool mHasCheckedStateInfo = false;

        /**
         * Which groups should be expanded. Each group is identified by its name (sub-groups IDs are prepended with parent
         * group and forward slash - e.g. "level1/level2/level3").
         */
        QSet<QString> mExpandedGroupNodes;

        /**
         * Which groups should be checked. Each group is identified by its name (sub-groups IDs are prepended with parent
         * group and forward slash - e.g. "level1/level2/level3").
         */
        QSet<QString> mCheckedGroupNodes;

        friend class QgsMapThemeCollection;
    };

    /**
     * Create map theme collection that handles themes of the given project.
     */
    QgsMapThemeCollection( QgsProject *project = nullptr );

    /**
     * Returns whether a map theme with a matching name exists.
     */
    bool hasMapTheme( const QString &name ) const;

    /**
     * Inserts a new map theme to the collection.
     * \see update()
     */
    void insert( const QString &name, const QgsMapThemeCollection::MapThemeRecord &state );

    /**
     * Updates a map theme within the collection.
     * \param name name of map theme to update
     * \param state map theme record to replace existing map theme
     * \see insert()
     */
    void update( const QString &name, const QgsMapThemeCollection::MapThemeRecord &state );

    /**
     * Removes an existing map theme from collection.
     */
    void removeMapTheme( const QString &name );

    /**
     * Renames the existing map theme called \a name to \a newName.
     * Returns TRUE if the rename was successful, or FALSE if it failed (e.g. due to a duplicate name for \a newName).
     * \since QGIS 3.14
     */
    bool renameMapTheme( const QString &name, const QString &newName );

    //! Removes all map themes from the collection.
    void clear();

    /**
     * Returns a list of existing map theme names.
     */
    QStringList mapThemes() const;

    /**
     * Returns the recorded state of a map theme.
     */
    QgsMapThemeCollection::MapThemeRecord mapThemeState( const QString &name ) const { return mMapThemes[name]; }

    /**
     * Returns the list of layer IDs that are visible for the specified map theme.
     *
     * \note The order of the returned list is not guaranteed to reflect the order of layers
     * in the canvas.
     */
    QStringList mapThemeVisibleLayerIds( const QString &name ) const;

    /**
     * Returns the list of layers that are visible for the specified map theme.
     *
     * \note The order of the returned list is not guaranteed to reflect the order of layers
     * in the canvas.
     */
    QList<QgsMapLayer *> mapThemeVisibleLayers( const QString &name ) const;

    /**
     * Gets layer style overrides (for QgsMapSettings) of the visible layers for given map theme.
     */
    QMap<QString, QString> mapThemeStyleOverrides( const QString &name );

    /**
     * Reads the map theme collection state from XML
     * \param doc DOM document
     * \see writeXml
     */
    void readXml( const QDomDocument &doc );

    /**
     * Writes the map theme collection state to XML.
     * \param doc DOM document
     * \see readXml
     */
    void writeXml( QDomDocument &doc );

    /**
     * Static method to create theme from the current state of layer visibilities in layer tree,
     * current style of layers and check state of legend items (from a layer tree model).
     */
    static QgsMapThemeCollection::MapThemeRecord createThemeFromCurrentState( QgsLayerTreeGroup *root, QgsLayerTreeModel *model );

    /**
     * Apply theme given by its name and modify layer tree, current style of layers and checked
     * legend items of passed layer tree model.
     */
    void applyTheme( const QString &name, QgsLayerTreeGroup *root, QgsLayerTreeModel *model );

    /**
     * Returns the QgsProject on which this map theme collection works.
     *
     * \see setProject()
     * \see projectChanged()
     */
    QgsProject *project();

    /**
     * Sets the \a project on which this map theme collection works.
     *
     * \see project()
     * \see projectChanged()
     */
    void setProject( QgsProject *project );

    /**
     * Returns the master layer order (this will always match the project's QgsProject::layerOrder() ).
     * All map themes will maintain the same layer order as the master layer order.
     * \see masterVisibleLayers()
     */
    QList< QgsMapLayer * > masterLayerOrder() const;

    /**
     * Returns the master list of visible layers. The order of returned layers will always match those
     * of masterLayerOrder(), but the returned layers are filtered to only include those visible
     * in the project's layer tree.
     * \see masterLayerOrder()
     */
    QList< QgsMapLayer * > masterVisibleLayers() const;

  signals:

    /**
     * Emitted when map themes within the collection are changed.
     */
    void mapThemesChanged();

    /**
     * Emitted when a map theme changes definition.
     */
    void mapThemeChanged( const QString &theme );

    /**
     * Emitted when a map theme within the collection is renamed.
     * \since QGIS 3.14
     */
    void mapThemeRenamed( const QString &name, const QString &newName );

    /**
     * Emitted when the project changes
     *
     * \see project()
     * \see setProject()
     */
    void projectChanged();

  private slots:

    /**
     * Handles updates of the map theme collection when layers are removed from the registry
     */
    void registryLayersRemoved( const QStringList &layerIDs );

    //! Update style name if a stored style gets renamed
    void layerStyleRenamed( const QString &oldName, const QString &newName );

  private:

    /**
     * Apply check states of legend nodes of a given layer as defined in the map theme.
     */
    void applyMapThemeCheckedLegendNodesToLayer( const MapThemeLayerRecord &layerRec, QgsMapLayer *layer );

    /**
     * Reconnects all map theme layers to handle style renames
     */
    void reconnectToLayersStyleManager();

    static bool findRecordForLayer( QgsMapLayer *layer, const MapThemeRecord &rec, MapThemeLayerRecord &layerRec );
    static MapThemeLayerRecord createThemeLayerRecord( QgsLayerTreeLayer *nodeLayer, QgsLayerTreeModel *model );
    static void createThemeFromCurrentState( QgsLayerTreeGroup *parent, QgsLayerTreeModel *model, MapThemeRecord &rec );
    static void applyThemeToLayer( QgsLayerTreeLayer *nodeLayer, QgsLayerTreeModel *model, const MapThemeRecord &rec );
    static void applyThemeToGroup( QgsLayerTreeGroup *parent, QgsLayerTreeModel *model, const MapThemeRecord &rec );

    typedef QMap<QString, MapThemeRecord> MapThemeRecordMap;
    MapThemeRecordMap mMapThemes;
    //! project used to retrieve layers from layer IDs
    QgsProject *mProject = nullptr;
};


#endif // QGSMAPTHEMECOLLECTION_H
