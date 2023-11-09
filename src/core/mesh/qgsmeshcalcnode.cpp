/***************************************************************************
                          qgsmeshcalcnode.cpp
                          -------------------
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
///@cond PRIVATE

#include <cfloat>

#include "qgsmeshcalcnode.h"
#include "qgsmeshmemorydataprovider.h"

QgsMeshCalcNode::QgsMeshCalcNode()
  : mType( tNoData )
{
}

QgsMeshCalcNode::QgsMeshCalcNode( double number )
  : mType( tNumber )
  , mNumber( number )
{
}


QgsMeshCalcNode::QgsMeshCalcNode( Operator op, QgsMeshCalcNode *left, QgsMeshCalcNode *right )
  : mType( tOperator )
  , mLeft( left )
  , mRight( right )
  , mOperator( op )
{
}

QgsMeshCalcNode::QgsMeshCalcNode( QgsMeshCalcNode *condition /* bool condition */,
                                  QgsMeshCalcNode *left /*if true */,
                                  QgsMeshCalcNode *right /* if false */ )
  : mType( tOperator )
  , mLeft( left )
  , mRight( right )
  , mCondition( condition )
  , mOperator( opIF )
{
}

QgsMeshCalcNode::QgsMeshCalcNode( const QString &datasetGroupName )
  : mType( tDatasetGroupRef )
  , mDatasetGroupName( datasetGroupName )
{
  if ( mDatasetGroupName.startsWith( '"' ) && mDatasetGroupName.endsWith( '"' ) )
    mDatasetGroupName = mDatasetGroupName.mid( 1, mDatasetGroupName.size() - 2 );
}

QgsMeshCalcNode::~QgsMeshCalcNode() = default;

QgsMeshCalcNode::Type QgsMeshCalcNode::type() const
{
  return mType;
}

void QgsMeshCalcNode::setLeft( QgsMeshCalcNode *left )
{
  mLeft.reset( left );
}

void QgsMeshCalcNode::setRight( QgsMeshCalcNode *right )
{
  mRight.reset( right );
}

QStringList QgsMeshCalcNode::usedDatasetGroupNames() const
{
  QStringList res;

  if ( mType == tDatasetGroupRef )
  {
    res.append( mDatasetGroupName );
  }

  if ( mLeft )
  {
    res += mLeft->usedDatasetGroupNames();
  }

  if ( mRight )
  {
    res += mRight->usedDatasetGroupNames();
  }

  if ( mCondition )
  {
    res += mCondition->usedDatasetGroupNames();
  }

  return res;
}

QStringList QgsMeshCalcNode::aggregatedUsedDatasetGroupNames() const
{
  QStringList res;

  switch ( mOperator )
  {
    case QgsMeshCalcNode::opPLUS:
    case QgsMeshCalcNode::opMINUS:
    case QgsMeshCalcNode::opMUL:
    case QgsMeshCalcNode::opDIV:
    case QgsMeshCalcNode::opPOW:
    case QgsMeshCalcNode::opEQ:
    case QgsMeshCalcNode::opNE:
    case QgsMeshCalcNode::opGT:
    case QgsMeshCalcNode::opLT:
    case QgsMeshCalcNode::opGE:
    case QgsMeshCalcNode::opLE:
    case QgsMeshCalcNode::opAND:
    case QgsMeshCalcNode::opOR:
    case QgsMeshCalcNode::opNOT:
    case QgsMeshCalcNode::opIF:
    case QgsMeshCalcNode::opSIGN:
    case QgsMeshCalcNode::opMIN:
    case QgsMeshCalcNode::opMAX:
    case QgsMeshCalcNode::opABS:
    case QgsMeshCalcNode::opNONE:
      if ( mLeft )
      {
        res += mLeft->aggregatedUsedDatasetGroupNames();
      }

      if ( mRight )
      {
        res += mRight->aggregatedUsedDatasetGroupNames();
      }

      if ( mCondition )
      {
        res += mCondition->aggregatedUsedDatasetGroupNames();
      }
      break;
    case QgsMeshCalcNode::opSUM_AGGR:
    case QgsMeshCalcNode::opMAX_AGGR:
    case QgsMeshCalcNode::opMIN_AGGR:
    case QgsMeshCalcNode::opAVG_AGGR:
      if ( mLeft )
      {
        res += mLeft->usedDatasetGroupNames();
      }

      if ( mRight )
      {
        res += mRight->usedDatasetGroupNames();
      }

      if ( mCondition )
      {
        res += mCondition->usedDatasetGroupNames();
      }
      break;
  }

  return res;
}

