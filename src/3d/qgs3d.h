/***************************************************************************
                         qgs3d.h
                         --------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3D_H
#define QGS3D_H

#include <memory>

#include "qgis.h"
#include "qgis_3d.h"
#include "qgis_sip.h"

class QgsMaterialRegistry;
class Qgs3DTerrainRegistry;
class QgsAbstractMaterial3DHandler;
class QgsAbstractMaterialSettings;
class QgsMaterial;
class QgsMaterialContext;
class QgsExpressionContext;
class QgsSettingsEntryBool;
template<class T> class QgsSettingsEntryEnumFlag;

#ifndef SIP_RUN
namespace Qt3DCore
{
  class QGeometry;
}

namespace Qt3DRender
{
  class QEffect;
}
#endif

/**
 * \ingroup gui
 * \brief A singleton class containing various registries and other global members
 * related to 3D classes.
 * \since QGIS 3.16
 */
class _3D_EXPORT Qgs3D
{
  public:
    static const QgsSettingsEntryBool *settingMsaaEnabled SIP_SKIP;
    static const QgsSettingsEntryEnumFlag<Qgis::TextureFilterQuality> *settingTextureFilterQuality SIP_SKIP;
    static const QgsSettingsEntryEnumFlag<Qgis::ShadowQuality> *settingShadowQuality SIP_SKIP;

    Qgs3D( const Qgs3D &other ) = delete;
    Qgs3D &operator=( const Qgs3D &other ) = delete;

    /**
     * Returns a pointer to the singleton instance.
     */
    static Qgs3D *instance();

    ~Qgs3D();

    /**
     * Initializes the 3D framework.
     */
    static void initialize();

    /**
     * Returns the material registry, used for managing 3D materials.
     */
    static QgsMaterialRegistry *materialRegistry();

    /**
     * Returns the terrain registry, used for managing 3D terrains.
     */
    static Qgs3DTerrainRegistry *terrainRegistry();

#ifndef SIP_RUN
    /**
     * Creates a new QgsMaterial object representing the material \a settings.
     *
     * The \a technique argument specifies the rendering technique which will be used with the returned
     * material.
     * \since QGIS 4.2
     */
    static QgsMaterial *toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context );

    /**
     * Returns the parameters to be exported to .mtl file
     * \since QGIS 4.2
     */
    static QMap<QString, QString> toMaterialExportParameters( const QgsAbstractMaterialSettings *settings );

    /**
     * Adds parameters from the material \a settings to a destination \a effect.
     * \since QGIS 4.2
     */
    static void addMaterialParametersToEffect( Qt3DRender::QEffect *effect, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &materialContext );

    /**
     * Applies the data defined bytes, \a dataDefinedBytes, on the \a geometry by filling a specific vertex buffer that will be used by the shader.
     * \since QGIS 4.2
     */
    static void applyMaterialDataDefinedToGeometry( const QgsAbstractMaterialSettings *settings, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &dataDefinedBytes );

    /**
     * Returns byte array corresponding to the data defined colors depending of the \a expressionContext,
     * used to fill the specific vertex buffer used for rendering the geometry
     * \since QGIS 4.2
     */
    static QByteArray materialDataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext );

    /**
     * Returns byte stride of the data defined colors,used to fill the vertex colors data defined buffer for rendering
     * \since QGIS 4.2
     */
    static int materialDataDefinedByteStride( const QgsAbstractMaterialSettings *settings );

    /**
     * Returns the handler to use for a material \a settings.
     */
    static const QgsAbstractMaterial3DHandler *handlerForMaterialSettings( const QgsAbstractMaterialSettings *settings );

    // if you change this, also update the corresponding constant in shadows.inc.frag!
    //! Number of shadow map cascades
    static constexpr int NUM_SHADOW_CASCADES = 4;

#endif

  private:
    Qgs3D();

#ifdef SIP_RUN
    Qgs3D( const Qgs3D &other );
#endif

    bool mInitialized = false;

    Qgs3DTerrainRegistry *mTerrainRegistry = nullptr;
    std::unique_ptr< QgsAbstractMaterial3DHandler > mNullMaterialHandler;
    std::unique_ptr< QgsAbstractMaterial3DHandler > mPhongMaterialHandler;
    std::unique_ptr< QgsAbstractMaterial3DHandler > mPhongTexturedMaterialHandler;
    std::unique_ptr< QgsAbstractMaterial3DHandler > mSimpleLineMaterialHandler;
    std::unique_ptr< QgsAbstractMaterial3DHandler > mGoochMaterialHandler;
    std::unique_ptr< QgsAbstractMaterial3DHandler > mMetalRoughMaterialHandler;
    std::unique_ptr< QgsAbstractMaterial3DHandler > mMetalRoughTexturedMaterialHandler;
};

#endif // QGS3D_H
