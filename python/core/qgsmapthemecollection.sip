/**
  \class QgsMapThemeCollection
  \ingroup core
  \brief Container class that allows storage of map themes consisting of visible
   map layers and layer styles.
  \note Added in QGIS 2.12, renamed for QGIS 3.0
*/

class QgsMapThemeCollection : QObject
{
%TypeHeaderCode
#include <qgsmapthemecollection.h>
%End
  public:

    /** \ingroup core
     * Individual map theme record of visible layers and styles.
     */
    class MapThemeRecord
    {
      public:

        bool operator==( const QgsMapThemeCollection::MapThemeRecord& other ) const;
        bool operator!=( const QgsMapThemeCollection::MapThemeRecord& other ) const;

        /**
         * Ordered list of visible layers
         */
        QStringList visibleLayerIds() const;

        /**
         * Ordered list of visible layers
         */
        void setVisibleLayerIds( const QStringList& visibleLayerIds );

        /**
         * Lists which legend symbols are checked for layers which support this and where
         * not all symbols are checked.
         * @note not available in Python bindings
         */
//        QMap<QString, QSet<QString> > perLayerCheckedLegendSymbols() const;

        /**
         * Lists which legend symbols are checked for layers which support this and where
         * not all symbols are checked.
         * @note not available in Python bindings
         */
//        void setPerLayerCheckedLegendSymbols(const QMap<QString, QSet<QString> >& perLayerCheckedLegendSymbols);

        /**
         * The currently used style name for layers with multiple styles.
         * The map has layer ids as keys and style names as values.
         */
        QMap<QString, QString> perLayerCurrentStyle() const;

        /**
         * The currently used style name for layers with multiple styles.
         * The map has layer ids as keys and style names as values.
         */
        void setPerLayerCurrentStyle(const QMap<QString, QString>& perLayerCurrentStyle);

    };

    QgsMapThemeCollection( QgsProject* project );

    /**
     * Returns whether a map theme with a matching name exists.
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
     */
    void removeMapTheme( const QString& name );

    //! Remove all map themes from the collection.
    void clear();

    //! Returns a list of existing map theme names.
    QStringList mapThemes() const;

    /**
     * Returns the recorded state of a map theme.
     */
    MapThemeRecord mapThemeState( const QString& name ) const;

    /**
     * Returns the list of layer IDs that are visible for the specified map theme.
     *
     * @note The order of the returned list is not guaranteed to reflect the order of layers
     * in the canvas.
     */
    QStringList mapThemeVisibleLayers( const QString& name ) const;

    /**
     * Apply check states of legend nodes of a given layer as defined in the map theme.
     */
    void applyMapThemeCheckedLegendNodesToLayer( const QString& name, const QString& layerID );

    /**
     * Get layer style overrides (for QgsMapSettings) of the visible layers for given map theme.
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

    /** Emitted when map themes within the collection are changed.
     */
    void mapThemesChanged();
};

