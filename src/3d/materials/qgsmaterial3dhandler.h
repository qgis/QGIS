/***************************************************************************
  qgsmaterial3dhandler.h
  --------------------------------------
  Date                 : March 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMATERIAL3DHANDLER_H
#define QGSMATERIAL3DHANDLER_H

#include "qgis_3d.h"
#include "qgis_sip.h"
#include "qgsmaterial.h"
#include "qgspropertycollection.h"

#include <QColor>
#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;

class QDomElement;
class QgsReadWriteContext;
class QgsLineMaterial;
class QgsExpressionContext;
class QgsAbstractMaterialSettings;

namespace Qt3DCore
{
  class QGeometry;
}

/**
 * \ingroup qgis_3d
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

    /**
     * Returns TRUE if the material should represent a highlighted state.
     *
     * \see setIsHighlighted()
     * \since QGIS 4.0
     */
    bool isHighlighted() const { return mIsHighlighted; }

    /**
     * Sets whether the material should represent a highlighted state.
     *
     * \see isHighlighted()
     * \since QGIS 4.0
     */
    void setIsHighlighted( bool isHighlighted ) { mIsHighlighted = isHighlighted; }

  private:
    bool mIsSelected = false;
    bool mIsHighlighted = false;

    QColor mSelectedColor;
};


/**
 * \ingroup qgis_3d
 * \brief Abstract base class for material 3D handlers.
 *
 * \warning Not available in Python bindings
 *
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsAbstractMaterial3DHandler SIP_ABSTRACT
{
  public:
    virtual ~QgsAbstractMaterial3DHandler() = default;

    /**
     * Creates a new QgsMaterial object representing the material \a settings.
     *
     * The \a technique argument specifies the rendering technique which will be used with the returned
     * material.
     */
    virtual QgsMaterial *toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const = 0 SIP_FACTORY;

    /**
     * Returns the parameters to be exported to .mtl file
     */
    virtual QMap<QString, QString> toExportParameters( const QgsAbstractMaterialSettings *settings ) const = 0;

    /**
     * Adds parameters from the material \a settings to a destination \a effect.
     */
    virtual void addParametersToEffect( Qt3DRender::QEffect *effect, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &materialContext ) const = 0;

    /**
     * Applies the data defined bytes, \a dataDefinedBytes, on the \a geometry by filling a specific vertex buffer that will be used by the shader.
     */
    virtual void applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *settings, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &dataDefinedBytes ) const;

    /**
     * Returns byte array corresponding to the data defined colors depending of the \a expressionContext,
     * used to fill the specific vertex buffer used for rendering the geometry
     * \see applyDataDefinedToGeometry()
     * \since QGIS 3.18
     */
    virtual QByteArray dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const;

    /**
     * Returns byte stride of the data defined colors,used to fill the vertex colors data defined buffer for rendering
     * \since QGIS 3.18
     */
    virtual int dataDefinedByteStride( const QgsAbstractMaterialSettings *settings ) const;
};


#endif // QGSMATERIAL3DHANDLER_H
