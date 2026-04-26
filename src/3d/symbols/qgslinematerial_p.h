/***************************************************************************
  qgslinematerial_p.h
  --------------------------------------
  Date                 : Apr 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLINEMATERIAL_P_H
#define QGSLINEMATERIAL_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgis_3d.h"
#include "qgsmaterial.h"

#include <Qt3DRender/QCamera>

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Implementation of material that renders 3D linestrings.
 *
 * Supports:
 *
 * - arbitrary line width (in pixels)
 * - bevel and miter line joins (including limit for miter join to avoid very long miters on sharp angles)
 * - flat line caps
 * - alpha blending
 *
 * The material needs information about viewport size (to correctly scale line widths) and to camera
 * parameters (to correctly clip lines).
 *
 * It is implemented by using a geometry shader that accepts primitive type Line strip with adjacency
 * (i.e. we have access to points p1-p2 which define line segment's endpoints, and in addition to that,
 * we have access to previous (p0) and next (p3) points). Geometry shader generates two triangles
 * for each segment and possibly another triangle for bevel join.
 */
class _3D_EXPORT QgsLineMaterial : public QgsMaterial
{
    Q_OBJECT
  public:
    QgsLineMaterial();

    //! Must be an SRGB color
    void setLineColor( const QColor &color );
    void setUseVertexColors( bool enabled );
    void setLineWidth( float width );

    Q_INVOKABLE void setViewportSize( const QSizeF &viewportSize );

  private:
    Qt3DRender::QParameter *mParameterThickness = nullptr;
    Qt3DRender::QParameter *mParameterMiterLimit = nullptr;
    Qt3DRender::QParameter *mParameterLineColor = nullptr;
    Qt3DRender::QParameter *mParameterUseVertexColors = nullptr;

    Qt3DRender::QParameter *mParameterWindowScale = nullptr;
};

/// @endcond

#endif // QGSLINEMATERIAL_P_H
