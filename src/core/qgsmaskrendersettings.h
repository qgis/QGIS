/***************************************************************************
  qgsmaskrendersettings.h
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMASKRENDERSETTINGS_H
#define QGSMASKRENDERSETTINGS_H

#include "qgis_core.h"

/**
 * \class QgsMaskRenderSettings
 * \ingroup core
 *
 * \brief Contains settings regarding how masks are calculated and handled during a map render.
 *
 * \since QGIS 3.38
*/
class CORE_EXPORT QgsMaskRenderSettings
{
  public:

    QgsMaskRenderSettings();

    /**
     * Returns the simplification tolerance (in painter units) to use for on-the-fly simplification of mask paths while rendering.
     *
     * A tolerance of 0 indicates no simplification. (No simplification is the default behavior).
     *
     * \note This property is only used when exporting to vector formats, and is ignored during raster format based rendering.
     *
     * \see setSimplificationTolerance()
     */
    double simplifyTolerance() const { return mSimplifyTolerance; }

    /**
     * Sets a simplification tolerance (in painter units) to use for on-the-fly simplification of mask paths while rendering.
     *
     * This will result in simpler, generalised paths.
     *
     * Set \a tolerance to 0 to disable simplification. (No simplification is the default behavior).
     *
     * \note This property is only used when exporting to vector formats, and is ignored during raster format based rendering.
     *
     * \see simplifyTolerance()
     */
    void setSimplificationTolerance( double tolerance );

  private:

    double mSimplifyTolerance = 0;

};

#endif // QGSMASKRENDERSETTINGS_H
