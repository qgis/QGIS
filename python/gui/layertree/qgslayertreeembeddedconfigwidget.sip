
/** \ingroup gui
 * \class QgsLayerTreeEmbeddedConfigWidget
 * A widget to configure layer tree embedded widgets for a particular map layer.
 * @note introduced in QGIS 2.16
 */
class QgsLayerTreeEmbeddedConfigWidget : public QWidget
{
%TypeHeaderCode
#include <qgslayertreeembeddedconfigwidget.h>
%End

  public:
    QgsLayerTreeEmbeddedConfigWidget( QWidget* parent /TransferThis/ = nullptr );

    //! Initialize widget with a map layer
    void setLayer( QgsMapLayer* layer );

    //! Store changes made in the widget to the layer
    void applyToLayer();

};
