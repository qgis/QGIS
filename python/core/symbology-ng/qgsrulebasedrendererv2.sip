/**
When drawing a vector layer with rule-based renderer, it goes through
the rules and draws features with symbols from rules that match.
 */
class QgsRuleBasedRendererV2 : QgsFeatureRendererV2
{
%TypeHeaderCode
#include <qgsrulebasedrendererv2.h>
%End

  public:
    // TODO: use QVarLengthArray instead of QList

    enum FeatureFlags { FeatIsSelected, FeatDrawMarkers, };

    // feature for rendering: QgsFeature and some flags
    struct FeatureToRender
    {
      FeatureToRender( QgsFeature& _f, int _flags );
      QgsFeature feat;
      int flags; // selected and/or draw markers
    };

    // rendering job: a feature to be rendered with a particular symbol
    // (both f, symbol are _not_ owned by this class)
    struct RenderJob
    {
      RenderJob( QgsRuleBasedRendererV2::FeatureToRender& _ftr, QgsSymbolV2* _s );
      QgsRuleBasedRendererV2::FeatureToRender& ftr;
      QgsSymbolV2* symbol;
    };

    // render level: a list of jobs to be drawn at particular level
    // (jobs are owned by this class)
    struct RenderLevel
    {
      RenderLevel( int z );
      ~RenderLevel();
      int zIndex;
      QList<QgsRuleBasedRendererV2::RenderJob*> jobs;
    };

    // rendering queue: a list of rendering levels
    typedef QList<QgsRuleBasedRendererV2::RenderLevel> RenderQueue;

    /**
      This class keeps data about a rules for rule-based renderer.
      A rule consists of a symbol, filter expression and range of scales.
      If filter is empty, it matches all features.
      If scale range has both values zero, it matches all scales.
      If one of the min/max scale denominators is zero, there is no lower/upper bound for scales.
      A rule matches if both filter and scale range match.
     */
    class Rule
    {
      public:
        //! Constructor takes ownership of the symbol
        Rule( QgsSymbolV2* symbol /Transfer/, int scaleMinDenom = 0, int scaleMaxDenom = 0, QString filterExp = QString(),
              QString label = QString(), QString description = QString(), bool elseRule = false );
        ~Rule();
        QString dump( int offset = 0 ) const;
        QSet<QString> usedAttributes();
        QgsSymbolV2List symbols();
        //! @note not available in python bindings
        // QgsLegendSymbolList legendSymbolItems();
        bool isFilterOK( QgsFeature& f ) const;
        bool isScaleOK( double scale ) const;

        QgsSymbolV2* symbol();
        QString label() const;
        bool dependsOnScale() const;
        int scaleMinDenom() const;
        int scaleMaxDenom() const;
        QgsExpression* filter() const;
        QString filterExpression() const;
        QString description() const;

        //! set a new symbol (or NULL). Deletes old symbol.
        void setSymbol( QgsSymbolV2* sym /Transfer/ );
        void setLabel( QString label );
        void setScaleMinDenom( int scaleMinDenom );
        void setScaleMaxDenom( int scaleMaxDenom );
        void setFilterExpression( QString filterExp );
        void setDescription( QString description );

        //! clone this rule, return new instance
        QgsRuleBasedRendererV2::Rule* clone() const /Factory/;

        void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props );
        static QgsRuleBasedRendererV2::Rule* createFromSld( QDomElement& element, QGis::GeometryType geomType ) /Factory/;

        QDomElement save( QDomDocument& doc, QgsSymbolV2Map& symbolMap );

        //! prepare the rule for rendering and its children (build active children array)
        bool startRender( QgsRenderContext& context, const QgsFields& fields );
        //! get all used z-levels from this rule and children
        QSet<int> collectZLevels();
        //! assign normalized z-levels [0..N-1] for this rule's symbol for quick access during rendering
        //! @note not available in python bindings
        // void setNormZLevels( const QMap<int, int>& zLevelsToNormLevels );

        bool renderFeature( QgsRuleBasedRendererV2::FeatureToRender& featToRender, QgsRenderContext& context, QgsRuleBasedRendererV2::RenderQueue& renderQueue );

        //! only tell whether a feature will be rendered without actually rendering it
        //! @note added in 1.9
        bool willRenderFeature( QgsFeature& feat );

        //! tell which symbols will be used to render the feature
        //! @note added in 1.9
        QgsSymbolV2List symbolsForFeature( QgsFeature& feat );

