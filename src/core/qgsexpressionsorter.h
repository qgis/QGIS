/***************************************************************************
  qgsexpressionsorter.h - QgsExpressionSorter
  -------------------------------------------

 begin                : 15.1.2016
 Copyright            : (C) 2016 Matthias Kuhn
 Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEXPRESSIONSORTER_H
#define QGSEXPRESSIONSORTER_H

#include <QLocale>

#include "qgsfeaturerequest.h"
#include "qgsindexedfeature.h"

/// @cond PRIVATE
class QgsExpressionSorter
{
  public:
    explicit QgsExpressionSorter( const QList<QgsFeatureRequest::OrderByClause> &preparedOrderBys )
      : mPreparedOrderBys( preparedOrderBys )
        // QString::localeAwareCompare() is case insensitive for common locales,
        // but case sensitive for the C locale. So use an explicit case
        // insensitive comparison in that later case to avoid test failures.
      , mUseCaseInsensitiveComparison( QLocale().name() == QLocale::c().name() )
    {}

    bool operator()( const QgsIndexedFeature &f1, const QgsIndexedFeature &f2 ) const
    {
      int i = 0;
      for ( const QgsFeatureRequest::OrderByClause &orderBy : std::as_const( mPreparedOrderBys ) )
      {
        const QVariant &v1 = f1.mIndexes.at( i );
        const QVariant &v2 = f2.mIndexes.at( i );
        ++i;

        // Both NULL: don't care
        if ( v1.isNull() && v2.isNull() )
          continue;

        // Check for NULLs first
        if ( v1.isNull() != v2.isNull() )
        {
          if ( orderBy.nullsFirst() )
            return v1.isNull();
          else
            return !v1.isNull();
        }

        // Both values are not NULL
        switch ( v1.type() )
        {
          case QVariant::Int:
          case QVariant::UInt:
          case QVariant::LongLong:
          case QVariant::ULongLong:
            if ( v1.toLongLong() == v2.toLongLong() )
              continue;
            if ( orderBy.ascending() )
              return v1.toLongLong() < v2.toLongLong();
            else
              return v1.toLongLong() > v2.toLongLong();

          case QVariant::Double:
            if ( qgsDoubleNear( v1.toDouble(), v2.toDouble() ) )
              continue;
            if ( orderBy.ascending() )
              return v1.toDouble() < v2.toDouble();
            else
              return v1.toDouble() > v2.toDouble();

          case QVariant::Date:
            if ( v1.toDate() == v2.toDate() )
              continue;
            if ( orderBy.ascending() )
              return v1.toDate() < v2.toDate();
            else
              return v1.toDate() > v2.toDate();

          case QVariant::Time:
            if ( v1.toTime() == v2.toTime() )
              continue;
            if ( orderBy.ascending() )
              return v1.toTime() < v2.toTime();
            else
              return v1.toTime() > v2.toTime();

          case QVariant::DateTime:
            if ( v1.toDateTime() == v2.toDateTime() )
              continue;
            if ( orderBy.ascending() )
              return v1.toDateTime() < v2.toDateTime();
            else
              return v1.toDateTime() > v2.toDateTime();

          case QVariant::Bool:
            if ( v1.toBool() == v2.toBool() )
              continue;
            if ( orderBy.ascending() )
              return !v1.toBool();
            else
              return v1.toBool();

          default:
            if ( 0 == v1.toString().localeAwareCompare( v2.toString() ) )
              continue;
            if ( mUseCaseInsensitiveComparison )
            {
              if ( orderBy.ascending() )
                return v1.toString().compare( v2.toString(), Qt::CaseInsensitive ) < 0;
              else
                return v1.toString().compare( v2.toString(), Qt::CaseInsensitive ) > 0;
            }
            else
            {
              if ( orderBy.ascending() )
                return v1.toString().localeAwareCompare( v2.toString() ) < 0;
              else
                return v1.toString().localeAwareCompare( v2.toString() ) > 0;
            }
        }
      }

      // Equal
      return false;
    }

    void sortFeatures( QList<QgsFeature> &features, QgsExpressionContext *expressionContext )
    {
      QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Expression Sorter" ) );

      expressionContext->appendScope( scope );

      QVector<QgsIndexedFeature> indexedFeatures;

      QgsIndexedFeature indexedFeatureToAppend;

      for ( const QgsFeature &f : std::as_const( features ) )
      {
        indexedFeatureToAppend.mIndexes.resize( mPreparedOrderBys.size() );
        indexedFeatureToAppend.mFeature = f;

        expressionContext->setFeature( indexedFeatureToAppend.mFeature );

        int i = 0;
        for ( const QgsFeatureRequest::OrderByClause &orderBy : std::as_const( mPreparedOrderBys ) )
        {
          indexedFeatureToAppend.mIndexes.replace( i++, orderBy.expression().evaluate( expressionContext ) );
        }
        indexedFeatures.append( indexedFeatureToAppend );
      }

      delete expressionContext->popScope();

      std::sort( indexedFeatures.begin(), indexedFeatures.end(), *this );

      features.clear();

      for ( const QgsIndexedFeature &indexedFeature : std::as_const( indexedFeatures ) )
        features.append( indexedFeature.mFeature );
    }

  private:
    QList<QgsFeatureRequest::OrderByClause> mPreparedOrderBys;
    bool mUseCaseInsensitiveComparison;
};

/// @endcond


#endif // QGSEXPRESSIONSORTER_H
