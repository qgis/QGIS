/***************************************************************************
                         qgspointcloudsubindex.h
                         -----------------------
    begin                : March 2023
    copyright            : (C) 2023 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDSUBINDEX_H
#define QGSPOINTCLOUDSUBINDEX_H

#include <memory>
#include <QString>
#include "qgsgeometry.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsPointCloudIndex;

class QgsPointCloudSubIndex
{
  public:
    std::shared_ptr<QgsPointCloudIndex> index;
    QString uri;
    QgsRectangle extent;
    QgsGeometry geometry;
    qint64 count = 0;
};

///@endcond
#endif // QGSPOINTCLOUDSUBINDEX_H
