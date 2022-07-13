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
#include "qgsexpressionfunction.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsfeaturerequest.h"
#include "qgscolorrampimpl.h"
#include "qgslogger.h"
#include "qgsexpressioncontext.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionutils.h"
#include "qgsexpression_p.h"

#include <QRegularExpression>

// from parser
extern QgsExpressionNode *parseExpression( const QString &str, QString &parserErrorMsg, QList<QgsExpression::ParserError> &parserErrors );

Q_GLOBAL_STATIC( HelpTextHash, sFunctionHelpTexts )
Q_GLOBAL_STATIC( QgsStringMap, sVariableHelpTexts )
Q_GLOBAL_STATIC( QgsStringMap, sGroups )

HelpTextHash &functionHelpTexts()
{
  return *sFunctionHelpTexts();
}

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
  d->mRootNode = ::parseExpression( expression, d->mParserErrorString, d->mParserErrors );
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
  return QStringLiteral( "\"%1\"" ).arg( name.replace( '\"', QLatin1String( "\"\"" ) ) );
}

QString QgsExpression::quotedString( QString text )
{
  text.replace( '\'', QLatin1String( "''" ) );
  text.replace( '\\', QLatin1String( "\\\\" ) );
  text.replace( '\n', QLatin1String( "\\n" ) );
  text.replace( '\t', QLatin1String( "\\t" ) );
  return QStringLiteral( "'%1'" ).arg( text );
}

QString QgsExpression::quotedValue( const QVariant &value )
{
  return quotedValue( value, value.type() );
}

QString QgsExpression::quotedValue( const QVariant &value, QVariant::Type type )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( type )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      return value.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );

    case QVariant::List:
    case QVariant::StringList:
    {
      QStringList quotedValues;
      const QVariantList values = value.toList();
      quotedValues.reserve( values.count() );
      for ( const QVariant &v : values )
      {
        quotedValues += quotedValue( v );
      }
      return QStringLiteral( "array( %1 )" ).arg( quotedValues.join( QLatin1String( ", " ) ) );
    }

    default:
    case QVariant::String:
      return quotedString( value.toString() );
  }

}

bool QgsExpression::isFunctionName( const QString &name )
{
  return functionIndex( name ) != -1;
}