QStringList QgsMeshCalcNode::notAggregatedUsedDatasetGroupNames() const
{
  QStringList res;

  if ( mType == tDatasetGroupRef )
  {
    res.append( mDatasetGroupName );
  }

  switch ( mOperator )
  {
    case QgsMeshCalcNode::opPLUS:
    case QgsMeshCalcNode::opMINUS:
    case QgsMeshCalcNode::opMUL:
    case QgsMeshCalcNode::opDIV:
    case QgsMeshCalcNode::opPOW:
    case QgsMeshCalcNode::opEQ:
    case QgsMeshCalcNode::opNE:
    case QgsMeshCalcNode::opGT:
    case QgsMeshCalcNode::opLT:
    case QgsMeshCalcNode::opGE:
    case QgsMeshCalcNode::opLE:
    case QgsMeshCalcNode::opAND:
    case QgsMeshCalcNode::opOR:
    case QgsMeshCalcNode::opNOT:
    case QgsMeshCalcNode::opIF:
    case QgsMeshCalcNode::opSIGN:
    case QgsMeshCalcNode::opMIN:
    case QgsMeshCalcNode::opMAX:
    case QgsMeshCalcNode::opABS:
      if ( mLeft )
      {
        res += mLeft->notAggregatedUsedDatasetGroupNames();
      }

      if ( mRight )
      {
        res += mRight->notAggregatedUsedDatasetGroupNames();
      }

      if ( mCondition )
      {
        res += mCondition->notAggregatedUsedDatasetGroupNames();
      }
      break;
    case QgsMeshCalcNode::opSUM_AGGR:
    case QgsMeshCalcNode::opMAX_AGGR:
    case QgsMeshCalcNode::opMIN_AGGR:
    case QgsMeshCalcNode::opAVG_AGGR:
    case QgsMeshCalcNode::opNONE:
      break;
  }

  return res;
}

