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
#include "moc_qgsprocessingmodelalgorithm.cpp"
#include "qgsprocessingregistry.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingutils.h"
#include "qgis.h"
#include "qgsxmlutils.h"
#include "qgsexception.h"
#include "qgsvectorlayer.h"
#include "qgsstringutils.h"
#include "qgsapplication.h"
#include "qgsprocessingparametertype.h"
#include "qgsexpressioncontextutils.h"
#include "qgsprocessingmodelgroupbox.h"
#include "qgsmessagelog.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
///@cond NOT_STABLE

QgsProcessingModelAlgorithm::QgsProcessingModelAlgorithm( const QString &name, const QString &group, const QString &groupId )
  : mModelName( name.isEmpty() ? QObject::tr( "model" ) : name )
  , mModelGroup( group )
  , mModelGroupId( groupId )
{}

void QgsProcessingModelAlgorithm::initAlgorithm( const QVariantMap & )
{
}

Qgis::ProcessingAlgorithmFlags QgsProcessingModelAlgorithm::flags() const
{
  Qgis::ProcessingAlgorithmFlags res = QgsProcessingAlgorithm::flags();

  // don't force algorithm attachment here, that's potentially too expensive
  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( childIt->algorithm() && childIt->algorithm()->flags().testFlag( Qgis::ProcessingAlgorithmFlag::SecurityRisk ) )
    {
      // security risk flag propagates from child algorithms to model
      res |= Qgis::ProcessingAlgorithmFlag::SecurityRisk;
    }
  }
  return res;
}

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

QString QgsProcessingModelAlgorithm::groupId() const
{
  return mModelGroupId;
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
  if ( mHelpContent.empty() )
    return QString();

  return QgsProcessingUtils::formatHelpMapAsHtml( mHelpContent, this );
}

QString QgsProcessingModelAlgorithm::shortDescription() const
{
  return mHelpContent.value( QStringLiteral( "SHORT_DESCRIPTION" ) ).toString();
}

QString QgsProcessingModelAlgorithm::helpUrl() const
{
  return mHelpContent.value( QStringLiteral( "HELP_URL" ) ).toString();
}

