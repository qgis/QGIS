/**
 * @brief The QgsLayerDefinition class holds generic methods for loading/exporting QLR files.
 *
 * QLR files are an export of the layer xml including the style and datasource location.  There is no link
 * to the QLR file once loaded.  Consider the QLR file a mini project file for layers and styles.  QLR
 * files also store the layer tree info for the exported layers, including group information.
 */
class QgsLayerDefinition
{
%TypeHeaderCode
#include <qgslayerdefinition.h>
%End
  public:
    /** Loads the QLR at path into QGIS.  New layers are added to rootGroup and the map layer registry*/
    static bool loadLayerDefinition( const QString & path, QgsLayerTreeGroup* rootGroup, QString &errorMessage /Out/ );
    /** Loads the QLR from the XML document.  New layers are added to rootGroup and the map layer registry */
    static bool loadLayerDefinition( QDomDocument doc, QgsLayerTreeGroup* rootGroup, QString &errorMessage /Out/ );
    /** Export the selected layer tree nodes to a QLR file */
    static bool exportLayerDefinition( QString path, QList<QgsLayerTreeNode*> selectedTreeNodes, QString &errorMessage /Out/ );
};

