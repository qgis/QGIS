/***************************************************************************
                          qgssearchtreenode.cpp
                  Implementation for evaluating parsed tree
                          --------------------
    begin                : 2005-07-26
    copyright            : (C) 2005 by Martin Dobias
    email                : won.der at centrum.sk
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsdistancearea.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgssearchtreenode.h"
#include <QRegExp>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <iostream>

#include <cmath>

#define EVAL_STR(x) (x.length() ? x : "(empty)")

QgsSearchTreeNode::QgsSearchTreeNode( QgsSearchTreeNode::Type t )
{
  Q_ASSERT( t == tNodeList );
  mType  = t;
  mLeft  = NULL;
  mRight = NULL;

  init();
}

QgsSearchTreeNode::QgsSearchTreeNode( double number )
{
  mType   = tNumber;
  mNumber = number;
  mLeft   = NULL;
  mRight  = NULL;

  init();
}


QgsSearchTreeNode::QgsSearchTreeNode( Operator op,
                                      QgsSearchTreeNode* left,
                                      QgsSearchTreeNode* right )
{
  mType  = tOperator;
  mOp    = op;
  mLeft  = left;
  mRight = right;

  init();
}


QgsSearchTreeNode::QgsSearchTreeNode( QString text, bool isColumnRef )
{
  mLeft  = NULL;
  mRight = NULL;

  if ( isColumnRef )
  {
    mType = tColumnRef;
    mText = text;
    if ( text.at( 0 ) == '\"' )
    {
      // column reference is quoted
      stripColRef();
    }
  }
  else
  {
    mType = tString;
    mText = text;
    stripText();
  }

  init();
}

QgsSearchTreeNode::QgsSearchTreeNode( const QgsSearchTreeNode& node )
{
  mType = node.mType;
  mOp = node.mOp;
  mNumber = node.mNumber;
  mText = node.mText;

  // recursively copy children
  if ( node.mLeft )
    mLeft =  new QgsSearchTreeNode( *node.mLeft );
  else
    mLeft = NULL;

  if ( node.mRight )
    mRight = new QgsSearchTreeNode( *node.mRight );
  else
    mRight = NULL;

  foreach( QgsSearchTreeNode * lnode, node.mNodeList )
  {
    mNodeList.append( new QgsSearchTreeNode( *lnode ) );
  }

  init();
}


QgsSearchTreeNode::~QgsSearchTreeNode()
{
  // delete children

  if ( mLeft )
    delete mLeft;

  if ( mRight )
    delete mRight;

  while ( !mNodeList.isEmpty() )
    delete mNodeList.takeFirst();

  delete mCalc;
}


void QgsSearchTreeNode::init()
{
  mCalc = NULL;

  if ( mType == tOperator && ( mOp == opLENGTH || mOp == opAREA || mOp == opPERIMETER ) )
  {
    //initialize QgsDistanceArea
    mCalc = new QgsDistanceArea;
    mCalc->setProjectionsEnabled( false );
    QSettings settings;
    QString ellipsoid = settings.value( "/qgis/measure/ellipsoid", "WGS84" ).toString();
    mCalc->setEllipsoid( ellipsoid );
  }
  else if ( mType == tOperator && mOp == opROWNUM )
  {
    // initialize row number to a sane value
    mNumber = 0;
  }
}

void QgsSearchTreeNode::stripText()
{
  // strip single quotes on start,end
  mText = mText.mid( 1, mText.length() - 2 );

  // make single "single quotes" from double "single quotes"
  mText.replace( QRegExp( "''" ), "'" );

  // strip \n \' etc.
  int index = 0;
  while (( index = mText.indexOf( '\\', index ) ) != -1 )
  {
    mText.remove( index, 1 ); // delete backslash
    QChar chr;
    switch ( mText[index].toLatin1() ) // evaluate backslashed character
    {
      case 'n':  chr = '\n'; break;
      case 't':  chr = '\t'; break;
      case '\\': chr = '\\'; break;
      case '\'': chr = '\''; break;
      default: chr = '?'; break;
    }
    mText[index++] = chr; // set new character and push index +1
  }

}

void QgsSearchTreeNode::stripColRef()
{
  // strip double quotes on start,end
  mText = mText.mid( 1, mText.length() - 2 );

  // make single "double quotes" from double "double quotes"
  mText.replace( QRegExp( "\"\"" ), "\"" );
}

QString QgsSearchTreeNode::quotedColumnRef( QString name )
{
  return QString( "\"%1\"" ).arg( name.replace( "\"", "\"\"" ) );
}


QString QgsSearchTreeNode::makeSearchString()
{
  QString str;
  if ( mType == tOperator )
  {
    if ( mOp == opSQRT || mOp == opSIN || mOp == opCOS || mOp == opTAN ||
         mOp == opASIN || mOp == opACOS || mOp == opATAN ||
         mOp == opTOINT || mOp == opTOREAL || mOp == opTOSTRING ||
         mOp == opLOWER || mOp == opUPPER || mOp == opSTRLEN ||
         mOp == opATAN2 || mOp == opREPLACE || mOp == opSUBSTR )
    {
      // functions
      switch ( mOp )
      {
        case opSQRT: str += "sqrt"; break;
        case opSIN: str += "sin"; break;
        case opCOS: str += "cos"; break;
        case opTAN: str += "tan"; break;
        case opASIN: str += "asin"; break;
        case opACOS: str += "acos"; break;
        case opATAN: str += "atan"; break;
        case opTOINT: str += "to int"; break;
        case opTOREAL: str += "to real"; break;
        case opTOSTRING: str += "to string"; break;
        case opLOWER: str += "lower"; break;
        case opUPPER: str += "upper"; break;
        case opATAN2: str += "atan2"; break;
        case opSTRLEN: str += "length"; break;
        case opREPLACE: str += "replace"; break;
        case opSUBSTR: str += "substr"; break;
        default: str += "?";
      }
      // currently all functions take one parameter
      str += QString( "(%1)" ).arg( mLeft->makeSearchString() );
    }
    else if ( mOp == opLENGTH || mOp == opAREA || mOp == opPERIMETER || mOp == opROWNUM || mOp == opID || mOp == opX || mOp == opY )
    {
      // special nullary opeators
      switch ( mOp )
      {
        case opLENGTH: str += "$length"; break;
        case opAREA: str += "$area"; break;
        case opPERIMETER: str += "$perimeter"; break;
        case opROWNUM: str += "$rownum"; break;
        case opX: str += "$x"; break;
        case opY: str += "$y"; break;
        case opID: str += "$id"; break;
        default: str += "?";
      }
    }
    else if ( mOp == opNOT )
    {
      // unary NOT operator
      str += "(NOT " + mLeft->makeSearchString() + ")";
    }
    else
    {
      // the rest of operator using infix notation
      str += "(";
      if ( mLeft )
      {
        str += mLeft->makeSearchString();
      }
      switch ( mOp )
      {
        case opAND: str += " AND "; break;
        case opOR: str += " OR "; break;

        case opPLUS:  str += "+"; break;
        case opMINUS: str += "-"; break;
        case opMUL:   str += "*"; break;
        case opMOD:   str += "%"; break;
        case opDIV:   str += "/"; break;
        case opPOW:   str += "^"; break;

        case opEQ: str += " = "; break;
        case opNE: str += " != "; break;
        case opGT: str += " > "; break;
        case opLT: str += " < "; break;
        case opGE: str += " >= "; break;
        case opLE: str += " <= "; break;

        case opISNULL: str += " IS NULL"; break;
        case opISNOTNULL: str += " IS NOT NULL"; break;

        case opRegexp: str += " ~ "; break;
        case opLike: str += " LIKE "; break;
        case opILike: str += " ILIKE "; break;
        case opIN: str += " IN "; break;
        case opNOTIN: str += " NOT IN "; break;

        case opCONCAT: str += " || "; break;

        default: str += " ? ";
      }

      if ( mRight )
      {
        str += mRight->makeSearchString();
      }
      str += ")";
    }
  }
  else if ( mType == tNumber )
  {
    str += QString::number( mNumber );
  }
  else if ( mType == tString || mType == tColumnRef )
  {
    str += mText;
  }
  else if ( mType == tNodeList )
  {
    QStringList items;
    foreach( QgsSearchTreeNode * node, mNodeList )
    {
      items << node->makeSearchString();
    }

    str += "(" + items.join( "," ) + ")";
  }
  else // unknown type
  {
    str += "unknown_node_type:";
    str += QString::number( mType );
  }

  return str;
}

QStringList QgsSearchTreeNode::referencedColumns()
{
  QList<QgsSearchTreeNode*> columnNodeList = columnRefNodes();
  QSet<QString> columnStringSet;

  QList<QgsSearchTreeNode*>::const_iterator nodeIt = columnNodeList.constBegin();
  for ( ; nodeIt != columnNodeList.constEnd(); ++nodeIt )
  {
    columnStringSet.insert(( *nodeIt )->columnRef() );
  }
  return columnStringSet.toList();
}

QList<QgsSearchTreeNode*> QgsSearchTreeNode::columnRefNodes()
{
  QList<QgsSearchTreeNode*> nodeList;
  if ( mType == tOperator )
  {
    if ( mLeft )
    {
      nodeList += mLeft->columnRefNodes();
    }
    if ( mRight )
    {
      nodeList += mRight->columnRefNodes();
    }
  }
  else if ( mType == tColumnRef )
  {
    nodeList.push_back( this );
  }
  return nodeList;
}

bool QgsSearchTreeNode::needsGeometry()
{
  if ( mType == tOperator )
  {
    if ( mOp == opLENGTH || mOp == opAREA || mOp == opPERIMETER || mOp == opX || mOp == opY )
      return true;

    if ( mLeft && mLeft->needsGeometry() )
      return true;
    if ( mRight && mRight->needsGeometry() )
      return true;
    return false;
  }
  else
  {
    return false;
  }
}

bool QgsSearchTreeNode::checkAgainst( const QgsFieldMap& fields, const QgsAttributeMap &attributes, QgsGeometry* geom )
{
  QgsFeature f;
  f.setAttributeMap( attributes );
  if ( geom )
    f.setGeometry( *geom );
  return checkAgainst( fields, f );
}

bool QgsSearchTreeNode::checkAgainst( const QgsFieldMap& fields, QgsFeature &f )
{
  QgsDebugMsgLevel( "checkAgainst: " + makeSearchString(), 2 );

  mError = "";

  // this error should be caught when checking syntax, but for sure...
  if ( mType != tOperator )
  {
    mError = QObject::tr( "Expected operator, got scalar value!" );
    return false;
  }

  QgsSearchTreeValue value1, value2;
  int res;

  switch ( mOp )
  {
    case opNOT:
      return !mLeft->checkAgainst( fields, f );

    case opAND:
      if ( !mLeft->checkAgainst( fields, f ) )
        return false;
      return mRight->checkAgainst( fields, f );

    case opOR:
      if ( mLeft->checkAgainst( fields, f ) )
        return true;
      return mRight->checkAgainst( fields, f );

    case opISNULL:
    case opISNOTNULL:
      if ( !getValue( value1, mLeft, fields, f ) )
        return false;

      return ( mOp == opISNULL ) == value1.isNull();

    case opEQ:
    case opNE:
    case opGT:
    case opLT:
    case opGE:
    case opLE:
      if ( !getValue( value1, mLeft, fields, f ) || !getValue( value2, mRight, fields, f ) )
        return false;

      if ( value1.isNull() || value2.isNull() )
      {
        // NULL values never match
        return false;
      }

      res = QgsSearchTreeValue::compare( value1, value2 );

      switch ( mOp )
      {
        case opEQ: return res == 0;
        case opNE: return res != 0;
        case opGT: return res >  0;
        case opLT: return res <  0;
        case opGE: return res >= 0;
        case opLE: return res <= 0;
        default:
          mError = QObject::tr( "Unexpected state when evaluating operator!" );
          return false;
      }

    case opIN:
    case opNOTIN:
    {
      if ( !getValue( value1, mLeft, fields, f ) ||
           !mRight || mRight->type() != tNodeList )
      {
        return false;
      }

      foreach( QgsSearchTreeNode * node, mRight->mNodeList )
      {
        if ( !getValue( value2, node, fields, f ) )
        {
          mError = QObject::tr( "Could not retrieve value of list value" );
          return false;
        }

        res = QgsSearchTreeValue::compare( value1, value2 );

        if ( res == 0 )
        {
          // found
          return mOp == opIN;
        }
      }

      return mOp == opNOTIN;
    }

    case opRegexp:
    case opLike:
    case opILike:
    {
      if ( !getValue( value1, mLeft, fields, f ) ||
           !getValue( value2, mRight, fields, f ) )
        return false;

      // value1 is string to be matched
      // value2 is regular expression

      // XXX does it make sense to use regexp on numbers?
      // in what format should they be?
      if ( value1.isNumeric() || value2.isNumeric() )
      {
        mError = QObject::tr( "Regular expressions on numeric values don't make sense. Use comparison instead." );
        return false;
      }

      // TODO: reuse QRegExp

      QString str = value2.string();
      if ( mOp == opLike || mOp == opILike ) // change from LIKE syntax to regexp
      {
        // XXX escape % and _  ???
        str.replace( "%", ".*" );
        str.replace( "_", "." );
        return QRegExp( str, mOp == opLike ? Qt::CaseSensitive : Qt::CaseInsensitive ).exactMatch( value1.string() );
      }
      else
      {
        return QRegExp( str ).indexIn( value1.string() ) != -1;
      }
    }

    default:
      mError = QObject::tr( "Unknown operator: %1" ).arg( mOp );
  }

  return false;
}

bool QgsSearchTreeNode::getValue( QgsSearchTreeValue& value,
                                  QgsSearchTreeNode* node,
                                  const QgsFieldMap &fields,
                                  const QgsAttributeMap &attributes,
                                  QgsGeometry* geom )
{
  QgsFeature f;
  f.setAttributeMap( attributes );
  if ( geom )
    f.setGeometry( *geom );
  return getValue( value, node, fields, f );
}

bool QgsSearchTreeNode::getValue( QgsSearchTreeValue& value,
                                  QgsSearchTreeNode* node,
                                  const QgsFieldMap& fields,
                                  QgsFeature &f )
{
  value = node->valueAgainst( fields, f );
  if ( value.isError() )
  {
    switch (( int ) value.number() )
    {
      case 1:
        mError = QObject::tr( "Referenced column wasn't found: %1" ).arg( value.string() );
        break;
      case 2:
        mError = QObject::tr( "Division by zero." );
        break;

        // these should never happen (no need to translate)
      case 3:
        mError = QObject::tr( "Unknown operator: %1" ).arg( value.string() );
        break;
      case 4:
        mError = QObject::tr( "Unknown token: %1" ).arg( value.string() );
        break;
      default:
        mError = QObject::tr( "Unknown error!" );
        break;
    }
    return false;
  }
  return true;
}

QgsSearchTreeValue QgsSearchTreeNode::valueAgainst( const QgsFieldMap& fields,
    const QgsAttributeMap &attributes,
    QgsGeometry* geom )
{
  QgsFeature f;
  f.setAttributeMap( attributes );
  if ( geom )
    f.setGeometry( *geom );
  return valueAgainst( fields, f );
}

QgsSearchTreeValue QgsSearchTreeNode::valueAgainst( const QgsFieldMap& fields, QgsFeature &f )
{
  QgsDebugMsgLevel( "valueAgainst: " + makeSearchString(), 2 );

  switch ( mType )
  {
    case tNumber:
      QgsDebugMsgLevel( "number: " + QString::number( mNumber ), 2 );
      return QgsSearchTreeValue( mNumber );

    case tString:
      QgsDebugMsgLevel( "text: " + EVAL_STR( mText ), 2 );
      return QgsSearchTreeValue( mText );

    case tColumnRef:
    {
      QgsDebugMsgLevel( "column (" + mText.toLower() + "): ", 2 );
      // find field index for the column
      QgsFieldMap::const_iterator it;
      for ( it = fields.begin(); it != fields.end(); it++ )
      {
        if ( QString::compare( it->name(), mText, Qt::CaseInsensitive ) == 0 )
          break;
      }

      if ( it == fields.end() )
      {
        // report missing column if not found
        QgsDebugMsgLevel( "ERROR!", 2 );
        return QgsSearchTreeValue( 1, mText );
      }

      // get the value
      QVariant val = f.attributeMap()[it.key()];
      if ( val.isNull() )
      {
        QgsDebugMsgLevel( "   NULL", 2 );
        return QgsSearchTreeValue();
      }
      else if ( val.type() == QVariant::Bool || val.type() == QVariant::Int || val.type() == QVariant::Double )
      {
        QgsDebugMsgLevel( "   number: " + QString::number( val.toDouble() ), 2 );
        return QgsSearchTreeValue( val.toDouble() );
      }
      else
      {
        QgsDebugMsgLevel( "   text: " + EVAL_STR( val.toString() ), 2 );
        return QgsSearchTreeValue( val.toString() );
      }

    }

    // arithmetic operators
    case tOperator:
    {
      QgsSearchTreeValue value1, value2, value3;
      if ( mLeft )
      {
        if ( mLeft->type() != tNodeList )
        {
          if ( !getValue( value1, mLeft, fields, f ) )
            return value1;
        }
        else
        {
          if ( mLeft->mNodeList.size() > 0 && !getValue( value1, mLeft->mNodeList[0], fields, f ) )
            return value1;
          if ( mLeft->mNodeList.size() > 1 && !getValue( value2, mLeft->mNodeList[1], fields, f ) )
            return value2;
          if ( mLeft->mNodeList.size() > 2 && !getValue( value3, mLeft->mNodeList[2], fields, f ) )
            return value3;
        }
      }
      if ( mRight )
      {
        Q_ASSERT( !mLeft || mLeft->type() != tNodeList );
        if ( !getValue( value2, mRight, fields, f ) )
          return value2;
      }

      if ( mOp == opLENGTH || mOp == opAREA || mOp == opPERIMETER || mOp == opX || mOp == opY )
      {
        if ( !f.geometry() )
        {
          return QgsSearchTreeValue( 2, "Geometry is 0" );
        }

        //check that we don't use area for lines or length for polygons
        if ( mOp == opLENGTH && f.geometry()->type() == QGis::Line )
        {
          return QgsSearchTreeValue( mCalc->measure( f.geometry() ) );
        }
        if ( mOp == opAREA && f.geometry()->type() == QGis::Polygon )
        {
          return QgsSearchTreeValue( mCalc->measure( f.geometry() ) );
        }
        if ( mOp == opPERIMETER && f.geometry()->type() == QGis::Polygon )
        {
          return QgsSearchTreeValue( mCalc->measurePerimeter( f.geometry() ) );
        }
        if ( mOp == opX && f.geometry()->type() == QGis::Point )
        {
          return QgsSearchTreeValue( f.geometry()->asPoint().x() );
        }
        if ( mOp == opY && f.geometry()->type() == QGis::Point )
        {
          return QgsSearchTreeValue( f.geometry()->asPoint().y() );
        }
        return QgsSearchTreeValue( 0 );
      }

      if ( mOp == opID )
      {
        return QgsSearchTreeValue( f.id() );
      }

      if ( mOp == opROWNUM )
      {
        // the row number has to be previously set by the caller using setCurrentRowNumber
        return QgsSearchTreeValue( mNumber );
      }

      //string operations with one argument
      if ( !mRight && !value1.isNumeric() )
      {
        if ( mOp == opTOINT )
        {
          return QgsSearchTreeValue( value1.string().toInt() );
        }
        else if ( mOp == opTOREAL )
        {
          return QgsSearchTreeValue( value1.string().toDouble() );
        }
      }

      //don't convert to numbers in case of string concatenation
      if ( mLeft && mRight && !value1.isNumeric() && !value2.isNumeric() )
      {
        // TODO: concatenation using '+' operator should be removed in favor of '||' operator
        // because it may lead to surprising behavior if numbers are stored in a string
        if ( mOp == opPLUS )
        {
          return QgsSearchTreeValue( value1.string() + value2.string() );
        }
      }

      // string concatenation ||
      if ( mLeft && mRight && mOp == opCONCAT )
      {
        if ( value1.isNumeric() && value2.isNumeric() )
        {
          return QgsSearchTreeValue( 5, "Operator doesn't match the argument types." );
        }
        else
        {
          QString arg1 = value1.isNumeric() ? QString::number( value1.number() ) : value1.string();
          QString arg2 = value2.isNumeric() ? QString::number( value2.number() ) : value2.string();
          return QgsSearchTreeValue( arg1 + arg2 );
        }
      }

      // string operations
      switch ( mOp )
      {
        case opLOWER:
          return QgsSearchTreeValue( value1.string().toLower() );
        case opUPPER:
          return QgsSearchTreeValue( value1.string().toUpper() );
        case opSTRLEN:
          return QgsSearchTreeValue( value1.string().length() );
        case opREPLACE:
          return QgsSearchTreeValue( value1.string().replace( value2.string(), value3.string() ) );
        case opSUBSTR:
          return QgsSearchTreeValue( value1.string().mid( value2.number() - 1, value3.number() ) );
        default:
          break;
      }

      // for other operators, convert strings to numbers if needed
      double val1, val2;
      if ( value1.isNumeric() )
        val1 = value1.number();
      else
        val1 = value1.string().toDouble();
      if ( value2.isNumeric() )
        val2 = value2.number();
      else
        val2 = value2.string().toDouble();

      switch ( mOp )
      {
        case opPLUS:
          return QgsSearchTreeValue( val1 + val2 );
        case opMINUS:
          return QgsSearchTreeValue( val1 - val2 );
        case opMUL:
          return QgsSearchTreeValue( val1 * val2 );
        case opMOD:
          // NOTE: we _might_ support float operators, like postgresql does
          // see 83c94a886c059 commit in postgresql git repo for more info
          return QgsSearchTreeValue( int(val1) % int(val2) );
        case opDIV:
          if ( val2 == 0 )
            return QgsSearchTreeValue( 2, "" ); // division by zero
          else
            return QgsSearchTreeValue( val1 / val2 );
        case opPOW:
          if (( val1 == 0 && val2 < 0 ) || ( val2 < 0 && ( val2 - floor( val2 ) ) > 0 ) )
          {
            return QgsSearchTreeValue( 4, "Error in power function" );
          }
          return QgsSearchTreeValue( pow( val1, val2 ) );
        case opSQRT:
          return QgsSearchTreeValue( sqrt( val1 ) );
        case opSIN:
          return QgsSearchTreeValue( sin( val1 ) );
        case opCOS:
          return QgsSearchTreeValue( cos( val1 ) );
        case opTAN:
          return QgsSearchTreeValue( tan( val1 ) );
        case opASIN:
          return QgsSearchTreeValue( asin( val1 ) );
        case opACOS:
          return QgsSearchTreeValue( acos( val1 ) );
        case opATAN:
          return QgsSearchTreeValue( atan( val1 ) );
        case opATAN2:
          return QgsSearchTreeValue( atan2( val1, val2 ) );
        case opTOINT:
          return QgsSearchTreeValue( int( val1 ) );
        case opTOREAL:
          return QgsSearchTreeValue( val1 );
        case opTOSTRING:
          return QgsSearchTreeValue( QString::number( val1 ) );

        default:
          return QgsSearchTreeValue( 3, QString::number( mOp ) ); // unknown operator
      }
    }

    default:
      return QgsSearchTreeValue( 4, QString::number( mType ) ); // unknown token
  }
}


void QgsSearchTreeNode::setCurrentRowNumber( int rownum )
{
  if ( mType == tOperator )
  {
    if ( mOp == opROWNUM )
      mNumber = rownum;
    else
    {
      // propagate the new row number to children
      if ( mLeft )
        mLeft->setCurrentRowNumber( rownum );
      if ( mRight )
        mRight->setCurrentRowNumber( rownum );
    }
  }
}

void QgsSearchTreeNode::append( QgsSearchTreeNode *node )
{
  Q_ASSERT( mType == tNodeList );
  mNodeList.append( node );
}

void QgsSearchTreeNode::append( QList<QgsSearchTreeNode *> nodes )
{
  foreach( QgsSearchTreeNode * node, nodes )
  {
    mNodeList.append( node );
  }
}

int QgsSearchTreeValue::compare( QgsSearchTreeValue& value1, QgsSearchTreeValue& value2, Qt::CaseSensitivity cs )
{
  if ( value1.isNumeric() || value2.isNumeric() )
  {
    // numeric comparison

    // convert to numbers if needed
    double val1, val2;
    if ( value1.isNumeric() )
      val1 = value1.number();
    else
      val1 = value1.string().toDouble();
    if ( value2.isNumeric() )
      val2 = value2.number();
    else
      val2 = value2.string().toDouble();

    QgsDebugMsgLevel( "NUM_COMP: " + QString::number( val1 ) + " ~ " + QString::number( val2 ), 2 );

    if ( val1 < val2 )
      return -1;
    else if ( val1 > val2 )
      return 1;
    else
      return 0;
  }
  else
  {
    // string comparison
    return value1.string().compare( value2.string(), cs );
  }
}
