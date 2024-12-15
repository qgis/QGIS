/***************************************************************************
  qgs3dutils.h
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

#ifndef QGS3DUTILS_H
#define QGS3DUTILS_H

class QgsLineString;
class QgsPolygon;
class QgsFeedback;

class QgsAbstract3DEngine;
class QgsAbstract3DSymbol;
class Qgs3DMapScene;
class QgsPointCloudRenderer;
class QgsPointCloudLayer3DRenderer;

namespace Qt3DExtras
{
  class QPhongMaterial;
}

class QSurface;

#include "qgs3dmapsettings.h"
#include "qgs3danimationsettings.h"
#include "qgs3dtypes.h"
#include "qgsaabb.h"
#include "qgsray3d.h"
#include "qgsraycastingutils.h"

#include <QSize>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCullFace>

#include <memory>

#define SIP_NO_FILE

class Qgs3DRenderContext;

/**
 * \ingroup 3d
 * \brief Miscellaneous utility functions used from 3D code.
 * \note Not available in Python bindings
 */
class _3D_EXPORT Qgs3DUtils
{
  public:
    /**
     * Captures image of the current 3D scene of a 3D engine. The function waits
     * until the scene is not fully loaded/updated before capturing the image.
     * \since QGIS 3.4
     */
    static QImage captureSceneImage( QgsAbstract3DEngine &engine, Qgs3DMapScene *scene );

    /**
     * Captures the depth buffer of the current 3D scene of a 3D engine. The function waits
     * until the scene is not fully loaded/updated before capturing the image.
     *
     * \note In order to get more precision, the depth values are encoded into RGB colors,
     * use Qgs3DUtils::decodeDepth() to get the correct depth value.
     * \since QGIS 3.24
     */
    static QImage captureSceneDepthBuffer( QgsAbstract3DEngine &engine, Qgs3DMapScene *scene );

    /**
     * Calculates approximate usage of GPU memory by an entity
     * \return GPU memory usage in megabytes
     * \since QGIS 3.34
     */
    static double calculateEntityGpuMemorySize( Qt3DCore::QEntity *entity );

    /**
     * Captures 3D animation frames to the selected folder
     *
     * \param animationSettings Settings for keyframes and camera
     * \param mapSettings 3d map settings
     * \param framesPerSecond number of frames per second to export
     * \param outputDirectory output directory where to export frames
     * \param fileNameTemplate template for exporting the frames.
     *        Must be in format prefix####.format, where number of
     *        # represents how many 0 should be left-padded to the frame number
     *        e.g. my###.jpg will create frames my001.jpg, my002.jpg, etc
     * \param outputSize size of the frame in pixels
     * \param error error string in case of failure
     * \param feedback optional feedback object used to cancel export or report progress
     * \return whether export succeeded. In case of failure, see error argument
     *
     * \since QGIS 3.8
     */
    static bool exportAnimation( const Qgs3DAnimationSettings &animationSettings, Qgs3DMapSettings &mapSettings, int framesPerSecond, const QString &outputDirectory, const QString &fileNameTemplate, const QSize &outputSize, QString &error, QgsFeedback *feedback = nullptr );

    /**
     * Calculates the highest needed zoom level for tiles in quad-tree given width of the base tile (zoom level 0)
     * in map units, resolution of the tile (e.g. tile's texture width) and desired maximum error in map units.
     */
    static int maxZoomLevel( double tile0width, double tileResolution, double maxError );

    //! Converts a value from AltitudeClamping enum to a string
    static QString altClampingToString( Qgis::AltitudeClamping altClamp );
    //! Converts a string to a value from AltitudeClamping enum
    static Qgis::AltitudeClamping altClampingFromString( const QString &str );

    //! Converts a value from AltitudeBinding enum to a string
    static QString altBindingToString( Qgis::AltitudeBinding altBind );
    //! Converts a string to a value from AltitudeBinding enum
    static Qgis::AltitudeBinding altBindingFromString( const QString &str );

    //! Converts a value from CullingMode enum to a string
    static QString cullingModeToString( Qgs3DTypes::CullingMode mode );
    //! Converts a string to a value from CullingMode enum
    static Qgs3DTypes::CullingMode cullingModeFromString( const QString &str );

    //! Clamps altitude of a vertex according to the settings, returns Z value
    static float clampAltitude( const QgsPoint &p, Qgis::AltitudeClamping altClamp, Qgis::AltitudeBinding altBind, float offset, const QgsPoint &centroid, const Qgs3DRenderContext &context );
    //! Clamps altitude of vertices of a linestring according to the settings
    static void clampAltitudes( QgsLineString *lineString, Qgis::AltitudeClamping altClamp, Qgis::AltitudeBinding altBind, const QgsPoint &centroid, float offset, const Qgs3DRenderContext &context );
    //! Clamps altitude of vertices of a polygon according to the settings
    static bool clampAltitudes( QgsPolygon *polygon, Qgis::AltitudeClamping altClamp, Qgis::AltitudeBinding altBind, float offset, const Qgs3DRenderContext &context );

