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
#include "qgsprocessingfeedback.h"
#include "qgsprocessingutils.h"
#include "qgsxmlutils.h"
#include "qgsexception.h"
#include <QFile>
#include <QTextStream>

///@cond NOT_STABLE

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

QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSources> QgsProcessingModelAlgorithm::ChildAlgorithm::parameterSources() const
{
  return mParams;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::setParameterSources( const QMap< QString, QgsProcessingModelAlgorithm::ChildParameterSources > &params )
{
  mParams = params;
}

void QgsProcessingModelAlgorithm::ChildAlgorithm::addParameterSources( const QString &name, const ChildParameterSources &source )
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

void QgsProcessingModelAlgorithm::Component::saveCommonProperties( QVariantMap &map ) const
{
  map.insert( QStringLiteral( "component_pos_x" ), mPosition.x() );
  map.insert( QStringLiteral( "component_pos_y" ), mPosition.y() );
  map.insert( QStringLiteral( "component_description" ), mDescription );
}

void QgsProcessingModelAlgorithm::Component::restoreCommonProperties( const QVariantMap &map )
{
  QPointF pos;
  pos.setX( map.value( QStringLiteral( "component_pos_x" ) ).toDouble() );
  pos.setY( map.value( QStringLiteral( "component_pos_y" ) ).toDouble() );
  mPosition = pos;
  mDescription = map.value( QStringLiteral( "component_description" ) ).toString();
}

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
  QMap<QString, QgsProcessingModelAlgorithm::ModelOutput>::iterator outputIt = mModelOutputs.begin();
  for ( ; outputIt != mModelOutputs.end(); ++outputIt )
  {
    // make sure values are consistent
    outputIt->setName( outputIt.key() );
    outputIt->setChildId( mId );
  }
}

QVariant QgsProcessingModelAlgorithm::ChildAlgorithm::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "id" ), mId );
  map.insert( QStringLiteral( "alg_id" ), mAlgorithmId );
  map.insert( QStringLiteral( "active" ), mActive );
  map.insert( QStringLiteral( "dependencies" ), mDependencies );
  map.insert( QStringLiteral( "parameters_collapsed" ), mParametersCollapsed );
  map.insert( QStringLiteral( "outputs_collapsed" ), mOutputsCollapsed );

  saveCommonProperties( map );

  QVariantMap paramMap;
  QMap< QString, QgsProcessingModelAlgorithm::ChildParameterSources >::const_iterator paramIt = mParams.constBegin();
  for ( ; paramIt != mParams.constEnd(); ++paramIt )
  {
    QVariantList sources;
    Q_FOREACH ( const ChildParameterSource &source, paramIt.value() )
    {
      sources << source.toVariant();
    }
    paramMap.insert( paramIt.key(), sources );
  }
  map.insert( "params", paramMap );

  QVariantMap outputMap;
  QMap< QString, QgsProcessingModelAlgorithm::ModelOutput >::const_iterator outputIt = mModelOutputs.constBegin();
  for ( ; outputIt != mModelOutputs.constEnd(); ++outputIt )
  {
    outputMap.insert( outputIt.key(), outputIt.value().toVariant() );
  }
  map.insert( "outputs", outputMap );

  return map;
}

bool QgsProcessingModelAlgorithm::ChildAlgorithm::loadVariant( const QVariant &child )
{
  QVariantMap map = child.toMap();

  mId = map.value( QStringLiteral( "id" ) ).toString();
  mAlgorithmId = map.value( QStringLiteral( "alg_id" ) ).toString();
  mActive = map.value( QStringLiteral( "active" ) ).toBool();
  mDependencies = map.value( QStringLiteral( "dependencies" ) ).toStringList();
  mParametersCollapsed = map.value( QStringLiteral( "parameters_collapsed" ) ).toBool();
  mOutputsCollapsed = map.value( QStringLiteral( "outputs_collapsed" ) ).toBool();

  restoreCommonProperties( map );

  mParams.clear();
  QVariantMap paramMap = map.value( QStringLiteral( "params" ) ).toMap();
  QVariantMap::const_iterator paramIt = paramMap.constBegin();
  for ( ; paramIt != paramMap.constEnd(); ++paramIt )
  {
    ChildParameterSources sources;
    Q_FOREACH ( const QVariant &sourceVar, paramIt->toList() )
    {
      ChildParameterSource param;
      if ( !param.loadVariant( sourceVar.toMap() ) )
        return false;
      sources << param;
    }
    mParams.insert( paramIt.key(), sources );
  }

  mModelOutputs.clear();
  QVariantMap outputMap = map.value( QStringLiteral( "outputs" ) ).toMap();
  QVariantMap::const_iterator outputIt = outputMap.constBegin();
  for ( ; outputIt != outputMap.constEnd(); ++outputIt )
  {
    ModelOutput output;
    if ( !output.loadVariant( outputIt.value().toMap() ) )
      return false;

    mModelOutputs.insert( outputIt.key(), output );
  }

  return true;
}

