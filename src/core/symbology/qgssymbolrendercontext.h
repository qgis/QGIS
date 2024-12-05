/***************************************************************************
 qgssymbolrendercontext.h
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLRENDERCONTEXT_H
#define QGSSYMBOLRENDERCONTEXT_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmapunitscale.h"
#include "qgsfields.h"

class QgsRenderContext;
class QgsFeature;
class QgsLegendPatchShape;
class QgsExpressionContextScope;

/**
 * \ingroup core
 * \class QgsSymbolRenderContext
 */
class CORE_EXPORT QgsSymbolRenderContext
{
  public:

    //TODO QGIS 4.0 - remove mapUnitScale and renderunit

    /**
     * Constructor for QgsSymbolRenderContext
     * \param c
     * \param u
     * \param opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \param selected set to TRUE if symbol should be drawn in a "selected" state
     * \param renderHints flags controlling rendering behavior
     * \param f
     * \param fields
     * \param mapUnitScale
     */
    QgsSymbolRenderContext( QgsRenderContext &c, Qgis::RenderUnit u, qreal opacity = 1.0, bool selected = false, Qgis::SymbolRenderHints renderHints = Qgis::SymbolRenderHints(), const QgsFeature *f = nullptr, const QgsFields &fields = QgsFields(), const QgsMapUnitScale &mapUnitScale = QgsMapUnitScale() );

    ~QgsSymbolRenderContext();

    QgsSymbolRenderContext( const QgsSymbolRenderContext &rh ) = delete;

    /**
     * Returns a reference to the context's render context.
     */
    QgsRenderContext &renderContext() { return mRenderContext; }

    /**
     * Returns a reference to the context's render context.
     * \note Not available in Python bindings.
     */
    const QgsRenderContext &renderContext() const SIP_SKIP { return mRenderContext; }

    /**
     * Sets the original value variable value for data defined symbology
     * \param value value for original value variable. This usually represents the symbol property value
     * before any data defined overrides have been applied.
     */
    void setOriginalValueVariable( const QVariant &value );

    /**
     * Returns the output unit for the context.
     * \deprecated QGIS 3.40. No longer used and will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED Qgis::RenderUnit outputUnit() const SIP_DEPRECATED { return mOutputUnit; }

    /**
     * Sets the output unit for the context.
     * \deprecated QGIS 3.40. No longer used and will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED void setOutputUnit( Qgis::RenderUnit u ) SIP_DEPRECATED { mOutputUnit = u; }

    /**
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED QgsMapUnitScale mapUnitScale() const SIP_DEPRECATED { return mMapUnitScale; }

    /**
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED void setMapUnitScale( const QgsMapUnitScale &scale ) SIP_DEPRECATED { mMapUnitScale = scale; }

    /**
     * Returns the opacity for the symbol.
     * \returns opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see setOpacity()
     */
    qreal opacity() const { return mOpacity; }

    /**
     * Sets the \a opacity for the symbol.
     * \param opacity opacity value between 0 (fully transparent) and 1 (fully opaque)
     * \see opacity()
     */
    void setOpacity( qreal opacity ) { mOpacity = opacity; }

    /**
     * Returns TRUE if symbols should be rendered using the selected symbol coloring and style.
     * \see setSelected()
     */
    bool selected() const { return mSelected; }

    /**
     * Sets whether symbols should be rendered using the selected symbol coloring and style.
     * \see selected()
     */
    void setSelected( bool selected ) { mSelected = selected; }

    /**
     * Returns the rendering hint flags for the symbol.
     * \see setRenderHints()
     */
    Qgis::SymbolRenderHints renderHints() const { return mRenderHints; }

    /**
     * Returns TRUE if symbol must be rendered using vector methods, and optimisations
     * like pre-rendered images must be disabled.
     *
     * \since QGIS 3.40
     */
    bool forceVectorRendering() const;

    /**
     * Sets rendering hint flags for the symbol.
     * \see renderHints()
     */
    void setRenderHints( Qgis::SymbolRenderHints hints ) { mRenderHints = hints; }

