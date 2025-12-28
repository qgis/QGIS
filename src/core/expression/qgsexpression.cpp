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

#include <memory>

#include "qgscolorrampimpl.h"
#include "qgsexpression_p.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionfunction.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsexpressionutils.h"
#include "qgsfeaturerequest.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsunittypes.h"
#include "qgsvariantutils.h"

#include <QRegularExpression>

// from parser
extern QgsExpressionNode *parseExpression( const QString &str, QString &parserErrorMsg, QList<QgsExpression::ParserError> &parserErrors );

Q_GLOBAL_STATIC( QgsStringMap, sVariableHelpTexts )
Q_GLOBAL_STATIC( QgsStringMap, sGroups )

HelpTextHash QgsExpression::sFunctionHelpTexts;
QRecursiveMutex QgsExpression::sFunctionsMutex;
QMap< QString, int> QgsExpression::sFunctionIndexMap;

///@cond PRIVATE
HelpTextHash &QgsExpression::functionHelpTexts()
{
  return sFunctionHelpTexts;
}
///@endcond

bool QgsExpression::checkExpression( const QString &text, const QgsExpressionContext *context, QString &errorMessage )
{
  QgsExpression exp( text );
  exp.prepare( context );
  errorMessage = exp.parserErrorString();
  return !exp.hasParserError();
}

void QgsExpression::setExpression( const QString &expression )
{
  detach();
  d->mRootNode.reset( ::parseExpression( expression, d->mParserErrorString, d->mParserErrors ) );
  d->mEvalErrorString = QString();
  d->mExp = expression;
  d->mIsPrepared = false;
}

QString QgsExpression::expression() const
{
  if ( !d->mExp.isNull() )
    return d->mExp;
  else
    return dump();
}

QString QgsExpression::quotedColumnRef( QString name )
{
  return u"\"%1\""_s.arg( name.replace( '\"', "\"\""_L1 ) );
}

QString QgsExpression::quotedString( QString text )
{
  text.replace( '\'', "''"_L1 );
  text.replace( '\\', "\\\\"_L1 );
  text.replace( '\n', "\\n"_L1 );
  text.replace( '\t', "\\t"_L1 );
  return u"'%1'"_s.arg( text );
}

QString QgsExpression::quotedValue( const QVariant &value )
{
  return quotedValue( value, static_cast<QMetaType::Type>( value.userType() ) );
}

QString QgsExpression::quotedValue( const QVariant &value, QMetaType::Type type )
{
  if ( QgsVariantUtils::isNull( value ) )
    return u"NULL"_s;

  switch ( type )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::Double:
      return value.toString();

    case QMetaType::Type::Bool:
      return value.toBool() ? u"TRUE"_s : u"FALSE"_s;

    case QMetaType::Type::QVariantList:
    case QMetaType::Type::QStringList:
    {
      QStringList quotedValues;
      const QVariantList values = value.toList();
      quotedValues.reserve( values.count() );
      for ( const QVariant &v : values )
      {
        quotedValues += quotedValue( v );
      }
      return u"array( %1 )"_s.arg( quotedValues.join( ", "_L1 ) );
    }

    default:
    case QMetaType::Type::QString:
      return quotedString( value.toString() );
  }

}

QString QgsExpression::quotedValue( const QVariant &value, QVariant::Type type )
{
  return quotedValue( value, QgsVariantUtils::variantTypeToMetaType( type ) );
}

bool QgsExpression::isFunctionName( const QString &name )
{
  return functionIndex( name ) != -1;
}

int QgsExpression::functionIndex( const QString &name )
{
  QMutexLocker locker( &sFunctionsMutex );

  auto it = sFunctionIndexMap.constFind( name );
  if ( it != sFunctionIndexMap.constEnd() )
    return *it;

  const QList<QgsExpressionFunction *> &functions = QgsExpression::Functions();
  int i = 0;
  for ( const QgsExpressionFunction *function : functions )
  {
    if ( QString::compare( name, function->name(), Qt::CaseInsensitive ) == 0 )
    {
      sFunctionIndexMap.insert( name, i );
      return i;
    }
    const QStringList aliases = function->aliases();
    for ( const QString &alias : aliases )
    {
      if ( QString::compare( name, alias, Qt::CaseInsensitive ) == 0 )
      {
        sFunctionIndexMap.insert( name, i );
        return i;
      }
    }
    i++;
  }
  return -1;
}

int QgsExpression::functionCount()
{
  return Functions().size();
}


QgsExpression::QgsExpression( const QString &expr )
  : d( new QgsExpressionPrivate )
{
  d->mRootNode.reset( ::parseExpression( expr, d->mParserErrorString, d->mParserErrors ) );
  d->mExp = expr;
  Q_ASSERT( !d->mParserErrorString.isNull() || d->mRootNode );
}

QgsExpression::QgsExpression( const QgsExpression &other )
  : d( other.d )
{
  d->ref.ref();
}

QgsExpression &QgsExpression::operator=( const QgsExpression &other )
{
  if ( this != &other )
  {
    if ( !d->ref.deref() )
    {
      delete d;
    }

    d = other.d;
    d->ref.ref();
  }
  return *this;
}

QgsExpression::operator QString() const
{
  return d->mExp;
}

QgsExpression::QgsExpression()
  : d( new QgsExpressionPrivate )
{
}

QgsExpression::~QgsExpression()
{
  Q_ASSERT( d );
  if ( !d->ref.deref() )
    delete d;
}

bool QgsExpression::operator==( const QgsExpression &other ) const
{
  return ( d == other.d || d->mExp == other.d->mExp );
}

bool QgsExpression::isValid() const
{
  return d->mRootNode.get();
}

bool QgsExpression::hasParserError() const
{
  return d->mParserErrors.count() > 0;
}

QString QgsExpression::parserErrorString() const
{
  return d->mParserErrorString.replace( "syntax error, unexpected end of file",
                                        tr( "Incomplete expression. You might not have finished the full expression." ) );
}

QList<QgsExpression::ParserError> QgsExpression::parserErrors() const
{
  return d->mParserErrors;
}

QSet<QString> QgsExpression::referencedColumns() const
{
  if ( !d->mRootNode )
    return QSet<QString>();

  return d->mRootNode->referencedColumns();
}

QSet<QString> QgsExpression::referencedVariables() const
{
  if ( !d->mRootNode )
    return QSet<QString>();

  return d->mRootNode->referencedVariables();
}

QSet<QString> QgsExpression::referencedFunctions() const
{
  if ( !d->mRootNode )
    return QSet<QString>();

  return d->mRootNode->referencedFunctions();
}

QSet<int> QgsExpression::referencedAttributeIndexes( const QgsFields &fields ) const
{
  if ( !d->mRootNode )
    return QSet<int>();

  const QSet<QString> referencedFields = d->mRootNode->referencedColumns();
  QSet<int> referencedIndexes;

  for ( const QString &fieldName : referencedFields )
  {
    if ( fieldName == QgsFeatureRequest::ALL_ATTRIBUTES )
    {
      const QgsAttributeList attributesList = fields.allAttributesList();
      referencedIndexes = QSet<int>( attributesList.begin(), attributesList.end() );
      break;
    }
    const int idx = fields.lookupField( fieldName );
    if ( idx >= 0 )
    {
      referencedIndexes << idx;
    }
  }

  return referencedIndexes;
}

bool QgsExpression::needsGeometry() const
{
  if ( !d->mRootNode )
    return false;
  return d->mRootNode->needsGeometry();
}

void QgsExpression::initGeomCalculator( const QgsExpressionContext *context )
{
  // Set the geometry calculator from the context if it has not been set by setGeomCalculator()
  if ( context && ! d->mCalc )
  {
    // actually don't do it right away, cos it's expensive to create and only a very small number of expression
    // functions actually require it. Let's lazily construct it when needed
    d->mDaEllipsoid = context->variable( u"project_ellipsoid"_s ).toString();
    d->mDaCrs = std::make_unique<QgsCoordinateReferenceSystem>( context->variable( u"_layer_crs"_s ).value<QgsCoordinateReferenceSystem>() );
    d->mDaTransformContext = std::make_unique<QgsCoordinateTransformContext>( context->variable( u"_project_transform_context"_s ).value<QgsCoordinateTransformContext>() );
  }

  // Set the distance units from the context if it has not been set by setDistanceUnits()
  if ( context && distanceUnits() == Qgis::DistanceUnit::Unknown )
  {
    QString distanceUnitsStr = context->variable( u"project_distance_units"_s ).toString();
    if ( ! distanceUnitsStr.isEmpty() )
      setDistanceUnits( QgsUnitTypes::stringToDistanceUnit( distanceUnitsStr ) );
  }

  // Set the area units from the context if it has not been set by setAreaUnits()
  if ( context && areaUnits() == Qgis::AreaUnit::Unknown )
  {
    QString areaUnitsStr = context->variable( u"project_area_units"_s ).toString();
    if ( ! areaUnitsStr.isEmpty() )
      setAreaUnits( QgsUnitTypes::stringToAreaUnit( areaUnitsStr ) );
  }
}