QString QgsProcessingModelAlgorithm::ChildAlgorithm::asPythonCode() const
{
  QStringList lines;

  if ( !algorithm() )
    return QString();

  QStringList paramParts;
  QMap< QString, QgsProcessingModelAlgorithm::ChildParameterSources >::const_iterator paramIt = mParams.constBegin();
  for ( ; paramIt != mParams.constEnd(); ++paramIt )
  {
    QStringList sourceParts;
    Q_FOREACH ( const ChildParameterSource &source, paramIt.value() )
    {
      QString part = source.asPythonCode();
      if ( !part.isEmpty() )
        sourceParts << QStringLiteral( "'%1':%2" ).arg( paramIt.key(), part );
    }
    if ( sourceParts.count() == 1 )
      paramParts << sourceParts.at( 0 );
    else
      paramParts << QStringLiteral( "[%1]" ).arg( paramParts.join( ',' ) );
  }

  lines << QStringLiteral( "outputs['%1']=processing.run('%2', {%3}, context=context, feedback=feedback)" ).arg( mId, mAlgorithmId, paramParts.join( ',' ) );

  QMap< QString, QgsProcessingModelAlgorithm::ModelOutput >::const_iterator outputIt = mModelOutputs.constBegin();
  for ( ; outputIt != mModelOutputs.constEnd(); ++outputIt )
  {
    lines << QStringLiteral( "results['%1']=outputs['%2']['%3']" ).arg( outputIt.key(), mId, outputIt.value().childOutputName() );
  }

  return lines.join( '\n' );
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

QString QgsProcessingModelAlgorithm::shortHelpString() const
{
  if ( mHelpContent.contains( QStringLiteral( "ALG_DESC" ) ) )
    return mHelpContent.value( QStringLiteral( "ALG_DESC" ) ).toString();
  return QString();
}

QString QgsProcessingModelAlgorithm::helpUrl() const
{
  return QgsProcessingUtils::formatHelpMapAsHtml( mHelpContent, this );
}

QVariantMap QgsProcessingModelAlgorithm::parametersForChildAlgorithm( const ChildAlgorithm &child, const QVariantMap &modelParameters, const QMap< QString, QVariantMap > &results ) const
{
  QVariantMap childParams;
  Q_FOREACH ( const QgsProcessingParameterDefinition *def, child.algorithm()->parameterDefinitions() )
  {
    if ( def->flags() & QgsProcessingParameterDefinition::FlagHidden )
      continue;

    if ( !def->isDestination() )
    {
      if ( !child.parameterSources().contains( def->name() ) )
        continue; // use default value

      ChildParameterSources paramSources = child.parameterSources().value( def->name() );

      QVariantList paramParts;
      Q_FOREACH ( const ChildParameterSource &source, paramSources )
      {
        switch ( source.source() )
        {
          case ChildParameterSource::StaticValue:
            paramParts << source.staticValue();
            break;

          case ChildParameterSource::ModelParameter:
            paramParts << modelParameters.value( source.parameterName() );
            break;

          case ChildParameterSource::ChildOutput:
          {
            QVariantMap linkedChildResults = results.value( source.outputChildId() );
            paramParts << linkedChildResults.value( source.outputName() );
            break;
          }
        }
      }
      if ( paramParts.count() == 1 )
        childParams.insert( def->name(), paramParts.at( 0 ) );
      else
        childParams.insert( def->name(), paramParts );

    }
    else
    {
      const QgsProcessingDestinationParameter *destParam = static_cast< const QgsProcessingDestinationParameter * >( def );

      // is destination linked to one of the final outputs from this model?
      bool isFinalOutput = false;
      QMap<QString, QgsProcessingModelAlgorithm::ModelOutput> outputs = child.modelOutputs();
      QMap<QString, QgsProcessingModelAlgorithm::ModelOutput>::const_iterator outputIt = outputs.constBegin();
      for ( ; outputIt != outputs.constEnd(); ++outputIt )
      {
        if ( outputIt->childOutputName() == destParam->name() )
        {
          QString paramName = child.childId() + ':' + outputIt.key();
          if ( modelParameters.contains( paramName ) )
          {
            QVariant value = modelParameters.value( paramName );
            if ( value.canConvert<QgsProcessingOutputLayerDefinition>() )
            {
              // make sure layout output name is correctly set
              QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
              fromVar.destinationName = outputIt.key();
              value = QVariant::fromValue( fromVar );
            }

            childParams.insert( destParam->name(), value );
          }
          isFinalOutput = true;
          break;
        }
      }

      if ( !isFinalOutput )
      {
        // output is temporary

        // check whether it's optional, and if so - is it required?
        bool required = true;
        if ( destParam->flags() & QgsProcessingParameterDefinition::FlagOptional )
        {
          required = childOutputIsRequired( child.childId(), destParam->name() );
        }

        // not optional, or required elsewhere in model
        if ( required )
          childParams.insert( destParam->name(), destParam->generateTemporaryDestination() );
      }
    }
  }
  return childParams;
}

bool QgsProcessingModelAlgorithm::childOutputIsRequired( const QString &childId, const QString &outputName ) const
{
  // look through all child algs
  QMap< QString, ChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( childIt->childId() == childId || !childIt->isActive() )
      continue;

    // look through all sources for child
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSources> candidateChildParams = childIt->parameterSources();
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSources>::const_iterator childParamIt = candidateChildParams.constBegin();
    for ( ; childParamIt != candidateChildParams.constEnd(); ++childParamIt )
    {
      Q_FOREACH ( const ChildParameterSource &source, childParamIt.value() )
      {
        if ( source.source() == ChildParameterSource::ChildOutput
             && source.outputChildId() == childId
             && source.outputName() == outputName )
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool QgsProcessingModelAlgorithm::processAlgorithm( QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QSet< QString > toExecute;
  QMap< QString, ChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( childIt->isActive() && childIt->algorithm() )
      toExecute.insert( childIt->childId() );
  }

  QTime totalTime;
  totalTime.start();

  QMap< QString, QVariantMap > childResults;
  QVariantMap finalResults;
  QSet< QString > executed;
  bool executedAlg = true;
  while ( executedAlg && executed.count() < toExecute.count() )
  {
    executedAlg = false;
    Q_FOREACH ( const QString &childId, toExecute )
    {
      if ( executed.contains( childId ) )
        continue;

      bool canExecute = true;
      Q_FOREACH ( const QString &dependency, dependsOnChildAlgorithms( childId ) )
      {
        if ( !executed.contains( dependency ) )
        {
          canExecute = false;
          break;
        }
      }

      if ( !canExecute )
        continue;

      executedAlg = true;
      feedback->pushDebugInfo( QObject::tr( "Prepare algorithm: %1" ).arg( childId ) );

      const ChildAlgorithm &child = mChildAlgorithms[ childId ];

      QVariantMap childParams = parametersForChildAlgorithm( child, mInputParameters, childResults );
      feedback->setProgressText( QObject::tr( "Running %1 [%2/%3]" ).arg( child.description() ).arg( executed.count() + 1 ).arg( toExecute.count() ) );
      //feedback->pushDebugInfo( "Parameters: " + ', '.join( [str( p ).strip() +
      //           '=' + str( p.value ) for p in alg.algorithm.parameters] ) )

      QTime childTime;
      childTime.start();

      bool ok = false;
      std::unique_ptr< QgsProcessingAlgorithm > childAlg( child.algorithm()->create() );
      QVariantMap results = childAlg->run( childParams, context, feedback, &ok );
      childAlg.reset( nullptr );
      if ( !ok )
      {
        QString error = QObject::tr( "Error encountered while running %1" ).arg( child.description() );
        feedback->reportError( error );
        throw QgsProcessingException( error );
      }
      childResults.insert( childId, results );

      // look through child alg's outputs to determine whether any of these should be copied
      // to the final model outputs
      QMap<QString, QgsProcessingModelAlgorithm::ModelOutput> outputs = child.modelOutputs();
      QMap<QString, QgsProcessingModelAlgorithm::ModelOutput>::const_iterator outputIt = outputs.constBegin();
      for ( ; outputIt != outputs.constEnd(); ++outputIt )
      {
        finalResults.insert( childId + ':' + outputIt->name(), results.value( outputIt->childOutputName() ) );
      }

      executed.insert( childId );
      feedback->pushDebugInfo( QObject::tr( "OK. Execution took %1 s (%2 outputs)." ).arg( childTime.elapsed() / 1000.0 ).arg( results.count() ) );
    }
  }
  feedback->pushDebugInfo( QObject::tr( "Model processed OK. Executed %1 algorithms total in %2 s." ).arg( executed.count() ).arg( totalTime.elapsed() / 1000.0 ) );

  mResults = finalResults;
  return true;
}

QVariantMap QgsProcessingModelAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  return mResults;
}

QString QgsProcessingModelAlgorithm::sourceFilePath() const
{
  return mSourceFile;
}

void QgsProcessingModelAlgorithm::setSourceFilePath( const QString &sourceFile )
{
  mSourceFile = sourceFile;
}

QString QgsProcessingModelAlgorithm::asPythonCode() const
{
  QStringList lines;
  lines << QStringLiteral( "##%1=name" ).arg( name() );

  QMap< QString, ModelParameter >::const_iterator paramIt = mParameterComponents.constBegin();
  for ( ; paramIt != mParameterComponents.constEnd(); ++paramIt )
  {
    QString name = paramIt.value().parameterName();
    if ( parameterDefinition( name ) )
    {
      lines << parameterDefinition( name )->asScriptCode();
    }
  }

  auto safeName = []( const QString & name )->QString
  {
    QString n = name.toLower().trimmed();
    QRegularExpression rx( "[^a-z_]" );
    n.replace( rx, QString() );
    return n;
  };

  QMap< QString, ChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( !childIt->isActive() || !childIt->algorithm() )
      continue;

    // look through all outputs for child
    QMap<QString, QgsProcessingModelAlgorithm::ModelOutput> outputs = childIt->modelOutputs();
    QMap<QString, QgsProcessingModelAlgorithm::ModelOutput>::const_iterator outputIt = outputs.constBegin();
    for ( ; outputIt != outputs.constEnd(); ++outputIt )
    {
      const QgsProcessingOutputDefinition *output = childIt->algorithm()->outputDefinition( outputIt->childOutputName() );
      lines << QStringLiteral( "##%1=output %2" ).arg( safeName( outputIt->name() ), output->type() );
    }
  }

  lines << QStringLiteral( "results={}" );

  QSet< QString > toExecute;
  childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( childIt->isActive() && childIt->algorithm() )
      toExecute.insert( childIt->childId() );
  }

  QSet< QString > executed;
  bool executedAlg = true;
  while ( executedAlg && executed.count() < toExecute.count() )
  {
    executedAlg = false;
    Q_FOREACH ( const QString &childId, toExecute )
    {
      if ( executed.contains( childId ) )
        continue;

      bool canExecute = true;
      Q_FOREACH ( const QString &dependency, dependsOnChildAlgorithms( childId ) )
      {
        if ( !executed.contains( dependency ) )
        {
          canExecute = false;
          break;
        }
      }

      if ( !canExecute )
        continue;

      executedAlg = true;

      const ChildAlgorithm &child = mChildAlgorithms[ childId ];
      lines << child.asPythonCode();

      executed.insert( childId );
    }
  }

  lines << QStringLiteral( "return results" );

  return lines.join( '\n' );
}

