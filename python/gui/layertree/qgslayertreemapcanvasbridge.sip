/**
 * The QgsLayerTreeMapCanvasBridge class takes care of updates of layer set
 * for QgsMapCanvas from a layer tree. The class listens to the updates in the layer tree
 * and updates the list of layers for rendering whenever some layers are added, removed,
 * or their visibility changes.
 *
 * The update of layers is not done immediately - it is postponed, so a series of updates
 * to the layer tree will trigger just one update of canvas layers.
 *
 * Also allows the client to override the default order of layers. This is useful
 * in advanced cases where the grouping in layer tree should be independent from the actual
 * order in the canvas.
 *
 * @added in 2.4
 */
class QgsLayerTreeMapCanvasBridge : QObject
{
%TypeHeaderCode
#include <qgslayertreemapcanvasbridge.h>
%End

  public:
    //! Constructor: does not take ownership of the layer tree nor canvas
    QgsLayerTreeMapCanvasBridge( QgsLayerTreeGroup* root, QgsMapCanvas* canvas, QObject* parent /TransferThis/ = 0 );

    void clear();

    QgsLayerTreeGroup* rootGroup() const;
    QgsMapCanvas* mapCanvas() const;

    bool hasCustomLayerOrder() const;
    QStringList customLayerOrder() const;

    QStringList defaultLayerOrder() const;

    //! if enabled, will automatically set full canvas extent and destination CRS + map units
    //! when first layer(s) are added
    void setAutoSetupOnFirstLayer( bool enabled );
    bool autoSetupOnFirstLayer() const;

    //! if enabled, will automatically turn on on-the-fly reprojection of layers if a layer
    //! with different source CRS is added
    void setAutoEnableCrsTransform( bool enabled );
    bool autoEnableCrsTransform() const;

  public slots:
    void setHasCustomLayerOrder( bool override );
    void setCustomLayerOrder( const QStringList& order );

    //! force update of canvas layers from the layer tree. Normally this should not be needed to be called.
    void setCanvasLayers();

    void readProject( const QDomDocument& doc );
    void writeProject( QDomDocument& doc );

  signals:
    void hasCustomLayerOrderChanged( bool );
    void customLayerOrderChanged( const QStringList& order );

  protected:

    void defaultLayerOrder( QgsLayerTreeNode* node, QStringList& order ) const;

    void setCanvasLayers( QgsLayerTreeNode* node, QList<QgsMapCanvasLayer>& layers );

    void deferredSetCanvasLayers();

  protected slots:
    void nodeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeRemovedChildren();
    void nodeVisibilityChanged();
    void nodeCustomPropertyChanged( QgsLayerTreeNode* node, QString key );
};
