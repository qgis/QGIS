/***************************************************************************
    qgsdatadefined.cpp - Data defined container class
     --------------------------------------
    Date                 : 9-May-2013
    Copyright            : (C) 2013 by Larry Shaffer
    Email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatadefined.h"

#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsfield.h"
#include "qgsvectorlayer.h"

QgsDataDefined::QgsDataDefined( bool active,
                                bool useexpr,
                                const QString& expr,
                                const QString& field ) :
    mActive( active )
    , mUseExpression( useexpr )
    , mExpressionString( expr )
    , mField( field )
{
  mExpression = 0;
  mExpressionPrepared = false;
}

QgsDataDefined::~QgsDataDefined()
{
  mExpressionParams.clear();
  delete mExpression;
}

void QgsDataDefined::setExpressionString( const QString &expr )
{
  mExpressionString = expr;
  mExpressionPrepared = false;
}

bool QgsDataDefined::prepareExpression( QgsVectorLayer* layer )
{
  if ( !mUseExpression || mExpressionString.isEmpty() )
  {
    return false;
  }

  mExpression = new QgsExpression( mExpressionString );
  if ( mExpression->hasParserError() )
  {
    QgsDebugMsg( "Parser error:" + mExpression->parserErrorString() );
    return false;
  }

  // setup expression parameters
  QVariant scaleV = mExpressionParams.value( "scale" );
  if ( scaleV.isValid() )
  {
    bool ok;
    double scale = scaleV.toDouble( &ok );
    if ( ok )
    {
      mExpression->setScale( scale );
    }
  }

  if ( layer )
  {
    mExpression->prepare( layer->pendingFields() );
  }
  else
  {
    //preparing expression without a layer set, so pass empty field list
    QgsFields empty;
    mExpression->prepare( empty );
  }

  if ( mExpression->hasEvalError() )
  {
    QgsDebugMsg( "Prepare error:" + mExpression->evalErrorString() );
    return false;
  }

  mExpressionPrepared = true;
  mExprRefColmuns = mExpression->referencedColumns();

  return true;
}

QStringList QgsDataDefined::referencedColumns( QgsVectorLayer* layer )
{
  if ( !mExprRefColmuns.isEmpty() )
  {
    return mExprRefColmuns;
  }

  if ( mUseExpression )
  {
    if ( !mExpression || !mExpressionPrepared )
    {
      prepareExpression( layer );
    }
  }
  else if ( !mField.isEmpty() )
  {
    mExprRefColmuns << mField;
  }

  return mExprRefColmuns;
}

void QgsDataDefined::insertExpressionParam( QString key, QVariant param )
{
  mExpressionParams.insert( key, param );
}

QMap< QString, QString > QgsDataDefined::toMap()
{
  QMap< QString, QString > map;
  map.insert( "active", ( mActive ? "1" : "0" ) );
  map.insert( "useexpr", ( mUseExpression ? "1" : "0" ) );
  map.insert( "expression", mExpressionString );
  map.insert( "field", mField );

  return map;
}
