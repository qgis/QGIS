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
#include "qgsvariantutils.h"
#include "qgsfeaturerequest.h"
#include "qgsreferencedgeometry.h"

#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QThread>
#include <QLocale>
#include <functional>

class QgsMapLayer;
class QgsGradientColorRamp;
class QgsVectorLayerFeatureSource;

#define ENSURE_NO_EVAL_ERROR   {  if ( parent->hasEvalError() ) return QVariant(); }
#define SET_EVAL_ERROR(x)   { parent->setEvalErrorString( x ); return QVariant(); }

#define FEAT_FROM_CONTEXT(c, f) if ( !(c) || !( c )->hasFeature() ) return QVariant(); \
  QgsFeature f = ( c )->feature();

/**
 * \ingroup core
 * \class QgsExpressionUtils
 * \brief A set of expression-related functions
 * \since QGIS 3.22
 */

class CORE_EXPORT QgsExpressionUtils
{
  public:
/// @cond PRIVATE
///////////////////////////////////////////////
// three-value logic
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
      if ( QgsVariantUtils::isNull( value ) )
        return Unknown;

      //handle some special cases
      int userType = value.userType();
      if ( value.type() == QVariant::UserType )
      {
        if ( userType == qMetaTypeId< QgsGeometry>() || userType == qMetaTypeId<QgsReferencedGeometry>() )
        {
          //geom is false if empty
          const QgsGeometry geom = getGeometry( value, nullptr );
          return geom.isNull() ? False : True;
        }
        else if ( userType == qMetaTypeId<QgsFeature>() )
        {
          //feat is false if non-valid
          const QgsFeature feat = value.value<QgsFeature>();
          return feat.isValid() ? True : False;
        }
      }

      if ( userType == QMetaType::Type::Int )
        return value.toInt() != 0 ? True : False;

      bool ok;
      const double x = value.toDouble( &ok );
      if ( !ok )
      {
        if ( parent )
          parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to boolean" ).arg( value.toString() ) );
        return Unknown;
      }
      return !qgsDoubleNear( x, 0.0 ) ? True : False;
    }


    static inline bool isIntSafe( const QVariant &v )
    {
      if ( v.userType() == QMetaType::Type::Int )
        return true;
      if ( v.userType() == QMetaType::Type::UInt )
        return true;
      if ( v.userType() == QMetaType::Type::LongLong )
        return true;
      if ( v.userType() == QMetaType::Type::ULongLong )
        return true;
      if ( v.userType() == QMetaType::Type::Double )
        return false;
      if ( v.userType() == QMetaType::Type::QString )
      {
        bool ok;
        v.toString().toInt( &ok );
        return ok;
      }
      return false;
    }

    static inline bool isDoubleSafe( const QVariant &v )
    {
      if ( v.userType() == QMetaType::Type::Double )
        return true;
      if ( v.userType() == QMetaType::Type::Int )
        return true;
      if ( v.userType() == QMetaType::Type::UInt )
        return true;
      if ( v.userType() == QMetaType::Type::LongLong )
        return true;
      if ( v.userType() == QMetaType::Type::ULongLong )
        return true;
      if ( v.userType() == QMetaType::Type::QString )
      {
        bool ok;
        const double val = v.toString().toDouble( &ok );
        ok = ok && std::isfinite( val ) && !std::isnan( val );
        return ok;
      }
      return false;
    }

    static inline bool isDateTimeSafe( const QVariant &v )
    {
      return v.userType() == QMetaType::Type::QDateTime
             || v.userType() == QMetaType::Type::QDate
             || v.userType() == QMetaType::Type::QTime;
    }

    static inline bool isIntervalSafe( const QVariant &v )
    {
      if ( v.userType() == qMetaTypeId<QgsInterval>() )
      {
        return true;
      }

      if ( v.userType() == QMetaType::Type::QString )
      {
        return QgsInterval::fromString( v.toString() ).isValid();
      }
      return false;
    }

    static inline bool isNull( const QVariant &v )
    {
      return QgsVariantUtils::isNull( v );
    }

    static inline bool isList( const QVariant &v )
    {
      return v.userType() == QMetaType::Type::QVariantList || v.userType() == QMetaType::Type::QStringList;
    }

