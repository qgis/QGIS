
/***************************************************************************
 qgsscaleexpression.h
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
#ifndef QGSSCALEEXPRESSION_H
#define QGSSCALEEXPRESSION_H

#include "qgsexpression.h"

//! Class to retrieve parameters of a
//! scale expression (size or width)
//!
//! It derives from QgsExpression,
class QgsScaleExpression: public QgsExpression
{
public:
  enum Type {Linear, Area, Flannery};

  //! parse string like an expression and
  //! detemine if it's a size expression
  QgsScaleExpression( const QString & expr );

  //! setup expression from parameters
  QgsScaleExpression( Type type, const QString & baseExpr, double minValue, double maxValue, double minSize, double maxSize );

  operator bool() const { return ! mExpression.isEmpty(); }
  double size( double value ) const;

  double minSize() const { return mMinSize; }
  double maxSize() const { return mMaxSize; }
  double minValue() const { return mMinValue; }
  double maxValue() const { return mMaxValue; }
  QString baseExpression() const { return mExpression; }
  Type type() const { return mType; }

private:
  QString mExpression;
  Type mType;
  double mMinSize;
  double mMaxSize;
  double mMinValue;
  double mMaxValue;

  void init();
  static QString createExpression( Type type, const QString & baseExpr, double minValue, double maxValue, double minSize, double maxSize );

};

#endif



