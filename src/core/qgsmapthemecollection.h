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
  \class QgsMapThemeCollection
  \ingroup core
  \brief Container class that allows storage of map themes consisting of visible
   map layers and layer styles.
  \note added in QGIS 2.12
*/

class CORE_EXPORT QgsMapThemeCollection : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QStringList mapThemes READ mapThemes NOTIFY mapThemesChanged )
    Q_PROPERTY( QgsProject* project READ project WRITE setProject NOTIFY projectChanged )

  public:

    /**
     * \ingroup core
     * Individual record of a visible layer in a map theme record.
     * @note Added in QGIS 3.0
     */
    class CORE_EXPORT MapThemeLayerRecord
    {
      public:
        //! Initialize layer record with a map layer - it will be stored as a weak pointer
        MapThemeLayerRecord( QgsMapLayer* l = nullptr ): usingCurrentStyle( false ), usingLegendItems( false ), mLayer( l ) {}

        bool operator==( const MapThemeLayerRecord& other ) const
        {
          return mLayer == other.mLayer &&
                 usingCurrentStyle == other.usingCurrentStyle && currentStyle == other.currentStyle &&
                 usingLegendItems == other.usingLegendItems && checkedLegendItems == other.checkedLegendItems;
        }
        bool operator!=( const MapThemeLayerRecord& other ) const
        {
          return !( *this == other );
        }

        //! Returns map layer or null if the layer does not exist anymore
        QgsMapLayer* layer() const { return mLayer; }

        //! Whether current style is valid and should be applied
        bool usingCurrentStyle;
        //! Name of the current style of the layer
        QString currentStyle;
        //! Whether checkedLegendItems should be applied
        bool usingLegendItems;
        //! Rule keys of check legend items in layer tree model
        QSet<QString> checkedLegendItems;
      private:
        //! Weak pointer to the layer
        QgsWeakMapLayerPointer mLayer;
    };

    /**
     * \ingroup core
     * Individual map theme record of visible layers and styles.
     *
     * @note Added in QGIS 3.0, Previously called PresetRecord
     */
    class CORE_EXPORT MapThemeRecord
    {
      public:

        bool operator==( const MapThemeRecord& other ) const
        {
          return validLayerRecords() == other.validLayerRecords();
        }
        bool operator!=( const MapThemeRecord& other ) const
        {
          return !( *this == other );
        }

        //! Returns a list of records for all visible layer belonging to the theme.
        QList<MapThemeLayerRecord> layerRecords() const { return mLayerRecords; }

        //! Sets layer records for the theme.
        void setLayerRecords( const QList<MapThemeLayerRecord>& records ) { mLayerRecords = records; }

        //! Return set with only records for valid layers
        //! @note not available in python bindings
        QHash<QgsMapLayer*, MapThemeLayerRecord> validLayerRecords() const;

      private:
        //! Layer-specific records for the theme. Only visible layers are listed.
        QList<MapThemeLayerRecord> mLayerRecords;

        friend class QgsMapThemeCollection;
    };

    /**
     * Create map theme collection that handles themes of the given project.
     */
    QgsMapThemeCollection( QgsProject* project = nullptr );

    /**
     * Returns whether a map theme with a matching name exists.
     * @note Added in QGIS 3.0
     */
    bool hasMapTheme( const QString& name ) const;

    /**
     * Inserts a new map theme to the collection.
     * @see update()
     */
    void insert( const QString& name, const MapThemeRecord& state );

    /**
     * Updates a map theme within the collection.
     * @param name name of map theme to update
     * @param state map theme record to replace existing map theme
     * @see insert()
     */
    void update( const QString& name, const MapThemeRecord& state );

    /**
     * Remove an existing map theme from collection.
     * @note Added in QGIS 3.0
     */
    void removeMapTheme( const QString& name );

    //! Remove all map themes from the collection.
    void clear();

    /**
     * Returns a list of existing map theme names.
     * @note Added in QGIS 3.0
     */
    QStringList mapThemes() const;

    /**
     * Returns the recorded state of a map theme.
     * @note Added in QGIS 3.0
     */
    MapThemeRecord mapThemeState( const QString& name ) const { return mMapThemes[name]; }

    /**
     * Returns the list of layer IDs that are visible for the specified map theme.
     *
     * @note The order of the returned list is not guaranteed to reflect the order of layers
     * in the canvas.
     * @note Added in QGIS 3.0
     */
    QStringList mapThemeVisibleLayerIds( const QString& name ) const;

    /**
     * Returns the list of layers that are visible for the specified map theme.
     *
     * @note The order of the returned list is not guaranteed to reflect the order of layers
     * in the canvas.
     * @note Added in QGIS 3.0
     */
    QList<QgsMapLayer*> mapThemeVisibleLayers( const QString& name ) const;

    /**
     * Get layer style overrides (for QgsMapSettings) of the visible layers for given map theme.
     * @note Added in QGIS 3.0
     */
    QMap<QString, QString> mapThemeStyleOverrides( const QString& name );

    /**
     * Reads the map theme collection state from XML
     * @param doc DOM document
     * @see writeXml
     */
    void readXml( const QDomDocument& doc );

    /** Writes the map theme collection state to XML.
     * @param doc DOM document
     * @see readXml
     */
    void writeXml( QDomDocument& doc );

    /**
     * Static method to create theme from the current state of layer visibilities in layer tree,
     * current style of layers and check state of legend items (from a layer tree model).
     * @note added in QGIS 3.0
     */
    static MapThemeRecord createThemeFromCurrentState( QgsLayerTreeGroup* root, QgsLayerTreeModel* model );

    /**
     * Apply theme given by its name and modify layer tree, current style of layers and checked
     * legend items of passed layer tree model.
     * @note added in QGIS 3.0
     */
    void applyTheme( const QString& name, QgsLayerTreeGroup* root, QgsLayerTreeModel* model );

    /**
     * The QgsProject on which this map theme collection works.
     *
     * \note Added in QGIS 3.0
     */
    QgsProject* project();

    /**
     * \copydoc project()
     * \note Added in QGIS 3.0
     */
    void setProject( QgsProject* project );

  signals:

    /**
     * Emitted when map themes within the collection are changed.
     * @note Added in QGIS 3.0
     */
    void mapThemesChanged();

    /**
     * Emitted when the project changes
     *
     * \copydoc project()
     * \note Added in QGIS 3.0
     */
    void projectChanged();

  private slots:

    /**
     * Handles updates of the map theme collection when layers are removed from the registry
     */
    void registryLayersRemoved( const QStringList& layerIDs );

    //! Update style name if a stored style gets renamed
    void layerStyleRenamed( const QString& oldName, const QString& newName );

  private:

    /**
     * Apply check states of legend nodes of a given layer as defined in the map theme.
     */
    void applyMapThemeCheckedLegendNodesToLayer( const MapThemeLayerRecord& layerRec, QgsMapLayer* layer );

    /**
     * Reconnects all map theme layers to handle style renames
     */
    void reconnectToLayersStyleManager();

    static bool findRecordForLayer( QgsMapLayer* layer, const MapThemeRecord& rec, MapThemeLayerRecord& layerRec );
    static MapThemeLayerRecord createThemeLayerRecord( QgsLayerTreeLayer* nodeLayer, QgsLayerTreeModel* model );
    static void createThemeFromCurrentState( QgsLayerTreeGroup* parent, QgsLayerTreeModel* model, MapThemeRecord& rec );
    static void applyThemeToLayer( QgsLayerTreeLayer* nodeLayer, QgsLayerTreeModel* model, const MapThemeRecord& rec );
    static void applyThemeToGroup( QgsLayerTreeGroup* parent, QgsLayerTreeModel* model, const MapThemeRecord& rec );

    typedef QMap<QString, MapThemeRecord> MapThemeRecordMap;
    MapThemeRecordMap mMapThemes;
    //! project used to retrieve layers from layer IDs
    QgsProject* mProject = nullptr;
};


#endif // QGSMAPTHEMECOLLECTION_H
