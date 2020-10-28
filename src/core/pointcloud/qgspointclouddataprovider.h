/***************************************************************************
                         qgspointclouddataprovider.h
                         ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDDATAPROVIDER_H
#define QGSPOINTCLOUDDATAPROVIDER_H

#include "qgis_core.h"
#include "qgsdataprovider.h"
#include <memory>

class QgsPointCloudIndex;

/**
 * \ingroup core
 * Base class for providing data for QgsPointCloudLayer
 *
 * Responsible for reading native point cloud data and returning the indexed data.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudDataProvider: public QgsDataProvider
{
    Q_OBJECT
  public:
    //! Ctor
    QgsPointCloudDataProvider( const QString &uri,
                               const QgsDataProvider::ProviderOptions &providerOptions,
                               QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsPointCloudDataProvider() override;
    QgsRectangle extent() const override;
    bool isValid() const override;

    /**
     * Returns the point cloud index associated with the provider.
     *
     * \note Not available in Python bindings
     */
    virtual QgsPointCloudIndex *index() const SIP_SKIP {return nullptr;}

  private:
    bool mIsValid = false;
};

#endif // QGSMESHDATAPROVIDER_H
