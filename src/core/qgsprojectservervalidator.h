#include "qgis_core.h"
#include "qgslayertreegroup.h"
#include "qgslayertree.h"

#ifndef QGSPROJECTSERVERVALIDATOR_H
#define QGSPROJECTSERVERVALIDATOR_H

/**
 * \ingroup core
 * \class QgsProjectServerValidator
 * \brief Project server validator.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProjectServerValidator
{

  public:

    /**
     * Recursive function to check a layer tree group.
     * The function will collect OWS names and encoding for each layers.
     * \since QGIS 3.14
     */
    static void checkOWS( QgsLayerTreeGroup *treeGroup, QStringList &owsNames, QStringList &encodingMessages );

    /**
     * Check a layer tree.
     * \since QGIS 3.14
     */
    static QString projectStatusHtml( QgsLayerTree *layerTree );

};

#endif // QGSPROJECTSERVERVALIDATOR_H
