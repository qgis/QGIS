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
#include "qgsgeometry.h"
#include "qgscomposition.h"
#include "qgscomposeritem.h"
#include "qgsatlascomposition.h"
#include <QSettings>
#include <QDir>


const QString QgsExpressionContext::EXPR_FIELDS( "_fields_" );
const QString QgsExpressionContext::EXPR_FEATURE( "_feature_" );

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

QgsExpressionContextScope& QgsExpressionContextScope::operator=( const QgsExpressionContextScope & other )
{
  mName = other.mName;
  mVariables = other.mVariables;

  qDeleteAll( mFunctions );
  mFunctions.clear();
  Q_FOREACH ( QString key, other.mFunctions.keys() )
  {
    mFunctions.insert( key, other.mFunctions.value( key )->clone() );
  }

  return *this;
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

bool QgsExpressionContextScope::removeVariable( const QString &name )
{
  return mVariables.remove( name ) > 0;
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

QStringList QgsExpressionContextScope::functionNames() const
{
  return mFunctions.keys();
}

void QgsExpressionContextScope::addFunction( const QString& name, QgsScopedExpressionFunction* function )
{
  mFunctions.insert( name, function );
}

void QgsExpressionContextScope::setFeature( const QgsFeature &feature )
{
  setVariable( QgsExpressionContext::EXPR_FEATURE, QVariant::fromValue( feature ) );
}

void QgsExpressionContextScope::setFields( const QgsFields &fields )
{
  setVariable( QgsExpressionContext::EXPR_FIELDS, QVariant::fromValue( fields ) );
}


//
// QgsExpressionContext
//

QgsExpressionContext::QgsExpressionContext( const QgsExpressionContext& other )
{
  Q_FOREACH ( const QgsExpressionContextScope* scope, other.mStack )
  {
    mStack << new QgsExpressionContextScope( *scope );
  }
  mHighlightedVariables = other.mHighlightedVariables;
}

QgsExpressionContext& QgsExpressionContext::operator=( const QgsExpressionContext & other )
{
  qDeleteAll( mStack );
  mStack.clear();
  Q_FOREACH ( const QgsExpressionContextScope* scope, other.mStack )
  {
    mStack << new QgsExpressionContextScope( *scope );
  }
  mHighlightedVariables = other.mHighlightedVariables;
  return *this;
}

QgsExpressionContext::~QgsExpressionContext()
{
  qDeleteAll( mStack );
  mStack.clear();
}

bool QgsExpressionContext::hasVariable( const QString& name ) const
{
  Q_FOREACH ( const QgsExpressionContextScope* scope, mStack )
  {
    if ( scope->hasVariable( name ) )
      return true;
  }
  return false;
}

QVariant QgsExpressionContext::variable( const QString& name ) const
{
  const QgsExpressionContextScope* scope = activeScopeForVariable( name );
  return scope ? scope->variable( name ) : QVariant();
}

bool QgsExpressionContext::isHighlightedVariable( const QString &name ) const
{
  return mHighlightedVariables.contains( name );
}

void QgsExpressionContext::setHighlightedVariables( const QStringList& variableNames )
{
  mHighlightedVariables = variableNames;
}

const QgsExpressionContextScope* QgsExpressionContext::activeScopeForVariable( const QString& name ) const
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope* >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if (( *it )->hasVariable( name ) )
      return ( *it );
  }
  return 0;
}

QgsExpressionContextScope* QgsExpressionContext::activeScopeForVariable( const QString& name )
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope* >::iterator it = mStack.end();
  while ( it != mStack.begin() )
  {
    --it;
    if (( *it )->hasVariable( name ) )
      return ( *it );
  }
  return 0;
}

QgsExpressionContextScope* QgsExpressionContext::scope( int index )
{
  if ( index < 0 || index >= mStack.count() )
    return 0;

  return mStack[index];
}

QgsExpressionContextScope *QgsExpressionContext::lastScope()
{
  if ( mStack.count() < 1 )
    return 0;

  return mStack.last();
}

int QgsExpressionContext::indexOfScope( QgsExpressionContextScope* scope ) const
{
  if ( !scope )
    return -1;

  return mStack.indexOf( scope );
}

