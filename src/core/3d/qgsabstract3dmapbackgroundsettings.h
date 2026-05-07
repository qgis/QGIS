/***************************************************************************
  qgsabstract3dmapbackgroundsettings.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACT3DMAPBACKGROUNDSETTINGS_H
#define QGSABSTRACT3DMAPBACKGROUNDSETTINGS_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgsabstract3dasset.h"

class QDomElement;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \brief Base class for all background settings classes used in a 3D map view.
 *
 * QgsAbstract3DMapBackgroundSettings subclasses are responsible for storing
 * the configuration of different background types (skybox, gradient).
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsAbstract3DMapBackgroundSettings : public QgsAbstract3DAsset
{
  public:
    Qgis::Asset3DType assetType() const override { return Qgis::Asset3DType::BackgroundSettings; }

    /**
     * Returns the unique type for this background settings class.
     */
    virtual Qgis::Map3DBackgroundType type() const = 0;

    //! Returns a deep copy of this background settings object.
    virtual QgsAbstract3DMapBackgroundSettings *clone() const override = 0 SIP_FACTORY;
};

#endif // QGSABSTRACT3DMAPBACKGROUNDSETTINGS_H
