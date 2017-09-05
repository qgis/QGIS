/***************************************************************************
                            qgsgeocmsproviderregistry.h
                            ---------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOCMSPROVIDERREGISTRY_H
#define QGSGEOCMSPROVIDERREGISTRY_H

#include "qgis.h"

/**
 * \ingroup app
 *
 * This is a trivial registry for GeoCMS providers. It will be 'fleshed out'
 * as additional GeoCMS providers are added, and the required common functionality
 * between the different providers is determined.
 *
 * \since QGIS 3.0
 */
class QgsGeoCmsProviderRegistry
{

  public:
    QgsGeoCmsProviderRegistry();

    //! QgsGeoCmsProviderRegistry cannot be copied.
    QgsGeoCmsProviderRegistry( const QgsGeoCmsProviderRegistry &rh ) = delete;

    //! QgsGeoCmsProviderRegistry cannot be copied.
    QgsGeoCmsProviderRegistry &operator=( const QgsGeoCmsProviderRegistry &rh ) = delete;

  private:

    //! Initializes the registry
    void init();

};

#endif // QGSGEOCMSPROVIDERREGISTRY_H
