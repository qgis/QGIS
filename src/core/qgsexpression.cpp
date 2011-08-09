/***************************************************************************
                              qgsexpression.cpp
                             -------------------
    begin                : August 2011
    copyright            : (C) 2011 Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpression.h"

#include <QtDebug>
#include <QSettings>
#include <math.h>

#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"

// from parser
extern QgsExpression::Node* parseExpression( const QString& str, QString& parserErrorMsg );


///////////////////////////////////////////////
// three-value logic

class TVL
{
  public:
    enum Value
    {
      False,
      True,
      Unknown
    };

    static TVL::Value AND[3][3];
    static TVL::Value OR[3][3];
    static TVL::Value NOT[3];

    TVL() : val( Unknown ) {}
    TVL( bool v ) : val( v ? True : False ) {}
    TVL( Value v ) : val( v ) {}

    TVL operator!() { return NOT[val]; }
    TVL operator&( const TVL& other ) { return AND[val][other.val]; }
    TVL operator|( const TVL& other ) { return OR[val][other.val]; }

    // conversion for result
    QVariant toVariant()
    {
      if ( val == Unknown )
        return QVariant();
      else
        return ( val == True ? 1 : 0 );
    }

    Value val;
};

// false       true        unknown
TVL::Value TVL::AND[3][3] = { { TVL::False, TVL::False,   TVL::False },  // false
  { TVL::False, TVL::True,    TVL::Unknown },  // true
  { TVL::False, TVL::Unknown, TVL::Unknown }
};// unknown

TVL::Value TVL::OR[3][3] = { { TVL::False,   TVL::True, TVL::Unknown }, // false
  { TVL::True,    TVL::True, TVL::True },     // true
  { TVL::Unknown, TVL::True, TVL::Unknown }
};// unknown

TVL::Value TVL::NOT[3] = { TVL::True, TVL::False, TVL::Unknown };

Q_DECLARE_METATYPE( TVL )

#define TVL_True     QVariant::fromValue(TVL(true))
#define TVL_False    QVariant::fromValue(TVL(false))
#define TVL_Unknown  QVariant::fromValue(TVL())

///////////////////////////////////////////////
// QVariant checks and conversions

inline bool isInt( const QVariant& v ) { return v.type() == QVariant::Int; }
inline bool isDouble( const QVariant& v ) { return v.type() == QVariant::Double; }
inline bool isNumeric( const QVariant& v ) { return v.type() == QVariant::Int || v.type() == QVariant::Double; }
inline bool isString( const QVariant& v ) { return v.type() == QVariant::String; }
inline bool isNull( const QVariant& v ) { return v.type() == QVariant::Invalid; }
inline bool isLogic( const QVariant& v ) { return v.canConvert<TVL>(); }
inline bool isNumericOrNull( const QVariant& v ) { return v.type() == QVariant::Int || v.type() == QVariant::Double; }

inline int qvInt( const QVariant& v ) { return v.toInt(); }
inline double qvDouble( const QVariant& v ) { return v.toDouble(); }
inline QString qvString( const QVariant& v ) { return v.toString(); }
inline TVL qvLogic( const QVariant& v ) { return v.value<TVL>(); }

///////////////////////////////////////////////
// evaluation error macros

#define ENSURE_NO_EVAL_ERROR   {  if (!parent->mEvalErrorString.isNull()) return QVariant(); }
#define SET_EVAL_ERROR(x)   { parent->mEvalErrorString = x; return QVariant(); }

///////////////////////////////////////////////
// operators

const char* QgsExpression::BinaryOperatorText[] =
{
  "OR", "AND",
  "=", "<>", "<=", ">=", "<", ">", "~", "LIKE", "ILIKE", "IS", "IS NOT",
  "+", "-", "*", "/", "%", "^",
  "||"
};

const char* QgsExpression::UnaryOperatorText[] =
{
  "NOT", "-"
};

///////////////////////////////////////////////
// functions

static int getIntArg( int i, const QVariantList& values, QgsExpression* parent )
{
  QVariant v = values.at( i );
  if ( !isInt( v ) )
  {
    parent->setEvalErrorString( "Function needs integer value" );
    return 0;
  }
  return v.toInt();
}

static double getNumericArg( int i, const QVariantList& values, QgsExpression* parent )
{
  QVariant v = values.at( i );
  if ( !isNumeric( v ) )
  {
    parent->setEvalErrorString( "Function needs numeric value" );
    return NAN;
  }
  return v.toDouble();
}

static QString getStringArg( int i, const QVariantList& values, QgsExpression* parent )
{
  QVariant v = values.at( i );
  if ( !isString( v ) )
  {
    parent->setEvalErrorString( "Function needs string value" );
    return QString();
  }
  return v.toString();
}

QVariant fcnSqrt( const QVariantList& values, QgsFeature* /*f*/, QgsExpression* parent )
{
  double x = getNumericArg( 0, values, parent );
  return isnan( x ) ? QVariant() : QVariant( sqrt( x ) );
}
QVariant fcnSin( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  double x = getNumericArg( 0, values, parent );
  return isnan( x ) ? QVariant() : QVariant( sin( x ) );
}
QVariant fcnCos( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  double x = getNumericArg( 0, values, parent );
  return isnan( x ) ? QVariant() : QVariant( cos( x ) );
}
QVariant fcnTan( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  double x = getNumericArg( 0, values, parent );
  return isnan( x ) ? QVariant() : QVariant( tan( x ) );
}
QVariant fcnAsin( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  double x = getNumericArg( 0, values, parent );
  return isnan( x ) ? QVariant() : QVariant( asin( x ) );
}
QVariant fcnAcos( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  double x = getNumericArg( 0, values, parent );
  return isnan( x ) ? QVariant() : QVariant( acos( x ) );
}
QVariant fcnAtan( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  double x = getNumericArg( 0, values, parent );
  return isnan( x ) ? QVariant() : QVariant( atan( x ) );
}
QVariant fcnAtan2( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  double y = getNumericArg( 0, values, parent );
  double x = getNumericArg( 1, values, parent );
  if ( isnan( y ) || isnan( x ) ) return QVariant();
  return QVariant( atan2( y, x ) );
}
QVariant fcnToInt( const QVariantList& values, QgsFeature* , QgsExpression* /*parent*/ )
{
  QVariant v = values.at( 0 );
  if ( v.type() == QVariant::Invalid ) return QVariant();
  return QVariant( v.toInt() );
}
QVariant fcnToReal( const QVariantList& values, QgsFeature* , QgsExpression* /*parent*/ )
{
  QVariant v = values.at( 0 );
  if ( v.type() == QVariant::Invalid ) return QVariant();
  return QVariant( v.toDouble() );
}
QVariant fcnToString( const QVariantList& values, QgsFeature* , QgsExpression* /*parent*/ )
{
  QVariant v = values.at( 0 );
  if ( v.type() == QVariant::Invalid ) return QVariant();
  return QVariant( v.toString() );
}
QVariant fcnLower( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  QString str = getStringArg( 0, values, parent );
  if ( str.isNull() ) return QVariant(); // error or null string
  return QVariant( str.toLower() );
}
QVariant fcnUpper( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  QString str = getStringArg( 0, values, parent );
  if ( str.isNull() ) return QVariant(); // error or null string
  return QVariant( str.toUpper() );
}
QVariant fcnLength( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  QString str = getStringArg( 0, values, parent );
  if ( str.isNull() ) return QVariant(); // error or null string
  return QVariant( str.length() );
}
QVariant fcnReplace( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  QString str = getStringArg( 0, values, parent );
  QString before = getStringArg( 1, values, parent );
  QString after = getStringArg( 2, values, parent );
  if ( str.isNull() || before.isNull() || after.isNull() ) return QVariant(); // error or null string
  return QVariant( str.replace( before, after ) );
}
QVariant fcnRegexpReplace( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  QString str = getStringArg( 0, values, parent );
  QString regexp = getStringArg( 1, values, parent );
  QString after = getStringArg( 2, values, parent );
  if ( str.isNull() || regexp.isNull() || after.isNull() ) return QVariant(); // error or null string

  QRegExp re( regexp );
  if ( !re.isValid() )
  {
    parent->setEvalErrorString( QString( "Invalid regular expression '%1': %2" ).arg( regexp ).arg( re.errorString() ) );
    return QVariant();
  }
  return QVariant( str.replace( re, after ) );
}
QVariant fcnSubstr( const QVariantList& values, QgsFeature* , QgsExpression* parent )
{
  QString str = getStringArg( 0, values, parent );
  if ( str.isNull() ) return QVariant(); // error or null string
  int from = getIntArg( 1, values, parent );
  if ( parent->hasEvalError() ) return QVariant();
  int  len = getIntArg( 2, values, parent );
  if ( parent->hasEvalError() ) return QVariant();
  return QVariant( str.mid( from -1, len ) );
}

