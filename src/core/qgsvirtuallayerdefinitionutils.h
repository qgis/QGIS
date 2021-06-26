/***************************************************************************
                qgsvirtuallayerdefinitionutils.h
begin                : Jan, 2016
copyright            : (C) 2016 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUALLAYERDEFINITION_UTILS_H
#define QGSVIRTUALLAYERDEFINITION_UTILS_H

#include "qgis_core.h"

class QgsVectorLayer;
class QgsVirtualLayerDefinition;

/**
 * \ingroup core
 * Utils class for QgsVirtualLayerDefinition
 */
class CORE_EXPORT QgsVirtualLayerDefinitionUtils
{
  public:
    //! Gets a virtual layer definition from a vector layer where vector joins are replaced by SQL LEFT JOINs
    static QgsVirtualLayerDefinition fromJoinedLayer( QgsVectorLayer *joinedLayer );
};

#endif
