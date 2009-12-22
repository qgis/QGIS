
class QgsSearchTreeNode
{
%TypeHeaderCode
#include "qgssearchtreenode.h"
%End

  public:
    //! defines possible types of node
    enum Type
    {
      tOperator = 1,
      tNumber,
      tColumnRef,
      tString
    };

    //! possible operators
    enum Operator
    {
      // binary
      opAND = 1,
      opOR,
      opNOT,

      // arithmetic
      opPLUS,
      opMINUS,
      opMUL,
      opDIV,
      opPOW,
      opSQRT,
      opSIN,
      opCOS,
      opTAN,
      opASIN,
      opACOS,
      opATAN,
      opTOINT,
      opTOREAL,
      opTOSTRING,
      opLENGTH,
      opAREA,

      // comparison
      opEQ,   // =
      opNE,   // != resp. <>
      opGT,   // >
      opLT,   // <
      opGE,   // >=
      opLE,   // <=
      opRegexp, // ~
      opLike  // LIKE
    };

    //! constructors
    QgsSearchTreeNode( double number );
    QgsSearchTreeNode( Operator o, QgsSearchTreeNode* left, QgsSearchTreeNode* right );
    QgsSearchTreeNode( QString text, bool isColumnRef );

    //! copy contructor - copies whole tree!
    QgsSearchTreeNode( const QgsSearchTreeNode& node );

    //! destructor - deletes children nodes (if any)
    ~QgsSearchTreeNode();

    //! returns type of current node
    Type type();

    //! node value getters
    // TODO: for some reason this function is not found by dynamic linker
    //Operator op();
    double number();
    QString columnRef();
    QString string();

    //! node value setters (type is set also)
    void setOp( Operator o );
    void setNumber( double number );
    void setColumnRef( QString& str );
    void setString( QString& str );

    //! children
    QgsSearchTreeNode* Left();
    QgsSearchTreeNode* Right();
    void setLeft( QgsSearchTreeNode* left );
    void setRight( QgsSearchTreeNode* right );

    //! returns search string that should be equal to original parsed string
    QString makeSearchString();

    //! checks whether the node tree is valid against supplied attributes
    bool checkAgainst( const QMap<int,QgsField>& fields, const QMap<int, QVariant>& attributes );

    //! checks if there were errors during evaluation
    bool hasError();

    //! returns error message
    const QString& errorMsg();

    //! wrapper around valueAgainst()
    bool getValue( QgsSearchTreeValue& value /Out/, QgsSearchTreeNode* node,
                   const QMap<int,QgsField>& fields, const QMap<int,QVariant>& attributes, QgsGeometry* geom = 0 );

  protected:


    //! returns scalar value of node
    QgsSearchTreeValue valueAgainst( const QMap<int,QgsField>& fields, const QMap<int,QVariant>& attributes, QgsGeometry* geom = 0 );

    //! strips mText when node is of string type
    void stripText();

};


class QgsSearchTreeValue
{
%TypeHeaderCode
#include "qgssearchtreenode.h"
%End

  public:

    enum Type
    {
      valError,
      valString,
      valNumber
    };

    QgsSearchTreeValue();
    QgsSearchTreeValue( QString string );
    QgsSearchTreeValue( double number );
    QgsSearchTreeValue( int error, QString errorMsg );

    static int compare( QgsSearchTreeValue& value1, QgsSearchTreeValue& value2,
                        Qt::CaseSensitivity = Qt::CaseSensitive );

    bool isNumeric();
    bool isError();

    QString& string();
    double number();

};
