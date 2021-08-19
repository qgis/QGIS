/***************************************************************************

 testqgspostgresexpressioncompiler.cpp

 ---------------------
 begin                : 23.07.2021
 copyright            : (C) 2021 by Marco Hugentobler
 email                : marco at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgspostgresexpressioncompiler.h"
#include "qgspostgresfeatureiterator.h"
#include "qgspostgresprovider.h"

//The only purpose of this class is to set geomColumn and srid
class QgsTestPostgresExpressionCompiler: public QgsPostgresExpressionCompiler
{
  public:
    QgsTestPostgresExpressionCompiler( QgsPostgresFeatureSource *source, const QString &srid, const QString &geometryColumn ): QgsPostgresExpressionCompiler( source )
    {
      mDetectedSrid = srid;
      mGeometryColumn = geometryColumn;
    }
};

class TestQgsPostgresExpressionCompiler: public QObject
{
    Q_OBJECT

  public:

    TestQgsPostgresExpressionCompiler() = default;

  private slots:
    void testGeometryFromWkt();
};

void TestQgsPostgresExpressionCompiler::testGeometryFromWkt()
{
  const QgsPostgresProvider p( QStringLiteral( "" ), QgsDataProvider::ProviderOptions() );
  QgsPostgresFeatureSource featureSource( &p );
  QgsTestPostgresExpressionCompiler compiler( &featureSource, QStringLiteral( "4326" ), QStringLiteral( "geom" ) );
  QgsExpression exp( QStringLiteral( "intersects($geometry,geom_from_wkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))'))" ) );
  const QgsExpressionContext expContext;
  exp.prepare( &expContext );
  const QgsSqlExpressionCompiler::Result r = compiler.compile( &exp );
  QCOMPARE( r, QgsSqlExpressionCompiler::Complete );
  const QString sql = compiler.result();
  QCOMPARE( sql, QStringLiteral( "ST_Intersects(\"geom\",ST_GeomFromText('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))',4326))" ) );
}

QGSTEST_MAIN( TestQgsPostgresExpressionCompiler )

#include "testqgspostgresexpressioncompiler.moc"
