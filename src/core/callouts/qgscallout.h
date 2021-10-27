/***************************************************************************
                             qgscallout.h
                             ----------------
    begin                : July 2019
    copyright            : (C) 2019 Nyall Dawson
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
#ifndef QGSCALLOUT_H
#define QGSCALLOUT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsexpressioncontext.h"
#include "qgsreadwritecontext.h"
#include "qgspropertycollection.h"
#include "qgsmapunitscale.h"
#include "qgscalloutposition.h"
#include "qgsmargins.h"
#include "qgscoordinatetransform.h"

#include <QPainter>
#include <QString>
#include <QRectF>
#include <memory>

class QgsLineSymbol;
class QgsFillSymbol;
class QgsGeometry;
class QgsRenderContext;

class QgsCalloutWidget; //stop sip breaking

/**
 * \ingroup core
 * \brief Abstract base class for callout renderers.
 *
 * Implementations of QgsCallout are responsible for performing the actual render of
 * callouts, including determining the desired shape of the callout and using any
 * relevant symbology elements to render them.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsCallout
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == "simple" && dynamic_cast<QgsSimpleLineCallout *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsSimpleLineCallout;
    }
    else if ( sipCpp->type() == "manhattan" && dynamic_cast<QgsManhattanLineCallout *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsManhattanLineCallout;
    }
    else if ( sipCpp->type() == "curved" && dynamic_cast<QgsCurvedLineCallout *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsCurvedLineCallout;
    }
    else if ( sipCpp->type() == "balloon" && dynamic_cast<QgsBalloonCallout *>( sipCpp ) != NULL )
    {
      sipType = sipType_QgsBalloonCallout;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    //! Data definable properties.
    enum Property
    {
      MinimumCalloutLength, //!< Minimum length of callouts
      OffsetFromAnchor, //!< Distance to offset lines from anchor points
      OffsetFromLabel, //!< Distance to offset lines from label area
      DrawCalloutToAllParts, //!< Whether callout lines should be drawn to all feature parts
      AnchorPointPosition, //!< Feature's anchor point position
      LabelAnchorPointPosition, //!< Label's anchor point position
      OriginX, //!< X-coordinate of callout origin (label anchor) (since QGIS 3.20)
      OriginY, //!< Y-coordinate of callout origin (label anchor) (since QGIS 3.20)
      DestinationX, //!< X-coordinate of callout destination (feature anchor) (since QGIS 3.20)
      DestinationY, //!< Y-coordinate of callout destination (feature anchor) (since QGIS 3.20)
      Curvature, //!< Curvature of curved line callouts (since QGIS 3.20)
      Orientation, //!< Orientation of curved line callouts (since QGIS 3.20)
      Margins, //!< Margin from text (since QGIS 3.20)
      WedgeWidth, //!< Balloon callout wedge width (since QGIS 3.20)
      CornerRadius, //!< Balloon callout corner radius (since QGIS 3.20)
      BlendMode, //!< Callout blend mode (since QGIS 3.20)
    };

    //! Options for draw order (stacking) of callouts
    enum DrawOrder
    {
      OrderBelowAllLabels, //!< Render callouts below all labels
      OrderBelowIndividualLabels, //!< Render callouts below their individual associated labels, some callouts may be drawn over other labels
    };

    //! Feature's anchor point position
    enum AnchorPoint
    {
      PoleOfInaccessibility = 0, //!< The surface's pole of inaccessibility used as anchor for polygon geometries
      PointOnExterior, //!< A point on the surface's outline closest to the label is used as anchor for polygon geometries
      PointOnSurface, //!< A point guaranteed to be on the surface is used as anchor for polygon geometries
      Centroid, //!< The surface's centroid is used as anchor for polygon geometries
    };

    /**
     * Label's anchor point position.
     * \since QGIS 3.14
     */
    enum LabelAnchorPoint
    {
      LabelPointOnExterior, //!< The point on the label's boundary closest to the feature
      LabelCentroid, //!< The labe's centroid
      LabelTopLeft, //!< Top left corner of the label's boundary
      LabelTopMiddle, //!< Top middle of the label's boundary
      LabelTopRight, //!< Top right corner of the label's boundary
      LabelMiddleLeft, //!< Middle left of the label's boundary
      LabelMiddleRight, //!< Middle right of the label's boundary
      LabelBottomLeft, //!< Bottom left corner of the label's boundary
      LabelBottomMiddle, //!< Bottom middle of the label's boundary
      LabelBottomRight, //!< Bottom right corner of the label's boundary
    };

    /**
     * Constructor for QgsCallout.
     */
    QgsCallout();
    virtual ~QgsCallout() = default;

    /**
     * Returns a unique string representing the callout type.
     */
    virtual QString type() const = 0;

    /**
     * Duplicates a callout by creating a deep copy of the callout.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsCallout *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the properties describing the callout encoded in a
     * string format.
     *
     * Subclasses must ensure that they include the base class' properties()
     * in their returned value.
     *
     * \see readProperties()
     * \see saveProperties()
     */
    virtual QVariantMap properties( const QgsReadWriteContext &context ) const;

    /**
     * Reads a string map of an callout's properties and restores the callout
     * to the state described by the properties map.
     *
     * Subclasses must ensure that they call the base class' readProperties()
     * method.
     *
     * \see properties()
     */
    virtual void readProperties( const QVariantMap &props, const QgsReadWriteContext &context );

    /**
     * Saves the current state of the callout to a DOM \a element. The default
     * behavior is to save the properties string map returned by
     * properties().
     * \returns TRUE if save was successful
     * \see readProperties()
     */
    virtual bool saveProperties( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const;

    /**
     * Restores the callout's properties from a DOM element.
     *
     * The default behavior is the read the DOM contents and call readProperties() on the subclass.
     *
     * \see readProperties()
     */
    virtual void restoreProperties( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns TRUE if the callout requires advanced effects such as blend modes, which require
     * output in raster formats to be fully respected.
     * \since QGIS 3.20
     */
    bool containsAdvancedEffects() const;

    /**
     * Prepares the callout for rendering on the specified render \a context.
     *
     * \warning This MUST be called prior to calling render() on the callout, and must always
     * be accompanied by a corresponding call to stopRender().
     *
     * \see stopRender()
     */
    virtual void startRender( QgsRenderContext &context );

    /**
     * Finalises the callout after a set of rendering operations on the specified render \a context.
     *
     * \warning This MUST be called after to after render() operations on the callout, and must always
     * be accompanied by a corresponding prior call to startRender().
     *
     * \see startRender()
     */
    virtual void stopRender( QgsRenderContext &context );

    /**
     * Returns the set of attributes referenced by the callout. This includes attributes
     * required by any data defined properties associated with the callout.
     *
     * \warning This must only be called after a corresponding call to startRender() with
     * the same render \a context.
     */
    virtual QSet< QString > referencedFields( const QgsRenderContext &context ) const;

    /**
     * Returns the desired drawing order (stacking) to use while rendering this callout.
     *
     * The default order is QgsCallout::OrderBelowIndividualLabels.
     */
    virtual DrawOrder drawOrder() const;

    /**
     * \brief Contains additional contextual information about the context in which a callout is
     * being rendered.
     * \ingroup core
     * \since QGIS 3.10
     */
    class CORE_EXPORT QgsCalloutContext
    {
      public:
        //! TRUE if all parts of associated feature were labeled
        bool allFeaturePartsLabeled = false;

        /**
         * Contains the CRS of the original feature associated with this callout.
         *
         * \since QGIS 3.20
         */
        QgsCoordinateReferenceSystem originalFeatureCrs;

        /**
         * Returns the coordinate transform to convert from the original layer associated with
         * the callout to the destination map CRS.
         *
         * \since QGIS 3.20
         */
        QgsCoordinateTransform originalFeatureToMapTransform( const QgsRenderContext &renderContext ) const;

        /**
         * Adds a rendered callout position.
         *
         * The position details such as the callout line origin and destination should be populated by the
         * callout subclass during rendering operations.
         *
         * \note the feature ID, layer ID and provider ID of the QgsCalloutPosition will be automatically populated.
         *
         * \since QGIS 3.20
         */
        void addCalloutPosition( const QgsCalloutPosition &position ) { return mPositions.push_back( position ); }

        /**
         * Returns the list of rendered callout positions.
         *
         * \since QGIS 3.20
         */
        QList< QgsCalloutPosition > positions() const { return mPositions; }

      private:
        //! Lazy initialized coordinate transform from original feature CRS to map CRS
        mutable QgsCoordinateTransform mOriginalFeatureToMapTransform;

        QList< QgsCalloutPosition > mPositions;
    };

    /**
     * Renders the callout onto the specified render \a context.
     *
     * The \a rect argument gives the desired size and position of the body of the callout (e.g. the
     * actual label geometry). The \a angle argument specifies the rotation of the callout body
     * (in degrees clockwise from horizontal). It is assumed that angle rotation specified via \a angle
     * is applied around the center of \a rect.
     *
     * The \a anchor argument dictates the geometry which the callout should connect to. Depending on the
     * callout subclass and anchor geometry type, the actual shape of the rendered callout may vary.
     * E.g. a subclass may prefer to attach to the centroid of the \a anchor, while another subclass may
     * prefer to attach to the closest point on \a anchor instead.
     *
     * Both \a rect and \a anchor must be specified in painter coordinates (i.e. pixels).
     *
     * The \a calloutContext argument is used to specify additional contextual information about
     * how a callout is being rendered.
     *
     * \warning A prior call to startRender() must have been made before calling this method, and
     * after all render() operations are complete a call to stopRender() must be made.
     */
    void render( QgsRenderContext &context, const QRectF &rect, const double angle, const QgsGeometry &anchor, QgsCalloutContext &calloutContext );

    /**
     * Returns TRUE if the the callout is enabled.
     * \see setEnabled()
     */
    bool enabled() const { return mEnabled; }

    /**
     * Sets whether the callout is \a enabled.
     * \see enabled()
     */
    void setEnabled( bool enabled );

    /**
     * Returns a reference to the callout's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the callout's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \see Property
     * \note not available in Python bindings
     */
    const QgsPropertyCollection &dataDefinedProperties() const SIP_SKIP { return mDataDefinedProperties; }

    /**
     * Sets the callout's property \a collection, used for data defined overrides.
     *
     * Any existing properties will be discarded.
     *
     * \see dataDefinedProperties()
     * \see Property
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection ) { mDataDefinedProperties = collection; }

    /**
     * Returns the definitions for data defined properties available for use in callouts.
     */
    static QgsPropertiesDefinition propertyDefinitions();

    /**
     * Returns the feature's anchor point position.
     *
     * \see setAnchorPoint()
     */
    AnchorPoint anchorPoint() const { return mAnchorPoint; }

    /**
     * Sets the feature's \a anchor point position.
     *
     * \see anchorPoint()
     */
    void setAnchorPoint( AnchorPoint anchor ) { mAnchorPoint = anchor; }

    /**
     * Encodes an \a anchor point to its string representation.
     * \returns encoded string
     * \see decodeAnchorPoint()
     */
    static QString encodeAnchorPoint( AnchorPoint anchor );

    /**
     * Attempts to decode a string representation of an anchor point name to the corresponding
     * anchor point.
     * \param name encoded anchor point name
     * \param ok if specified, will be set to TRUE if the anchor point was successfully decoded
     * \returns decoded name
     * \see encodeAnchorPoint()
     */
    static QgsCallout::AnchorPoint decodeAnchorPoint( const QString &name, bool *ok = nullptr );


    /**
     * Returns the label's anchor point position.
     *
     * \see setLabelAnchorPoint()
     * \since QGIS 3.14
     */
    LabelAnchorPoint labelAnchorPoint() const { return mLabelAnchorPoint; }

    /**
     * Sets the label's \a anchor point position.
     *
     * \see labelAnchorPoint()
     * \since QGIS 3.14
     */
    void setLabelAnchorPoint( LabelAnchorPoint anchor ) { mLabelAnchorPoint = anchor; }

    /**
     * Encodes a label \a anchor point to its string representation.
     * \returns encoded string
     * \see decodeLabelAnchorPoint()
     * \since QGIS 3.14
     */
    static QString encodeLabelAnchorPoint( LabelAnchorPoint anchor );

    /**
     * Attempts to decode a string representation of a label anchor point name to the corresponding
     * anchor point.
     * \param name encoded label anchor point name
     * \param ok if specified, will be set to TRUE if the anchor point was successfully decoded
     * \returns decoded name
     * \see encodeLabelAnchorPoint()
     * \since QGIS 3.14
     */
    static QgsCallout::LabelAnchorPoint decodeLabelAnchorPoint( const QString &name, bool *ok = nullptr );

    /**
     * Returns the blending mode used for drawing callouts.
     * \see setBlendMode()
     * \since QGIS 3.20
     */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    /**
     * Sets the blending \a mode used for drawing callouts.
     * \see blendMode()
     * \since QGIS 3.20
     */
    void setBlendMode( QPainter::CompositionMode mode ) { mBlendMode = mode; }

  protected:

    /**
     * Performs the actual rendering of the callout implementation onto the specified render \a context.
     *
     * The \a bodyBoundingBox argument gives the desired size and position of the body of the callout (e.g. the
     * actual label geometry). The \a angle argument specifies the rotation of the callout body
     * (in degrees clockwise from horizontal). It is assumed that angle rotation specified via \a angle
     * is applied around the center of \a rect.
     *
     * The \a anchor argument dictates the geometry which the callout should connect to. Depending on the
     * callout subclass and anchor geometry type, the actual shape of the rendered callout may vary.
     * E.g. a subclass may prefer to attach to the centroid of the \a anchor, while another subclass may
     * prefer to attach to the closest point on \a anchor instead.
     *
     * Both \a rect and \a anchor are specified in painter coordinates (i.e. pixels).
     *
     * The \a calloutContext argument is used to specify additional contextual information about
     * how a callout is being rendered.
     */
    virtual void draw( QgsRenderContext &context, const QRectF &bodyBoundingBox, const double angle, const QgsGeometry &anchor, QgsCalloutContext &calloutContext ) = 0;

    /**
     * Returns the anchor point geometry for a label with the given bounding box and \a anchor point mode.
     * \deprecated QGIS 3.20 use calloutLabelPoint() instead
     */
    Q_DECL_DEPRECATED QgsGeometry labelAnchorGeometry( const QRectF &bodyBoundingBox, const double angle, LabelAnchorPoint anchor ) const SIP_DEPRECATED;

    /**
     * Returns the anchor point geometry for a label with the given bounding box and \a anchor point mode.
     *
     * The \a pinned argument will be set to TRUE if the callout label point is pinned (manually placed).
     *
     * \since QGIS 3.20
     */
    QgsGeometry calloutLabelPoint( const QRectF &bodyBoundingBox, double angle, LabelAnchorPoint anchor, QgsRenderContext &context, const QgsCalloutContext &calloutContext, bool &pinned ) const;

    /**
     * Calculates the direct line from a label geometry to an anchor geometry part, respecting the various
     * callout settings which influence how the callout end should be placed in the anchor geometry.
     *
     * Returns a null geometry if the callout line cannot be calculated.
     *
     * The \a pinned argument will be set to TRUE if the callout anchor point is pinned (manually placed).
     *
     * \since QGIS 3.20
     */
    QgsGeometry calloutLineToPart( const QgsGeometry &labelGeometry, const QgsAbstractGeometry *partGeometry, QgsRenderContext &context, const QgsCalloutContext &calloutContext, bool &pinned ) const;

  private:

    bool mEnabled = false;

    AnchorPoint mAnchorPoint = PoleOfInaccessibility;
    LabelAnchorPoint mLabelAnchorPoint = LabelPointOnExterior;

    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;

    //! Property collection for data defined callout settings
    QgsPropertyCollection mDataDefinedProperties;

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

    static void initPropertyDefinitions();
};

/**
 * \ingroup core
 * \brief A simple direct line callout style.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsSimpleLineCallout : public QgsCallout
{
  public:

    QgsSimpleLineCallout();
    ~QgsSimpleLineCallout() override;

#ifndef SIP_RUN

    /**
     * Copy constructor.
     */
    QgsSimpleLineCallout( const QgsSimpleLineCallout &other );
    QgsSimpleLineCallout &operator=( const QgsSimpleLineCallout & ) = delete;
#endif

    /**
     * Creates a new QgsSimpleLineCallout, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsSimpleLineCallout::properties() ).
     */
    static QgsCallout *create( const QVariantMap &properties = QVariantMap(), const QgsReadWriteContext &context = QgsReadWriteContext() ) SIP_FACTORY;

    QString type() const override;
    QgsSimpleLineCallout *clone() const override;
    QVariantMap properties( const QgsReadWriteContext &context ) const override;
    void readProperties( const QVariantMap &props, const QgsReadWriteContext &context ) override;
    void startRender( QgsRenderContext &context ) override;
    void stopRender( QgsRenderContext &context ) override;
    QSet< QString > referencedFields( const QgsRenderContext &context ) const override;

    /**
     * Returns the line symbol used to render the callout line.
     *
     * Ownership is not transferred.
     *
     * \see setLineSymbol()
     */
    QgsLineSymbol *lineSymbol();

    /**
     * Sets the line \a symbol used to render the callout line. Ownership of \a symbol is
     * transferred to the callout.
     *
     * \see lineSymbol()
     */
    void setLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the minimum length of callout lines. Units are specified through minimumLengthUnits().
     * \see setMinimumLength()
     * \see minimumLengthUnit()
     */
    double minimumLength() const { return mMinCalloutLength; }

    /**
     * Sets the minimum \a length of callout lines. Units are specified through setMinimumLengthUnit().
     * \see minimumLength()
     * \see setMinimumLengthUnit()
     */
    void setMinimumLength( double length ) { mMinCalloutLength = length; }

    /**
     * Sets the \a unit for the minimum length of callout lines.
     * \see minimumLengthUnit()
     * \see setMinimumLength()
    */
    void setMinimumLengthUnit( QgsUnitTypes::RenderUnit unit ) { mMinCalloutLengthUnit = unit; }

    /**
     * Returns the units for the minimum length of callout lines.
     * \see setMinimumLengthUnit()
     * \see minimumLength()
    */
    QgsUnitTypes::RenderUnit minimumLengthUnit() const { return mMinCalloutLengthUnit; }

    /**
     * Sets the map unit \a scale for the minimum callout length.
     * \see minimumLengthMapUnitScale()
     * \see setMinimumLengthUnit()
     * \see setMinimumLength()
     */
    void setMinimumLengthMapUnitScale( const QgsMapUnitScale &scale ) { mMinCalloutLengthScale = scale; }

    /**
     * Returns the map unit scale for the minimum callout length.
     * \see setMinimumLengthMapUnitScale()
     * \see minimumLengthUnit()
     * \see minimumLength()
     */
    const QgsMapUnitScale &minimumLengthMapUnitScale() const { return mMinCalloutLengthScale; }


    /**
     * Returns the offset distance from the anchor point at which to start the line. Units are specified through offsetFromAnchorUnit().
     * \see setOffsetFromAnchor()
     * \see offsetFromAnchorUnit()
     */
    double offsetFromAnchor() const { return mOffsetFromAnchorDistance; }

    /**
     * Sets the offset \a distance from the anchor point at which to start the line. Units are specified through setOffsetFromAnchorUnit().
     * \see offsetFromAnchor()
     * \see setOffsetFromAnchorUnit()
     */
    void setOffsetFromAnchor( double distance ) { mOffsetFromAnchorDistance = distance; }

    /**
     * Sets the \a unit for the offset from anchor distance.
     * \see offsetFromAnchor()
     * \see setOffsetFromAnchor()
    */
    void setOffsetFromAnchorUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetFromAnchorUnit = unit; }

    /**
     * Returns the units for the offset from anchor point.
     * \see setOffsetFromAnchorUnit()
     * \see offsetFromAnchor()
    */
    QgsUnitTypes::RenderUnit offsetFromAnchorUnit() const { return mOffsetFromAnchorUnit; }

    /**
     * Sets the map unit \a scale for the offset from anchor.
     * \see offsetFromAnchorMapUnitScale()
     * \see setOffsetFromAnchorUnit()
     * \see setOffsetFromAnchor()
     */
    void setOffsetFromAnchorMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetFromAnchorScale = scale; }

    /**
     * Returns the map unit scale for the offset from anchor.
     * \see setOffsetFromAnchorMapUnitScale()
     * \see offsetFromAnchorUnit()
     * \see offsetFromAnchor()
     */
    const QgsMapUnitScale &offsetFromAnchorMapUnitScale() const { return mOffsetFromAnchorScale; }

    /**
     * Returns the offset distance from label area at which to end the line. Units are specified through offsetFromLabelUnit().
     * \see setOffsetFromLabel()
     * \see offsetFromLabelUnit()
     */
    double offsetFromLabel() const { return mOffsetFromLabelDistance; }

    /**
     * Sets the offset \a distance from label area at which to end the line. Units are specified through setOffsetFromLabelUnit().
     * \see offsetFromLabel()
     * \see setOffsetFromLabelUnit()
     */
    void setOffsetFromLabel( double distance ) { mOffsetFromLabelDistance = distance; }

    /**
     * Sets the \a unit for the offset from label area distance.
     * \see offsetFromLabel()
     * \see setOffsetFromLabel()
    */
    void setOffsetFromLabelUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetFromLabelUnit = unit; }

    /**
     * Returns the units for the offset from label area.
     * \see setOffsetFromLabelUnit()
     * \see offsetFromLabel()
    */
    QgsUnitTypes::RenderUnit offsetFromLabelUnit() const { return mOffsetFromLabelUnit; }

    /**
     * Sets the map unit \a scale for the offset from label area.
     * \see offsetFromLabelMapUnitScale()
     * \see setOffsetFromLabelUnit()
     * \see setOffsetFromLabel()
     */
    void setOffsetFromLabelMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetFromLabelScale = scale; }

    /**
     * Returns the map unit scale for the minimum callout length.
     * \see setOffsetFromLabelMapUnitScale()
     * \see offsetFromLabelUnit()
     * \see offsetFromLabel()
     */
    const QgsMapUnitScale &offsetFromLabelMapUnitScale() const { return mOffsetFromLabelScale; }

    /**
     * Returns TRUE if callout lines should be drawn to all feature parts.
     *
     * \see setDrawCalloutToAllParts()
     */
    bool drawCalloutToAllParts() const { return mDrawCalloutToAllParts; }

    /**
     * Sets whether callout lines should be drawn to all feature parts.
     *
     * \see drawCalloutToAllParts()
     */
    void setDrawCalloutToAllParts( bool drawToAllParts ) { mDrawCalloutToAllParts = drawToAllParts; }

  protected:
    void draw( QgsRenderContext &context, const QRectF &bodyBoundingBox, const double angle, const QgsGeometry &anchor, QgsCallout::QgsCalloutContext &calloutContext ) override;

    /**
     * Creates a callout line between \a start and \a end in the desired style.
     *
     * The base class method returns a straight line.
     *
     * \since QGIS 3.20
     */
    virtual QgsCurve *createCalloutLine( const QgsPoint &start, const QgsPoint &end, QgsRenderContext &context, const QRectF &bodyBoundingBox, const double angle, const QgsGeometry &anchor, QgsCallout::QgsCalloutContext &calloutContext ) const SIP_FACTORY;

  private:

