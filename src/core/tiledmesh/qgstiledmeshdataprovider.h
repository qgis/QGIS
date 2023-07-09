/***************************************************************************
                         qgstiledmeshdataprovider.h
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDMESHDATAPROVIDER_H
#define QGSTILEDMESHDATAPROVIDER_H

#include "qgis_core.h"
#include "qgsdataprovider.h"
#include "qgis.h"

/**
 * \ingroup core
 * \brief Base class for data providers for QgsTiledMeshLayer
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshDataProvider: public QgsDataProvider
{
    Q_OBJECT
  public:


    //! Constructor for QgsTiledMeshDataProvider
    QgsTiledMeshDataProvider( const QString &uri,
                              const QgsDataProvider::ProviderOptions &providerOptions,
                              QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsTiledMeshDataProvider() override;

    /**
     * Copy constructor.
     */
    QgsTiledMeshDataProvider( const QgsTiledMeshDataProvider &other );

    /**
     * QgsTiledMeshDataProvider cannot be assigned.
     */
    QgsTiledMeshDataProvider &operator=( const QgsTiledMeshDataProvider &other ) = delete;

    /**
     * Returns flags containing the supported capabilities for the data provider.
     */
    virtual Qgis::TiledMeshProviderCapabilities capabilities() const;

    /**
     * Returns a clone of the data provider.
     */
    virtual QgsTiledMeshDataProvider *clone() const = 0 SIP_FACTORY;


};

#endif // QGSTILEDMESHDATAPROVIDER_H
