/***************************************************************************
                              qgspointdistancerenderer.cpp
                              ----------------------------
  begin                : January 26, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTDISTANCERENDERER_H
#define QGSPOINTDISTANCERENDERER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsrenderer.h"
#include "qgsmapunitscale.h"
#include <QFont>

class QgsSpatialIndex;
class QgsMarkerSymbol;
class QgsSymbolRenderContext;

/**
 * \class QgsPointDistanceRenderer
 * \ingroup core
 * \brief An abstract base class for distance based point renderers (e.g., clusterer and displacement renderers).
 * QgsPointDistanceRenderer handles calculation of point clusters using a distance based threshold.
 * Subclasses must implement drawGroup() to handle the rendering of individual point clusters
 * in the desired style.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsPointDistanceRenderer: public QgsFeatureRenderer
{
  public:

    //! Contains properties for a feature within a clustered group.
    struct CORE_EXPORT GroupedFeature
    {

        /**
         * Constructor for GroupedFeature.
        * \param feature feature
        * \param symbol base symbol for rendering feature (owned by GroupedFeature)
        * \param isSelected set to TRUE if feature is selected and should be rendered in a selected state
        * \param label optional label text, or empty string for no label
        */
        GroupedFeature( const QgsFeature &feature, QgsMarkerSymbol *symbol SIP_TRANSFER, bool isSelected, const QString &label = QString() );
        ~GroupedFeature();

        //! Feature
        QgsFeature feature;

        //! Base symbol for rendering feature
        QgsMarkerSymbol *symbol() const { return mSymbol.get(); }

        //! True if feature is selected and should be rendered in a selected state
        bool isSelected;

        //! Optional label text
        QString label;

      private:
        std::shared_ptr< QgsMarkerSymbol > mSymbol;
    };

    //! A group of clustered points (ie features within the distance tolerance).
    typedef QList< QgsPointDistanceRenderer::GroupedFeature > ClusteredGroup;

    /**
     * Constructor for QgsPointDistanceRenderer.
     * \param rendererName name of renderer for registry
     * \param labelAttributeName optional attribute for labeling points
     */
    QgsPointDistanceRenderer( const QString &rendererName, const QString &labelAttributeName = QString() );

    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const override;
    bool renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false ) override SIP_THROW( QgsCsException );
    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool filterNeedsGeometry() const override;
    QgsFeatureRenderer::Capabilities capabilities() override;
    QgsSymbolList symbols( QgsRenderContext &context ) const override;
    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QgsSymbol *originalSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QgsSymbolList symbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QgsSymbolList originalSymbolsForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QSet< QString > legendKeysForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QString legendKeyToExpression( const QString &key, QgsVectorLayer *layer, bool &ok ) const override;
    bool willRenderFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    void stopRender( QgsRenderContext &context ) override;
    QgsLegendSymbolList legendSymbolItems() const override;
    void setEmbeddedRenderer( QgsFeatureRenderer *r SIP_TRANSFER ) override;
    const QgsFeatureRenderer *embeddedRenderer() const override;
    void setLegendSymbolItem( const QString &key, QgsSymbol *symbol SIP_TRANSFER ) override;
    bool legendSymbolItemsCheckable() const override;
    bool legendSymbolItemChecked( const QString &key ) override;
    void checkLegendSymbolItem( const QString &key, bool state ) override;
    QString filter( const QgsFields &fields = QgsFields() ) override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

    /**
     * Sets the attribute name for labeling points.
     * \param name attribute name, or empty string to avoid labeling features by the renderer
     * \see labelAttributeName()
     * \see setLabelFont()
     * \see setLabelColor()
     * \see setMinimumLabelScale()
     */
    void setLabelAttributeName( const QString &name ) { mLabelAttributeName = name; }

    /**
     * Returns the attribute name used for labeling points, or an empty string if no labeling
     * will be done by the renderer.
     * \see setLabelAttributeName()
     * \see labelFont()
     * \see minimumLabelScale()
     * \see labelColor()
     */
    QString labelAttributeName() const { return mLabelAttributeName; }

    /**
     * Sets the font used for labeling points.
     * \param font label font
     * \see labelFont()
     * \see setLabelAttributeName()
     * \see setLabelColor()
     */
    void setLabelFont( const QFont &font ) { mLabelFont = font; }

    /**
     * Returns the font used for labeling points.
     * \see setLabelFont()
     * \see labelAttributeName()
     * \see labelColor()
     */
    QFont labelFont() const { return mLabelFont;}

    /**
     * Sets the minimum map \a scale (i.e. most "zoomed out") at which points should be labeled by the renderer.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see minimumLabelScale()
     * \see setLabelAttributeName()
     */
    void setMinimumLabelScale( double scale ) { mMinLabelScale = scale; }

    /**
     * Returns the minimum map scale (i.e. most "zoomed out") at which points should be labeled by the renderer.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see setMinimumLabelScale()
     * \see labelAttributeName()
     */
    double minimumLabelScale() const { return mMinLabelScale; }

    /**
     * Sets the color to use for for labeling points.
     * \param color label color
     * \see labelColor()
     * \see setLabelAttributeName()
     * \see setLabelFont()
     */
    void setLabelColor( const QColor &color ) { mLabelColor = color;}

    /**
     * Returns the color used for for labeling points.
     * \see setLabelColor()
     * \see labelAttributeName()
     * \see labelFont()
     */
    QColor labelColor() const { return mLabelColor; }

    /**
     * Sets the tolerance distance for grouping points. Units are specified using
     * setToleranceUnit().
     * \param distance tolerance distance
     * \see tolerance()
     * \see setToleranceUnit()
     */
    void setTolerance( double distance ) { mTolerance = distance; }

    /**
     * Returns the tolerance distance for grouping points. Units are retrieved using
     * toleranceUnit().
     * \see setTolerance()
     * \see toleranceUnit()
     */
    double tolerance() const { return mTolerance; }

    /**
     * Sets the units for the tolerance distance.
     * \param unit tolerance distance units
     * \see setTolerance()
     * \see toleranceUnit()
     * \since QGIS 2.12
     */
    void setToleranceUnit( QgsUnitTypes::RenderUnit unit ) { mToleranceUnit = unit; }

    /**
     * Returns the units for the tolerance distance.
     * \see tolerance()
     * \see setToleranceUnit()
     * \since QGIS 2.12
     */
    QgsUnitTypes::RenderUnit toleranceUnit() const { return mToleranceUnit; }

    /**
     * Sets the map unit scale object for the distance tolerance. This is only used if the
     * toleranceUnit() is set to QgsUnitTypes::RenderMapUnits.
     * \param scale scale for distance tolerance
     * \see toleranceMapUnitScale()
     * \see setToleranceUnit()
     */
    void setToleranceMapUnitScale( const QgsMapUnitScale &scale ) { mToleranceMapUnitScale = scale; }

    /**
     * Returns the map unit scale object for the distance tolerance. This is only used if the
     * toleranceUnit() is set to QgsUnitTypes::RenderMapUnits.
     * \see setToleranceMapUnitScale()
     * \see toleranceUnit()
     */
    const QgsMapUnitScale &toleranceMapUnitScale() const { return mToleranceMapUnitScale; }

  protected:

    //! Embedded base renderer. This can be used for rendering individual, isolated points.
    std::unique_ptr< QgsFeatureRenderer > mRenderer;

    //! Attribute name for labeling. An empty string indicates that no labels should be rendered.
    QString mLabelAttributeName;

    //! Label attribute index (or -1 if none). This index is not stored, it is requested in the startRender() method.
    int mLabelIndex;

    //! Distance tolerance. Points that are closer together than this distance are considered clustered.
    double mTolerance;
    //! Unit for distance tolerance.
    QgsUnitTypes::RenderUnit mToleranceUnit;
    //! Map unit scale for distance tolerance.
    QgsMapUnitScale mToleranceMapUnitScale;

    //! Label font.
    QFont mLabelFont;
    //! Label text color.
    QColor mLabelColor;
    //! Whether labels should be drawn for points. This is set internally from startRender() depending on scale denominator.
    bool mDrawLabels;
    //! Maximum scale denominator for label display. A zero value indicates no scale limitation.
    double mMinLabelScale = 0;

    //! Groups of features that are considered clustered together.
    QList<ClusteredGroup> mClusteredGroups;

    //! Mapping of feature ID to the feature's group index.
    QMap<QgsFeatureId, int> mGroupIndex;

    //! Mapping of feature ID to approximate group location
    QMap<QgsFeatureId, QgsPointXY > mGroupLocations;

    //! Spatial index for fast lookup of nearby points.
    QgsSpatialIndex *mSpatialIndex = nullptr;

    /**
     * Renders the labels for a group.
     * \param centerPoint center point of group
     * \param context destination render context
     * \param labelShifts displacement for individual label positions
     * \param group group of clustered features to label
     * \note may not be available in Python bindings on some platforms
     */
    void drawLabels( QPointF centerPoint, QgsSymbolRenderContext &context, const QList<QPointF> &labelShifts, const ClusteredGroup &group ) const;

  private:

    /**
     * Draws a group of clustered points.
     * \param centerPoint central point (geographic centroid) of all points contained within the cluster
     * \param context destination render context
     * \param group contents of group
     */
    virtual void drawGroup( QPointF centerPoint, QgsRenderContext &context, const ClusteredGroup &group ) const = 0 SIP_FORCE;

    //! Creates a search rectangle with specified distance tolerance.
    QgsRectangle searchRect( const QgsPointXY &p, double distance ) const;

    //! Debugging function to check the entries in the clustered groups
    void printGroupInfo() const;

    //! Returns the label text for a feature (using mLabelAttributeName as attribute field)
    QString getLabel( const QgsFeature &feature ) const;

    //! Internal group rendering helper
    void drawGroup( const ClusteredGroup &group, QgsRenderContext &context ) const;

    /**
     * Returns first symbol from the embedded renderer for a feature or NULLPTR if none
     * \param feature source feature
     * \param context target render context
    */
    QgsMarkerSymbol *firstSymbolForFeature( const QgsFeature &feature, QgsRenderContext &context );

    /**
     * Creates an expression context scope for a clustered group, with variables reflecting the group's properties.
     * \param group clustered group
     * \returns new expression context scope
     */
    QgsExpressionContextScope *createGroupScope( const ClusteredGroup &group ) const;

};

#endif // QGSPOINTDISTANCERENDERER_H
