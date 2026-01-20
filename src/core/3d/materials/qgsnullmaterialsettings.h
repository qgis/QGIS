/***************************************************************************
  qgsnullmaterialsettings.h
  --------------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSNULLMATERIALSETTINGS_H
#define QGSNULLMATERIALSETTINGS_H

#include "qgis_core.h"
#include "qgsabstractmaterialsettings.h"

#include <QColor>

class QDomElement;

/**
 * \ingroup core
 * \brief Null shading material used for rendering models and scenes with native textures.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.16.2
 */
class CORE_EXPORT QgsNullMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    QgsNullMaterialSettings() = default;

    QString type() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the material.
     */
    static bool supportsTechnique( Qgis::MaterialRenderingTechnique technique );

    /**
     * Returns a new instance of QgsNullMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsNullMaterialSettings *clone() const override SIP_FACTORY;
    bool equals( const QgsAbstractMaterialSettings *other ) const override;

    QColor averageColor() const override;
};


#endif // QGSNULLMATERIALSETTINGS_H