QVariantMap QgsProcessingModelAlgorithm::parametersForChildAlgorithm( const QgsProcessingModelChildAlgorithm &child, const QVariantMap &modelParameters, const QVariantMap &results, const QgsExpressionContext &expressionContext, QString &error, const QgsProcessingContext *context ) const
{
  error.clear();
  auto evaluateSources = [&child, &modelParameters, &results, &error, &expressionContext]( const QgsProcessingParameterDefinition * def )->QVariant
  {
    const QgsProcessingModelChildParameterSources paramSources = child.parameterSources().value( def->name() );

    QString expressionText;
    QVariantList paramParts;
    for ( const QgsProcessingModelChildParameterSource &source : paramSources )
    {
      switch ( source.source() )
      {
        case Qgis::ProcessingModelChildParameterSource::StaticValue:
          paramParts << source.staticValue();
          break;

        case Qgis::ProcessingModelChildParameterSource::ModelParameter:
          paramParts << modelParameters.value( source.parameterName() );
          break;

        case Qgis::ProcessingModelChildParameterSource::ChildOutput:
        {
          QVariantMap linkedChildResults = results.value( source.outputChildId() ).toMap();
          paramParts << linkedChildResults.value( source.outputName() );
          break;
        }

        case Qgis::ProcessingModelChildParameterSource::Expression:
        {
          QgsExpression exp( source.expression() );
          paramParts << exp.evaluate( &expressionContext );
          if ( exp.hasEvalError() )
          {
            error = QObject::tr( "Could not evaluate expression for parameter %1 for %2: %3" ).arg( def->name(), child.description(), exp.evalErrorString() );
          }
          break;
        }
        case Qgis::ProcessingModelChildParameterSource::ExpressionText:
        {
          expressionText = QgsExpression::replaceExpressionText( source.expressionText(), &expressionContext );
          break;
        }

        case Qgis::ProcessingModelChildParameterSource::ModelOutput:
          break;
      }
    }

    if ( ! expressionText.isEmpty() )
    {
      return expressionText;
    }
    else if ( paramParts.count() == 1 )
      return paramParts.at( 0 );
    else
      return paramParts;
  };


  QVariantMap childParams;
  const QList< const QgsProcessingParameterDefinition * > childParameterDefinitions = child.algorithm()->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *def : childParameterDefinitions )
  {
    if ( !def->isDestination() )
    {
      if ( !child.parameterSources().contains( def->name() ) )
        continue; // use default value

      const QVariant value = evaluateSources( def );
      childParams.insert( def->name(), value );
    }
    else
    {
      const QgsProcessingDestinationParameter *destParam = static_cast< const QgsProcessingDestinationParameter * >( def );

      // is destination linked to one of the final outputs from this model?
      bool isFinalOutput = false;
      QMap<QString, QgsProcessingModelOutput> outputs = child.modelOutputs();
      QMap<QString, QgsProcessingModelOutput>::const_iterator outputIt = outputs.constBegin();
      for ( ; outputIt != outputs.constEnd(); ++outputIt )
      {
        if ( outputIt->childOutputName() == destParam->name() )
        {
          QString paramName = child.childId() + ':' + outputIt.key();
          bool foundParam = false;
          QVariant value;

          // if parameter was specified using child_id:child_name directly, take that
          if ( modelParameters.contains( paramName ) )
          {
            value = modelParameters.value( paramName );
            foundParam  = true;
          }

          // ...otherwise we need to find the corresponding model parameter which matches this output
          if ( !foundParam )
          {
            if ( const QgsProcessingParameterDefinition *modelParam = modelParameterFromChildIdAndOutputName( child.childId(), outputIt.key() ) )
            {
              if ( modelParameters.contains( modelParam->name() ) )
              {
                value = modelParameters.value( modelParam->name() );
                foundParam = true;
              }
            }
          }

          if ( foundParam )
          {
            if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
            {
              // make sure layer output name is correctly set
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

      bool hasExplicitDefinition = false;
      if ( !isFinalOutput && child.parameterSources().contains( def->name() ) )
      {
        // explicitly defined source for output
        const QVariant value = evaluateSources( def );
        if ( value.isValid() )
        {
          childParams.insert( def->name(), value );
          hasExplicitDefinition = true;
        }
      }

      if ( !isFinalOutput && !hasExplicitDefinition )
      {
        // output is temporary

        // check whether it's optional, and if so - is it required?
        bool required = true;
        if ( destParam->flags() & Qgis::ProcessingParameterFlag::Optional )
        {
          required = childOutputIsRequired( child.childId(), destParam->name() );
        }

        // not optional, or required elsewhere in model
        if ( required )
          childParams.insert( destParam->name(), destParam->generateTemporaryDestination( context ) );
      }
    }
  }
  return childParams;
}

const QgsProcessingParameterDefinition *QgsProcessingModelAlgorithm::modelParameterFromChildIdAndOutputName( const QString &childId, const QString &childOutputName ) const
{
  for ( const QgsProcessingParameterDefinition *definition : mParameters )
  {
    if ( !definition->isDestination() )
      continue;

    const QString modelChildId = definition->metadata().value( QStringLiteral( "_modelChildId" ) ).toString();
    const QString modelOutputName = definition->metadata().value( QStringLiteral( "_modelChildOutputName" ) ).toString();

    if ( modelChildId == childId && modelOutputName == childOutputName )
      return definition;
  }
  return nullptr;
}

bool QgsProcessingModelAlgorithm::childOutputIsRequired( const QString &childId, const QString &outputName ) const
{
  // look through all child algs
  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( childIt->childId() == childId || !childIt->isActive() )
      continue;

    // look through all sources for child
    QMap<QString, QgsProcessingModelChildParameterSources> candidateChildParams = childIt->parameterSources();
    QMap<QString, QgsProcessingModelChildParameterSources>::const_iterator childParamIt = candidateChildParams.constBegin();
    for ( ; childParamIt != candidateChildParams.constEnd(); ++childParamIt )
    {
      const auto constValue = childParamIt.value();
      for ( const QgsProcessingModelChildParameterSource &source : constValue )
      {
        if ( source.source() == Qgis::ProcessingModelChildParameterSource::ChildOutput
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

QVariantMap QgsProcessingModelAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QSet< QString > toExecute;
  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  QSet< QString > broken;
  const QSet<QString> childSubset = context.modelInitialRunConfig() ? context.modelInitialRunConfig()->childAlgorithmSubset() : QSet<QString>();
  const bool useSubsetOfChildren = !childSubset.empty();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( childIt->isActive() && ( !useSubsetOfChildren || childSubset.contains( childIt->childId() ) ) )
    {
      if ( childIt->algorithm() )
        toExecute.insert( childIt->childId() );
      else
        broken.insert( childIt->childId() );
    }
  }

  if ( !broken.empty() )
    throw QgsProcessingException( QCoreApplication::translate( "QgsProcessingModelAlgorithm", "Cannot run model, the following algorithms are not available on this system: %1" ).arg( qgsSetJoin( broken, QLatin1String( ", " ) ) ) );

  QElapsedTimer totalTime;
  totalTime.start();

  QgsProcessingMultiStepFeedback modelFeedback( toExecute.count(), feedback );
  QgsExpressionContext baseContext = createExpressionContext( parameters, context );

  QVariantMap &childInputs = context.modelResult().rawChildInputs();
  QVariantMap &childResults = context.modelResult().rawChildOutputs();
  QSet< QString > &executed = context.modelResult().executedChildIds();

  // start with initial configuration from the context's model configuration (allowing us to
  // resume execution using a previous state)
  if ( QgsProcessingModelInitialRunConfig *config = context.modelInitialRunConfig() )
  {
    childInputs = config->initialChildInputs();
    childResults = config->initialChildOutputs();
    executed = config->previouslyExecutedChildAlgorithms();
    // discard the model config, this should only be used when running the top level model
    context.setModelInitialRunConfig( nullptr );
  }
  if ( useSubsetOfChildren )
  {
    executed.subtract( childSubset );
  }

  QVariantMap finalResults;

  bool executedAlg = true;
  int previousHtmlLogLength = feedback->htmlLog().length();
  int countExecuted = 0;
  while ( executedAlg && countExecuted < toExecute.count() )
  {
    executedAlg = false;
    for ( const QString &childId : std::as_const( toExecute ) )
    {
      if ( feedback && feedback->isCanceled() )
        break;

      if ( executed.contains( childId ) )
        continue;

      bool canExecute = true;
      const QSet< QString > dependencies = dependsOnChildAlgorithms( childId );
      for ( const QString &dependency : dependencies )
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

      const QgsProcessingModelChildAlgorithm &child = mChildAlgorithms[ childId ];
      std::unique_ptr< QgsProcessingAlgorithm > childAlg( child.algorithm()->create( child.configuration() ) );

      bool skipGenericLogging = true;
      switch ( context.logLevel() )
      {
        case Qgis::ProcessingLogLevel::DefaultLevel:
          // at default log level we skip all the generic logs about prepare steps, step inputs and outputs
          skipGenericLogging = true;
          break;
        case Qgis::ProcessingLogLevel::Verbose:
          // at verbose log level we include all the generic logs about prepare steps, step inputs and outputs
          // UNLESS the algorithm specifically tells to skip these (eg raise warning steps and other special cases)
          skipGenericLogging = childAlg->flags() & Qgis::ProcessingAlgorithmFlag::SkipGenericModelLogging;
          break;
        case Qgis::ProcessingLogLevel::ModelDebug:
          // at model debug log level we'll show all the generic logs for step preparation, inputs and outputs
          // for every child algorithm
          skipGenericLogging = false;
          break;
      }

      if ( feedback && !skipGenericLogging )
        feedback->pushDebugInfo( QObject::tr( "Prepare algorithm: %1" ).arg( childId ) );

      QgsExpressionContext expContext = baseContext;
      expContext << QgsExpressionContextUtils::processingAlgorithmScope( child.algorithm(), parameters, context )
                 << createExpressionContextScopeForChildAlgorithm( childId, context, parameters, childResults );
      context.setExpressionContext( expContext );

      QString error;
      QVariantMap childParams = parametersForChildAlgorithm( child, parameters, childResults, expContext, error, &context );
      if ( !error.isEmpty() )
        throw QgsProcessingException( error );

      if ( feedback && !skipGenericLogging )
        feedback->setProgressText( QObject::tr( "Running %1 [%2/%3]" ).arg( child.description() ).arg( executed.count() + 1 ).arg( toExecute.count() ) );

      QgsProcessingModelChildAlgorithmResult childResult;

      const QVariantMap thisChildParams = QgsProcessingUtils::removePointerValuesFromMap( childParams );
      childInputs.insert( childId, thisChildParams );
      childResult.setInputs( thisChildParams );

      QStringList params;
      for ( auto childParamIt = childParams.constBegin(); childParamIt != childParams.constEnd(); ++childParamIt )
      {
        params << QStringLiteral( "%1: %2" ).arg( childParamIt.key(),
               child.algorithm()->parameterDefinition( childParamIt.key() )->valueAsPythonString( childParamIt.value(), context ) );
      }

      if ( feedback && !skipGenericLogging )
      {
        feedback->pushInfo( QObject::tr( "Input Parameters:" ) );
        feedback->pushCommandInfo( QStringLiteral( "{ %1 }" ).arg( params.join( QLatin1String( ", " ) ) ) );
      }

      QElapsedTimer childTime;
      childTime.start();

      bool ok = false;

      QThread *modelThread = QThread::currentThread();

      auto prepareOnMainThread = [modelThread, &ok, &childAlg, &childParams, &context, &modelFeedback]
      {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "QgsProcessingModelAlgorithm::processAlgorithm", "childAlg->prepare() must be run on the main thread" );
        ok = childAlg->prepare( childParams, context, &modelFeedback );
        context.pushToThread( modelThread );
      };

      // Make sure we only run prepare steps on the main thread!
      if ( modelThread == qApp->thread() )
        ok = childAlg->prepare( childParams, context, &modelFeedback );
      else
      {
        context.pushToThread( qApp->thread() );
// silence false positive leak warning
#ifndef __clang_analyzer__
        QMetaObject::invokeMethod( qApp, prepareOnMainThread, Qt::BlockingQueuedConnection );
#endif
      }

      Q_ASSERT_X( QThread::currentThread() == context.thread(), "QgsProcessingModelAlgorithm::processAlgorithm", "context was not transferred back to model thread" );

      if ( !ok )
      {
        const QString error = ( childAlg->flags() & Qgis::ProcessingAlgorithmFlag::CustomException ) ? QString() : QObject::tr( "Error encountered while running %1" ).arg( child.description() );
        throw QgsProcessingException( error );
      }

      QVariantMap results;

      bool runResult = false;
      try
      {
        if ( ( childAlg->flags() & Qgis::ProcessingAlgorithmFlag::NoThreading ) && ( QThread::currentThread() != qApp->thread() ) )
        {
          // child algorithm run step must be called on main thread
          auto runOnMainThread = [modelThread, &context, &modelFeedback, &results, &childAlg, &childParams]
          {
            Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "QgsProcessingModelAlgorithm::processAlgorithm", "childAlg->runPrepared() must be run on the main thread" );
            results = childAlg->runPrepared( childParams, context, &modelFeedback );
            context.pushToThread( modelThread );
          };

          if ( feedback && !skipGenericLogging && modelThread != qApp->thread() )
            feedback->pushWarning( QObject::tr( "Algorithm “%1” cannot be run in a background thread, switching to main thread for this step" ).arg( childAlg->displayName() ) );

          context.pushToThread( qApp->thread() );
// silence false positive leak warning
#ifndef __clang_analyzer__
          QMetaObject::invokeMethod( qApp, runOnMainThread, Qt::BlockingQueuedConnection );
#endif
        }
        else
        {
          // safe to run on model thread
          results = childAlg->runPrepared( childParams, context, &modelFeedback );
        }
        runResult = true;
        childResult.setExecutionStatus( Qgis::ProcessingModelChildAlgorithmExecutionStatus::Success );
      }
      catch ( QgsProcessingException &e )
      {
        error = ( childAlg->flags() & Qgis::ProcessingAlgorithmFlag::CustomException ) ? e.what() : QObject::tr( "Error encountered while running %1: %2" ).arg( child.description(), e.what() );
        childResult.setExecutionStatus( Qgis::ProcessingModelChildAlgorithmExecutionStatus::Failed );
      }

      Q_ASSERT_X( QThread::currentThread() == context.thread(), "QgsProcessingModelAlgorithm::processAlgorithm", "context was not transferred back to model thread" );

      QVariantMap ppRes;
      auto postProcessOnMainThread = [modelThread, &ppRes, &childAlg, &context, &modelFeedback, runResult]
      {
        Q_ASSERT_X( QThread::currentThread() == qApp->thread(), "QgsProcessingModelAlgorithm::processAlgorithm", "childAlg->postProcess() must be run on the main thread" );
        ppRes = childAlg->postProcess( context, &modelFeedback, runResult );
        context.pushToThread( modelThread );
      };

      // Make sure we only run postProcess steps on the main thread!
      if ( modelThread == qApp->thread() )
        ppRes = childAlg->postProcess( context, &modelFeedback, runResult );
      else
      {
        context.pushToThread( qApp->thread() );
// silence false positive leak warning
#ifndef __clang_analyzer__
        QMetaObject::invokeMethod( qApp, postProcessOnMainThread, Qt::BlockingQueuedConnection );
#endif
      }

      Q_ASSERT_X( QThread::currentThread() == context.thread(), "QgsProcessingModelAlgorithm::processAlgorithm", "context was not transferred back to model thread" );

      if ( !ppRes.isEmpty() )
        results = ppRes;

      childResults.insert( childId, results );
      childResult.setOutputs( results );

      if ( runResult )
      {
        if ( feedback && !skipGenericLogging )
        {
          const QVariantMap displayOutputs = QgsProcessingUtils::removePointerValuesFromMap( results );
          QStringList formattedOutputs;
          for ( auto displayOutputIt = displayOutputs.constBegin(); displayOutputIt != displayOutputs.constEnd(); ++displayOutputIt )
          {
            formattedOutputs << QStringLiteral( "%1: %2" ).arg( displayOutputIt.key(),
                             QgsProcessingUtils::variantToPythonLiteral( displayOutputIt.value() ) );;
          }
          feedback->pushInfo( QObject::tr( "Results:" ) );
          feedback->pushCommandInfo( QStringLiteral( "{ %1 }" ).arg( formattedOutputs.join( QLatin1String( ", " ) ) ) );
        }

        // look through child alg's outputs to determine whether any of these should be copied
        // to the final model outputs
        const QMap<QString, QgsProcessingModelOutput> outputs = child.modelOutputs();
        for ( auto outputIt = outputs.constBegin(); outputIt != outputs.constEnd(); ++outputIt )
        {
          const int outputSortKey = mOutputOrder.indexOf( QStringLiteral( "%1:%2" ).arg( childId, outputIt->childOutputName() ) );
          switch ( mInternalVersion )
          {
            case QgsProcessingModelAlgorithm::InternalVersion::Version1:
              finalResults.insert( childId + ':' + outputIt->name(), results.value( outputIt->childOutputName() ) );
              break;
            case QgsProcessingModelAlgorithm::InternalVersion::Version2:
              if ( const QgsProcessingParameterDefinition *modelParam = modelParameterFromChildIdAndOutputName( child.childId(), outputIt.key() ) )
              {
                finalResults.insert( modelParam->name(), results.value( outputIt->childOutputName() ) );
              }
              break;
          }

          const QString outputLayer = results.value( outputIt->childOutputName() ).toString();
          if ( !outputLayer.isEmpty() && context.willLoadLayerOnCompletion( outputLayer ) )
          {
            QgsProcessingContext::LayerDetails &details = context.layerToLoadOnCompletionDetails( outputLayer );
            details.groupName = mOutputGroup;
            if ( outputSortKey > 0 )
              details.layerSortKey = outputSortKey;
          }
        }

        executed.insert( childId );

        std::function< void( const QString &, const QString & )> pruneAlgorithmBranchRecursive;
        pruneAlgorithmBranchRecursive = [&]( const QString & id, const QString &branch = QString() )
        {
          const QSet<QString> toPrune = dependentChildAlgorithms( id, branch );
          for ( const QString &targetId : toPrune )
          {
            if ( executed.contains( targetId ) )
              continue;

            executed.insert( targetId );
            pruneAlgorithmBranchRecursive( targetId, branch );
          }
        };

        // prune remaining algorithms if they are dependent on a branch from this child which didn't eventuate
        const QgsProcessingOutputDefinitions outputDefs = childAlg->outputDefinitions();
        for ( const QgsProcessingOutputDefinition *outputDef : outputDefs )
        {
          if ( outputDef->type() == QgsProcessingOutputConditionalBranch::typeName() && !results.value( outputDef->name() ).toBool() )
          {
            pruneAlgorithmBranchRecursive( childId, outputDef->name() );
          }
        }

        if ( childAlg->flags() & Qgis::ProcessingAlgorithmFlag::PruneModelBranchesBasedOnAlgorithmResults )
        {
          // check if any dependent algorithms should be canceled based on the outputs of this algorithm run
          // first find all direct dependencies of this algorithm by looking through all remaining child algorithms
          for ( const QString &candidateId : std::as_const( toExecute ) )
          {
            if ( executed.contains( candidateId ) )
              continue;

            // a pending algorithm was found..., check it's parameter sources to see if it links to any of the current
            // algorithm's outputs
            const QgsProcessingModelChildAlgorithm &candidate = mChildAlgorithms[ candidateId ];
            const QMap<QString, QgsProcessingModelChildParameterSources> candidateParams = candidate.parameterSources();
            QMap<QString, QgsProcessingModelChildParameterSources>::const_iterator paramIt = candidateParams.constBegin();
            bool pruned = false;
            for ( ; paramIt != candidateParams.constEnd(); ++paramIt )
            {
              for ( const QgsProcessingModelChildParameterSource &source : paramIt.value() )
              {
                if ( source.source() == Qgis::ProcessingModelChildParameterSource::ChildOutput && source.outputChildId() == childId )
                {
                  // ok, this one is dependent on the current alg. Did we get a value for it?
                  if ( !results.contains( source.outputName() ) )
                  {
                    // oh no, nothing returned for this parameter. Gotta trim the branch back!
                    pruned = true;
                    // skip the dependent alg..
                    executed.insert( candidateId );
                    //... and everything which depends on it
                    pruneAlgorithmBranchRecursive( candidateId, QString() );
                    break;
                  }
                }
              }
              if ( pruned )
                break;
            }
          }
        }

        childAlg.reset( nullptr );
        countExecuted++;
        modelFeedback.setCurrentStep( countExecuted );
        if ( feedback && !skipGenericLogging )
        {
          feedback->pushInfo( QObject::tr( "OK. Execution took %1 s (%n output(s)).", nullptr, results.count() ).arg( childTime.elapsed() / 1000.0 ) );
        }
      }

      // trim out just the portion of the overall log which relates to this child
      const QString thisAlgorithmHtmlLog = feedback->htmlLog().mid( previousHtmlLogLength );
      previousHtmlLogLength = feedback->htmlLog().length();

      if ( !runResult )
      {
        const QString formattedException = QStringLiteral( "<span style=\"color:red\">%1</span><br/>" ).arg( error.toHtmlEscaped() ).replace( '\n', QLatin1String( "<br>" ) );
        const QString formattedRunTime = QStringLiteral( "<span style=\"color:red\">%1</span><br/>" ).arg( QObject::tr( "Failed after %1 s." ).arg( childTime.elapsed() / 1000.0 ).toHtmlEscaped() ).replace( '\n', QLatin1String( "<br>" ) );

        childResult.setHtmlLog( thisAlgorithmHtmlLog + formattedException + formattedRunTime );
        context.modelResult().childResults().insert( childId, childResult );

        throw QgsProcessingException( error );
      }
      else
      {
        childResult.setHtmlLog( thisAlgorithmHtmlLog );
        context.modelResult().childResults().insert( childId, childResult );
      }
    }

    if ( feedback && feedback->isCanceled() )
      break;
  }
  if ( feedback )
    feedback->pushDebugInfo( QObject::tr( "Model processed OK. Executed %n algorithm(s) total in %1 s.", nullptr, countExecuted ).arg( static_cast< double >( totalTime.elapsed() ) / 1000.0 ) );

  mResults = finalResults;
  mResults.insert( QStringLiteral( "CHILD_RESULTS" ), childResults );
  mResults.insert( QStringLiteral( "CHILD_INPUTS" ), childInputs );
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

bool QgsProcessingModelAlgorithm::modelNameMatchesFilePath() const
{
  if ( mSourceFile.isEmpty() )
    return false;

  const QFileInfo fi( mSourceFile );
  return fi.completeBaseName().compare( mModelName, Qt::CaseInsensitive ) == 0;
}

QStringList QgsProcessingModelAlgorithm::asPythonCode( const QgsProcessing::PythonOutputType outputType, const int indentSize ) const
{
  QStringList fileDocString;
  fileDocString << QStringLiteral( "\"\"\"" );
  fileDocString << QStringLiteral( "Model exported as python." );
  fileDocString << QStringLiteral( "Name : %1" ).arg( displayName() );
  fileDocString << QStringLiteral( "Group : %1" ).arg( group() );
  fileDocString << QStringLiteral( "With QGIS : %1" ).arg( Qgis::versionInt() );
  fileDocString << QStringLiteral( "\"\"\"" );
  fileDocString << QString();

  QStringList lines;
  QString indent = QString( ' ' ).repeated( indentSize );
  QString currentIndent;

  QMap< QString, QString> friendlyChildNames;
  QMap< QString, QString> friendlyOutputNames;
  auto uniqueSafeName = []( const QString & name, bool capitalize, const QMap< QString, QString > &friendlyNames )->QString
  {
    const QString base = safeName( name, capitalize );
    QString candidate = base;
    int i = 1;
    while ( friendlyNames.contains( candidate ) )
    {
      i++;
      candidate = QStringLiteral( "%1_%2" ).arg( base ).arg( i );
    }
    return candidate;
  };

  const QString algorithmClassName = safeName( name(), true );

  QSet< QString > toExecute;
  for ( auto childIt = mChildAlgorithms.constBegin(); childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( childIt->isActive() && childIt->algorithm() )
    {
      toExecute.insert( childIt->childId() );
      friendlyChildNames.insert( childIt->childId(), uniqueSafeName( childIt->description().isEmpty() ? childIt->childId() : childIt->description(), !childIt->description().isEmpty(), friendlyChildNames ) );
    }
  }
  const int totalSteps = toExecute.count();

  QStringList importLines; // not a set - we need regular ordering
  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
    {
      // add specific parameter type imports
      const auto params = parameterDefinitions();
      importLines.reserve( params.count() + 6 );
      importLines << QStringLiteral( "from typing import Any, Optional" );
      importLines << QString();
      importLines << QStringLiteral( "from qgis.core import QgsProcessing" );
      importLines << QStringLiteral( "from qgis.core import QgsProcessingAlgorithm" );
      importLines << QStringLiteral( "from qgis.core import QgsProcessingContext" );
      importLines << QStringLiteral( "from qgis.core import QgsProcessingFeedback, QgsProcessingMultiStepFeedback" );

      bool hasAdvancedParams = false;
      for ( const QgsProcessingParameterDefinition *def : params )
      {
        if ( def->flags() & Qgis::ProcessingParameterFlag::Advanced )
          hasAdvancedParams = true;

        const QString importString = QgsApplication::processingRegistry()->parameterType( def->type() )->pythonImportString();
        if ( !importString.isEmpty() && !importLines.contains( importString ) )
          importLines << importString;
      }

      if ( hasAdvancedParams )
        importLines << QStringLiteral( "from qgis.core import QgsProcessingParameterDefinition" );

      lines << QStringLiteral( "from qgis import processing" );
      lines << QString() << QString();

      lines << QStringLiteral( "class %1(QgsProcessingAlgorithm):" ).arg( algorithmClassName );
      lines << QString();

      // initAlgorithm, parameter definitions
      lines << indent + QStringLiteral( "def initAlgorithm(self, config: Optional[dict[str, Any]] = None):" );
      if ( params.empty() )
      {
        lines << indent + indent + QStringLiteral( "pass" );
      }
      else
      {
        lines.reserve( lines.size() + params.size() );
        for ( const QgsProcessingParameterDefinition *def : params )
        {
          std::unique_ptr< QgsProcessingParameterDefinition > defClone( def->clone() );

          if ( defClone->isDestination() )
          {
            const QString uniqueChildName = defClone->metadata().value( QStringLiteral( "_modelChildId" ) ).toString() + ':' + defClone->metadata().value( QStringLiteral( "_modelChildOutputName" ) ).toString();
            const QString friendlyName = !defClone->description().isEmpty() ? uniqueSafeName( defClone->description(), true, friendlyOutputNames ) : defClone->name();
            friendlyOutputNames.insert( uniqueChildName, friendlyName );
            defClone->setName( friendlyName );
          }
          else
          {
            if ( !mParameterComponents.value( defClone->name() ).comment()->description().isEmpty() )
            {
              const QStringList parts = mParameterComponents.value( defClone->name() ).comment()->description().split( QStringLiteral( "\n" ) );
              for ( const QString &part : parts )
              {
                lines << indent + indent + QStringLiteral( "# %1" ).arg( part );
              }
            }
          }

          if ( defClone->flags() & Qgis::ProcessingParameterFlag::Advanced )
          {
            lines << indent + indent + QStringLiteral( "param = %1" ).arg( defClone->asPythonString() );
            lines << indent + indent + QStringLiteral( "param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)" );
            lines << indent + indent + QStringLiteral( "self.addParameter(param)" );
          }
          else
          {
            lines << indent + indent + QStringLiteral( "self.addParameter(%1)" ).arg( defClone->asPythonString() );
          }
        }
      }

      lines << QString();
      lines << indent + QStringLiteral( "def processAlgorithm(self, parameters: dict[str, Any], context: QgsProcessingContext, model_feedback: QgsProcessingFeedback) -> dict[str, Any]:" );
      currentIndent = indent + indent;

      lines << currentIndent + QStringLiteral( "# Use a multi-step feedback, so that individual child algorithm progress reports are adjusted for the" );
      lines << currentIndent + QStringLiteral( "# overall progress through the model" );
      lines << currentIndent + QStringLiteral( "feedback = QgsProcessingMultiStepFeedback(%1, model_feedback)" ).arg( totalSteps );
      break;
    }
#if 0
    case Script:
    {
      QgsStringMap params;
      QgsProcessingContext context;
      QMap< QString, QgsProcessingModelParameter >::const_iterator paramIt = mParameterComponents.constBegin();
      for ( ; paramIt != mParameterComponents.constEnd(); ++paramIt )
      {
        QString name = paramIt.value().parameterName();
        if ( parameterDefinition( name ) )
        {
          // TODO - generic value to string method
          params.insert( name, parameterDefinition( name )->valueAsPythonString( parameterDefinition( name )->defaultValue(), context ) );
        }
      }

      if ( !params.isEmpty() )
      {
        lines << QStringLiteral( "parameters = {" );
        for ( auto it = params.constBegin(); it != params.constEnd(); ++it )
        {
          lines << QStringLiteral( "  '%1':%2," ).arg( it.key(), it.value() );
        }
        lines << QStringLiteral( "}" )
              << QString();
      }

      lines << QStringLiteral( "context = QgsProcessingContext()" )
            << QStringLiteral( "context.setProject(QgsProject.instance())" )
            << QStringLiteral( "feedback = QgsProcessingFeedback()" )
            << QString();

      break;
    }
#endif

  }

  lines << currentIndent + QStringLiteral( "results = {}" );
  lines << currentIndent + QStringLiteral( "outputs = {}" );
  lines << QString();

  QSet< QString > executed;
  bool executedAlg = true;
  int currentStep = 0;
  while ( executedAlg && executed.count() < toExecute.count() )
  {
    executedAlg = false;
    const auto constToExecute = toExecute;
    for ( const QString &childId : constToExecute )
    {
      if ( executed.contains( childId ) )
        continue;

      bool canExecute = true;
      const auto constDependsOnChildAlgorithms = dependsOnChildAlgorithms( childId );
      for ( const QString &dependency : constDependsOnChildAlgorithms )
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

      const QgsProcessingModelChildAlgorithm &child = mChildAlgorithms[ childId ];

      // fill in temporary outputs
      const QgsProcessingParameterDefinitions childDefs = child.algorithm()->parameterDefinitions();
      QgsStringMap childParams;
      for ( const QgsProcessingParameterDefinition *def : childDefs )
      {
        if ( def->isDestination() )
        {
          const QgsProcessingDestinationParameter *destParam = static_cast< const QgsProcessingDestinationParameter * >( def );

          // is destination linked to one of the final outputs from this model?
          bool isFinalOutput = false;
          QMap<QString, QgsProcessingModelOutput> outputs = child.modelOutputs();
          QMap<QString, QgsProcessingModelOutput>::const_iterator outputIt = outputs.constBegin();
          for ( ; outputIt != outputs.constEnd(); ++outputIt )
          {
            if ( outputIt->childOutputName() == destParam->name() )
            {
              QString paramName = child.childId() + ':' + outputIt.key();
              paramName = friendlyOutputNames.value( paramName, paramName );
              childParams.insert( destParam->name(), QStringLiteral( "parameters['%1']" ).arg( paramName ) );
              isFinalOutput = true;
              break;
            }
          }

          if ( !isFinalOutput )
          {
            // output is temporary

            // check whether it's optional, and if so - is it required?
            bool required = true;
            if ( destParam->flags() & Qgis::ProcessingParameterFlag::Optional )
            {
              required = childOutputIsRequired( child.childId(), destParam->name() );
            }

            // not optional, or required elsewhere in model
            if ( required )
            {
              childParams.insert( destParam->name(), QStringLiteral( "QgsProcessing.TEMPORARY_OUTPUT" ) );
            }
          }
        }
      }

      lines << child.asPythonCode( outputType, childParams, currentIndent.size(), indentSize, friendlyChildNames, friendlyOutputNames );
      currentStep++;
      if ( currentStep < totalSteps )
      {
        lines << QString();
        lines << currentIndent + QStringLiteral( "feedback.setCurrentStep(%1)" ).arg( currentStep );
        lines << currentIndent + QStringLiteral( "if feedback.isCanceled():" );
        lines << currentIndent + indent + QStringLiteral( "return {}" );
        lines << QString();
      }
      executed.insert( childId );
    }
  }

  switch ( outputType )
  {
    case QgsProcessing::PythonOutputType::PythonQgsProcessingAlgorithmSubclass:
      lines << currentIndent + QStringLiteral( "return results" );
      lines << QString();

      // name, displayName
      lines << indent + QStringLiteral( "def name(self) -> str:" );
      lines << indent + indent + QStringLiteral( "return '%1'" ).arg( mModelName );
      lines << QString();
      lines << indent + QStringLiteral( "def displayName(self) -> str:" );
      lines << indent + indent + QStringLiteral( "return '%1'" ).arg( mModelName );
      lines << QString();

      // group, groupId
      lines << indent + QStringLiteral( "def group(self) -> str:" );
      lines << indent + indent + QStringLiteral( "return '%1'" ).arg( mModelGroup );
      lines << QString();
      lines << indent + QStringLiteral( "def groupId(self) -> str:" );
      lines << indent + indent + QStringLiteral( "return '%1'" ).arg( mModelGroupId );
      lines << QString();

      // help
      if ( !shortHelpString().isEmpty() )
      {
        lines << indent + QStringLiteral( "def shortHelpString(self) -> str:" );
        lines << indent + indent + QStringLiteral( "return \"\"\"%1\"\"\"" ).arg( shortHelpString() );
        lines << QString();
      }
      if ( !helpUrl().isEmpty() )
      {
        lines << indent + QStringLiteral( "def helpUrl(self) -> str:" );
        lines << indent + indent + QStringLiteral( "return '%1'" ).arg( helpUrl() );
        lines << QString();
      }

      // createInstance
      lines << indent + QStringLiteral( "def createInstance(self):" );
      lines << indent + indent + QStringLiteral( "return self.__class__()" );

      // additional import lines
      static QMap< QString, QString > sAdditionalImports
      {
        { QStringLiteral( "QgsCoordinateReferenceSystem" ), QStringLiteral( "from qgis.core import QgsCoordinateReferenceSystem" ) },
        { QStringLiteral( "QgsExpression" ), QStringLiteral( "from qgis.core import QgsExpression" ) },
        { QStringLiteral( "QgsRectangle" ), QStringLiteral( "from qgis.core import QgsRectangle" ) },
        { QStringLiteral( "QgsReferencedRectangle" ), QStringLiteral( "from qgis.core import QgsReferencedRectangle" ) },
        { QStringLiteral( "QgsPoint" ), QStringLiteral( "from qgis.core import QgsPoint" ) },
        { QStringLiteral( "QgsReferencedPoint" ), QStringLiteral( "from qgis.core import QgsReferencedPoint" ) },
        { QStringLiteral( "QgsProperty" ), QStringLiteral( "from qgis.core import QgsProperty" ) },
        { QStringLiteral( "QgsRasterLayer" ), QStringLiteral( "from qgis.core import QgsRasterLayer" ) },
        { QStringLiteral( "QgsMeshLayer" ), QStringLiteral( "from qgis.core import QgsMeshLayer" ) },
        { QStringLiteral( "QgsVectorLayer" ), QStringLiteral( "from qgis.core import QgsVectorLayer" ) },
        { QStringLiteral( "QgsMapLayer" ), QStringLiteral( "from qgis.core import QgsMapLayer" ) },
        { QStringLiteral( "QgsProcessingFeatureSourceDefinition" ), QStringLiteral( "from qgis.core import QgsProcessingFeatureSourceDefinition" ) },
        { QStringLiteral( "QgsPointXY" ), QStringLiteral( "from qgis.core import QgsPointXY" ) },
        { QStringLiteral( "QgsReferencedPointXY" ), QStringLiteral( "from qgis.core import QgsReferencedPointXY" ) },
        { QStringLiteral( "QgsGeometry" ), QStringLiteral( "from qgis.core import QgsGeometry" ) },
        { QStringLiteral( "QgsProcessingOutputLayerDefinition" ), QStringLiteral( "from qgis.core import QgsProcessingOutputLayerDefinition" ) },
        { QStringLiteral( "QColor" ), QStringLiteral( "from qgis.PyQt.QtGui import QColor" ) },
        { QStringLiteral( "QDateTime" ), QStringLiteral( "from qgis.PyQt.QtCore import QDateTime" ) },
        { QStringLiteral( "QDate" ), QStringLiteral( "from qgis.PyQt.QtCore import QDate" ) },
        { QStringLiteral( "QTime" ), QStringLiteral( "from qgis.PyQt.QtCore import QTime" ) },
      };

      for ( auto it = sAdditionalImports.constBegin(); it != sAdditionalImports.constEnd(); ++it )
      {
        if ( importLines.contains( it.value() ) )
        {
          // already got this import
          continue;
        }

        bool found = false;
        for ( const QString &line : std::as_const( lines ) )
        {
          if ( line.contains( it.key() ) )
          {
            found = true;
            break;
          }
        }
        if ( found )
        {
          importLines << it.value();
        }
      }

      lines = fileDocString + importLines + lines;
      break;
  }

  lines << QString();

  return lines;
}

QMap<QString, QgsProcessingModelAlgorithm::VariableDefinition> QgsProcessingModelAlgorithm::variablesForChildAlgorithm( const QString &childId, QgsProcessingContext *context, const QVariantMap &modelParameters, const QVariantMap &results ) const
{
  QMap<QString, QgsProcessingModelAlgorithm::VariableDefinition> variables;

  auto safeName = []( const QString & name )->QString
  {
    QString s = name;
    const thread_local QRegularExpression safeNameRe( QStringLiteral( "[\\s'\"\\(\\):\\.]" ) );
    return s.replace( safeNameRe, QStringLiteral( "_" ) );
  };

  // "static"/single value sources
  QgsProcessingModelChildParameterSources sources = availableSourcesForChild( childId, QStringList() << QgsProcessingParameterNumber::typeName()
      << QgsProcessingParameterDistance::typeName()
      << QgsProcessingParameterArea::typeName()
      << QgsProcessingParameterVolume::typeName()
      << QgsProcessingParameterDuration::typeName()
      << QgsProcessingParameterScale::typeName()
      << QgsProcessingParameterBoolean::typeName()
      << QgsProcessingParameterEnum::typeName()
      << QgsProcessingParameterExpression::typeName()
      << QgsProcessingParameterField::typeName()
      << QgsProcessingParameterString::typeName()
      << QgsProcessingParameterAuthConfig::typeName()
      << QgsProcessingParameterCrs::typeName()
      << QgsProcessingParameterRange::typeName()
      << QgsProcessingParameterPoint::typeName()
      << QgsProcessingParameterGeometry::typeName()
      << QgsProcessingParameterFile::typeName()
      << QgsProcessingParameterFolderDestination::typeName()
      << QgsProcessingParameterBand::typeName()
      << QgsProcessingParameterLayout::typeName()
      << QgsProcessingParameterLayoutItem::typeName()
      << QgsProcessingParameterColor::typeName()
      << QgsProcessingParameterCoordinateOperation::typeName()
      << QgsProcessingParameterMapTheme::typeName()
      << QgsProcessingParameterDateTime::typeName()
      << QgsProcessingParameterProviderConnection::typeName()
      << QgsProcessingParameterDatabaseSchema::typeName()
      << QgsProcessingParameterDatabaseTable::typeName(),
      QStringList() << QgsProcessingOutputNumber::typeName()
      << QgsProcessingOutputString::typeName()
      << QgsProcessingOutputBoolean::typeName()
      << QgsProcessingOutputVariant::typeName() );

  for ( const QgsProcessingModelChildParameterSource &source : std::as_const( sources ) )
  {
    QString name;
    QVariant value;
    QString description;
    switch ( source.source() )
    {
      case Qgis::ProcessingModelChildParameterSource::ModelParameter:
      {
        name = source.parameterName();
        value = modelParameters.value( source.parameterName() );
        description = parameterDefinition( source.parameterName() )->description();
        break;
      }
      case Qgis::ProcessingModelChildParameterSource::ChildOutput:
      {
        const QgsProcessingModelChildAlgorithm &child = mChildAlgorithms.value( source.outputChildId() );
        name = QStringLiteral( "%1_%2" ).arg( child.description().isEmpty() ?
                                              source.outputChildId() : child.description(), source.outputName() );
        if ( const QgsProcessingAlgorithm *alg = child.algorithm() )
        {
          description = QObject::tr( "Output '%1' from algorithm '%2'" ).arg( alg->outputDefinition( source.outputName() )->description(),
                        child.description() );
        }
        value = results.value( source.outputChildId() ).toMap().value( source.outputName() );
        break;
      }

      case Qgis::ProcessingModelChildParameterSource::Expression:
      case Qgis::ProcessingModelChildParameterSource::ExpressionText:
      case Qgis::ProcessingModelChildParameterSource::StaticValue:
      case Qgis::ProcessingModelChildParameterSource::ModelOutput:
        continue;
    }
    variables.insert( safeName( name ), VariableDefinition( value, source, description ) );
  }

  // layer sources
  sources = availableSourcesForChild( childId, QStringList()
                                      << QgsProcessingParameterVectorLayer::typeName()
                                      << QgsProcessingParameterRasterLayer::typeName(),
                                      QStringList() << QgsProcessingOutputVectorLayer::typeName()
                                      << QgsProcessingOutputRasterLayer::typeName()
                                      << QgsProcessingOutputMapLayer::typeName() );

  for ( const QgsProcessingModelChildParameterSource &source : std::as_const( sources ) )
  {
    QString name;
    QVariant value;
    QString description;

    switch ( source.source() )
    {
      case Qgis::ProcessingModelChildParameterSource::ModelParameter:
      {
        name = source.parameterName();
        value = modelParameters.value( source.parameterName() );
        description = parameterDefinition( source.parameterName() )->description();
        break;
      }
      case Qgis::ProcessingModelChildParameterSource::ChildOutput:
      {
        const QgsProcessingModelChildAlgorithm &child = mChildAlgorithms.value( source.outputChildId() );
        name = QStringLiteral( "%1_%2" ).arg( child.description().isEmpty() ?
                                              source.outputChildId() : child.description(), source.outputName() );
        value = results.value( source.outputChildId() ).toMap().value( source.outputName() );
        if ( const QgsProcessingAlgorithm *alg = child.algorithm() )
        {
          description = QObject::tr( "Output '%1' from algorithm '%2'" ).arg( alg->outputDefinition( source.outputName() )->description(),
                        child.description() );
        }
        break;
      }

      case Qgis::ProcessingModelChildParameterSource::Expression:
      case Qgis::ProcessingModelChildParameterSource::ExpressionText:
      case Qgis::ProcessingModelChildParameterSource::StaticValue:
      case Qgis::ProcessingModelChildParameterSource::ModelOutput:
        continue;

    }

    if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
    {
      QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
      value = fromVar.sink;
      if ( value.userType() == qMetaTypeId<QgsProperty>() && context )
      {
        value = value.value< QgsProperty >().valueAsString( context->expressionContext() );
      }
    }
    QgsMapLayer *layer = nullptr;
    if ( context )
    {
      layer = qobject_cast< QgsMapLayer * >( qvariant_cast<QObject *>( value ) );
      if ( !layer )
        layer = QgsProcessingUtils::mapLayerFromString( value.toString(), *context );
    }

    variables.insert( safeName( name ), VariableDefinition( layer ? QVariant::fromValue( QgsWeakMapLayerPointer( layer ) ) : QVariant(), source, description ) );
    variables.insert( safeName( QStringLiteral( "%1_minx" ).arg( name ) ), VariableDefinition( layer ? layer->extent().xMinimum() : QVariant(), source, QObject::tr( "Minimum X of %1" ).arg( description ) ) );
    variables.insert( safeName( QStringLiteral( "%1_miny" ).arg( name ) ), VariableDefinition( layer ? layer->extent().yMinimum() : QVariant(), source, QObject::tr( "Minimum Y of %1" ).arg( description ) ) );
    variables.insert( safeName( QStringLiteral( "%1_maxx" ).arg( name ) ), VariableDefinition( layer ? layer->extent().xMaximum() : QVariant(), source, QObject::tr( "Maximum X of %1" ).arg( description ) ) );
    variables.insert( safeName( QStringLiteral( "%1_maxy" ).arg( name ) ), VariableDefinition( layer ? layer->extent().yMaximum() : QVariant(), source, QObject::tr( "Maximum Y of %1" ).arg( description ) ) );
  }

  sources = availableSourcesForChild( childId, QStringList()
                                      << QgsProcessingParameterFeatureSource::typeName() );
  for ( const QgsProcessingModelChildParameterSource &source : std::as_const( sources ) )
  {
    QString name;
    QVariant value;
    QString description;

    switch ( source.source() )
    {
      case Qgis::ProcessingModelChildParameterSource::ModelParameter:
      {
        name = source.parameterName();
        value = modelParameters.value( source.parameterName() );
        description = parameterDefinition( source.parameterName() )->description();
        break;
      }
      case Qgis::ProcessingModelChildParameterSource::ChildOutput:
      {
        const QgsProcessingModelChildAlgorithm &child = mChildAlgorithms.value( source.outputChildId() );
        name = QStringLiteral( "%1_%2" ).arg( child.description().isEmpty() ?
                                              source.outputChildId() : child.description(), source.outputName() );
        value = results.value( source.outputChildId() ).toMap().value( source.outputName() );
        if ( const QgsProcessingAlgorithm *alg = child.algorithm() )
        {
          description = QObject::tr( "Output '%1' from algorithm '%2'" ).arg( alg->outputDefinition( source.outputName() )->description(),
                        child.description() );
        }
        break;
      }

      case Qgis::ProcessingModelChildParameterSource::Expression:
      case Qgis::ProcessingModelChildParameterSource::ExpressionText:
      case Qgis::ProcessingModelChildParameterSource::StaticValue:
      case Qgis::ProcessingModelChildParameterSource::ModelOutput:
        continue;

    }

    QgsFeatureSource *featureSource = nullptr;
    if ( value.userType() == qMetaTypeId<QgsProcessingFeatureSourceDefinition>() )
    {
      QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( value );
      value = fromVar.source;
    }
    else if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
    {
      QgsProcessingOutputLayerDefinition fromVar = qvariant_cast<QgsProcessingOutputLayerDefinition>( value );
      value = fromVar.sink;
      if ( context && value.userType() == qMetaTypeId<QgsProperty>() )
      {
        value = value.value< QgsProperty >().valueAsString( context->expressionContext() );
      }
    }
    if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( qvariant_cast<QObject *>( value ) ) )
    {
      featureSource = layer;
    }
    if ( context && !featureSource )
    {
      if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( value.toString(), *context, true, QgsProcessingUtils::LayerHint::Vector ) ) )
        featureSource = vl;
    }

    variables.insert( safeName( name ), VariableDefinition( value, source, description ) );
    variables.insert( safeName( QStringLiteral( "%1_minx" ).arg( name ) ), VariableDefinition( featureSource ? featureSource->sourceExtent().xMinimum() : QVariant(), source, QObject::tr( "Minimum X of %1" ).arg( description ) ) );
    variables.insert( safeName( QStringLiteral( "%1_miny" ).arg( name ) ), VariableDefinition( featureSource ? featureSource->sourceExtent().yMinimum() : QVariant(), source, QObject::tr( "Minimum Y of %1" ).arg( description ) ) );
    variables.insert( safeName( QStringLiteral( "%1_maxx" ).arg( name ) ), VariableDefinition( featureSource ? featureSource->sourceExtent().xMaximum() : QVariant(), source, QObject::tr( "Maximum X of %1" ).arg( description ) ) );
    variables.insert( safeName( QStringLiteral( "%1_maxy" ).arg( name ) ), VariableDefinition( featureSource ? featureSource->sourceExtent().yMaximum() : QVariant(), source, QObject::tr( "Maximum Y of %1" ).arg( description ) ) );
  }

  return variables;
}

