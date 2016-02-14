/**
  \class QgsVisibilityPresetCollection
  \ingroup core
  \brief Container class that allows storage of visibility presets consisting of visible
   map layers and layer styles.
  \note added in QGIS 2.12
*/

class QgsVisibilityPresetCollection : QObject
{
%TypeHeaderCode
#include <qgsvisibilitypresetcollection.h>
%End
  public:

    /** Individual preset record of visible layers and styles.
     */
    class PresetRecord
    {
      public:

        bool operator==( const QgsVisibilityPresetCollection::PresetRecord& other ) const;
        bool operator!=( const QgsVisibilityPresetCollection::PresetRecord& other ) const;

        //! Ordered list of layers that are visible
        QStringList mVisibleLayerIDs;
        /** For layers that have checkable legend symbols and not all symbols are checked - list which ones are
         * @note not available in Python bindings
         */
        //QMap<QString, QSet<QString> > mPerLayerCheckedLegendSymbols;
        //! For layers that use multiple styles - which one is currently selected
        QMap<QString, QString> mPerLayerCurrentStyle;
    };

    QgsVisibilityPresetCollection();

    /** Returns whether a preset with a matching name exists.
     * @param name name of preset to check
     * @returns true if preset exists
     */
    bool hasPreset( const QString& name ) const;

    /** Inserts a new preset to the collection.
     * @param name name of preset
     * @param state preset record
     * @see update()
     */
    void insert( const QString& name, const PresetRecord& state );

    /** Updates a preset within the collection.
     * @param name name of preset to update
     * @param state preset record to replace existing preset
     * @see insert()
     */
    void update( const QString& name, const PresetRecord& state );

    /** Remove existing preset from collection.
     * @param name preset name
     */
    void removePreset( const QString& name );

    //! Remove all presets from the collection.
    void clear();

    //! Returns a list of existing preset names.
    QStringList presets() const;

    /** Returns the recorded state of a preset.
     * @param name name of preset
     */
    PresetRecord presetState( const QString& name ) const;

    /** Returns the list of layer IDs that should be visible for the specified preset.
     * @note The order of the returned list is not guaranteed to reflect the order of layers
     * in the canvas.
     * @param name preset name
     */
    QStringList presetVisibleLayers( const QString& name ) const;

    /** Apply check states of legend nodes of a given layer as defined in the preset.
     * @param name preset name
     * @param layerID layer ID
     */
    void applyPresetCheckedLegendNodesToLayer( const QString& name, const QString& layerID );

    /** Get layer style overrides (for QgsMapSettings) of the visible layers for given preset.
     * @param name preset name
     */
    QMap<QString, QString> presetStyleOverrides( const QString& name );

    /** Reads the preset collection state from XML
     * @param doc DOM document
     * @see writeXML
     */
    void readXML( const QDomDocument& doc );

    /** Writes the preset collection state to XML.
     * @param doc DOM document
     * @see readXML
     */
    void writeXML( QDomDocument& doc );

    /** Static method for adding visible layers from a layer tree group to a preset
     * record.
     * @param parent layer tree group parent
     * @param rec preset record to amend
     */
    static void addVisibleLayersToPreset( QgsLayerTreeGroup* parent, PresetRecord& rec );

  signals:

    /** Emitted when presets within the collection are changed.
     */
    void presetsChanged();

  protected slots:

    /** Handles updates of the preset collection when layers are removed from the registry
     */
    void registryLayersRemoved( const QStringList& layerIDs );

    //! Update style name if a stored style gets renamed
    void layerStyleRenamed( const QString& oldName, const QString& newName );

  protected:

    /** Reconnects all preset layers to handle style renames
     */
    void reconnectToLayersStyleManager();
};