#ifdef SIP_RUN
    QgsSimpleLineCallout( const QgsSimpleLineCallout &other );
    QgsSimpleLineCallout &operator=( const QgsSimpleLineCallout & );
#endif

    std::unique_ptr< QgsLineSymbol > mLineSymbol;
    double mMinCalloutLength = 0;
    QgsUnitTypes::RenderUnit mMinCalloutLengthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mMinCalloutLengthScale;

    double mOffsetFromAnchorDistance = 0;
    QgsUnitTypes::RenderUnit mOffsetFromAnchorUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetFromAnchorScale;

    double mOffsetFromLabelDistance = 0;
    QgsUnitTypes::RenderUnit mOffsetFromLabelUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetFromLabelScale;

    bool mDrawCalloutToAllParts = false;
};


/**
 * \ingroup core
 * \brief Draws straight (right angled) lines as callouts.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsManhattanLineCallout : public QgsSimpleLineCallout
{
  public:

    QgsManhattanLineCallout();

#ifndef SIP_RUN

    /**
     * Copy constructor.
     */
    QgsManhattanLineCallout( const QgsManhattanLineCallout &other );

    QgsManhattanLineCallout &operator=( const QgsManhattanLineCallout & ) = delete;
#endif

    /**
     * Creates a new QgsManhattanLineCallout, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsManhattanLineCallout::properties() ).
     */
    static QgsCallout *create( const QVariantMap &properties = QVariantMap(), const QgsReadWriteContext &context = QgsReadWriteContext() ) SIP_FACTORY;

    QString type() const override;
    QgsManhattanLineCallout *clone() const override;

  protected:
    QgsCurve *createCalloutLine( const QgsPoint &start, const QgsPoint &end, QgsRenderContext &context, const QRectF &bodyBoundingBox, const double angle, const QgsGeometry &anchor, QgsCallout::QgsCalloutContext &calloutContext ) const override SIP_FACTORY;

  private:
