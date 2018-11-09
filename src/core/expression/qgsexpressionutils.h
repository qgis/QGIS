/***************************************************************************
                               qgsexpressionutils.h
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSEXPRESSIONUTILS_H
#define QGSEXPRESSIONUTILS_H

#define SIP_NO_FILE

#include "qgsfeature.h"
#include "qgsexpression.h"
#include "qgscolorramp.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsrasterlayer.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"

#define ENSURE_NO_EVAL_ERROR   {  if ( parent->hasEvalError() ) return QVariant(); }
#define SET_EVAL_ERROR(x)   { parent->setEvalErrorString( x ); return QVariant(); }

#define FEAT_FROM_CONTEXT(c, f) if ( !(c) || !( c )->hasFeature() ) return QVariant(); \
  QgsFeature f = ( c )->feature();

///////////////////////////////////////////////
// three-value logic

/// @cond PRIVATE
class QgsExpressionUtils
{
  public:
    enum TVL
    {
      False,
      True,
      Unknown
    };


    static TVL AND[3][3];

    static TVL OR[3][3];

    static TVL NOT[3];

#define TVL_True QVariant( 1 )
#define TVL_False QVariant( 0 )
#define TVL_Unknown QVariant()

    static QVariant tvl2variant( TVL v )
    {
      switch ( v )
      {
        case False:
          return TVL_False;
        case True:
          return TVL_True;
        case Unknown:
        default:
          return TVL_Unknown;
      }
    }

// this handles also NULL values
    static TVL getTVLValue( const QVariant &value, QgsExpression *parent )
    {
      // we need to convert to TVL
      if ( value.isNull() )
        return Unknown;

      //handle some special cases
      if ( value.canConvert<QgsGeometry>() )
      {
        //geom is false if empty
        QgsGeometry geom = value.value<QgsGeometry>();
        return geom.isNull() ? False : True;
      }
      else if ( value.canConvert<QgsFeature>() )
      {
        //feat is false if non-valid
        QgsFeature feat = value.value<QgsFeature>();
        return feat.isValid() ? True : False;
      }

      if ( value.type() == QVariant::Int )
        return value.toInt() != 0 ? True : False;

      bool ok;
      double x = value.toDouble( &ok );
      if ( !ok )
      {
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to boolean" ).arg( value.toString() ) );
        return Unknown;
      }
      return !qgsDoubleNear( x, 0.0 ) ? True : False;
    }


    static inline bool isIntSafe( const QVariant &v )
    {
      if ( v.type() == QVariant::Int )
        return true;
      if ( v.type() == QVariant::UInt )
        return true;
      if ( v.type() == QVariant::LongLong )
        return true;
      if ( v.type() == QVariant::ULongLong )
        return true;
      if ( v.type() == QVariant::Double )
        return false;
      if ( v.type() == QVariant::String )
      {
        bool ok;
        v.toString().toInt( &ok );
        return ok;
      }
      return false;
    }

    static inline bool isDoubleSafe( const QVariant &v )
    {
      if ( v.type() == QVariant::Double )
        return true;
      if ( v.type() == QVariant::Int )
        return true;
      if ( v.type() == QVariant::UInt )
        return true;
      if ( v.type() == QVariant::LongLong )
        return true;
      if ( v.type() == QVariant::ULongLong )
        return true;
      if ( v.type() == QVariant::String )
      {
        bool ok;
        double val = v.toString().toDouble( &ok );
        ok = ok && std::isfinite( val ) && !std::isnan( val );
        return ok;
      }
      return false;
    }

    static inline bool isDateTimeSafe( const QVariant &v )
    {
      return v.type() == QVariant::DateTime
             || v.type() == QVariant::Date
             || v.type() == QVariant::Time;
    }

    static inline bool isIntervalSafe( const QVariant &v )
    {
      if ( v.canConvert<QgsInterval>() )
      {
        return true;
      }

      if ( v.type() == QVariant::String )
      {
        return QgsInterval::fromString( v.toString() ).isValid();
      }
      return false;
    }

    static inline bool isNull( const QVariant &v )
    {
      return v.isNull();
    }

    static inline bool isList( const QVariant &v )
    {
      return v.type() == QVariant::List;
    }

// implicit conversion to string
    static QString getStringValue( const QVariant &value, QgsExpression * )
    {
      return value.toString();
    }

    static double getDoubleValue( const QVariant &value, QgsExpression *parent )
    {
      bool ok;
      double x = value.toDouble( &ok );
      if ( !ok || std::isnan( x ) || !std::isfinite( x ) )
      {
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to double" ).arg( value.toString() ) );
        return 0;
      }
      return x;
    }

    static qlonglong getIntValue( const QVariant &value, QgsExpression *parent )
    {
      bool ok;
      qlonglong x = value.toLongLong( &ok );
      if ( ok )
      {
        return x;
      }
      else
      {
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to int" ).arg( value.toString() ) );
        return 0;
      }
    }

    static int getNativeIntValue( const QVariant &value, QgsExpression *parent )
    {
      bool ok;
      qlonglong x = value.toLongLong( &ok );
      if ( ok && x >= std::numeric_limits<int>::min() && x <= std::numeric_limits<int>::max() )
      {
        return x;
      }
      else
      {
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to native int" ).arg( value.toString() ) );
        return 0;
      }
    }

    static QDateTime getDateTimeValue( const QVariant &value, QgsExpression *parent )
    {
      QDateTime d = value.toDateTime();
      if ( d.isValid() )
      {
        return d;
      }
      else
      {
        QTime t = value.toTime();
        if ( t.isValid() )
        {
          return QDateTime( QDate( 1, 1, 1 ), t );
        }

        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to DateTime" ).arg( value.toString() ) );
        return QDateTime();
      }
    }

    static QDate getDateValue( const QVariant &value, QgsExpression *parent )
    {
      QDate d = value.toDate();
      if ( d.isValid() )
      {
        return d;
      }
      else
      {
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to Date" ).arg( value.toString() ) );
        return QDate();
      }
    }

    static QTime getTimeValue( const QVariant &value, QgsExpression *parent )
    {
      QTime t = value.toTime();
      if ( t.isValid() )
      {
        return t;
      }
      else
      {
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to Time" ).arg( value.toString() ) );
        return QTime();
      }
    }

    static QgsInterval getInterval( const QVariant &value, QgsExpression *parent, bool report_error = false )
    {
      if ( value.canConvert<QgsInterval>() )
        return value.value<QgsInterval>();

      QgsInterval inter = QgsInterval::fromString( value.toString() );
      if ( inter.isValid() )
      {
        return inter;
      }
      // If we get here then we can't convert so we just error and return invalid.
      if ( report_error )
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to interval" ).arg( value.toString() ) );

      return QgsInterval();
    }

    static QgsGradientColorRamp getRamp( const QVariant &value, QgsExpression *parent, bool report_error = false )
    {
      if ( value.canConvert<QgsGradientColorRamp>() )
        return value.value<QgsGradientColorRamp>();

      // If we get here then we can't convert so we just error and return invalid.
      if ( report_error )
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to gradient ramp" ).arg( value.toString() ) );

      return QgsGradientColorRamp();
    }

    static QgsGeometry getGeometry( const QVariant &value, QgsExpression *parent )
    {
      if ( value.canConvert<QgsGeometry>() )
        return value.value<QgsGeometry>();

      parent->setEvalErrorString( QStringLiteral( "Cannot convert to geometry" ) );
      return QgsGeometry();
    }

    static QgsFeature getFeature( const QVariant &value, QgsExpression *parent )
    {
      if ( value.canConvert<QgsFeature>() )
        return value.value<QgsFeature>();

      parent->setEvalErrorString( QStringLiteral( "Cannot convert to feature" ) );
      return 0;
    }

    static QgsExpressionNode *getNode( const QVariant &value, QgsExpression *parent )
    {
      if ( value.canConvert<QgsExpressionNode *>() )
        return value.value<QgsExpressionNode *>();

      parent->setEvalErrorString( QStringLiteral( "Cannot convert to node" ) );
      return nullptr;
    }

    static QgsMapLayer *getMapLayer( const QVariant &value, QgsExpression * )
    {
      // First check if we already received a layer pointer
      QgsMapLayer *ml = value.value< QgsWeakMapLayerPointer >().data();
      QgsProject *project = QgsProject::instance();

      // No pointer yet, maybe it's a layer id?
      if ( !ml )
        ml = project->mapLayer( value.toString() );

      // Still nothing? Check for layer name
      if ( !ml )
        ml = project->mapLayersByName( value.toString() ).value( 0 );

      return ml;
    }

    static std::unique_ptr<QgsVectorLayerFeatureSource> getFeatureSource( const QVariant &value, QgsExpression *e )
    {
      std::unique_ptr<QgsVectorLayerFeatureSource> featureSource;

      auto getFeatureSource = [ &value, e, &featureSource ]
      {
        QgsVectorLayer *layer = getVectorLayer( value, e );

        if ( layer )
        {
          featureSource.reset( new QgsVectorLayerFeatureSource( layer ) );
        }
      };

#if QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 )
      // Make sure we only deal with the vector layer on the main thread where it lives.
      // Anything else risks a crash.
      if ( QThread::currentThread() == qApp->thread() )
        getFeatureSource();
      else
        QMetaObject::invokeMethod( qApp, getFeatureSource, Qt::BlockingQueuedConnection );
#else
      getFeatureSource();
#endif

      return featureSource;
    }

    static QgsVectorLayer *getVectorLayer( const QVariant &value, QgsExpression *e )
    {
      return qobject_cast<QgsVectorLayer *>( getMapLayer( value, e ) );
    }

    static QgsRasterLayer *getRasterLayer( const QVariant &value, QgsExpression *e )
    {
      return qobject_cast<QgsRasterLayer *>( getMapLayer( value, e ) );
    }

    static QVariantList getListValue( const QVariant &value, QgsExpression *parent )
    {
      if ( value.type() == QVariant::List || value.type() == QVariant::StringList )
      {
        return value.toList();
      }
      else
      {
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to array" ).arg( value.toString() ) );
        return QVariantList();
      }
    }

    static QVariantMap getMapValue( const QVariant &value, QgsExpression *parent )
    {
      if ( value.type() == QVariant::Map )
      {
        return value.toMap();
      }
      else
      {
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to map" ).arg( value.toString() ) );
        return QVariantMap();
      }
    }
};

/// @endcond

#endif // QGSEXPRESSIONUTILS_H
