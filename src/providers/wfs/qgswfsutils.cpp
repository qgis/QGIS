#include "qgswfsutils.h"

#include "qgsexpression.h"


class QgsExpressionOGCVisitor : public QgsExpression::Visitor
{
  public:
    QgsExpressionOGCVisitor( QDomDocument& doc, QDomElement& parent )
        : mDoc( doc ), mParent( parent ), mResult( false )
    {}

    bool result() { return mResult; }

    void visit( QgsExpression::NodeUnaryOperator* n )
    {
      mResult = false;

      if ( n->op() == QgsExpression::uoNot && n->operand() )
      {
        QDomElement notElemParent = mParent;
        QDomElement notElem = mDoc.createElement( "Not" );

        mParent = notElem;
        n->operand()->accept( *this );
        if ( !mResult )
          return; // visit failed

        mParent = notElemParent;
        mParent.appendChild( notElem );
        mResult = true;
      }
    }

    void visit( QgsExpression::NodeBinaryOperator* n )
    {
      QString opName;
      switch ( n->op() )
      {
        case QgsExpression::boEQ:  opName = "PropertyIsEqualTo"; break;
        case QgsExpression::boNE:  opName = "PropertyIsNotEqualTo"; break;
        case QgsExpression::boLE:  opName = "PropertyIsLessThanOrEqualTo"; break;
        case QgsExpression::boGE:  opName = "PropertyIsLessThanOrEqualTo"; break;
        case QgsExpression::boLT:  opName = "PropertyIsLessThan"; break;
        case QgsExpression::boGT:  opName = "PropertyIsGreaterThan"; break;
        case QgsExpression::boOr:  opName = "Or"; break;
        case QgsExpression::boAnd: opName = "And"; break;
        default: break;
      }

      mResult = false;
      if ( opName.isEmpty() || !n->opLeft() || !n->opRight() )
        return; // unknown operation -> fail

      QDomElement opElem = mDoc.createElement( opName );
      QDomElement opElemParent = mParent;

      mParent = opElem;
      n->opLeft()->accept( *this );
      if ( !mResult )
        return; // visit failed

      mParent = opElem;
      n->opRight()->accept( *this );
      if ( !mResult )
        return; // visit failed

      mParent = opElemParent;
      mParent.appendChild( opElem );
      mResult = true;
    }

    void visit( QgsExpression::NodeInOperator* ) { mResult = false; }
    void visit( QgsExpression::NodeFunction* ) { mResult = false; }

    void visit( QgsExpression::NodeLiteral* n )
    {
      QDomElement literalElem = mDoc.createElement( "Literal" );
      QDomText literalText = mDoc.createTextNode( n->value().toString() );
      literalElem.appendChild( literalText );
      mParent.appendChild( literalElem );
      mResult = true;
    }

    void visit( QgsExpression::NodeColumnRef* n )
    {
      QDomElement propertyElem = mDoc.createElement( "PropertyName" );
      QDomText propertyText = mDoc.createTextNode( n->name() );
      propertyElem.appendChild( propertyText );
      mParent.appendChild( propertyElem );
      mResult = true;
    }

  protected:
    QDomDocument mDoc;
    QDomElement mParent;
    bool mResult;

};


bool QgsWFSUtils::expressionToOGCFilter( QgsExpression& exp, QDomDocument& doc )
{
  doc.clear();
  QDomElement filterElem = doc.createElement( "Filter" );
  doc.appendChild( filterElem );

  QgsExpressionOGCVisitor v( doc, filterElem );
  exp.acceptVisitor( v );
  return v.result();
}
