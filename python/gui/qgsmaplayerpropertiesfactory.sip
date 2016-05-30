/** \ingroup gui
 * \class QgsMapLayerPropertiesFactory
 * \note added in QGIS 2.16
 * Factory class for creating custom map layer property pages
 */
class QgsMapLayerPropertiesFactory
{
%TypeHeaderCode
#include <qgsmaplayerpropertiesfactory.h>
%End

  public:
    /** Constructor */
    QgsMapLayerPropertiesFactory();

    /** Destructor */
    virtual ~QgsMapLayerPropertiesFactory();

    /**
     * @brief Create a new properties page
     * @param layer The layer for which to create the page
     * @param parent The parent widget
     * @return The new properties page instance
     */
    virtual QgsVectorLayerPropertiesPage* createVectorLayerPropertiesPage( QgsVectorLayer* layer, QWidget* parent ) = 0;

    /**
     * @brief Creates the QListWidgetItem for the properties page
     * @param layer The layer for which to create the item
     * @param view The parent QListView
     * @return The QListWidgetItem for the properties page
     */
    virtual QListWidgetItem* createVectorLayerPropertiesItem( QgsVectorLayer* layer, QListWidget* view ) = 0 /Factory/;
};