bool QgsMeshCalcNode::calculate( const  QgsMeshCalcUtils &dsu, QgsMeshMemoryDatasetGroup &result, bool isAggregate ) const
{
  if ( mType == tDatasetGroupRef )
  {
    dsu.copy( result, mDatasetGroupName, isAggregate );
    return true;
  }
  else if ( mType == tOperator )
  {
    QgsMeshMemoryDatasetGroup leftDatasetGroup( "left", dsu.outputType() );
    QgsMeshMemoryDatasetGroup rightDatasetGroup( "right", dsu.outputType() );

    bool currentOperatorIsAggregate = mOperator == opSUM_AGGR ||
                                      mOperator == opMAX_AGGR ||
                                      mOperator == opMIN_AGGR ||
                                      mOperator == opAVG_AGGR;

    if ( !mLeft || !mLeft->calculate( dsu, leftDatasetGroup, isAggregate || currentOperatorIsAggregate ) )
    {
      return false;
    }
    if ( mRight && !mRight->calculate( dsu, rightDatasetGroup, isAggregate || currentOperatorIsAggregate ) )
    {
      return false;
    }

    QgsMeshMemoryDatasetGroup condition( "condition", dsu.outputType() );
    switch ( mOperator )
    {
      case opIF:
        // Evaluate boolean condition
        if ( !mCondition->calculate( dsu, condition ) )
        {
          // invalid boolean condition
          return false;
        }
        dsu.addIf( leftDatasetGroup, rightDatasetGroup, condition );
        break;

      case opPLUS:
        dsu.add( leftDatasetGroup, rightDatasetGroup );
        break;
      case opMINUS:
        dsu.subtract( leftDatasetGroup, rightDatasetGroup );
        break;
      case opMUL:
        dsu.multiply( leftDatasetGroup, rightDatasetGroup );
        break;
      case opDIV:
        dsu.divide( leftDatasetGroup, rightDatasetGroup );
        break;
      case opPOW:
        dsu.power( leftDatasetGroup, rightDatasetGroup );
        break;
      case opEQ:
        dsu.equal( leftDatasetGroup, rightDatasetGroup );
        break;
      case opNE:
        dsu.notEqual( leftDatasetGroup, rightDatasetGroup );
        break;
      case opGT:
        dsu.greaterThan( leftDatasetGroup, rightDatasetGroup );
        break;
      case opLT:
        dsu.lesserThan( leftDatasetGroup, rightDatasetGroup );
        break;
      case opGE:
        dsu.greaterEqual( leftDatasetGroup, rightDatasetGroup );
        break;
      case opLE:
        dsu.lesserEqual( leftDatasetGroup, rightDatasetGroup );
        break;
      case opAND:
        dsu.logicalAnd( leftDatasetGroup, rightDatasetGroup );
        break;
      case opOR:
        dsu.logicalOr( leftDatasetGroup, rightDatasetGroup );
        break;
      case opNOT:
        dsu.logicalNot( leftDatasetGroup );
        break;
      case opMIN:
        dsu.minimum( leftDatasetGroup, rightDatasetGroup );
        break;
      case opMAX:
        dsu.maximum( leftDatasetGroup, rightDatasetGroup );
        break;
      case opABS:
        dsu.abs( leftDatasetGroup );
        break;
      case opSUM_AGGR:
        dsu.sumAggregated( leftDatasetGroup );
        break;
      case opMIN_AGGR:
        dsu.minimumAggregated( leftDatasetGroup );
        break;
      case opMAX_AGGR:
        dsu.maximumAggregated( leftDatasetGroup );
        break;
      case opAVG_AGGR:
        dsu.averageAggregated( leftDatasetGroup );
        break;
      case opSIGN:
        dsu.changeSign( leftDatasetGroup );
        break;
      default:
        return false;
    }
    dsu.transferDatasets( result, leftDatasetGroup );
    return true;
  }
  else if ( mType == tNumber )
  {
    dsu.number( result, mNumber );
    return true;
  }
  else if ( mType == tNoData )
  {
    dsu.nodata( result );
    return true;
  }

  // invalid type
  return false;
}

QgsMeshCalcNode *QgsMeshCalcNode::parseMeshCalcString( const QString &str, QString &parserErrorMsg )
{
  extern QgsMeshCalcNode *localParseMeshCalcString( const QString & str, QString & parserErrorMsg );
  return localParseMeshCalcString( str, parserErrorMsg );
}

bool QgsMeshCalcNode::isNonTemporal() const
{
  if ( mType == tNoData || mType == tNumber )
    return true;

  if ( mType == tDatasetGroupRef )
    return false;

  switch ( mOperator )
  {
    case QgsMeshCalcNode::opIF:
      return ( mLeft && mLeft->isNonTemporal() ) &&
             ( mRight && mRight->isNonTemporal() &&
               mCondition->isNonTemporal() );
      break;
    case QgsMeshCalcNode::opPLUS:
    case QgsMeshCalcNode::opMINUS:
    case QgsMeshCalcNode::opMUL:
    case QgsMeshCalcNode::opDIV:
    case QgsMeshCalcNode::opPOW:
    case QgsMeshCalcNode::opEQ:
    case QgsMeshCalcNode::opNE:
    case QgsMeshCalcNode::opGT:
    case QgsMeshCalcNode::opLT:
    case QgsMeshCalcNode::opGE:
    case QgsMeshCalcNode::opLE:
    case QgsMeshCalcNode::opAND:
    case QgsMeshCalcNode::opOR:
    case QgsMeshCalcNode::opNOT:
    case QgsMeshCalcNode::opSIGN:
    case QgsMeshCalcNode::opMIN:
    case QgsMeshCalcNode::opMAX:
    case QgsMeshCalcNode::opABS:
      return ( mLeft && mLeft->isNonTemporal() ) &&
             ( mRight && mRight->isNonTemporal() );
      break;
    case QgsMeshCalcNode::opSUM_AGGR:
    case QgsMeshCalcNode::opMAX_AGGR:
    case QgsMeshCalcNode::opMIN_AGGR:
    case QgsMeshCalcNode::opAVG_AGGR:
    case QgsMeshCalcNode::opNONE:
      return true;
      break;
  }

  return true;
}

///@endcond