QgsExpressionContextScope *QgsProcessingModelAlgorithm::createExpressionContextScopeForChildAlgorithm( const QString &childId, QgsProcessingContext &context, const QVariantMap &modelParameters, const QVariantMap &results ) const
{
  std::unique_ptr< QgsExpressionContextScope > scope( new QgsExpressionContextScope( QStringLiteral( "algorithm_inputs" ) ) );
  QMap< QString, QgsProcessingModelAlgorithm::VariableDefinition> variables = variablesForChildAlgorithm( childId, &context, modelParameters, results );
  QMap< QString, QgsProcessingModelAlgorithm::VariableDefinition>::const_iterator varIt = variables.constBegin();
  for ( ; varIt != variables.constEnd(); ++varIt )
  {
    scope->addVariable( QgsExpressionContextScope::StaticVariable( varIt.key(), varIt->value, true, false, varIt->description ) );
  }
  return scope.release();
}

QgsProcessingModelChildParameterSources QgsProcessingModelAlgorithm::availableSourcesForChild( const QString &childId, const QStringList &parameterTypes, const QStringList &outputTypes, const QList<int> &dataTypes ) const
{
  QgsProcessingModelChildParameterSources sources;

  // first look through model parameters
  QMap< QString, QgsProcessingModelParameter >::const_iterator paramIt = mParameterComponents.constBegin();
  for ( ; paramIt != mParameterComponents.constEnd(); ++paramIt )
  {
    const QgsProcessingParameterDefinition *def = parameterDefinition( paramIt->parameterName() );
    if ( !def )
      continue;

    if ( parameterTypes.contains( def->type() ) )
    {
      if ( !dataTypes.isEmpty() )
      {
        if ( def->type() == QgsProcessingParameterField::typeName() )
        {
          const QgsProcessingParameterField *fieldDef = static_cast< const QgsProcessingParameterField * >( def );
          if ( !( dataTypes.contains( static_cast< int >( fieldDef->dataType() ) ) || fieldDef->dataType() == Qgis::ProcessingFieldParameterDataType::Any ) )
          {
            continue;
          }
        }
        else if ( def->type() == QgsProcessingParameterFeatureSource::typeName() || def->type() == QgsProcessingParameterVectorLayer::typeName() )
        {
          const QgsProcessingParameterLimitedDataTypes *sourceDef = dynamic_cast< const QgsProcessingParameterLimitedDataTypes *>( def );
          if ( !sourceDef )
            continue;

          bool ok = sourceDef->dataTypes().isEmpty();
          const auto constDataTypes = sourceDef->dataTypes();
          for ( int type : constDataTypes )
          {
            if ( dataTypes.contains( type ) || type == static_cast< int >( Qgis::ProcessingSourceType::MapLayer ) || type == static_cast< int >( Qgis::ProcessingSourceType::Vector ) || type == static_cast< int >( Qgis::ProcessingSourceType::VectorAnyGeometry ) )
            {
              ok = true;
              break;
            }
          }
          if ( dataTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::MapLayer ) ) || dataTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::Vector ) ) || dataTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) )
            ok = true;

          if ( !ok )
            continue;
        }
      }
      sources << QgsProcessingModelChildParameterSource::fromModelParameter( paramIt->parameterName() );
    }
  }

  QSet< QString > dependents;
  if ( !childId.isEmpty() )
  {
    dependents = dependentChildAlgorithms( childId );
    dependents << childId;
  }

  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( dependents.contains( childIt->childId() ) )
      continue;

    const QgsProcessingAlgorithm *alg = childIt->algorithm();
    if ( !alg )
      continue;

    const auto constOutputDefinitions = alg->outputDefinitions();
    for ( const QgsProcessingOutputDefinition *out : constOutputDefinitions )
    {
      if ( outputTypes.contains( out->type() ) )
      {
        if ( !dataTypes.isEmpty() )
        {
          if ( out->type() == QgsProcessingOutputVectorLayer::typeName() )
          {
            const QgsProcessingOutputVectorLayer *vectorOut = static_cast< const QgsProcessingOutputVectorLayer *>( out );

            if ( !vectorOutputIsCompatibleType( dataTypes, vectorOut->dataType() ) )
            {
              //unacceptable output
              continue;
            }
          }
        }
        sources << QgsProcessingModelChildParameterSource::fromChildOutput( childIt->childId(), out->name() );
      }
    }
  }

  return sources;
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

