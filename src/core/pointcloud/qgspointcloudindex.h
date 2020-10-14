/***************************************************************************
                         QgsPointCloudIndex.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Martin Dobias and Peter Petrik
    email                : wonder dot sk at gmail dot com, zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDINDEX_H
#define QGSPOINTCLOUDINDEX_H

#include <QString>

/**
 * \ingroup core
 *
 * Represents a indexed point clouds data in octree
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudIndex
{
    Q_OBJECT
  public:

    explicit QgsPointCloudIndex();
     ~QgsPointCloudIndex();

    void load(const QString& fileName);
};


#endif // QGSPOINTCLOUDINDEX_H
