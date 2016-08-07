/***************************************************************************
                              qgspointdisplacementrenderer.cpp
                              --------------------------------
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

#ifndef QGSPOINTDISPLACEMENTRENDERER_H
#define QGSPOINTDISPLACEMENTRENDERER_H

#include "qgsfeature.h"
#include "qgssymbol.h"
#include "qgsrenderer.h"
#include <QFont>
#include <QSet>

class QgsSpatialIndex;

/** \ingroup core
 * A renderer that automatically displaces points with the same position
*/
class CORE_EXPORT QgsPointDisplacementRenderer: public QgsFeatureRenderer
{
  public:

    /** Placement methods for dispersing points
     */
    enum Placement
    {
      Ring, /*!< Place points in a single ring around group*/
      ConcentricRings /*!< Place points in concentric rings around group*/
    };

    QgsPointDisplacementRenderer( const QString& labelAttributeName = "" );
    ~QgsPointDisplacementRenderer();

    QgsPointDisplacementRenderer* clone() const override;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const override;

    /** Reimplemented from QgsFeatureRenderer*/
    bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false ) override;

    /** Partial proxy that will call this method on the embedded renderer. */
    virtual QList<QString> usedAttributes() override;
    /** Proxy that will call this method on the embedded renderer. */
    virtual Capabilities capabilities() override;
    /** Proxy that will call this method on the embedded renderer.
      @note available in python as symbols2
     */
    virtual QgsSymbolList symbols( QgsRenderContext& context ) override;
    /** Proxy that will call this method on the embedded renderer.
      @note available in python as symbolForFeature2
     */
    virtual QgsSymbol* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;
    /** Proxy that will call this method on the embedded renderer.
      @note available in python as originalSymbolForFeature2
     */
    virtual QgsSymbol* originalSymbolForFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    /** Proxy that will call this method on the embedded renderer.
      @note available in python as symbolsForFeature2
     */
    virtual QgsSymbolList symbolsForFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    /** Proxy that will call this method on the embedded renderer.
      @note available in python as originalSymbolsForFeature2
     */
    virtual QgsSymbolList originalSymbolsForFeature( QgsFeature& feat, QgsRenderContext& context ) override;
    /** Proxy that will call this method on the embedded renderer.
      @note available in python as willRenderFeature2
     */
    virtual bool willRenderFeature( QgsFeature& feat, QgsRenderContext& context ) override;

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;

    void stopRender( QgsRenderContext& context ) override;

    //! create a renderer from XML element
    static QgsFeatureRenderer* create( QDomElement& symbologyElem );
    QDomElement save( QDomDocument& doc ) override;

    QgsLegendSymbologyList legendSymbologyItems( QSize iconSize ) override;

    //! @note not available in python bindings
    QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = "" ) override;

    void setLabelAttributeName( const QString& name ) { mLabelAttributeName = name; }
    QString labelAttributeName() const { return mLabelAttributeName; }

    void setEmbeddedRenderer( QgsFeatureRenderer* r ) override;
    const QgsFeatureRenderer* embeddedRenderer() const override;

    virtual void setLegendSymbolItem( const QString& key, QgsSymbol* symbol ) override;

    virtual bool legendSymbolItemsCheckable() const override;
    virtual bool legendSymbolItemChecked( const QString& key ) override;
    virtual void checkLegendSymbolItem( const QString& key, bool state = true ) override;

    //! not available in python bindings
    //! @deprecated since 2.4
    Q_DECL_DEPRECATED void setDisplacementGroups( const QList<QMap<QgsFeatureId, QgsFeature> >& list ) { Q_UNUSED( list ); }

    void setLabelFont( const QFont& f ) { mLabelFont = f; }
    QFont labelFont() const { return mLabelFont;}

    void setCircleWidth( double w ) { mCircleWidth = w; }
    double circleWidth() const { return mCircleWidth; }

    void setCircleColor( const QColor& c ) { mCircleColor = c; }
    QColor circleColor() const { return mCircleColor; }

    void setLabelColor( const QColor& c ) { mLabelColor = c;}
    QColor labelColor() const { return mLabelColor; }

    void setCircleRadiusAddition( double d ) { mCircleRadiusAddition = d; }
    double circleRadiusAddition() const { return mCircleRadiusAddition; }

    void setMaxLabelScaleDenominator( double d ) { mMaxLabelScaleDenominator = d; }
    double maxLabelScaleDenominator() const { return mMaxLabelScaleDenominator; }

    /** Returns the placement method used for dispersing the points.
     * @see setPlacement()
     * @note added in QGIS 2.12
     */
    Placement placement() const { return mPlacement; }

    /** Sets the placement method used for dispersing the points.
     * @param placement placement method
     * @see placement()
     * @note added in QGIS 2.12
     */
    void setPlacement( Placement placement ) { mPlacement = placement; }

    /** Returns the symbol for the center of a displacement group (but _not_ ownership of the symbol)*/
    QgsMarkerSymbol* centerSymbol() { return mCenterSymbol;}
    /** Sets the center symbol (takes ownership)*/
    void setCenterSymbol( QgsMarkerSymbol* symbol );

    /** Sets the tolerance distance for grouping points. Units are specified using
     * setToleranceUnit().
     * @param t tolerance distance
     * @see tolerance()
     * @see setToleranceUnit()
     */
    void setTolerance( double t ) { mTolerance = t; }

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
     * toleranceUnit() is set to QgsUnitTypes::MapUnit.
     * @param scale scale for distance tolerance
     * @see toleranceMapUnitScale()
     * @see setToleranceUnit()
     */
    void setToleranceMapUnitScale( const QgsMapUnitScale& scale ) { mToleranceMapUnitScale = scale; }

    /** Returns the map unit scale object for the distance tolerance. This is only used if the
     * toleranceUnit() is set to QgsUnitTypes::MapUnit.
     * @see setToleranceMapUnitScale()
     * @see toleranceUnit()
     */
    const QgsMapUnitScale& toleranceMapUnitScale() const { return mToleranceMapUnitScale; }

    //! creates a QgsPointDisplacementRenderer from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsPointDisplacementRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer );

  private:

    /** Embedded renderer. Like This, it is possible to use a classification together with point displacement*/
    QgsFeatureRenderer* mRenderer;

    /** Attribute name for labeling. Empty string means no labelling will be done*/
    QString mLabelAttributeName;
    /** Label attribute index (or -1 if none). This index is not stored, it is requested in the startRender() method*/
    int mLabelIndex;

    /** Center symbol for a displacement group*/
    QgsMarkerSymbol* mCenterSymbol;

    /** Tolerance. Points that are closer together are considered as equal*/
    double mTolerance;
    QgsUnitTypes::RenderUnit mToleranceUnit;
    QgsMapUnitScale mToleranceMapUnitScale;

    Placement mPlacement;

    /** Font that is passed to the renderer*/
    QFont mLabelFont;
    QColor mLabelColor;
    /** Line width for the circle*/
    double mCircleWidth;
    /** Color to draw the circle*/
    QColor mCircleColor;
    /** Addition to the default circle radius*/
    double mCircleRadiusAddition;
    /** Is set internally from startRender() depending on scale denominator*/
    bool mDrawLabels;
    /** Maximum scale denominator for label display. Negative number means no scale limitation*/
    double mMaxLabelScaleDenominator;

    typedef QMap<QgsFeatureId, QPair< QgsFeature, QgsSymbol* > > DisplacementGroup;
    /** Groups of features that have the same position*/
    QList<DisplacementGroup> mDisplacementGroups;
    /** Mapping from feature ID to its group index*/
    QMap<QgsFeatureId, int> mGroupIndex;
    /** Spatial index for fast lookup of close points*/
    QgsSpatialIndex* mSpatialIndex;
    /** Keeps track which features are selected */
    QSet<QgsFeatureId> mSelectedFeatures;

    /** Creates a search rectangle with specified distance tolerance */
    QgsRectangle searchRect( const QgsPoint& p, double distance ) const;
    /** This is a debugging function to check the entries in the displacement groups*/
    void printInfoDisplacementGroups();

    /** Returns the label for a feature (using mLabelAttributeName as attribute field)*/
    QString getLabel( const QgsFeature& f );

    //rendering methods
    void renderPoint( const QPointF& point, QgsSymbolRenderContext& context, const QList<QgsMarkerSymbol*>& symbols,
                      const QStringList& labels );

    //helper functions
    void calculateSymbolAndLabelPositions( QgsSymbolRenderContext &symbolContext, QPointF centerPoint, int nPosition, double symbolDiagonal, QList<QPointF>& symbolPositions, QList<QPointF>& labelShifts , double &circleRadius ) const;
    void drawGroup( const DisplacementGroup& group, QgsRenderContext& context );
    void drawCircle( double radiusPainterUnits, QgsSymbolRenderContext& context, QPointF centerPoint, int nSymbols );
    void drawSymbols( const QgsFeatureList& features, QgsRenderContext& context, const QList< QgsMarkerSymbol* >& symbolList, const QList<QPointF>& symbolPositions, bool selected = false );
    void drawLabels( QPointF centerPoint, QgsSymbolRenderContext& context, const QList<QPointF>& labelShifts, const QStringList& labelList );
    /** Returns first symbol for feature or 0 if none*/
    QgsSymbol* firstSymbolForFeature( QgsFeatureRenderer* r, QgsFeature& f, QgsRenderContext& context );
};

#endif // QGSPOINTDISPLACEMENTRENDERER_H