void QgsExpression::detach()
{
  Q_ASSERT( d );

  if ( d->ref > 1 )
  {
    ( void )d->ref.deref();

    d = new QgsExpressionPrivate( *d );
  }
}

void QgsExpression::initFunctionHelp()
{
  static std::once_flag initialized;
  std::call_once( initialized, buildFunctionHelp );
}

void QgsExpression::setGeomCalculator( const QgsDistanceArea *calc )
{
  detach();
  if ( calc )
    d->mCalc = std::make_shared<QgsDistanceArea>( *calc );
  else
    d->mCalc.reset();
}

bool QgsExpression::prepare( const QgsExpressionContext *context )
{
  detach();
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    //re-parse expression. Creation of QgsExpressionContexts may have added extra
    //known functions since this expression was created, so we have another try
    //at re-parsing it now that the context must have been created
    d->mRootNode.reset( ::parseExpression( d->mExp, d->mParserErrorString, d->mParserErrors ) );
  }

  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return false;
  }

  initGeomCalculator( context );
  d->mIsPrepared = true;
  return d->mRootNode->prepare( this, context );
}

QVariant QgsExpression::evaluate()
{
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return QVariant();
  }

  return d->mRootNode->eval( this, static_cast<const QgsExpressionContext *>( nullptr ) );
}

QVariant QgsExpression::evaluate( const QgsExpressionContext *context )
{
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return QVariant();
  }

  if ( ! d->mIsPrepared )
  {
    prepare( context );
  }
  return d->mRootNode->eval( this, context );
}

bool QgsExpression::hasEvalError() const
{
  return !d->mEvalErrorString.isNull();
}

QString QgsExpression::evalErrorString() const
{
  return d->mEvalErrorString;
}

void QgsExpression::setEvalErrorString( const QString &str )
{
  d->mEvalErrorString = str;
}

QString QgsExpression::dump() const
{
  if ( !d->mRootNode )
    return QString();

  return d->mRootNode->dump();
}

QgsDistanceArea *QgsExpression::geomCalculator()
{
  if ( !d->mCalc && d->mDaCrs && d->mDaCrs->isValid() && d->mDaTransformContext )
  {
    // calculator IS required, so initialize it now...
    d->mCalc = std::make_shared<QgsDistanceArea>( );
    d->mCalc->setEllipsoid( d->mDaEllipsoid.isEmpty() ? Qgis::geoNone() : d->mDaEllipsoid );
    d->mCalc->setSourceCrs( *d->mDaCrs.get(), *d->mDaTransformContext.get() );
  }

  return d->mCalc.get();
}

Qgis::DistanceUnit QgsExpression::distanceUnits() const
{
  return d->mDistanceUnit;
}

void QgsExpression::setDistanceUnits( Qgis::DistanceUnit unit )
{
  d->mDistanceUnit = unit;
}

Qgis::AreaUnit QgsExpression::areaUnits() const
{
  return d->mAreaUnit;
}

void QgsExpression::setAreaUnits( Qgis::AreaUnit unit )
{
  d->mAreaUnit = unit;
}

QString QgsExpression::replaceExpressionText( const QString &action, const QgsExpressionContext *context, const QgsDistanceArea *distanceArea )
{
  QString expr_action;

  int index = 0;
  while ( index < action.size() )
  {
    static const QRegularExpression sRegEx{ u"\\[%(.*?)%\\]"_s,  QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption };

    const QRegularExpressionMatch match = sRegEx.match( action, index );
    if ( !match.hasMatch() )
      break;

    const int pos = action.indexOf( sRegEx, index );
    const int start = index;
    index = pos + match.capturedLength( 0 );
    const QString toReplace = match.captured( 1 ).trimmed();
    QgsDebugMsgLevel( "Found expression: " + toReplace, 3 );

    QgsExpression exp( toReplace );
    if ( exp.hasParserError() )
    {
      QgsDebugError( "Expression parser error: " + exp.parserErrorString() );
      expr_action += QStringView {action} .mid( start, index - start );
      continue;
    }

    if ( distanceArea )
    {
      //if QgsDistanceArea specified for area/distance conversion, use it
      exp.setGeomCalculator( distanceArea );
    }

    QVariant result = exp.evaluate( context );

    if ( exp.hasEvalError() )
    {
      QgsDebugError( "Expression parser eval error: " + exp.evalErrorString() );
      expr_action += QStringView {action} .mid( start, index - start );
      continue;
    }

    QString resultString;
    if ( !QgsVariantUtils::isNull( result ) )
      resultString = result.toString();

    QgsDebugMsgLevel( "Expression result is: " + resultString, 3 );

    expr_action += action.mid( start, pos - start ) + resultString;
  }

  expr_action += QStringView {action} .mid( index ).toString();
  return expr_action;
}

QSet<QString> QgsExpression::referencedVariables( const QString &text )
{
  QSet<QString> variables;
  int index = 0;
  while ( index < text.size() )
  {
    const thread_local QRegularExpression rx( "\\[%([^\\]]+)%\\]" );
    const QRegularExpressionMatch match = rx.match( text );
    if ( !match.hasMatch() )
      break;

    index = match.capturedStart() + match.capturedLength();
    QString to_replace = match.captured( 1 ).trimmed();

    QgsExpression exp( to_replace );
    variables.unite( exp.referencedVariables() );
  }

  return variables;
}

double QgsExpression::evaluateToDouble( const QString &text, const double fallbackValue )
{
  bool ok;
  //first test if text is directly convertible to double
  // use system locale: e.g. in German locale, user is presented with numbers "1,23" instead of "1.23" in C locale
  // so we also want to allow user to rewrite it to "5,23" and it is still accepted
  double convertedValue = QLocale().toDouble( text, &ok );
  if ( ok )
  {
    return convertedValue;
  }

  //otherwise try to evaluate as expression
  QgsExpression expr( text );

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( QgsProject::instance() ); // skip-keyword-check

  QVariant result = expr.evaluate( &context );
  convertedValue = result.toDouble( &ok );
  if ( expr.hasEvalError() || !ok )
  {
    return fallbackValue;
  }
  return convertedValue;
}

