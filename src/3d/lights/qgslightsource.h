/***************************************************************************
                          qgslightsource.h
                          ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLIGHTSOURCE_H
#define QGSLIGHTSOURCE_H

#include "qgis_3d.h"

/**
 * \ingroup 3d
 * \brief Base class for light sources in 3d scenes.
 *
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsLightSource
{

  public:

    virtual ~QgsLightSource();

};


#endif // QGSLIGHTSOURCE_H