bool QgsProcessingModelAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &, QgsProcessingFeedback * )
{
  mInputParameters = parameters;
  return true;
}

QVariantMap QgsProcessingModelAlgorithm::helpContent() const
{
  return mHelpContent;
}

void QgsProcessingModelAlgorithm::setHelpContent( const QVariantMap &helpContent )
{
  mHelpContent = helpContent;
}

void QgsProcessingModelAlgorithm::setName( const QString &name )
{
  mModelName = name;
}

void QgsProcessingModelAlgorithm::setGroup( const QString &group )
{
  mModelGroup = group;
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

void QgsProcessingModelAlgorithm::updateDestinationParameters()
{
  //delete existing destination parameters
  QMutableListIterator<const QgsProcessingParameterDefinition *> it( mParameters );
  while ( it.hasNext() )
  {
    const QgsProcessingParameterDefinition *def = it.next();
    if ( def->isDestination() )
    {
      delete def;
      it.remove();
    }
  }
  // also delete outputs
  qDeleteAll( mOutputs );
  mOutputs.clear();

  // rebuild
  QMap< QString, ChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    QMap<QString, QgsProcessingModelAlgorithm::ModelOutput> outputs = childIt->modelOutputs();
    QMap<QString, QgsProcessingModelAlgorithm::ModelOutput>::const_iterator outputIt = outputs.constBegin();
    for ( ; outputIt != outputs.constEnd(); ++outputIt )
    {
      if ( !childIt->isActive() || !childIt->algorithm() )
        continue;

      // child algorithm has a destination parameter set, copy it to the model
      const QgsProcessingParameterDefinition *source = childIt->algorithm()->parameterDefinition( outputIt->childOutputName() );
      if ( !source )
        continue;

      QgsProcessingParameterDefinition *param = QgsProcessingParameters::parameterFromVariantMap( source->toVariantMap() );
      param->setName( outputIt->childId() + ':' + outputIt->name() );
      param->setDescription( outputIt->description() );
      addParameter( param );

      if ( const QgsProcessingDestinationParameter *destParam = dynamic_cast< const QgsProcessingDestinationParameter *>( param ) )
      {
        QgsProcessingOutputDefinition *output = destParam->toOutputDefinition();
        if ( output )
        {
          addOutput( output );
        }
      }
    }
  }
}

