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
 /* $Id$ */
 
#include "qgssearchtreenode.h"
#include <qregexp.h>
#include <qobject.h>
#include <iostream>

// turn on/off debugging of search tree evaulation
#undef DEBUG_TREE_EVAL

#ifdef DEBUG_TREE_EVAL
#define TREE_EVAL(x)  std::cout << x;
#define TREE_EVAL2(x,y) std::cout << x << y << std::endl;
#define TREE_EVAL3(x,y,z) std::cout << x << y << z << std::endl;
#define TREE_EVAL4(x,y,z,zz) std::cout << x << y << z << zz << std::endl;
#define EVAL_STR(x) (x.length() ? x : "(empty)")
#else
#define TREE_EVAL(x) 
#define TREE_EVAL2(x,y) 
#define TREE_EVAL3(x,y,z) 
#define TREE_EVAL4(x,y,z,zz) 
#endif

QgsSearchTreeNode::QgsSearchTreeNode(double number)
{
  mType   = tNumber;
  mNumber = number;
  mLeft   = NULL;
  mRight  = NULL;
}


QgsSearchTreeNode::QgsSearchTreeNode(Operator op, QgsSearchTreeNode* left,
                                      QgsSearchTreeNode* right)
{
  mType  = tOperator;
  mOp    = op;
  mLeft  = left;
  mRight = right;
}


QgsSearchTreeNode::QgsSearchTreeNode(QString text, bool isColumnRef)
{
  mLeft  = NULL;
  mRight = NULL;
  
  if (isColumnRef)
  {
    mType = tColumnRef;
    mText = text;
  }
  else
  {
    mType = tString;
    mText = text;
    stripText();
  }
}


QgsSearchTreeNode::QgsSearchTreeNode(const QgsSearchTreeNode& node)
{
  mType = node.mType;
  mOp = node.mOp;
  mNumber = node.mNumber;
  mText = node.mText;

  // recursively copy children
  if (node.mLeft)
    mLeft =  new QgsSearchTreeNode(*node.mLeft);
  else
    mLeft = NULL;
  
  if (node.mRight)
    mRight = new QgsSearchTreeNode(*node.mRight);
  else
    mRight = NULL;
}


QgsSearchTreeNode::~QgsSearchTreeNode()
{
  // delete children
  
  if (mLeft)
    delete mLeft;

  if (mRight)
    delete mRight;
}

