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

#include <QMap>
#include "qgis_sip.h"
#include "qgis.h"
#include <QString>
#include "qgis_analysis.h"

class QgsRasterBlock;
class QgsRasterMatrix;

/**
 * \ingroup analysis
 * \class QgsRasterCalcNode
 */
class ANALYSIS_EXPORT QgsRasterCalcNode
{
  public:
    //! defines possible types of node
    enum Type
    {
      tOperator = 1,
      tNumber,
      tRasterRef,
      tMatrix
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
      opSIGN,       // change sign
      opLOG,
      opLOG10,
      opNONE,
    };

    /**
     * Constructor for QgsRasterCalcNode.
     */
    QgsRasterCalcNode() = default;

    QgsRasterCalcNode( double number );
    QgsRasterCalcNode( QgsRasterMatrix *matrix );
    QgsRasterCalcNode( Operator op, QgsRasterCalcNode *left, QgsRasterCalcNode *right );
    QgsRasterCalcNode( const QString &rasterName );
    ~QgsRasterCalcNode();

    //! QgsRasterCalcNode cannot be copied
    QgsRasterCalcNode( const QgsRasterCalcNode &rh ) = delete;
    //! QgsRasterCalcNode cannot be copied
    QgsRasterCalcNode &operator=( const QgsRasterCalcNode &rh ) = delete;

    Type type() const { return mType; }

    //set left node
    void setLeft( QgsRasterCalcNode *left ) { delete mLeft; mLeft = left; }
    void setRight( QgsRasterCalcNode *right ) { delete mRight; mRight = right; }

    /**
     * Calculates result of raster calculation (might be real matrix or single number).
     * \param rasterData input raster data references, map of raster name to raster data block
     * \param result destination raster matrix for calculation results
     * \param row optional row number to calculate for calculating result by rows, or -1 to
     * calculate entire result
     * \note not available in Python bindings
     * \since QGIS 2.10
     */
    bool calculate( QMap<QString, QgsRasterBlock * > &rasterData, QgsRasterMatrix &result, int row = -1 ) const SIP_SKIP;

    /**
     * Returns a string representation of the expression
     * \param cStyle if TRUE operators will follow C syntax
     * \since QGIS 3.6
     */
    QString toString( bool cStyle = false ) const;

    /**
     * Returns a list of nodes of a specific \a type
     * \since QGIS 3.6
     */
    QList<const QgsRasterCalcNode *> findNodes( const QgsRasterCalcNode::Type type ) const;

    static QgsRasterCalcNode *parseRasterCalcString( const QString &str, QString &parserErrorMsg ) SIP_FACTORY;

  private:
#ifdef SIP_RUN
    QgsRasterCalcNode( const QgsRasterCalcNode &rh );
#endif

    Type mType = tNumber;
    QgsRasterCalcNode *mLeft = nullptr;
    QgsRasterCalcNode *mRight = nullptr;
    double mNumber = 0;
    QString mRasterName;
    QgsRasterMatrix *mMatrix = nullptr;
    Operator mOperator = opNONE;

};


#endif // QGSRASTERCALCNODE_H
