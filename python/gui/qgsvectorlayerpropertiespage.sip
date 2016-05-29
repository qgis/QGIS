/** \ingroup gui
 * \note added in 2.1
 */

/**
 * @brief Base class for custom vector layer property pages
 */
class QgsVectorLayerPropertiesPage : QWidget
{
%TypeHeaderCode
#include <qgsvectorlayerpropertiespage.h>
%End

  public:
    /** Constructor */
    explicit QgsVectorLayerPropertiesPage( QWidget *parent = 0 );

  public slots:
    /** Apply changes */
    virtual void apply() = 0;
};