QString QgsExpression::helpText( QString name )
{
  QgsExpression::initFunctionHelp();

  if ( !sFunctionHelpTexts.contains( name ) )
    return tr( "function help for %1 missing" ).arg( name );

  const Help &f = sFunctionHelpTexts[ name ];

  name = f.mName;
  if ( f.mType == tr( "group" ) )
  {
    name = group( name );
    name = name.toLower();
  }

  name = name.toHtmlEscaped();

  QString helpContents( u"<h3>%1</h3>\n<div class=\"description\"><p>%2</p></div>"_s
                        .arg( tr( "%1 %2" ).arg( f.mType, name ),
                              f.mDescription ) );

  for ( const HelpVariant &v : std::as_const( f.mVariants ) )
  {
    if ( f.mVariants.size() > 1 )
    {
      helpContents += u"<h3>%1</h3>\n<div class=\"description\">%2</p></div>"_s.arg( v.mName, v.mDescription );
    }

    if ( f.mType != tr( "group" ) && f.mType != tr( "expression" ) )
      helpContents += u"<h4>%1</h4>\n<div class=\"syntax\">\n"_s.arg( tr( "Syntax" ) );

    if ( f.mType == tr( "operator" ) )
    {
      if ( v.mArguments.size() == 1 )
      {
        helpContents += u"<code><span class=\"functionname\">%1</span> <span class=\"argument\">%2</span></code>"_s
                        .arg( name, v.mArguments[0].mArg );
      }
      else if ( v.mArguments.size() == 2 )
      {
        helpContents += u"<code><span class=\"argument\">%1</span> <span class=\"functionname\">%2</span> <span class=\"argument\">%3</span></code>"_s
                        .arg( v.mArguments[0].mArg, name, v.mArguments[1].mArg );
      }
    }
    else if ( f.mType != tr( "group" ) && f.mType != tr( "expression" ) )
    {
      helpContents += u"<code><span class=\"functionname\">%1</span>"_s.arg( name );

      bool hasOptionalArgs = false;

      if ( f.mType == tr( "function" ) && ( f.mName[0] != '$' || !v.mArguments.isEmpty() || v.mVariableLenArguments ) )
      {
        helpContents += '(';

        QString delim;
        for ( const HelpArg &a : std::as_const( v.mArguments ) )
        {
          if ( !a.mDescOnly )
          {
            if ( a.mOptional )
            {
              hasOptionalArgs = true;
              helpContents += QLatin1Char( '[' );
            }

            helpContents += delim;
            helpContents += u"<span class=\"argument\">%2%3</span>"_s.arg(
                              a.mArg,
                              a.mDefaultVal.isEmpty() ? QString() : ":=" + a.mDefaultVal
                            );

            if ( a.mOptional )
              helpContents += QLatin1Char( ']' );
          }
          delim = u","_s;
        }

        if ( v.mVariableLenArguments )
        {
          helpContents += QChar( 0x2026 );
        }

        helpContents += ')';
      }

      helpContents += "</code>"_L1;

      if ( hasOptionalArgs )
      {
        helpContents += "<br/><br/>"_L1 + tr( "[ ] marks optional components" );
      }
    }

    if ( !v.mArguments.isEmpty() )
    {
      helpContents += u"<h4>%1</h4>\n<div class=\"arguments\">\n<table>"_s.arg( tr( "Arguments" ) );

      for ( const HelpArg &a : std::as_const( v.mArguments ) )
      {
        if ( a.mSyntaxOnly )
          continue;

        helpContents += u"<tr><td class=\"argument\">%1</td><td>%2</td></tr>"_s.arg( a.mArg, a.mDescription );
      }

      helpContents += "</table>\n</div>\n"_L1;
    }

    if ( !v.mExamples.isEmpty() )
    {
      helpContents += u"<h4>%1</h4>\n<div class=\"examples\">\n<ul>\n"_s.arg( tr( "Examples" ) );

      for ( const HelpExample &e : std::as_const( v.mExamples ) )
      {
        helpContents += "<li><code>" + e.mExpression + "</code> &rarr; <code>" + e.mReturns + "</code>";

        if ( !e.mNote.isEmpty() )
          helpContents += u" (%1)"_s.arg( e.mNote );

        helpContents += "</li>\n"_L1;
      }

      helpContents += "</ul>\n</div>\n"_L1;
    }

    if ( !v.mNotes.isEmpty() )
    {
      helpContents += u"<h4>%1</h4>\n<div class=\"notes\"><p>%2</p></div>\n"_s.arg( tr( "Notes" ), v.mNotes );
    }
  }

  return helpContents;
}

QStringList QgsExpression::tags( const QString &name )
{
  QStringList tags = QStringList();

  QgsExpression::initFunctionHelp();

  if ( sFunctionHelpTexts.contains( name ) )
  {
    const Help &f = sFunctionHelpTexts[ name ];

    for ( const HelpVariant &v : std::as_const( f.mVariants ) )
    {
      tags << v.mTags;
    }
  }

  return tags;
}

void QgsExpression::initVariableHelp()
{
  static std::once_flag initialized;
  std::call_once( initialized, buildVariableHelp );
}