bool QgsProcessingModelAlgorithm::validate( QStringList &issues ) const
{
  issues.clear();
  bool res = true;

  if ( mChildAlgorithms.empty() )
  {
    res = false;
    issues << QObject::tr( "Model does not contain any algorithms" );
  }

  for ( auto it = mChildAlgorithms.constBegin(); it != mChildAlgorithms.constEnd(); ++it )
  {
    QStringList childIssues;
    res = validateChildAlgorithm( it->childId(), childIssues ) && res;

    for ( const QString &issue : std::as_const( childIssues ) )
    {
      issues << QStringLiteral( "<b>%1</b>: %2" ).arg( it->description(), issue );
    }
  }
  return res;
}

QMap<QString, QgsProcessingModelChildAlgorithm> QgsProcessingModelAlgorithm::childAlgorithms() const
{
  return mChildAlgorithms;
}

void QgsProcessingModelAlgorithm::setParameterComponents( const QMap<QString, QgsProcessingModelParameter> &parameterComponents )
{
  mParameterComponents = parameterComponents;
}

void QgsProcessingModelAlgorithm::setParameterComponent( const QgsProcessingModelParameter &component )
{
  mParameterComponents.insert( component.parameterName(), component );
}

QgsProcessingModelParameter &QgsProcessingModelAlgorithm::parameterComponent( const QString &name )
{
  if ( !mParameterComponents.contains( name ) )
  {
    QgsProcessingModelParameter &component = mParameterComponents[ name ];
    component.setParameterName( name );
    return component;
  }
  return mParameterComponents[ name ];
}

