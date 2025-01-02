/***************************************************************************
  qgsprofilesourceregistry.h
  --------------------------------------
  Date                 : April 2024
  Copyright            : (C) 2024 by Germán Carrillo
  Email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROFILESOURCEREGISTRY_H
#define QGSPROFILESOURCEREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsAbstractProfileSource;

#include <QList>


/**
 * \ingroup core
 * \brief Registry of profile sources used by QgsProfilePlotRenderer
 *
 * QgsProfileSourceRegistry is not usually directly created, but rather accessed through
 * QgsApplication::profileSourceRegistry().
 *
 * \since QGIS 3.38
 */
class CORE_EXPORT QgsProfileSourceRegistry
{
  public:

    /**
     * Constructor - creates a registry of profile sources
     */
    QgsProfileSourceRegistry();

    ~QgsProfileSourceRegistry();

    /**
     * Returns a list of registered profile sources
     */
    QList< QgsAbstractProfileSource * > profileSources() const;

    /**
     * Registers a profile \a source and takes ownership of it
     */
    void registerProfileSource( QgsAbstractProfileSource *source SIP_TRANSFER );

    /**
     * Unregisters a profile \a source and destroys its instance
     */
    void unregisterProfileSource( QgsAbstractProfileSource *source );

  private:
    QList< QgsAbstractProfileSource * > mSources;
};

#endif // QGSPROFILESOURCEREGISTRY_H
