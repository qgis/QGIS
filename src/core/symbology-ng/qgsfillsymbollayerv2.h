/***************************************************************************
    qgsfillsymbollayerv2.h
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

#ifndef QGSFILLSYMBOLLAYERV2_H
#define QGSFILLSYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

#define DEFAULT_SIMPLEFILL_COLOR        QColor(0,0,255)
#define DEFAULT_SIMPLEFILL_STYLE        Qt::SolidPattern
#define DEFAULT_SIMPLEFILL_BORDERCOLOR  QColor(0,0,0)
#define DEFAULT_SIMPLEFILL_BORDERSTYLE  Qt::SolidLine
#define DEFAULT_SIMPLEFILL_BORDERWIDTH  DEFAULT_LINE_WIDTH

#include <QPen>
#include <QBrush>

class CORE_EXPORT QgsSimpleFillSymbolLayerV2 : public QgsFillSymbolLayerV2
{
  public:
    QgsSimpleFillSymbolLayerV2( QColor color = DEFAULT_SIMPLEFILL_COLOR,
                                Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE,
                                QColor borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR,
                                Qt::PenStyle borderStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE,
                                double borderWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;

    Qt::BrushStyle brushStyle() const { return mBrushStyle; }
    void setBrushStyle( Qt::BrushStyle style ) { mBrushStyle = style; }

    QColor borderColor() const { return mBorderColor; }
    void setBorderColor( QColor borderColor ) { mBorderColor = borderColor; }

    Qt::PenStyle borderStyle() const { return mBorderStyle; }
    void setBorderStyle( Qt::PenStyle borderStyle ) { mBorderStyle = borderStyle; }

    double borderWidth() const { return mBorderWidth; }
    void setBorderWidth( double borderWidth ) { mBorderWidth = borderWidth; }

    void setOffset( QPointF offset ) { mOffset = offset; }
    QPointF offset() { return mOffset; }

    void setBorderWidthUnit( QgsSymbolV2::OutputUnit unit ) { mBorderWidthUnit = unit; }
    QgsSymbolV2::OutputUnit borderWidthUnit() const { return mBorderWidthUnit; }

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

  protected:
    QBrush mBrush;
    QBrush mSelBrush;
    Qt::BrushStyle mBrushStyle;
    QColor mBorderColor;
    Qt::PenStyle mBorderStyle;
    double mBorderWidth;
    QgsSymbolV2::OutputUnit mBorderWidthUnit;
    QPen mPen;
    QPen mSelPen;

    QPointF mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;

  private:
    //helper functions for data defined symbology
    void applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, QBrush& brush, QPen& pen, QPen& selPen );
};

/**Base class for polygon renderers generating texture images*/
class CORE_EXPORT QgsImageFillSymbolLayer: public QgsFillSymbolLayerV2
{
  public:
    QgsImageFillSymbolLayer();
    virtual ~QgsImageFillSymbolLayer();
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    virtual QgsSymbolV2* subSymbol() { return mOutline; }
    virtual bool setSubSymbol( QgsSymbolV2* symbol );

    void setOutlineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mOutlineWidthUnit = unit; }
    QgsSymbolV2::OutputUnit outlineWidthUnit() const { return mOutlineWidthUnit; }

  protected:
    QBrush mBrush;

    /**Outline width*/
    double mOutlineWidth;
    QgsSymbolV2::OutputUnit mOutlineWidthUnit;

    /**Custom outline*/
    QgsLineSymbolV2* mOutline;

    virtual void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context ) { Q_UNUSED( context ); }
};

/**A class for svg fill patterns. The class automatically scales the pattern to
   the appropriate pixel dimensions of the output device*/