int QgsExpression::functionIndex( const QString &name )
{
  int count = functionCount();
  for ( int i = 0; i < count; i++ )
  {
    if ( QString::compare( name, QgsExpression::Functions()[i]->name(), Qt::CaseInsensitive ) == 0 )
      return i;
    const QStringList aliases = QgsExpression::Functions()[i]->aliases();
    for ( const QString &alias : aliases )
    {
      if ( QString::compare( name, alias, Qt::CaseInsensitive ) == 0 )
        return i;
    }
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
  d->mRootNode = ::parseExpression( expr, d->mParserErrorString, d->mParserErrors );
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
  return d->mRootNode;
}

bool QgsExpression::hasParserError() const
{
  return d->mParserErrors.count() > 0;
}

QString QgsExpression::parserErrorString() const
{
  return d->mParserErrorString;
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
      referencedIndexes = qgis::listToSet( fields.allAttributesList() );
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
    d->mDaEllipsoid = context->variable( QStringLiteral( "project_ellipsoid" ) ).toString();
    d->mDaCrs = std::make_unique<QgsCoordinateReferenceSystem>( context->variable( QStringLiteral( "_layer_crs" ) ).value<QgsCoordinateReferenceSystem>() );
    d->mDaTransformContext = std::make_unique<QgsCoordinateTransformContext>( context->variable( QStringLiteral( "_project_transform_context" ) ).value<QgsCoordinateTransformContext>() );
  }

  // Set the distance units from the context if it has not been set by setDistanceUnits()
  if ( context && distanceUnits() == QgsUnitTypes::DistanceUnknownUnit )
  {
    QString distanceUnitsStr = context->variable( QStringLiteral( "project_distance_units" ) ).toString();
    if ( ! distanceUnitsStr.isEmpty() )
      setDistanceUnits( QgsUnitTypes::stringToDistanceUnit( distanceUnitsStr ) );
  }

  // Set the area units from the context if it has not been set by setAreaUnits()
  if ( context && areaUnits() == QgsUnitTypes::AreaUnknownUnit )
  {
    QString areaUnitsStr = context->variable( QStringLiteral( "project_area_units" ) ).toString();
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

void QgsExpression::setGeomCalculator( const QgsDistanceArea *calc )
{
  detach();
  if ( calc )
    d->mCalc = std::shared_ptr<QgsDistanceArea>( new QgsDistanceArea( *calc ) );
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
    d->mRootNode = ::parseExpression( d->mExp, d->mParserErrorString, d->mParserErrors );
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
    d->mCalc = std::shared_ptr<QgsDistanceArea>( new QgsDistanceArea() );
    d->mCalc->setEllipsoid( d->mDaEllipsoid.isEmpty() ? geoNone() : d->mDaEllipsoid );
    d->mCalc->setSourceCrs( *d->mDaCrs.get(), *d->mDaTransformContext.get() );
  }

  return d->mCalc.get();
}

QgsUnitTypes::DistanceUnit QgsExpression::distanceUnits() const
{
  return d->mDistanceUnit;
}

void QgsExpression::setDistanceUnits( QgsUnitTypes::DistanceUnit unit )
{
  d->mDistanceUnit = unit;
}

QgsUnitTypes::AreaUnit QgsExpression::areaUnits() const
{
  return d->mAreaUnit;
}

void QgsExpression::setAreaUnits( QgsUnitTypes::AreaUnit unit )
{
  d->mAreaUnit = unit;
}

QString QgsExpression::replaceExpressionText( const QString &action, const QgsExpressionContext *context, const QgsDistanceArea *distanceArea )
{
  QString expr_action;

  int index = 0;
  while ( index < action.size() )
  {
    static const QRegularExpression sRegEx{ QStringLiteral( "\\[%(.*?)%\\]" ),  QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption };

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
      QgsDebugMsg( "Expression parser error: " + exp.parserErrorString() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
      expr_action += action.midRef( start, index - start );
#else
      expr_action += QStringView {action}.mid( start, index - start );
#endif
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
      QgsDebugMsg( "Expression parser eval error: " + exp.evalErrorString() );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
      expr_action += action.midRef( start, index - start );
#else
      expr_action += QStringView {action}.mid( start, index - start );
#endif
      continue;
    }

    QgsDebugMsgLevel( "Expression result is: " + result.toString(), 3 );
    expr_action += action.mid( start, pos - start ) + result.toString();
  }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
  expr_action += action.midRef( index );
#else
  expr_action += QStringView {action}.mid( index ).toString();
#endif

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
          << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

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

  if ( !sFunctionHelpTexts()->contains( name ) )
    return tr( "function help for %1 missing" ).arg( name );

  const Help &f = ( *sFunctionHelpTexts() )[ name ];

  name = f.mName;
  if ( f.mType == tr( "group" ) )
  {
    name = group( name );
    name = name.toLower();
  }

  name = name.toHtmlEscaped();

  QString helpContents( QStringLiteral( "<h3>%1</h3>\n<div class=\"description\"><p>%2</p></div>" )
                        .arg( tr( "%1 %2" ).arg( f.mType, name ),
                              f.mDescription ) );

  for ( const HelpVariant &v : std::as_const( f.mVariants ) )
  {
    if ( f.mVariants.size() > 1 )
    {
      helpContents += QStringLiteral( "<h3>%1</h3>\n<div class=\"description\">%2</p></div>" ).arg( v.mName, v.mDescription );
    }

    if ( f.mType != tr( "group" ) && f.mType != tr( "expression" ) )
      helpContents += QStringLiteral( "<h4>%1</h4>\n<div class=\"syntax\">\n" ).arg( tr( "Syntax" ) );

    if ( f.mType == tr( "operator" ) )
    {
      if ( v.mArguments.size() == 1 )
      {
        helpContents += QStringLiteral( "<code><span class=\"functionname\">%1</span> <span class=\"argument\">%2</span></code>" )
                        .arg( name, v.mArguments[0].mArg );
      }
      else if ( v.mArguments.size() == 2 )
      {
        helpContents += QStringLiteral( "<code><span class=\"argument\">%1</span> <span class=\"functionname\">%2</span> <span class=\"argument\">%3</span></code>" )
                        .arg( v.mArguments[0].mArg, name, v.mArguments[1].mArg );
      }
    }
    else if ( f.mType != tr( "group" ) && f.mType != tr( "expression" ) )
    {
      helpContents += QStringLiteral( "<code><span class=\"functionname\">%1</span>" ).arg( name );

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
            helpContents += QStringLiteral( "<span class=\"argument\">%2%3</span>" ).arg(
                              a.mArg,
                              a.mDefaultVal.isEmpty() ? QString() : '=' + a.mDefaultVal
                            );

            if ( a.mOptional )
              helpContents += QLatin1Char( ']' );
          }
          delim = QStringLiteral( "," );
        }

        if ( v.mVariableLenArguments )
        {
          helpContents += QChar( 0x2026 );
        }

        helpContents += ')';
      }

      helpContents += QLatin1String( "</code>" );

      if ( hasOptionalArgs )
      {
        helpContents += QLatin1String( "<br/><br/>" ) + tr( "[ ] marks optional components" );
      }
    }

    if ( !v.mArguments.isEmpty() )
    {
      helpContents += QStringLiteral( "<h4>%1</h4>\n<div class=\"arguments\">\n<table>" ).arg( tr( "Arguments" ) );

      for ( const HelpArg &a : std::as_const( v.mArguments ) )
      {
        if ( a.mSyntaxOnly )
          continue;

        helpContents += QStringLiteral( "<tr><td class=\"argument\">%1</td><td>%2</td></tr>" ).arg( a.mArg, a.mDescription );
      }

      helpContents += QLatin1String( "</table>\n</div>\n" );
    }

    if ( !v.mExamples.isEmpty() )
    {
      helpContents += QStringLiteral( "<h4>%1</h4>\n<div class=\"examples\">\n<ul>\n" ).arg( tr( "Examples" ) );

      for ( const HelpExample &e : std::as_const( v.mExamples ) )
      {
        helpContents += "<li><code>" + e.mExpression + "</code> &rarr; <code>" + e.mReturns + "</code>";

        if ( !e.mNote.isEmpty() )
          helpContents += QStringLiteral( " (%1)" ).arg( e.mNote );

        helpContents += QLatin1String( "</li>\n" );
      }

      helpContents += QLatin1String( "</ul>\n</div>\n" );
    }

    if ( !v.mNotes.isEmpty() )
    {
      helpContents += QStringLiteral( "<h4>%1</h4>\n<div class=\"notes\"><p>%2</p></div>\n" ).arg( tr( "Notes" ), v.mNotes );
    }
  }

  return helpContents;
}

