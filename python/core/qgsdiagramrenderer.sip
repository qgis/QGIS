/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdiagramrenderer.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsDiagramLayerSettings
{
%Docstring
 Stores the settings for rendering of all diagrams for a layer.

QgsDiagramSettings stores the settings related to rendering the individual diagrams themselves, while
QgsDiagramLayerSettings stores settings which control how ALL diagrams within a layer are rendered.
%End

%TypeHeaderCode
#include "qgsdiagramrenderer.h"
%End
  public:

    enum Placement
    {
      AroundPoint,
      OverPoint,
      Line,
      Curved,
      Horizontal,
      Free
    };

    enum LinePlacementFlag
    {
      OnLine,
      AboveLine,
      BelowLine,
      MapOrientation,
    };
    typedef QFlags<QgsDiagramLayerSettings::LinePlacementFlag> LinePlacementFlags;


    enum Property
    {
      BackgroundColor,
      StrokeColor,
      StrokeWidth,
      PositionX,
      PositionY,
      Distance,
      Priority,
      ZIndex,
      IsObstacle,
      Show,
      AlwaysShow,
      StartAngle,
    };

    static const QgsPropertiesDefinition &propertyDefinitions();
%Docstring
Returns the diagram property definitions.

.. versionadded:: 3.0
%End

    QgsDiagramLayerSettings();
%Docstring
Constructor for QgsDiagramLayerSettings.
%End

    QgsDiagramLayerSettings( const QgsDiagramLayerSettings &rh );
%Docstring
Copy constructor
%End


    ~QgsDiagramLayerSettings();

    Placement placement() const;
%Docstring
Returns the diagram placement.

.. seealso:: :py:func:`setPlacement`

.. versionadded:: 2.16
%End

    void setPlacement( Placement value );
%Docstring
Sets the diagram placement.

:param value: placement value

.. seealso:: :py:func:`placement`

.. versionadded:: 2.16
%End

    LinePlacementFlags linePlacementFlags() const;
%Docstring
Returns the diagram placement flags. These are only used if the diagram placement
is set to a line type.

.. seealso:: :py:func:`setLinePlacementFlags`

.. versionadded:: 2.16
%End

    void setLinePlacementFlags( LinePlacementFlags flags );
%Docstring
Sets the the diagram placement flags. These are only used if the diagram placement
is set to a line type.

:param flags: placement value

.. seealso:: :py:func:`getPlacement`

.. versionadded:: 2.16
%End

    int priority() const;
%Docstring
Returns the diagram priority.

:return: diagram priority, where 0 = low and 10 = high

.. note::

   placement priority is shared with labeling, so diagrams with a high priority may displace labels
and vice-versa

.. seealso:: :py:func:`setPriority`

.. versionadded:: 2.16
%End

    void setPriority( int value );
%Docstring
Sets the diagram priority.

:param value: priority, where 0 = low and 10 = high

.. seealso:: :py:func:`priority`

.. versionadded:: 2.16
%End

    double zIndex() const;
%Docstring
Returns the diagram z-index. Diagrams (or labels) with a higher z-index are drawn over diagrams
with a lower z-index.

.. note::

   z-index ordering is shared with labeling, so diagrams with a high z-index may be drawn over labels
with a low z-index and vice-versa

.. seealso:: :py:func:`setZIndex`

.. versionadded:: 2.16
%End

    void setZIndex( double index );
%Docstring
Sets the diagram z-index. Diagrams (or labels) with a higher z-index are drawn over diagrams
with a lower z-index.

:param index: diagram z-index

.. seealso:: :py:func:`zIndex`

.. versionadded:: 2.16
%End

    bool isObstacle() const;
%Docstring
Returns whether the feature associated with a diagram acts as an obstacle for other labels or diagrams.

.. seealso:: :py:func:`setIsObstacle`

.. versionadded:: 2.16
%End

    void setIsObstacle( bool isObstacle );
%Docstring
Sets whether the feature associated with a diagram acts as an obstacle for other labels or diagrams.

:param isObstacle: set to true for feature to act as obstacle

.. seealso:: :py:func:`isObstacle`

.. versionadded:: 2.16
%End

    double distance() const;
%Docstring
Returns the distance between the diagram and the feature (in mm).

.. seealso:: :py:func:`setDistance`

.. versionadded:: 2.16
%End

    void setDistance( double distance );
%Docstring
Sets the distance between the diagram and the feature.

:param distance: distance in mm

.. seealso:: :py:func:`distance`

.. versionadded:: 2.16
%End

    QgsDiagramRenderer *renderer();
%Docstring
Returns the diagram renderer associated with the layer.

.. seealso:: :py:func:`setRenderer`

.. versionadded:: 2.16
%End


    void setRenderer( QgsDiagramRenderer *diagramRenderer /Transfer/ );
%Docstring
Sets the diagram renderer associated with the layer.

:param diagramRenderer: diagram renderer. Ownership is transferred to the object.

.. seealso:: :py:func:`renderer`

.. versionadded:: 2.16
%End

    QgsCoordinateTransform coordinateTransform() const;
%Docstring
Returns the coordinate transform associated with the layer, or an
invalid transform if no transformation is required.

.. seealso:: :py:func:`setCoordinateTransform`

.. versionadded:: 2.16
%End

    void setCoordinateTransform( const QgsCoordinateTransform &transform );
%Docstring
Sets the coordinate transform associated with the layer.

:param transform: coordinate transform. Ownership is transferred to the object.

.. seealso:: :py:func:`coordinateTransform`

.. versionadded:: 2.16
%End

    bool showAllDiagrams() const;
%Docstring
Returns whether the layer should show all diagrams, including overlapping diagrams

.. seealso:: :py:func:`setShowAllDiagrams`

.. versionadded:: 2.16
%End

    void setShowAllDiagrams( bool showAllDiagrams );
%Docstring
Sets whether the layer should show all diagrams, including overlapping diagrams

:param showAllDiagrams: set to true to show all diagrams

.. seealso:: :py:func:`showAllDiagrams`

.. versionadded:: 2.16
%End

    void readXml( const QDomElement &elem );
%Docstring
Reads the diagram settings from a DOM element.

.. seealso:: :py:func:`writeXml`
%End

    void writeXml( QDomElement &layerElem, QDomDocument &doc ) const;
%Docstring
Writes the diagram settings to a DOM element.

.. seealso:: :py:func:`readXml`
%End

    bool prepare( const QgsExpressionContext &context = QgsExpressionContext() ) const;
%Docstring
Prepares the diagrams for a specified expression context. Calling prepare before rendering
multiple diagrams allows precalculation of expensive setup tasks such as parsing expressions.
Returns true if preparation was successful.

.. versionadded:: 3.0
%End

    QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const;
%Docstring
Returns the set of any fields referenced by the layer's diagrams.

:param context: expression context the diagrams will be drawn using

.. versionadded:: 2.16
%End

    QgsPropertyCollection &dataDefinedProperties();
%Docstring
Returns a reference to the diagram's property collection, used for data defined overrides.

.. versionadded:: 3.0

.. seealso:: :py:func:`setDataDefinedProperties`
%End


    void setDataDefinedProperties( const QgsPropertyCollection &collection );
%Docstring
Sets the diagram's property collection, used for data defined overrides.

:param collection: property collection. Existing properties will be replaced.

.. versionadded:: 3.0

.. seealso:: :py:func:`dataDefinedProperties`
%End

};


