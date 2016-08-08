/***************************************************************************
    qgscustomdrophandler.h
    ---------------------
    begin                : August 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCUSTOMDROPHANDLER_H
#define QGSCUSTOMDROPHANDLER_H

#include "qgsmimedatautils.h"

/** \ingroup gui
 * Abstract base class that may be implemented to handle new types of data to be dropped in QGIS.
 * Implementations will be used when a QgsMimeDataUtils::Uri has layerType equal to "custom",
 * and the providerKey is equal to key() returned by the implementation.
 * @note added in QGIS 3.0
 */
class GUI_EXPORT QgsCustomDropHandler
{
  public:
    virtual ~QgsCustomDropHandler();

    //! Type of custom URI recognized by the handler
    virtual QString key() const = 0;

    //! Method called from QGIS after a drop event with custom URI known by the handler
    virtual void handleDrop( const QgsMimeDataUtils::Uri& uri ) const = 0;
};

#endif // QGSCUSTOMDROPHANDLER_H
