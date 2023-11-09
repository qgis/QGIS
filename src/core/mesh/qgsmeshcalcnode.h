/***************************************************************************
                          qgsmeshcalcnode.h
                          --------------------------------
    begin                : December 18th, 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHCALCNODE_H
#define QGSMESHCALCNODE_H

#define SIP_NO_FILE

///@cond PRIVATE

#include <QMap>
#include <QString>
#include <QStringList>
#include <memory>
#include <limits>

#include "qgis_core.h"

#include "qgsmeshcalcutils.h"

/**
 * \ingroup core
 * \class QgsMeshCalcNode
 * \brief Represents a single calculation node
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsMeshCalcNode
{
  public:
    //! types of mesh node
    enum Type
    {
      tOperator = 1, //!< Operator (e.g. +, -)
      tNumber, //!< Number (e.g. 1)
      tNoData, //!< Nodata (NaN)
      tDatasetGroupRef //!< Dataset group
    };

    //! operators between dataset groups
    enum Operator
    {
      opPLUS,     //!< Plus
      opMINUS,    //!< Minus
      opMUL,      //!< Multiply
      opDIV,      //!< Divide
      opPOW,      //!< Power
      opEQ,       //!< Equal
      opNE,       //!< Not equal
      opGT,       //!< Greater than
      opLT,       //!< Lower than
      opGE,       //!< Greater or equal
      opLE,       //!< Lower or equal
      opAND,      //!< Boolean AND
      opOR,       //!< Boolean OR
      opNOT,      //!< Boolean NOT (unary)
      opIF,       //!< Boolean IF
      opSIGN,     //!< Change sign (unary)
      opMIN,      //!< Minimum value
      opMAX,      //!< Maximum value
      opABS,      //!< Absolute value (unary)
      opSUM_AGGR, //!< Aggregated sum (unary)
      opMAX_AGGR, //!< Aggregated maximum (unary)
      opMIN_AGGR, //!< Aggregated minimum (unary)
      opAVG_AGGR, //!< Aggregated average (unary)
      opNONE,     //!< Nodata value
    };

    /**
     * Constructs a Type::tNoData node initialized with NODATA values
     */
    QgsMeshCalcNode();

    /**
     * Constructs a Type::tNumber node initialized with selected number
     */
    QgsMeshCalcNode( double number );

    /**
     * Constructs a Type::tOperator node for binary or unary operator
     * \param op Operator to perform
     * \param left Left node. This node takes ownership of the node
     * \param right Right node (for binary operators). This node takes ownership of the node
     */
    QgsMeshCalcNode( Operator op, QgsMeshCalcNode *left, QgsMeshCalcNode *right );

    /**
     * Constructs a Type::tOperator node for IF operator (condition)
     * \param condition node to determine if left or right value is used
     * \param left Left node. This node takes ownership of the node
     * \param right Right node. This node takes ownership of the node
     */
    QgsMeshCalcNode( QgsMeshCalcNode *condition /* bool condition */,
                     QgsMeshCalcNode *left /*if true */,
                     QgsMeshCalcNode *right /* if false */ );

    /**
     * Constructs a Type::tDatasetGroupRef node with values from dataset group
     * \param datasetName dataset group to fetch data and populate node data
     */
    QgsMeshCalcNode( const QString &datasetGroupName );

    //! Destructor
    ~QgsMeshCalcNode();

    //! Returns type of node
    Type type() const;

    //! Sets left node. This node takes ownership of the node
    void setLeft( QgsMeshCalcNode *left );

    //! Sets right node. This node takes ownership of the node
    void setRight( QgsMeshCalcNode *right );

    /**
     * Calculates result of mesh calculation
     * \param dsu utils with initial conditions for calculation (times, dataset groups)
     * \param result destination dataset group for calculation results
     * \returns TRUE on success, FALSE on failure
     */
    bool calculate( const QgsMeshCalcUtils &dsu, QgsMeshMemoryDatasetGroup &result, bool isAggregate = false ) const;

    //! Returns all dataset group names used in formula
    QStringList usedDatasetGroupNames() const;

    //! Returns dataset group names used in formula involved in aggregate function
    QStringList aggregatedUsedDatasetGroupNames() const;

    //! Returns dataset group names used in formula not involved in aggregate function
    QStringList notAggregatedUsedDatasetGroupNames() const;

    /**
     * Parses string to calculation node. Caller takes responsibility to delete the node
     * \param str string with formula definition
     * \param parserErrorMsg error message on error
     * \returns calculation node. Nullptr on error
     */
    static QgsMeshCalcNode *parseMeshCalcString( const QString &str, QString &parserErrorMsg );

    /**
     * Returns whether the calculation will leads to a non temporal dataset group result
     * \returns TRUE if the result will be non temporal
     */
    bool isNonTemporal() const;

  private:
    Q_DISABLE_COPY( QgsMeshCalcNode )

    Type mType = tNoData;
    std::unique_ptr<QgsMeshCalcNode> mLeft;
    std::unique_ptr<QgsMeshCalcNode> mRight;
    std::unique_ptr<QgsMeshCalcNode> mCondition;
    double mNumber = std::numeric_limits<double>::quiet_NaN();
    QString mDatasetGroupName;
    Operator mOperator = opNONE;
};

///@endcond

#endif // QGSMESHCALCNODE_H