QStringList QgsExpressionContext::variableNames() const
{
  QStringList names;
  Q_FOREACH ( const QgsExpressionContextScope* scope, mStack )
  {
    names << scope->variableNames();
  }
  return names.toSet().toList();
}

QStringList QgsExpressionContext::filteredVariableNames() const
{
  QStringList allVariables = variableNames();
  QStringList filtered;
  Q_FOREACH ( QString variable, allVariables )
  {
    if ( variable.startsWith( "_" ) )
      continue;

    filtered << variable;
  }

  filtered.sort();
  return filtered;
}

bool QgsExpressionContext::isReadOnly( const QString& name ) const
{
  Q_FOREACH ( const QgsExpressionContextScope* scope, mStack )
  {
    if ( scope->isReadOnly( name ) )
      return true;
  }
  return false;
}

bool QgsExpressionContext::hasFunction( const QString &name ) const
{
  Q_FOREACH ( const QgsExpressionContextScope* scope, mStack )
  {
    if ( scope->hasFunction( name ) )
      return true;
  }
  return false;
}

QStringList QgsExpressionContext::functionNames() const
{
  QStringList result;
  Q_FOREACH ( const QgsExpressionContextScope* scope, mStack )
  {
    result << scope->functionNames();
  }
  result = result.toSet().toList();
  result.sort();
  return result;
}

QgsExpression::Function *QgsExpressionContext::function( const QString &name ) const
{
  //iterate through stack backwards, so that higher priority variables take precedence
  QList< QgsExpressionContextScope* >::const_iterator it = mStack.constEnd();
  while ( it != mStack.constBegin() )
  {
    --it;
    if (( *it )->hasFunction( name ) )
      return ( *it )->function( name );
  }
  return 0;
}

int QgsExpressionContext::scopeCount() const
{
  return mStack.count();
}

void QgsExpressionContext::appendScope( QgsExpressionContextScope* scope )
{
  mStack.append( scope );
}

QgsExpressionContext& QgsExpressionContext::operator<<( QgsExpressionContextScope* scope )
{
  mStack.append( scope );
  return *this;
}

void QgsExpressionContext::setFeature( const QgsFeature &feature )
{
  if ( mStack.isEmpty() )
    mStack.append( new QgsExpressionContextScope() );

  mStack.last()->setFeature( feature );
}

void QgsExpressionContext::setFields( const QgsFields &fields )
{
  if ( mStack.isEmpty() )
    mStack.append( new QgsExpressionContextScope() );

  mStack.last()->setFields( fields );
}


//
// QgsExpressionContextUtils
//

QgsExpressionContextScope* QgsExpressionContextUtils::globalScope()
{
  QgsExpressionContextScope* scope = new QgsExpressionContextScope( QObject::tr( "Global" ) );

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

      scope->setVariable( name, value );
      variableIndex++;
    }
  }

  //add some extra global variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "qgis_version", QGis::QGIS_VERSION, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "qgis_version_no", QGis::QGIS_VERSION_INT, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "qgis_release_name", QGis::QGIS_RELEASE_NAME, true ) );

  return scope;
}

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

void QgsExpressionContextUtils::setGlobalVariables( const QgsStringMap &variables )
{
  QSettings settings;

  QList< QVariant > customVariableVariants;
  QList< QVariant > customVariableNames;

  Q_FOREACH ( QString variable, variables.keys() )
  {
    customVariableNames << variable;
    customVariableVariants << variables.value( variable );
  }

  settings.setValue( QString( "/variables/names" ), customVariableNames );
  settings.setValue( QString( "/variables/values" ), customVariableVariants );
}

class GetNamedProjectColor : public QgsScopedExpressionFunction
{
  public:
    GetNamedProjectColor()
        : QgsScopedExpressionFunction( "project_color", 1, "Color" )
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

QgsExpressionContextScope* QgsExpressionContextUtils::projectScope()
{
  QgsProject* project = QgsProject::instance();

  QgsExpressionContextScope* scope = new QgsExpressionContextScope( QObject::tr( "Project" ) );

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
    scope->setVariable( variableName, varValueString );
  }