        //! tell which rules will be used to render the feature
        QList<QgsRuleBasedRendererV2::Rule*> rulesForFeature( QgsFeature& feat );

        void stopRender( QgsRenderContext& context );

        static QgsRuleBasedRendererV2::Rule* create( QDomElement& ruleElem, QgsSymbolV2Map& symbolMap ) /Factory/;

        QList<QgsRuleBasedRendererV2::Rule*>& children();
        QList<QgsRuleBasedRendererV2::Rule*> descendants() const;
        QgsRuleBasedRendererV2::Rule* parent();

        //! add child rule, take ownership, sets this as parent
        void appendChild( QgsRuleBasedRendererV2::Rule* rule /Transfer/ );
        //! add child rule, take ownership, sets this as parent
        void insertChild( int i, QgsRuleBasedRendererV2::Rule* rule /Transfer/ );
        //! delete child rule
        void removeChild( QgsRuleBasedRendererV2::Rule* rule );
        //! delete child rule
        void removeChildAt( int i );
        //! take child rule out, set parent as null
        void takeChild( QgsRuleBasedRendererV2::Rule* rule );
        //! take child rule out, set parent as null
        QgsRuleBasedRendererV2::Rule* takeChildAt( int i ) /TransferBack/;

        void updateElseRules();

        void setIsElse( bool iselse );
        bool isElse();

      protected:
        void initFilter();
    };

    /////

    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

    //! Constructs the renderer from given tree of rules (takes ownership)
    QgsRuleBasedRendererV2( QgsRuleBasedRendererV2::Rule* root /Transfer/ );
    //! Constructor for convenience. Creates a root rule and adds a default rule with symbol (takes ownership)
    QgsRuleBasedRendererV2( QgsSymbolV2* defaultSymbol /Transfer/ );

    ~QgsRuleBasedRendererV2();

    //! return symbol for current feature. Should not be used individually: there could be more symbols for a feature
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature );

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QList<QString> usedAttributes();

    virtual QgsFeatureRendererV2* clone() /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    static QgsFeatureRendererV2* createFromSld( QDomElement& element, QGis::GeometryType geomType ) /Factory/;

    virtual QgsSymbolV2List symbols();

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const;

    //! items of symbology items in legend is checked
    //! @note added in 2.5
    virtual bool legendSymbolItemChecked( QString key );

    //! item in symbology was checked
    //! @note added in 2.5
    virtual void checkLegendSymbolItem( QString key, bool state = true );

    //! return a list of item text / symbol
    //! @note: this method was added in version 1.5
    //! @note not available in python bindings
    // virtual QgsLegendSymbolList legendSymbolItems();

    //! Return a list of symbology items for the legend. Better choice than legendSymbolItems().
    //! Default fallback implementation just uses legendSymbolItems() implementation
    //! @note added in 2.6
    virtual QgsLegendSymbolListV2 legendSymbolItemsV2() const;

    //! for debugging
    virtual QString dump() const;

    //! return whether the renderer will render a feature or not.
    //! Must be called between startRender() and stopRender() calls.
    //! @note added in 1.9
    virtual bool willRenderFeature( QgsFeature& feat );

    //! return list of symbols used for rendering the feature.
    //! For renderers that do not support MoreSymbolsPerFeature it is more efficient
    //! to use symbolForFeature()
    //! @note added in 1.9
    virtual QgsSymbolV2List symbolsForFeature( QgsFeature& feat );

    //! returns bitwise OR-ed capabilities of the renderer
    //! \note added in 2.0
    virtual int capabilities();

    /////

    QgsRuleBasedRendererV2::Rule* rootRule();

    //////

    //! take a rule and create a list of new rules based on the categories from categorized symbol renderer
    static void refineRuleCategories( QgsRuleBasedRendererV2::Rule* initialRule, QgsCategorizedSymbolRendererV2* r );
    //! take a rule and create a list of new rules based on the ranges from graduated symbol renderer
    static void refineRuleRanges( QgsRuleBasedRendererV2::Rule* initialRule, QgsGraduatedSymbolRendererV2* r );
    //! take a rule and create a list of new rules with intervals of scales given by the passed scale denominators
    static void refineRuleScales( QgsRuleBasedRendererV2::Rule* initialRule, QList<int> scales );

  private:
    QgsRuleBasedRendererV2( const QgsRuleBasedRendererV2 & );
    QgsRuleBasedRendererV2 & operator=( const QgsRuleBasedRendererV2 & );
};