QList< QgsProcessingModelParameter > QgsProcessingModelAlgorithm::orderedParameters() const
{
  QList< QgsProcessingModelParameter > res;
  QSet< QString > found;
  for ( const QString &parameter : mParameterOrder )
  {
    if ( mParameterComponents.contains( parameter ) )
    {
      res << mParameterComponents.value( parameter );
      found << parameter;
    }
  }

  // add any missing ones to end of list
  for ( auto it = mParameterComponents.constBegin(); it != mParameterComponents.constEnd(); ++it )
  {
    if ( !found.contains( it.key() ) )
    {
      res << it.value();
    }
  }
  return res;
}

void QgsProcessingModelAlgorithm::setParameterOrder( const QStringList &order )
{
  mParameterOrder = order;
}

QList<QgsProcessingModelOutput> QgsProcessingModelAlgorithm::orderedOutputs() const
{
  QList< QgsProcessingModelOutput > res;
  QSet< QString > found;

  for ( const QString &output : mOutputOrder )
  {
    bool foundOutput = false;
    for ( auto it = mChildAlgorithms.constBegin(); it != mChildAlgorithms.constEnd(); ++it )
    {
      const QMap<QString, QgsProcessingModelOutput> outputs = it.value().modelOutputs();
      for ( auto outputIt = outputs.constBegin(); outputIt != outputs.constEnd(); ++outputIt )
      {
        if ( output == QStringLiteral( "%1:%2" ).arg( outputIt->childId(), outputIt->childOutputName() ) )
        {
          res << outputIt.value();
          foundOutput = true;
          found.insert( QStringLiteral( "%1:%2" ).arg( outputIt->childId(), outputIt->childOutputName() ) );
        }
      }
      if ( foundOutput )
        break;
    }
  }

  // add any missing ones to end of list
  for ( auto it = mChildAlgorithms.constBegin(); it != mChildAlgorithms.constEnd(); ++it )
  {
    const QMap<QString, QgsProcessingModelOutput> outputs = it.value().modelOutputs();
    for ( auto outputIt = outputs.constBegin(); outputIt != outputs.constEnd(); ++outputIt )
    {
      if ( !found.contains( QStringLiteral( "%1:%2" ).arg( outputIt->childId(), outputIt->childOutputName() ) ) )
      {
        res << outputIt.value();
      }
    }
  }

  return res;
}

