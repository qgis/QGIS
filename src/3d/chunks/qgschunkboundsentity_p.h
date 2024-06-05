/***************************************************************************
  qgschunkboundsentity_p.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCHUNKBOUNDSENTITY_P_H
#define QGSCHUNKBOUNDSENTITY_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DCore/QEntity>

class QgsAABB;
class Qgs3DWiredMesh;

#define SIP_NO_FILE


/**
 * \ingroup 3d
 * \brief Draws bounds of axis aligned bounding boxes
 * \note Not available in Python bindings
 */
class QgsChunkBoundsEntity : public Qt3DCore::QEntity
{
    Q_OBJECT

  public:
    //! Constructs the entity
    QgsChunkBoundsEntity( Qt3DCore::QNode *parent = nullptr );

    //! Sets a list of bounding boxes to be rendered by the entity
    void setBoxes( const QList<QgsAABB> &bboxes );

  private:
    Qgs3DWiredMesh *mAabbMesh = nullptr;
};

/// @endcond

#endif // QGSCHUNKBOUNDSENTITY_P_H
