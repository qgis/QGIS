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

class QDomElement;
class QgsReadWriteContext;
class QgsLineMaterial;


/**
 * Material rendering techniques
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
};


/**
 * \ingroup 3d
 * Context settings for a material.
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
 * Abstract base class for material settings.
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
    virtual void readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;

    //! Writes settings to a DOM \a element
    virtual void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const = 0;

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
#endif

};


#endif // QGSABSTRACTMATERIALSETTINGS_H
