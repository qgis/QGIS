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

#include "qgsrenderer.h"
#include <QFont>

class QgsSpatialIndex;

/** \class QgsPointDistanceRenderer
 * \ingroup core
 * An abstract base class for distance based point renderers (eg clusterer and displacement renderers).
 * QgsPointDistanceRenderer handles calculation of point clusters using a distance based threshold.
 * Subclasses must implement drawGroup() to handle the rendering of individual point clusters
 * in the desired style.
 * \note added in QGIS 3.0
 */

class CORE_EXPORT QgsPointDistanceRenderer: public QgsFeatureRenderer
{
  public:

    //! Contains properties for a feature within a clustered group.
    struct GroupedFeature
    {
      /** Constructor for GroupedFeature.
      * @param feature feature
      * @param symbol base symbol for rendering feature
      * @param isSelected set to true if feature is selected and should be rendered in a selected state
      * @param label optional label text, or empty string for no label
      */
      GroupedFeature( const QgsFeature& feature, QgsMarkerSymbol* symbol, bool isSelected, const QString& label = QString() )
          : feature( feature )
          , symbol( symbol )
          , isSelected( isSelected )
          , label( label )
      {}

      //! Feature
      QgsFeature feature;

      //! Base symbol for rendering feature
      QgsMarkerSymbol* symbol;

      //! True if feature is selected and should be rendered in a selected state
      bool isSelected;

      //! Optional label text
      QString label;
    };

    //! A group of clustered points (ie features within the distance tolerance).
    typedef QList< GroupedFeature > ClusteredGroup;

    /** Constructor for QgsPointDistanceRenderer.
     * @param rendererName name of renderer for registry
     * @param labelAttributeName optional attribute for labeling points
     */
    QgsPointDistanceRenderer( const QString& rendererName, const QString& labelAttributeName = QString() );

