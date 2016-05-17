class QgsExpression
{
%TypeHeaderCode
#include "qgsexpression.h"
%End

  public:
    /**
     * Creates a new expression based on the provided string.
     * The string will immediately be parsed. For optimization
     * {@link prepare()} should alwys be called before every
     * loop in which this expression is used.
     */
    QgsExpression( const QString& expr );
    ~QgsExpression();

    //! Returns true if an error occurred when parsing the input expression
    bool hasParserError() const;
    //! Returns parser error
    QString parserErrorString() const;

    //! Returns root node of the expression. Root node is null is parsing has failed
    const QgsExpression::Node* rootNode() const;

    //! Get the expression ready for evaluation - find out column indexes.
    //! @deprecated use prepare( const QgsExpressionContext *context ) instead
    bool prepare( const QgsFields &fields ) /Deprecated/;

    /** Get the expression ready for evaluation - find out column indexes.
     * @param context context for preparing expression
     * @note added in QGIS 2.12
     */
    bool prepare( const QgsExpressionContext *context );

    /**
     * Get list of columns referenced by the expression.
     * @note if the returned list contains the QgsFeatureRequest::AllAttributes constant then
     * all attributes from the layer are required for evaluation of the expression.
     * QgsFeatureRequest::setSubsetOfAttributes automatically handles this case.
     *
     * TODO QGIS3: Return QSet<QString>
     */
    QStringList referencedColumns() const;

    //! Returns true if the expression uses feature geometry for some computation
    bool needsGeometry() const;

    // evaluation

    //! Evaluate the feature and return the result
    //! @note prepare() should be called before calling this method
    //! @deprecated use evaluate( const QgsExpressionContext* context ) instead
    QVariant evaluate( const QgsFeature* f ) /Deprecated/;

    //! Evaluate the feature and return the result
    //! @note prepare() should be called before calling this method
    //! @note available in python bindings as evaluatePrepared
    //! @deprecated use evaluate( const QgsExpressionContext* context ) instead
    QVariant evaluate( const QgsFeature& f ) /PyName=evaluatePrepared,Deprecated/;

    //! Evaluate the feature and return the result
    //! @note this method does not expect that prepare() has been called on this instance
    //! @deprecated use evaluate( const QgsExpressionContext* context ) instead
    QVariant evaluate( const QgsFeature* f, const QgsFields& fields ) /Deprecated/;

    //! Evaluate the feature and return the result
    //! @note this method does not expect that prepare() has been called on this instance
    //! @note not available in python bindings
    //! @deprecated use evaluate( const QgsExpressionContext* context ) instead
    // inline QVariant evaluate( const QgsFeature& f, const QgsFields& fields ) { return evaluate( &f, fields ); }

    /** Evaluate the feature and return the result.
     * @note this method does not expect that prepare() has been called on this instance
     * @note added in QGIS 2.12
     */
    QVariant evaluate();

    /** Evaluate the expression against the specified context and return the result.
     * @param context context for evaluating expression
     * @note prepare() should be called before calling this method.
     * @note added in QGIS 2.12
     */
    QVariant evaluate( const QgsExpressionContext* context );

    //! Returns true if an error occurred when evaluating last input
    bool hasEvalError() const;
    //! Returns evaluation error
    QString evalErrorString() const;
    //! Set evaluation error (used internally by evaluation functions)
    void setEvalErrorString( const QString& str );

    //! Set the number for $rownum special column
    //! @deprecated use QgsExpressionContext to set row number instead
    void setCurrentRowNumber( int rowNumber ) /Deprecated/;
    //! Return the number used for $rownum special column
    //! @deprecated use QgsExpressionContext to retrieve row number instead
    int currentRowNumber() /Deprecated/;    //TODO QGIS 3.0: make the following methods private. They are still required for replaceExpressionText
    //but should not be publicly used
    /** Assign a special column
     * @deprecated use global or project QgsExpressionContext variables instead
     */
    static void setSpecialColumn( const QString& name, const QVariant& value ) /Deprecated/;
    /** Unset a special column
     * @deprecated use global or project QgsExpressionContext variables instead
     */
    static void unsetSpecialColumn( const QString& name ) /Deprecated/;
    /** Return the value of the given special column or a null QVariant if undefined
     * @deprecated use global or project QgsExpressionContext variables instead
     */
    static QVariant specialColumn( const QString& name ) /Deprecated/;

    /** Check whether a special column exists
     * @note added in 2.2
     */
    static bool hasSpecialColumn( const QString& name );

    /** Checks whether an expression consists only of a single field reference
     * @note added in 2.9
     */
    bool isField() const;

    //! @deprecated use QgsExpressionContext variant instead
    static bool isValid( const QString& text, const QgsFields& fields, QString &errorMessage ) /Deprecated/;

    /** Tests whether a string is a valid expression.
     * @param text string to test
     * @param context optional expression context
     * @param errorMessage will be filled with any error message from the validation
     * @returns true if string is a valid expression
     * @note added in QGIS 2.12
     */
    static bool isValid( const QString& text, const QgsExpressionContext* context, QString &errorMessage );

    void setScale( double scale );

    double scale();

    //! Return the original, unmodified expression string.
    //! If there was none supplied because it was constructed by sole
    //! API calls, dump() will be used to create one instead.
    QString expression() const;

    //! Return an expression string, constructed from the internal
    //! abstract syntax tree. This does not contain any nice whitespace
    //! formatting or comments. In general it is preferrable to use
    //! expression() instead.
    QString dump() const;

    /** Return calculator used for distance and area calculations
     * (used by $length, $area and $perimeter functions only)
     * @see setGeomCalculator()
     * @see distanceUnits()
     */
    QgsDistanceArea *geomCalculator();

    /** Sets the geometry calculator used for distance and area calculations in expressions.
     * (used by $length, $area and $perimeter functions only). By default, no geometry
     * calculator is set and all distance and area calculations are performed using simple
     * cartesian methods (ie no ellipsoidal calculations).
     * @see geomCalculator()
     */
    //TODO QGIS 3.0 change calc to a pointer, so that calculator can be cleared by passing nullptr
    void setGeomCalculator( const QgsDistanceArea &calc );

    /** Returns the desired distance units for calculations involving geomCalculator(), eg "$length" and "$perimeter".
     * @note distances are only converted when a geomCalculator() has been set
     * @note added in QGIS 2.14
     * @see setDistanceUnits()
     */
    QGis::UnitType distanceUnits() const;

    /** Sets the desired distance units for calculations involving geomCalculator(), eg "$length" and "$perimeter".
     * @note distances are only converted when a geomCalculator() has been set
     * @note added in QGIS 2.14
     * @see distanceUnits()
     */
    void setDistanceUnits( QGis::UnitType unit );

    /** Returns the desired areal units for calculations involving geomCalculator(), eg "$area".
     * @note areas are only converted when a geomCalculator() has been set
     * @note added in QGIS 2.14
     * @see setAreaUnits()
     * @see distanceUnits()
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /** Sets the desired areal units for calculations involving geomCalculator(), eg "$area".
     * @note areas are only converted when a geomCalculator() has been set
     * @note added in QGIS 2.14
     * @see areaUnits()
     * @see setDistanceUnits()
     */
    void setAreaUnits( QgsUnitTypes::AreaUnit unit );

    /** This function currently replaces each expression between [% and %]
     * in the string with the result of its evaluation on the feature
     * passed as argument.
     *
     * Additional substitutions can be passed through the substitutionMap
     * parameter
     * @param action
     * @param feat
     * @param layer
     * @param substitutionMap
     * @param distanceArea optional QgsDistanceArea. If specified, the QgsDistanceArea is used for distance and area conversion
     * @deprecated use QgsExpressionContext variant instead
     */
    static QString replaceExpressionText( const QString &action, const QgsFeature *feat,
                                          QgsVectorLayer *layer,
                                          const QMap<QString, QVariant> *substitutionMap = 0,
                                          const QgsDistanceArea* distanceArea = 0 ) /Deprecated/;

    /** This function replaces each expression between [% and %]
     * in the string with the result of its evaluation with the specified context
     *
     * Additional substitutions can be passed through the substitutionMap parameter
     * @param action
     * @param context expression context
     * @param substitutionMap
     * @param distanceArea optional QgsDistanceArea. If specified, the QgsDistanceArea is used for distance
     * and area conversion
     * @note added in QGIS 2.12
     */
    static QString replaceExpressionText( const QString &action, const QgsExpressionContext* context,
                                          const QMap<QString, QVariant> *substitutionMap = 0,
                                          const QgsDistanceArea* distanceArea = 0 );

    /** Attempts to evaluate a text string as an expression to a resultant double
     * value.
     * @param text text to evaluate as expression
     * @param fallbackValue value to return if text can not be evaluated as a double
     * @returns evaluated double value, or fallback value
     * @note added in QGIS 2.7
     * @note this method is inefficient for bulk evaluation of expressions, it is intended
     * for one-off evaluations only.
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

    //! @note not available in Python bindings
    // static const char* BinaryOperatorText[];

    //! @note not available in Python bindings
    // static const char* UnaryOperatorText[];

    /**
      * Represents a single parameter passed to a function.
      * \note added in QGIS 2.16
      */
    class Parameter
    {
      public:

        /** Constructor for Parameter.
         * @param name parameter name, used when named parameter are specified in an expression
         * @param optional set to true if parameter should be optional
         * @param defaultValue default value to use for optional parameters
         */
        Parameter( const QString& name,
                   bool optional = false,
                   const QVariant& defaultValue = QVariant() );

        //! Returns the name of the parameter.
        QString name() const;

        //! Returns true if the parameter is optional.
        bool optional() const;

        //! Returns the default value for the parameter.
        QVariant defaultValue() const;

        bool operator==( const QgsExpression::Parameter& other ) const;

    };

    //! List of parameters, used for function definition
    typedef QList< QgsExpression::Parameter > ParameterList;

    /**
     * A abstract base class for defining QgsExpression functions.
     */
    class Function
    {
      public:

        //! Constructor for function which uses unnamed parameters
        Function( const QString& fnname,
                  int params,
                  const QString& group,
                  const QString& helpText = QString(),
                  bool usesGeometry = false,
                  const QStringList& referencedColumns = QStringList(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false );

        /** Constructor for function which uses named parameter list.
         * @note added in QGIS 2.16
         */
        Function( const QString& fnname,
                  const QgsExpression::ParameterList& params,
                  const QString& group,
                  const QString& helpText = QString(),
                  bool usesGeometry = false,
                  const QStringList& referencedColumns = QStringList(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false );

        virtual ~Function();

        /** The name of the function. */
        QString name() const;
        /** The number of parameters this function takes. */
        int params() const;

        /** The mininum number of parameters this function takes. */
        int minParams() const;

        /** Returns the list of named parameters for the function, if set.
         * @note added in QGIS 2.16
        */
        const QgsExpression::ParameterList& parameters() const;

        /** Does this function use a geometry object. */
        bool usesgeometry() const;

        /** Returns a list of possible aliases for the function. These include
         * other permissible names for the function, eg deprecated names.
         * @return list of known aliases
         * @note added in QGIS 2.9
         */
        virtual QStringList aliases() const;

        /** True if this function should use lazy evaluation.  Lazy evaluation functions take QgsExpression::Node objects
         * rather than the node results when called.  You can use node->eval(parent, feature) to evaluate the node and return the result
         * Functions are non lazy default and will be given the node return value when called
         */
        bool lazyEval() const;

        virtual QStringList referencedColumns() const;

        /** Returns whether the function is only available if provided by a QgsExpressionContext object.
         * @note added in QGIS 2.12
         */
        bool isContextual() const;

        /** The group the function belongs to. */
        QString group() const;
        /** The help text for the function. */
        const QString helptext() const;

        //! @deprecated Use QgsExpressionContext variant instead
        virtual QVariant func( const QVariantList& values, const QgsFeature* f, QgsExpression* parent ) /Deprecated/;

        /** Returns result of evaluating the function.
         * @param values list of values passed to the function
         * @param context context expression is being evaluated against
         * @param parent parent expression
         * @returns result of function
         * @note named funcV2 in Python bindings. Will be renamed to func to replace deprecated method in QGIS 3.0.
         */
        //TODO QGIS 3.0 - rename python method
        virtual QVariant func( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent ) /PyName=funcV2/;

        virtual bool handlesNull() const;
    };

    static const QList<QgsExpression::Function *>& Functions();
    static const QStringList& BuiltinFunctions();

    /** Registers a function to the expression engine. This is required to allow expressions to utilise the function.
     * @param function function to register
     * @param transferOwnership set to true to transfer ownership of function to expression engine
     * @returns true on successful registration
     * @see unregisterFunction
     */
    static bool registerFunction( Function* function );

    /** Unregisters a function from the expression engine. The function will no longer be usable in expressions.
     * @param name function name
     * @see registerFunction
     */
    static bool unregisterFunction( const QString& name );

    /** Deletes all registered functions whose ownership have been transferred to the expression engine.
     * @note added in QGIS 2.12
     */
    static void cleanRegisteredFunctions();

    //! tells whether the identifier is a name of existing function
    static bool isFunctionName( const QString& name );

    //! return index of the function in Functions array
    static int functionIndex( const QString& name );

    /** Returns the number of functions defined in the parser
     *  @return The number of function defined in the parser.
     */
    static int functionCount();

    /**
     * Returns a list of special Column definitions
     */
    static QList<QgsExpression::Function *> specialColumns();

    /** Returns a quoted column reference (in double quotes)
     * @see quotedString()
     * @see quotedValue()
     */
    static QString quotedColumnRef( QString name );

    /** Returns a quoted version of a string (in single quotes)
     * @see quotedValue()
     * @see quotedColumnRef()
     */
    static QString quotedString( QString text );

    /** Returns a string representation of a literal value, including appropriate
     * quotations where required.
     * @param value value to convert to a string representation
     * @note added in QGIS 2.14
     * @see quotedString()
     * @see quotedColumnRef()
     */
    static QString quotedValue( const QVariant& value );

    /** Returns a string representation of a literal value, including appropriate
     * quotations where required.
     * @param value value to convert to a string representation
     * @param type value type
     * @note added in QGIS 2.14
     * @see quotedString()
     * @see quotedColumnRef()
     */
    static QString quotedValue( const QVariant& value, QVariant::Type type );

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
          case QgsExpression::ntUnaryOperator:   sipType = sipType_QgsExpression_NodeUnaryOperator; break;
          case QgsExpression::ntBinaryOperator:  sipType = sipType_QgsExpression_NodeBinaryOperator; break;
          case QgsExpression::ntInOperator:      sipType = sipType_QgsExpression_NodeInOperator; break;
          case QgsExpression::ntFunction:        sipType = sipType_QgsExpression_NodeFunction; break;
          case QgsExpression::ntLiteral:         sipType = sipType_QgsExpression_NodeLiteral; break;
          case QgsExpression::ntColumnRef:       sipType = sipType_QgsExpression_NodeColumnRef; break;
          case QgsExpression::ntCondition:       sipType = sipType_QgsExpression_NodeCondition; break;
          default:                               sipType = 0; break;
        }
      %End

      public:
        virtual ~Node();

        /**
         * Abstract virtual that returns the type of this node.
         *
         * @return The type of this node
         */
        virtual QgsExpression::NodeType nodeType() const = 0;

        /**
         * Abstract virtual eval method
         * Errors are reported to the parent
         * @deprecated use QgsExpressionContext variant instead
         */
        virtual QVariant eval( QgsExpression* parent, const QgsFeature* f ) /Deprecated/;

        /**
         * Abstract virtual eval method
         * Errors are reported to the parent
         * @note added in QGIS 2.12
         */
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context );

        /**
         * Abstract virtual preparation method
         * Errors are reported to the parent
         * @deprecated use QgsExpressionContext variant instead
         */
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields ) /Deprecated/;

        /**
         * Abstract virtual preparation method
         * Errors are reported to the parent
         * @note added in QGIS 2.12
         */
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context );

        /**
         * Abstract virtual dump method
         *
         * @return An expression which represents this node as string
         */
        virtual QString dump() const = 0;

        /**
         * Generate a clone of this node.
         * Make sure that the clone does not contain any information which is
         * generated in prepare and context related.
         * Ownership is transferred to the caller.
         *
         * @return a deep copy of this node.
         */
        virtual QgsExpression::Node* clone() const = 0;

        /**
         * Abstract virtual method which returns a list of columns required to
         * evaluate this node.
         *
         * When reimplementing this, you need to return any column that is required to
         * evaluate this node and in addition recursively collect all the columns required
         * to evaluate child nodes.
         *
         * @return A list of columns required to evaluate this expression
         */
        virtual QStringList referencedColumns() const = 0;

        /**
         * Abstract virtual method which returns if the geometry is required to evaluate
         * this expression.
         *
         * This needs to call `needsGeometry()` recursively on any child nodes.
         *
         * @return true if a geometry is required to evaluate this expression
         */
        virtual bool needsGeometry() const = 0;

        /**
         * Support the visitor pattern.
         *
         * For any implementation this should look like
         *
         * C++:
         *
         *     v.visit( *this );
         *
         * Python:
         *
         *     v.visit( self)
         *
         * @param v A visitor that visits this node.
         */
        virtual void accept( QgsExpression::Visitor& v ) const = 0;
    };

    //! Named node
    //! @note added in QGIS 2.16
    class NamedNode
    {
      public:

        /** Constructor for NamedNode
         * @param name node name
         * @param node node
         */
        NamedNode( const QString& name, QgsExpression::Node* node );

        //! Node name
        QString name;

        //! Node
        QgsExpression::Node* node;
    };

    class NodeList
    {
      public:
        NodeList();
        ~NodeList();
        /** Takes ownership of the provided node */
        void append( QgsExpression::Node* node /Transfer/ );

        /** Adds a named node. Takes ownership of the provided node.
         * @note added in QGIS 2.16
        */
        void append( QgsExpression::NamedNode* node /Transfer/ );

        /** Returns the number of nodes in the list.
         */
        int count() const;

        //! Returns true if list contains any named nodes
        //! @note added in QGIS 2.16
        bool hasNamedNodes() const;

        const QList<QgsExpression::Node*>& list();

        //! Returns a list of names for nodes. Unnamed nodes will be indicated by an empty string in the list.
        //! @note added in QGIS 2.16
        QStringList names() const;

        /** Creates a deep copy of this list. Ownership is transferred to the caller */
        QgsExpression::NodeList* clone() const;

        virtual QString dump() const;
    };

    //TODO QGIS 3.0 - remove
    //! @deprecated use QgsInterval instead
    class Interval
   {
      public:
        Interval( int seconds = 0 );

        //! interval length in years
        double years() const;
        //! interval length in months
        double months() const;
        //! interval length in weeks
        double weeks() const;
        //! interval length in days
        double days() const;
        //! interval length in hours
        double hours() const;
        //! interval length in minutus
        double minutes() const;
        //! interval length in seconds
        double seconds() const;
        //! getter interval validity
        bool isValid() const;
        //! setter interval validity
        void setValid( bool valid );
        //! compare two intervals
        bool operator==( QgsExpression::Interval other ) const;
        //! convert a string to an interval
        static QgsExpression::Interval fromString( const QString& string );
    };

    class NodeUnaryOperator : QgsExpression::Node
    {
      public:
        NodeUnaryOperator( QgsExpression::UnaryOperator op, QgsExpression::Node* operand /Transfer/ );
        ~NodeUnaryOperator();

        QgsExpression::UnaryOperator op() const;
        QgsExpression::Node* operand() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
        virtual QgsExpression::Node* clone() const;
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
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
        virtual QgsExpression::Node* clone() const;

        int precedence() const;
        bool leftAssociative() const;

      protected:
        bool compare( double diff );
        int computeInt( int x, int y );
        double computeDouble( double x, double y );

        /** Computes the result date time calculation from a start datetime and an interval
         * @param d start datetime
         * @param i interval to add or subtract (depending on mOp)
         */
        QDateTime computeDateTimeFromInterval( const QDateTime& d, QgsInterval *i );
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
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
        virtual QgsExpression::Node* clone() const;
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
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
        virtual QgsExpression::Node* clone() const;

        //! Tests whether the provided argument list is valid for the matching function
        static bool validateParams( int fnIndex, QgsExpression::NodeList* args, QString& error );
    };

    class NodeLiteral : QgsExpression::Node
    {
      public:
        NodeLiteral( const QVariant& value );

        /** The value of the literal. */
        const QVariant& value() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QString dump() const;
        virtual QgsExpression::Node* clone() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeColumnRef : QgsExpression::Node
    {
      public:
        NodeColumnRef( const QString& name );

        /** The name of the column. */
        QString name() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;

        virtual void accept( QgsExpression::Visitor& v ) const;
        virtual QgsExpression::Node* clone() const;
    };

    class WhenThen
    {
      public:
        WhenThen( QgsExpression::Node* whenExp /Transfer/, QgsExpression::Node* thenExp /Transfer/ );
        ~WhenThen();

        //protected:
        QgsExpression::Node* mWhenExp;
        QgsExpression::Node* mThenExp;

      private:
        WhenThen( const QgsExpression::WhenThen& rh );

    };

    class NodeCondition : QgsExpression::Node
    {
      public:
        NodeCondition( QList<QgsExpression::WhenThen*> *conditions, QgsExpression::Node* elseExp = 0 );
        ~NodeCondition();

        virtual QgsExpression::NodeType nodeType() const;
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context );
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
        virtual QgsExpression::Node* clone() const;
    };

    //////

    /** Support for visitor pattern - algorithms dealing with the expressions
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

    /** Entry function for the visitor pattern */
    void acceptVisitor( QgsExpression::Visitor& v ) const;

    /** Returns the help text for a specified function.
     * @param name function name
     * @see variableHelpText()
     */
    static QString helptext( QString name );

    /** Returns the help text for a specified variable.
     * @param variableName name of variable
     * @param showValue set to true to include current value of variable in help text
     * @param value current value of variable to show in help text
     * @see helptext()
     * @note added in QGIS 2.12
     */
    static QString variableHelpText( const QString& variableName, bool showValue = true, const QVariant& value = QVariant() );

    /** Returns the translated name for a function group.
     * @param group untranslated group name
     */
    static QString group( const QString& group );

    /** Formats an expression result for friendly display to the user. Truncates the result to a sensible
     * length, and presents text representations of non numeric/text types (eg geometries and features).
     * @param value expression result to format
     * @returns formatted string, may contain HTML formatting characters
     * @note added in QGIS 2.14
     */
    static QString formatPreviewString( const QVariant& value );

  protected:
    void initGeomCalculator();
};
