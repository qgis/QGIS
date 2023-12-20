/***************************************************************************
                          qgspdfrenderer.h
                             -------------------
    begin                : December 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#ifndef QGSPDFRENDERER_H
#define QGSPDFRENDERER_H

#include "qgis_sip.h"
#include "qgis_core.h"

/**
 * \class QgsPdfRenderer
 * \ingroup core
 * \brief Utility class for rendering PDF documents.
 *
 * This functionality is not available on all platforms -- it requires a build
 * with the PDF4Qt library support enabled. On other platforms calling these
 * methods will raise a QgsNotSupportedException.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsPdfRenderer
{
  public:
};

#endif // QGSPDFRENDERER_H
