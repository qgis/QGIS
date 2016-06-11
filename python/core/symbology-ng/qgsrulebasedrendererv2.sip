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

      RenderLevel( const QgsRuleBasedRendererV2::RenderLevel& other );

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
        //! The result of rendering a rule
        enum RenderResult
        {
          Filtered = 0, //!< The rule does not apply
          Inactive,     //!< The rule is inactive
          Rendered      //!< Something was rendered
        };

        //! Constructor takes ownership of the symbol
        Rule( QgsSymbolV2* symbol /Transfer/, int scaleMinDenom = 0, int scaleMaxDenom = 0, const QString& filterExp = QString(),
              const QString& label = QString(), const QString& description = QString(), bool elseRule = false );
        ~Rule();

        /**
         * Dump for debug purpose
         * @param indent How many characters to indent. Will increase by two with every of the recursive calls
         * @return A string representing this rule
         */
        QString dump( int indent  = 0 ) const;

        /**
         * Return the attributes used to evaluate the expression of this rule
         * @return A set of attribute names
         */
        QSet<QString> usedAttributes() const;

        /**
         * Returns true if this rule or one of its chilren needs the geometry to be applied.
         */
        bool needsGeometry() const;

	//! @note available in python bindings as symbol2
        QgsSymbolV2List symbols( const QgsRenderContext& context = QgsRenderContext() ) /PyName=symbols2/;

        //! @note not available in python bindings
        // QgsLegendSymbolList legendSymbolItems( double scaleDenominator = -1, const QString& rule = "" ) const;

        //! @note added in 2.6
        QgsLegendSymbolListV2 legendSymbolItemsV2( int currentLevel = -1 ) const;

        /**
         * Check if a given feature shall be rendered by this rule
         *
         * @param f         The feature to test
         * @param context   The context in which the rendering happens
         * @return          True if the feature shall be rendered
         */
        bool isFilterOK( QgsFeature& f, QgsRenderContext *context = 0 ) const;

        /**
         * Check if this rule applies for a given scale
         * @param scale The scale to check. If set to 0, it will always return true.
         *
         * @return If the rule will be evaluated at this scale
         */
        bool isScaleOK( double scale ) const;

        QgsSymbolV2* symbol();
        QString label() const;
        bool dependsOnScale() const;
        int scaleMinDenom() const;
        int scaleMaxDenom() const;

        /**
         * A filter that will check if this rule applies
         * @return An expression
         */
        QgsExpression* filter() const;

        /**
         * A filter that will check if this rule applies
         * @return An expression
         */
        QString filterExpression() const;

        /**
         * A human readable description for this rule
         *
         * @return Description
         */
        QString description() const;

        //! @note added in 2.6
        //! @deprecated use active instead
        bool checkState() const /Deprecated/;

        /**
         * Returns if this rule is active
         *
         * @return True if the rule is active
         */
        bool active() const;

        //! Unique rule identifier (for identification of rule within renderer)
        //! @note added in 2.6
        QString ruleKey() const;
        //! Override the assigned rule key (should be used just internally by rule-based renderer)
        //! @note added in 2.6
        void setRuleKey( const QString& key );

        //! set a new symbol (or NULL). Deletes old symbol.
        void setSymbol( QgsSymbolV2* sym /Transfer/ );
        void setLabel( const QString& label );

        /**
         * Set the minimum denominator for which this rule shall apply.
         * E.g. 1000 if it shall be evaluated between 1:1000 and 1:100'000
         * Set to 0 to disable the minimum check
         * @param scaleMinDenom The minimum scale denominator for this rule
         */
        void setScaleMinDenom( int scaleMinDenom );

        /**
         * Set the maximum denominator for which this rule shall apply.
         * E.g. 100'000 if it shall be evaluated between 1:1000 and 1:100'000
         * Set to 0 to disable the maximum check
         * @param scaleMaxDenom maximum scale denominator for this rule
         */
        void setScaleMaxDenom( int scaleMaxDenom );

        /**
         * Set the expression used to check if a given feature shall be rendered with this rule
         *
         * @param filterExp An expression
         */
        void setFilterExpression( const QString& filterExp );

        /**
         * Set a human readable description for this rule
         *
         * @param description Description
         */
        void setDescription( const QString& description );

        //! @note added in 2.6
        //! @deprecated use setActive instead
        void setCheckState( bool state ) /Deprecated/;

        /**
         * Sets if this rule is active
         * @param state Determines if the rule should be activated or deactivated
         */
        void setActive( bool state );

        //! clone this rule, return new instance
        QgsRuleBasedRendererV2::Rule* clone() const /Factory/;

        void toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props );
        static QgsRuleBasedRendererV2::Rule* createFromSld( QDomElement& element, QGis::GeometryType geomType ) /Factory/;

        QDomElement save( QDomDocument& doc, QgsSymbolV2Map& symbolMap );

        /** Prepare the rule for rendering and its children (build active children array)
         * @deprecated use startRender( QgsRenderContext& context, const QgsFields& fields, QString& filter ) instead
         */
        bool startRender( QgsRenderContext& context, const QgsFields& fields ) /Deprecated/;

        //! prepare the rule for rendering and its children (build active children array)
        bool startRender( QgsRenderContext& context, const QgsFields& fields, QString& filter );

        //! get all used z-levels from this rule and children
        QSet<int> collectZLevels();

        //! assign normalized z-levels [0..N-1] for this rule's symbol for quick access during rendering
        //! @note not available in python bindings
        // void setNormZLevels( const QMap<int, int>& zLevelsToNormLevels );

        /**
         * Render a given feature, will recursively call subclasses and only render if the constraints apply.
         *
         * @param featToRender The feature to render
         * @param context      The rendering context
         * @param renderQueue  The rendering queue to which the feature should be added
         * @return             The result of the rendering. In explicit if the feature is added to the queue or
         *                     the reason for not rendering the feature.
         */
        QgsRuleBasedRendererV2::Rule::RenderResult renderFeature( QgsRuleBasedRendererV2::FeatureToRender& featToRender, QgsRenderContext& context, QgsRuleBasedRendererV2::RenderQueue& renderQueue );

        //! only tell whether a feature will be rendered without actually rendering it
        bool willRenderFeature( QgsFeature& feat, QgsRenderContext* context = 0);

        //! tell which symbols will be used to render the feature
        QgsSymbolV2List symbolsForFeature( QgsFeature& feat, QgsRenderContext* context = 0 );

        /** Returns which legend keys match the feature
         * @note added in QGIS 2.14
         */
        QSet< QString > legendKeysForFeature( QgsFeature& feat, QgsRenderContext* context = nullptr );

        //! tell which rules will be used to render the feature
        QList<QgsRuleBasedRendererV2::Rule*> rulesForFeature( QgsFeature& feat, QgsRenderContext* context = 0 );

        /**
         * Stop a rendering process. Used to clean up the internal state of this rule
         *
         * @param context The rendering context
         */
        void stopRender( QgsRenderContext& context );

        /**
         * Create a rule from an XML definition
         *
         * @param ruleElem  The XML rule element
         * @param symbolMap Symbol map
         *
         * @return A new rule
         */
        static QgsRuleBasedRendererV2::Rule* create( QDomElement& ruleElem, QgsSymbolV2Map& symbolMap ) /Factory/;

        /**
         * Return all children rules of this rule
         *
         * @return A list of rules
         */
        QList<QgsRuleBasedRendererV2::Rule*>& children();

        /**
         * Returns all children, grand-children, grand-grand-children, grand-gra... you get it
         *
         * @return A list of descendant rules
         */
        QList<QgsRuleBasedRendererV2::Rule*> descendants() const;

        /**
         * The parent rule
         *
         * @return Parent rule
         */
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
        QgsRuleBasedRendererV2::Rule* takeChild( QgsRuleBasedRendererV2::Rule* rule ) /TransferBack/;

        //! take child rule out, set parent as null
        QgsRuleBasedRendererV2::Rule* takeChildAt( int i ) /TransferBack/;

        //! Try to find a rule given its unique key
        //! @note added in 2.6
        QgsRuleBasedRendererV2::Rule* findRuleByKey( const QString& key );

        /**
         * Check which child rules are else rules and update the internal list of else rules
         *
         * TODO QGIS 3: Does this need to be public?
         */
        void updateElseRules();

        /**
         * Sets if this rule is an ELSE rule
         *
         * @param iselse If true, this rule is an ELSE rule
         */
        void setIsElse( bool iselse );

        /**
         * Check if this rule is an ELSE rule
         *
         * @return True if this rule is an else rule
         */
        bool isElse();

      protected:
        void initFilter();

      private:

        Rule( const QgsRuleBasedRendererV2::Rule& rh );

    };

    /////

    static QgsFeatureRendererV2* create( QDomElement& element ) /Factory/;

    //! Constructs the renderer from given tree of rules (takes ownership)
    QgsRuleBasedRendererV2( QgsRuleBasedRendererV2::Rule* root /Transfer/ );
    //! Constructor for convenience. Creates a root rule and adds a default rule with symbol (takes ownership)
    QgsRuleBasedRendererV2( QgsSymbolV2* defaultSymbol /Transfer/ );

    ~QgsRuleBasedRendererV2();

    //! return symbol for current feature. Should not be used individually: there could be more symbols for a feature
    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext &context );

    virtual bool renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );

    virtual void stopRender( QgsRenderContext& context );

    virtual QString filter( const QgsFields& fields = QgsFields() );

    virtual QList<QString> usedAttributes();

    virtual bool filterNeedsGeometry() const;

    virtual QgsRuleBasedRendererV2* clone() const /Factory/;

    virtual void toSld( QDomDocument& doc, QDomElement &element ) const;

    static QgsFeatureRendererV2* createFromSld( QDomElement& element, QGis::GeometryType geomType ) /Factory/;

    virtual QgsSymbolV2List symbols( QgsRenderContext &context );

    //! store renderer info to XML element
    virtual QDomElement save( QDomDocument& doc );

    //! return a list of symbology items for the legend
    virtual QgsLegendSymbologyList legendSymbologyItems( QSize iconSize );

    //! items of symbology items in legend should be checkable
    //! @note added in 2.5
    virtual bool legendSymbolItemsCheckable() const;

    //! items of symbology items in legend is checked
    //! @note added in 2.5
    virtual bool legendSymbolItemChecked( const QString& key );

    //! item in symbology was checked
    //! @note added in 2.5
    virtual void checkLegendSymbolItem( const QString& key, bool state = true );

    virtual void setLegendSymbolItem( const QString& key, QgsSymbolV2* symbol /Transfer/ );

    //! return a list of item text / symbol
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
    virtual bool willRenderFeature( QgsFeature& feat, QgsRenderContext &context );

    //! return list of symbols used for rendering the feature.
    //! For renderers that do not support MoreSymbolsPerFeature it is more efficient
    //! to use symbolForFeature()
    virtual QgsSymbolV2List symbolsForFeature( QgsFeature& feat, QgsRenderContext &context );

    virtual QgsSymbolV2List originalSymbolsForFeature( QgsFeature& feat, QgsRenderContext &context );

    virtual QSet<QString> legendKeysForFeature( QgsFeature& feature, QgsRenderContext& context );

    //! returns bitwise OR-ed capabilities of the renderer
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

    //! creates a QgsRuleBasedRendererV2 from an existing renderer.
    //! @note added in 2.5
    //! @returns a new renderer if the conversion was possible, otherwise 0.
    static QgsRuleBasedRendererV2* convertFromRenderer( const QgsFeatureRendererV2 *renderer ) /Factory/;

    //! helper function to convert the size scale and rotation fields present in some other renderers to data defined symbology
    static void convertToDataDefinedSymbology( QgsSymbolV2* symbol, const QString& sizeScaleField, const QString& rotationField = QString() );

  private:
    QgsRuleBasedRendererV2( const QgsRuleBasedRendererV2 & );
    QgsRuleBasedRendererV2 & operator=( const QgsRuleBasedRendererV2 & );
};
