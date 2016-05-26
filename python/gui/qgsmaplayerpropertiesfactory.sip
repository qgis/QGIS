/** \ingroup gui
 * \note added in 2.1
 */
class QgsMapLayerPropertiesFactory
{
%TypeHeaderCode
#include <qgsmaplayerpropertiesfactory.h>
%End

  public:
    QgsMapLayerPropertiesFactory();

    virtual QgsVectorLayerPropertiesPage* createVectorLayerPropertiesPage( QgsVectorLayer* layer, QWidget* parent ) = 0;
    virtual QListWidgetItem* createVectorLayerPropertiesItem( QgsVectorLayer* layer, QListWidget* view ) = 0;
};
