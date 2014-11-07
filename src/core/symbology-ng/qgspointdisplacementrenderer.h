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
#include "qgssymbolv2.h"
#include "qgspoint.h"
#include "qgsrendererv2.h"
#include <QFont>
#include <QSet>

class QgsSpatialIndex;

/**A renderer that automatically displaces points with the same position*/
class CORE_EXPORT QgsPointDisplacementRenderer: public QgsFeatureRendererV2
{
  public:
    QgsPointDisplacementRenderer( const QString& labelAttributeName = "" );
    ~QgsPointDisplacementRenderer();

    QgsFeatureRendererV2* clone() const;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    /**Reimplemented from QgsFeatureRendererV2*/
    bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    /** Partial proxy that will call this method on the embedded renderer. */
    virtual QList<QString> usedAttributes();
    /** Proxy that will call this method on the embedded renderer. */
    virtual int capabilities();
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2List symbols();
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2* originalSymbolForFeature( QgsFeature& feat );
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2List symbolsForFeature( QgsFeature& feat );
    /** Proxy that will call this method on the embedded renderer. */
    virtual QgsSymbolV2List originalSymbolsForFeature( QgsFeature& feat );
    /** Proxy that will call this method on the embedded renderer. */
    virtual bool willRenderFeature( QgsFeature& feat );

    void startRender( QgsRenderContext& context, const QgsFields& fields );

    void stopRender( QgsRenderContext& context );

    //! create a renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& symbologyElem );
    QDomElement save( QDomDocument& doc );

    QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! @note not available in python bindings
    QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, QString rule = "" );

    void setLabelAttributeName( const QString& name ) { mLabelAttributeName = name; }
    QString labelAttributeName() const { return mLabelAttributeName; }

    /**Sets embedded renderer (takes ownership)*/
    void setEmbeddedRenderer( QgsFeatureRendererV2* r );
    QgsFeatureRendererV2* embeddedRenderer() const { return mRenderer;}

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

    /**Returns the symbol for the center of a displacement group (but _not_ ownership of the symbol)*/
    QgsMarkerSymbolV2* centerSymbol() { return mCenterSymbol;}
    /**Sets the center symbol (takes ownership)*/
    void setCenterSymbol( QgsMarkerSymbolV2* symbol );

    void setTolerance( double t ) { mTolerance = t; }
    double tolerance() const { return mTolerance; }

    //! creates a QgsPointDisplacementRenderer from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsPointDisplacementRenderer* convertFromRenderer( const QgsFeatureRendererV2 *renderer );

  private:

    /**Embedded renderer. Like This, it is possible to use a classification together with point displacement*/
    QgsFeatureRendererV2* mRenderer;

    /**Attribute name for labeling. Empty string means no labelling will be done*/
    QString mLabelAttributeName;
    /**Label attribute index (or -1 if none). This index is not stored, it is requested in the startRender() method*/
    int mLabelIndex;

    /**Center symbol for a displacement group*/
    QgsMarkerSymbolV2* mCenterSymbol;

    /**Tolerance. Points that are closer together are considered as equal*/
    double mTolerance;

    /**Font that is passed to the renderer*/
    QFont mLabelFont;
    QColor mLabelColor;
    /**Line width for the circle*/
    double mCircleWidth;
    /**Color to draw the circle*/
    QColor mCircleColor;
    /**Addition to the default circle radius*/
    double mCircleRadiusAddition;
    /**Is set internally from startRender() depending on scale denominator*/
    bool mDrawLabels;
    /**Maximum scale denominator for label display. Negative number means no scale limitation*/
    double mMaxLabelScaleDenominator;

    typedef QMap<QgsFeatureId, QgsFeature> DisplacementGroup;
    /**Groups of features that have the same position*/
    QList<DisplacementGroup> mDisplacementGroups;
    /**Mapping from feature ID to its group index*/
    QMap<QgsFeatureId, int> mGroupIndex;
    /**Spatial index for fast lookup of close points*/
    QgsSpatialIndex* mSpatialIndex;
    /** keeps trask which features are selected */
    QSet<QgsFeatureId> mSelectedFeatures;

    /**Creates a search rectangle with mTolerance*/
    QgsRectangle searchRect( const QgsPoint& p ) const;
    /**This is a debugging function to check the entries in the displacement groups*/
    void printInfoDisplacementGroups();

    /**Returns the label for a feature (using mLabelAttributeName as attribute field)*/
    QString getLabel( const QgsFeature& f );

    //rendering methods
    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context, const QList<QgsMarkerSymbolV2*>& symbols,
                      const QStringList& labels );

    //helper functions
    void calculateSymbolAndLabelPositions( const QPointF& centerPoint, int nPosition, double radius, double symbolDiagonal, QList<QPointF>& symbolPositions, QList<QPointF>& labelShifts ) const;
    void drawGroup( const DisplacementGroup& group, QgsRenderContext& context );
    void drawCircle( double radiusPainterUnits, QgsSymbolV2RenderContext& context, const QPointF& centerPoint, int nSymbols );
    void drawSymbols( const QgsFeature& f, QgsRenderContext& context, const QList<QgsMarkerSymbolV2*>& symbolList, const QList<QPointF>& symbolPositions, bool selected = false );
    void drawLabels( const QPointF& centerPoint, QgsSymbolV2RenderContext& context, const QList<QPointF>& labelShifts, const QStringList& labelList );
    /**Returns first symbol for feature or 0 if none*/
    QgsSymbolV2* firstSymbolForFeature( QgsFeatureRendererV2* r, QgsFeature& f );
};

#endif // QGSPOINTDISPLACEMENTRENDERER_H