class CORE_EXPORT QgsSVGFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsSVGFillSymbolLayer( const QString& svgFilePath = "", double width = 20, double rotation = 0.0 );
    QgsSVGFillSymbolLayer( const QByteArray& svgData, double width = 20, double rotation = 0.0 );
    ~QgsSVGFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    //getters and setters
    void setSvgFilePath( const QString& svgPath );
    QString svgFilePath() const { return mSvgFilePath; }
    void setPatternWidth( double width ) { mPatternWidth = width;}
    double patternWidth() const { return mPatternWidth; }

    void setSvgFillColor( const QColor& c ) { mSvgFillColor = c; }
    QColor svgFillColor() const { return mSvgFillColor; }
    void setSvgOutlineColor( const QColor& c ) { mSvgOutlineColor = c; }
    QColor svgOutlineColor() const { return mSvgOutlineColor; }
    void setSvgOutlineWidth( double w ) { mSvgOutlineWidth = w; }
    double svgOutlineWidth() const { return mSvgOutlineWidth; }

    void setPatternWidthUnit( QgsSymbolV2::OutputUnit unit ) { mPatternWidthUnit = unit; }
    QgsSymbolV2::OutputUnit patternWidthUnit() const { return mPatternWidthUnit; }

    void setSvgOutlineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mSvgOutlineWidthUnit = unit; }
    QgsSymbolV2::OutputUnit svgOutlineWidthUnit() const { return mSvgOutlineWidthUnit; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

  protected:
    /**Width of the pattern (in output units)*/
    double mPatternWidth;
    QgsSymbolV2::OutputUnit mPatternWidthUnit;

    /**SVG data*/
    QByteArray mSvgData;
    /**Path to the svg file (or empty if constructed directly from data)*/
    QString mSvgFilePath;
    /**SVG view box (to keep the aspect ratio */
    QRectF mSvgViewBox;
    /** SVG pattern image
     * @note added in 1.9 */
    QImage* mSvgPattern;

    //param(fill), param(outline), param(outline-width) are going
    //to be replaced in memory
    QColor mSvgFillColor;
    QColor mSvgOutlineColor;
    double mSvgOutlineWidth;
    QgsSymbolV2::OutputUnit mSvgOutlineWidthUnit;

    void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context );

  private:
    /**Helper function that gets the view box from the byte array*/
    void storeViewBox();
    void setDefaultSvgParams(); //fills mSvgFillColor, mSvgOutlineColor, mSvgOutlineWidth with default values for mSvgFilePath

    /**Applies the svg pattern to the brush*/
    void applyPattern( QBrush& brush, const QString& svgFilePath, double patternWidth, QgsSymbolV2::OutputUnit patternWidthUnit, const QColor& svgFillColor, const QColor& svgOutlineColor,
                       double svgOutlineWidth, QgsSymbolV2::OutputUnit svgOutlineWidthUnit, const QgsSymbolV2RenderContext& context );
};

class CORE_EXPORT QgsLinePatternFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsLinePatternFillSymbolLayer();
    ~QgsLinePatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    QString ogrFeatureStyleWidth( double widthScaleFactor ) const;

    //getters and setters
    void setLineAngle( double a ) { mLineAngle = a; }
    double lineAngle() const { return mLineAngle; }
    void setDistance( double d ) { mDistance = d; }
    double distance() const { return mDistance; }
    void setLineWidth( double w ) { mLineWidth = w; }
    double lineWidth() const { return mLineWidth; }
    void setColor( const QColor& c ) { mColor = c; }
    QColor color() const { return mColor; }
    void setOffset( double offset ) { mOffset = offset; }
    double offset() const { return mOffset; }

    void setDistanceUnit( QgsSymbolV2::OutputUnit unit ) { mDistanceUnit = unit; }
    QgsSymbolV2::OutputUnit distanceUnit() const { return mDistanceUnit; }

    void setLineWidthUnit( QgsSymbolV2::OutputUnit unit ) { mLineWidthUnit = unit; }
    QgsSymbolV2::OutputUnit lineWidthUnit() const { return mLineWidthUnit; }

    void setOffsetUnit( QgsSymbolV2::OutputUnit unit ) { mOffsetUnit = unit; }
    QgsSymbolV2::OutputUnit offsetUnit() const { return mOffsetUnit; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

  protected:
    /**Distance (in mm or map units) between lines*/
    double mDistance;
    QgsSymbolV2::OutputUnit mDistanceUnit;
    /**Line width (in mm or map units)*/
    double mLineWidth;
    QgsSymbolV2::OutputUnit mLineWidthUnit;
    QColor mColor;
    /**Vector line angle in degrees (0 = horizontal, counterclockwise)*/
    double mLineAngle;
    /**Offset perpendicular to line direction*/
    double mOffset;
    QgsSymbolV2::OutputUnit mOffsetUnit;

    void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context );

  private:
    /**Applies the svg pattern to the brush*/
    void applyPattern( const QgsSymbolV2RenderContext& context, QBrush& brush, double lineAngle, double distance, double lineWidth, const QColor& color );
};