void QgsProcessingModelAlgorithm::setOutputOrder( const QStringList &order )
{
  mOutputOrder = order;
}

QString QgsProcessingModelAlgorithm::outputGroup() const
{
  return mOutputGroup;
}

void QgsProcessingModelAlgorithm::setOutputGroup( const QString &group )
{
  mOutputGroup = group;
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
  QSet< QString > usedFriendlyNames;
  auto uniqueSafeName = [&usedFriendlyNames ]( const QString & name )->QString
  {
    const QString base = safeName( name, false );
    QString candidate = base;
    int i = 1;
    while ( usedFriendlyNames.contains( candidate ) )
    {
      i++;
      candidate = QStringLiteral( "%1_%2" ).arg( base ).arg( i );
    }
    usedFriendlyNames.insert( candidate );
    return candidate;
  };

  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    QMap<QString, QgsProcessingModelOutput> outputs = childIt->modelOutputs();
    QMap<QString, QgsProcessingModelOutput>::const_iterator outputIt = outputs.constBegin();
    for ( ; outputIt != outputs.constEnd(); ++outputIt )
    {
      if ( !childIt->isActive() || !childIt->algorithm() )
        continue;

      // child algorithm has a destination parameter set, copy it to the model
      const QgsProcessingParameterDefinition *source = childIt->algorithm()->parameterDefinition( outputIt->childOutputName() );
      if ( !source )
        continue;

      std::unique_ptr< QgsProcessingParameterDefinition > param( source->clone() );
      // Even if an output was hidden in a child algorithm, we want to show it here for the final
      // outputs.
      param->setFlags( param->flags() & ~ static_cast< int >( Qgis::ProcessingParameterFlag::Hidden ) );
      if ( outputIt->isMandatory() )
        param->setFlags( param->flags() & ~static_cast< int >( Qgis::ProcessingParameterFlag::Optional ) );
      if ( mInternalVersion != InternalVersion::Version1 && !outputIt->description().isEmpty() )
      {
        QString friendlyName = uniqueSafeName( outputIt->description() );
        param->setName( friendlyName );
      }
      else
      {
        param->setName( outputIt->childId() + ':' + outputIt->name() );
      }
      // add some metadata so we can easily link this parameter back to the child source
      param->metadata().insert( QStringLiteral( "_modelChildId" ), outputIt->childId() );
      param->metadata().insert( QStringLiteral( "_modelChildOutputName" ), outputIt->name() );
      param->metadata().insert( QStringLiteral( "_modelChildProvider" ), childIt->algorithm()->provider() ? childIt->algorithm()->provider()->id() : QString() );

      param->setDescription( outputIt->description() );
      param->setDefaultValue( outputIt->defaultValue() );

      QgsProcessingDestinationParameter *newDestParam = dynamic_cast< QgsProcessingDestinationParameter * >( param.get() );
      if ( addParameter( param.release() ) && newDestParam )
      {
        if ( QgsProcessingProvider *provider = childIt->algorithm()->provider() )
        {
          // we need to copy the constraints given by the provider which creates this output across
          // and replace those which have been set to match the model provider's constraints
          newDestParam->setSupportsNonFileBasedOutput( provider->supportsNonFileBasedOutput() );
          newDestParam->mOriginalProvider = provider;
        }
      }
    }
  }
}

void QgsProcessingModelAlgorithm::addGroupBox( const QgsProcessingModelGroupBox &groupBox )
{
  mGroupBoxes.insert( groupBox.uuid(), groupBox );
}

QList<QgsProcessingModelGroupBox> QgsProcessingModelAlgorithm::groupBoxes() const
{
  return mGroupBoxes.values();
}

void QgsProcessingModelAlgorithm::removeGroupBox( const QString &uuid )
{
  mGroupBoxes.remove( uuid );
}

QVariant QgsProcessingModelAlgorithm::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "model_name" ), mModelName );
  map.insert( QStringLiteral( "model_group" ), mModelGroup );
  map.insert( QStringLiteral( "help" ), mHelpContent );
  map.insert( QStringLiteral( "internal_version" ), qgsEnumValueToKey( mInternalVersion ) );

  QVariantMap childMap;
  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    childMap.insert( childIt.key(), childIt.value().toVariant() );
  }
  map.insert( QStringLiteral( "children" ), childMap );

  QVariantMap paramMap;
  QMap< QString, QgsProcessingModelParameter >::const_iterator paramIt = mParameterComponents.constBegin();
  for ( ; paramIt != mParameterComponents.constEnd(); ++paramIt )
  {
    paramMap.insert( paramIt.key(), paramIt.value().toVariant() );
  }
  map.insert( QStringLiteral( "parameters" ), paramMap );

  QVariantMap paramDefMap;
  for ( const QgsProcessingParameterDefinition *def : mParameters )
  {
    paramDefMap.insert( def->name(), def->toVariantMap() );
  }
  map.insert( QStringLiteral( "parameterDefinitions" ), paramDefMap );

  QVariantList groupBoxDefs;
  for ( auto it = mGroupBoxes.constBegin(); it != mGroupBoxes.constEnd(); ++it )
  {
    groupBoxDefs.append( it.value().toVariant() );
  }
  map.insert( QStringLiteral( "groupBoxes" ), groupBoxDefs );

  map.insert( QStringLiteral( "modelVariables" ), mVariables );

  map.insert( QStringLiteral( "designerParameterValues" ), mDesignerParameterValues );

  map.insert( QStringLiteral( "parameterOrder" ), mParameterOrder );
  map.insert( QStringLiteral( "outputOrder" ), mOutputOrder );
  map.insert( QStringLiteral( "outputGroup" ), mOutputGroup );

  return map;
}

