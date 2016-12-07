/***************************************************************************
 testqgsscaleexpression.cpp
 --------------------------
 begin                : November 2014
 copyright            : (C) 2014 by Vincent Mora
 email                : vincent dor mora at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QString>

#include "qgsscaleexpression.h"
#include "qgsapplication.h"

class TestQgsScaleExpression: public QObject
{
    Q_OBJECT
  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();
    }
    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void parsing()
    {
      {
        QgsScaleExpression exp( QStringLiteral( "coalesce(scale_linear(column, 1, 7, 2, 10), 0)" ) );
        QCOMPARE( bool( exp ), true );
        QCOMPARE( exp.type(), QgsScaleExpression::Linear );
        QCOMPARE( exp.baseExpression(), QString( "column" ) );
        QCOMPARE( exp.minValue(), 1. );
        QCOMPARE( exp.maxValue(), 7. );
        QCOMPARE( exp.minSize(), 2. );
        QCOMPARE( exp.maxSize(), 10. );
      }
      {
        QgsScaleExpression exp( QStringLiteral( "coalesce(scale_exp(column, 1, 7, 2, 10, 0.5), 0)" ) );
        QCOMPARE( bool( exp ), true );
        QCOMPARE( exp.type(), QgsScaleExpression::Area );
      }
      {
        QgsScaleExpression exp( QStringLiteral( "coalesce(scale_exp(column, 1, 7, 2, 10, 0.57), 0)" ) );
        QCOMPARE( bool( exp ), true );
        QCOMPARE( exp.type(), QgsScaleExpression::Flannery );
      }
      {
        QgsScaleExpression exp( QStringLiteral( "scale_linear(column, 1, 7, 2, 10)" ) );
        QCOMPARE( bool( exp ), true );
        QCOMPARE( exp.type(), QgsScaleExpression::Linear );
        QCOMPARE( exp.baseExpression(), QString( "column" ) );
        QCOMPARE( exp.minValue(), 1. );
        QCOMPARE( exp.maxValue(), 7. );
        QCOMPARE( exp.minSize(), 2. );
        QCOMPARE( exp.maxSize(), 10. );
      }
      {
        QgsScaleExpression exp( QStringLiteral( "scale_exp(column, 1, 7, 2, 10, 0.5)" ) );
        QCOMPARE( bool( exp ), true );
        QCOMPARE( exp.type(), QgsScaleExpression::Area );
      }
      {
        QgsScaleExpression exp( QStringLiteral( "scale_exp(column, 1, 7, 2, 10, 0.57)" ) );
        QCOMPARE( bool( exp ), true );
        QCOMPARE( exp.type(), QgsScaleExpression::Flannery );
      }
      {
        QgsScaleExpression exp( QStringLiteral( "coalesce(scale_exp(column, 1, 7, 2, 10, 0.51), 0)" ) );
        QCOMPARE( bool( exp ), true );
        QCOMPARE( exp.type(), QgsScaleExpression::Exponential );
      }
      {
        QgsScaleExpression exp( QStringLiteral( "coalesce(scale_exp(column, 1, 7, a, 10, 0.5), 0)" ) );
        QCOMPARE( bool( exp ), false );
        QCOMPARE( exp.type(), QgsScaleExpression::Unknown );
      }
      {
        QgsScaleExpression exp( QStringLiteral( "coalesce(scale_exp(column, 1, 7), 0)" ) );
        QCOMPARE( bool( exp ), false );
        QCOMPARE( exp.type(), QgsScaleExpression::Unknown );
      }
      {
        QgsScaleExpression exp( QgsScaleExpression::Linear, QStringLiteral( "column" ), 1, 7, 2, 10 );
        QCOMPARE( bool( exp ), true );
        QCOMPARE( exp.type(), QgsScaleExpression::Linear );
        QCOMPARE( exp.baseExpression(), QString( "column" ) );
        QCOMPARE( exp.minValue(), 1. );
        QCOMPARE( exp.maxValue(), 7. );
        QCOMPARE( exp.minSize(), 2. );
        QCOMPARE( exp.maxSize(), 10. );
      }

    }
};

QGSTEST_MAIN( TestQgsScaleExpression )

#include "testqgsscaleexpression.moc"
