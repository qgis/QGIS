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

    /**
     * \ingroup core
     * Individual record of a visible layer in a map theme record.
     * @note Added in QGIS 3.0
     */
    class MapThemeLayerRecord
    {
      public:
        //! Initialize layer record with a map layer - it will be stored as a weak pointer
        MapThemeLayerRecord( QgsMapLayer* l = nullptr );

        bool operator==( const QgsMapThemeCollection::MapThemeLayerRecord& other ) const;
        bool operator!=( const QgsMapThemeCollection::MapThemeLayerRecord& other ) const;

        QgsMapLayer* layer() const;


        void setLayer( QgsMapLayer* layer );


        bool usingCurrentStyle;

        QString currentStyle;

        bool usingLegendItems;

        QSet<QString> checkedLegendItems;
    };

    /** \ingroup core
     * Individual map theme record of visible layers and styles.
     */
    class MapThemeRecord
    {
      public:

        bool operator==( const QgsMapThemeCollection::MapThemeRecord& other ) const;
        bool operator!=( const QgsMapThemeCollection::MapThemeRecord& other ) const;

        //! Returns a list of records for all visible layer belonging to the theme.
        QList<QgsMapThemeCollection::MapThemeLayerRecord> layerRecords() const;

        //! Sets layer records for the theme.
        void setLayerRecords( const QList<QgsMapThemeCollection::MapThemeLayerRecord>& records );
        void removeLayerRecord( QgsMapLayer* layer );


        void addLayerRecord( const QgsMapThemeCollection::MapThemeLayerRecord& record );

        //! Return set with only records for valid layers
        //! @note not available in python bindings
        // QHash<QgsMapLayer*, QgsMapThemeCollection::MapThemeLayerRecord> validLayerRecords() const;
    };

    /**
     * Create map theme collection that handles themes of the given project.
     */
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
    QgsProject* project();
    void setProject( QgsProject* project );
  signals:

    void mapThemesChanged();

    void mapThemeChanged( const QString &theme );

    void projectChanged();
};

