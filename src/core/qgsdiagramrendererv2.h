/***************************************************************************
    qgsdiagramrendererv2.h
    ---------------------
    begin                : March 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDIAGRAMRENDERERV2_H
#define QGSDIAGRAMRENDERERV2_H

#include <QColor>
#include <QFont>
#include <QList>
#include <QPointF>
#include <QSizeF>
#include <QDomDocument>

#include "qgsfeature.h"

class QgsDiagram;
class QgsDiagramRendererV2;
class QgsFeature;
class QgsRenderContext;
class QDomElement;
class QgsPalGeometry;
class QgsCoordinateTransform;
class QgsMapToPixel;
class QgsVectorLayer;

namespace pal { class Layer; }

class CORE_EXPORT QgsDiagramLayerSettings
{
  public:
    //avoid inclusion of QgsPalLabeling
    enum Placement
    {
      AroundPoint = 0, // Point / Polygon
      OverPoint, // Point / Polygon
      Line, // Line / Polygon
      Curved, // Line
      Horizontal, // Polygon
      Free // Polygon
    };

    enum LinePlacementFlags
    {
      OnLine    = 1,
      AboveLine = 2,
      BelowLine = 4,
      MapOrientation = 8
    };

    QgsDiagramLayerSettings();

    ~QgsDiagramLayerSettings();

    //pal placement properties
    Placement placement;
    LinePlacementFlags placementFlags;
    int priority; // 0 = low, 10 = high
    bool obstacle; // whether it's an obstacle
    double dist; // distance from the feature (in mm)
    QgsDiagramRendererV2* renderer; // if any renderer is assigned, it is owned by this class

    //assigned when layer gets prepared
    pal::Layer* palLayer;
    const QgsCoordinateTransform* ct;
    const QgsMapToPixel* xform;
    QList<QgsPalGeometry*> geometries;
    QgsFields fields;

    int xPosColumn; //attribute index for x coordinate (or -1 if position not data defined)
    int yPosColumn;//attribute index for y coordinate (or -1 if position not data defined)
    bool showAll;

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;
};

//diagram settings for rendering
class CORE_EXPORT QgsDiagramSettings
{
  public:
    enum SizeType
    {
      MM,
      MapUnits
    };

    enum LabelPlacementMethod
    {
      Height,
      XHeight
    };

    //! Orientation of histogram
    enum DiagramOrientation
    {
      Up,
      Down,
      Left,
      Right
    };

    QgsDiagramSettings()
        : enabled( true )
        , sizeType( MM )
        , penWidth( 0.0 )
        , labelPlacementMethod( QgsDiagramSettings::Height )
        , diagramOrientation( QgsDiagramSettings::Up )
        , barWidth( 5.0 )
        , transparency( 0 )
        , scaleByArea( true )
        , angleOffset( 90 * 16 ) //top
        , minScaleDenominator( -1 )
        , maxScaleDenominator( -1 )
        , minimumSize( 0.0 )
    {}
    bool enabled;
    QFont font;
    QList< QColor > categoryColors;
    QList< QString > categoryAttributes;
    QSizeF size; //size
    SizeType sizeType; //mm or map units
    QColor backgroundColor;
    QColor penColor;
    double penWidth;
    LabelPlacementMethod labelPlacementMethod;
    DiagramOrientation diagramOrientation;
    double barWidth;
    int transparency; // 0 - 100
    bool scaleByArea;
    int angleOffset;

    //scale range (-1 if no lower / upper bound )
    double minScaleDenominator;
    double maxScaleDenominator;

    //! Scale diagrams smaller than mMinimumSize to mMinimumSize
    double minimumSize;

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void writeXML( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;
};

//additional diagram settings for interpolated size rendering
class CORE_EXPORT QgsDiagramInterpolationSettings
{
  public:
    QSizeF lowerSize;
    QSizeF upperSize;
    double lowerValue;
    double upperValue;
    /**Index of the classification attribute*/
    int classificationAttribute;
    QString classificationAttributeExpression;
    bool classificationAttributeIsExpression;
};

/**Returns diagram settings for a feature*/
class CORE_EXPORT QgsDiagramRendererV2
{
  public:

    QgsDiagramRendererV2();
    virtual ~QgsDiagramRendererV2();

    /** Returns new instance that is equivalent to this one
     * @note added in 2.4 */
    virtual QgsDiagramRendererV2* clone() const = 0;

    /**Returns size of the diagram for a feature in map units. Returns an invalid QSizeF in case of error*/
    virtual QSizeF sizeMapUnits( const QgsFeature& feature, const QgsRenderContext& c );

    virtual QString rendererName() const = 0;

    /**Returns attribute indices needed for diagram rendering*/
    virtual QList<QString> diagramAttributes() const = 0;