    //! Converts a 4x4 transform matrix to a string
    static QString matrix4x4toString( const QMatrix4x4 &m );
    //! Convert a string to a 4x4 transform matrix
    static QMatrix4x4 stringToMatrix4x4( const QString &str );

    //! Calculates (x,y,z) positions of (multi)point from the given feature
    static void extractPointPositions( const QgsFeature &f, const Qgs3DRenderContext &context, const QgsVector3D &chunkOrigin, Qgis::AltitudeClamping altClamp, QVector<QVector3D> &positions );

    /**
     * Returns TRUE if bbox is completely outside the current viewing volume.
     * This is used to perform object culling checks.
    */
    static bool isCullable( const QgsAABB &bbox, const QMatrix4x4 &viewProjectionMatrix );

    //! Converts map coordinates to 3D world coordinates (applies offset)
    static QgsVector3D mapToWorldCoordinates( const QgsVector3D &mapCoords, const QgsVector3D &origin );
    //! Converts 3D world coordinates to map coordinates (applies offset)
    static QgsVector3D worldToMapCoordinates( const QgsVector3D &worldCoords, const QgsVector3D &origin );

    /**
     * Converts extent (in map layer's CRS) to axis aligned bounding box in 3D world coordinates
     * \since QGIS 3.12
     */
    static QgsAABB layerToWorldExtent( const QgsRectangle &extent, double zMin, double zMax, const QgsCoordinateReferenceSystem &layerCrs, const QgsVector3D &mapOrigin, const QgsCoordinateReferenceSystem &mapCrs, const QgsCoordinateTransformContext &context );

    /**
     * Converts axis aligned bounding box in 3D world coordinates to extent in map layer CRS
     * \since QGIS 3.12
     */
    static QgsRectangle worldToLayerExtent( const QgsAABB &bbox, const QgsCoordinateReferenceSystem &layerCrs, const QgsVector3D &mapOrigin, const QgsCoordinateReferenceSystem &mapCrs, const QgsCoordinateTransformContext &context );

    /**
     * Converts map extent to axis aligned bounding box in 3D world coordinates
     * \since QGIS 3.12
     */
    static QgsAABB mapToWorldExtent( const QgsRectangle &extent, double zMin, double zMax, const QgsVector3D &mapOrigin );

    /**
     * Converts 3D box in map coordinates to AABB in world coordinates.
     * \since QGIS 3.42
     */
    static QgsAABB mapToWorldExtent( const QgsBox3D &box3D, const QgsVector3D &mapOrigin );

    /**
     * Converts axis aligned bounding box in 3D world coordinates to extent in map coordinates
     * \since QGIS 3.12
     */
    static QgsRectangle worldToMapExtent( const QgsAABB &bbox, const QgsVector3D &mapOrigin );

    //! Transforms a world point from (origin1, crs1) to (origin2, crs2)
    static QgsVector3D transformWorldCoordinates( const QgsVector3D &worldPoint1, const QgsVector3D &origin1, const QgsCoordinateReferenceSystem &crs1, const QgsVector3D &origin2, const QgsCoordinateReferenceSystem &crs2, const QgsCoordinateTransformContext &context );

    /**
     * Try to estimate range of Z values used in the given vector layer and store that in zMin and zMax.
     * The implementation scans a small amount of features and looks at the Z values of geometries
     * (we don't need exact range, just a rough estimate is fine to know where to expect the data to be).
     * For layers with geometries without Z values, the returned range will be [0, 0].
     * \since QGIS 3.12
     */
    static void estimateVectorLayerZRange( QgsVectorLayer *layer, double &zMin, double &zMax );

    //! Returns expression context for use in preparation of 3D data of a layer
    static QgsExpressionContext globalProjectLayerExpressionContext( QgsVectorLayer *layer );

    //! Returns phong material settings object based on the Qt3D material
    static QgsPhongMaterialSettings phongMaterialFromQt3DComponent( Qt3DExtras::QPhongMaterial *material );

    //! Convert from clicked point on the screen to a ray in world coordinates
    static QgsRay3D rayFromScreenPoint( const QPoint &point, const QSize &windowSize, Qt3DRender::QCamera *camera );

    /**
     * Converts the clicked mouse position to the corresponding 3D world coordinates
     * \since QGIS 3.24
     */
    static QVector3D screenPointToWorldPos( const QPoint &screenPoint, double depth, const QSize &screenSize, Qt3DRender::QCamera *camera );

    /**
     * Function used to extract the pitch and yaw (also known as heading) angles in degrees from the view vector of the camera [cameraViewCenter - cameraPosition]
     * \since QGIS 3.24
     */
    static void pitchAndYawFromViewVector( QVector3D vect, double &pitch, double &yaw );

    /**
     * Converts from screen coordinates to texture coordinates
     * \note Expected return values are in [0, 1] range
     * \see textureToScreenCoordinates()
     * \since QGIS 3.24
     */
    static QVector2D screenToTextureCoordinates( QVector2D screenXY, QSize winSize );

