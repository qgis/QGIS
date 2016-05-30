/** \ingroup gui
 * \class QgsVectorLayerPropertiesPage
 * \note added in QGIS 2.16
 * Base class for custom vector layer property pages
 */
class QgsVectorLayerPropertiesPage : QWidget
{
%TypeHeaderCode
#include <qgsvectorlayerpropertiespage.h>
%End

  public:

    /** Constructor for QgsVectorLayerPropertiesPage.
     * @param parent parent widget
    */
    explicit QgsVectorLayerPropertiesPage( QWidget *parent /TransferThis/ = 0 );

  public slots:
    /** Apply changes */
    virtual void apply() = 0;
};
