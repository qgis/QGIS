
/***************************************************************************
 qgsscaleexpression.cpp
 ---------------------
 begin                : November 2015
 copyright            : (C) 2015 by Vincent Mora
 email                : vincent dor mora at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsscaleexpression.h"

#include <QStringList>
#include <qmath.h>

void QgsScaleExpression::init()
{
  bool ok;
  if ( rootNode() )
  {
    const NodeFunction * f = dynamic_cast<const NodeFunction*>( rootNode() );
    if ( f )
    {
      QList<Node*> args = f->args()->list();
      if ( "scale_linear" == Functions()[f->fnIndex()]->name() )
      {
        mType = Linear;
      }
      if ( "scale_exp" == Functions()[f->fnIndex()]->name() )
      {
        const double exp = QgsExpression( args[5]->dump() ).evaluate().toDouble( &ok );
        if ( ! ok ) return;
        if ( qAbs( exp - .57 ) < .001 ) mType = Flannery;
        else if ( qAbs( exp - .5 ) < .001 ) mType = Area;
        else return;
      }
      mMinValue = QgsExpression( args[1]->dump() ).evaluate().toDouble( &ok );
      if ( ! ok ) return;
      mMaxValue = QgsExpression( args[2]->dump() ).evaluate().toDouble( &ok );
      if ( ! ok ) return;
      mMinSize = QgsExpression( args[3]->dump() ).evaluate().toDouble( &ok );
      if ( ! ok ) return;
      mMaxSize = QgsExpression( args[4]->dump() ).evaluate().toDouble( &ok );
      if ( ! ok ) return;
      mExpression = args[0]->dump();
    }
  }
}

QgsScaleExpression::QgsScaleExpression( const QString & expr )
    : QgsExpression( expr )
{
  init();
}

QgsScaleExpression::QgsScaleExpression( Type type, const QString & baseExpr, double minValue, double maxValue, double minSize, double maxSize )
    : QgsExpression( createExpression( type, baseExpr, minValue, maxValue, minSize, maxSize ) )
{
  init();
}

QString QgsScaleExpression::createExpression( Type type, const QString & baseExpr, double minValue, double maxValue, double minSize, double maxSize )
{
  switch ( type )
  {
    case Linear:
      return "scale_linear(" + baseExpr
             + ", " + QString::number( minValue )
             + ", " + QString::number( maxValue )
             + ", " + QString::number( minSize )
             + ", " + QString::number( maxSize ) + ")";
    case Area:
      return "scale_exp(" + baseExpr
             + ", " + QString::number( minValue )
             + ", " + QString::number( maxValue )
             + ", " + QString::number( minSize )
             + ", " + QString::number( maxSize ) + ", .5)";
    case Flannery:
      return "scale_exp(" + baseExpr
             + ", " + QString::number( minValue )
             + ", " + QString::number( maxValue )
             + ", " + QString::number( minSize )
             + ", " + QString::number( maxSize ) + ", .57)";
  }
  return "";
}


double QgsScaleExpression::size( double value ) const
{
  switch ( mType )
  {
    case Linear: return mMinSize + ( qBound( mMinValue, value, mMaxValue ) - mMinValue ) * ( mMaxSize - mMinSize ) / ( mMaxValue - mMinValue );
    case Area: return qPow( qBound( mMinValue, value, mMaxValue ) - mMinValue, .5 ) * ( mMaxSize - mMinSize ) / qPow( mMaxValue - mMinValue, .5 );
    case Flannery: return qPow( qBound( mMinValue, value, mMaxValue ) - mMinValue, .57 ) * ( mMaxSize - mMinSize ) / qPow( mMaxValue - mMinValue, .57 );
  }
  return 0;
}
