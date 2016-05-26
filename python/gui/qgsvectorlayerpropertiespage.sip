/** \ingroup gui
 * \note added in 2.1
 */
class QgsVectorLayerPropertiesPage : QWidget
{
%TypeHeaderCode
#include <qgsvectorlayerpropertiespage.h>
%End

  public:
    explicit QgsVectorLayerPropertiesPage( QWidget *parent = 0 );

  public slots:
    virtual void apply() = 0;
};
