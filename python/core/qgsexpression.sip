class QgsExpression
{
%TypeHeaderCode
#include "qgsexpression.h"
%End

  public:
    QgsExpression( const QString& expr );
    ~QgsExpression();

    //! Returns true if an error occurred when parsing the input expression
    bool hasParserError() const;
    //! Returns parser error
    QString parserErrorString() const;

    //! Returns root node of the expression. Root node is null is parsing has failed
    const QgsExpression::Node* rootNode() const;

    //! Get the expression ready for evaluation - find out column indexes.
    bool prepare( const QgsFields &fields );

    /**
     * Get list of columns referenced by the expression.
     * @note if the returned list contains the QgsFeatureRequest::AllAttributes constant then
     * all attributes from the layer are required for evaluation of the expression.
     * QgsFeatureRequest::setSubsetOfAttributes automatically handles this case.
     */
    QStringList referencedColumns() const;

    //! Returns true if the expression uses feature geometry for some computation
    bool needsGeometry() const;

    // evaluation

    //! Evaluate the feature and return the result
    //! @note prepare() should be called before calling this method
    QVariant evaluate( const QgsFeature* f = NULL );

    //! Evaluate the feature and return the result
    //! @note prepare() should be called before calling this method
    //! @note available in python bindings as evaluatePrepared
    QVariant evaluate( const QgsFeature& f ) /PyName=evaluatePrepared/;

    //! Evaluate the feature and return the result
    //! @note this method does not expect that prepare() has been called on this instance
    QVariant evaluate( const QgsFeature* f, const QgsFields& fields );

    //! Evaluate the feature and return the result
    //! @note this method does not expect that prepare() has been called on this instance
    //! @note not available in python bindings
    // inline QVariant evaluate( const QgsFeature& f, const QgsFields& fields ) { return evaluate( &f, fields ); }

    //! Returns true if an error occurred when evaluating last input
    bool hasEvalError() const;
    //! Returns evaluation error
    QString evalErrorString() const;
    //! Set evaluation error (used internally by evaluation functions)
    void setEvalErrorString( QString str );

    //! Set the number for $rownum special column
    void setCurrentRowNumber( int rowNumber );
    //! Return the number used for $rownum special column
    int currentRowNumber();

    //! Assign a special column
    static void setSpecialColumn( const QString& name, QVariant value );
    //! Unset a special column
    static void unsetSpecialColumn( const QString& name );
    //! Return the value of the given special column or a null QVariant if undefined
    static QVariant specialColumn( const QString& name );
    //! Check whether a special column exists
    //! @note added in 2.2
    static bool hasSpecialColumn( const QString& name );

    /** Checks whether an expression consists only of a single field reference
     * @note added in 2.9
     */
    bool isField() const;

    static bool isValid( const QString& text, const QgsFields& fields, QString &errorMessage );

    void setScale( double scale );

    double scale();

    //! Alias for dump()
    const QString expression() const;

    //! Return the expression string that represents this QgsExpression.
    QString dump() const;

    //! Return calculator used for distance and area calculations
    //! (used by internal functions)
    QgsDistanceArea *geomCalculator();

    //! Sets the geometry calculator used in evaluation of expressions,
    // instead of the default.
    void setGeomCalculator( const QgsDistanceArea &calc );

    /** This function currently replaces each expression between [% and %]
       in the string with the result of its evaluation on the feature
       passed as argument.

       Additional substitutions can be passed through the substitutionMap
       parameter
       @param action
       @param feat
       @param layer
       @param substitutionMap
       @param distanceArea optional QgsDistanceArea. If specified, the QgsDistanceArea is used for distance
       and area conversion
    */
    static QString replaceExpressionText( const QString &action, const QgsFeature *feat,
                                          QgsVectorLayer *layer,
                                          const QMap<QString, QVariant> *substitutionMap = 0,
                                          const QgsDistanceArea* distanceArea = 0
                                        );

    /**Attempts to evaluate a text string as an expression to a resultant double
     * value.
     * @param text text to evaluate as expression
     * @param fallbackValue value to return if text can not be evaluated as a double
     * @returns evaluated double value, or fallback value
     * @note added in QGIS 2.7
    */
    static double evaluateToDouble( const QString& text, const double fallbackValue );

    /**
     * @brief list of unary operators
     * @note if any change is made here, the definition of QgsExpression::UnaryOperatorText[] must be adapted.
     */
    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    /**
     * @brief list of binary operators
     * @note if any change is made here, the definition of QgsExpression::BinaryOperatorText[] must be adapted.
     */
    enum BinaryOperator
    {
      // logical
      boOr,
      boAnd,

      // comparison
      boEQ,  // =
      boNE,  // <>
      boLE,  // <=
      boGE,  // >=
      boLT,  // <
      boGT,  // >
      boRegexp,
      boLike,
      boNotLike,
      boILike,
      boNotILike,
      boIs,
      boIsNot,

      // math
      boPlus,
      boMinus,
      boMul,
      boDiv,
      boIntDiv,
      boMod,
      boPow,

      // strings
      boConcat,
    };
    enum SpatialOperator
    {
      soBbox,
      soIntersects,
      soContains,
      soCrosses,
      soEquals,
      soDisjoint,
      soOverlaps,
      soTouches,
      soWithin,
    };

    // static const char* BinaryOperatorText[];
    // static const char* UnaryOperatorText[];

    /**
      * A abstract base class for defining QgsExpression functions.
      */
    class Function
    {
      public:
        Function( const QString& fnname, int params, const QString& group, const QString& helpText = QString(), bool usesGeometry = false, QStringList referencedColumns = QStringList(), bool lazyEval = false );

        virtual ~Function();

        /** The name of the function. */
        QString name();
        /** The number of parameters this function takes. */
        int params();
        /** Does this function use a geometry object. */
        bool usesgeometry();

        /** Returns a list of possible aliases for the function. These include
         * other permissible names for the function, eg deprecated names.
         * @return list of known aliases
         * @note added in QGIS 2.9
         */
        virtual QStringList aliases() const;

        /** True if this function should use lazy evaluation.  Lazy evaluation functions take QgsExpression::Node objects
         * rather than the node results when called.  You can use node->eval(parent, feature) to evaluate the node and return the result
         * Functions are non lazy default and will be given the node return value when called **/
        bool lazyEval();

        virtual QStringList referencedColumns() const;

        /** The group the function belongs to. */
        QString group();
        /** The help text for the function. */
        const QString helptext();

        virtual QVariant func( const QVariantList& values, const QgsFeature* f, QgsExpression* parent ) = 0;
    };


    static const QList<QgsExpression::Function *>& Functions();

    static const QStringList& BuiltinFunctions();

    static bool registerFunction( Function* function );
    static bool unregisterFunction( QString name );

    // tells whether the identifier is a name of existing function
    static bool isFunctionName( QString name );

    // return index of the function in Functions array
    static int functionIndex( const QString& name );

    /**  Returns the number of functions defined in the parser
      *  @return The number of function defined in the parser.
      */
    static int functionCount();

    /**
     * Returns a list of special Column definitions
     */
    static QList<QgsExpression::Function *> specialColumns();

    //! return quoted column reference (in double quotes)
    static QString quotedColumnRef( QString name );
    //! return quoted string (in single quotes)
    static QString quotedString( QString text );

    //////

    enum NodeType
    {
      ntUnaryOperator,
      ntBinaryOperator,
      ntInOperator,
      ntFunction,
      ntLiteral,
      ntColumnRef,
      ntCondition
    };

    class Node
    {
      %ConvertToSubClassCode
        switch (sipCpp->nodeType())
        {
          case QgsExpression::ntUnaryOperator:   sipClass = sipClass_QgsExpression_NodeUnaryOperator; break;
          case QgsExpression::ntBinaryOperator:  sipClass = sipClass_QgsExpression_NodeBinaryOperator; break;
          case QgsExpression::ntInOperator:      sipClass = sipClass_QgsExpression_NodeInOperator; break;
          case QgsExpression::ntFunction:        sipClass = sipClass_QgsExpression_NodeFunction; break;
          case QgsExpression::ntLiteral:         sipClass = sipClass_QgsExpression_NodeLiteral; break;
          case QgsExpression::ntColumnRef:       sipClass = sipClass_QgsExpression_NodeColumnRef; break;
          case QgsExpression::ntCondition:       sipClass = sipClass_QgsExpression_NodeCondition; break;
          default:                               sipClass = 0; break;
        }
      %End

      public:
        virtual ~Node();
        virtual QgsExpression::NodeType nodeType() const = 0;
        // abstract virtual eval function
        // errors are reported to the parent
        virtual QVariant eval( QgsExpression* parent, const QgsFeature* f ) = 0;

        // abstract virtual preparation function
        // errors are reported to the parent
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields ) = 0;

        virtual QString dump() const = 0;

        virtual QStringList referencedColumns() const = 0;
        virtual bool needsGeometry() const = 0;

        // support for visitor pattern
        virtual void accept( QgsExpression::Visitor& v ) const = 0;
    };

    class NodeList
    {
      public:
        NodeList();
        ~NodeList();
        void append( QgsExpression::Node* node /Transfer/ );
        int count();
        const QList<QgsExpression::Node*>& list();

        virtual QString dump() const;

//      protected:
//        QList<QgsExpression::Node*> mList;
    };

    class Interval
    {
      public:
        Interval( int seconds = 0 );
        ~Interval();
        double years();
        double months();
        double weeks();
        double days();
        double hours();
        double minutes();
        double seconds();
        bool isValid();
        void setValid( bool valid );
        bool operator==( const QgsExpression::Interval& other ) const;
        static QgsExpression::Interval invalidInterVal();
        static QgsExpression::Interval fromString( QString string );
    };

    class NodeUnaryOperator : QgsExpression::Node
    {
      public:
        NodeUnaryOperator( QgsExpression::UnaryOperator op, QgsExpression::Node* operand /Transfer/ );
        ~NodeUnaryOperator();

        QgsExpression::UnaryOperator op() const;
        QgsExpression::Node* operand() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, const QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeBinaryOperator : QgsExpression::Node
    {
      public:
        NodeBinaryOperator( QgsExpression::BinaryOperator op, QgsExpression::Node* opLeft /Transfer/, QgsExpression::Node* opRight /Transfer/ );
        ~NodeBinaryOperator();

        QgsExpression::BinaryOperator op() const;
        QgsExpression::Node* opLeft() const;
        QgsExpression::Node* opRight() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, const QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;

        int precedence() const;
    };

    class NodeInOperator : QgsExpression::Node
    {
      public:
        NodeInOperator( QgsExpression::Node* node /Transfer/, QgsExpression::NodeList* list /Transfer/, bool notin = false );
        ~NodeInOperator();

        QgsExpression::Node* node() const;
        bool isNotIn() const;
        QgsExpression::NodeList* list() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, const QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeFunction : QgsExpression::Node
    {
      public:
        NodeFunction( int fnIndex, QgsExpression::NodeList* args /Transfer/ );
        //NodeFunction( QString name, QgsExpression::NodeList* args );
        ~NodeFunction();

        int fnIndex() const;
        QgsExpression::NodeList* args() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, const QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeLiteral : QgsExpression::Node
    {
      public:
        NodeLiteral( const QVariant& value );

        const QVariant& value() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, const QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeColumnRef : QgsExpression::Node
    {
      public:
        NodeColumnRef( const QString& name );

        QString name() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, const QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;

        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class WhenThen
    {
      public:
        WhenThen( QgsExpression::Node* whenExp /Transfer/, QgsExpression::Node* thenExp /Transfer/ );
        ~WhenThen();

        //protected:
        QgsExpression::Node* mWhenExp;
        QgsExpression::Node* mThenExp;
    };

    class NodeCondition : QgsExpression::Node
    {
      public:
        NodeCondition( QList<QgsExpression::WhenThen*> *conditions, QgsExpression::Node* elseExp = 0 );
        ~NodeCondition();

        virtual QgsExpression::NodeType nodeType() const;
        virtual QVariant eval( QgsExpression* parent, const QgsFeature* f );
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    //////

    /** support for visitor pattern - algorithms dealing with the expressions
        may be implemented without modifying the Node classes */
    class Visitor
    {
      public:
        virtual ~Visitor();
        virtual void visit( const QgsExpression::NodeUnaryOperator& n ) = 0;
        virtual void visit( const QgsExpression::NodeBinaryOperator& n ) = 0;
        virtual void visit( const QgsExpression::NodeInOperator& n ) = 0;
        virtual void visit( const QgsExpression::NodeFunction& n ) = 0;
        virtual void visit( const QgsExpression::NodeLiteral& n ) = 0;
        virtual void visit( const QgsExpression::NodeColumnRef& n ) = 0;
        virtual void visit( const QgsExpression::NodeCondition& n ) = 0;
    };

    /** entry function for the visitor pattern */
    void acceptVisitor( QgsExpression::Visitor& v ) const;

    static QString helptext( QString name );
    static QString group( QString group );

  protected:
    void initGeomCalculator();

  private:
    QgsExpression( const QgsExpression& );
    QgsExpression & operator=( const QgsExpression& );
};
