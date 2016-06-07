class QgsRendererV2Widget /External/;

/**
 Stores metadata about one renderer class.

 @note It's necessary to implement createRenderer() function.
   In C++ you can use QgsRendererV2Metadata convenience class.
 */
class QgsRendererV2AbstractMetadata
{
%TypeHeaderCode
#include <qgsrendererv2registry.h>
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
    typedef QFlags<QgsRendererV2AbstractMetadata::LayerType> LayerTypes;

    QgsRendererV2AbstractMetadata( const QString& name, const QString& visibleName, const QIcon& icon = QIcon() );
    virtual ~QgsRendererV2AbstractMetadata();

    QString name() const;
    QString visibleName() const;

    QIcon icon() const;
    void setIcon( const QIcon& icon );

    /** Returns flags indicating the types of layer the renderer is compatible with.
     * @note added in QGIS 2.16
     */
    virtual QgsRendererV2AbstractMetadata::LayerTypes compatibleLayerTypes() const;

    /** Return new instance of the renderer given the DOM element. Returns NULL on error.
     * Pure virtual function: must be implemented in derived classes.  */
    virtual QgsFeatureRendererV2* createRenderer( QDomElement& elem ) = 0 /Factory/;
    /** Return new instance of settings widget for the renderer. Returns NULL on error.
     *
     * The \a oldRenderer argument may refer to previously used renderer (or it is null).
     * If not null, it may be used to initialize GUI of the widget from the previous settings.
     * The old renderer does not have to be of the same type as returned by createRenderer().
     * When using \a oldRenderer make sure to make a copy of it - it will be deleted afterwards.
     */
    virtual QgsRendererV2Widget* createRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* oldRenderer ) /Factory/;

    virtual QgsFeatureRendererV2* createRendererFromSld( QDomElement& elem, QGis::GeometryType geomType ) /Factory/;
};

QFlags<QgsRendererV2AbstractMetadata::LayerType> operator|(QgsRendererV2AbstractMetadata::LayerType f1, QFlags<QgsRendererV2AbstractMetadata::LayerType> f2);

/**
 Convenience metadata class that uses static functions to create renderer and its widget.
 */
class QgsRendererV2Metadata : QgsRendererV2AbstractMetadata
{
%TypeHeaderCode
#include <qgsrendererv2registry.h>
%End
  public:
    virtual QgsFeatureRendererV2* createRenderer( QDomElement& elem ) /Factory/;
    virtual QgsRendererV2Widget* createRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;
    virtual QgsFeatureRendererV2* createRendererFromSld( QDomElement& elem, QGis::GeometryType geomType ) /Factory/;

    // QgsRendererV2CreateFunc createFunction() const;
    // QgsRendererV2WidgetFunc widgetFunction() const;
    // QgsFeatureRendererV2* (*)( QDomElement&, QGis::GeometryType geomType ) createFromSldFunction() const;
    // void setWidgetFunction( QgsRendererV2WidgetFunc f );

    virtual QgsRendererV2AbstractMetadata::LayerTypes compatibleLayerTypes() const;

  private:
    QgsRendererV2Metadata(); // pretend this is private
};

/** \ingroup core
 * \class QgsRendererV2Registry
 * \brief Registry of renderers.
 *
 * This is a singleton, renderers can be added / removed at any time
 */

class QgsRendererV2Registry
{
%TypeHeaderCode
#include <qgsrendererv2registry.h>
%End

  public:

    //! Returns a pointer to the QgsRendererV2Registry singleton
    static QgsRendererV2Registry* instance();

    //! Adds a renderer to the registry. Takes ownership of the metadata object.
    //! @param metadata renderer metadata
    //! @returns true if renderer was added successfully, or false if renderer could not
    //! be added (eg a renderer with a duplicate name already exists)
    bool addRenderer( QgsRendererV2AbstractMetadata* metadata /Transfer/ );

    //! Removes a renderer from registry.
    //! @param rendererName name of renderer to remove from registry
    //! @returns true if renderer was sucessfully removed, or false if matching
    //! renderer could not be found
    bool removeRenderer( const QString& rendererName );

    //! Returns the metadata for a specified renderer. Returns NULL if a matching
    //! renderer was not found in the registry.
    QgsRendererV2AbstractMetadata* rendererMetadata( const QString& rendererName );

    //! Returns a list of available renderers.
    //! @param layerTypes flags to filter the renderers by compatible layer types
    QStringList renderersList( QgsRendererV2AbstractMetadata::LayerTypes layerTypes = QgsRendererV2AbstractMetadata::All ) const;

    //! Returns a list of available renderers which are compatible with a specified layer.
    //! @param layer vector layer
    //! @note added in QGIS 2.16
    QStringList renderersList( const QgsVectorLayer* layer ) const;

  protected:
    //! protected constructor
    QgsRendererV2Registry();
    ~QgsRendererV2Registry();

  private:
    QgsRendererV2Registry( const QgsRendererV2Registry& rh );
    //QgsRendererV2Registry& operator=( const QgsRendererV2Registry& rh );

};
