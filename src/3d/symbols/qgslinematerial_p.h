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
 * \brief Implementation of material that renders the straight segments of 3D linestrings.
 *
 * Supports:
 *
 * - arbitrary line width (in pixels)
 * - flat line caps
 *
 * The material needs information about viewport size (to correctly scale line widths) and to camera
 * parameters (to correctly clip lines).
 *
 * It is implemented with GPU instancing: a single quad is instanced once per line segment, with
 * the segment's endpoints (pointA/pointB) and its neighboring points (pointPrev/pointNext)
 * provided as per-instance vertex attributes. The vertex shader expands the quad into a
 * screen-space rectangle of the correct width around the segment, pulling back the vertex
 * shared with the neighboring segment so adjacent quads don't overlap under alpha blending.
 *
 * Line joins require a separate instanced draw, using the same class with LinePart::Join - it draws
 * one instanced triangle "wedge" per interior line vertex, filling the gap/overlap that would
 * otherwise appear between two independently-clipped segment quads drawn by LinePart::Segment at a bend.
 */
class _3D_EXPORT QgsLineMaterial : public QgsMaterial
{
    Q_OBJECT
  public:
    enum class LinePart
    {
      Segment, //!< Straight segments, with flat caps (see class docs)
      Join,    //!< Joins (corners) between segments (see class docs)
    };

    explicit QgsLineMaterial( LinePart part = LinePart::Segment );

    //! Returns whether this material renders line segments or their joins.
    LinePart part() const { return mPart; }

    //! Must be an SRGB color
    void setLineColor( const QColor &color );
    void setUseVertexColors( bool enabled );
    void setLineWidth( float width );

    Q_INVOKABLE void setViewportSize( const QSizeF &viewportSize );

    /**
     * Copies this material's line color, width, viewport size and useVertexColors settings to \a other.
     *
     * Used to keep a LinePart::Join material's appearance in sync with its LinePart::Segment counterpart.
     */
    void copyLineParametersTo( QgsLineMaterial *other ) const;

  private:
    LinePart mPart;

    Qt3DRender::QParameter *mParameterThickness = nullptr;
    Qt3DRender::QParameter *mParameterLineColor = nullptr;
    Qt3DRender::QParameter *mParameterUseVertexColors = nullptr;
    Qt3DRender::QParameter *mParameterWindowScale = nullptr;

    //! Only set for LinePart::Join
    Qt3DRender::QParameter *mParameterMiterLimit = nullptr;
};

/// @endcond

#endif // QGSLINEMATERIAL_P_H