class QgsDiagramSettings
{
%Docstring
 Stores the settings for rendering a single diagram.

QgsDiagramSettings stores the settings related to rendering the individual diagrams themselves, while
QgsDiagramLayerSettings stores settings which control how ALL diagrams within a layer are rendered.
%End

%TypeHeaderCode
#include "qgsdiagramrenderer.h"
%End
  public:

    enum LabelPlacementMethod
    {
      Height,
      XHeight
    };

    enum DiagramOrientation
    {
      Up,
      Down,
      Left,
      Right
    };

    QgsDiagramSettings();
%Docstring
Constructor for QgsDiagramSettings
%End

    bool enabled;
    QFont font;
    QList< QColor > categoryColors;
    QList< QString > categoryAttributes;
    QList< QString > categoryLabels;
    QSizeF size; //size

    QgsUnitTypes::RenderUnit sizeType;

    QgsMapUnitScale sizeScale;

    QgsUnitTypes::RenderUnit lineSizeUnit;

    QgsMapUnitScale lineSizeScale;

    QColor backgroundColor;
    QColor penColor;
    double penWidth;
    LabelPlacementMethod labelPlacementMethod;
    DiagramOrientation diagramOrientation;
    double barWidth;

    double opacity;

    bool scaleByArea;

    double rotationOffset;

    bool scaleBasedVisibility;

    double maximumScale;

    double minimumScale;

    double minimumSize;

    void readXml( const QDomElement &elem );
%Docstring
Reads diagram settings from XML
%End
    void writeXml( QDomElement &rendererElem, QDomDocument &doc ) const;
%Docstring
Writes diagram settings to XML
%End

