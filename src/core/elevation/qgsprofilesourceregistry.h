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
 * \brief Registry of profile sources used by QgsProfilePlotRenderer.
 *
 * QgsProfileSourceRegistry is not usually directly created, but rather accessed through
 * QgsApplication::profileSourceRegistry().
 *
 * \since QGIS 3.38
 */
class CORE_EXPORT QgsProfileSourceRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor - creates a registry of profile sources
     */
    QgsProfileSourceRegistry();

    ~QgsProfileSourceRegistry();

    /**
     * Returns a list of registered profile sources.
     */
    QList< QgsAbstractProfileSource * > profileSources() const;

    /**
     * Registers a profile \a source and takes ownership of it.
     *
     * Returns TRUE if the profile \a source could be registered and FALSE otherwise.
     */
    bool registerProfileSource( QgsAbstractProfileSource *source SIP_TRANSFER );

    /**
     * Unregisters a profile \a source and destroys its instance.
     *
     * \deprecated QGIS 4.0. Unregister the profile source by ID instead.
     */
    Q_DECL_DEPRECATED bool unregisterProfileSource( QgsAbstractProfileSource *source ) SIP_DEPRECATED;

    /**
     * Unregisters a profile source by a given ID and destroys its instance.
     *
     * Returns TRUE if the source id was found in the registry and FALSE otherwise.
     *
     * \param sourceId  Profile source ID to be unregistered.
     * \since QGIS 4.0
     */
    bool unregisterProfileSource( const QString &sourceId );

    /**
     * Finds a registered profile source by id.
     * Returns NULLPTR if the source is not found in the registry.
     *
     * \param sourceId  Id of the source to be found in the registry.
     * \since QGIS 4.0
     */
    QgsAbstractProfileSource *findSourceById( const QString &sourceId ) const;

  signals:

    /**
     * Signal emitted once a profile source is registered.
     *
     * \param sourceId    Unique identifier of the profile source that has been registered.
     * \param sourceName  Name of the profile source that has been registered.
     *
     * \since QGIS 4.0
     */
    void profileSourceRegistered( const QString &sourceId, const QString &sourceName );

    /**
     * Signal emitted once a profile source is unregistered.
     *
     * \param sourceId    Unique identifier of the profile source that has been unregistered.
     *
     * \since QGIS 4.0
     */
    void profileSourceUnregistered( const QString &sourceId );

  private:
    QList< QgsAbstractProfileSource * > mSources;

    Q_DISABLE_COPY( QgsProfileSourceRegistry )
};

#endif // QGSPROFILESOURCEREGISTRY_H