#ifdef SIP_RUN
    QgsManhattanLineCallout( const QgsManhattanLineCallout &other );
    QgsManhattanLineCallout &operator=( const QgsManhattanLineCallout & );
#endif
};


/**
 * \ingroup core
 * \brief Draws curved lines as callouts.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsCurvedLineCallout : public QgsSimpleLineCallout
{
  public:

    /**
     * Curve orientation
     */
    enum Orientation
    {
      Automatic, //!< Automatically choose most cartographically pleasing orientation based on label and callout arrangement
      Clockwise, //!< Curve lines in a clockwise direction
      CounterClockwise, //!< Curve lines in a counter-clockwise direction
    };

    QgsCurvedLineCallout();

#ifndef SIP_RUN

    /**
     * Copy constructor.
     */
    QgsCurvedLineCallout( const QgsCurvedLineCallout &other );

    QgsCurvedLineCallout &operator=( const QgsCurvedLineCallout & ) = delete;
#endif

    /**
     * Creates a new QgsCurvedLineCallout, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsCurvedLineCallout::properties() ).
     */
    static QgsCallout *create( const QVariantMap &properties = QVariantMap(), const QgsReadWriteContext &context = QgsReadWriteContext() ) SIP_FACTORY;

    QString type() const override;
    QgsCurvedLineCallout *clone() const override;
    QVariantMap properties( const QgsReadWriteContext &context ) const override;

    /**
     * Returns the callout line's curvature.
     *
     * The curvature is a percentage value (with typical ranges between 0.0 and 1.0), representing the overall curvature of the line.
     *
     * \see setCurvature()
     */
    double curvature() const;

    /**
     * Sets the callout line's \a curvature.
     *
     * The \a curvature is a percentage value (with typical ranges between 0.0 and 1.0), representing the overall curvature of the line.
     *
     * \see curvature()
     */
    void setCurvature( double curvature );

    /**
     * Returns the callout line's curve orientation.
     *
     * \see setOrientation()
     */
    Orientation orientation() const;

    /**
     * Sets the callout line's curve \a orientation.
     *
     * \see orientation()
     */
    void setOrientation( Orientation orientation );

  protected:
    QgsCurve *createCalloutLine( const QgsPoint &start, const QgsPoint &end, QgsRenderContext &context, const QRectF &bodyBoundingBox, const double angle, const QgsGeometry &anchor, QgsCalloutContext &calloutContext ) const override SIP_FACTORY;

  private:
