/***************************************************************************
    qgsvectorfieldtracessettings.h
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

#ifndef QGSVECTORFIELDTRACESSETTINGS_H
#define QGSVECTORFIELDTRACESSETTINGS_H

#include "qgis.h"
#include "qgis_core.h"

#include <QDomElement>

/**
 * \ingroup core
 *
 * \brief Represents a trace renderer settings for vector datasets displayed by particle traces.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVectorFieldTracesSettings
{
  public:
    //! Returns the maximum tail length
    double maximumTailLength() const;
    //! Sets the maximums tail length
    void setMaximumTailLength( double maximumTailLength );
    //! Returns particles count
    int particlesCount() const;
    //! Sets particles count
    void setParticlesCount( int value );
    //! Returns the maximum tail length unit
    Qgis::RenderUnit maximumTailLengthUnit() const;
    //! Sets the maximum tail length unit
    void setMaximumTailLengthUnit( Qgis::RenderUnit maximumTailLengthUnit );

    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem );
    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc ) const;

  private:
    int mParticlesCount = 1000;
    double mMaximumTailLength = 100;
    Qgis::RenderUnit mMaximumTailLengthUnit = Qgis::RenderUnit::Millimeters;
};

#endif // QGSVECTORFIELDTRACESSETTINGS_H