    QList< QgsLayerTreeModelLegendNode * > legendItems( QgsLayerTreeLayer *nodeLayer ) const /Factory/;
%Docstring
Returns list of legend nodes for the diagram

.. note::

   caller is responsible for deletion of :py:class:`QgsLayerTreeModelLegendNodes`

.. versionadded:: 2.10
%End

};

class QgsDiagramInterpolationSettings
{
%Docstring
Additional diagram settings for interpolated size rendering.
%End

%TypeHeaderCode
#include "qgsdiagramrenderer.h"
%End
  public:
    QSizeF lowerSize;
    QSizeF upperSize;
    double lowerValue;
    double upperValue;

    QString classificationField;

    QString classificationAttributeExpression;
    bool classificationAttributeIsExpression;
};



class QgsDiagramRenderer
{
%Docstring
 Evaluates and returns the diagram settings relating to a diagram for a specific feature.
%End

%TypeHeaderCode
#include "qgsdiagramrenderer.h"
%End
%ConvertToSubClassCode
    if ( sipCpp->rendererName() == QStringLiteral( "SingleCategory" ) )
      sipType = sipType_QgsSingleCategoryDiagramRenderer;
    else if ( sipCpp->rendererName() == QStringLiteral( "LinearlyInterpolated" ) )
      sipType = sipType_QgsLinearlyInterpolatedDiagramRenderer;
    else
      sipType = NULL;
%End
  public:

    QgsDiagramRenderer();
%Docstring
Constructor for QgsDiagramRenderer.
%End
    virtual ~QgsDiagramRenderer();

    virtual QgsDiagramRenderer *clone() const = 0 /Factory/;
%Docstring
Returns new instance that is equivalent to this one

.. versionadded:: 2.4
%End

    virtual QSizeF sizeMapUnits( const QgsFeature &feature, const QgsRenderContext &c ) const;
%Docstring
Returns size of the diagram for a feature in map units. Returns an invalid QSizeF in case of error
%End

    virtual QString rendererName() const = 0;

    virtual QList<QString> diagramAttributes() const = 0;
%Docstring
Returns attribute indices needed for diagram rendering
%End

    virtual QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const;
%Docstring
Returns the set of any fields required for diagram rendering

:param context: expression context the diagrams will be drawn using

.. versionadded:: 2.16
%End

    void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, QPointF pos, const QgsPropertyCollection &properties = QgsPropertyCollection() ) const;
%Docstring
Renders the diagram for a specified feature at a specific position in the passed render context.
%End

    void setDiagram( QgsDiagram *d /Transfer/ );
    QgsDiagram *diagram() const;

    virtual QList<QgsDiagramSettings> diagramSettings() const = 0;
%Docstring
Returns list with all diagram settings in the renderer
%End

    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;
%Docstring
Reads diagram state from a DOM element. Subclasses should ensure that _readXml() is called
by their readXml implementation to restore the general QgsDiagramRenderer settings.

.. seealso:: :py:func:`writeXml`
%End

    virtual void writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;
%Docstring
Writes diagram state to a DOM element. Subclasses should ensure that _writeXml() is called
by their writeXml implementation to save the general QgsDiagramRenderer settings.

.. seealso:: :py:func:`readXml`
%End

    virtual QList< QgsLayerTreeModelLegendNode * > legendItems( QgsLayerTreeLayer *nodeLayer ) const /Factory/;
%Docstring
Returns list of legend nodes for the diagram

.. note::

   caller is responsible for deletion of :py:class:`QgsLayerTreeModelLegendNodes`

.. versionadded:: 2.10
%End

    bool attributeLegend() const;
%Docstring
Returns true if renderer will show legend items for diagram attributes.

.. versionadded:: 2.16

.. seealso:: :py:func:`setAttributeLegend`

.. seealso:: :py:func:`sizeLegend`
%End

    void setAttributeLegend( bool enabled );
%Docstring
Sets whether the renderer will show legend items for diagram attributes.

:param enabled: set to true to show diagram attribute legend

.. versionadded:: 2.16

.. seealso:: :py:func:`attributeLegend`

.. seealso:: :py:func:`setSizeLegend`
%End

  protected:
    QgsDiagramRenderer( const QgsDiagramRenderer &other );

    virtual bool diagramSettings( const QgsFeature &feature, const QgsRenderContext &c, QgsDiagramSettings &s ) const = 0;
%Docstring
Returns diagram settings for a feature (or false if the diagram for the feature is not to be rendered). Used internally within renderDiagram()

:param feature: the feature
:param c: render context
:param s: out: diagram settings for the feature
%End

    virtual QSizeF diagramSize( const QgsFeature &features, const QgsRenderContext &c ) const = 0;