    /**
     * Sets a rendering \a hint flag for the symbol.
     * \see renderHints()
     *
     * \since QGIS 3.40
     */
    void setRenderHint( Qgis::SymbolRenderHint hint, bool enabled = true ) { mRenderHints.setFlag( hint, enabled ); }

    void setFeature( const QgsFeature *f ) { mFeature = f; }

    /**
     * Returns the current feature being rendered. This may be NULLPTR.
     */
    const QgsFeature *feature() const { return mFeature; }

    /**
     * Sets the geometry type for the original feature geometry being rendered.
     * \see originalGeometryType()
     */
    void setOriginalGeometryType( Qgis::GeometryType type ) { mOriginalGeometryType = type; }

    /**
     * Returns the geometry type for the original feature geometry being rendered. This can be
     * useful if symbol layers alter their appearance based on geometry type - eg offsetting a
     * simple line style will look different if the simple line is rendering a polygon feature
     * (a closed buffer) vs a line feature (an unclosed offset line).
     * \see originalGeometryType()
     */
    Qgis::GeometryType originalGeometryType() const { return mOriginalGeometryType; }

    /**
     * Fields of the layer. Currently only available in startRender() calls
     * to allow symbols with data-defined properties prepare the expressions
     * (other times fields() returns an empty QgsFields object).
     */
    QgsFields fields() const { return mFields; }

    /**
     * Part count of current geometry
     */
    int geometryPartCount() const { return mGeometryPartCount; }

    /**
     * Sets the part count of current geometry
     */
    void setGeometryPartCount( int count ) { mGeometryPartCount = count; }

    /**
     * Part number of current geometry
     */
    int geometryPartNum() const { return mGeometryPartNum; }

    /**
     * Sets the part number of current geometry
     */
    void setGeometryPartNum( int num ) { mGeometryPartNum = num; }

    /**
     * \deprecated QGIS 3.40. Use the size conversion methods in QgsRenderContext instead.
     */
    Q_DECL_DEPRECATED double outputLineWidth( double width ) const SIP_DEPRECATED;

    /**
     * \deprecated QGIS 3.40. Use the size conversion methods in QgsRenderContext instead.
     */
    Q_DECL_DEPRECATED double outputPixelSize( double size ) const SIP_DEPRECATED;

    // workaround for sip 4.7. Don't use assignment - will fail with assertion error
    QgsSymbolRenderContext &operator=( const QgsSymbolRenderContext & );

    /**
     * This scope is always available when a symbol of this type is being rendered.
     *
     * \returns An expression scope for details about this symbol
     */
    QgsExpressionContextScope *expressionContextScope();

    /**
     * Set an expression scope for this symbol.
     *
     * Will take ownership.
     *
     * \param contextScope An expression scope for details about this symbol
     */
    void setExpressionContextScope( QgsExpressionContextScope *contextScope SIP_TRANSFER );

    /**
     * Returns the symbol patch shape, to use if rendering symbol preview icons.
     *
     * \see setPatchShape()
     * \since QGIS 3.14
     */
    const QgsLegendPatchShape *patchShape() const;

    /**
     * Sets the symbol patch \a shape, to use if rendering symbol preview icons.
     *
     * \see patchShape()
     * \since QGIS 3.14
     */
    void setPatchShape( const QgsLegendPatchShape &shape );

  private:

#ifdef SIP_RUN
    QgsSymbolRenderContext( const QgsSymbolRenderContext &rh ) SIP_FORCE;
#endif

    QgsRenderContext &mRenderContext;
    std::unique_ptr< QgsExpressionContextScope > mExpressionContextScope;
    Qgis::RenderUnit mOutputUnit;
    QgsMapUnitScale mMapUnitScale;
    qreal mOpacity = 1.0;
    bool mSelected;
    Qgis::SymbolRenderHints mRenderHints;
    const QgsFeature *mFeature; //current feature
    QgsFields mFields;
    int mGeometryPartCount;
    int mGeometryPartNum;
    Qgis::GeometryType mOriginalGeometryType = Qgis::GeometryType::Unknown;
    std::unique_ptr< QgsLegendPatchShape > mPatchShape;
};


#endif // QGSSYMBOLRENDERCONTEXT_H

