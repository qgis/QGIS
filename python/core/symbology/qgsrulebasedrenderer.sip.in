/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsrulebasedrenderer.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRuleBasedRenderer : QgsFeatureRenderer
{
%Docstring
When drawing a vector layer with rule-based renderer, it goes through
the rules and draws features with symbols from rules that match.
%End

%TypeHeaderCode
#include "qgsrulebasedrenderer.h"
%End
  public:

    enum FeatureFlags
    {
      FeatIsSelected,
      FeatDrawMarkers
    };

    struct FeatureToRender
    {
      FeatureToRender( QgsFeature &_f, int _flags );
      QgsFeature feat;
      int flags; // selected and/or draw markers
    };

    struct RenderJob
    {
      RenderJob( QgsRuleBasedRenderer::FeatureToRender &_ftr, QgsSymbol *_s );
      QgsRuleBasedRenderer::FeatureToRender &ftr;
      QgsSymbol *symbol;
    };

    struct RenderLevel
    {
      explicit RenderLevel( int z );
      ~RenderLevel();
      int zIndex;
      QList<QgsRuleBasedRenderer::RenderJob *> jobs;


      RenderLevel( const QgsRuleBasedRenderer::RenderLevel &other );

    };

    typedef QList<QgsRuleBasedRenderer::RenderLevel> RenderQueue;

    typedef QList<QgsRuleBasedRenderer::Rule *> RuleList;

    class Rule
{
%Docstring
This class keeps data about a rules for rule-based renderer.
A rule consists of a symbol, filter expression and range of scales.
If filter is empty, it matches all features.
If scale range has both values zero, it matches all scales.
If one of the min/max scale denominators is zero, there is no lower/upper bound for scales.
A rule matches if both filter and scale range match.
%End

%TypeHeaderCode
#include "qgsrulebasedrenderer.h"
%End
      public:
        enum RenderResult
        {
          Filtered,
          Inactive,
          Rendered
        };

        Rule( QgsSymbol *symbol /Transfer/, int maximumScale = 0, int minimumScale = 0, const QString &filterExp = QString(),
              const QString &label = QString(), const QString &description = QString(), bool elseRule = false );
%Docstring
Constructor takes ownership of the symbol
%End
        ~Rule();


        QString dump( int indent = 0 ) const;
%Docstring
Dump for debug purpose

:param indent: How many characters to indent. Will increase by two with every of the recursive calls

:return: A string representing this rule
%End

        QSet<QString> usedAttributes( const QgsRenderContext &context ) const;
%Docstring
Return the attributes used to evaluate the expression of this rule

:return: A set of attribute names
%End

        bool needsGeometry() const;
%Docstring
Returns true if this rule or one of its chilren needs the geometry to be applied.
%End

        QgsSymbolList symbols( const QgsRenderContext &context = QgsRenderContext() ) const;
%Docstring

.. note::

   available in Python bindings as symbol2
%End

        QgsLegendSymbolList legendSymbolItems( int currentLevel = -1 ) const;
%Docstring

.. versionadded:: 2.6
%End

        bool isFilterOK( QgsFeature &f, QgsRenderContext *context = 0 ) const;
%Docstring
Check if a given feature shall be rendered by this rule

:param f:         The feature to test
:param context:   The context in which the rendering happens

:return:          True if the feature shall be rendered
%End

        bool isScaleOK( double scale ) const;
%Docstring
Check if this rule applies for a given ``scale``.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
If set to 0, it will always return true.

:return: If the rule will be evaluated at this scale
%End

        QgsSymbol *symbol();
        QString label() const;
        bool dependsOnScale() const;

        double maximumScale() const;
%Docstring
Returns the maximum map scale (i.e. most "zoomed in" scale) at which the rule will be active.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A scale of 0 indicates no maximum scale visibility.

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`setMaximumScale`

.. versionadded:: 3.0
%End

        double minimumScale() const;
%Docstring
Returns the minimum map scale (i.e. most "zoomed out" scale) at which the rule will be active.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A scale of 0 indicates no minimum scale visibility.

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`setMinimumScale`

.. versionadded:: 3.0
%End

        QgsExpression *filter() const;
%Docstring
A filter that will check if this rule applies

:return: An expression
%End

        QString filterExpression() const;
%Docstring
A filter that will check if this rule applies

:return: An expression
%End

        QString description() const;
%Docstring
A human readable description for this rule

:return: Description
%End

        bool active() const;
%Docstring
Returns if this rule is active

:return: True if the rule is active
%End

        QString ruleKey() const;
%Docstring
Unique rule identifier (for identification of rule within renderer)

.. versionadded:: 2.6
%End

        void setRuleKey( const QString &key );
%Docstring
Override the assigned rule key (should be used just internally by rule-based renderer)

.. versionadded:: 2.6
%End

        void setSymbol( QgsSymbol *sym /Transfer/ );
%Docstring
set a new symbol (or NULL). Deletes old symbol.
%End
        void setLabel( const QString &label );

        void setMinimumScale( double scale );
%Docstring
Sets the minimum map ``scale`` (i.e. most "zoomed out" scale) at which the rule will be active.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A ``scale`` of 0 indicates no minimum scale visibility.

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`setMaximumScale`
%End

        void setMaximumScale( double scale );
%Docstring
Sets the maximum map ``scale`` (i.e. most "zoomed in" scale) at which the rule will be active.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A ``scale`` of 0 indicates no maximum scale visibility.

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`setMinimumScale`
%End

        void setFilterExpression( const QString &filterExp );
%Docstring
Set the expression used to check if a given feature shall be rendered with this rule

:param filterExp: An expression
%End

        void setDescription( const QString &description );
%Docstring
Set a human readable description for this rule

:param description: Description
%End

        void setActive( bool state );
%Docstring
Sets if this rule is active

:param state: Determines if the rule should be activated or deactivated
%End

        QgsRuleBasedRenderer::Rule *clone() const /Factory/;
%Docstring
clone this rule, return new instance
%End

        void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const;

        static QgsRuleBasedRenderer::Rule *createFromSld( QDomElement &element, QgsWkbTypes::GeometryType geomType ) /Factory/;
%Docstring
Create a rule from the SLD provided in element and for the specified geometry type.
%End

        QDomElement save( QDomDocument &doc, QgsSymbolMap &symbolMap ) const;

        bool startRender( QgsRenderContext &context, const QgsFields &fields, QString &filter );
%Docstring
prepare the rule for rendering and its children (build active children array)
%End

        QSet<int> collectZLevels();
%Docstring
get all used z-levels from this rule and children
%End


        QgsRuleBasedRenderer::Rule::RenderResult renderFeature( QgsRuleBasedRenderer::FeatureToRender &featToRender, QgsRenderContext &context, QgsRuleBasedRenderer::RenderQueue &renderQueue );
%Docstring
Render a given feature, will recursively call subclasses and only render if the constraints apply.

:param featToRender: The feature to render
:param context:      The rendering context
:param renderQueue:  The rendering queue to which the feature should be added

:return:             The result of the rendering. In explicit if the feature is added to the queue or
the reason for not rendering the feature.
%End

        bool willRenderFeature( QgsFeature &feat, QgsRenderContext *context = 0 );
%Docstring
only tell whether a feature will be rendered without actually rendering it
%End

        QgsSymbolList symbolsForFeature( QgsFeature &feat, QgsRenderContext *context = 0 );
%Docstring
tell which symbols will be used to render the feature
%End

        QSet< QString > legendKeysForFeature( QgsFeature &feat, QgsRenderContext *context = 0 );
%Docstring
Returns which legend keys match the feature

.. versionadded:: 2.14
%End

        QgsRuleBasedRenderer::RuleList rulesForFeature( QgsFeature &feat, QgsRenderContext *context = 0 );
%Docstring
tell which rules will be used to render the feature
%End

        void stopRender( QgsRenderContext &context );
%Docstring
Stop a rendering process. Used to clean up the internal state of this rule

:param context: The rendering context
%End

        static QgsRuleBasedRenderer::Rule *create( QDomElement &ruleElem, QgsSymbolMap &symbolMap ) /Factory/;
%Docstring
Create a rule from an XML definition

:param ruleElem:  The XML rule element
:param symbolMap: Symbol map

:return: A new rule
%End

        QgsRuleBasedRenderer::RuleList &children();
%Docstring
Return all children rules of this rule

:return: A list of rules
%End

        QgsRuleBasedRenderer::RuleList descendants() const;
%Docstring
Returns all children, grand-children, grand-grand-children, grand-gra... you get it

:return: A list of descendant rules
%End

        QgsRuleBasedRenderer::Rule *parent();
%Docstring
The parent rule

:return: Parent rule
%End

        void appendChild( QgsRuleBasedRenderer::Rule *rule /Transfer/ );
%Docstring
add child rule, take ownership, sets this as parent
%End

        void insertChild( int i, QgsRuleBasedRenderer::Rule *rule /Transfer/ );
%Docstring
add child rule, take ownership, sets this as parent
%End

        void removeChild( QgsRuleBasedRenderer::Rule *rule );
%Docstring
delete child rule
%End

        void removeChildAt( int i );
%Docstring
delete child rule
%End

        QgsRuleBasedRenderer::Rule *takeChild( QgsRuleBasedRenderer::Rule *rule ) /TransferBack/;
%Docstring
take child rule out, set parent as null
%End

        QgsRuleBasedRenderer::Rule *takeChildAt( int i ) /TransferBack/;
%Docstring
take child rule out, set parent as null
%End

        QgsRuleBasedRenderer::Rule *findRuleByKey( const QString &key );
%Docstring
Try to find a rule given its unique key

.. versionadded:: 2.6
%End

        void setIsElse( bool iselse );
%Docstring
Sets if this rule is an ELSE rule

:param iselse: If true, this rule is an ELSE rule
%End

        bool isElse();
%Docstring
Check if this rule is an ELSE rule

:return: True if this rule is an else rule
%End

      protected:
        void initFilter();




      private:
        Rule( const QgsRuleBasedRenderer::Rule &rh );
    };


    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Creates a new rule-based renderer instance from XML
%End

    QgsRuleBasedRenderer( QgsRuleBasedRenderer::Rule *root /Transfer/ );
%Docstring
Constructs the renderer from given tree of rules (takes ownership)
%End
    QgsRuleBasedRenderer( QgsSymbol *defaultSymbol /Transfer/ );
%Docstring
Constructor for convenience. Creates a root rule and adds a default rule with symbol (takes ownership)
%End

    ~QgsRuleBasedRenderer();

    virtual QgsSymbol *symbolForFeature( QgsFeature &feature, QgsRenderContext &context );

%Docstring
return symbol for current feature. Should not be used individually: there could be more symbols for a feature
%End

    virtual bool renderFeature( QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false );


    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );


    virtual void stopRender( QgsRenderContext &context );


    virtual QString filter( const QgsFields &fields = QgsFields() );


    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;


    virtual bool filterNeedsGeometry() const;


    virtual QgsRuleBasedRenderer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const;


    static QgsFeatureRenderer *createFromSld( QDomElement &element, QgsWkbTypes::GeometryType geomType ) /Factory/;

    virtual QgsSymbolList symbols( QgsRenderContext &context );


    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );

    virtual bool legendSymbolItemsCheckable() const;

    virtual bool legendSymbolItemChecked( const QString &key );

    virtual void checkLegendSymbolItem( const QString &key, bool state = true );


    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol /Transfer/ );

    virtual QgsLegendSymbolList legendSymbolItems() const;

    virtual QString dump() const;

    virtual bool willRenderFeature( QgsFeature &feat, QgsRenderContext &context );

    virtual QgsSymbolList symbolsForFeature( QgsFeature &feat, QgsRenderContext &context );

    virtual QgsSymbolList originalSymbolsForFeature( QgsFeature &feat, QgsRenderContext &context );

    virtual QSet<QString> legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context );

    virtual QgsFeatureRenderer::Capabilities capabilities();


    QgsRuleBasedRenderer::Rule *rootRule();


    static void refineRuleCategories( QgsRuleBasedRenderer::Rule *initialRule, QgsCategorizedSymbolRenderer *r );
%Docstring
take a rule and create a list of new rules based on the categories from categorized symbol renderer
%End
    static void refineRuleRanges( QgsRuleBasedRenderer::Rule *initialRule, QgsGraduatedSymbolRenderer *r );
%Docstring
take a rule and create a list of new rules based on the ranges from graduated symbol renderer
%End
    static void refineRuleScales( QgsRuleBasedRenderer::Rule *initialRule, QList<int> scales );
%Docstring
take a rule and create a list of new rules with intervals of scales given by the passed scale denominators
%End

    static QgsRuleBasedRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
creates a QgsRuleBasedRenderer from an existing renderer.

:return: a new renderer if the conversion was possible, otherwise 0.

.. versionadded:: 2.5
%End

    static void convertToDataDefinedSymbology( QgsSymbol *symbol, const QString &sizeScaleField, const QString &rotationField = QString() );
%Docstring
helper function to convert the size scale and rotation fields present in some other renderers to data defined symbology
%End

  protected:



  private:
    QgsRuleBasedRenderer( const QgsRuleBasedRenderer & );
    QgsRuleBasedRenderer &operator=( const QgsRuleBasedRenderer & );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsrulebasedrenderer.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