QVariant fcnRowNumber( const QVariantList& , QgsFeature* , QgsExpression* parent )
{
  return QVariant( parent->currentRowNumber() );
}

QVariant fcnFeatureId( const QVariantList& , QgsFeature* f, QgsExpression* )
{
  // TODO: handling of 64-bit feature ids?
  return f ? QVariant(( int )f->id() ) : QVariant();
}

#define ENSURE_GEOM_TYPE(f, g, geomtype)   if (!f) return QVariant(); \
  QgsGeometry* g = f->geometry(); \
  if (!g || g->type() != geomtype) return QVariant();


QVariant fcnX( const QVariantList& , QgsFeature* f, QgsExpression* )
{
  ENSURE_GEOM_TYPE( f, g, QGis::Point );
  return g->asPoint().x();
}
QVariant fcnY( const QVariantList& , QgsFeature* f, QgsExpression* )
{
  ENSURE_GEOM_TYPE( f, g, QGis::Point );
  return g->asPoint().y();
}

static QVariant pointAt( const QVariantList& values, QgsFeature* f, QgsExpression* parent ) // helper function
{
  int idx = getIntArg( 0, values, parent );
  ENSURE_GEOM_TYPE( f, g, QGis::Line );
  QgsPolyline polyline = g->asPolyline();
  if ( idx < 0 )
    idx += polyline.count();

  if ( idx < 0 || idx >= polyline.count() )
  {
    parent->setEvalErrorString( "Index is out of range" );
    return QVariant();
  }
  return QVariant( QPointF( polyline[idx].x(), polyline[idx].y() ) );
}