QVariant QgsProcessingModelAlgorithm::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "model_name" ), mModelName );
  map.insert( QStringLiteral( "model_group" ), mModelGroup );
  map.insert( QStringLiteral( "help" ), mHelpContent );

  QVariantMap childMap;
  QMap< QString, ChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    childMap.insert( childIt.key(), childIt.value().toVariant() );
  }
  map.insert( "children", childMap );

  QVariantMap paramMap;
  QMap< QString, ModelParameter >::const_iterator paramIt = mParameterComponents.constBegin();
  for ( ; paramIt != mParameterComponents.constEnd(); ++paramIt )
  {
    paramMap.insert( paramIt.key(), paramIt.value().toVariant() );
  }
  map.insert( "parameters", paramMap );

  QVariantMap paramDefMap;
  Q_FOREACH ( const QgsProcessingParameterDefinition *def, mParameters )
  {
    paramDefMap.insert( def->name(), def->toVariantMap() );
  }
  map.insert( "parameterDefinitions", paramDefMap );

  return map;
}

bool QgsProcessingModelAlgorithm::loadVariant( const QVariant &model )
{
  QVariantMap map = model.toMap();

  mModelName = map.value( QStringLiteral( "model_name" ) ).toString();
  mModelGroup = map.value( QStringLiteral( "model_group" ) ).toString();
  mHelpContent = map.value( QStringLiteral( "help" ) ).toMap();

  mChildAlgorithms.clear();
  QVariantMap childMap = map.value( QStringLiteral( "children" ) ).toMap();
  QVariantMap::const_iterator childIt = childMap.constBegin();
  for ( ; childIt != childMap.constEnd(); ++childIt )
  {
    ChildAlgorithm child;
    if ( !child.loadVariant( childIt.value() ) )
      return false;

    mChildAlgorithms.insert( child.childId(), child );
  }

  mParameterComponents.clear();
  QVariantMap paramMap = map.value( QStringLiteral( "parameters" ) ).toMap();
  QVariantMap::const_iterator paramIt = paramMap.constBegin();
  for ( ; paramIt != paramMap.constEnd(); ++paramIt )
  {
    ModelParameter param;
    if ( !param.loadVariant( paramIt.value().toMap() ) )
      return false;

    mParameterComponents.insert( param.parameterName(), param );
  }

  qDeleteAll( mParameters );
  mParameters.clear();
  QVariantMap paramDefMap = map.value( QStringLiteral( "parameterDefinitions" ) ).toMap();
  QVariantMap::const_iterator paramDefIt = paramDefMap.constBegin();
  for ( ; paramDefIt != paramDefMap.constEnd(); ++paramDefIt )
  {
    QgsProcessingParameterDefinition *param = QgsProcessingParameters::parameterFromVariantMap( paramDefIt.value().toMap() );
    if ( !param )
      return false;

    addParameter( param );
  }

  updateDestinationParameters();

  return true;
}

