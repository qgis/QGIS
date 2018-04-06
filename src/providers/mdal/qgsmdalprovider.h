/***************************************************************************
    qgsmdalprovider.h
    -----------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMDALPROVIDER_H
#define QGSGDALPROVIDER_H

#include <cstddef>

#include "qgscoordinatereferencesystem.h"
#include "qgsdataitem.h"
#include "qgsmeshdataprovider.h"
#include "qgsrectangle.h"

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QMap>
#include <QVector>

#include <mdal.h>

class QMutex;
class QgsCoordinateTransform;

/**
  \brief Data provider for MDAL layers.
*/
class QgsMdalProvider : public QgsMeshDataProvider
{
    Q_OBJECT

  public:

    /**
     * Constructor for the provider.
     *
     * \param   uri         file name
     * \param   newDataset  handle of newly created dataset.
     *
     */
    QgsMdalProvider( const QString &uri = QString() );
    ~QgsMdalProvider();

    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override;

    size_t vertexCount() const override;
    size_t faceCount() const override;
    QgsMeshVertex vertex( size_t index ) const override;
    QgsMeshFace face( size_t index ) const override;

  private:
    MeshH mMeshH;
};

#endif