    /**
     * Converts from texture coordinates coordinates to screen coordinates
     * \note Expected return values are in [0, winSize.width], [0, winSize.height] range
     * \see screenToTextureCoordinates()
     * \since QGIS 3.24
     */
    static QVector2D textureToScreenCoordinates( QVector2D textureXY, QSize winSize );

    /**
     * Decodes the depth value from the pixel's color value
     * The depth value is encoded from OpenGL side (the depth render pass) into the 3 RGB channels to preserve precision.
     *
     * \since QGIS 3.24
     */
    static double decodeDepth( const QRgb &pixel )
    {
      return ( ( qRed( pixel ) / 255.0 + qGreen( pixel ) ) / 255.0 + qBlue( pixel ) ) / 255.0;
    }

    /**
     * Creates a QgsPointCloudLayer3DRenderer matching the symbol settings of a given QgsPointCloudRenderer
     * \note This function was formerly in Qgs3DAppUtils
     * \since QGIS 3.26
     */
    static std::unique_ptr<QgsPointCloudLayer3DRenderer> convert2DPointCloudRendererTo3D( QgsPointCloudRenderer *renderer );

    /**
     * Casts a \a ray through the \a scene and returns information about the intersecting entities (ray uses World coordinates).
     * The resulting hits are grouped by layer in a QHash.
     * \note Hits on the terrain have nullptr as their key in the returning QHash.
     *
     * \since QGIS 3.32
     */
    static QHash<QgsMapLayer *, QVector<QgsRayCastingUtils::RayHit>> castRay( Qgs3DMapScene *scene, const QgsRay3D &ray, const QgsRayCastingUtils::RayCastContext &context );

    /**
     * Reprojects \a extent from \a crs1 to \a crs2 coordinate reference system with context \a context.
     * If \a crs1 and \a crs2 are identical, \a extent is returned.
     * \param extent extent to reproject
     * \param crs1 source coordinate reference system
     * \param crs2 destination coordinate reference system
     * \param context the context under which the transform is applied
     * \returns reprojected extent. In case of failure, \a extent is returned
     *
     * \since QGIS 3.32
     */
    static QgsRectangle tryReprojectExtent2D( const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs1, const QgsCoordinateReferenceSystem &crs2, const QgsCoordinateTransformContext &context );

    /**
     * This routine approximately calculates how an error (\a epsilon) of an object in world coordinates
     * at given \a distance (between camera and the object) will look like in screen coordinates.
     *
     * \param epsilon error in world coordinates
     * \param distance distance between camera and object
     * \param screenSize screen width or height in pixels
     * \param fov camera's field of view in degrees
     *
     * \since QGIS 3.32
     */
    static float screenSpaceError( float epsilon, float distance, int screenSize, float fov );

    /**
     * This routine computes \a nearPlane \a farPlane from the closest and farthest corners point
     * of bounding box \a bbox.
     * In case of error, fnear will equal 1e9 and ffar 0.
     *
     * \param bbox in world coordinates
     * \param viewMatrix camera view matrix
     * \param fnear near plane
     * \param ffar far plane
     *
     * \since QGIS 3.34
     */
    static void computeBoundingBoxNearFarPlanes( const QgsAABB &bbox, const QMatrix4x4 &viewMatrix, float &fnear, float &ffar );

    /**
     * Converts Qgs3DTypes::CullingMode \a mode into its Qt3D equivalent.
     *
     * \param mode culling mode
     *
     * \since QGIS 3.34
     */
    static Qt3DRender::QCullFace::CullingMode qt3DcullingMode( Qgs3DTypes::CullingMode mode );

    /**
     * Inserts some define macros into a shader source code.
     *
     * \param shaderCode shader code
     * \param defines list of defines to add
     *
     * \since QGIS 3.40
     */
    static QByteArray addDefinesToShaderCode( const QByteArray &shaderCode, const QStringList &defines );

    /**
     * Removes some define macros from a shader source code.
     *
     * \param shaderCode shader code
     * \param defines list of defines to remove
     *
     * \since QGIS 3.40
     */
    static QByteArray removeDefinesFromShaderCode( const QByteArray &shaderCode, const QStringList &defines );

    /**
     * Tries to decompose a 4x4 transform matrix into translation, rotation and scale components.
     * It is expected that the matrix has been created by only applying these transforms, otherwise
     * the results are undefined.
     *
     * \since QGIS 3.42
     */
    static void decomposeTransformMatrix( const QMatrix4x4 &matrix, QVector3D &translation, QQuaternion &rotation, QVector3D &scale );

    /**
     * Gets the maximum number of clip planes that can be used.
     * This value depends on the OpenGL implementation. It should be at least 6.
     *
     * \since QGIS 3.42
     */
    static int openGlMaxClipPlanes( QSurface *surface );

    /**
     * Returns rotation quaternion that performs rotation around X axis by pitchAngle,
     * followed by rotation around Z axis by headingAngle (both angles in degrees).
     *
     * \since QGIS 3.42
     */
    static QQuaternion rotationFromPitchHeadingAngles( float pitchAngle, float headingAngle );
};

#endif // QGS3DUTILS_H
