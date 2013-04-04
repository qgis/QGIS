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

    //! Get list of columns referenced by the expression
    QStringList referencedColumns();
    //! Returns true if the expression uses feature geometry for some computation
    bool needsGeometry();

    // evaluation

    //! Evaluate the feature and return the result
    //! @note prepare() should be called before calling this method
    QVariant evaluate( QgsFeature* f = NULL );

    //! Evaluate the feature and return the result
    //! @note this method does not expect that prepare() has been called on this instance
    QVariant evaluate( QgsFeature* f, const QgsFields& fields );

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

    void setScale( double scale );

    int scale();

    //! Return the parsed expression as a string - useful for debugging
    QString dump() const;

    //! Return calculator used for distance and area calculations
    //! (used by internal functions)
    QgsDistanceArea* geomCalculator();

  //! Sets the geometry calculator used in evaluation of expressions,
  // instead of the default.
  void setGeomCalculator( QgsDistanceArea& calc );

    /** This function currently replaces each expression between [% and %]
       in the string with the result of its evaluation on the feature
       passed as argument.

       Additional substitutions can be passed through the substitutionMap
       parameter
    */
    static QString replaceExpressionText( QString action, QgsFeature &feat,
                                          QgsVectorLayer* layer,
                                          const QMap<QString, QVariant> *substitutionMap = 0 );
    //

    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };
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
      boMod,
      boPow,

      // strings
      boConcat,
    };

    // static const char* BinaryOperatorText[];
    // static const char* UnaryOperatorText[];

    class Function
    {
      public:
        Function( QString fnname, int params, QString group, QString helpText = QString(), bool usesGeometry = false );
        /** The name of the function. */
        QString name();
        /** The number of parameters this function takes. */
        int params();
        /** Does this function use a geometry object. */
        bool usesgeometry();
        /** The group the function belongs to. */
        QString group();
        /** The help text for the function. */
        QString helptext();

        virtual QVariant func(const QVariantList& values, QgsFeature* f, QgsExpression* parent) = 0;
    };

    static const QList<QgsExpression::Function *> &Functions();

    static bool registerFunction(Function* function);
    static bool unregisterFunction(QString name);

    // tells whether the identifier is a name of existing function
    static bool isFunctionName( QString name );

    // return index of the function in Functions array
    static int functionIndex( QString name );

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
      public:
        virtual ~Node();
        virtual QgsExpression::NodeType nodeType() const = 0;
        // abstract virtual eval function
        // errors are reported to the parent
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f ) = 0;

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
        void append( QgsExpression::Node* node );
        int count();
        QList<QgsExpression::Node*> list();

        virtual QString dump() const;
    };

    class Interval
    {
    public:
      Interval(int seconds);
      ~Interval();
      int years();
      int weeks();
      int days();
      int hours();
      int minutes();
      int seconds();
      bool isValid();
      void setValid(bool valid);
      bool operator==(const QgsExpression::Interval& other) const;
      static QgsExpression::Interval invalidInterVal();
      static QgsExpression::Interval fromString(QString string);
    };


    class NodeUnaryOperator : QgsExpression::Node
    {
      public:
        NodeUnaryOperator( QgsExpression::UnaryOperator op, QgsExpression::Node* operand );
        ~NodeUnaryOperator();

        QgsExpression::UnaryOperator op() const;
        QgsExpression::Node* operand() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeBinaryOperator : QgsExpression::Node
    {
      public:
        NodeBinaryOperator( QgsExpression::BinaryOperator op, QgsExpression::Node* opLeft, QgsExpression::Node* opRight );
        ~NodeBinaryOperator();

        QgsExpression::BinaryOperator op() const;
        QgsExpression::Node* opLeft() const;
        QgsExpression::Node* opRight() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeInOperator : QgsExpression::Node
    {
      public:
        NodeInOperator( QgsExpression::Node* node, QgsExpression::NodeList* list, bool notin = false );
        ~NodeInOperator();

        QgsExpression::Node* node() const;
        bool isNotIn() const;
        QgsExpression::NodeList* list() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeFunction : QgsExpression::Node
    {
      public:
        NodeFunction( int fnIndex, QgsExpression::NodeList* args );
        //NodeFunction( QString name, QgsExpression::NodeList* args );
        ~NodeFunction();

        int fnIndex() const;
        QgsExpression::NodeList* args() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeLiteral : QgsExpression::Node
    {
      public:
        NodeLiteral( QVariant value );

        QVariant value() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class NodeColumnRef : QgsExpression::Node
    {
      public:
        NodeColumnRef( QString name );

        QString name() const;

        virtual QgsExpression::NodeType nodeType() const;
        virtual bool prepare( QgsExpression* parent, const QgsFields &fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v ) const;
    };

    class WhenThen
    {
      public:
        WhenThen( QgsExpression::Node* whenExp, QgsExpression::Node* thenExp );
        ~WhenThen();

        //protected:
        QgsExpression::Node* mWhenExp;
        QgsExpression::Node* mThenExp;
    };
    typedef QList<QgsExpression::WhenThen*> WhenThenList;

    class NodeCondition : QgsExpression::Node
    {
      public:
        NodeCondition( QgsExpression::WhenThenList* conditions, QgsExpression::Node* elseExp = NULL );
        ~NodeCondition();

        virtual QgsExpression::NodeType nodeType() const;
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
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

  protected:
    void initGeomCalculator();
};