QVariant fcnXat( const QVariantList& values, QgsFeature* f, QgsExpression* parent )
{
  QVariant v = pointAt( values, f, parent );
  if ( v.type() == QVariant::PointF )
    return QVariant( v.toPointF().x() );
  else
    return QVariant();
}
QVariant fcnYat( const QVariantList& values, QgsFeature* f, QgsExpression* parent )
{
  QVariant v = pointAt( values, f, parent );
  if ( v.type() == QVariant::PointF )
    return QVariant( v.toPointF().y() );
  else
    return QVariant();
}

QVariant fcnGeomArea( const QVariantList& , QgsFeature* f, QgsExpression* parent )
{
  ENSURE_GEOM_TYPE( f, g, QGis::Polygon );
  QgsDistanceArea* calc = parent->geomCalculator();
  return QVariant( calc->measure( f->geometry() ) );
}
QVariant fcnGeomLength( const QVariantList& , QgsFeature* f, QgsExpression* parent )
{
  ENSURE_GEOM_TYPE( f, g, QGis::Line );
  QgsDistanceArea* calc = parent->geomCalculator();
  return QVariant( calc->measure( f->geometry() ) );
}
QVariant fcnGeomPerimeter( const QVariantList& , QgsFeature* f, QgsExpression* parent )
{
  ENSURE_GEOM_TYPE( f, g, QGis::Polygon );
  QgsDistanceArea* calc = parent->geomCalculator();
  return QVariant( calc->measurePerimeter( f->geometry() ) );
}

typedef QgsExpression::FunctionDef FnDef;

