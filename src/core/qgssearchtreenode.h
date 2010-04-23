/***************************************************************************
                          qgssearchtreenode.h
            Definition of node for parsed tree of search string
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

#ifndef QGSSEARCHTREENODE_H
#define QGSSEARCHTREENODE_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <qgsfield.h>
#include <qgsfeature.h>

class QgsDistanceArea;
class QgsSearchTreeValue;

/** \ingroup core
 * A representation of a node in a search tree.
 *
 * node in tree of parsed search string
 * node is terminal (has no children) if it's a number, column ref or string
 * non-terminal is only node with operator - with 1 or 2 children
 */
class CORE_EXPORT QgsSearchTreeNode
{
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
      opISNULL,  // IS NULL
      opISNOTNULL,  // IS NOT NULL
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
    QgsSearchTreeNode( Operator op, QgsSearchTreeNode* left, QgsSearchTreeNode* right );
    QgsSearchTreeNode( QString text, bool isColumnRef );

    //! copy contructor - copies whole tree!
    QgsSearchTreeNode( const QgsSearchTreeNode& node );

    //! destructor - deletes children nodes (if any)
    ~QgsSearchTreeNode();

    //! returns type of current node
    Type type()   { return mType; }

    //! node value getters
    Operator op();
    double number() { return mNumber; }
    QString columnRef() { return mText; }
    QString string() { return mText; }

    //! node value setters (type is set also)
    void setOp( Operator op )         { mType = tOperator;  mOp = op; }
    void setNumber( double number )   { mType = tNumber;    mNumber = number; }
    void setColumnRef( QString& str ) { mType = tColumnRef; mText = str; }
    void setString( QString& str )    { mType = tString;    mText = str; stripText(); }

    //! children
    QgsSearchTreeNode* Left()  { return mLeft;  }
    QgsSearchTreeNode* Right() { return mRight; }
    void setLeft( QgsSearchTreeNode* left ) { mLeft = left;   }
    void setRight( QgsSearchTreeNode* right ) { mRight = right; }

    //! returns search string that should be equal to original parsed string
    QString makeSearchString();

    //! checks whether the node tree is valid against supplied attributes
    bool checkAgainst( const QgsFieldMap& fields, const QgsAttributeMap& attributes );

    //! checks if there were errors during evaluation
    bool hasError() { return ( !mError.isEmpty() ); }

    //! returns error message
    const QString& errorMsg() { return mError; }

    //! wrapper around valueAgainst()
    bool getValue( QgsSearchTreeValue& value, QgsSearchTreeNode* node,
                   const QgsFieldMap& fields, const QgsAttributeMap& attributes, QgsGeometry* geom = 0 );

    //! return a list of referenced columns in the tree
    //! @note added in 1.5
    QStringList referencedColumns();

  protected:


    //! returns scalar value of node
    QgsSearchTreeValue valueAgainst( const QgsFieldMap& fields, const QgsAttributeMap& attributes, QgsGeometry* geom = 0 );

    //! strips mText when node is of string type
    void stripText();

    //! initialize node's internals
    void init();

  private:

    //! node type
    Type mType;

    //! data
    Operator mOp;
    double mNumber;
    QString mText;

    QString mError;

    //! children
    QgsSearchTreeNode* mLeft;
    QgsSearchTreeNode* mRight;

    /**For length() and area() functions*/
    QgsDistanceArea* mCalc;
};

// TODO: put it into separate file
class CORE_EXPORT QgsSearchTreeValue
{
  public:

    enum Type
    {
      valError,
      valString,
      valNumber,
      valNull
    };

    QgsSearchTreeValue() { mType = valNull; }
    QgsSearchTreeValue( QString string ) { mType = string.isNull() ? valNull : valString; mString = string; }
    QgsSearchTreeValue( double number ) { mType = valNumber; mNumber = number; }
    QgsSearchTreeValue( int error, QString errorMsg ) { mType = valError; mNumber = error; mString = errorMsg; }

    static int compare( QgsSearchTreeValue& value1, QgsSearchTreeValue& value2,
                        Qt::CaseSensitivity = Qt::CaseSensitive );

    bool isNumeric() { return mType == valNumber; }
    bool isError() { return mType == valError; }
    bool isNull() { return mType == valNull; }

    QString& string() { return mString; }
    double number() { return mNumber; }

  private:
    Type mType;
    QString mString;
    double mNumber;

};

#endif

