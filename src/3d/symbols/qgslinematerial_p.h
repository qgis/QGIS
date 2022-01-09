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

#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QCamera>

#define SIP_NO_FILE

/**
 * \ingroup 3d
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
class QgsLineMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT
  public:
    QgsLineMaterial();

    void setLineColor( const QColor &color );
    QColor lineColor() const;

    void setLineWidth( float width );
    float lineWidth() const;

    Q_INVOKABLE void setViewportSize( const QSizeF &viewportSize );

  private:
    Qt3DRender::QParameter *mParameterThickness = nullptr;
    Qt3DRender::QParameter *mParameterMiterLimit = nullptr;
    Qt3DRender::QParameter *mParameterLineColor = nullptr;

    Qt3DRender::QParameter *mParameterWindowScale = nullptr;

};

/// @endcond

#endif // QGSLINEMATERIAL_P_H