bool QgsProcessingModelAlgorithm::toFile( const QString &path ) const
{
  QDomDocument doc = QDomDocument( "model" );
  QDomElement elem = QgsXmlUtils::writeVariant( toVariant(), doc );
  doc.appendChild( elem );

  QFile file( path );
  if ( file.open( QFile::WriteOnly | QFile::Truncate ) )
  {
    QTextStream stream( &file );
    doc.save( stream, 2 );
    file.close();
    return true;
  }
  return false;
}

bool QgsProcessingModelAlgorithm::fromFile( const QString &path )
{
  QDomDocument doc;

  QFile file( path );
  if ( file.open( QFile::ReadOnly ) )
  {
    if ( !doc.setContent( &file ) )
      return false;

    file.close();
  }

  QVariant props = QgsXmlUtils::readVariant( doc.firstChildElement() );
  return loadVariant( props );
}

void QgsProcessingModelAlgorithm::setChildAlgorithms( const QMap<QString, ChildAlgorithm> &childAlgorithms )
{
  mChildAlgorithms = childAlgorithms;
  updateDestinationParameters();
}

void QgsProcessingModelAlgorithm::setChildAlgorithm( const QgsProcessingModelAlgorithm::ChildAlgorithm &algorithm )
{
  mChildAlgorithms.insert( algorithm.childId(), algorithm );
  updateDestinationParameters();
}

