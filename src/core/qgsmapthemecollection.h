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

#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>

class QDomDocument;
class QgsLayerTreeNode;
class QgsLayerTreeGroup;

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

  public:

    /**
     * \ingroup core
     * Individual map theme record of visible layers and styles.
     *
     * @note Added in QGIS 3.0, Previously called PresetRecord
     */
    class MapThemeRecord
    {
      public:

        bool operator==( const MapThemeRecord& other ) const
        {
          return mVisibleLayerIds.toSet() == other.mVisibleLayerIds.toSet()
                 && mPerLayerCheckedLegendSymbols == other.mPerLayerCheckedLegendSymbols
                 && mPerLayerCurrentStyle == other.mPerLayerCurrentStyle;
        }
        bool operator!=( const MapThemeRecord& other ) const
        {
          return !( *this == other );
        }

        /**
         * Ordered list of visible layers
         * @note Added in QGIS 3.0
         */
        QStringList visibleLayerIds() const;

        /**
         * Ordered list of visible layers
         * @note Added in QGIS 3.0
         */
        void setVisibleLayerIds( const QStringList& visibleLayerIds );

        /**
         * Lists which legend symbols are checked for layers which support this and where
         * not all symbols are checked.
         * @note not available in Python bindings
         * @note Added in QGIS 3.0
         */
        QMap<QString, QSet<QString> > perLayerCheckedLegendSymbols() const;

        /**
         * Lists which legend symbols are checked for layers which support this and where
         * not all symbols are checked.
         * @note not available in Python bindings
         * @note Added in QGIS 3.0
         */
        void setPerLayerCheckedLegendSymbols( const QMap<QString, QSet<QString> >& perLayerCheckedLegendSymbols );

        /**
         * The currently used style name for layers with multiple styles.
         * The map has layer ids as keys and style names as values.
         * @note Added in QGIS 3.0
         */
        QMap<QString, QString> perLayerCurrentStyle() const;

        /**
         * The currently used style name for layers with multiple styles.
         * The map has layer ids as keys and style names as values.
         * @note Added in QGIS 3.0
         */
        void setPerLayerCurrentStyle( const QMap<QString, QString>& perLayerCurrentStyle );

      private:
        QStringList mVisibleLayerIds;
        QMap<QString, QSet<QString> > mPerLayerCheckedLegendSymbols;
        QMap<QString, QString> mPerLayerCurrentStyle;

        friend class QgsMapThemeCollection;
    };

    QgsMapThemeCollection();

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
    MapThemeRecord mapThemeState( const QString& name ) const
    {
      return mMapThemes[name];
    }

    /**
     * Returns the list of layer IDs that are visible for the specified map theme.
     *
     * @note The order of the returned list is not guaranteed to reflect the order of layers
     * in the canvas.
     * @note Added in QGIS 3.0
     */
    QStringList mapThemeVisibleLayers( const QString& name ) const;

    /**
     * Apply check states of legend nodes of a given layer as defined in the map theme.
     * @note Added in QGIS 3.0
     */
    void applyMapThemeCheckedLegendNodesToLayer( const QString& name, const QString& layerID );

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
     * Static method for adding visible layers from a layer tree group to a map theme
     * record.
     * @param parent layer tree group parent
     * @param rec map theme record to amend
     */
    static void addVisibleLayersToMapTheme( QgsLayerTreeGroup* parent, MapThemeRecord& rec );

  signals:

    /**
     * Emitted when map themes within the collection are changed.
     * @note Added in QGIS 3.0
     */
    void mapThemesChanged();

  private slots:

    /**
     * Handles updates of the map theme collection when layers are removed from the registry
     */
    void registryLayersRemoved( const QStringList& layerIDs );

    //! Update style name if a stored style gets renamed
    void layerStyleRenamed( const QString& oldName, const QString& newName );

  private:

    /**
     * Reconnects all map theme layers to handle style renames
     */
    void reconnectToLayersStyleManager();

    typedef QMap<QString, MapThemeRecord> MapThemeRecordMap;
    MapThemeRecordMap mMapThemes;
};


#endif // QGSMAPTHEMECOLLECTION_H
