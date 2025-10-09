/***************************************************************************
  qgsbillboardgeometry.h
  --------------------------------------
  Date                 : Jul 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBILLBOARDGEOMETRY_H
#define QGSBILLBOARDGEOMETRY_H

#include "qgis_3d.h"
#include <QObject>
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>
#else
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QAttribute>
#endif
#include <QVector3D>
#include <QVector2D>

#define SIP_NO_FILE


/**
 * \ingroup qgis_3d
 * \brief Geometry of the billboard rendering for points in 3D map view.
 *
 * This class is designed for use with the QgsPoint3DBillboardMaterial material class.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.10
 */
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
class _3D_EXPORT QgsBillboardGeometry : public Qt3DRender::QGeometry
#else
class _3D_EXPORT QgsBillboardGeometry : public Qt3DCore::QGeometry
#endif
{
    Q_OBJECT

    Q_PROPERTY( int count READ count NOTIFY countChanged )
  public:
    //! Constructor of QgsBillboardGeometry.
    QgsBillboardGeometry( Qt3DCore::QNode *parent = nullptr );

    /**
     * Sets the vertex positions for the billboards.
     *
     * Use this method when rendering multiple billboards with the same texture, and texture
     * atlas handling is not required.
     *
     * \see setBillboardData()
     */
    void setPositions( const QVector<QVector3D> &vertices );

    /**
     * \ingroup qgis_3d
     * \brief Contains the billboard positions and texture information.
     *
     * \note Not available in Python bindings
     * \since QGIS 4.0
     */
    struct BillboardAtlasData
    {
        //! Vertex position for billboard placement
        QVector3D position;
        //! Texture atlas offset for associated billboard texture
        QVector2D textureAtlasOffset;
        //! Texture atlas size for associated billboard texture
        QVector2D textureAtlasSize;
        //! Optional pixel offset for billboard
        QPoint pixelOffset;
    };

    /**
     * Set the position and texture data for the billboard.
     *
     * Use this method when rendering billboards with the different textures, and texture
     * atlas handling is required.
     *
     * Set \a includePixelOffsets to TRUE if the billboard data includes the optional pixel offsets for each billboard
     *
     * \see setPositions()
     */
    void setBillboardData( const QVector<QgsBillboardGeometry::BillboardAtlasData> &billboards, bool includePixelOffsets = false );

    //! Returns the number of points.
    int count() const;

  signals:
    //! Signal when the number of points changed.
    void countChanged( int count );

  private:
    enum class Mode
    {
      PositionOnly,
      PositionAndTextureData,
      PositionAndTextureDataWithPixelOffsets,
    };
    void setMode( Mode mode );

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mAtlasOffsetAttribute = nullptr;
    Qt3DRender::QAttribute *mAtlasSizeAttribute = nullptr;
    Qt3DRender::QAttribute *mAtlasPixelOffsetAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
#else
    Qt3DCore::QAttribute *mPositionAttribute = nullptr;
    Qt3DCore::QAttribute *mAtlasOffsetAttribute = nullptr;
    Qt3DCore::QAttribute *mAtlasSizeAttribute = nullptr;
    Qt3DCore::QAttribute *mAtlasPixelOffsetAttribute = nullptr;
    Qt3DCore::QBuffer *mVertexBuffer = nullptr;
#endif
    int mVertexCount = 0;
};

#endif // QGSBILLBOARDGEOMETRY_H