bool QgsProcessingModelAlgorithm::loadVariant( const QVariant &model )
{
  QVariantMap map = model.toMap();

  mModelName = map.value( QStringLiteral( "model_name" ) ).toString();
  mModelGroup = map.value( QStringLiteral( "model_group" ) ).toString();
  mModelGroupId = map.value( QStringLiteral( "model_group" ) ).toString();
  mHelpContent = map.value( QStringLiteral( "help" ) ).toMap();

  mInternalVersion = qgsEnumKeyToValue( map.value( QStringLiteral( "internal_version" ) ).toString(), InternalVersion::Version1 );

  mVariables = map.value( QStringLiteral( "modelVariables" ) ).toMap();
  mDesignerParameterValues = map.value( QStringLiteral( "designerParameterValues" ) ).toMap();

  mParameterOrder = map.value( QStringLiteral( "parameterOrder" ) ).toStringList();
  mOutputOrder = map.value( QStringLiteral( "outputOrder" ) ).toStringList();
  mOutputGroup = map.value( QStringLiteral( "outputGroup" ) ).toString();

  mChildAlgorithms.clear();
  QVariantMap childMap = map.value( QStringLiteral( "children" ) ).toMap();
  QVariantMap::const_iterator childIt = childMap.constBegin();
  for ( ; childIt != childMap.constEnd(); ++childIt )
  {
    QgsProcessingModelChildAlgorithm child;
    // we be lenient here - even if we couldn't load a parameter, don't interrupt the model loading
    // otherwise models may become unusable (e.g. due to removed plugins providing algs/parameters)
    // with no way for users to repair them
    if ( !child.loadVariant( childIt.value() ) )
      continue;

    mChildAlgorithms.insert( child.childId(), child );
  }

  mParameterComponents.clear();
  QVariantMap paramMap = map.value( QStringLiteral( "parameters" ) ).toMap();
  QVariantMap::const_iterator paramIt = paramMap.constBegin();
  for ( ; paramIt != paramMap.constEnd(); ++paramIt )
  {
    QgsProcessingModelParameter param;
    if ( !param.loadVariant( paramIt.value().toMap() ) )
      return false;

    mParameterComponents.insert( param.parameterName(), param );
  }

  qDeleteAll( mParameters );
  mParameters.clear();
  QVariantMap paramDefMap = map.value( QStringLiteral( "parameterDefinitions" ) ).toMap();

  auto addParam = [this]( const QVariant & value )
  {
    std::unique_ptr< QgsProcessingParameterDefinition > param( QgsProcessingParameters::parameterFromVariantMap( value.toMap() ) );
    // we be lenient here - even if we couldn't load a parameter, don't interrupt the model loading
    // otherwise models may become unusable (e.g. due to removed plugins providing algs/parameters)
    // with no way for users to repair them
    if ( param )
    {
      if ( param->name() == QLatin1String( "VERBOSE_LOG" ) )
        return; // internal parameter -- some versions of QGIS incorrectly stored this in the model definition file

      // set parameter help from help content
      param->setHelp( mHelpContent.value( param->name() ).toString() );

      // add parameter
      addParameter( param.release() );
    }
    else
    {
      QVariantMap map = value.toMap();
      QString type = map.value( QStringLiteral( "parameter_type" ) ).toString();
      QString name = map.value( QStringLiteral( "name" ) ).toString();

      QgsMessageLog::logMessage( QCoreApplication::translate( "Processing", "Could not load parameter %1 of type %2." ).arg( name, type ), QCoreApplication::translate( "Processing", "Processing" ) );
    }
  };

  QSet< QString > loadedParams;
  // first add parameters respecting mParameterOrder
  for ( const QString &name : std::as_const( mParameterOrder ) )
  {
    if ( paramDefMap.contains( name ) )
    {
      addParam( paramDefMap.value( name ) );
      loadedParams << name;
    }
  }
  // then load any remaining parameters
  QVariantMap::const_iterator paramDefIt = paramDefMap.constBegin();
  for ( ; paramDefIt != paramDefMap.constEnd(); ++paramDefIt )
  {
    if ( !loadedParams.contains( paramDefIt.key() ) )
      addParam( paramDefIt.value() );
  }

  mGroupBoxes.clear();
  const QVariantList groupBoxList = map.value( QStringLiteral( "groupBoxes" ) ).toList();
  for ( const QVariant &groupBoxDef : groupBoxList )
  {
    QgsProcessingModelGroupBox groupBox;
    groupBox.loadVariant( groupBoxDef.toMap() );
    mGroupBoxes.insert( groupBox.uuid(), groupBox );
  }

  updateDestinationParameters();

  return true;
}

bool QgsProcessingModelAlgorithm::vectorOutputIsCompatibleType( const QList<int> &acceptableDataTypes, Qgis::ProcessingSourceType outputType )
{
  // This method is intended to be "permissive" rather than "restrictive".
  // I.e. we only reject outputs which we know can NEVER be acceptable, but
  // if there's doubt then we default to returning true.
  return ( acceptableDataTypes.empty()
           || acceptableDataTypes.contains( static_cast< int >( outputType ) )
           || outputType == Qgis::ProcessingSourceType::MapLayer
           || outputType == Qgis::ProcessingSourceType::Vector
           || outputType == Qgis::ProcessingSourceType::VectorAnyGeometry
           || acceptableDataTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::Vector ) )
           || acceptableDataTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::MapLayer ) )
           || ( acceptableDataTypes.contains( static_cast< int >( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) && ( outputType == Qgis::ProcessingSourceType::VectorPoint ||
                outputType == Qgis::ProcessingSourceType::VectorLine ||
                outputType == Qgis::ProcessingSourceType::VectorPolygon ) ) );
}

void QgsProcessingModelAlgorithm::reattachAlgorithms() const
{
  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( !childIt->algorithm() )
      childIt->reattach();
  }
}

bool QgsProcessingModelAlgorithm::toFile( const QString &path ) const
{
  QDomDocument doc = QDomDocument( QStringLiteral( "model" ) );
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
  else
  {
    return false;
  }

  QVariant props = QgsXmlUtils::readVariant( doc.firstChildElement() );
  return loadVariant( props );
}

void QgsProcessingModelAlgorithm::setChildAlgorithms( const QMap<QString, QgsProcessingModelChildAlgorithm> &childAlgorithms )
{
  mChildAlgorithms = childAlgorithms;
  updateDestinationParameters();
}

void QgsProcessingModelAlgorithm::setChildAlgorithm( const QgsProcessingModelChildAlgorithm &algorithm )
{
  mChildAlgorithms.insert( algorithm.childId(), algorithm );
  updateDestinationParameters();
}

QString QgsProcessingModelAlgorithm::addChildAlgorithm( QgsProcessingModelChildAlgorithm &algorithm )
{
  if ( algorithm.childId().isEmpty() || mChildAlgorithms.contains( algorithm.childId() ) )
    algorithm.generateChildId( *this );

  mChildAlgorithms.insert( algorithm.childId(), algorithm );
  updateDestinationParameters();
  return algorithm.childId();
}

QgsProcessingModelChildAlgorithm &QgsProcessingModelAlgorithm::childAlgorithm( const QString &childId )
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
  const auto constDependentChildAlgorithms = dependentChildAlgorithms( id );
  for ( const QString &child : constDependentChildAlgorithms )
  {
    childAlgorithm( child ).setActive( false );
  }
  childAlgorithm( id ).setActive( false );
  updateDestinationParameters();
}

bool QgsProcessingModelAlgorithm::activateChildAlgorithm( const QString &id )
{
  const auto constDependsOnChildAlgorithms = dependsOnChildAlgorithms( id );
  for ( const QString &child : constDependsOnChildAlgorithms )
  {
    if ( !childAlgorithm( child ).isActive() )
      return false;
  }
  childAlgorithm( id ).setActive( true );
  updateDestinationParameters();
  return true;
}