FnDef QgsExpression::BuiltinFunctions[] =
{
  // math
  FnDef( "sqrt", 1, fcnSqrt ),
  FnDef( "sin", 1, fcnSin ),
  FnDef( "cos", 1, fcnCos ),
  FnDef( "tan", 1, fcnTan ),
  FnDef( "asin", 1, fcnAsin ),
  FnDef( "acos", 1, fcnAcos ),
  FnDef( "atan", 1, fcnAtan ),
  FnDef( "atan2", 2, fcnAtan2 ),
  // casts
  FnDef( "toint", 1, fcnToInt ),
  FnDef( "toreal", 1, fcnToReal ),
  FnDef( "tostring", 1, fcnToString ),
  // string manipulation
  FnDef( "lower", 1, fcnLower ),
  FnDef( "upper", 1, fcnUpper ),
  FnDef( "length", 1, fcnLength ),
  FnDef( "replace", 3, fcnReplace ),
  FnDef( "regexp_replace", 3, fcnRegexpReplace ),
  FnDef( "substr", 3, fcnSubstr ),
  // geometry accessors
  FnDef( "xat", 1, fcnXat, true ),
  FnDef( "yat", 1, fcnYat, true ),
  // special columns
  FnDef( "$rownum", 0, fcnRowNumber ),
  FnDef( "$area", 0, fcnGeomArea, true ),
  FnDef( "$length", 0, fcnGeomLength, true ),
  FnDef( "$perimeter", 0, fcnGeomPerimeter, true ),
  FnDef( "$x", 0, fcnX, true ),
  FnDef( "$y", 0, fcnY, true ),
  FnDef( "$id", 0, fcnFeatureId ),
};


bool QgsExpression::isFunctionName( QString name )
{
  return functionIndex( name ) != -1;
}

int QgsExpression::functionIndex( QString name )
{
  int count = sizeof( BuiltinFunctions ) / sizeof( FunctionDef );
  for ( int i = 0; i < count; i++ )
  {
    if ( QString::compare( name, BuiltinFunctions[i].mName, Qt::CaseInsensitive ) == 0 )
      return i;
  }
  return -1;
}


QgsExpression::QgsExpression( const QString& expr )
    : mExpression( expr ), mRowNumber( 0 ), mCalc( NULL )
{
  mRootNode = ::parseExpression( mExpression, mParserErrorString );

  if ( mParserErrorString.isNull() )
  {
    Q_ASSERT( mRootNode != NULL );
  }
}

QgsExpression::~QgsExpression()
{
  delete mRootNode;
  delete mCalc;
}

QStringList QgsExpression::referencedColumns()
{
  if ( !mRootNode )
    return QStringList();
  QStringList columns = mRootNode->referencedColumns();

  // filter out duplicates
  for ( int i = 0; i < columns.count(); i++ )
  {
    QString col = columns.at( i );
    for ( int j = i + 1; j < columns.count(); j++ )
    {
      if ( QString::compare( col, columns[j], Qt::CaseInsensitive ) == 0 )
      {
        // this column is repeated: remove it!
        columns.removeAt( j-- );
      }
    }
  }

  return columns;
}

bool QgsExpression::needsGeometry()
{
  if ( !mRootNode )
    return false;
  return mRootNode->needsGeometry();
}

void QgsExpression::initGeomCalculator()
{
  mCalc = new QgsDistanceArea;
  mCalc->setProjectionsEnabled( false );
  QSettings settings;
  QString ellipsoid = settings.value( "/qgis/measure/ellipsoid", "WGS84" ).toString();
  mCalc->setEllipsoid( ellipsoid );
}

bool QgsExpression::prepare( const QgsFieldMap& fields )
{
  mEvalErrorString = QString();
  if ( !mRootNode )
  {
    mEvalErrorString = "No root node! Parsing failed?";
    return false;
  }

  return mRootNode->prepare( this, fields );
}

QVariant QgsExpression::evaluate( QgsFeature* f )
{
  mEvalErrorString = QString();
  if ( !mRootNode )
  {
    mEvalErrorString = "No root node! Parsing failed?";
    return QVariant();
  }

  QVariant res = mRootNode->eval( this, f );
  if ( res.canConvert<TVL>() )
  {
    // convert 3-value logic to int (0/1) or null
    return res.value<TVL>().toVariant();
  }
  return res;
}

QVariant QgsExpression::evaluate( QgsFeature* f, const QgsFieldMap& fields )
{
  // first prepare
  bool res = prepare( fields );
  if ( !res )
    return QVariant();

  // then evaluate
  return evaluate( f );
}

