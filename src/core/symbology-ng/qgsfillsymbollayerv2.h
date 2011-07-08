
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

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

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

  protected:
    QBrush mBrush;
    QBrush mSelBrush;
    Qt::BrushStyle mBrushStyle;
    QColor mBorderColor;
    Qt::PenStyle mBorderStyle;
    double mBorderWidth;
    QPen mPen;

    QPointF mOffset;
};

/**Base class for polygon renderers generating texture images*/
class CORE_EXPORT QgsImageFillSymbolLayer: public QgsFillSymbolLayerV2
{
  public:
    QgsImageFillSymbolLayer();
    virtual ~QgsImageFillSymbolLayer();
    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsSymbolV2* subSymbol() { return mOutline; }
    bool setSubSymbol( QgsSymbolV2* symbol );

  protected:
    QBrush mBrush;

    /**Outline width*/
    double mOutlineWidth;
    /**Custom outline*/
    QgsLineSymbolV2* mOutline;
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

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );
    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    //getters and setters
    void setSvgFilePath( const QString& svgPath );
    QString svgFilePath() const { return mSvgFilePath; }
    void setPatternWidth( double width ) { mPatternWidth = width;}
    double patternWidth() const { return mPatternWidth; }

  protected:
    /**Width of the pattern (in QgsSymbolV2 output units)*/
    double mPatternWidth;
    /**SVG data*/
    QByteArray mSvgData;
    /**Path to the svg file (or empty if constructed directly from data)*/
    QString mSvgFilePath;
    /**SVG view box (to keep the aspect ratio */
    QRectF mSvgViewBox;

  private:
    /**Helper function that gets the view box from the byte array*/
    void storeViewBox();
};

class CORE_EXPORT QgsLinePatternFillSymbolLayer: public QgsImageFillSymbolLayer
{
  public:
    QgsLinePatternFillSymbolLayer();
    ~QgsLinePatternFillSymbolLayer();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    //getters and setters
    void setAngle( double a ){ mAngle = a; }
    double angle() const { return mAngle; }
    void setDistance( double d ){ mDistance = d; }
    double distance() const { return mDistance; }
    void setLineWidth( double w ){ mLineWidth = w; }
    double lineWidth() const { return mLineWidth; }
    void setColor( const QColor& c ){ mColor = c; }
    QColor color() const{ return mColor; }

  protected:
    /**Distance (in mm or map units) between lines*/
    double mDistance;
    /**Line width (in mm or map units)*/
    double mLineWidth;
    QColor mColor;
    //todo: line type
     double mAngle;
};



class CORE_EXPORT QgsCentroidFillSymbolLayerV2 : public QgsFillSymbolLayerV2
{
  public:
    QgsCentroidFillSymbolLayerV2();
    ~QgsCentroidFillSymbolLayerV2();

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    void setColor( const QColor& color );

    QgsSymbolV2* subSymbol();
    bool setSubSymbol( QgsSymbolV2* symbol );

  protected:
    QgsMarkerSymbolV2* mMarker;
};

#endif