void QgsProcessingModelAlgorithm::addModelParameter( QgsProcessingParameterDefinition *definition, const QgsProcessingModelParameter &component )
{
  if ( addParameter( definition ) )
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

void QgsProcessingModelAlgorithm::changeParameterName( const QString &oldName, const QString &newName )
{
  QgsProcessingContext context;
  QgsExpressionContext expressionContext = createExpressionContext( QVariantMap(), context );

  auto replaceExpressionVariable = [oldName, newName, &expressionContext]( const QString & expressionString ) -> std::tuple< bool, QString >
  {
    QgsExpression expression( expressionString );
    expression.prepare( &expressionContext );
    QSet<QString> variables = expression.referencedVariables();
    if ( variables.contains( oldName ) )
    {
      QString newExpression = expressionString;
      newExpression.replace( QStringLiteral( "@%1" ).arg( oldName ), QStringLiteral( "@%2" ).arg( newName ) );
      return { true, newExpression };
    }
    return { false, QString() };
  };

  QMap< QString, QgsProcessingModelChildAlgorithm >::iterator childIt = mChildAlgorithms.begin();
  for ( ; childIt != mChildAlgorithms.end(); ++childIt )
  {
    bool changed = false;
    QMap<QString, QgsProcessingModelChildParameterSources> childParams = childIt->parameterSources();
    QMap<QString, QgsProcessingModelChildParameterSources>::iterator paramIt = childParams.begin();
    for ( ; paramIt != childParams.end(); ++paramIt )
    {
      QList< QgsProcessingModelChildParameterSource > &value = paramIt.value();
      for ( auto valueIt = value.begin(); valueIt != value.end(); ++valueIt )
      {
        switch ( valueIt->source() )
        {
          case Qgis::ProcessingModelChildParameterSource::ModelParameter:
          {
            if ( valueIt->parameterName() == oldName )
            {
              valueIt->setParameterName( newName );
              changed = true;
            }
            break;
          }

          case Qgis::ProcessingModelChildParameterSource::Expression:
          {
            bool updatedExpression = false;
            QString newExpression;
            std::tie( updatedExpression, newExpression ) = replaceExpressionVariable( valueIt->expression() );
            if ( updatedExpression )
            {
              valueIt->setExpression( newExpression );
              changed = true;
            }
            break;
          }

          case Qgis::ProcessingModelChildParameterSource::StaticValue:
          {
            if ( valueIt->staticValue().userType() == qMetaTypeId<QgsProperty>() )
            {
              QgsProperty property = valueIt->staticValue().value< QgsProperty >();
              if ( property.propertyType() == Qgis::PropertyType::Expression )
              {
                bool updatedExpression = false;
                QString newExpression;
                std::tie( updatedExpression, newExpression ) = replaceExpressionVariable( property.expressionString() );
                if ( updatedExpression )
                {
                  property.setExpressionString( newExpression );
                  valueIt->setStaticValue( property );
                  changed = true;
                }
              }
            }
            break;
          }

          case Qgis::ProcessingModelChildParameterSource::ChildOutput:
          case Qgis::ProcessingModelChildParameterSource::ExpressionText:
          case Qgis::ProcessingModelChildParameterSource::ModelOutput:
            break;
        }
      }
    }
    if ( changed )
      childIt->setParameterSources( childParams );
  }
}

bool QgsProcessingModelAlgorithm::childAlgorithmsDependOnParameter( const QString &name ) const
{
  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    // check whether child requires this parameter
    QMap<QString, QgsProcessingModelChildParameterSources> childParams = childIt->parameterSources();
    QMap<QString, QgsProcessingModelChildParameterSources>::const_iterator paramIt = childParams.constBegin();
    for ( ; paramIt != childParams.constEnd(); ++paramIt )
    {
      const auto constValue = paramIt.value();
      for ( const QgsProcessingModelChildParameterSource &source : constValue )
      {
        if ( source.source() == Qgis::ProcessingModelChildParameterSource::ModelParameter
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
  const auto constMParameters = mParameters;
  for ( const QgsProcessingParameterDefinition *def : constMParameters )
  {
    if ( def->name() == name )
      continue;

    if ( def->dependsOnOtherParameters().contains( name ) )
      return true;
  }
  return false;
}

QMap<QString, QgsProcessingModelParameter> QgsProcessingModelAlgorithm::parameterComponents() const
{
  return mParameterComponents;
}

void QgsProcessingModelAlgorithm::dependentChildAlgorithmsRecursive( const QString &childId, QSet<QString> &depends, const QString &branch ) const
{
  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
  for ( ; childIt != mChildAlgorithms.constEnd(); ++childIt )
  {
    if ( depends.contains( childIt->childId() ) )
      continue;

    // does alg have a direct dependency on this child?
    const QList< QgsProcessingModelChildDependency > constDependencies = childIt->dependencies();
    bool hasDependency = false;
    for ( const QgsProcessingModelChildDependency &dep : constDependencies )
    {
      if ( dep.childId == childId && ( branch.isEmpty() || dep.conditionalBranch == branch ) )
      {
        hasDependency = true;
        break;
      }
    }

    if ( hasDependency )
    {
      depends.insert( childIt->childId() );
      dependentChildAlgorithmsRecursive( childIt->childId(), depends, branch );
      continue;
    }

    // check whether child requires any outputs from the target alg
    QMap<QString, QgsProcessingModelChildParameterSources> childParams = childIt->parameterSources();
    QMap<QString, QgsProcessingModelChildParameterSources>::const_iterator paramIt = childParams.constBegin();
    for ( ; paramIt != childParams.constEnd(); ++paramIt )
    {
      const auto constValue = paramIt.value();
      for ( const QgsProcessingModelChildParameterSource &source : constValue )
      {
        if ( source.source() == Qgis::ProcessingModelChildParameterSource::ChildOutput
             && source.outputChildId() == childId )
        {
          depends.insert( childIt->childId() );
          dependentChildAlgorithmsRecursive( childIt->childId(), depends, branch );
          break;
        }
      }
    }
  }
}

QSet<QString> QgsProcessingModelAlgorithm::dependentChildAlgorithms( const QString &childId, const QString &conditionalBranch ) const
{
  QSet< QString > algs;

  // temporarily insert the target child algorithm to avoid
  // unnecessarily recursion though it
  algs.insert( childId );

  dependentChildAlgorithmsRecursive( childId, algs, conditionalBranch );

  // remove temporary target alg
  algs.remove( childId );

  return algs;
}


void QgsProcessingModelAlgorithm::dependsOnChildAlgorithmsRecursive( const QString &childId, QSet< QString > &depends ) const
{
  const QgsProcessingModelChildAlgorithm &alg = mChildAlgorithms.value( childId );

  // add direct dependencies
  const QList< QgsProcessingModelChildDependency > constDependencies = alg.dependencies();
  for ( const QgsProcessingModelChildDependency &val : constDependencies )
  {
    if ( !depends.contains( val.childId ) )
    {
      depends.insert( val.childId );
      dependsOnChildAlgorithmsRecursive( val.childId, depends );
    }
  }

  // check through parameter dependencies
  QMap<QString, QgsProcessingModelChildParameterSources> childParams = alg.parameterSources();
  QMap<QString, QgsProcessingModelChildParameterSources>::const_iterator paramIt = childParams.constBegin();
  for ( ; paramIt != childParams.constEnd(); ++paramIt )
  {
    const auto constValue = paramIt.value();
    for ( const QgsProcessingModelChildParameterSource &source : constValue )
    {
      switch ( source.source() )
      {
        case Qgis::ProcessingModelChildParameterSource::ChildOutput:
          if ( !depends.contains( source.outputChildId() ) )
          {
            depends.insert( source.outputChildId() );
            dependsOnChildAlgorithmsRecursive( source.outputChildId(), depends );
          }
          break;

        case Qgis::ProcessingModelChildParameterSource::Expression:
        {
          const QgsExpression exp( source.expression() );
          const QSet<QString> vars = exp.referencedVariables();
          if ( vars.empty() )
            break;

          // find the source of referenced variables and check if it's another child algorithm
          const QMap<QString, QgsProcessingModelAlgorithm::VariableDefinition> availableVariables = variablesForChildAlgorithm( childId );
          for ( auto childVarIt = availableVariables.constBegin(); childVarIt != availableVariables.constEnd(); ++childVarIt )
          {
            // we're only looking here for variables coming from other child algorithm outputs
            if ( childVarIt->source.source() != Qgis::ProcessingModelChildParameterSource::ChildOutput )
              continue;

            if ( !vars.contains( childVarIt.key() ) || depends.contains( childVarIt->source.outputChildId() ) )
              continue;

            // this variable is required for the child's expression, so the corresponding algorithm must be run first
            depends.insert( childVarIt->source.outputChildId() );
            dependsOnChildAlgorithmsRecursive( childVarIt->source.outputChildId(), depends );
          }
          break;
        }

        case Qgis::ProcessingModelChildParameterSource::ModelParameter:
        case Qgis::ProcessingModelChildParameterSource::StaticValue:
        case Qgis::ProcessingModelChildParameterSource::ExpressionText:
        case Qgis::ProcessingModelChildParameterSource::ModelOutput:
          break;
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

QList<QgsProcessingModelChildDependency> QgsProcessingModelAlgorithm::availableDependenciesForChildAlgorithm( const QString &childId ) const
{
  QSet< QString > dependent;
  if ( !childId.isEmpty() )
  {
    dependent.unite( dependentChildAlgorithms( childId ) );
    dependent.insert( childId );
  }

  QList<QgsProcessingModelChildDependency> res;
  for ( auto it = mChildAlgorithms.constBegin(); it != mChildAlgorithms.constEnd(); ++it )
  {
    if ( !dependent.contains( it->childId() ) )
    {
      // check first if algorithm provides output branches
      bool hasBranches = false;
      if ( it->algorithm() )
      {
        const QgsProcessingOutputDefinitions defs = it->algorithm()->outputDefinitions();
        for ( const QgsProcessingOutputDefinition *def : defs )
        {
          if ( def->type() == QgsProcessingOutputConditionalBranch::typeName() )
          {
            hasBranches = true;
            QgsProcessingModelChildDependency alg;
            alg.childId = it->childId();
            alg.conditionalBranch = def->name();
            res << alg;
          }
        }
      }

      if ( !hasBranches )
      {
        QgsProcessingModelChildDependency alg;
        alg.childId = it->childId();
        res << alg;
      }
    }
  }
  return res;
}

bool QgsProcessingModelAlgorithm::validateChildAlgorithm( const QString &childId, QStringList &issues ) const
{
  issues.clear();
  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constFind( childId );
  if ( childIt != mChildAlgorithms.constEnd() )
  {
    if ( !childIt->algorithm() )
    {
      issues << QObject::tr( "Algorithm is not available: <i>%1</i>" ).arg( childIt->algorithmId() );
      return false;
    }
    bool res = true;

    // loop through child algorithm parameters and check that they are all valid
    const QgsProcessingParameterDefinitions defs = childIt->algorithm()->parameterDefinitions();
    for ( const QgsProcessingParameterDefinition *def : defs )
    {
      if ( childIt->parameterSources().contains( def->name() ) )
      {
        // is the value acceptable?
        const QList< QgsProcessingModelChildParameterSource > sources = childIt->parameterSources().value( def->name() );
        for ( const QgsProcessingModelChildParameterSource &source : sources )
        {
          switch ( source.source() )
          {
            case Qgis::ProcessingModelChildParameterSource::StaticValue:
              if ( !def->checkValueIsAcceptable( source.staticValue() ) )
              {
                res = false;
                issues <<  QObject::tr( "Value for <i>%1</i> is not acceptable for this parameter" ).arg( def->name() );
              }
              break;

            case Qgis::ProcessingModelChildParameterSource::ModelParameter:
              if ( !parameterComponents().contains( source.parameterName() ) )
              {
                res = false;
                issues <<  QObject::tr( "Model input <i>%1</i> used for parameter <i>%2</i> does not exist" ).arg( source.parameterName(), def->name() );
              }
              break;

            case Qgis::ProcessingModelChildParameterSource::ChildOutput:
              if ( !childAlgorithms().contains( source.outputChildId() ) )
              {
                res = false;
                issues <<  QObject::tr( "Child algorithm <i>%1</i> used for parameter <i>%2</i> does not exist" ).arg( source.outputChildId(), def->name() );
              }
              break;

            case Qgis::ProcessingModelChildParameterSource::Expression:
            case Qgis::ProcessingModelChildParameterSource::ExpressionText:
            case Qgis::ProcessingModelChildParameterSource::ModelOutput:
              break;
          }
        }
      }
      else
      {
        // not specified. Is it optional?

        // ignore destination parameters -- they shouldn't ever be mandatory
        if ( def->isDestination() )
          continue;

        if ( !def->checkValueIsAcceptable( QVariant() ) )
        {
          res = false;
          issues <<  QObject::tr( "Parameter <i>%1</i> is mandatory" ).arg( def->name() );
        }
      }
    }

    return res;
  }
  else
  {
    issues << QObject::tr( "Invalid child ID: <i>%1</i>" ).arg( childId );
    return false;
  }
}

bool QgsProcessingModelAlgorithm::canExecute( QString *errorMessage ) const
{
  reattachAlgorithms();
  QMap< QString, QgsProcessingModelChildAlgorithm >::const_iterator childIt = mChildAlgorithms.constBegin();
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

QgsExpressionContext QgsProcessingModelAlgorithm::createExpressionContext( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeatureSource *source ) const
{
  QgsExpressionContext res = QgsProcessingAlgorithm::createExpressionContext( parameters, context, source );
  res << QgsExpressionContextUtils::processingModelAlgorithmScope( this, parameters, context );
  return res;
}

QgsProcessingAlgorithm *QgsProcessingModelAlgorithm::createInstance() const
{
  QgsProcessingModelAlgorithm *alg = new QgsProcessingModelAlgorithm();
  alg->loadVariant( toVariant() );
  alg->setProvider( provider() );
  alg->setSourceFilePath( sourceFilePath() );
  return alg;
}

QString QgsProcessingModelAlgorithm::safeName( const QString &name, bool capitalize )
{
  QString n = name.toLower().trimmed();
  const thread_local QRegularExpression rx( QStringLiteral( "[^\\sa-z_A-Z0-9]" ) );
  n.replace( rx, QString() );
  const thread_local QRegularExpression rx2( QStringLiteral( "^\\d*" ) ); // name can't start in a digit
  n.replace( rx2, QString() );
  if ( !capitalize )
    n = n.replace( ' ', '_' );
  return capitalize ? QgsStringUtils::capitalize( n, Qgis::Capitalization::UpperCamelCase ) : n;
}

QVariantMap QgsProcessingModelAlgorithm::variables() const
{
  return mVariables;
}

void QgsProcessingModelAlgorithm::setVariables( const QVariantMap &variables )
{
  mVariables = variables;
}

QVariantMap QgsProcessingModelAlgorithm::designerParameterValues() const
{
  return mDesignerParameterValues;
}

///@endcond
