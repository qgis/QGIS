/***************************************************************************
     qgsexpressioncontext.cpp
     ------------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressioncontext.h"

#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsfield.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgssymbollayerv2utils.h"
#include <QSettings>
#include <QDir>


QgsExpressionContext::~QgsExpressionContext()
{

}

bool QgsExpressionContext::hasVariable( const QString& name ) const
{
  Q_FOREACH ( const QgsExpressionContextScope& scope, mStack )
  {
    if ( scope.hasVariable( name ) )
      return true;
  }
  return false;
}

QVariant QgsExpressionContext::variable( const QString& name ) const
{
  const QgsExpressionContextScope* scope = activeScopeForVariable( name );
  return scope ? scope->variable( name ) : QVariant();
}

const QgsExpressionContextScope* QgsExpressionContext::activeScopeForVariable( const QString& name ) const
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if (( *it ).hasVariable( name ) )
      return ( &( *it ) );
  }
  return 0;
}

QgsExpressionContextScope* QgsExpressionContext::activeScopeForVariable( const QString& name )
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope >::iterator it = mStack.end();
  while ( it != mStack.begin() )
  {
    --it;
    if (( *it ).hasVariable( name ) )
      return ( &( *it ) );
  }
  return 0;
}

QgsExpressionContextScope* QgsExpressionContext::scope( int index )
{
  return &( mStack[index] );
}

QStringList QgsExpressionContext::variableNames() const
{
  QStringList names;
  Q_FOREACH ( const QgsExpressionContextScope& scope, mStack )
  {
    names << scope.variableNames();
  }
  return names.toSet().toList();
}

bool QgsExpressionContext::isReadOnly( const QString& name ) const { Q_UNUSED( name ); return true; }

bool QgsExpressionContext::hasFunction( const QString &name ) const
{
  Q_FOREACH ( const QgsExpressionContextScope& scope, mStack )
  {
    if ( scope.hasFunction( name ) )
      return true;
  }
  return false;
}

QgsExpression::Function *QgsExpressionContext::function( const QString &name ) const
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if (( *it ).hasFunction( name ) )
      return ( *it ).function( name );
  }
  return 0;
}

int QgsExpressionContext::childCount() const
{
  return mStack.count();
}

void QgsExpressionContext::appendScope( const QgsExpressionContextScope& scope )
{
  mStack.append( scope );
}

QgsExpressionContext& QgsExpressionContext::operator<<( const QgsExpressionContextScope& scope )
{
  mStack.append( scope );
  return *this;
}

//
// QgsExpressionContextScope
//

QgsExpressionContextScope::QgsExpressionContextScope( const QString& name )
    : mName( name )
{

}

QgsExpressionContextScope::QgsExpressionContextScope( const QgsExpressionContextScope& other )
    : mName( other.mName )
    , mVariables( other.mVariables )
{
  Q_FOREACH ( QString key, other.mFunctions.keys() )
  {
    mFunctions.insert( key, other.mFunctions.value( key )->clone() );
  }
}

QgsExpressionContextScope::~QgsExpressionContextScope()
{
  qDeleteAll( mFunctions );
}

void QgsExpressionContextScope::setVariable( const QString &name, const QVariant &value )
{
  if ( mVariables.contains( name ) )
  {
    StaticVariable existing = mVariables.value( name );
    existing.value = value;
    addVariable( existing );
  }
  else
  {
    addVariable( QgsExpressionContextScope::StaticVariable( name, value ) );
  }
}

void QgsExpressionContextScope::addVariable( const QgsExpressionContextScope::StaticVariable &variable )
{
  mVariables.insert( variable.name, variable );
}

void QgsExpressionContextScope::removeVariable( const QString &name )
{
  mVariables.remove( name );
}

bool QgsExpressionContextScope::hasVariable( const QString &name ) const
{
  return mVariables.contains( name );
}

QVariant QgsExpressionContextScope::variable( const QString &name ) const
{
  return hasVariable( name ) ? mVariables.value( name ).value : QVariant();
}

QStringList QgsExpressionContextScope::variableNames() const
{
  QStringList names = mVariables.keys();
  return names;
}

bool QgsExpressionContextScope::isReadOnly( const QString &name ) const
{
  return hasVariable( name ) ? mVariables.value( name ).readOnly : false;
}

bool QgsExpressionContextScope::hasFunction( const QString& name ) const
{
  return mFunctions.contains( name );
}

QgsExpression::Function* QgsExpressionContextScope::function( const QString& name ) const
{
  return mFunctions.contains( name ) ? mFunctions.value( name ) : 0;
}

void QgsExpressionContextScope::addFunction( const QString& name, QgsScopedExpressionFunction* function )
{
  mFunctions.insert( name, function );
}

QgsExpressionContextScope QgsExpressionContextUtils::globalScope()
{
  QgsExpressionContextScope scope;

  //read values from QSettings
  QSettings settings;

  //check if settings contains any variables
  if ( settings.contains( QString( "/variables/values" ) ) )
  {
    QList< QVariant > customVariableVariants = settings.value( QString( "/variables/values" ) ).toList();
    QList< QVariant > customVariableNames = settings.value( QString( "/variables/names" ) ).toList();
    int variableIndex = 0;
    for ( QList< QVariant >::const_iterator it = customVariableVariants.constBegin();
          it != customVariableVariants.constEnd(); ++it )
    {
      if ( variableIndex >= customVariableNames.length() )
      {
        break;
      }

      QVariant value = ( *it );
      QString name = customVariableNames.at( variableIndex ).toString();

      scope.setVariable( name, value );
      variableIndex++;
    }
  }

  //add some extra global variables
  scope.addVariable( QgsExpressionContextScope::StaticVariable( "qgis_version", QGis::QGIS_VERSION, true ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( "qgis_version_no", QGis::QGIS_VERSION_INT, true ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( "qgis_release_name", QGis::QGIS_RELEASE_NAME, true ) );

  return scope;
}


//
// QgsExpressionContextUtils
//


void QgsExpressionContextUtils::setGlobalVariable( const QString& name, const QVariant& value )
{
  // save variable to settings
  QSettings settings;

  QList< QVariant > customVariableVariants = settings.value( QString( "/variables/values" ) ).toList();
  QList< QVariant > customVariableNames = settings.value( QString( "/variables/names" ) ).toList();

  customVariableVariants << value;
  customVariableNames << name;

  settings.setValue( QString( "/variables/names" ), customVariableNames );
  settings.setValue( QString( "/variables/values" ), customVariableVariants );
}


class GetNamedProjectColor : public QgsScopedExpressionFunction
{
  public:
    GetNamedProjectColor()
        : QgsScopedExpressionFunction( "project_color", 1, "Colors" )
    {
      //build up color list from project. Do this in advance for speed
      QStringList colorStrings = QgsProject::instance()->readListEntry( "Palette", "/Colors" );
      QStringList colorLabels = QgsProject::instance()->readListEntry( "Palette", "/Labels" );

      //generate list from custom colors
      int colorIndex = 0;
      for ( QStringList::iterator it = colorStrings.begin();
            it != colorStrings.end(); ++it )
      {
        QColor color = QgsSymbolLayerV2Utils::decodeColor( *it );
        QString label;
        if ( colorLabels.length() > colorIndex )
        {
          label = colorLabels.at( colorIndex );
        }

        mColors.insert( label.toLower(), color );
        colorIndex++;
      }
    }

    virtual QVariant func( const QVariantList& values, const QgsExpressionContext*, QgsExpression* ) override
    {
      QString colorName = values.at( 0 ).toString().toLower();
      if ( mColors.contains( colorName ) )
      {
        return QString( "%1,%2,%3" ).arg( mColors.value( colorName ).red() ).arg( mColors.value( colorName ).green() ).arg( mColors.value( colorName ).blue() );
      }
      else
        return QVariant();
    }

    QgsScopedExpressionFunction* clone() const override
    {
      return new GetNamedProjectColor();
    }

  private:

    QHash< QString, QColor > mColors;

};

QgsExpressionContextScope QgsExpressionContextUtils::projectScope()
{
  QgsProject* project = QgsProject::instance();

  QgsExpressionContextScope scope;

  //add variables defined in project file
  QStringList variableNames = project->readListEntry( "Variables", "/variableNames" );
  QStringList variableValues = project->readListEntry( "Variables", "/variableValues" );

  int varIndex = 0;
  foreach ( QString variableName, variableNames )
  {
    if ( varIndex >= variableValues.length() )
    {
      break;
    }

    QString varValueString = variableValues.at( varIndex );
    varIndex++;
    scope.setVariable( variableName, varValueString );
  }

  //add other known project variables
  scope.addVariable( QgsExpressionContextScope::StaticVariable( "project_title", project->title(), true ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( "project_path", project->fileInfo().filePath(), true ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( "project_folder", project->fileInfo().dir().path(), true ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( "project_filename", project->fileInfo().fileName(), true ) );

  scope.addFunction( "project_color", new GetNamedProjectColor() );
  return scope;
}

void QgsExpressionContextUtils::setProjectVariable( const QString& name, const QVariant& value )
{
  QgsProject* project = QgsProject::instance();

  //write variable to project
  QStringList variableNames = project->readListEntry( "Variables", "/variableNames" );
  QStringList variableValues = project->readListEntry( "Variables", "/variableValues" );

  variableNames << name;
  variableValues << value.toString();

  project->writeEntry( "Variables", "/variableNames", variableNames );
  project->writeEntry( "Variables", "/variableValues", variableValues );
}

QgsExpressionContextScope QgsExpressionContextUtils::layerScope( QgsMapLayer* layer )
{
  QgsExpressionContextScope scope;

  if ( !layer )
    return scope;

  scope.addVariable( QgsExpressionContextScope::StaticVariable( "layer_name", layer->name(), true ) );
  scope.addVariable( QgsExpressionContextScope::StaticVariable( "layer_id", layer->id(), true ) );

  QgsVectorLayer* vLayer = dynamic_cast< QgsVectorLayer* >( layer );
  if ( vLayer )
  {
    scope.addVariable( QgsExpressionContextScope::StaticVariable( "_fields_", QVariant::fromValue( vLayer->pendingFields() ), true ) );
  }

  return scope;
}

void QgsExpressionContextUtils::registerContextFunctions()
{
  QgsExpression::registerFunction( new GetNamedProjectColor() );
}