    void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QPointF& pos );

    void setDiagram( QgsDiagram* d );
    QgsDiagram* diagram() const { return mDiagram; }

    /**Returns list with all diagram settings in the renderer*/
    virtual QList<QgsDiagramSettings> diagramSettings() const = 0;

    virtual void readXML( const QDomElement& elem, const QgsVectorLayer* layer ) = 0;
    virtual void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const = 0;

  protected:
    QgsDiagramRendererV2( const QgsDiagramRendererV2& other );

    /**Returns diagram settings for a feature (or false if the diagram for the feature is not to be rendered). Used internally within renderDiagram()
     * @param feature the feature
     * @param c render context
     * @param s out: diagram settings for the feature
     */
    virtual bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s ) = 0;

    /**Returns size of the diagram (in painter units) or an invalid size in case of error*/
    virtual QSizeF diagramSize( const QgsFeature& features, const QgsRenderContext& c ) = 0;

    /**Converts size from mm to map units*/
    void convertSizeToMapUnits( QSizeF& size, const QgsRenderContext& context ) const;

    /**Returns the paint device dpi (or -1 in case of error*/
    static int dpiPaintDevice( const QPainter* );

    //read / write diagram
    void _readXML( const QDomElement& elem, const QgsVectorLayer* layer );
    void _writeXML( QDomElement& rendererElem, QDomDocument& doc, const QgsVectorLayer* layer ) const;

    /**Reference to the object that does the real diagram rendering*/
    QgsDiagram* mDiagram;
};

/**Renders the diagrams for all features with the same settings*/
class CORE_EXPORT QgsSingleCategoryDiagramRenderer : public QgsDiagramRendererV2
{
  public:
    QgsSingleCategoryDiagramRenderer();
    ~QgsSingleCategoryDiagramRenderer();

    QgsDiagramRendererV2* clone() const override;

    QString rendererName() const override { return "SingleCategory"; }

    QList<QString> diagramAttributes() const override { return mSettings.categoryAttributes; }

    void setDiagramSettings( const QgsDiagramSettings& s ) { mSettings = s; }

    QList<QgsDiagramSettings> diagramSettings() const override;

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer ) override;
    void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const override;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s ) override;

    QSizeF diagramSize( const QgsFeature&, const QgsRenderContext& c ) override;

  private:
    QgsDiagramSettings mSettings;
};

class CORE_EXPORT QgsLinearlyInterpolatedDiagramRenderer : public QgsDiagramRendererV2
{
  public:
    QgsLinearlyInterpolatedDiagramRenderer();
    ~QgsLinearlyInterpolatedDiagramRenderer();

    QgsDiagramRendererV2* clone() const override;

    /**Returns list with all diagram settings in the renderer*/
    QList<QgsDiagramSettings> diagramSettings() const override;

    void setDiagramSettings( const QgsDiagramSettings& s ) { mSettings = s; }

    QList<QString> diagramAttributes() const override;

    QString rendererName() const override { return "LinearlyInterpolated"; }

    void setLowerValue( double val ) { mInterpolationSettings.lowerValue = val; }
    double lowerValue() const { return mInterpolationSettings.lowerValue; }

    void setUpperValue( double val ) { mInterpolationSettings.upperValue = val; }
    double upperValue() const { return mInterpolationSettings.upperValue; }

    void setLowerSize( QSizeF s ) { mInterpolationSettings.lowerSize = s; }
    QSizeF lowerSize() const { return mInterpolationSettings.lowerSize; }

    void setUpperSize( QSizeF s ) { mInterpolationSettings.upperSize = s; }
    QSizeF upperSize() const { return mInterpolationSettings.upperSize; }

    int classificationAttribute() const { return mInterpolationSettings.classificationAttribute; }
    void setClassificationAttribute( int index ) { mInterpolationSettings.classificationAttribute = index; }

    QString classificationAttributeExpression() const { return mInterpolationSettings.classificationAttributeExpression; }
    void setClassificationAttributeExpression( QString expression ) { mInterpolationSettings.classificationAttributeExpression = expression; }

    bool classificationAttributeIsExpression() const { return mInterpolationSettings.classificationAttributeIsExpression; }
    void setClassificationAttributeIsExpression( bool isExpression ) { mInterpolationSettings.classificationAttributeIsExpression = isExpression; }

    void readXML( const QDomElement& elem, const QgsVectorLayer* layer ) override;
    void writeXML( QDomElement& layerElem, QDomDocument& doc, const QgsVectorLayer* layer ) const override;

  protected:
    bool diagramSettings( const QgsFeature &feature, const QgsRenderContext& c, QgsDiagramSettings& s ) override;

    QSizeF diagramSize( const QgsFeature&, const QgsRenderContext& c ) override;

  private:
    QgsDiagramSettings mSettings;
    QgsDiagramInterpolationSettings mInterpolationSettings;
};

#endif // QGSDIAGRAMRENDERERV2_H
