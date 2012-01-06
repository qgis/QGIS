
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

  //! Get the expression ready for evaluation - find out column indexes.
  bool prepare( const QgsFieldMap& fields );

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
  QVariant evaluate( QgsFeature* f, const QgsFieldMap& fields );

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

  //! Return the parsed expression as a string - useful for debugging
  QString dump() const;

  //! Return calculator used for distance and area calculations
  //! (used by internal functions)
  QgsDistanceArea* geomCalculator();

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
      boILike,
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


    struct FunctionDef /NoDefaultCtors/
    {
      //FunctionDef( QString fnname, int params, FcnEval fcn, QString group, QString helpText = QString(), bool usesGeometry = false );
      /** The name of the function. */
      QString mName;
      /** The number of parameters this function takes. */
      int mParams;
      /** Pointer to fucntion. */
      //FcnEval mFcn;
      /** Does this function use a geometry object. */
      bool mUsesGeometry;
      /** The group the function belongs to. */
      QString mGroup;
      /** The help text for the function. */
      QString mHelpText;
    };

    static const QList<QgsExpression::FunctionDef> &BuiltinFunctions();

    // tells whether the identifier is a name of existing function
    static bool isFunctionName( QString name );

    // return index of the function in BuiltinFunctions array
    static int functionIndex( QString name );

    /**  Returns the number of functions defined in the parser
      *  @return The number of function defined in the parser.
      */
    static int functionCount();

    //! return quoted column reference (in double quotes)
    static QString quotedColumnRef( QString name );

    //////

    class Node
    {
      public:
        virtual ~Node();
        // abstract virtual eval function
        // errors are reported to the parent
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f ) = 0;

        // abstract virtual preparation function
        // errors are reported to the parent
        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields ) = 0;

        virtual QString dump() const = 0;

        virtual QStringList referencedColumns() const = 0;
        virtual bool needsGeometry() const = 0;

        // support for visitor pattern
        virtual void accept( QgsExpression::Visitor& v ) = 0;
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

    class NodeUnaryOperator : QgsExpression::Node
    {
      public:
        NodeUnaryOperator( QgsExpression::UnaryOperator op, QgsExpression::Node* operand );
        ~NodeUnaryOperator();

        QgsExpression::UnaryOperator op();
        QgsExpression::Node* operand();

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v );
    };

    class NodeBinaryOperator : QgsExpression::Node
    {
      public:
        NodeBinaryOperator( QgsExpression::BinaryOperator op, QgsExpression::Node* opLeft, QgsExpression::Node* opRight );
        ~NodeBinaryOperator();

        QgsExpression::BinaryOperator op();
        QgsExpression::Node* opLeft();
        QgsExpression::Node* opRight();

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v );
    };

    class NodeInOperator : QgsExpression::Node
    {
      public:
        NodeInOperator( QgsExpression::Node* node, QgsExpression::NodeList* list, bool notin = false );
        ~NodeInOperator();

        QgsExpression::Node* node();
        bool isNotIn();
        QgsExpression::NodeList* list();

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v );
    };

    class NodeFunction : QgsExpression::Node
    {
      public:
        NodeFunction( int fnIndex, QgsExpression::NodeList* args );
        //NodeFunction( QString name, QgsExpression::NodeList* args );
        ~NodeFunction();

        int fnIndex();
        QgsExpression::NodeList* args();

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v );
    };

    class NodeLiteral : QgsExpression::Node
    {
      public:
        NodeLiteral( QVariant value );

        QVariant value();

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v );
    };

    class NodeColumnRef : QgsExpression::Node
    {
      public:
        NodeColumnRef( QString name );

        QString name();

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( QgsExpression::Visitor& v );
    };

    //////

    /** support for visitor pattern - algorithms dealing with the expressions
        may be implemented without modifying the Node classes */
    class Visitor
    {
    public:
      virtual ~Visitor();
      virtual void visit( QgsExpression::NodeUnaryOperator* n ) = 0;
      virtual void visit( QgsExpression::NodeBinaryOperator* n ) = 0;
      virtual void visit( QgsExpression::NodeInOperator* n ) = 0;
      virtual void visit( QgsExpression::NodeFunction* n ) = 0;
      virtual void visit( QgsExpression::NodeLiteral* n ) = 0;
      virtual void visit( QgsExpression::NodeColumnRef* n ) = 0;
    };

    /** entry function for the visitor pattern */
    void acceptVisitor( QgsExpression::Visitor& v );

};