QString QgsProcessingModelAlgorithm::addChildAlgorithm( ChildAlgorithm &algorithm )
{
  if ( algorithm.childId().isEmpty() || mChildAlgorithms.contains( algorithm.childId() ) )
    algorithm.generateChildId( *this );

  mChildAlgorithms.insert( algorithm.childId(), algorithm );
  updateDestinationParameters();
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
  updateDestinationParameters();
  return true;
}

void QgsProcessingModelAlgorithm::deactivateChildAlgorithm( const QString &id )
{
  Q_FOREACH ( const QString &child, dependentChildAlgorithms( id ) )
  {
    childAlgorithm( child ).setActive( false );
  }
  childAlgorithm( id ).setActive( false );
  updateDestinationParameters();
}

bool QgsProcessingModelAlgorithm::activateChildAlgorithm( const QString &id )
{
  Q_FOREACH ( const QString &child, dependsOnChildAlgorithms( id ) )
  {
    if ( !childAlgorithm( child ).isActive() )
      return false;
  }
  childAlgorithm( id ).setActive( true );
  updateDestinationParameters();
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

void QgsProcessingModelAlgorithm::removeModelParameter( const QString &name )
{
  removeParameter( name );
  mParameterComponents.remove( name );
}

bool QgsProcessingModelAlgorithm::childAlgorithmsDependOnParameter( const QString &name ) const
{
  QMap< QString, ChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    // check whether child requires this parameter
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSources> childParams = childIt->parameterSources();
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSources>::const_iterator paramIt = childParams.constBegin();
    for ( ; paramIt != childParams.constEnd(); ++paramIt )
    {
      Q_FOREACH ( const ChildParameterSource &source, paramIt.value() )
      {
        if ( source.source() == ChildParameterSource::ModelParameter
             && source.parameterName() == name )
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool QgsProcessingModelAlgorithm::otherParametersDependOnParameter( const QString &name ) const
{
  Q_FOREACH ( const QgsProcessingParameterDefinition *def, mParameters )
  {
    if ( def->name() == name )
      continue;

    if ( def->dependsOnOtherParameters().contains( name ) )
      return true;
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
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSources> childParams = childIt->parameterSources();
    QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSources>::const_iterator paramIt = childParams.constBegin();
    for ( ; paramIt != childParams.constEnd(); ++paramIt )
    {
      Q_FOREACH ( const ChildParameterSource &source, paramIt.value() )
      {
        if ( source.source() == ChildParameterSource::ChildOutput
             && source.outputChildId() == childId )
        {
          depends.insert( childIt->childId() );
          dependsOnChildAlgorithmsRecursive( childIt->childId(), depends );
          break;
        }
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
  QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSources> childParams = alg.parameterSources();
  QMap<QString, QgsProcessingModelAlgorithm::ChildParameterSources>::const_iterator paramIt = childParams.constBegin();
  for ( ; paramIt != childParams.constEnd(); ++paramIt )
  {
    Q_FOREACH ( const ChildParameterSource &source, paramIt.value() )
    {
      if ( source.source() == ChildParameterSource::ChildOutput && !depends.contains( source.outputChildId() ) )
      {
        depends.insert( source.outputChildId() );
        dependsOnChildAlgorithmsRecursive( source.outputChildId(), depends );
      }
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

QString QgsProcessingModelAlgorithm::asPythonCommand( const QVariantMap &parameters, QgsProcessingContext &context ) const
{
  if ( mSourceFile.isEmpty() )
    return QString(); // temporary model - can't run as python command

  return QgsProcessingAlgorithm::asPythonCommand( parameters, context );
}

QgsProcessingModelAlgorithm *QgsProcessingModelAlgorithm::create() const
{
  QgsProcessingModelAlgorithm *alg = new QgsProcessingModelAlgorithm();
  alg->loadVariant( toVariant() );
  alg->setProvider( provider() );
  return alg;
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

QVariant QgsProcessingModelAlgorithm::ChildParameterSource::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "source" ), mSource );
  switch ( mSource )
  {
    case ModelParameter:
      map.insert( QStringLiteral( "parameter_name" ), mParameterName );
      break;

    case ChildOutput:
      map.insert( QStringLiteral( "child_id" ), mChildId );
      map.insert( QStringLiteral( "output_name" ), mOutputName );
      break;

    case StaticValue:
      map.insert( QStringLiteral( "static_value" ), mStaticValue );
      break;
  }
  return map;
}

bool QgsProcessingModelAlgorithm::ChildParameterSource::loadVariant( const QVariantMap &map )
{
  mSource = static_cast< Source >( map.value( QStringLiteral( "source" ) ).toInt() );
  switch ( mSource )
  {
    case ModelParameter:
      mParameterName = map.value( QStringLiteral( "parameter_name" ) ).toString();
      break;

    case ChildOutput:
      mChildId = map.value( QStringLiteral( "child_id" ) ).toString();
      mOutputName = map.value( QStringLiteral( "output_name" ) ).toString();
      break;

    case StaticValue:
      mStaticValue = map.value( QStringLiteral( "static_value" ) );
      break;
  }
  return true;
}

QString QgsProcessingModelAlgorithm::ChildParameterSource::asPythonCode() const
{
  switch ( mSource )
  {
    case ModelParameter:
      return QStringLiteral( "parameters['%1']" ).arg( mParameterName );

    case ChildOutput:
      return QStringLiteral( "outputs['%1']['%2']" ).arg( mChildId, mOutputName );

    case StaticValue:
      return mStaticValue.toString();
  }
  return QString();
}

QgsProcessingModelAlgorithm::ModelOutput::ModelOutput( const QString &name, const QString &description )
  : QgsProcessingModelAlgorithm::Component( description )
  , mName( name )
{}

QVariant QgsProcessingModelAlgorithm::ModelOutput::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "name" ), mName );
  map.insert( QStringLiteral( "child_id" ), mChildId );
  map.insert( QStringLiteral( "output_name" ), mOutputName );
  saveCommonProperties( map );
  return map;
}

bool QgsProcessingModelAlgorithm::ModelOutput::loadVariant( const QVariantMap &map )
{
  mName = map.value( QStringLiteral( "name" ) ).toString();
  mChildId = map.value( QStringLiteral( "child_id" ) ).toString();
  mOutputName = map.value( QStringLiteral( "output_name" ) ).toString();
  restoreCommonProperties( map );
  return true;
}

QgsProcessingModelAlgorithm::ModelParameter::ModelParameter( const QString &parameterName )
  : QgsProcessingModelAlgorithm::Component()
  , mParameterName( parameterName )
{

}

QVariant QgsProcessingModelAlgorithm::ModelParameter::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "name" ), mParameterName );
  saveCommonProperties( map );
  return map;
}

bool QgsProcessingModelAlgorithm::ModelParameter::loadVariant( const QVariantMap &map )
{
  mParameterName = map.value( QStringLiteral( "name" ) ).toString();
  restoreCommonProperties( map );
  return true;
}

///@endcond