QString QgsExpression::dump() const
{
  if ( !mRootNode ) return "(no root)";

  return mRootNode->dump();
}


///////////////////////////////////////////////
// nodes

QString QgsExpression::NodeList::dump() const
{
  QString msg; bool first = true;
  foreach( Node* n, mList )
  {
    if ( !first ) msg += ", "; else first = false;
    msg += n->dump();
  }
  return msg;
}

//

QVariant QgsExpression::NodeUnaryOperator::eval( QgsExpression* parent, QgsFeature* f )
{
  QVariant val = mOperand->eval( parent, f );
  ENSURE_NO_EVAL_ERROR;

  switch ( mOp )
  {
    case uoNot:
      if ( isLogic( val ) )
        val.setValue( ! qvLogic( val ) );
      else
        SET_EVAL_ERROR( "NOT applicable only on boolean" );
      break;

    case uoMinus:
      if ( isInt( val ) )
        val.setValue( - qvInt( val ) );
      else if ( isDouble( val ) )
        val.setValue( - qvDouble( val ) );
      else
        SET_EVAL_ERROR( "Unary minus only for numeric values." );
      break;
    default:
      Q_ASSERT( 0 && "unknown unary operation" );
  }

  return val;
}

bool QgsExpression::NodeUnaryOperator::prepare( QgsExpression* parent, const QgsFieldMap& fields )
{
  return mOperand->prepare( parent, fields );
}

QString QgsExpression::NodeUnaryOperator::dump() const
{
  return QString( "%1 %2" ).arg( UnaryOperatorText[mOp] ).arg( mOperand->dump() );
}

//

QVariant QgsExpression::NodeBinaryOperator::eval( QgsExpression* parent, QgsFeature* f )
{
  QVariant vL = mOpLeft->eval( parent, f );
  ENSURE_NO_EVAL_ERROR;
  QVariant vR = mOpRight->eval( parent, f );
  ENSURE_NO_EVAL_ERROR;

  switch ( mOp )
  {
    case boPlus:
    case boMinus:
    case boMul:
    case boDiv:
    case boMod:
      if ( isNull( vL ) || isNull( vR ) )
        return QVariant();
      else if ( isNumeric( vL ) && isNumeric( vR ) )
      {
        if ( mOp == boDiv && qvDouble( vR ) == 0 )
          return QVariant(); // silently handle division by zero and return NULL
        if ( isInt( vL ) && isInt( vR ) )
          return QVariant( computeInt( qvInt( vL ), qvInt( vR ) ) );
        else
          return QVariant( computeDouble( qvDouble( vL ), qvDouble( vR ) ) );
      }
      else
        SET_EVAL_ERROR( "Arithmetic possible only with numeric values" );

    case boPow:
      if ( isNull( vL ) || isNull( vR ) )
        return QVariant();
      else if ( isNumeric( vL ) && isNumeric( vR ) )
        return QVariant( pow( qvDouble( vL ), qvDouble( vR ) ) );
      else
        SET_EVAL_ERROR( "Arithmetic possible only with numeric values" );

    case boAnd:
      if ( isLogic( vL ) && isLogic( vR ) )
        return QVariant::fromValue( qvLogic( vL ) & qvLogic( vR ) );
      else
        SET_EVAL_ERROR( "AND applicable only on boolean" );

    case boOr:
      if ( isLogic( vL ) && isLogic( vR ) )
        return QVariant::fromValue( qvLogic( vL ) | qvLogic( vR ) );
      else
        SET_EVAL_ERROR( "OR applicable only on boolean" );

    case boEQ:
    case boNE:
    case boLT:
    case boGT:
    case boLE:
    case boGE:
      if ( isNull( vL ) || isNull( vR ) )
      {
        return TVL_Unknown;
      }
      else if ( isNumeric( vL ) && isNumeric( vR ) )
      {
        double diff = qvDouble( vL ) - qvDouble( vR );
        return compare( diff ) ? TVL_True : TVL_False;
      }
      else if ( isString( vL ) && isString( vR ) )
      {
        int diff = QString::compare( qvString( vL ), qvString( vR ) );
        return compare( diff ) ? TVL_True : TVL_False;
      }
      else
        SET_EVAL_ERROR( "Invalid arguments for comparison" );

    case boIs:
    case boIsNot:
      if ( isNull( vL ) && isNull( vR ) ) // both operators null
        return ( mOp == boIs ? TVL_True : TVL_False );
      else if ( isNull( vL ) || isNull( vR ) ) // one operator null
        return ( mOp == boIs ? TVL_False : TVL_True );
      else // both operators non-null
      {
        bool equal = false;
        if ( isNumeric( vL ) && isNumeric( vR ) )
          equal = qvDouble( vL ) == qvDouble( vR );
        else if ( isString( vL ) && isString( vR ) )
          equal = QString::compare( qvString( vL ), qvString( vR ) ) == 0;
        else
          SET_EVAL_ERROR( "Invalid arguments for comparison" );
        if ( equal )
          return mOp == boIs ? TVL_True : TVL_False;
        else
          return mOp == boIs ? TVL_False : TVL_True;
      }

    case boRegexp:
    case boLike:
    case boILike:
      if ( isNull( vL ) || isNull( vR ) )
        return TVL_Unknown;
      else if ( isString( vL ) && isString( vR ) )
      {
        QString str = qvString( vL ), regexp = qvString( vR );
        // TODO: cache QRegExp in case that regexp is a literal string (i.e. it will stay constant)
        bool matches;
        if ( mOp == boLike || mOp == boILike ) // change from LIKE syntax to regexp
        {
          // XXX escape % and _  ???
          regexp.replace( "%", ".*" );
          regexp.replace( "_", "." );
          matches = QRegExp( regexp, mOp == boLike ? Qt::CaseSensitive : Qt::CaseInsensitive ).exactMatch( str );
        }
        else
        {
          matches = QRegExp( regexp ).indexIn( str ) != -1;
        }
        return matches ? TVL_True : TVL_False;
      }
      else
        SET_EVAL_ERROR( "Invalid arguments for regexp" );

    case boConcat:
      if ( isNull( vL ) || isNull( vR ) )
        return QVariant();
      else if ( isString( vL ) || isString( vR ) )
      {
        return QVariant( qvString( vL ) + qvString( vR ) );
      }
      else
        SET_EVAL_ERROR( "Invalid arguments for concatenation" );

    default: break;
  }
  Q_ASSERT( false );
  return QVariant();
}

