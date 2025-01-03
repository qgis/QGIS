/***************************************************************************
  qgsgeotransform.h
  --------------------------------------
  Date                 : November 2024
  Copyright            : (C) 2024 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOTRANSFORM_H
#define QGSGEOTRANSFORM_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

#include <Qt3DCore/QTransform>

#include "qgsvector3d.h"


/**
 * Specialied kind of QTransform that:
 *
 * - stores translation as QgsVector3D, i.e. in double precision
 * - reacts to map scene's origin shifts and updates the QTransform accordingly
 */
class QgsGeoTransform : public Qt3DCore::QTransform
{
    Q_OBJECT
  public:
    explicit QgsGeoTransform( Qt3DCore::QNode *parent = nullptr );

    //! Sets translation in map coordinates and updates the underlying QTransform
    void setGeoTranslation( const QgsVector3D &translation );

    //! Sets 3D scene's origin in map coordinates and updates the underlying QTransform
    void setOrigin( const QgsVector3D &origin );

  private:
    QgsVector3D mTranslation;
    QgsVector3D mOrigin;
};

///@endcond

#endif // QGSGEOTRANSFORM_H
