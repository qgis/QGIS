/***************************************************************************
    qgsvectorfieldstreamlinesettings.h
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORFIELDSTREAMLINESETTINGS_H
#define QGSVECTORFIELDSTREAMLINESETTINGS_H

#include "qgis.h"
#include "qgis_core.h"

#include <QDomElement>

/**
 * \ingroup core
 *
 * \brief Represents a streamline renderer settings for vector datasets displayed by streamlines.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVectorFieldStreamlineSettings
{
  public:
    //! Returns the method used for seeding start points of strealines
    Qgis::VectorFieldSeedingMethod seedingMethod() const;
    //! Sets the method used for seeding start points of strealines
    void setSeedingMethod( const Qgis::VectorFieldSeedingMethod &seedingMethod );
    //! Returns the density used for seeding start points
    double seedingDensity() const;
    //! Sets the density used for seeding start points
    void setSeedingDensity( double seedingDensity );
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );
    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;

  private:
    Qgis::VectorFieldSeedingMethod mSeedingMethod = Qgis::VectorFieldSeedingMethod::Gridded;
    double mSeedingDensity = 0.15;
};

#endif // QGSVECTORFIELDSTREAMLINESETTINGS_H
