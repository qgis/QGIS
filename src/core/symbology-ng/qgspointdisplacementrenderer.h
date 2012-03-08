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

class QgsVectorLayer;

/**A renderer that automatically displaces points with the same position*/
class CORE_EXPORT QgsPointDisplacementRenderer: public QgsFeatureRendererV2
{
  public:
    QgsPointDisplacementRenderer( const QString& labelAttributeName = "" );
    ~QgsPointDisplacementRenderer();

    QgsFeatureRendererV2* clone();

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    /**Reimplemented from QgsFeatureRendererV2*/
    bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    void startRender( QgsRenderContext& context, const QgsVectorLayer *vlayer );

    void stopRender( QgsRenderContext& context );

    QList<QString> usedAttributes();
    QgsSymbolV2List symbols();

    //! create a renderer from XML element
    static QgsFeatureRendererV2* create( QDomElement& symbologyElem );
    QDomElement save( QDomDocument& doc );

    QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    QgsLegendSymbolList legendSymbolItems();

    void setLabelAttributeName( const QString& name ) { mLabelAttributeName = name; }
    QString labelAttributeName() const { return mLabelAttributeName; }

    /**Sets embedded renderer (takes ownership)*/
    void setEmbeddedRenderer( QgsFeatureRendererV2* r );
    QgsFeatureRendererV2* embeddedRenderer() { return mRenderer;}

    void setDisplacementGroups( const QList<QMap<QgsFeatureId, QgsFeature> >& list );

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

    /**Groups of features that have the same position*/
    QList<QMap<QgsFeatureId, QgsFeature> > mDisplacementGroups;
    /**Set that contains all the ids the display groups (for quicker lookup)*/
    QSet<QgsFeatureId> mDisplacementIds;

    /**Create the displacement groups efficiently using a spatial index*/
    void createDisplacementGroups( QgsVectorLayer *vlayer, const QgsRectangle& viewExtent );
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
    void drawCircle( double radiusPainterUnits, QgsSymbolV2RenderContext& context, const QPointF& centerPoint, int nSymbols );
    void drawSymbols( QgsFeature& f, QgsRenderContext& context, const QList<QgsMarkerSymbolV2*>& symbolList, const QList<QPointF>& symbolPositions, bool selected = false );
    void drawLabels( const QPointF& centerPoint, QgsSymbolV2RenderContext& context, const QList<QPointF>& labelShifts, const QStringList& labelList );
    /**Returns first symbol for feature or 0 if none*/
    QgsSymbolV2* firstSymbolForFeature( QgsFeatureRendererV2* r, QgsFeature& f );
};

#endif // QGSPOINTDISPLACEMENTRENDERER_H
