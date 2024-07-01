/***************************************************************************
                         qgspointcloudrequest.h
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

#ifndef QGSPOINTCLOUDREQUEST_H
#define QGSPOINTCLOUDREQUEST_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgsgeometry.h"
#include <QPair>
#include <QString>
#include <QVector>
#include <QByteArray>

#define SIP_NO_FILE

#include "qgspointcloudattribute.h"

#include "qgsgeometry.h"

/**
 * \ingroup core
 *
 * \brief Point cloud data request
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRequest
{
  public:
    //! Ctor
    QgsPointCloudRequest();

    //! Equality operator
    bool operator==( const QgsPointCloudRequest &other ) const;

    //! Returns attributes
    QgsPointCloudAttributeCollection attributes() const;

    //! Set attributes filter in the request
    void setAttributes( const QgsPointCloudAttributeCollection &attributes );

    /**
     * Returns the rectangle from which points will be taken, in point cloud's crs. If the returned rectangle is empty, then no filter rectangle is set.
     * \since QGIS 3.40
     */
    QgsGeometry filterGeometry() const { return mFilterGeometry; }

    /**
     * Sets the rectangle from which points will be taken, in point cloud's crs. An empty rectangle removes the filter.
     * \since QGIS 3.40
     */
    void setFilterGeometry( const QgsGeometry &geometry ) { mFilterGeometry = geometry; }
  private:
    QgsPointCloudAttributeCollection mAttributes;
    QgsGeometry mFilterGeometry;
};

//! Hash function for QgsPointCloudRequest
uint qHash( const QgsPointCloudRequest &request );

#endif // QGSPOINTCLOUDREQUEST_H