bool QgsExpression::NodeBinaryOperator::compare( double diff )
{
  switch ( mOp )
  {
    case boEQ: return diff == 0;
    case boNE: return diff != 0;
    case boLT: return diff < 0;
    case boGT: return diff > 0;
    case boLE: return diff <= 0;
    case boGE: return diff >= 0;
    default: Q_ASSERT( false ); return false;
  }
}

int QgsExpression::NodeBinaryOperator::computeInt( int x, int y )
{
  switch ( mOp )
  {
    case boPlus: return x+y;
    case boMinus: return x-y;
    case boMul: return x*y;
    case boDiv: return x/y;
    case boMod: return x%y;
    default: Q_ASSERT( false ); return 0;
  }
}

double QgsExpression::NodeBinaryOperator::computeDouble( double x, double y )
{
  switch ( mOp )
  {
    case boPlus: return x+y;
    case boMinus: return x-y;
    case boMul: return x*y;
    case boDiv: return x/y;
    case boMod: return fmod( x,y );
    default: Q_ASSERT( false ); return 0;
  }
}


bool QgsExpression::NodeBinaryOperator::prepare( QgsExpression* parent, const QgsFieldMap& fields )
{
  bool resL = mOpLeft->prepare( parent, fields );
  bool resR = mOpRight->prepare( parent, fields );
  return resL && resR;
}

QString QgsExpression::NodeBinaryOperator::dump() const
{
  return QString( "%1 %2 %3" ).arg( mOpLeft->dump() ).arg( BinaryOperatorText[mOp] ).arg( mOpRight->dump() );
}

//

