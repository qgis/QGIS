/***************************************************************************
                         qgsprocessingmodelalgorithm.cpp
                         ------------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingregistry.h"

QgsProcessingModelAlgorithm::ChildAlgorithm::ChildAlgorithm( const QString &algorithmId )
  : mAlgorithmId( algorithmId )
{

}

const QgsProcessingAlgorithm *QgsProcessingModelAlgorithm::ChildAlgorithm::algorithm() const
{
  return QgsApplication::processingRegistry()->algorithmById( mAlgorithmId );
}

QString QgsProcessingModelAlgorithm::Component::description() const
{
  return mDescription;
}

void QgsProcessingModelAlgorithm::Component::setDescription( const QString &description )
{
  mDescription = description;
}

QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSource> QgsProcessingModelAlgorithm::ChildAlgorithm::parameterSources() const
{
  return mParams;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::setParameterSources( const QMap< QString, QgsProcessingModelAlgorithm::ChildParameterSource > &params )
{
  mParams = params;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::addParameterSource( const QString &name, const ChildParameterSource &source )
{
  mParams.insert( name, source );
}

bool QgsProcessingModelAlgorithm::ChildAlgorithm::isActive() const
{
  return mActive;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::setActive( bool active )
{
  mActive = active;
}

QPointF QgsProcessingModelAlgorithm::Component::position() const
{
  return mPosition;
}

void QgsProcessingModelAlgorithm::Component::setPosition( const QPointF &position )
{
  mPosition = position;
}

QgsProcessingModelAlgorithm::Component::Component( const QString &description )
  : mDescription( description )
{}

QStringList QgsProcessingModelAlgorithm::ChildAlgorithm::dependencies() const
{
  return mDependencies;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::setDependencies( const QStringList &dependencies )
{
  mDependencies = dependencies;
}

bool QgsProcessingModelAlgorithm::ChildAlgorithm::outputsCollapsed() const
{
  return mOutputsCollapsed;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::setOutputsCollapsed( bool outputsCollapsed )
{
  mOutputsCollapsed = outputsCollapsed;
}

QMap<QString, QgsProcessingModelAlgorithm::ModelOutput> QgsProcessingModelAlgorithm::ChildAlgorithm::modelOutputs() const
{
  return mModelOutputs;
}

QgsProcessingModelAlgorithm::ModelOutput &QgsProcessingModelAlgorithm::ChildAlgorithm::modelOutput( const QString &name )
{
  return mModelOutputs[ name ];
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::setModelOutputs( const QMap<QString, QgsProcessingModelAlgorithm::ModelOutput> &modelOutputs )
{
  mModelOutputs = modelOutputs;
}

bool QgsProcessingModelAlgorithm::ChildAlgorithm::parametersCollapsed() const
{
  return mParametersCollapsed;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::setParametersCollapsed( bool parametersCollapsed )
{
  mParametersCollapsed = parametersCollapsed;
}

QString  QgsProcessingModelAlgorithm::ChildAlgorithm::childId() const
{
  return mId;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::setChildId( const QString &id )
{
  mId = id;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::generateChildId( const QgsProcessingModelAlgorithm &model )
{
  int i = 1;
  QString id;
  while ( true )
  {
    id = QStringLiteral( "%1_%2" ).arg( mAlgorithmId ).arg( i );
    if ( !model.childAlgorithms().contains( id ) )
      break;
    i++;
  }
  mId = id;
}

QString QgsProcessingModelAlgorithm::ChildAlgorithm::algorithmId() const
{
  return mAlgorithmId;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::setAlgorithmId( const QString &algorithmId )
{
  mAlgorithmId = algorithmId;
}


//
// QgsProcessingModelAlgorithm
//

QgsProcessingModelAlgorithm::QgsProcessingModelAlgorithm( const QString &name, const QString &group )
  : QgsProcessingAlgorithm()
  , mModelName( name.isEmpty() ? QObject::tr( "model" ) : name )
  , mModelGroup( group )
{}

QString QgsProcessingModelAlgorithm::name() const
{
  return mModelName;
}

QString QgsProcessingModelAlgorithm::displayName() const
{
  return mModelName;
}

QString QgsProcessingModelAlgorithm::group() const
{
  return mModelGroup;
}

QIcon QgsProcessingModelAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/processingModel.svg" ) );
}

QString QgsProcessingModelAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "processingModel.svg" ) );
}

QVariantMap QgsProcessingModelAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );
  Q_UNUSED( feedback );
  return QVariantMap();
}

QMap<QString, QgsProcessingModelAlgorithm::ChildAlgorithm> QgsProcessingModelAlgorithm::childAlgorithms() const
{
  return mChildAlgorithms;
}

void QgsProcessingModelAlgorithm::setParameterComponents( const QMap<QString, QgsProcessingModelAlgorithm::ModelParameter> &parameterComponents )
{
  mParameterComponents = parameterComponents;
}

void QgsProcessingModelAlgorithm::setParameterComponent( const QgsProcessingModelAlgorithm::ModelParameter &component )
{
  mParameterComponents.insert( component.parameterName(), component );
}

QgsProcessingModelAlgorithm::ModelParameter &QgsProcessingModelAlgorithm::parameterComponent( const QString &name )
{
  if ( !mParameterComponents.contains( name ) )
  {
    QgsProcessingModelAlgorithm::ModelParameter &component = mParameterComponents[ name ];
    component.setParameterName( name );
    return component;
  }
  return mParameterComponents[ name ];
}

void QgsProcessingModelAlgorithm::setChildAlgorithms( const QMap<QString, ChildAlgorithm> &childAlgorithms )
{
  mChildAlgorithms = childAlgorithms;
}

void QgsProcessingModelAlgorithm::setChildAlgorithm( const QgsProcessingModelAlgorithm::ChildAlgorithm &algorithm )
{
  mChildAlgorithms.insert( algorithm.childId(), algorithm );
}

QString QgsProcessingModelAlgorithm::addChildAlgorithm( ChildAlgorithm &algorithm )
{
  if ( algorithm.childId().isEmpty() || mChildAlgorithms.contains( algorithm.childId() ) )
    algorithm.generateChildId( *this );

  mChildAlgorithms.insert( algorithm.childId(), algorithm );
  return algorithm.childId();
}

QgsProcessingModelAlgorithm::ChildAlgorithm &QgsProcessingModelAlgorithm::childAlgorithm( const QString &childId )
{
  return mChildAlgorithms[ childId ];
}

bool QgsProcessingModelAlgorithm::removeChildAlgorithm( const QString &id )
{
  if ( !dependentChildAlgorithms( id ).isEmpty() )
    return false;

  mChildAlgorithms.remove( id );
  return true;
}

void QgsProcessingModelAlgorithm::addModelParameter( QgsProcessingParameterDefinition *definition, const QgsProcessingModelAlgorithm::ModelParameter &component )
{
  addParameter( definition );
  mParameterComponents.insert( definition->name(), component );
}

void QgsProcessingModelAlgorithm::updateModelParameter( QgsProcessingParameterDefinition *definition )
{
  removeParameter( definition->name() );
  addParameter( definition );
}

bool QgsProcessingModelAlgorithm::removeModelParameter( const QString &name )
{
  if ( childAlgorithmsDependOnParameter( name ) )
    return false;

  removeParameter( name );
  mParameterComponents.remove( name );
  return true;
}

bool QgsProcessingModelAlgorithm::childAlgorithmsDependOnParameter( const QString &name ) const
{
  QMap< QString, ChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    // check whether child requires this parameter
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSource> childParams = childIt->parameterSources();
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSource>::const_iterator paramIt = childParams.constBegin();
    for ( ; paramIt != childParams.constEnd(); ++paramIt )
    {
      if ( paramIt->source() == ChildParameterSource::ModelParameter
           && paramIt->parameterName() == name )
      {
        return true;
      }
    }
  }
  return false;
}

QMap<QString, QgsProcessingModelAlgorithm::ModelParameter> QgsProcessingModelAlgorithm::parameterComponents() const
{
  return mParameterComponents;
}

void QgsProcessingModelAlgorithm::dependentChildAlgorithmsRecursive( const QString &childId, QSet<QString> &depends ) const
{
  QMap< QString, ChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( depends.contains( childIt->childId() ) )
      continue;

    // does alg have a direct dependency on this child?
    if ( childIt->dependencies().contains( childId ) )
    {
      depends.insert( childIt->childId() );
      dependentChildAlgorithmsRecursive( childIt->childId(), depends );
      continue;
    }

    // check whether child requires any outputs from the target alg
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSource> childParams = childIt->parameterSources();
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSource>::const_iterator paramIt = childParams.constBegin();
    for ( ; paramIt != childParams.constEnd(); ++paramIt )
    {
      if ( paramIt->source() == ChildParameterSource::ChildOutput
           && paramIt->outputChildId() == childId )
      {
        depends.insert( childIt->childId() );
        dependsOnChildAlgorithmsRecursive( childIt->childId(), depends );
        break;
      }
    }
  }
}

QSet<QString> QgsProcessingModelAlgorithm::dependentChildAlgorithms( const QString &childId ) const
{
  QSet< QString > algs;

  // temporarily insert the target child algorithm to avoid
  // unnecessarily recursion though it
  algs.insert( childId );

  dependentChildAlgorithmsRecursive( childId, algs );

  // remove temporary target alg
  algs.remove( childId );

  return algs;
}


void QgsProcessingModelAlgorithm::dependsOnChildAlgorithmsRecursive( const QString &childId, QSet< QString > &depends ) const
{
  ChildAlgorithm alg = mChildAlgorithms.value( childId );

  // add direct dependencies
  Q_FOREACH ( const QString &c, alg.dependencies() )
  {
    if ( !depends.contains( c ) )
    {
      depends.insert( c );
      dependsOnChildAlgorithmsRecursive( c, depends );
    }
  }

  // check through parameter dependencies
  QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSource> childParams = alg.parameterSources();
  QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSource>::const_iterator paramIt = childParams.constBegin();
  for ( ; paramIt != childParams.constEnd(); ++paramIt )
  {
    if ( paramIt->source() == ChildParameterSource::ChildOutput && !depends.contains( paramIt->outputChildId() ) )
    {
      depends.insert( paramIt->outputChildId() );
      dependsOnChildAlgorithmsRecursive( paramIt->outputChildId(), depends );
    }
  }
}

QSet< QString > QgsProcessingModelAlgorithm::dependsOnChildAlgorithms( const QString &childId ) const
{
  QSet< QString > algs;

  // temporarily insert the target child algorithm to avoid
  // unnecessarily recursion though it
  algs.insert( childId );

  dependsOnChildAlgorithmsRecursive( childId, algs );

  // remove temporary target alg
  algs.remove( childId );

  return algs;
}

bool QgsProcessingModelAlgorithm::canExecute( QString *errorMessage ) const
{
  QMap< QString, ChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( !childIt->algorithm() )
    {
      if ( errorMessage )
      {
        *errorMessage = QObject::tr( "The model you are trying to run contains an algorithm that is not available: <i>%1</i>" ).arg( childIt->algorithmId() );
      }
      return false;
    }
  }
  return true;
}


bool QgsProcessingModelAlgorithm::ChildParameterSource::operator==( const QgsProcessingModelAlgorithm::ChildParameterSource &other ) const
{
  if ( mSource != other.mSource )
    return false;

  switch ( mSource )
  {
    case StaticValue:
      return mStaticValue == other.mStaticValue;
    case ChildOutput:
      return mChildId == other.mChildId && mOutputName == other.mOutputName;
    case ModelParameter:
      return mParameterName == other.mParameterName;
  }
  return false;
}

QgsProcessingModelAlgorithm::ChildParameterSource QgsProcessingModelAlgorithm::ChildParameterSource::fromStaticValue( const QVariant &value )
{
  ChildParameterSource src;
  src.mSource = StaticValue;
  src.mStaticValue = value;
  return src;
}

QgsProcessingModelAlgorithm::ChildParameterSource QgsProcessingModelAlgorithm::ChildParameterSource::fromModelParameter( const QString &parameterName )
{
  ChildParameterSource src;
  src.mSource = ModelParameter;
  src.mParameterName = parameterName;
  return src;
}

QgsProcessingModelAlgorithm::ChildParameterSource QgsProcessingModelAlgorithm::ChildParameterSource::fromChildOutput( const QString &childId, const QString &outputName )
{
  ChildParameterSource src;
  src.mSource = ChildOutput;
  src.mChildId = childId;
  src.mOutputName = outputName;
  return src;
}

QgsProcessingModelAlgorithm::ChildParameterSource::Source QgsProcessingModelAlgorithm::ChildParameterSource::source() const
{
  return mSource;
}

QgsProcessingModelAlgorithm::ModelOutput::ModelOutput( const QString &description )
  : QgsProcessingModelAlgorithm::Component( description )
{}

QgsProcessingModelAlgorithm::ModelParameter::ModelParameter( const QString &parameterName )
  : QgsProcessingModelAlgorithm::Component()
  , mParameterName( parameterName )
{

}