QStringList QgsExpression::tags( const QString &name )
{
  QStringList tags = QStringList();

  QgsExpression::initFunctionHelp();

  if ( sFunctionHelpTexts()->contains( name ) )
  {
    const Help &f = ( *sFunctionHelpTexts() )[ name ];

    for ( const HelpVariant &v : std::as_const( f.mVariants ) )
    {
      tags << v.mTags;
    }
  }

  return tags;
}

void QgsExpression::initVariableHelp()
{
  if ( !sVariableHelpTexts()->isEmpty() )
    return;

  //global variables
  sVariableHelpTexts()->insert( QStringLiteral( "qgis_version" ), QCoreApplication::translate( "variable_help", "Current QGIS version string." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "qgis_version_no" ), QCoreApplication::translate( "variable_help", "Current QGIS version number." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "qgis_release_name" ), QCoreApplication::translate( "variable_help", "Current QGIS release name." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "qgis_short_version" ), QCoreApplication::translate( "variable_help", "Short QGIS version string." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "qgis_os_name" ), QCoreApplication::translate( "variable_help", "Operating system name, e.g., 'windows', 'linux' or 'osx'." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "qgis_platform" ), QCoreApplication::translate( "variable_help", "QGIS platform, e.g., 'desktop' or 'server'." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "qgis_locale" ), QCoreApplication::translate( "variable_help", "Two letter identifier for current QGIS locale." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "user_account_name" ), QCoreApplication::translate( "variable_help", "Current user's operating system account name." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "user_full_name" ), QCoreApplication::translate( "variable_help", "Current user's operating system user name (if available)." ) );

  //project variables
  sVariableHelpTexts()->insert( QStringLiteral( "project_title" ), QCoreApplication::translate( "variable_help", "Title of current project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_path" ), QCoreApplication::translate( "variable_help", "Full path (including file name) of current project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_folder" ), QCoreApplication::translate( "variable_help", "Folder for current project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_filename" ), QCoreApplication::translate( "variable_help", "Filename of current project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_basename" ), QCoreApplication::translate( "variable_help", "Base name of current project's filename (without path and extension)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_home" ), QCoreApplication::translate( "variable_help", "Home path of current project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_crs" ), QCoreApplication::translate( "variable_help", "Coordinate reference system of project (e.g., 'EPSG:4326')." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_crs_definition" ), QCoreApplication::translate( "variable_help", "Coordinate reference system of project (full definition)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_units" ), QCoreApplication::translate( "variable_help", "Unit of the project's CRS." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_crs_description" ), QCoreApplication::translate( "variable_help", "Name of the coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_crs_acronym" ), QCoreApplication::translate( "variable_help", "Acronym of the coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_crs_ellipsoid" ), QCoreApplication::translate( "variable_help", "Acronym of the ellipsoid of the coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_crs_proj4" ), QCoreApplication::translate( "variable_help", "Proj4 definition of the coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_crs_wkt" ), QCoreApplication::translate( "variable_help", "WKT definition of the coordinate reference system of the project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_author" ), QCoreApplication::translate( "variable_help", "Project author, taken from project metadata." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_abstract" ), QCoreApplication::translate( "variable_help", "Project abstract, taken from project metadata." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_creation_date" ), QCoreApplication::translate( "variable_help", "Project creation date, taken from project metadata." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_identifier" ), QCoreApplication::translate( "variable_help", "Project identifier, taken from project metadata." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_last_saved" ), QCoreApplication::translate( "variable_help", "Date/time when project was last saved." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_keywords" ), QCoreApplication::translate( "variable_help", "Project keywords, taken from project metadata." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_area_units" ), QCoreApplication::translate( "variable_help", "Area unit for current project, used when calculating areas of geometries." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_distance_units" ), QCoreApplication::translate( "variable_help", "Distance unit for current project, used when calculating lengths of geometries." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "project_ellipsoid" ), QCoreApplication::translate( "variable_help", "Name of ellipsoid of current project, used when calculating geodetic areas and lengths of geometries." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layer_ids" ), QCoreApplication::translate( "variable_help", "List of all map layer IDs from the current project." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layers" ), QCoreApplication::translate( "variable_help", "List of all map layers from the current project." ) );

  //layer variables
  sVariableHelpTexts()->insert( QStringLiteral( "layer_name" ), QCoreApplication::translate( "variable_help", "Name of current layer." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layer_id" ), QCoreApplication::translate( "variable_help", "ID of current layer." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layer_crs" ), QCoreApplication::translate( "variable_help", "CRS Authority ID of current layer." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layer" ), QCoreApplication::translate( "variable_help", "The current layer." ) );

  //composition variables
  sVariableHelpTexts()->insert( QStringLiteral( "layout_name" ), QCoreApplication::translate( "variable_help", "Name of composition." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layout_numpages" ), QCoreApplication::translate( "variable_help", "Number of pages in composition." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layout_page" ), QCoreApplication::translate( "variable_help", "Current page number in composition." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layout_pageheight" ), QCoreApplication::translate( "variable_help", "Composition page height in mm (or specified custom units)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layout_pagewidth" ), QCoreApplication::translate( "variable_help", "Composition page width in mm (or specified custom units)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layout_pageoffsets" ), QCoreApplication::translate( "variable_help", "Array of Y coordinate of the top of each page." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "layout_dpi" ), QCoreApplication::translate( "variable_help", "Composition resolution (DPI)." ) );

  //atlas variables
  sVariableHelpTexts()->insert( QStringLiteral( "atlas_layerid" ), QCoreApplication::translate( "variable_help", "Current atlas coverage layer ID." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "atlas_layername" ), QCoreApplication::translate( "variable_help", "Current atlas coverage layer name." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "atlas_totalfeatures" ), QCoreApplication::translate( "variable_help", "Total number of features in atlas." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "atlas_featurenumber" ), QCoreApplication::translate( "variable_help", "Current atlas feature number." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "atlas_filename" ), QCoreApplication::translate( "variable_help", "Current atlas file name." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "atlas_pagename" ), QCoreApplication::translate( "variable_help", "Current atlas page name." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "atlas_feature" ), QCoreApplication::translate( "variable_help", "Current atlas feature (as feature object)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "atlas_featureid" ), QCoreApplication::translate( "variable_help", "Current atlas feature ID." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "atlas_geometry" ), QCoreApplication::translate( "variable_help", "Current atlas feature geometry." ) );

  //layout item variables
  sVariableHelpTexts()->insert( QStringLiteral( "item_id" ), QCoreApplication::translate( "variable_help", "Layout item user-assigned ID (not necessarily unique)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "item_uuid" ), QCoreApplication::translate( "variable_help", "layout item unique ID." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "item_left" ), QCoreApplication::translate( "variable_help", "Left position of layout item (in mm)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "item_top" ), QCoreApplication::translate( "variable_help", "Top position of layout item (in mm)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "item_width" ), QCoreApplication::translate( "variable_help", "Width of layout item (in mm)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "item_height" ), QCoreApplication::translate( "variable_help", "Height of layout item (in mm)." ) );

  //map settings item variables
  sVariableHelpTexts()->insert( QStringLiteral( "map_id" ), QCoreApplication::translate( "variable_help", "ID of current map destination. This will be 'canvas' for canvas renders, and the item ID for layout map renders." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_rotation" ), QCoreApplication::translate( "variable_help", "Current rotation of map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_scale" ), QCoreApplication::translate( "variable_help", "Current scale of map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_extent" ), QCoreApplication::translate( "variable_help", "Geometry representing the current extent of the map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_extent_center" ), QCoreApplication::translate( "variable_help", "Center of map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_extent_width" ), QCoreApplication::translate( "variable_help", "Width of map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_extent_height" ), QCoreApplication::translate( "variable_help", "Height of map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_crs" ), QCoreApplication::translate( "variable_help", "Coordinate reference system of map (e.g., 'EPSG:4326')." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_crs_description" ), QCoreApplication::translate( "variable_help", "Name of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_units" ), QCoreApplication::translate( "variable_help", "Units for map measurements." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_crs_definition" ), QCoreApplication::translate( "variable_help", "Coordinate reference system of the map (full definition)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_crs_acronym" ), QCoreApplication::translate( "variable_help", "Acronym of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_crs_projection" ), QCoreApplication::translate( "variable_help", "Projection method used by the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_crs_ellipsoid" ), QCoreApplication::translate( "variable_help", "Acronym of the ellipsoid of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_crs_proj4" ), QCoreApplication::translate( "variable_help", "Proj4 definition of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_crs_wkt" ), QCoreApplication::translate( "variable_help", "WKT definition of the coordinate reference system of the map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_layer_ids" ), QCoreApplication::translate( "variable_help", "List of map layer IDs visible in the map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_layers" ), QCoreApplication::translate( "variable_help", "List of map layers visible in the map." ) );

  sVariableHelpTexts()->insert( QStringLiteral( "map_start_time" ), QCoreApplication::translate( "variable_help", "Start of the map's temporal time range (as a datetime value)" ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_end_time" ), QCoreApplication::translate( "variable_help", "End of the map's temporal time range (as a datetime value)" ) );
  sVariableHelpTexts()->insert( QStringLiteral( "map_interval" ), QCoreApplication::translate( "variable_help", "Duration of the map's temporal time range (as an interval value)" ) );

  sVariableHelpTexts()->insert( QStringLiteral( "frame_rate" ), QCoreApplication::translate( "variable_help", "Number of frames per second during animation playback" ) );
  sVariableHelpTexts()->insert( QStringLiteral( "frame_number" ), QCoreApplication::translate( "variable_help", "Current frame number during animation playback" ) );
  sVariableHelpTexts()->insert( QStringLiteral( "frame_duration" ), QCoreApplication::translate( "variable_help", "Temporal duration of each animation frame (as an interval value)" ) );
  sVariableHelpTexts()->insert( QStringLiteral( "frame_timestep" ), QCoreApplication::translate( "variable_help", "Frame time step during animation playback" ) );
  sVariableHelpTexts()->insert( QStringLiteral( "frame_timestep_unit" ), QCoreApplication::translate( "variable_help", "Unit of the frame time step during animation playback" ) );
  sVariableHelpTexts()->insert( QStringLiteral( "animation_start_time" ), QCoreApplication::translate( "variable_help", "Start of the animation's overall temporal time range (as a datetime value)" ) );
  sVariableHelpTexts()->insert( QStringLiteral( "animation_end_time" ), QCoreApplication::translate( "variable_help", "End of the animation's overall temporal time range (as a datetime value)" ) );
  sVariableHelpTexts()->insert( QStringLiteral( "animation_interval" ), QCoreApplication::translate( "variable_help", "Duration of the animation's overall temporal time range (as an interval value)" ) );

  // vector tile layer variables
  sVariableHelpTexts()->insert( QStringLiteral( "zoom_level" ), QCoreApplication::translate( "variable_help", "Vector tile zoom level of the map that is being rendered (derived from the current map scale). Normally in interval [0, 20]." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "vector_tile_zoom" ), QCoreApplication::translate( "variable_help", "Exact vector tile zoom level of the map that is being rendered (derived from the current map scale). Normally in interval [0, 20]. Unlike @zoom_level, this variable is a floating point value which can be used to interpolate values between two integer zoom levels." ) );

  sVariableHelpTexts()->insert( QStringLiteral( "row_number" ), QCoreApplication::translate( "variable_help", "Stores the number of the current row." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "grid_number" ), QCoreApplication::translate( "variable_help", "Current grid annotation value." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "grid_axis" ), QCoreApplication::translate( "variable_help", "Current grid annotation axis (e.g., 'x' for longitude, 'y' for latitude)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "column_number" ), QCoreApplication::translate( "variable_help", "Stores the number of the current column." ) );

  // map canvas item variables
  sVariableHelpTexts()->insert( QStringLiteral( "canvas_cursor_point" ), QCoreApplication::translate( "variable_help", "Last cursor position on the canvas in the project's geographical coordinates." ) );

  // legend canvas item variables
  sVariableHelpTexts()->insert( QStringLiteral( "legend_title" ), QCoreApplication::translate( "variable_help", "Title of the legend." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "legend_column_count" ), QCoreApplication::translate( "variable_help", "Number of column in the legend." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "legend_split_layers" ), QCoreApplication::translate( "variable_help", "Boolean indicating if layers can be split in the legend." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "legend_wrap_string" ), QCoreApplication::translate( "variable_help", "Characters used to wrap the legend text." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "legend_filter_by_map" ), QCoreApplication::translate( "variable_help", "Boolean indicating if the content of the legend is filtered by the map." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "legend_filter_out_atlas" ), QCoreApplication::translate( "variable_help", "Boolean indicating if the Atlas is filtered out of the legend." ) );

  // scalebar rendering
  sVariableHelpTexts()->insert( QStringLiteral( "scale_value" ), QCoreApplication::translate( "variable_help", "Current scale bar distance value." ) );

  // map tool capture variables
  sVariableHelpTexts()->insert( QStringLiteral( "snapping_results" ), QCoreApplication::translate( "variable_help",
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
  sVariableHelpTexts()->insert( QStringLiteral( "geometry_part_count" ), QCoreApplication::translate( "variable_help", "Number of parts in rendered feature's geometry." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "geometry_part_num" ), QCoreApplication::translate( "variable_help", "Current geometry part number for feature being rendered." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "geometry_ring_num" ), QCoreApplication::translate( "variable_help", "Current geometry ring number for feature being rendered (for polygon features only). The exterior ring has a value of 0." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "geometry_point_count" ), QCoreApplication::translate( "variable_help", "Number of points in the rendered geometry's part. It is only meaningful for line geometries and for symbol layers that set this variable." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "geometry_point_num" ), QCoreApplication::translate( "variable_help", "Current point number in the rendered geometry's part. It is only meaningful for line geometries and for symbol layers that set this variable." ) );

  sVariableHelpTexts()->insert( QStringLiteral( "symbol_color" ), QCoreApplication::translate( "symbol_color", "Color of symbol used to render the feature." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "symbol_angle" ), QCoreApplication::translate( "symbol_angle", "Angle of symbol used to render the feature (valid for marker symbols only)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "symbol_layer_count" ), QCoreApplication::translate( "symbol_layer_count", "Total number of symbol layers in the symbol." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "symbol_layer_index" ), QCoreApplication::translate( "symbol_layer_index", "Current symbol layer index." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "symbol_marker_row" ), QCoreApplication::translate( "symbol_marker_row", "Row number for marker (valid for point pattern fills only)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "symbol_marker_column" ), QCoreApplication::translate( "symbol_marker_column", "Column number for marker (valid for point pattern fills only)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "symbol_frame" ), QCoreApplication::translate( "symbol_frame", "Frame number (for animated symbols only)." ) );

  sVariableHelpTexts()->insert( QStringLiteral( "symbol_label" ), QCoreApplication::translate( "symbol_label", "Label for the symbol (either a user defined label or the default autogenerated label)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "symbol_id" ), QCoreApplication::translate( "symbol_id", "Internal ID of the symbol." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "symbol_count" ), QCoreApplication::translate( "symbol_count", "Total number of features represented by the symbol." ) );

  //cluster variables
  sVariableHelpTexts()->insert( QStringLiteral( "cluster_color" ), QCoreApplication::translate( "cluster_color", "Color of symbols within a cluster, or NULL if symbols have mixed colors." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "cluster_size" ), QCoreApplication::translate( "cluster_size", "Number of symbols contained within a cluster." ) );

  //processing variables
  sVariableHelpTexts()->insert( QStringLiteral( "algorithm_id" ), QCoreApplication::translate( "algorithm_id", "Unique ID for algorithm." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "model_path" ), QCoreApplication::translate( "variable_help", "Full path (including file name) of current model (or project path if model is embedded in a project)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "model_folder" ), QCoreApplication::translate( "variable_help", "Folder containing current model (or project folder if model is embedded in a project)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "model_name" ), QCoreApplication::translate( "variable_help", "Name of current model." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "model_group" ), QCoreApplication::translate( "variable_help", "Group for current model." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "fullextent_minx" ), QCoreApplication::translate( "fullextent_minx", "Minimum x-value from full canvas extent (including all layers)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "fullextent_miny" ), QCoreApplication::translate( "fullextent_miny", "Minimum y-value from full canvas extent (including all layers)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "fullextent_maxx" ), QCoreApplication::translate( "fullextent_maxx", "Maximum x-value from full canvas extent (including all layers)." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "fullextent_maxy" ), QCoreApplication::translate( "fullextent_maxy", "Maximum y-value from full canvas extent (including all layers)." ) );

  //provider notification
  sVariableHelpTexts()->insert( QStringLiteral( "notification_message" ), QCoreApplication::translate( "notification_message", "Content of the notification message sent by the provider (available only for actions triggered by provider notifications)." ) );

  //form context variable
  sVariableHelpTexts()->insert( QStringLiteral( "current_geometry" ), QCoreApplication::translate( "current_geometry", "Represents the geometry of the feature currently being edited in the form or the table row. Can be used in a form/row context to filter the related features." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "current_feature" ), QCoreApplication::translate( "current_feature", "Represents the feature currently being edited in the form or the table row. Can be used in a form/row context to filter the related features." ) );

  //parent form context variable
  sVariableHelpTexts()->insert( QStringLiteral( "current_parent_geometry" ), QCoreApplication::translate( "current_parent_geometry",
                                "Only usable in an embedded form context, "
                                "represents the geometry of the feature currently being edited in the parent form.\n"
                                "Can be used in a form/row context to filter the related features using a value "
                                "from the feature currently edited in the parent form, to make sure that the filter "
                                "still works with standalone forms it is recommended to wrap this variable in a "
                                "'coalesce()'." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "current_parent_feature" ), QCoreApplication::translate( "current_parent_feature",
                                "Only usable in an embedded form context, "
                                "represents the feature currently being edited in the parent form.\n"
                                "Can be used in a form/row context to filter the related features using a value "
                                "from the feature currently edited in the parent form, to make sure that the filter "
                                "still works with standalone forms it is recommended to wrap this variable in a "
                                "'coalesce()'." ) );

  //form variable
  sVariableHelpTexts()->insert( QStringLiteral( "form_mode" ), QCoreApplication::translate( "form_mode", "What the form is used for, like AddFeatureMode, SingleEditMode, MultiEditMode, SearchMode, AggregateSearchMode or IdentifyMode as string." ) );

  // plots
  sVariableHelpTexts()->insert( QStringLiteral( "plot_axis" ), QCoreApplication::translate( "plot_axis", "The associated plot axis, e.g. 'x' or 'y'." ) );
  sVariableHelpTexts()->insert( QStringLiteral( "plot_axis_value" ), QCoreApplication::translate( "plot_axis_value", "The current value for the plot axis." ) );
}


bool QgsExpression::addVariableHelpText( const QString name, const QString &description )
{
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
  QString text = !description.isEmpty() ? QStringLiteral( "<p>%1</p>" ).arg( description ) : QString();
  if ( showValue )
  {
    QString valueString;
    if ( !value.isValid() )
    {
      valueString = QCoreApplication::translate( "variable_help", "not set" );
    }
    else
    {
      valueString = QStringLiteral( "<b>%1</b>" ).arg( formatPreviewString( value ) );
    }
    text.append( QCoreApplication::translate( "variable_help", "<p>Current value: %1</p>" ).arg( valueString ) );
  }
  return text;
}

QString QgsExpression::group( const QString &name )
{
  if ( sGroups()->isEmpty() )
  {
    sGroups()->insert( QStringLiteral( "Aggregates" ), tr( "Aggregates" ) );
    sGroups()->insert( QStringLiteral( "Arrays" ), tr( "Arrays" ) );
    sGroups()->insert( QStringLiteral( "Color" ), tr( "Color" ) );
    sGroups()->insert( QStringLiteral( "Conditionals" ), tr( "Conditionals" ) );
    sGroups()->insert( QStringLiteral( "Conversions" ), tr( "Conversions" ) );
    sGroups()->insert( QStringLiteral( "Date and Time" ), tr( "Date and Time" ) );
    sGroups()->insert( QStringLiteral( "Fields and Values" ), tr( "Fields and Values" ) );
    sGroups()->insert( QStringLiteral( "Files and Paths" ), tr( "Files and Paths" ) );
    sGroups()->insert( QStringLiteral( "Fuzzy Matching" ), tr( "Fuzzy Matching" ) );
    sGroups()->insert( QStringLiteral( "General" ), tr( "General" ) );
    sGroups()->insert( QStringLiteral( "GeometryGroup" ), tr( "Geometry" ) );
    sGroups()->insert( QStringLiteral( "Map Layers" ), tr( "Map Layers" ) );
    sGroups()->insert( QStringLiteral( "Maps" ), tr( "Maps" ) );
    sGroups()->insert( QStringLiteral( "Math" ), tr( "Math" ) );
    sGroups()->insert( QStringLiteral( "Operators" ), tr( "Operators" ) );
    sGroups()->insert( QStringLiteral( "Rasters" ), tr( "Rasters" ) );
    sGroups()->insert( QStringLiteral( "Record and Attributes" ), tr( "Record and Attributes" ) );
    sGroups()->insert( QStringLiteral( "String" ), tr( "String" ) );
    sGroups()->insert( QStringLiteral( "Variables" ), tr( "Variables" ) );
    sGroups()->insert( QStringLiteral( "Recent (%1)" ), tr( "Recent (%1)" ) );
    sGroups()->insert( QStringLiteral( "UserGroup" ), tr( "User expressions" ) );
  }

  //return the translated name for this group. If group does not
  //have a translated name in the gGroups hash, return the name
  //unchanged
  return sGroups()->value( name, name );
}

QString QgsExpression::formatPreviewString( const QVariant &value, const bool htmlOutput, int maximumPreviewLength )
{
  const QString startToken = htmlOutput ? QStringLiteral( "<i>&lt;" ) : QStringLiteral( "<" );
  const QString endToken = htmlOutput ? QStringLiteral( "&gt;</i>" ) : QStringLiteral( ">" );

  if ( value.canConvert<QgsGeometry>() )
  {
    //result is a geometry
    QgsGeometry geom = value.value<QgsGeometry>();
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
  else if ( value.canConvert< QgsFeature >() )
  {
    //result is a feature
    QgsFeature feat = value.value<QgsFeature>();
    return startToken + tr( "feature: %1" ).arg( feat.id() ) + endToken;
  }
  else if ( value.canConvert< QgsInterval >() )
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
  else if ( value.canConvert< QgsGradientColorRamp >() )
  {
    return startToken + tr( "gradient ramp" ) + endToken;
  }
  else if ( value.type() == QVariant::Date )
  {
    const QDate dt = value.toDate();
    return startToken + tr( "date: %1" ).arg( dt.toString( QStringLiteral( "yyyy-MM-dd" ) ) ) + endToken;
  }
  else if ( value.type() == QVariant::Time )
  {
    const QTime tm = value.toTime();
    return startToken + tr( "time: %1" ).arg( tm.toString( QStringLiteral( "hh:mm:ss" ) ) ) + endToken;
  }
  else if ( value.type() == QVariant::DateTime )
  {
    const QDateTime dt = value.toDateTime();
    return startToken + tr( "datetime: %1 (%2)" ).arg( dt.toString( QStringLiteral( "yyyy-MM-dd hh:mm:ss" ) ), dt.timeZoneAbbreviation() ) + endToken;
  }
  else if ( value.type() == QVariant::String )
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
  else if ( value.type() == QVariant::Map )
  {
    QString mapStr = QStringLiteral( "{" );
    const QVariantMap map = value.toMap();
    QString separator;
    for ( QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it )
    {
      mapStr.append( separator );
      if ( separator.isEmpty() )
        separator = QStringLiteral( "," );

      mapStr.append( QStringLiteral( " '%1': %2" ).arg( it.key(), formatPreviewString( it.value(), htmlOutput ) ) );
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
  else if ( value.type() == QVariant::List || value.type() == QVariant::StringList )
  {
    QString listStr = QStringLiteral( "[" );
    const QVariantList list = value.toList();
    QString separator;
    for ( const QVariant &arrayValue : list )
    {
      listStr.append( separator );
      if ( separator.isEmpty() )
        separator = QStringLiteral( "," );

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
  else if ( value.type() == QVariant::Int ||
            value.type() == QVariant::UInt ||
            value.type() == QVariant::LongLong ||
            value.type() == QVariant::ULongLong ||
            value.type() == QVariant::Double ||
            // Qt madness with QMetaType::Float :/
            value.type() == static_cast<QVariant::Type>( QMetaType::Float ) )
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

QString QgsExpression::createFieldEqualityExpression( const QString &fieldName, const QVariant &value, QVariant::Type fieldType )
{
  QString expr;

  if ( value.isNull() )
    expr = QStringLiteral( "%1 IS NULL" ).arg( quotedColumnRef( fieldName ) );
  else if ( fieldType == QVariant::Type::Invalid )
    expr = QStringLiteral( "%1 = %2" ).arg( quotedColumnRef( fieldName ), quotedValue( value ) );
  else
    expr = QStringLiteral( "%1 = %2" ).arg( quotedColumnRef( fieldName ), quotedValue( value, fieldType ) );

  return expr;
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
  result = QStringLiteral( "%1 IN (%2)" ).arg( quotedColumnRef( inField ), values.join( ',' ) );
  return true;
}

const QgsExpressionNode *QgsExpression::rootNode() const
{
  return d->mRootNode;
}

bool QgsExpression::isField() const
{
  return d->mRootNode && d->mRootNode->nodeType() == QgsExpressionNode::ntColumnRef;
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



