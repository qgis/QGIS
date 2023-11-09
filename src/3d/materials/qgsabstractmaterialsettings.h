/***************************************************************************
  qgsabstractmaterialsettings.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTMATERIALSETTINGS_H
#define QGSABSTRACTMATERIALSETTINGS_H

#include "qgis_3d.h"
#include "qgis_sip.h"

#include <QColor>
#include <Qt3DRender/qmaterial.h>
#include "qgspropertycollection.h"

class QDomElement;
class QgsReadWriteContext;
class QgsLineMaterial;
class QgsExpressionContext;

#ifndef SIP_RUN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace Qt3DRender
{
  class QGeometry;
}
#else
namespace Qt3DCore
{
  class QGeometry;
}
#endif
#endif //SIP_RUN

/**
 * \brief Material rendering techniques
 * \ingroup 3d
 * \since QGIS 3.16
 */
enum class QgsMaterialSettingsRenderingTechnique SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsAbstractMaterialSettings, RenderingTechnique ): int
  {
  Triangles, //!< Triangle based rendering (default)
  Lines, //!< Line based rendering, requires line data
  InstancedPoints, //!< Instanced based rendering, requiring triangles and point data
  Points, //!< Point based rendering, requires point data
  TrianglesWithFixedTexture, //!< Triangle based rendering, using a fixed, non-user-configurable texture (e.g. for terrain rendering)
  TrianglesFromModel, //!< Triangle based rendering, using a model object source
  TrianglesDataDefined, //!< Triangle based rendering with possibility of datadefined color \since QGIS 3.18
};


/**
 * \ingroup 3d
 * \brief Context settings for a material.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsMaterialContext
{
  public:

    /**
     * Returns TRUE if the material should represent a selected state.
     *
     * \see setIsSelected()
     */
    bool isSelected() const { return mIsSelected; }

    /**
     * Sets whether the material should represent a selected state.
     *
     * \see isSelected()
     */
    void setIsSelected( bool isSelected ) { mIsSelected = isSelected; }

    /**
     * Returns the color for representing materials in a selected state.
     *
     * \see setSelectionColor()
     */
    QColor selectionColor() const { return mSelectedColor; }

    /**
     * Sets the color for representing materials in a selected state.
     *
     * \see selectionColor()
     */
    void setSelectionColor( const QColor &color ) { mSelectedColor = color; }

  private:

    bool mIsSelected = false;

    QColor mSelectedColor;

};


/**
 * \ingroup 3d
 * \brief Abstract base class for material settings.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsAbstractMaterialSettings SIP_ABSTRACT
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == QLatin1String( "gooch" ) )
    {
      sipType = sipType_QgsGoochMaterialSettings;
    }
    else if ( sipCpp->type() == QLatin1String( "phong" ) )
    {
      sipType = sipType_QgsPhongMaterialSettings;
    }
    else if ( sipCpp->type() == "phongtextured" )
    {
      sipType = sipType_QgsPhongTexturedMaterialSettings;
    }
    else if ( sipCpp->type() == "simpleline" )
    {
      sipType = sipType_QgsSimpleLineMaterialSettings;
    }
    else if ( sipCpp->type() == "null" )
    {
      sipType = sipType_QgsNullMaterialSettings;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    virtual ~QgsAbstractMaterialSettings() = default;

    /**
     * Returns the unique type name for the material.
     */
    virtual QString type() const = 0;

    /**
     * Clones the material settings.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsAbstractMaterialSettings *clone() const = 0 SIP_FACTORY;

    //! Reads settings from a DOM \a element
    virtual void readXml( const QDomElement &element, const QgsReadWriteContext & );

    //! Writes settings to a DOM \a element
    virtual void writeXml( QDomElement &element, const QgsReadWriteContext & ) const;

#ifndef SIP_RUN

    /**
     * Creates a new QMaterial object representing the material settings.
     *
     * The \a technique argument specifies the rendering technique which will be used with the returned
     * material.
     */
    virtual Qt3DRender::QMaterial *toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const = 0 SIP_FACTORY;

    /**
     * Returns the parameters to be exported to .mtl file
     */
    virtual QMap<QString, QString> toExportParameters() const = 0;

    /**
     * Adds parameters from the material to a destination \a effect.
     */
    virtual void addParametersToEffect( Qt3DRender::QEffect *effect ) const = 0;

    //! Data definable properties.
    enum Property
    {
      Diffuse, //!< Diffuse color
      Ambient, //!< Ambient color (phong material)
      Warm, //!< Warm color (gooch material)
      Cool,//!< Cool color (gooch material)
      Specular //!< Specular color
    };

    /**
     * Sets the material property collection, used for data defined overrides.
     * \since QGIS 3.18
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection );

    /**
    * Returns the symbol material property collection, used for data defined overrides.
    * \since QGIS 3.18
    */
    QgsPropertyCollection dataDefinedProperties() const;

    /**
    * Returns a reference to the material properties definition, used for data defined overrides.
    * \since QGIS 3.18
    */
    const QgsPropertiesDefinition  &propertyDefinitions() const;

    /**
     * Applies the data defined bytes, \a dataDefinedBytes, on the \a geometry by filling a specific vertex buffer that will be used by the shader.
     * \since QGIS 3.18
     */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    virtual void applyDataDefinedToGeometry( Qt3DRender::QGeometry *geometry, int vertexCount, const QByteArray &dataDefinedBytes ) const;
#else
    virtual void applyDataDefinedToGeometry( Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &dataDefinedBytes ) const;
#endif

    /**
     * Returns byte array corresponding to the data defined colors depending of the \a expressionContext,
     * used to fill the specific vertex buffer used for rendering the geometry
     * \see applyDataDefinedToGeometry()
     * \since QGIS 3.18
     */
    virtual QByteArray dataDefinedVertexColorsAsByte( const QgsExpressionContext &expressionContext ) const;

    /**
     * Returns byte stride of the data defined colors,used to fill the vertex colors data defined buffer for rendering
     * \since QGIS 3.18
     */
    virtual int dataDefinedByteStride() const {return 0;}
#endif

  private:
    QgsPropertyCollection mDataDefinedProperties;
    static QgsPropertiesDefinition sPropertyDefinitions;
    void initPropertyDefinitions() const;
};


#endif // QGSABSTRACTMATERIALSETTINGS_H