void QgsSearchTreeNode::stripText()
{
  // strip single quotes on start,end
  mText = mText.mid(1, mText.length()-2);
  
  // make single "single quotes" from double "single quotes"
  mText.replace(QRegExp("''"), "'");
  
  // strip \n \' etc.
  int index = 0;
  while ((index = mText.find('\\', index)) != -1)
  {
    mText.remove(index,1); // delete backslash
    QChar chr;
    switch (mText[index].latin1()) // evaluate backslashed character
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

QString QgsSearchTreeNode::makeSearchString()
{
  QString str;
  if (mType == tOperator)
  {
    str += "(";
    if (mOp != opNOT)
    {
      str += mLeft->makeSearchString();
      switch (mOp)
      {
        case opAND: str += " AND "; break;
        case opOR: str += " OR "; break;

        case opPLUS:  str += "+"; break;
        case opMINUS: str += "-"; break;
        case opMUL:   str += "*"; break;
        case opDIV:   str += "/"; break;

        case opEQ: str += " = "; break;
        case opNE: str += " != "; break;
        case opGT: str += " > "; break;
        case opLT: str += " < "; break;
        case opGE: str += " >= "; break;
        case opLE: str += " <= "; break;

        case opRegexp: str += " ~ "; break;
        case opLike: str += " LIKE "; break;

        default: str += " ? ";
      }
      str += mRight->makeSearchString();
    }
    else
    {
      str += "NOT ";
      str += mLeft->makeSearchString();
    }
    str += ")";
  }
  else if (mType == tNumber)
  {
    str += QString::number(mNumber);
  }
  else if (mType == tString || mType == tColumnRef)
  {
    str += mText;
  }
  else // unknown type
  {
    str += "unknown_node_type:";
    str += QString::number(mType);
  }

  return str;
}


bool QgsSearchTreeNode::checkAgainst(const std::vector<QgsFeatureAttribute>& attributes)
{
  TREE_EVAL2("checkAgainst: ", makeSearchString());

  mError = "";
  
  // this error should be caught when checking syntax, but for sure...
  if (mType != tOperator)
  {
    mError = "Expected operator, got scalar value!";
    return false;
  }

  QgsSearchTreeValue value1, value2;
  int res;

  switch (mOp)
  {
    case opNOT:
      return !mLeft->checkAgainst(attributes);
    
    case opAND:
      if (!mLeft->checkAgainst(attributes))
        return false;
      return mRight->checkAgainst(attributes);
    
    case opOR:
      if (mLeft->checkAgainst(attributes))
        return true;
      return mRight->checkAgainst(attributes);

    case opEQ:
    case opNE:
    case opGT:
    case opLT:
    case opGE:
    case opLE:
        
      if (!getValue(value1, mLeft, attributes) || !getValue(value2, mRight, attributes))
            return false;
        
      res = QgsSearchTreeValue::compare(value1, value2);
        
      switch (mOp)
      {
        case opEQ: return (res == 0);
        case opNE: return (res != 0);
        case opGT: return (res >  0);
        case opLT: return (res <  0);
        case opGE: return (res >= 0);
        case opLE: return (res <= 0);
        default: 
          mError = "Unexpected state when evaluating operator!";
          return false;
      }

    case opRegexp:
    case opLike:
    {
      if (!getValue(value1, mLeft, attributes) || !getValue(value2, mRight, attributes))
        return false;
      
      // value1 is string to be matched
      // value2 is regular expression
      
      // XXX does it make sense to use regexp on numbers?
      // in what format should they be?
      if (value1.isNumeric() || value2.isNumeric())
      {
        mError = QObject::tr("Regular expressions on numeric values don't make sense. Use comparison insted.");
        return false;
      }
      
      QString str = value2.string();
      if (mOp == opLike) // change from LIKE syntax to regexp
      {
        // XXX escape % and _  ???
        str.replace("%", ".*");
        str.replace("_", ".");
      }
      
      QRegExp re(str);
      res = re.search(value1.string());
      TREE_EVAL4("REGEXP: ", str, " ~ ", value2.string());
      TREE_EVAL2("   res: ", res);
      return (res != -1);
    }

    default:
      mError = "Unknown operator: ";
      mError += QString::number(mOp);
      return false;
  }

  return false; // will never get there
}

bool QgsSearchTreeNode::getValue(QgsSearchTreeValue& value, QgsSearchTreeNode* node, const std::vector<QgsFeatureAttribute>& attributes)
{
  value = node->valueAgainst(attributes);
  if (value.isError())
  {
    switch ((int)value.number())
    {
      case 1:
        mError = QObject::tr("Referenced column wasn't found: ");
        mError += value.string();
        break;
      case 2:
        mError = QObject::tr("Division by zero.");
        break;
        
      // these should never happen (no need to translate)
      case 3:
        mError = "Unknown operator: ";
        mError += value.string();
        break;
      case 4:
        mError = "Unknown token: ";
        mError += value.string();
        break;
      default:
        mError = "Unknown error!";
        break;
    }
    return false;        
  }
  return true;
}

QgsSearchTreeValue QgsSearchTreeNode::valueAgainst(const std::vector<QgsFeatureAttribute>& attributes)
{
  TREE_EVAL2("valueAgainst: ", makeSearchString());

  switch (mType)
  {

    case tNumber:
      TREE_EVAL2("number: ", mNumber);
      return QgsSearchTreeValue(mNumber);
  
    case tString:
      TREE_EVAL2("text: ", EVAL_STR(mText));
      return QgsSearchTreeValue(mText);
  
    case tColumnRef:
    {
      TREE_EVAL3("column (", mText, "): ");;
      // find value for the column
      std::vector<QgsFeatureAttribute>::const_iterator it;
      for (it = attributes.begin(); it != attributes.end(); it++)
      {
        if ( (*it).fieldName().lower() == mText.lower()) // TODO: optimize
        {
          QString value = (*it).fieldValue();
          if ((*it).isNumeric())
          {
            TREE_EVAL2("   number: ", value.toDouble());
            return QgsSearchTreeValue(value.toDouble());
          }
          else
          {
            TREE_EVAL2("   text: ", EVAL_STR(value));
            return QgsSearchTreeValue(value);
          }
        }
      }
          
      // else report missing column
      TREE_EVAL("ERROR");
      return QgsSearchTreeValue(1, mText);
    }
    
    // arithmetic operators
    case tOperator:
      {
        QgsSearchTreeValue value1, value2;
        if (!getValue(value1, mLeft,  attributes)) return value1;
        if (!getValue(value2, mRight, attributes)) return value2;
        
        // convert to numbers if needed
        double val1, val2;
        if (value1.isNumeric())
          val1 = value1.number();
        else
          val1 = value1.string().toDouble();
        if (value2.isNumeric())
          val2 = value2.number();
        else
          val2 = value2.string().toDouble();
      
        switch (mOp)
        {
          case opPLUS:
            return QgsSearchTreeValue(val1 + val2);
          case opMINUS:
            return QgsSearchTreeValue(val1 - val2);
          case opMUL:
            return QgsSearchTreeValue(val1 * val2);
          case opDIV:
            if (val2 == 0)
              return QgsSearchTreeValue(2, ""); // division by zero
            else
              return QgsSearchTreeValue(val1 / val2);
          default:
            return QgsSearchTreeValue(3, QString::number(mOp)); // unknown operator
        }
      }
      
    default:
      return QgsSearchTreeValue(4, QString::number(mType)); // unknown token
  }
}


int QgsSearchTreeValue::compare(QgsSearchTreeValue& value1, QgsSearchTreeValue& value2)
{
  if (value1.isNumeric() || value2.isNumeric())
  {
    // numeric comparison
    
    // convert to numbers if needed
    double val1, val2;
    if (value1.isNumeric())
      val1 = value1.number();
    else
      val1 = value1.string().toDouble();
    if (value2.isNumeric())
      val2 = value2.number();
    else
      val2 = value2.string().toDouble();
   
    TREE_EVAL4("NUM_COMP: ", val1, " ~ ", val2);

    if (val1 < val2)
      return -1;
    else if (val1 > val2)
      return 1;
    else
      return 0;
  }
  else
  {
    // string comparison
    return value1.string().compare(value2.string());
  }
}