// implicit conversion to string
    static QString getStringValue( const QVariant &value, QgsExpression * )
    {
      return value.toString();
    }

    /**
     * Returns an expression value converted to binary (byte array) value.
     *
     * An empty byte array will be returned if the value is NULL.
     *
     * \since QGIS 3.12
     */
    static QByteArray getBinaryValue( const QVariant &value, QgsExpression *parent )
    {
      if ( value.userType() != QMetaType::Type::QByteArray )
      {
        if ( parent )
          parent->setEvalErrorString( QObject::tr( "Value is not a binary value" ) );
        return QByteArray();
      }
      return value.toByteArray();
    }

    static double getDoubleValue( const QVariant &value, QgsExpression *parent )
    {
      bool ok;
      const double x = value.toDouble( &ok );
      if ( !ok || std::isnan( x ) || !std::isfinite( x ) )
      {
        if ( parent )
          parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to double" ).arg( value.toString() ) );
        return 0;
      }
      return x;
    }

    static qlonglong getIntValue( const QVariant &value, QgsExpression *parent )
    {
      bool ok;
      const qlonglong x = value.toLongLong( &ok );
      if ( ok )
      {
        return x;
      }
      else
      {
        if ( parent )
          parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to int" ).arg( value.toString() ) );
        return 0;
      }
    }

    static int getNativeIntValue( const QVariant &value, QgsExpression *parent )
    {
      bool ok;
      const qlonglong x = value.toLongLong( &ok );
      if ( ok && x >= std::numeric_limits<int>::min() && x <= std::numeric_limits<int>::max() )
      {
        return static_cast<int>( x );
      }
      else
      {
        if ( parent )
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
        const QTime t = value.toTime();
        if ( t.isValid() )
        {
          return QDateTime( QDate( 1, 1, 1 ), t );
        }

        if ( parent )
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
        if ( parent )
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
        if ( parent )
          parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to Time" ).arg( value.toString() ) );
        return QTime();
      }
    }

    static QColor getColorValue( const QVariant &value, QgsExpression *parent, bool &isQColor );

    static QgsInterval getInterval( const QVariant &value, QgsExpression *parent, bool report_error = false )
    {
      if ( value.userType() == qMetaTypeId<QgsInterval>() )
        return value.value<QgsInterval>();

      QgsInterval inter = QgsInterval::fromString( value.toString() );
      if ( inter.isValid() )
      {
        return inter;
      }
      // If we get here then we can't convert so we just error and return invalid.
      if ( report_error && parent )
        parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to interval" ).arg( value.toString() ) );

      return QgsInterval();
    }

    static QgsGradientColorRamp getRamp( const QVariant &value, QgsExpression *parent, bool report_error = false );

    static QgsGeometry getGeometry( const QVariant &value, QgsExpression *parent, bool tolerant = false )
    {
      if ( value.userType() == qMetaTypeId< QgsReferencedGeometry>() )
        return value.value<QgsReferencedGeometry>();

      if ( value.userType() == qMetaTypeId< QgsGeometry>() )
        return value.value<QgsGeometry>();

      if ( !tolerant && parent )
        parent->setEvalErrorString( QStringLiteral( "Cannot convert to geometry" ) );
      return QgsGeometry();
    }

    static QgsFeature getFeature( const QVariant &value, QgsExpression *parent )
    {
      if ( value.userType() == qMetaTypeId<QgsFeature>() )
        return value.value<QgsFeature>();

      if ( parent )
        parent->setEvalErrorString( QStringLiteral( "Cannot convert to feature" ) );
      return 0;
    }

    static QgsExpressionNode *getNode( const QVariant &value, QgsExpression *parent )
    {
      if ( value.canConvert<QgsExpressionNode *>() )
        return value.value<QgsExpressionNode *>();

      if ( parent )
        parent->setEvalErrorString( QStringLiteral( "Cannot convert to node" ) );
      return nullptr;
    }

    /**
     * \deprecated QGIS 3.40. Not actually deprecated, but this method is not thread safe -- use with extreme caution only when the thread safety has already been taken care of by the caller!.
     */
    Q_DECL_DEPRECATED static QgsMapLayer *getMapLayer( const QVariant &value, const QgsExpressionContext *context, QgsExpression * );

    /**
     * Executes a lambda \a function for a \a value which corresponds to a map layer, in a thread-safe way.
     *
     * \since QGIS 3.30
     */
    static void executeLambdaForMapLayer( const QVariant &value, const QgsExpressionContext *context, QgsExpression *expression, const std::function< void( QgsMapLayer * )> &function, bool &foundLayer );

    /**
     * Evaluates a \a value to a map layer, then runs a \a function on the layer in a thread safe way before returning the result of the function.
     *
     * \since QGIS 3.30
     */
    static QVariant runMapLayerFunctionThreadSafe( const QVariant &value, const QgsExpressionContext *context, QgsExpression *expression, const std::function<QVariant( QgsMapLayer * ) > &function, bool &foundLayer );

    /**
     * Gets a vector layer feature source for a \a value which corresponds to a vector layer, in a thread-safe way.
     */
    static std::unique_ptr<QgsVectorLayerFeatureSource> getFeatureSource( const QVariant &value, const QgsExpressionContext *context, QgsExpression *e, bool &foundLayer );

    /**
     * \deprecated QGIS 3.40. Not actually deprecated, but this method is not thread safe -- use with extreme caution only when the thread safety has already been taken care of by the caller!.
     */
    Q_DECL_DEPRECATED static QgsVectorLayer *getVectorLayer( const QVariant &value, const QgsExpressionContext *context, QgsExpression *e );

    /**
     * Tries to convert a \a value to a file path.
     *
     * \since QGIS 3.24
     */
    static QString getFilePathValue( const QVariant &value, const QgsExpressionContext *context, QgsExpression *parent );

    static QVariantList getListValue( const QVariant &value, QgsExpression *parent )
    {
      if ( value.userType() == QMetaType::Type::QVariantList || value.userType() == QMetaType::Type::QStringList )
      {
        return value.toList();
      }
      else
      {
        if ( parent )
          parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to array" ).arg( value.toString() ) );
        return QVariantList();
      }
    }

    static QVariantMap getMapValue( const QVariant &value, QgsExpression *parent )
    {
      if ( value.userType() == QMetaType::Type::QVariantMap )
      {
        return value.toMap();
      }
      else
      {
        if ( parent )
          parent->setEvalErrorString( QObject::tr( "Cannot convert '%1' to map" ).arg( value.toString() ) );
        return QVariantMap();
      }
    }

    /**
     * Returns the localized string representation of a QVariant, converting numbers according to locale settings.
     * \param value the QVariant to convert.
     * \returns the string representation of the value.
     * \since QGIS 3.20
     */
    static QString toLocalizedString( const QVariant &value )
    {
      if ( value.userType() == QMetaType::Type::Int || value.userType() == QMetaType::Type::UInt || value.userType() == QMetaType::Type::LongLong || value.userType() == QMetaType::Type::ULongLong )
      {
        bool ok;
        QString res;

        if ( value.userType() == QMetaType::Type::ULongLong )
        {
          res = QLocale().toString( value.toULongLong( &ok ) );
        }
        else
        {
          res = QLocale().toString( value.toLongLong( &ok ) );
        }

        if ( ok )
        {
          return res;
        }
        else
        {
          return value.toString();
        }
      }
      // Qt madness with QMetaType::Float :/
      else if ( value.userType() == QMetaType::Type::Double || value.userType() == static_cast<QMetaType::Type>( QMetaType::Float ) )
      {
        bool ok;
        const QString strVal = value.toString();
        const int dotPosition = strVal.indexOf( '.' );
        const int precision = dotPosition > 0 ? strVal.length() - dotPosition - 1 : 0;
        const QString res = QLocale().toString( value.toDouble( &ok ), 'f', precision );

        if ( ok )
        {
          return res;
        }
        else
        {
          return value.toString();
        }
      }
      else
      {
        return value.toString();
      }
    }
/// @endcond

    /**
     * Returns a value type and user type for a given expression.
     * \param expression An expression string.
     * \param layer A vector layer from which the expression will be executed against.
     * \param request A feature request object.
     * \param context An expression context object.
     * \param foundFeatures An optional boolean parameter that will be set when features are found.
     * \since QGIS 3.22
     */
    static std::tuple<QMetaType::Type, int> determineResultType( const QString &expression, const QgsVectorLayer *layer, const QgsFeatureRequest &request = QgsFeatureRequest(), const QgsExpressionContext &context = QgsExpressionContext(), bool *foundFeatures = nullptr );

  private:

    /**
     * \warning Only call when thread safety has been taken care of by the caller!
     */
    static QgsMapLayer *getMapLayerPrivate( const QVariant &value, const QgsExpressionContext *context, QgsExpression * );

};


#endif // QGSEXPRESSIONUTILS_H
