class QgsRendererWidget /External/;

/**
 Stores metadata about one renderer class.

 @note It's necessary to implement createRenderer() function.
   In C++ you can use QgsRendererMetadata convenience class.
 */
class QgsRendererAbstractMetadata
{
%TypeHeaderCode
#include <qgsrendererregistry.h>
%End

  public:


    //! Layer types the renderer is compatible with
    //! @note added in QGIS 2.16
    enum LayerType
    {
      PointLayer, //!< Compatible with point layers
      LineLayer, //!< Compatible with line layers
      PolygonLayer, //!< Compatible with polygon layers
      All, //!< Compatible with all vector layers
    };
    typedef QFlags<QgsRendererAbstractMetadata::LayerType> LayerTypes;

    QgsRendererAbstractMetadata( const QString& name, const QString& visibleName, const QIcon& icon = QIcon() );
    virtual ~QgsRendererAbstractMetadata();

    QString name() const;
    QString visibleName() const;

    QIcon icon() const;
    void setIcon( const QIcon& icon );

    /** Returns flags indicating the types of layer the renderer is compatible with.
     * @note added in QGIS 2.16
     */
    virtual QgsRendererAbstractMetadata::LayerTypes compatibleLayerTypes() const;

    /** Return new instance of the renderer given the DOM element. Returns NULL on error.
     * Pure virtual function: must be implemented in derived classes.  */
    virtual QgsFeatureRenderer* createRenderer( QDomElement& elem ) = 0 /Factory/;
    /** Return new instance of settings widget for the renderer. Returns NULL on error.
     *
     * The \a oldRenderer argument may refer to previously used renderer (or it is null).
     * If not null, it may be used to initialize GUI of the widget from the previous settings.
     * The old renderer does not have to be of the same type as returned by createRenderer().
     * When using \a oldRenderer make sure to make a copy of it - it will be deleted afterwards.
     */
    virtual QgsRendererWidget* createRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* oldRenderer ) /Factory/;

    /**
     * Create a renderer from the SLD provided in ele and for the specified geometry type.
     * This is done on a best effort basis.
     */
    virtual QgsFeatureRenderer* createRendererFromSld( QDomElement& elem, QgsWkbTypes::GeometryType geomType ) /Factory/;
};

QFlags<QgsRendererAbstractMetadata::LayerType> operator|(QgsRendererAbstractMetadata::LayerType f1, QFlags<QgsRendererAbstractMetadata::LayerType> f2);

/**
 Convenience metadata class that uses static functions to create renderer and its widget.
 */
class QgsRendererMetadata : QgsRendererAbstractMetadata
{
%TypeHeaderCode
#include <qgsrendererregistry.h>
%End
  public:
    virtual QgsFeatureRenderer* createRenderer( QDomElement& elem ) /Factory/;
    virtual QgsRendererWidget* createRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer ) /Factory/;
    virtual QgsFeatureRenderer* createRendererFromSld( QDomElement& elem, QgsWkbTypes::GeometryType geomType ) /Factory/;

    // QgsRendererCreateFunc createFunction() const;
    // QgsRendererWidgetFunc widgetFunction() const;
    // QgsFeatureRenderer* (*)( QDomElement&, QgsWkbTypes::GeometryType geomType ) createFromSldFunction() const;
    // void setWidgetFunction( QgsRendererWidgetFunc f );

    virtual QgsRendererAbstractMetadata::LayerTypes compatibleLayerTypes() const;

  private:
    QgsRendererMetadata(); // pretend this is private
};

/** \ingroup core
 * \class QgsRendererRegistry
 * \brief Registry of renderers.
 *
 * This is a singleton, renderers can be added / removed at any time
 */

class QgsRendererRegistry
{
%TypeHeaderCode
#include <qgsrendererregistry.h>
%End

  public:

    //! Returns a pointer to the QgsRendererRegistry singleton
    static QgsRendererRegistry* instance();

    //! Adds a renderer to the registry. Takes ownership of the metadata object.
    //! @param metadata renderer metadata
    //! @returns true if renderer was added successfully, or false if renderer could not
    //! be added (eg a renderer with a duplicate name already exists)
    bool addRenderer( QgsRendererAbstractMetadata* metadata /Transfer/ );

    //! Removes a renderer from registry.
    //! @param rendererName name of renderer to remove from registry
    //! @returns true if renderer was sucessfully removed, or false if matching
    //! renderer could not be found
    bool removeRenderer( const QString& rendererName );

    //! Returns the metadata for a specified renderer. Returns NULL if a matching
    //! renderer was not found in the registry.
    QgsRendererAbstractMetadata* rendererMetadata( const QString& rendererName );

    //! Returns a list of available renderers.
    //! @param layerTypes flags to filter the renderers by compatible layer types
    QStringList renderersList( QgsRendererAbstractMetadata::LayerTypes layerTypes = QgsRendererAbstractMetadata::All ) const;

    //! Returns a list of available renderers which are compatible with a specified layer.
    //! @param layer vector layer
    //! @note added in QGIS 2.16
    QStringList renderersList( const QgsVectorLayer* layer ) const;

  protected:
    //! protected constructor
    QgsRendererRegistry();
    ~QgsRendererRegistry();

  private:
    QgsRendererRegistry( const QgsRendererRegistry& rh );
    //QgsRendererRegistry& operator=( const QgsRendererRegistry& rh );

};