#ifdef SIP_RUN
    QgsCurvedLineCallout( const QgsCurvedLineCallout &other );
    QgsCurvedLineCallout &operator=( const QgsCurvedLineCallout & );
#endif

    /**
     * Decodes a string to an orientation value
     */
    static Orientation decodeOrientation( const QString &string );

    /**
     * Encodes an orientation string
     */
    static QString encodeOrientation( Orientation orientation );


    Orientation mOrientation = Automatic;
    double mCurvature = 0.1;
};


/**
 * \ingroup core
 * \brief A cartoon talking bubble callout style.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsBalloonCallout : public QgsCallout
{
  public:

    QgsBalloonCallout();
    ~QgsBalloonCallout() override;

#ifndef SIP_RUN

    /**
     * Copy constructor.
     */
    QgsBalloonCallout( const QgsBalloonCallout &other );
    QgsBalloonCallout &operator=( const QgsBalloonCallout & ) = delete;
#endif

    /**
     * Creates a new QgsBalloonCallout, using the settings
     * serialized in the \a properties map (corresponding to the output from
     * QgsBalloonCallout::properties() ).
     */
    static QgsCallout *create( const QVariantMap &properties = QVariantMap(), const QgsReadWriteContext &context = QgsReadWriteContext() ) SIP_FACTORY;

    QString type() const override;
    QgsBalloonCallout *clone() const override;
    QVariantMap properties( const QgsReadWriteContext &context ) const override;
    void readProperties( const QVariantMap &props, const QgsReadWriteContext &context ) override;
    void startRender( QgsRenderContext &context ) override;
    void stopRender( QgsRenderContext &context ) override;
    QSet< QString > referencedFields( const QgsRenderContext &context ) const override;

    /**
     * Returns the fill symbol used to render the callout.
     *
     * Ownership is not transferred.
     *
     * \see setFillSymbol()
     */
    QgsFillSymbol *fillSymbol();

    /**
     * Sets the fill \a symbol used to render the callout. Ownership of \a symbol is
     * transferred to the callout.
     *
     * \see fillSymbol()
     */
    void setFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the offset distance from the anchor point at which to start the line. Units are specified through offsetFromAnchorUnit().
     * \see setOffsetFromAnchor()
     * \see offsetFromAnchorUnit()
     */
    double offsetFromAnchor() const { return mOffsetFromAnchorDistance; }

    /**
     * Sets the offset \a distance from the anchor point at which to start the line. Units are specified through setOffsetFromAnchorUnit().
     * \see offsetFromAnchor()
     * \see setOffsetFromAnchorUnit()
     */
    void setOffsetFromAnchor( double distance ) { mOffsetFromAnchorDistance = distance; }

    /**
     * Sets the \a unit for the offset from anchor distance.
     * \see offsetFromAnchor()
     * \see setOffsetFromAnchor()
    */
    void setOffsetFromAnchorUnit( QgsUnitTypes::RenderUnit unit ) { mOffsetFromAnchorUnit = unit; }

    /**
     * Returns the units for the offset from anchor point.
     * \see setOffsetFromAnchorUnit()
     * \see offsetFromAnchor()
    */
    QgsUnitTypes::RenderUnit offsetFromAnchorUnit() const { return mOffsetFromAnchorUnit; }

    /**
     * Sets the map unit \a scale for the offset from anchor.
     * \see offsetFromAnchorMapUnitScale()
     * \see setOffsetFromAnchorUnit()
     * \see setOffsetFromAnchor()
     */
    void setOffsetFromAnchorMapUnitScale( const QgsMapUnitScale &scale ) { mOffsetFromAnchorScale = scale; }

    /**
     * Returns the map unit scale for the offset from anchor.
     * \see setOffsetFromAnchorMapUnitScale()
     * \see offsetFromAnchorUnit()
     * \see offsetFromAnchor()
     */
    const QgsMapUnitScale &offsetFromAnchorMapUnitScale() const { return mOffsetFromAnchorScale; }

    /**
     * Returns the margins between the outside of the callout frame and the label's bounding rectangle.
     *
     * Units are retrieved via marginsUnit()
     *
     * \note Negative margins are acceptable.
     *
     * \see setMargins()
     * \see marginsUnit()
     */
    const QgsMargins &margins() const { return mMargins; }

    /**
     * Sets the \a margins between the outside of the callout frame and the label's bounding rectangle.
     *
     * Units are set via setMarginsUnit()
     *
     * \note Negative margins are acceptable.
     *
     * \see margins()
     * \see setMarginsUnit()
     */
    void setMargins( const QgsMargins &margins ) { mMargins = margins; }

    /**
     * Sets the \a unit for the margins between the outside of the callout frame and the label's bounding rectangle.
     *
     * \see margins()
     * \see marginsUnit()
    */
    void setMarginsUnit( QgsUnitTypes::RenderUnit unit ) { mMarginUnit = unit; }

    /**
     * Returns the units for the margins between the outside of the callout frame and the label's bounding rectangle.
     *
     * \see setMarginsUnit()
     * \see margins()
    */
    QgsUnitTypes::RenderUnit marginsUnit() const { return mMarginUnit; }

    /**
     * Returns the width of the wedge shape at the side it connects with the label.
     *
     * Units are specified through wedgeWidthUnit().
     *
     * \see setWedgeWidth()
     * \see wedgeWidthUnit()
     */
    double wedgeWidth() const { return mWedgeWidth; }

    /**
     * Sets the \a width of the wedge shape at the side it connects with the label.
     *
     * Units are specified through setWedgeWidthUnit().
     *
     * \see wedgeWidth()
     * \see setWedgeWidthUnit()
     */
    void setWedgeWidth( double width ) { mWedgeWidth = width; }

    /**
     * Sets the \a unit for the wedge width.
     *
     * \see wedgeWidthUnit()
     * \see setWedgeWidth()
    */
    void setWedgeWidthUnit( QgsUnitTypes::RenderUnit unit ) { mWedgeWidthUnit = unit; }

    /**
     * Returns the units for the wedge width.
     *
     * \see setWedgeWidthUnit()
     * \see wedgeWidth()
    */
    QgsUnitTypes::RenderUnit wedgeWidthUnit() const { return mWedgeWidthUnit; }

    /**
     * Sets the map unit \a scale for the wedge width.
     *
     * \see wedgeWidthMapUnitScale()
     * \see setWedgeWidthUnit()
     * \see setWedgeWidth()
     */
    void setWedgeWidthMapUnitScale( const QgsMapUnitScale &scale ) { mWedgeWidthScale = scale; }

    /**
     * Returns the map unit scale for the wedge width.
     *
     * \see setWedgeWidthMapUnitScale()
     * \see wedgeWidthUnit()
     * \see wedgeWidth()
     */
    const QgsMapUnitScale &wedgeWidthMapUnitScale() const { return mWedgeWidthScale; }

    /**
     * Returns the corner radius of the balloon shapes.
     *
     * Units are specified through wedgeWidthUnit().
     *
     * \see setCornerRadius()
     * \see cornerRadiusUnit()
     */
    double cornerRadius() const { return mCornerRadius; }

    /**
     * Sets the \a radius of the corners for the balloon shapes.
     *
     * Units are specified through setCornerRadiusUnit().
     *
     * \see cornerRadius()
     * \see setCornerRadiusUnit()
     */
    void setCornerRadius( double radius ) { mCornerRadius = radius; }

    /**
     * Sets the \a unit for the corner radius.
     *
     * \see cornerRadiusUnit()
     * \see setCornerRadius()
    */
    void setCornerRadiusUnit( QgsUnitTypes::RenderUnit unit ) { mCornerRadiusUnit = unit; }

    /**
     * Returns the units for the corner radius.
     *
     * \see setCornerRadiusUnit()
     * \see cornerRadius()
    */
    QgsUnitTypes::RenderUnit cornerRadiusUnit() const { return mCornerRadiusUnit; }

    /**
     * Sets the map unit \a scale for the corner radius.
     *
     * \see cornerRadiusMapUnitScale()
     * \see setCornerRadiusUnit()
     * \see setCornerRadius()
     */
    void setCornerRadiusMapUnitScale( const QgsMapUnitScale &scale ) { mCornerRadiusScale = scale; }

    /**
     * Returns the map unit scale for the corner radius.
     *
     * \see setCornerRadiusMapUnitScale()
     * \see cornerRadiusUnit()
     * \see cornerRadius()
     */
    const QgsMapUnitScale &cornerRadiusMapUnitScale() const { return mCornerRadiusScale; }


  protected:
    void draw( QgsRenderContext &context, const QRectF &bodyBoundingBox, const double angle, const QgsGeometry &anchor, QgsCallout::QgsCalloutContext &calloutContext ) override;

  private:

    QPolygonF getPoints( QgsRenderContext &context, QgsPointXY origin, QRectF rect ) const;

#ifdef SIP_RUN
    QgsBalloonCallout( const QgsBalloonCallout &other );
    QgsBalloonCallout &operator=( const QgsBalloonCallout & );
#endif

    std::unique_ptr< QgsFillSymbol > mFillSymbol;

    double mOffsetFromAnchorDistance = 0;
    QgsUnitTypes::RenderUnit mOffsetFromAnchorUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mOffsetFromAnchorScale;

    QgsMargins mMargins;
    QgsUnitTypes::RenderUnit mMarginUnit = QgsUnitTypes::RenderMillimeters;

    double mWedgeWidth = 2.64;
    QgsUnitTypes::RenderUnit mWedgeWidthUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mWedgeWidthScale;

    double mCornerRadius = 0.0;
    QgsUnitTypes::RenderUnit mCornerRadiusUnit = QgsUnitTypes::RenderMillimeters;
    QgsMapUnitScale mCornerRadiusScale;

};



#endif // QGSCALLOUT_H

