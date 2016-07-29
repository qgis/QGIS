/**
 * The QgsCustomLayerOrderWidget class provides a list box where the user can define
 * custom order for drawing of layers. It also features a checkbox for enabling
 * or disabling the custom order. Any changes made by the user are automatically
 * propagated to the assigned QgsLayerTreeMapCanvasBridge. Also, any updates
 * to the layer tree cause refresh of the list.
 *
 * @see QgsLayerTreeMapCanvasBridge
 * @note added in 2.4
 */
class QgsCustomLayerOrderWidget : QWidget
{
%TypeHeaderCode
#include <qgscustomlayerorderwidget.h>
%End

  public:
    explicit QgsCustomLayerOrderWidget( QgsLayerTreeMapCanvasBridge* bridge, QWidget *parent /TransferThis/ = 0 );

  protected slots:
    void bridgeHasCustomLayerOrderChanged( bool state );
    void bridgeCustomLayerOrderChanged( const QStringList& order );
    void nodeVisibilityChanged( QgsLayerTreeNode* node, Qt::CheckState state );

    void modelUpdated();
};