  //add other known project variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "project_title", project->title(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "project_path", project->fileInfo().filePath(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "project_folder", project->fileInfo().dir().path(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "project_filename", project->fileInfo().fileName(), true ) );

  scope->addFunction( "project_color", new GetNamedProjectColor() );
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

void QgsExpressionContextUtils::setProjectVariables( const QgsStringMap &variables )
{
  QgsProject* project = QgsProject::instance();

  //write variable to project
  QStringList variableNames;
  QStringList variableValues;

  Q_FOREACH ( QString variable, variables.keys() )
  {
    variableNames << variable;
    variableValues << variables.value( variable );
  }

  project->writeEntry( "Variables", "/variableNames", variableNames );
  project->writeEntry( "Variables", "/variableValues", variableValues );
}

QgsExpressionContextScope* QgsExpressionContextUtils::layerScope( const QgsMapLayer* layer )
{
  QgsExpressionContextScope* scope = new QgsExpressionContextScope( QObject::tr( "Layer" ) );

  if ( !layer )
    return scope;

  //add variables defined in layer properties
  QStringList variableNames = layer->customProperty( "variableNames" ).toStringList();
  QStringList variableValues = layer->customProperty( "variableValues" ).toStringList();

  int varIndex = 0;
  foreach ( QString variableName, variableNames )
  {
    if ( varIndex >= variableValues.length() )
    {
      break;
    }

    QVariant varValue = variableValues.at( varIndex );
    varIndex++;
    scope->setVariable( variableName, varValue );
  }

  scope->addVariable( QgsExpressionContextScope::StaticVariable( "layer_name", layer->name(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "layer_id", layer->id(), true ) );

  const QgsVectorLayer* vLayer = dynamic_cast< const QgsVectorLayer* >( layer );
  if ( vLayer )
  {
    scope->setFields( vLayer->fields() );
  }

  //TODO - add functions. Possibilities include:
  //is_selected
  //field summary stats

  return scope;
}

void QgsExpressionContextUtils::setLayerVariable( QgsMapLayer* layer, const QString& name, const QVariant& value )
{
  if ( !layer )
    return;

  //write variable to layer
  QStringList variableNames = layer->customProperty( "variableNames" ).toStringList();
  QStringList variableValues = layer->customProperty( "variableValues" ).toStringList();

  variableNames << name;
  variableValues << value.toString();

  layer->setCustomProperty( "variableNames", variableNames );
  layer->setCustomProperty( "variableValues", variableValues );
}

void QgsExpressionContextUtils::setLayerVariables( QgsMapLayer* layer, const QgsStringMap variables )
{
  if ( !layer )
    return;

  QStringList variableNames;
  QStringList variableValues;

  Q_FOREACH ( QString variable, variables.keys() )
  {
    variableNames << variable;
    variableValues << variables.value( variable );
  }

  layer->setCustomProperty( "variableNames", variableNames );
  layer->setCustomProperty( "variableValues", variableValues );
}

QgsExpressionContextScope *QgsExpressionContextUtils::compositionScope( const QgsComposition *composition )
{
  QgsExpressionContextScope* scope = new QgsExpressionContextScope( QObject::tr( "Composition" ) );
  if ( !composition )
    return scope;

  //add variables defined in composition properties
  QStringList variableNames = composition->customProperty( "variableNames" ).toStringList();
  QStringList variableValues = composition->customProperty( "variableValues" ).toStringList();

  int varIndex = 0;
  foreach ( QString variableName, variableNames )
  {
    if ( varIndex >= variableValues.length() )
    {
      break;
    }

    QVariant varValue = variableValues.at( varIndex );
    varIndex++;
    scope->setVariable( variableName, varValue );
  }

  //add known composition context variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "layout_numpages", composition->numPages(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "layout_pageheight", composition->paperHeight(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "layout_pagewidth", composition->paperWidth(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "layout_dpi", composition->printResolution(), true ) );

  return scope;
}

void QgsExpressionContextUtils::setCompositionVariable( QgsComposition* composition, const QString& name, const QVariant& value )
{
  if ( !composition )
    return;

  //write variable to composition
  QStringList variableNames = composition->customProperty( "variableNames" ).toStringList();
  QStringList variableValues = composition->customProperty( "variableValues" ).toStringList();

  variableNames << name;
  variableValues << value.toString();

  composition->setCustomProperty( "variableNames", variableNames );
  composition->setCustomProperty( "variableValues", variableValues );
}

void QgsExpressionContextUtils::setCompositionVariables( QgsComposition* composition, const QgsStringMap variables )
{
  if ( !composition )
    return;

  QStringList variableNames;
  QStringList variableValues;

  Q_FOREACH ( QString variable, variables.keys() )
  {
    variableNames << variable;
    variableValues << variables.value( variable );
  }

  composition->setCustomProperty( "variableNames", variableNames );
  composition->setCustomProperty( "variableValues", variableValues );
}

QgsExpressionContextScope* QgsExpressionContextUtils::atlasScope( const QgsAtlasComposition* atlas )
{
  QgsExpressionContextScope* scope = new QgsExpressionContextScope( QObject::tr( "Atlas" ) );
  if ( !atlas )
    return scope;

  //add known atlas variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "atlas_totalfeatures", atlas->numFeatures(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "atlas_featurenumber", atlas->currentFeatureNumber() + 1, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "atlas_filename", atlas->currentFilename(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "atlas_pagename", atlas->currentPageName(), true ) );

  if ( atlas->enabled() && atlas->coverageLayer() )
  {
    scope->setFields( atlas->coverageLayer()->fields() );
  }

  if ( atlas->enabled() )
  {
    QgsFeature atlasFeature = atlas->feature();
    scope->setFeature( atlasFeature );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( "atlas_feature", QVariant::fromValue( atlasFeature ), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( "atlas_featureid", atlasFeature.id(), true ) );
  }

  return scope;
}

QgsExpressionContextScope *QgsExpressionContextUtils::composerItemScope( const QgsComposerItem *composerItem )
{
  QgsExpressionContextScope* scope = new QgsExpressionContextScope( QObject::tr( "Composer Item" ) );
  if ( !composerItem )
    return scope;

  //add variables defined in composer item properties
  QStringList variableNames = composerItem->customProperty( "variableNames" ).toStringList();
  QStringList variableValues = composerItem->customProperty( "variableValues" ).toStringList();

  int varIndex = 0;
  foreach ( QString variableName, variableNames )
  {
    if ( varIndex >= variableValues.length() )
    {
      break;
    }

    QVariant varValue = variableValues.at( varIndex );
    varIndex++;
    scope->setVariable( variableName, varValue );
  }

  //add known composer item context variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "item_id", composerItem->id(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( "item_uuid", composerItem->uuid(), true ) );

  return scope;
}

void QgsExpressionContextUtils::setComposerItemVariable( QgsComposerItem* composerItem, const QString& name, const QVariant& value )
{
  if ( !composerItem )
    return;

  //write variable to composer item
  QStringList variableNames = composerItem->customProperty( "variableNames" ).toStringList();
  QStringList variableValues = composerItem->customProperty( "variableValues" ).toStringList();

  variableNames << name;
  variableValues << value.toString();

  composerItem->setCustomProperty( "variableNames", variableNames );
  composerItem->setCustomProperty( "variableValues", variableValues );
}

void QgsExpressionContextUtils::setComposerItemVariables( QgsComposerItem* composerItem, const QgsStringMap variables )
{
  if ( !composerItem )
    return;

  QStringList variableNames;
  QStringList variableValues;

  Q_FOREACH ( QString variable, variables.keys() )
  {
    variableNames << variable;
    variableValues << variables.value( variable );
  }

  composerItem->setCustomProperty( "variableNames", variableNames );
  composerItem->setCustomProperty( "variableValues", variableValues );
}

QgsExpressionContext QgsExpressionContextUtils::createFeatureBasedContext( const QgsFeature &feature, const QgsFields &fields )
{
  QgsExpressionContextScope* scope = new QgsExpressionContextScope();
  scope->setFeature( feature );
  scope->setFields( fields );
  return QgsExpressionContext() << scope;
}

void QgsExpressionContextUtils::registerContextFunctions()
{
  QgsExpression::registerFunction( new GetNamedProjectColor() );
}