QVariant QgsExpression::NodeInOperator::eval( QgsExpression* parent, QgsFeature* f )
{
  if ( mList->count() == 0 )
    return mNotIn ? TVL_True : TVL_False;
  QVariant v1 = mNode->eval( parent, f );
  ENSURE_NO_EVAL_ERROR;
  if ( isNull( v1 ) )
    return TVL_Unknown;

  bool listHasNull = false;

  foreach( Node* n, mList->list() )
  {
    QVariant v2 = n->eval( parent, f );
    ENSURE_NO_EVAL_ERROR;
    if ( isNull( v2 ) )
      listHasNull = true;
    else
    {
      bool equal = false;
      // check whether they are equal
      if ( isNumeric( v1 ) && isNumeric( v2 ) )
        equal = ( qvDouble( v1 ) == qvDouble( v2 ) );
      else if ( isString( v1 ) && isString( v2 ) )
        equal = ( QString::compare( qvString( v1 ), qvString( v2 ) ) == 0 );
      else
        SET_EVAL_ERROR( "Invalid arguments for comparison (IN operator)" );

      if ( equal ) // we know the result
        return mNotIn ? TVL_False : TVL_True;
    }
  }

  // item not found
  if ( listHasNull )
    return TVL_Unknown;
  else
    return mNotIn ? TVL_True : TVL_False;
}

bool QgsExpression::NodeInOperator::prepare( QgsExpression* parent, const QgsFieldMap& fields )
{
  bool res = true;
  foreach( Node* n, mList->list() )
  {
    res = res && n->prepare( parent, fields );
  }
  return res;
}

QString QgsExpression::NodeInOperator::dump() const
{
  return QString( "%1 IN (%2)" ).arg( mNode->dump() ).arg( mList->dump() );
}

//

QVariant QgsExpression::NodeFunction::eval( QgsExpression* parent, QgsFeature* f )
{
  const FunctionDef& fd = BuiltinFunctions[mFnIndex];

  // evaluate arguments
  QVariantList argValues;
  if ( mArgs )
  {
    foreach( Node* n, mArgs->list() )
    {
      argValues.append( n->eval( parent, f ) );
      ENSURE_NO_EVAL_ERROR;
    }
  }

  // run the function
  QVariant res = fd.mFcn( argValues, f, parent );
  ENSURE_NO_EVAL_ERROR;

  // everything went fine
  return res;
}

bool QgsExpression::NodeFunction::prepare( QgsExpression* parent, const QgsFieldMap& fields )
{
  bool res = true;
  foreach( Node* n, mArgs->list() )
  {
    res = res && n->prepare( parent, fields );
  }
  return res;
}

QString QgsExpression::NodeFunction::dump() const
{
  const FnDef& fd = BuiltinFunctions[mFnIndex];
  if ( fd.mParams == 0 )
    return fd.mName; // special column
  else
    return QString( "%1(%2)" ).arg( fd.mName ).arg( mArgs->dump() ); // function
}

//

QVariant QgsExpression::NodeLiteral::eval( QgsExpression* , QgsFeature* )
{
  return mValue;
}

bool QgsExpression::NodeLiteral::prepare( QgsExpression* /*parent*/, const QgsFieldMap& /*fields*/ )
{
  return true;
}


QString QgsExpression::NodeLiteral::dump() const
{
  switch ( mValue.type() )
  {
    case QVariant::Invalid: return "NULL";
    case QVariant::Int: return QString::number( mValue.toInt() );
    case QVariant::Double: return QString::number( mValue.toDouble() );
    case QVariant::String: return QString( "'%1'" ).arg( mValue.toString() );
    default: return "[unsupported value]";
  }
}

//

QVariant QgsExpression::NodeColumnRef::eval( QgsExpression* /*parent*/, QgsFeature* f )
{
  return f->attributeMap()[mIndex];
}

bool QgsExpression::NodeColumnRef::prepare( QgsExpression* parent, const QgsFieldMap& fields )
{
  foreach( int i, fields.keys() )
  {
    if ( QString::compare( fields[i].name(), mName, Qt::CaseInsensitive ) == 0 )
    {
      mIndex = i;
      return true;
    }
  }
  parent->mEvalErrorString = QString( "Column %1 not found" ).arg( mName );
  mIndex = -1;
  return false;
}

QString QgsExpression::NodeColumnRef::dump() const
{
  return mName;
}