class CORE_EXPORT QgsPointPatternFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsPointPatternFillSymbolLayer();
    ~QgsPointPatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    //getters and setters
    double distanceX() const { return mDistanceX; }
    void setDistanceX( double d ) { mDistanceX = d; }

    double distanceY() const { return mDistanceY; }
    void setDistanceY( double d ) { mDistanceY = d; }

    double displacementX() const { return mDisplacementX; }
    void setDisplacementX( double d ) { mDisplacementX = d; }

    double displacementY() const { return mDisplacementY; }
    void setDisplacementY( double d ) { mDisplacementY = d; }

    bool setSubSymbol( QgsSymbolV2* symbol );
    virtual QgsSymbolV2* subSymbol() { return mMarkerSymbol; }

    void setDistanceXUnit( QgsSymbolV2::OutputUnit unit ) { mDistanceXUnit = unit; }
    QgsSymbolV2::OutputUnit distanceXUnit() const { return mDistanceXUnit; }

    void setDistanceYUnit( QgsSymbolV2::OutputUnit unit ) { mDistanceYUnit = unit; }
    QgsSymbolV2::OutputUnit distanceYUnit() const { return mDistanceYUnit; }

    void setDisplacementXUnit( QgsSymbolV2::OutputUnit unit ) { mDisplacementXUnit = unit; }
    QgsSymbolV2::OutputUnit displacementXUnit() const { return mDisplacementXUnit; }

    void setDisplacementYUnit( QgsSymbolV2::OutputUnit unit ) { mDisplacementYUnit = unit; }
    QgsSymbolV2::OutputUnit displacementYUnit() const { return mDisplacementYUnit; }

    void setOutputUnit( QgsSymbolV2::OutputUnit unit );
    QgsSymbolV2::OutputUnit outputUnit() const;

    const QgsExpression* dataDefinedProperty( const QString& property ) const;
    QString dataDefinedPropertyString( const QString& property ) const;
    void setDataDefinedProperty( const QString& property, const QString& expressionString );
    void removeDataDefinedProperty( const QString& property );
    void removeDataDefinedProperties();

    QSet<QString> usedAttributes() const;

  protected:
    QgsMarkerSymbolV2* mMarkerSymbol;
    double mDistanceX;
    QgsSymbolV2::OutputUnit mDistanceXUnit;
    double mDistanceY;
    QgsSymbolV2::OutputUnit mDistanceYUnit;
    double mDisplacementX;
    QgsSymbolV2::OutputUnit mDisplacementXUnit;
    double mDisplacementY;
    QgsSymbolV2::OutputUnit mDisplacementYUnit;

    QgsExpression* mDistanceXExpression;
    QgsExpression* mDistanceYExpression;
    QgsExpression* mDisplacementXExpression;
    QgsExpression* mDisplacementYExpression;

    void applyDataDefinedSettings( const QgsSymbolV2RenderContext& context );

  private:
    void applyPattern( const QgsSymbolV2RenderContext& context, QBrush& brush, double distanceX, double distanceY,
                       double displacementX, double displacementY );
    void prepareExpressions( const QgsVectorLayer* vl );
};

class CORE_EXPORT QgsCentroidFillSymbolLayerV2 : public QgsFillSymbolLayerV2
{
  public:
    QgsCentroidFillSymbolLayerV2();
    ~QgsCentroidFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );
    static QgsSymbolLayerV2* createFromSld( QDomElement &element );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

    void setColor( const QColor& color );

    QgsSymbolV2* subSymbol();
    bool setSubSymbol( QgsSymbolV2* symbol );

  protected:
    QgsMarkerSymbolV2* mMarker;
};

#endif
