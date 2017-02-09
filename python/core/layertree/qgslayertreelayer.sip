/**
 * Layer tree node points to a map layer.
 *
 * The node can exist also without a valid instance of a layer (just ID). That
 * means the referenced layer does not need to be loaded in order to use it
 * in layer tree. In such case, resolveReferences() method can be called
 * once the layer is loaded.
 *
 * A map layer is supposed to be present in one layer tree just once. It is
 * however possible that temporarily a layer exists in one tree more than just
 * once, e.g. while reordering items with drag and drop.
 *
 * @note added in 2.4
 */
class QgsLayerTreeLayer : QgsLayerTreeNode
{
%TypeHeaderCode
#include <qgslayertreelayer.h>
%End

  public:
    explicit QgsLayerTreeLayer( QgsMapLayer* layer );

    explicit QgsLayerTreeLayer( const QString& layerId, const QString& name = QString() );

    QString layerId() const;

    QgsMapLayer* layer() const;

    //! Get layer's name
    //! @note added in 3.0
    QString name() const;
    //! Set layer's name
    //! @note added in 3.0
    void setName( const QString& n );

    //! Read layer node from XML. Returns new instance.
    //! Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
    static QgsLayerTreeLayer* readXml( QDomElement& element ) /Factory/;
    //! Read layer node from XML. Returns new instance.
    //! Also resolves textual references to layers from the project (calls resolveReferences() internally).
    //! @note added in 3.0
    static QgsLayerTreeLayer* readXml( QDomElement& element, const QgsProject* project ) /Factory/;

    virtual void writeXml( QDomElement& parentElement );

    virtual QString dump() const;

    virtual QgsLayerTreeLayer* clone() const /Factory/;

    //! Resolves reference to layer from stored layer ID (if it has not been resolved already)
    //! @note added in 3.0
    virtual void resolveReferences( const QgsProject* project );

  signals:
    //! emitted when a previously unavailable layer got loaded
    void layerLoaded();
    //! emitted when a previously available layer got unloaded (from layer registry)
    //! @note added in 2.6
    void layerWillBeUnloaded();

  protected:
    void attachToLayer();

  private:
    QgsLayerTreeLayer( const QgsLayerTreeLayer& other );

};
