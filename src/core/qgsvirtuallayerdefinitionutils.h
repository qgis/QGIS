/***************************************************************************
                qgsvirtuallayerdefinitionutils.h
begin                : Jan, 2016
copyright            : (C) 2016 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUALLAYERDEFINITION_UTILS_H
#define QGSVIRTUALLAYERDEFINITION_UTILS_H

#include "qgsvirtuallayerdefinition.h"

class QgsVectorLayer;

/** \ingroup core
 * Utils class for QgsVirtualLayerDefinition
 */
class CORE_EXPORT QgsVirtualLayerDefinitionUtils
{
  public:
    //! Get a virtual layer definition from a vector layer where vector joins are replaced by SQL LEFT JOINs
    static QgsVirtualLayerDefinition fromJoinedLayer( QgsVectorLayer* joinedLayer );
};

#endif