void QgsExpression::buildVariableHelp()
{
  //global variables
  sVariableHelpTexts()->insert( u"qgis_version"_s, QCoreApplication::translate( "variable_help", "Current QGIS version string." ) );
  sVariableHelpTexts()->insert( u"qgis_version_no"_s, QCoreApplication::translate( "variable_help", "Current QGIS version number." ) );
  sVariableHelpTexts()->insert( u"qgis_release_name"_s, QCoreApplication::translate( "variable_help", "Current QGIS release name." ) );
  sVariableHelpTexts()->insert( u"qgis_short_version"_s, QCoreApplication::translate( "variable_help", "Short QGIS version string." ) );
  sVariableHelpTexts()->insert( u"qgis_os_name"_s, QCoreApplication::translate( "variable_help", "Operating system name, e.g., 'windows', 'linux' or 'osx'." ) );
  sVariableHelpTexts()->insert( u"qgis_platform"_s, QCoreApplication::translate( "variable_help", "QGIS platform, e.g., 'desktop' or 'server'." ) );
  sVariableHelpTexts()->insert( u"qgis_locale"_s, QCoreApplication::translate( "variable_help", "Two letter identifier for current QGIS locale." ) );
  sVariableHelpTexts()->insert( u"user_account_name"_s, QCoreApplication::translate( "variable_help", "Current user's operating system account name." ) );
  sVariableHelpTexts()->insert( u"user_full_name"_s, QCoreApplication::translate( "variable_help", "Current user's operating system user name (if available)." ) );

  //project variables
  sVariableHelpTexts()->insert( u"project_title"_s, QCoreApplication::translate( "variable_help", "Title of current project." ) );
  sVariableHelpTexts()->insert( u"project_path"_s, QCoreApplication::translate( "variable_help", "Full path (including file name) of current project." ) );
  sVariableHelpTexts()->insert( u"project_folder"_s, QCoreApplication::translate( "variable_help", "Folder for current project." ) );
  sVariableHelpTexts()->insert( u"project_filename"_s, QCoreApplication::translate( "variable_help", "Filename of current project." ) );
  sVariableHelpTexts()->insert( u"project_basename"_s, QCoreApplication::translate( "variable_help", "Base name of current project's filename (without path and extension)." ) );
  sVariableHelpTexts()->insert( u"project_home"_s, QCoreApplication::translate( "variable_help", "Home path of current project." ) );
  sVariableHelpTexts()->insert( u"project_crs"_s, QCoreApplication::translate( "variable_help", "Identifier for the coordinate reference system of project (e.g., 'EPSG:4326')." ) );
  sVariableHelpTexts()->insert( u"project_crs_definition"_s, QCoreApplication::translate( "variable_help", "Coordinate reference system of project (full definition)." ) );
  sVariableHelpTexts()->insert( u"project_units"_s, QCoreApplication::translate( "variable_help", "Unit of the project's CRS." ) );
  sVariableHelpTexts()->insert( u"project_crs_description"_s, QCoreApplication::translate( "variable_help", "Name of the coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( u"project_crs_acronym"_s, QCoreApplication::translate( "variable_help", "Acronym of the coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( u"project_crs_ellipsoid"_s, QCoreApplication::translate( "variable_help", "Acronym of the ellipsoid of the coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( u"project_crs_proj4"_s, QCoreApplication::translate( "variable_help", "Proj4 definition of the coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( u"project_crs_wkt"_s, QCoreApplication::translate( "variable_help", "WKT definition of the coordinate reference system of the project." ) );

  sVariableHelpTexts()->insert( u"project_vertical_crs"_s, QCoreApplication::translate( "variable_help", "Identifier for the vertical coordinate reference system of the project (e.g., 'EPSG:5703')." ) );
  sVariableHelpTexts()->insert( u"project_vertical_crs_definition"_s, QCoreApplication::translate( "variable_help", "Vertical coordinate reference system of project (full definition)." ) );
  sVariableHelpTexts()->insert( u"project_vertical_crs_description"_s, QCoreApplication::translate( "variable_help", "Name of the vertical coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( u"project_vertical_crs_wkt"_s, QCoreApplication::translate( "variable_help", "WKT definition of the vertical coordinate reference system of the project." ) );

  sVariableHelpTexts()->insert( u"project_author"_s, QCoreApplication::translate( "variable_help", "Project author, taken from project metadata." ) );
  sVariableHelpTexts()->insert( u"project_abstract"_s, QCoreApplication::translate( "variable_help", "Project abstract, taken from project metadata." ) );
  sVariableHelpTexts()->insert( u"project_creation_date"_s, QCoreApplication::translate( "variable_help", "Project creation date, taken from project metadata." ) );
  sVariableHelpTexts()->insert( u"project_identifier"_s, QCoreApplication::translate( "variable_help", "Project identifier, taken from project metadata." ) );
  sVariableHelpTexts()->insert( u"project_last_saved"_s, QCoreApplication::translate( "variable_help", "Date/time when project was last saved." ) );
  sVariableHelpTexts()->insert( u"project_keywords"_s, QCoreApplication::translate( "variable_help", "Project keywords, taken from project metadata." ) );
  sVariableHelpTexts()->insert( u"project_area_units"_s, QCoreApplication::translate( "variable_help", "Area unit for current project, used when calculating areas of geometries." ) );
  sVariableHelpTexts()->insert( u"project_distance_units"_s, QCoreApplication::translate( "variable_help", "Distance unit for current project, used when calculating lengths of geometries." ) );
  sVariableHelpTexts()->insert( u"project_ellipsoid"_s, QCoreApplication::translate( "variable_help", "Name of ellipsoid of current project, used when calculating geodetic areas and lengths of geometries." ) );
  sVariableHelpTexts()->insert( u"layer_ids"_s, QCoreApplication::translate( "variable_help", "List of all map layer IDs from the current project." ) );
  sVariableHelpTexts()->insert( u"layers"_s, QCoreApplication::translate( "variable_help", "List of all map layers from the current project." ) );

  //layer variables
  sVariableHelpTexts()->insert( u"layer_name"_s, QCoreApplication::translate( "variable_help", "Name of current layer." ) );
  sVariableHelpTexts()->insert( u"layer_id"_s, QCoreApplication::translate( "variable_help", "ID of current layer." ) );
  sVariableHelpTexts()->insert( u"layer_crs"_s, QCoreApplication::translate( "variable_help", "CRS Authority ID of current layer." ) );
  sVariableHelpTexts()->insert( u"layer"_s, QCoreApplication::translate( "variable_help", "The current layer." ) );
  sVariableHelpTexts()->insert( u"layer_crs_ellipsoid"_s, QCoreApplication::translate( "variable_help", "Ellipsoid acronym of current layer CRS." ) );

  sVariableHelpTexts()->insert( u"layer_vertical_crs"_s, QCoreApplication::translate( "variable_help", "Identifier for the vertical coordinate reference system of the layer (e.g., 'EPSG:5703')." ) );
  sVariableHelpTexts()->insert( u"layer_vertical_crs_definition"_s, QCoreApplication::translate( "variable_help", "Vertical coordinate reference system of layer (full definition)." ) );
  sVariableHelpTexts()->insert( u"layer_vertical_crs_description"_s, QCoreApplication::translate( "variable_help", "Name of the vertical coordinate reference system of the layer." ) );
  sVariableHelpTexts()->insert( u"layer_vertical_crs_wkt"_s, QCoreApplication::translate( "variable_help", "WKT definition of the vertical coordinate reference system of the layer." ) );

  //feature variables
  sVariableHelpTexts()->insert( u"feature"_s, QCoreApplication::translate( "variable_help", "The current feature being evaluated. This can be used with the 'attribute' function to evaluate attribute values from the current feature." ) );
  sVariableHelpTexts()->insert( u"id"_s, QCoreApplication::translate( "variable_help", "The ID of the current feature being evaluated." ) );
  sVariableHelpTexts()->insert( u"geometry"_s, QCoreApplication::translate( "variable_help", "The geometry of the current feature being evaluated." ) );

  //composition variables
  sVariableHelpTexts()->insert( u"layout_name"_s, QCoreApplication::translate( "variable_help", "Name of composition." ) );
  sVariableHelpTexts()->insert( u"layout_numpages"_s, QCoreApplication::translate( "variable_help", "Number of pages in composition." ) );
  sVariableHelpTexts()->insert( u"layout_page"_s, QCoreApplication::translate( "variable_help", "Current page number in composition." ) );
  sVariableHelpTexts()->insert( u"layout_pageheight"_s, QCoreApplication::translate( "variable_help", "Composition page height in mm (or specified custom units)." ) );
  sVariableHelpTexts()->insert( u"layout_pagewidth"_s, QCoreApplication::translate( "variable_help", "Composition page width in mm (or specified custom units)." ) );
  sVariableHelpTexts()->insert( u"layout_pageoffsets"_s, QCoreApplication::translate( "variable_help", "Array of Y coordinate of the top of each page." ) );
  sVariableHelpTexts()->insert( u"layout_dpi"_s, QCoreApplication::translate( "variable_help", "Composition resolution (DPI)." ) );

  //atlas variables
  sVariableHelpTexts()->insert( u"atlas_layerid"_s, QCoreApplication::translate( "variable_help", "Current atlas coverage layer ID." ) );
  sVariableHelpTexts()->insert( u"atlas_layername"_s, QCoreApplication::translate( "variable_help", "Current atlas coverage layer name." ) );
  sVariableHelpTexts()->insert( u"atlas_totalfeatures"_s, QCoreApplication::translate( "variable_help", "Total number of features in atlas." ) );
  sVariableHelpTexts()->insert( u"atlas_featurenumber"_s, QCoreApplication::translate( "variable_help", "Current atlas feature number." ) );
  sVariableHelpTexts()->insert( u"atlas_filename"_s, QCoreApplication::translate( "variable_help", "Current atlas file name." ) );
  sVariableHelpTexts()->insert( u"atlas_pagename"_s, QCoreApplication::translate( "variable_help", "Current atlas page name." ) );
  sVariableHelpTexts()->insert( u"atlas_feature"_s, QCoreApplication::translate( "variable_help", "Current atlas feature (as feature object)." ) );
  sVariableHelpTexts()->insert( u"atlas_featureid"_s, QCoreApplication::translate( "variable_help", "Current atlas feature ID." ) );
  sVariableHelpTexts()->insert( u"atlas_geometry"_s, QCoreApplication::translate( "variable_help", "Current atlas feature geometry." ) );

  //layout item variables
  sVariableHelpTexts()->insert( u"item_id"_s, QCoreApplication::translate( "variable_help", "Layout item user-assigned ID (not necessarily unique)." ) );
  sVariableHelpTexts()->insert( u"item_uuid"_s, QCoreApplication::translate( "variable_help", "layout item unique ID." ) );
  sVariableHelpTexts()->insert( u"item_left"_s, QCoreApplication::translate( "variable_help", "Left position of layout item (in mm)." ) );
  sVariableHelpTexts()->insert( u"item_top"_s, QCoreApplication::translate( "variable_help", "Top position of layout item (in mm)." ) );
  sVariableHelpTexts()->insert( u"item_width"_s, QCoreApplication::translate( "variable_help", "Width of layout item (in mm)." ) );
  sVariableHelpTexts()->insert( u"item_height"_s, QCoreApplication::translate( "variable_help", "Height of layout item (in mm)." ) );

  //map settings item variables
  sVariableHelpTexts()->insert( u"map_id"_s, QCoreApplication::translate( "variable_help", "ID of current map destination. This will be 'canvas' for canvas renders, and the item ID for layout map renders." ) );
  sVariableHelpTexts()->insert( u"map_rotation"_s, QCoreApplication::translate( "variable_help", "Current rotation of map." ) );
  sVariableHelpTexts()->insert( u"map_scale"_s, QCoreApplication::translate( "variable_help", "Current scale of map." ) );
  sVariableHelpTexts()->insert( u"map_extent"_s, QCoreApplication::translate( "variable_help", "Geometry representing the current extent of the map." ) );
  sVariableHelpTexts()->insert( u"map_extent_center"_s, QCoreApplication::translate( "variable_help", "Center of map." ) );
  sVariableHelpTexts()->insert( u"map_extent_width"_s, QCoreApplication::translate( "variable_help", "Width of map." ) );
  sVariableHelpTexts()->insert( u"map_extent_height"_s, QCoreApplication::translate( "variable_help", "Height of map." ) );
  sVariableHelpTexts()->insert( u"map_crs"_s, QCoreApplication::translate( "variable_help", "Coordinate reference system of map (e.g., 'EPSG:4326')." ) );
  sVariableHelpTexts()->insert( u"map_crs_description"_s, QCoreApplication::translate( "variable_help", "Name of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( u"map_units"_s, QCoreApplication::translate( "variable_help", "Units for map measurements." ) );
  sVariableHelpTexts()->insert( u"map_crs_definition"_s, QCoreApplication::translate( "variable_help", "Coordinate reference system of the map (full definition)." ) );
  sVariableHelpTexts()->insert( u"map_crs_acronym"_s, QCoreApplication::translate( "variable_help", "Acronym of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( u"map_crs_projection"_s, QCoreApplication::translate( "variable_help", "Projection method used by the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( u"map_crs_ellipsoid"_s, QCoreApplication::translate( "variable_help", "Acronym of the ellipsoid of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( u"map_crs_proj4"_s, QCoreApplication::translate( "variable_help", "Proj4 definition of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( u"map_crs_wkt"_s, QCoreApplication::translate( "variable_help", "WKT definition of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( u"map_layer_ids"_s, QCoreApplication::translate( "variable_help", "List of map layer IDs visible in the map." ) );
  sVariableHelpTexts()->insert( u"map_layers"_s, QCoreApplication::translate( "variable_help", "List of map layers visible in the map." ) );

  sVariableHelpTexts()->insert( u"map_start_time"_s, QCoreApplication::translate( "variable_help", "Start of the map's temporal time range (as a datetime value)" ) );
  sVariableHelpTexts()->insert( u"map_end_time"_s, QCoreApplication::translate( "variable_help", "End of the map's temporal time range (as a datetime value)" ) );
  sVariableHelpTexts()->insert( u"map_interval"_s, QCoreApplication::translate( "variable_help", "Duration of the map's temporal time range (as an interval value)" ) );
  sVariableHelpTexts()->insert( u"map_z_range_lower"_s, QCoreApplication::translate( "variable_help", "Lower elevation of the map's elevation range" ) );
  sVariableHelpTexts()->insert( u"map_z_range_upper"_s, QCoreApplication::translate( "variable_help", "Upper elevation of the map's elevation range" ) );

  sVariableHelpTexts()->insert( u"frame_rate"_s, QCoreApplication::translate( "variable_help", "Number of frames per second during animation playback" ) );
  sVariableHelpTexts()->insert( u"frame_number"_s, QCoreApplication::translate( "variable_help", "Current frame number during animation playback" ) );
  sVariableHelpTexts()->insert( u"frame_duration"_s, QCoreApplication::translate( "variable_help", "Temporal duration of each animation frame (as an interval value)" ) );
  sVariableHelpTexts()->insert( u"frame_timestep"_s, QCoreApplication::translate( "variable_help", "Frame time step during animation playback" ) );
  sVariableHelpTexts()->insert( u"frame_timestep_unit"_s, QCoreApplication::translate( "variable_help", "Unit value of the frame time step during animation playback" ) );
  sVariableHelpTexts()->insert( u"frame_timestep_units"_s, QCoreApplication::translate( "variable_help", "String representation of the frame time step unit during animation playback" ) );
  sVariableHelpTexts()->insert( u"animation_start_time"_s, QCoreApplication::translate( "variable_help", "Start of the animation's overall temporal time range (as a datetime value)" ) );
  sVariableHelpTexts()->insert( u"animation_end_time"_s, QCoreApplication::translate( "variable_help", "End of the animation's overall temporal time range (as a datetime value)" ) );
  sVariableHelpTexts()->insert( u"animation_interval"_s, QCoreApplication::translate( "variable_help", "Duration of the animation's overall temporal time range (as an interval value)" ) );

  // vector tile layer variables
  sVariableHelpTexts()->insert( u"zoom_level"_s, QCoreApplication::translate( "variable_help", "Vector tile zoom level of the map that is being rendered (derived from the current map scale). Normally in interval [0, 20]." ) );
  sVariableHelpTexts()->insert( u"vector_tile_zoom"_s, QCoreApplication::translate( "variable_help", "Exact vector tile zoom level of the map that is being rendered (derived from the current map scale). Normally in interval [0, 20]. Unlike @zoom_level, this variable is a floating point value which can be used to interpolate values between two integer zoom levels." ) );

  sVariableHelpTexts()->insert( u"row_number"_s, QCoreApplication::translate( "variable_help", "Stores the number of the current row." ) + u"\n\n"_s + QCoreApplication::translate( "variable_help", "When used for calculations within the attribute table the row number will respect the original order of features from the underlying data source." ) + u"\n\n"_s + QCoreApplication::translate( "variable_help", "When used from the field calculator the row numbering starts at 1, otherwise (e.g. from Processing tools) the row numbering starts from 0." ) );
  sVariableHelpTexts()->insert( u"grid_number"_s, QCoreApplication::translate( "variable_help", "Current grid annotation value." ) );
  sVariableHelpTexts()->insert( u"grid_axis"_s, QCoreApplication::translate( "variable_help", "Current grid annotation axis (e.g., 'x' for longitude, 'y' for latitude)." ) );
  sVariableHelpTexts()->insert( u"grid_count"_s, QCoreApplication::translate( "variable_help", "Total number of visible grid lines for the current grid axis." ) );
  sVariableHelpTexts()->insert( u"grid_index"_s, QCoreApplication::translate( "variable_help", "The index of the grid line currently being drawn (starting at 1 for the first grid line). The index is specific to the current grid axis." ) );
  sVariableHelpTexts()->insert( u"column_number"_s, QCoreApplication::translate( "variable_help", "Stores the number of the current column." ) );

  // map canvas item variables
  sVariableHelpTexts()->insert( u"canvas_cursor_point"_s, QCoreApplication::translate( "variable_help", "Last cursor position on the canvas in the project's geographical coordinates." ) );
  sVariableHelpTexts()->insert( u"layer_cursor_point"_s, QCoreApplication::translate( "variable_help", "Last cursor position on the canvas in the current layers's geographical coordinates. QGIS Server: When used in a maptip expression for a raster layer, this variable holds the GetFeatureInfo position." ) );

  // legend canvas item variables
  sVariableHelpTexts()->insert( u"legend_title"_s, QCoreApplication::translate( "variable_help", "Title of the legend." ) );
  sVariableHelpTexts()->insert( u"legend_column_count"_s, QCoreApplication::translate( "variable_help", "Number of column in the legend." ) );
  sVariableHelpTexts()->insert( u"legend_split_layers"_s, QCoreApplication::translate( "variable_help", "Boolean indicating if layers can be split in the legend." ) );
  sVariableHelpTexts()->insert( u"legend_wrap_string"_s, QCoreApplication::translate( "variable_help", "Characters used to wrap the legend text." ) );
  sVariableHelpTexts()->insert( u"legend_filter_by_map"_s, QCoreApplication::translate( "variable_help", "Boolean indicating if the content of the legend is filtered by the map." ) );
  sVariableHelpTexts()->insert( u"legend_filter_out_atlas"_s, QCoreApplication::translate( "variable_help", "Boolean indicating if the Atlas is filtered out of the legend." ) );

  // scalebar rendering
  sVariableHelpTexts()->insert( u"scale_value"_s, QCoreApplication::translate( "variable_help", "Current scale bar distance value." ) );

  // map tool capture variables
  sVariableHelpTexts()->insert( u"snapping_results"_s, QCoreApplication::translate( "variable_help",
                                "<p>An array with an item for each snapped point.</p>"
                                "<p>Each item is a map with the following keys:</p>"
                                "<dl>"
                                "<dt>valid</dt><dd>Boolean that indicates if the snapping result is valid</dd>"
                                "<dt>layer</dt><dd>The layer on which the snapped feature is</dd>"
                                "<dt>feature_id</dt><dd>The feature id of the snapped feature</dd>"
                                "<dt>vertex_index</dt><dd>The index of the snapped vertex</dd>"
                                "<dt>distance</dt><dd>The distance between the mouse cursor and the snapped point at the time of snapping</dd>"
                                "</dl>" ) );


  //symbol variables
  sVariableHelpTexts()->insert( u"geometry_part_count"_s, QCoreApplication::translate( "variable_help", "Number of parts in rendered feature's geometry." ) );
  sVariableHelpTexts()->insert( u"geometry_part_num"_s, QCoreApplication::translate( "variable_help", "Current geometry part number for feature being rendered." ) );
  sVariableHelpTexts()->insert( u"geometry_ring_num"_s, QCoreApplication::translate( "variable_help", "Current geometry ring number for feature being rendered (for polygon features only). The exterior ring has a value of 0." ) );
  sVariableHelpTexts()->insert( u"geometry_point_count"_s, QCoreApplication::translate( "variable_help", "Number of points in the rendered geometry's part. It is only meaningful for line geometries and for symbol layers that set this variable." ) );
  sVariableHelpTexts()->insert( u"geometry_point_num"_s, QCoreApplication::translate( "variable_help", "Current point number in the rendered geometry's part. It is only meaningful for line geometries and for symbol layers that set this variable." ) );

  sVariableHelpTexts()->insert( u"symbol_color"_s, QCoreApplication::translate( "symbol_color", "Color of symbol used to render the feature." ) );
  sVariableHelpTexts()->insert( u"symbol_angle"_s, QCoreApplication::translate( "symbol_angle", "Angle of symbol used to render the feature (valid for marker symbols only)." ) );
  sVariableHelpTexts()->insert( u"symbol_layer_count"_s, QCoreApplication::translate( "symbol_layer_count", "Total number of symbol layers in the symbol." ) );
  sVariableHelpTexts()->insert( u"symbol_layer_index"_s, QCoreApplication::translate( "symbol_layer_index", "Current symbol layer index." ) );
  sVariableHelpTexts()->insert( u"symbol_marker_row"_s, QCoreApplication::translate( "symbol_marker_row", "Row number for marker (valid for point pattern fills only)." ) );
  sVariableHelpTexts()->insert( u"symbol_marker_column"_s, QCoreApplication::translate( "symbol_marker_column", "Column number for marker (valid for point pattern fills only)." ) );
  sVariableHelpTexts()->insert( u"symbol_frame"_s, QCoreApplication::translate( "symbol_frame", "Frame number (for animated symbols only)." ) );

  sVariableHelpTexts()->insert( u"symbol_label"_s, QCoreApplication::translate( "symbol_label", "Label for the symbol (either a user defined label or the default autogenerated label)." ) );
  sVariableHelpTexts()->insert( u"symbol_id"_s, QCoreApplication::translate( "symbol_id", "Internal ID of the symbol." ) );
  sVariableHelpTexts()->insert( u"symbol_count"_s, QCoreApplication::translate( "symbol_count", "Total number of features represented by the symbol." ) );

  //cluster variables
  sVariableHelpTexts()->insert( u"cluster_color"_s, QCoreApplication::translate( "cluster_color", "Color of symbols within a cluster, or NULL if symbols have mixed colors." ) );
  sVariableHelpTexts()->insert( u"cluster_size"_s, QCoreApplication::translate( "cluster_size", "Number of symbols contained within a cluster." ) );

  //processing variables
  sVariableHelpTexts()->insert( u"algorithm_id"_s, QCoreApplication::translate( "algorithm_id", "Unique ID for algorithm." ) );
  sVariableHelpTexts()->insert( u"model_path"_s, QCoreApplication::translate( "variable_help", "Full path (including file name) of current model (or project path if model is embedded in a project)." ) );
  sVariableHelpTexts()->insert( u"model_folder"_s, QCoreApplication::translate( "variable_help", "Folder containing current model (or project folder if model is embedded in a project)." ) );
  sVariableHelpTexts()->insert( u"model_name"_s, QCoreApplication::translate( "variable_help", "Name of current model." ) );
  sVariableHelpTexts()->insert( u"model_group"_s, QCoreApplication::translate( "variable_help", "Group for current model." ) );
  sVariableHelpTexts()->insert( u"fullextent_minx"_s, QCoreApplication::translate( "fullextent_minx", "Minimum x-value from full canvas extent (including all layers)." ) );
  sVariableHelpTexts()->insert( u"fullextent_miny"_s, QCoreApplication::translate( "fullextent_miny", "Minimum y-value from full canvas extent (including all layers)." ) );
  sVariableHelpTexts()->insert( u"fullextent_maxx"_s, QCoreApplication::translate( "fullextent_maxx", "Maximum x-value from full canvas extent (including all layers)." ) );
  sVariableHelpTexts()->insert( u"fullextent_maxy"_s, QCoreApplication::translate( "fullextent_maxy", "Maximum y-value from full canvas extent (including all layers)." ) );

  //provider notification
  sVariableHelpTexts()->insert( u"notification_message"_s, QCoreApplication::translate( "notification_message", "Content of the notification message sent by the provider (available only for actions triggered by provider notifications)." ) );

  //form context variable
  sVariableHelpTexts()->insert( u"current_geometry"_s, QCoreApplication::translate( "current_geometry", "Represents the geometry of the feature currently being edited in the form or the table row. Can be used in a form/row context to filter the related features." ) );
  sVariableHelpTexts()->insert( u"current_feature"_s, QCoreApplication::translate( "current_feature", "Represents the feature currently being edited in the form or the table row. Can be used in a form/row context to filter the related features." ) );

  //parent form context variable
  sVariableHelpTexts()->insert( u"current_parent_geometry"_s, QCoreApplication::translate( "current_parent_geometry",
                                "Only usable in an embedded form context, "
                                "represents the geometry of the feature currently being edited in the parent form.\n"
                                "Can be used in a form/row context to filter the related features using a value "
                                "from the feature currently edited in the parent form, to make sure that the filter "
                                "still works with standalone forms it is recommended to wrap this variable in a "
                                "'coalesce()'." ) );
  sVariableHelpTexts()->insert( u"current_parent_feature"_s, QCoreApplication::translate( "current_parent_feature",
                                "Only usable in an embedded form context, "
                                "represents the feature currently being edited in the parent form.\n"
                                "Can be used in a form/row context to filter the related features using a value "
                                "from the feature currently edited in the parent form, to make sure that the filter "
                                "still works with standalone forms it is recommended to wrap this variable in a "
                                "'coalesce()'." ) );

  //form variable
  sVariableHelpTexts()->insert( u"form_mode"_s, QCoreApplication::translate( "form_mode", "What the form is used for, like AddFeatureMode, SingleEditMode, MultiEditMode, SearchMode, AggregateSearchMode or IdentifyMode as string." ) );

  // plots and charts
  sVariableHelpTexts()->insert( u"plot_axis"_s, QCoreApplication::translate( "plot_axis", "The associated plot axis, e.g. 'x' or 'y'." ) );
  sVariableHelpTexts()->insert( u"plot_axis_value"_s, QCoreApplication::translate( "plot_axis_value", "The current value for the plot axis grid line." ) );
  sVariableHelpTexts()->insert( u"chart_category"_s, QCoreApplication::translate( "plot_axis", "The chart item category, e.g. 'fruit' or 'june'." ) );
  sVariableHelpTexts()->insert( u"chart_value"_s, QCoreApplication::translate( "plot_axis_value", "The chart item value." ) );
}

bool QgsExpression::addVariableHelpText( const QString name, const QString &description )
{
  QgsExpression::initVariableHelp();
  if ( sVariableHelpTexts()->contains( name ) )
  {
    return false;
  }
  sVariableHelpTexts()->insert( name, description );
  return true;
}

QString QgsExpression::variableHelpText( const QString &variableName )
{
  QgsExpression::initVariableHelp();
  return sVariableHelpTexts()->value( variableName, QString() );
}

QString QgsExpression::formatVariableHelp( const QString &description, bool showValue, const QVariant &value )
{
  QString text = !description.isEmpty() ? u"<p>%1</p>"_s.arg( description ) : QString();
  if ( showValue )
  {
    QString valueString;
    if ( !value.isValid() )
    {
      valueString = QCoreApplication::translate( "variable_help", "not set" );
    }
    else
    {
      valueString = u"<b>%1</b>"_s.arg( formatPreviewString( value ) );
    }
    text.append( QCoreApplication::translate( "variable_help", "<p>Current value: %1</p>" ).arg( valueString ) );
  }
  return text;
}

QString QgsExpression::group( const QString &name )
{
  if ( sGroups()->isEmpty() )
  {
    sGroups()->insert( u"Aggregates"_s, tr( "Aggregates" ) );
    sGroups()->insert( u"Arrays"_s, tr( "Arrays" ) );
    sGroups()->insert( u"Color"_s, tr( "Color" ) );
    sGroups()->insert( u"Conditionals"_s, tr( "Conditionals" ) );
    sGroups()->insert( u"Conversions"_s, tr( "Conversions" ) );
    sGroups()->insert( u"Date and Time"_s, tr( "Date and Time" ) );
    sGroups()->insert( u"Fields and Values"_s, tr( "Fields and Values" ) );
    sGroups()->insert( u"Files and Paths"_s, tr( "Files and Paths" ) );
    sGroups()->insert( u"Fuzzy Matching"_s, tr( "Fuzzy Matching" ) );
    sGroups()->insert( u"General"_s, tr( "General" ) );
    sGroups()->insert( u"GeometryGroup"_s, tr( "Geometry" ) );
    sGroups()->insert( u"Map Layers"_s, tr( "Map Layers" ) );
    sGroups()->insert( u"Maps"_s, tr( "Maps" ) );
    sGroups()->insert( u"Math"_s, tr( "Math" ) );
    sGroups()->insert( u"Operators"_s, tr( "Operators" ) );
    sGroups()->insert( u"Rasters"_s, tr( "Rasters" ) );
    sGroups()->insert( u"Record and Attributes"_s, tr( "Record and Attributes" ) );
    sGroups()->insert( u"String"_s, tr( "String" ) );
    sGroups()->insert( u"Variables"_s, tr( "Variables" ) );
    sGroups()->insert( u"Recent (%1)"_s, tr( "Recent (%1)" ) );
    sGroups()->insert( u"UserGroup"_s, tr( "User expressions" ) );
  }

  //return the translated name for this group. If group does not
  //have a translated name in the gGroups hash, return the name
  //unchanged
  return sGroups()->value( name, name );
}

QString QgsExpression::formatPreviewString( const QVariant &value, const bool htmlOutput, int maximumPreviewLength )
{
  const QString startToken = htmlOutput ? u"<i>&lt;"_s : u"<"_s;
  const QString endToken = htmlOutput ? u"&gt;</i>"_s : u">"_s;

  QgsGeometry geom = QgsExpressionUtils::getGeometry( value, nullptr );
  if ( !geom.isNull() )
  {
    //result is a geometry
    if ( geom.isNull() )
      return startToken + tr( "empty geometry" ) + endToken;
    else
      return startToken + tr( "geometry: %1" ).arg( QgsWkbTypes::displayString( geom.constGet()->wkbType() ) )
             + endToken;
  }
  else if ( value.value< QgsWeakMapLayerPointer >().data() )
  {
    return startToken + tr( "map layer" ) + endToken;
  }
  else if ( !value.isValid() )
  {
    return htmlOutput ? tr( "<i>NULL</i>" ) : QString();
  }
  else if ( value.userType() == qMetaTypeId< QgsFeature>() )
  {
    //result is a feature
    QgsFeature feat = value.value<QgsFeature>();
    return startToken + tr( "feature: %1" ).arg( feat.id() ) + endToken;
  }
  else if ( value.userType() == qMetaTypeId< QgsCoordinateReferenceSystem>() )
  {
    const QgsCoordinateReferenceSystem crs = value.value<QgsCoordinateReferenceSystem>();
    return startToken + tr( "crs: %1" ).arg( crs.userFriendlyIdentifier() ) + endToken;
  }
  else if ( value.userType() == qMetaTypeId< QTimeZone>() )
  {
    const QTimeZone tz = value.value<QTimeZone>();
#if QT_FEATURE_timezone > 0
    return startToken + tr( "time zone: %1" ).arg( tz.isValid() ? tz.displayName( QTimeZone::GenericTime, QTimeZone::ShortName ) : tr( "invalid" ) ) + endToken;
#else
    QgsDebugError( u"Qt is built without Qt timezone support, timezone preview not available"_s );
#endif
  }
  else if ( value.userType() == qMetaTypeId< QgsInterval>() )
  {
    QgsInterval interval = value.value<QgsInterval>();
    if ( interval.days() > 1 )
    {
      return startToken + tr( "interval: %1 days" ).arg( interval.days() ) + endToken;
    }
    else if ( interval.hours() > 1 )
    {
      return startToken + tr( "interval: %1 hours" ).arg( interval.hours() ) + endToken;
    }
    else if ( interval.minutes() > 1 )
    {
      return startToken + tr( "interval: %1 minutes" ).arg( interval.minutes() ) + endToken;
    }
    else
    {
      return startToken + tr( "interval: %1 seconds" ).arg( interval.seconds() ) + endToken;
    }
  }
  else if ( value.userType() == qMetaTypeId< QgsGradientColorRamp>() )
  {
    return startToken + tr( "gradient ramp" ) + endToken;
  }
  else if ( value.userType() == QMetaType::Type::QDate )
  {
    const QDate dt = value.toDate();
    return startToken + tr( "date: %1" ).arg( dt.toString( u"yyyy-MM-dd"_s ) ) + endToken;
  }
  else if ( value.userType() == QMetaType::Type::QTime )
  {
    const QTime tm = value.toTime();
    return startToken + tr( "time: %1" ).arg( tm.toString( u"hh:mm:ss"_s ) ) + endToken;
  }
  else if ( value.userType() == QMetaType::Type::QDateTime )
  {
    const QDateTime dt = value.toDateTime();
    return startToken + tr( "datetime: %1 (%2)" ).arg( dt.toString( u"yyyy-MM-dd hh:mm:ss"_s ), dt.timeZoneAbbreviation() ) + endToken;
  }
  else if ( value.userType() == QMetaType::Type::QString )
  {
    const QString previewString = value.toString();
    if ( previewString.length() > maximumPreviewLength + 3 )
    {
      return tr( "'%1…'" ).arg( previewString.left( maximumPreviewLength ) );
    }
    else
    {
      return '\'' + previewString + '\'';
    }
  }
  else if ( value.userType() == QMetaType::Type::QVariantMap )
  {
    QString mapStr = u"{"_s;
    const QVariantMap map = value.toMap();
    QString separator;
    for ( QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
    {
      mapStr.append( separator );
      if ( separator.isEmpty() )
        separator = u","_s;

      mapStr.append( u" '%1': %2"_s.arg( it.key(), formatPreviewString( it.value(), htmlOutput ) ) );
      if ( mapStr.length() > maximumPreviewLength - 3 )
      {
        mapStr = tr( "%1…" ).arg( mapStr.left( maximumPreviewLength - 2 ) );
        break;
      }
    }
    if ( !map.empty() )
      mapStr += QLatin1Char( ' ' );
    mapStr += QLatin1Char( '}' );
    return mapStr;
  }
  else if ( value.userType() == QMetaType::Type::QVariantList || value.userType() == QMetaType::Type::QStringList )
  {
    QString listStr = u"["_s;
    const QVariantList list = value.toList();
    QString separator;
    for ( const QVariant &arrayValue : list )
    {
      listStr.append( separator );
      if ( separator.isEmpty() )
        separator = u","_s;

      listStr.append( " " );
      listStr.append( formatPreviewString( arrayValue, htmlOutput ) );
      if ( listStr.length() > maximumPreviewLength - 3 )
      {
        listStr = QString( tr( "%1…" ) ).arg( listStr.left( maximumPreviewLength - 2 ) );
        break;
      }
    }
    if ( !list.empty() )
      listStr += QLatin1Char( ' ' );
    listStr += QLatin1Char( ']' );
    return listStr;
  }
  else if ( value.type() == QVariant::Color )
  {
    const QColor color = value.value<QColor>();

    if ( !color.isValid() )
    {
      return tr( "<i>Invalid</i>" );
    }

    switch ( color.spec() )
    {
      case QColor::Spec::Cmyk:
        return u"CMYKA: %1,%2,%3,%4,%5"_s
               .arg( color.cyanF(), 0, 'f', 2 ).arg( color.magentaF(), 0, 'f', 2 )
               .arg( color.yellowF(), 0, 'f', 2 ).arg( color.blackF(), 0, 'f', 2 )
               .arg( color.alphaF(), 0, 'f', 2 );

      case QColor::Spec::Hsv:
        return u"HSVA: %1,%2,%3,%4"_s
               .arg( color.hsvHueF(), 0, 'f', 2 ).arg( color.hsvSaturationF(), 0, 'f', 2 )
               .arg( color.valueF(), 0, 'f', 2 ).arg( color.alphaF(), 0, 'f', 2 );

      case QColor::Spec::Hsl:
        return u"HSLA: %1,%2,%3,%4"_s
               .arg( color.hslHueF(), 0, 'f', 2 ).arg( color.hslSaturationF(), 0, 'f', 2 )
               .arg( color.lightnessF(), 0, 'f', 2 ).arg( color.alphaF(), 0, 'f', 2 );

      case QColor::Spec::Rgb:
      case QColor::Spec::ExtendedRgb:
        return u"RGBA: %1,%2,%3,%4"_s
               .arg( color.redF(), 0, 'f', 2 ).arg( color.greenF(), 0, 'f', 2 )
               .arg( color.blueF(), 0, 'f', 2 ).arg( color.alphaF(), 0, 'f', 2 );

      case QColor::Spec::Invalid:
        return tr( "<i>Invalid</i>" );
    }
    QgsDebugError( u"Unknown color format: %1"_s.arg( color.spec() ) );
    return tr( "<i>Unknown color format: %1</i>" ).arg( color.spec() );
  }
  else if ( value.userType() == QMetaType::Type::Int ||
            value.userType() == QMetaType::Type::UInt ||
            value.userType() == QMetaType::Type::LongLong ||
            value.userType() == QMetaType::Type::ULongLong ||
            value.userType() == QMetaType::Type::Double ||
            // Qt madness with QMetaType::Float :/
            value.userType() == static_cast<QMetaType::Type>( QMetaType::Float ) )
  {
    return QgsExpressionUtils::toLocalizedString( value );
  }
  else
  {
    QString str { value.toString() };
    if ( str.length() > maximumPreviewLength - 3 )
    {
      str = tr( "%1…" ).arg( str.left( maximumPreviewLength - 2 ) );
    }
    return str;
  }
}

QString QgsExpression::createFieldEqualityExpression( const QString &fieldName, const QVariant &value, QMetaType::Type fieldType )
{
  QString expr;

  if ( QgsVariantUtils::isNull( value ) )
    expr = u"%1 IS NULL"_s.arg( quotedColumnRef( fieldName ) );
  else if ( fieldType == QMetaType::Type::UnknownType )
    expr = u"%1 = %2"_s.arg( quotedColumnRef( fieldName ), quotedValue( value ) );
  else
    expr = u"%1 = %2"_s.arg( quotedColumnRef( fieldName ), quotedValue( value, fieldType ) );

  return expr;
}

QString QgsExpression::createFieldEqualityExpression( const QString &fieldName, const QVariant &value, QVariant::Type fieldType )
{
  return createFieldEqualityExpression( fieldName, value, QgsVariantUtils::variantTypeToMetaType( fieldType ) );
}

bool QgsExpression::isFieldEqualityExpression( const QString &expression, QString &field, QVariant &value )
{
  QgsExpression e( expression );

  if ( !e.rootNode() )
    return false;

  if ( const QgsExpressionNodeBinaryOperator *binOp = dynamic_cast<const QgsExpressionNodeBinaryOperator *>( e.rootNode() ) )
  {
    if ( binOp->op() == QgsExpressionNodeBinaryOperator::boEQ )
    {
      const QgsExpressionNodeColumnRef *columnRef = dynamic_cast<const QgsExpressionNodeColumnRef *>( binOp->opLeft() );
      const QgsExpressionNodeLiteral *literal = dynamic_cast<const QgsExpressionNodeLiteral *>( binOp->opRight() );
      if ( columnRef && literal )
      {
        field = columnRef->name();
        value = literal->value();
        return true;
      }
    }
  }
  return false;
}

bool QgsExpression::attemptReduceToInClause( const QStringList &expressions, QString &result )
{
  if ( expressions.empty() )
    return false;

  QString inField;
  bool first = true;
  QStringList values;
  for ( const QString &expression : expressions )
  {
    QString field;
    QVariant value;
    if ( QgsExpression::isFieldEqualityExpression( expression, field, value ) )
    {
      if ( first )
      {
        inField = field;
        first = false;
      }
      else if ( field != inField )
      {
        return false;
      }
      values << QgsExpression::quotedValue( value );
    }
    else
    {
      // we also allow reducing similar 'field IN (...)' expressions!
      QgsExpression e( expression );

      if ( !e.rootNode() )
        return false;

      if ( const QgsExpressionNodeInOperator *inOp = dynamic_cast<const QgsExpressionNodeInOperator *>( e.rootNode() ) )
      {
        if ( inOp->isNotIn() )
          return false;

        const QgsExpressionNodeColumnRef *columnRef = dynamic_cast<const QgsExpressionNodeColumnRef *>( inOp->node() );
        if ( !columnRef )
          return false;

        if ( first )
        {
          inField = columnRef->name();
          first = false;
        }
        else if ( columnRef->name() != inField )
        {
          return false;
        }

        if ( QgsExpressionNode::NodeList *nodeList = inOp->list() )
        {
          const QList<QgsExpressionNode *> nodes = nodeList->list();
          for ( const QgsExpressionNode *node : nodes )
          {
            const QgsExpressionNodeLiteral *literal = dynamic_cast<const QgsExpressionNodeLiteral *>( node );
            if ( !literal )
              return false;

            values << QgsExpression::quotedValue( literal->value() );
          }
        }
      }
      // Collect ORs
      else if ( const QgsExpressionNodeBinaryOperator *orOp = dynamic_cast<const QgsExpressionNodeBinaryOperator *>( e.rootNode() ) )
      {

        // OR Collector function: returns a possibly empty list of the left and right operands of an OR expression
        std::function<QStringList( QgsExpressionNode *, QgsExpressionNode * )> collectOrs = [ &collectOrs ]( QgsExpressionNode * opLeft,  QgsExpressionNode * opRight ) -> QStringList
        {
          QStringList orParts;
          if ( const QgsExpressionNodeBinaryOperator *leftOrOp = dynamic_cast<const QgsExpressionNodeBinaryOperator *>( opLeft ) )
          {
            if ( leftOrOp->op( ) == QgsExpressionNodeBinaryOperator::BinaryOperator::boOr )
            {
              orParts.append( collectOrs( leftOrOp->opLeft(), leftOrOp->opRight() ) );
            }
            else
            {
              orParts.append( leftOrOp->dump() );
            }
          }
          else if ( const QgsExpressionNodeInOperator *leftInOp = dynamic_cast<const QgsExpressionNodeInOperator *>( opLeft ) )
          {
            orParts.append( leftInOp->dump() );
          }
          else
          {
            return {};
          }

          if ( const QgsExpressionNodeBinaryOperator *rightOrOp = dynamic_cast<const QgsExpressionNodeBinaryOperator *>( opRight ) )
          {
            if ( rightOrOp->op( ) == QgsExpressionNodeBinaryOperator::BinaryOperator::boOr )
            {
              orParts.append( collectOrs( rightOrOp->opLeft(), rightOrOp->opRight() ) );
            }
            else
            {
              orParts.append( rightOrOp->dump() );
            }
          }
          else if ( const QgsExpressionNodeInOperator *rightInOp = dynamic_cast<const QgsExpressionNodeInOperator *>( opRight ) )
          {
            orParts.append( rightInOp->dump() );
          }
          else
          {
            return {};
          }

          return orParts;
        };

        if ( orOp->op( ) == QgsExpressionNodeBinaryOperator::BinaryOperator::boOr )
        {
          // Try to collect all OR conditions
          const QStringList orParts = collectOrs( orOp->opLeft(), orOp->opRight() );
          if ( orParts.isEmpty() )
          {
            return false;
          }
          else
          {
            QString orPartsResult;
            if ( attemptReduceToInClause( orParts, orPartsResult ) )
            {
              // Need to check if the IN field is correct,
              QgsExpression inExp { orPartsResult };
              if ( ! inExp.rootNode() )
              {
                return false;
              }

              if ( const QgsExpressionNodeInOperator *inOpInner = dynamic_cast<const QgsExpressionNodeInOperator *>( inExp.rootNode() ) )
              {
                if ( inOpInner->node()->nodeType() != QgsExpressionNode::NodeType::ntColumnRef || inOpInner->node()->referencedColumns().size() < 1 )
                {
                  return false;
                }

                const QString innerInfield { inOpInner->node()->referencedColumns().values().first() };

                if ( first )
                {
                  inField = innerInfield;
                  first = false;
                }

                if ( innerInfield != inField )
                {
                  return false;
                }
                else
                {
                  const auto constInnerValuesList { inOpInner->list()->list() };
                  for ( const auto &innerInValueNode : std::as_const( constInnerValuesList ) )
                  {
                    values.append( innerInValueNode->dump() );
                  }
                }

              }
              else
              {
                return false;
              }
            }
            else
            {
              return false;
            }
          }
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
  }
  result = u"%1 IN (%2)"_s.arg( quotedColumnRef( inField ), values.join( ',' ) );
  return true;
}

const QgsExpressionNode *QgsExpression::rootNode() const
{
  return d->mRootNode.get();
}

bool QgsExpression::isField() const
{
  return d->mRootNode && d->mRootNode.get()->nodeType() == QgsExpressionNode::ntColumnRef;
}

int QgsExpression::expressionToLayerFieldIndex( const QString &expression, const QgsVectorLayer *layer )
{
  if ( !layer )
    return -1;

  // easy check first -- lookup field directly.
  int attrIndex = layer->fields().lookupField( expression.trimmed() );
  if ( attrIndex >= 0 )
    return attrIndex;

  // may still be a simple field expression, just one which is enclosed in "" or similar
  QgsExpression candidate( expression );
  if ( candidate.isField() )
  {
    const QString fieldName =  qgis::down_cast<const QgsExpressionNodeColumnRef *>( candidate.rootNode() )->name();
    return layer->fields().lookupField( fieldName );
  }
  return -1;
}

QString QgsExpression::quoteFieldExpression( const QString &expression, const QgsVectorLayer *layer )
{
  if ( !layer )
    return expression;

  const int fieldIndex = QgsExpression::expressionToLayerFieldIndex( expression, layer );
  if ( !expression.contains( '\"' ) && fieldIndex != -1 )
  {
    // retrieve actual field name from layer, so that we correctly remove any unwanted leading/trailing whitespace
    return QgsExpression::quotedColumnRef( layer->fields().at( fieldIndex ).name() );
  }
  else
  {
    return expression;
  }
}

QList<const QgsExpressionNode *> QgsExpression::nodes() const
{
  if ( !d->mRootNode )
    return QList<const QgsExpressionNode *>();

  return d->mRootNode->nodes();
}
