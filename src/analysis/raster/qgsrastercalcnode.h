/***************************************************************************
                          qgsrastercalcnode.h
            Node for raster calculator tree
                          --------------------
    begin                : 2010-10-23
    copyright            : (C) 20010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERCALCNODE_H
#define QGSRASTERCALCNODE_H

#include "qgsrastermatrix.h"
#include <QMap>
#include <QString>

class ANALYSIS_EXPORT QgsRasterCalcNode
{
  public:
    //! defines possible types of node
    enum Type
    {
      tOperator = 1,
      tNumber,
      tRasterRef
    };

    //! possible operators
    enum Operator
    {
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
      opEQ,         // =
      opNE,         //!=
      opGT,         // >
      opLT,         // <
      opGE,         // >=
      opLE,         // <=
      opAND,
      opOR,
      opSIGN    //change sign
    };

    QgsRasterCalcNode();
    QgsRasterCalcNode( double number );
    QgsRasterCalcNode( Operator op, QgsRasterCalcNode* left, QgsRasterCalcNode* right );
    QgsRasterCalcNode( const QString& rasterName );
    ~QgsRasterCalcNode();

    Type type() const { return mType; }

    //set left node
    void setLeft( QgsRasterCalcNode* left ) { delete mLeft; mLeft = left; }
    void setRight( QgsRasterCalcNode* right ) { delete mRight; mRight = right; }

    /**Calculates result (might be real matrix or single number)*/
    bool calculate( QMap<QString, QgsRasterMatrix*>& rasterData, QgsRasterMatrix& result ) const;

    static QgsRasterCalcNode* parseRasterCalcString( const QString& str, QString& parserErrorMsg );

  private:
    Type mType;
    QgsRasterCalcNode* mLeft;
    QgsRasterCalcNode* mRight;
    double mNumber;
    QString mRasterName;
    Operator mOperator;
};


#endif // QGSRASTERCALCNODE_H
