#ifndef QGSLAYERDEFINITION_H
#define QGSLAYERDEFINITION_H

#include "qgslayertreegroup.h"

/**
 * @brief The QgsLayerDefinition class holds generic methods for loading/exporting QLR files.
 */
class CORE_EXPORT QgsLayerDefinition
{
public:
  /* Loads the QLR at path into QGIS.  New layers are added to rootGroup and the map layer registry*/
  static bool loadLayerDefinition( const QString & path, QgsLayerTreeGroup* rootGroup, QString &errorMessage);
  /* Loads the QLR from the XML document.  New layers are added to rootGroup and the map layer registry */
  static bool loadLayerDefinition( QDomDocument doc, QgsLayerTreeGroup* rootGroup, QString &errorMessage);
  /* Export the selected layer tree nodes to a QLR file */
  static bool exportLayerDefinition( QString path, QList<QgsLayerTreeNode*> selectedTreeNodes, QString &errorMessage );
};

#endif // QGSLAYERDEFINITION_H