%Docstring
Returns size of the diagram (in painter units) or an invalid size in case of error
%End

    void convertSizeToMapUnits( QSizeF &size, const QgsRenderContext &context ) const;
%Docstring
Converts size from mm to map units
%End

    static int dpiPaintDevice( const QPainter * );
%Docstring
Returns the paint device dpi (or -1 in case of error
%End


    void _readXml( const QDomElement &elem, const QgsReadWriteContext &context );
%Docstring
Reads internal QgsDiagramRenderer state from a DOM element.

.. seealso:: _writeXml
%End

    void _writeXml( QDomElement &rendererElem, QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
Writes internal QgsDiagramRenderer diagram state to a DOM element.

.. seealso:: _readXml
%End


};

class QgsSingleCategoryDiagramRenderer : QgsDiagramRenderer
{
%Docstring
Renders the diagrams for all features with the same settings
%End

%TypeHeaderCode
#include "qgsdiagramrenderer.h"
%End
  public:

    QgsSingleCategoryDiagramRenderer();
%Docstring
Constructor for QgsSingleCategoryDiagramRenderer
%End

    virtual QgsSingleCategoryDiagramRenderer *clone() const /Factory/;


    virtual QString rendererName() const;

    virtual QList<QString> diagramAttributes() const;

    void setDiagramSettings( const QgsDiagramSettings &s );

    virtual QList<QgsDiagramSettings> diagramSettings() const;


    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    virtual void writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const;


    virtual QList< QgsLayerTreeModelLegendNode * > legendItems( QgsLayerTreeLayer *nodeLayer ) const /Factory/;


  protected:
    virtual bool diagramSettings( const QgsFeature &feature, const QgsRenderContext &c, QgsDiagramSettings &s ) const;


    virtual QSizeF diagramSize( const QgsFeature &, const QgsRenderContext &c ) const;


};

class QgsLinearlyInterpolatedDiagramRenderer : QgsDiagramRenderer
{

%TypeHeaderCode
#include "qgsdiagramrenderer.h"
%End
  public:
    QgsLinearlyInterpolatedDiagramRenderer();
    ~QgsLinearlyInterpolatedDiagramRenderer();

    virtual QgsLinearlyInterpolatedDiagramRenderer *clone() const /Factory/;


    virtual QList<QgsDiagramSettings> diagramSettings() const;

%Docstring
Returns list with all diagram settings in the renderer
%End

    void setDiagramSettings( const QgsDiagramSettings &s );

    virtual QList<QString> diagramAttributes() const;


    virtual QSet< QString > referencedFields( const QgsExpressionContext &context = QgsExpressionContext() ) const;


    virtual QString rendererName() const;

    void setLowerValue( double val );
    double lowerValue() const;

    void setUpperValue( double val );
    double upperValue() const;

    void setLowerSize( QSizeF s );
    QSizeF lowerSize() const;

    void setUpperSize( QSizeF s );
    QSizeF upperSize() const;

    QString classificationField() const;
%Docstring
Returns the field name used for interpolating the diagram size.

.. seealso:: :py:func:`setClassificationField`

.. versionadded:: 3.0
%End

    void setClassificationField( const QString &field );
%Docstring
Sets the field name used for interpolating the diagram size.

.. seealso:: :py:func:`classificationField`

.. versionadded:: 3.0
%End

    QString classificationAttributeExpression() const;
    void setClassificationAttributeExpression( const QString &expression );

    bool classificationAttributeIsExpression() const;
    void setClassificationAttributeIsExpression( bool isExpression );

    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    virtual void writeXml( QDomElement &layerElem, QDomDocument &doc, const QgsReadWriteContext &context ) const;


    virtual QList< QgsLayerTreeModelLegendNode * > legendItems( QgsLayerTreeLayer *nodeLayer ) const /Factory/;


    void setDataDefinedSizeLegend( QgsDataDefinedSizeLegend *settings /Transfer/ );
%Docstring
Configures appearance of legend. Takes ownership of the passed settings objects.

.. versionadded:: 3.0
%End

    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const;
%Docstring
Returns configuration of appearance of legend. Will return null if no configuration has been set.

.. versionadded:: 3.0
%End

  protected:
    virtual bool diagramSettings( const QgsFeature &feature, const QgsRenderContext &c, QgsDiagramSettings &s ) const;


    virtual QSizeF diagramSize( const QgsFeature &, const QgsRenderContext &c ) const;


    QgsLinearlyInterpolatedDiagramRenderer( const QgsLinearlyInterpolatedDiagramRenderer &other );
%Docstring
Copy constructor
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdiagramrenderer.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