    virtual void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props = QgsStringMap() ) const override;
    bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false ) override;
    virtual QSet<QString> usedAttributes() const override;
    virtual Capabilities capabilities() override;
    virtual QgsSymbolList symbols( QgsRenderContext& context ) override;
    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    virtual QgsSymbolList symbolsForFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    virtual QgsSymbolList originalSymbolsForFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    virtual bool willRenderFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;
    void stopRender( QgsRenderContext& context ) override;
    QgsLegendSymbologyList legendSymbologyItems( QSize iconSize ) override;
    QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = "" ) override;
    void setEmbeddedRenderer( QgsFeatureRenderer* r ) override;
    const QgsFeatureRenderer* embeddedRenderer() const override;
    void setLegendSymbolItem( const QString& key, QgsSymbol* symbol ) override;
    bool legendSymbolItemsCheckable() const override;
    bool legendSymbolItemChecked( const QString& key ) override;
    void checkLegendSymbolItem( const QString& key, bool state ) override;
    virtual QString filter( const QgsFields& fields = QgsFields() ) override;

    /** Sets the attribute name for labeling points.
     * @param name attribute name, or empty string to avoid labeling features by the renderer
     * @see labelAttributeName()
     * @see setLabelFont()
     * @see setLabelColor()
     * @see setMaxLabelScaleDenominator()
     */
    void setLabelAttributeName( const QString& name ) { mLabelAttributeName = name; }

    /** Returns the attribute name used for labeling points, or an empty string if no labeling
     * will be done by the renderer.
     * @see setLabelAttributeName()
     * @see labelFont()
     * @see maxLabelScaleDenominator()
     * @see labelColor()
     */
    QString labelAttributeName() const { return mLabelAttributeName; }

    /** Sets the font used for labeling points.
     * @param font label font
     * @see labelFont()
     * @see setLabelAttributeName()
     * @see setLabelColor()
     */
    void setLabelFont( const QFont& font ) { mLabelFont = font; }

    /** Returns the font used for labeling points.
     * @see setLabelFont()
     * @see labelAttributeName()
     * @see labelColor()
     */
    QFont labelFont() const { return mLabelFont;}

    /** Sets the maximum scale at which points should be labeled by the renderer.
     * @param denominator maximum scale denominator
     * @see maxLabelScaleDenominator()
     * @see setLabelAttributeName()
     */
    void setMaxLabelScaleDenominator( double denominator ) { mMaxLabelScaleDenominator = denominator; }

    /** Returns the denominator for the maximum scale at which points should be labeled by the renderer.
     * @see setMaxLabelScaleDenominator()
     * @see labelAttributeName()
     */
    double maxLabelScaleDenominator() const { return mMaxLabelScaleDenominator; }

    /** Sets the color to use for for labeling points.
     * @param color label color
     * @see labelColor()
     * @see setLabelAttributeName()
     * @see setLabelFont()
     */
    void setLabelColor( const QColor& color ) { mLabelColor = color;}

    /** Returns the color used for for labeling points.
     * @see setLabelColor()
     * @see labelAttributeName()
     * @see labelFont()
     */
    QColor labelColor() const { return mLabelColor; }

    /** Sets the tolerance distance for grouping points. Units are specified using
     * setToleranceUnit().
     * @param distance tolerance distance
     * @see tolerance()
     * @see setToleranceUnit()
     */
    void setTolerance( double distance ) { mTolerance = distance; }

    /** Returns the tolerance distance for grouping points. Units are retrieved using
     * toleranceUnit().
     * @see setTolerance()
     * @see toleranceUnit()
     */
    double tolerance() const { return mTolerance; }

    /** Sets the units for the tolerance distance.
     * @param unit tolerance distance units
     * @see setTolerance()
     * @see toleranceUnit()
     * @note added in QGIS 2.12
     */
    void setToleranceUnit( QgsUnitTypes::RenderUnit unit ) { mToleranceUnit = unit; }

    /** Returns the units for the tolerance distance.
     * @see tolerance()
     * @see setToleranceUnit()
     * @note added in QGIS 2.12
     */
    QgsUnitTypes::RenderUnit toleranceUnit() const { return mToleranceUnit; }

    /** Sets the map unit scale object for the distance tolerance. This is only used if the
     * toleranceUnit() is set to QgsUnitTypes::RenderMapUnits.
     * @param scale scale for distance tolerance
     * @see toleranceMapUnitScale()
     * @see setToleranceUnit()
     */
    void setToleranceMapUnitScale( const QgsMapUnitScale& scale ) { mToleranceMapUnitScale = scale; }

    /** Returns the map unit scale object for the distance tolerance. This is only used if the
     * toleranceUnit() is set to QgsUnitTypes::RenderMapUnits.
     * @see setToleranceMapUnitScale()
     * @see toleranceUnit()
     */
    const QgsMapUnitScale& toleranceMapUnitScale() const { return mToleranceMapUnitScale; }

  protected:

    //! Embedded base renderer. This can be used for rendering individual, isolated points.
    QScopedPointer< QgsFeatureRenderer > mRenderer;

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
    //! Maximum scale denominator for label display. A negative number indicatese no scale limitation.
    double mMaxLabelScaleDenominator;

    //! Groups of features that are considered clustered together.
    QList<ClusteredGroup> mClusteredGroups;

    //! Mapping of feature ID to the feature's group index.
    QMap<QgsFeatureId, int> mGroupIndex;

    //! Mapping of feature ID to approximate group location
    QMap<QgsFeatureId, QgsPoint > mGroupLocations;

    //! Spatial index for fast lookup of nearby points.
    QgsSpatialIndex* mSpatialIndex;

    /** Renders the labels for a group.
     * @param centerPoint center point of group
     * @param context destination render context
     * @param labelShifts displacement for individual label positions
     * @param group group of clustered features to label
     * @note may not be available in Python bindings on some platforms
     */
    void drawLabels( QPointF centerPoint, QgsSymbolRenderContext& context, const QList<QPointF>& labelShifts, const ClusteredGroup& group );

  private:

    /** Draws a group of clustered points.
     * @param centerPoint central point (geographic centroid) of all points contained within the cluster
     * @param context destination render context
     * @param group contents of group
     */
    virtual void drawGroup( QPointF centerPoint, QgsRenderContext& context, const ClusteredGroup& group ) = 0;

    //! Creates a search rectangle with specified distance tolerance.
    QgsRectangle searchRect( const QgsPoint& p, double distance ) const;

    //! Debugging function to check the entries in the clustered groups
    void printGroupInfo() const;

    //! Returns the label text for a feature (using mLabelAttributeName as attribute field)
    QString getLabel( const QgsFeature& feature ) const;

    //! Internal group rendering helper
    void drawGroup( const ClusteredGroup& group, QgsRenderContext& context );

    /** Returns first symbol from the embedded renderer for a feature or nullptr if none
     * @param feature source feature
     * @param context target render context
    */
    QgsMarkerSymbol* firstSymbolForFeature( QgsFeature& feature, QgsRenderContext& context );

    /** Creates an expression context scope for a clustered group, with variables reflecting the group's properties.
     * @param group clustered group
     * @returns new expression context scope
     */
    QgsExpressionContextScope* createGroupScope( const ClusteredGroup& group ) const;

};

#endif // QGSPOINTDISTANCERENDERER_H
