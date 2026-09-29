/***************************************************************************
                             qgsprocessingparameterswidget.cpp
                             ------------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparameterswidget.h"

#include "qgsgui.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingparameters.h"

#include <QLabel>
#include <QString>

#include "moc_qgsprocessingparameterswidget.cpp"

using namespace Qt::StringLiterals;

///@cond NOT_STABLE

QgsProcessingParametersWidget::QgsProcessingParametersWidget( const QgsProcessingAlgorithm *algorithm, bool inPlace, QgsMapLayer *activeLayer, QgsMessageBar *messageBar, QWidget *parent )
  : QgsPanelWidget( parent )
  , mAlgorithm( algorithm )
  , mInPlace( inPlace )
  , mActiveLayer( activeLayer )
  , mMessageBar( messageBar )
{
  Q_ASSERT( mAlgorithm );

  setupUi( this );

  grpAdvanced->hide();
  scrollAreaWidgetContents->setContentsMargins( 4, 4, 4, 4 );
}

QgsProcessingParametersWidget::~QgsProcessingParametersWidget()
{
  qDeleteAll( mWrappers );
}

const QgsProcessingAlgorithm *QgsProcessingParametersWidget::algorithm() const
{
  return mAlgorithm;
}

QgsProcessingContext *QgsProcessingParametersWidget::processingContext() const
{
  return mContextGenerator ? mContextGenerator->processingContext() : nullptr;
}

QVariantMap QgsProcessingParametersWidget::createProcessingParameters( Flags flags )
{
  QList< QgsProcessingParametersGenerator::ParameterValidationResult > validationResults;
  return createAndValidateParameters( flags, validationResults );
}

void QgsProcessingParametersWidget::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  mContextGenerator = generator;
}

QList<QgsProcessingParametersGenerator::ParameterValidationResult> QgsProcessingParametersWidget::validate() const
{
  QList< QgsProcessingParametersGenerator::ParameterValidationResult > validationResults;
  createAndValidateParameters( QgsProcessingParametersGenerator::Flags(), validationResults );
  return validationResults;
}

QVariantMap QgsProcessingParametersWidget::createAndValidateParameters(
  QgsProcessingParametersGenerator::Flags flags, QList< QgsProcessingParametersGenerator::ParameterValidationResult > &validationResults
) const
{
  QgsProcessingContext *context = processingContext();

  const bool includeDefault = !flags.testFlag( Flag::SkipDefaultValueParameters );
  const bool validate = !flags.testFlag( Flag::SkipValidation );

  QVariantMap parameters = mExtraParameters;
  const QgsProcessingParameterDefinitions defs = mAlgorithm->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *definition : defs )
  {
    if ( definition->flags().testFlag( Qgis::ProcessingParameterFlag::Hidden ) )
    {
      continue;
    }

    if ( !definition->isDestination() )
    {
      QgsAbstractProcessingParameterWidgetWrapper *wrapper = mWrappers.value( definition->name() );
      if ( !wrapper )
        continue;

      QWidget *widget = wrapper->wrappedWidget();

      if ( !dynamic_cast< QgsProcessingHiddenWidgetWrapper *>( wrapper ) && !widget )
      {
        continue;
      }

      const QVariant value = wrapper->parameterValue();
      if ( definition->defaultValue() != value || includeDefault )
      {
        parameters[definition->name()] = value;
      }

      if ( validate && !definition->checkValueIsAcceptable( value ) )
      {
        QgsProcessingParametersGenerator::ParameterValidationResult result;
        result.parameterName = definition->name();
        result.result = QgsProcessingParametersGenerator::ValidationResult::InvalidValue;
        validationResults.append( result );
      }
    }
    else
    {
      if ( mInPlace && definition->name() == "OUTPUT"_L1 )
      {
        parameters[definition->name()] = u"memory:"_s;
        continue;
      }

      QgsAbstractProcessingParameterWidgetWrapper *wrapper = mWrappers.value( definition->name() );
      if ( !wrapper )
        continue;

      QVariant value = wrapper->parameterValue();

      QgsProject *destinationProject = nullptr;
      if ( wrapper->customProperties().value( u"OPEN_AFTER_RUNNING"_s ).toBool() )
      {
        destinationProject = QgsProject::instance();
      }

      if ( value.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition>() )
      {
        QgsProcessingOutputLayerDefinition outputDefinition = value.value< QgsProcessingOutputLayerDefinition>();
        outputDefinition.destinationProject = destinationProject;
        value = QVariant::fromValue( outputDefinition );
      }

      if ( !QgsVariantUtils::isNull( value ) && ( definition->defaultValue() != value || includeDefault ) )
      {
        parameters[definition->name()] = value;
        if ( validate )
        {
          auto destinationParam = dynamic_cast< const QgsProcessingDestinationParameter * >( definition );
          if ( destinationParam )
          {
            QString error;
            const bool ok = destinationParam->isSupportedOutputValue( value, *context, error );
            if ( !ok )
            {
              QgsProcessingParametersGenerator::ParameterValidationResult result;
              result.parameterName = definition->name();
              result.result = QgsProcessingParametersGenerator::ValidationResult::InvalidOutputExtension;
              result.message = error;
              validationResults.append( result );
            }
          }
        }
      }
    }
  }

  return const_cast< QgsProcessingAlgorithm * >( mAlgorithm )->preprocessParameters( parameters );
}

void QgsProcessingParametersWidget::setParameters( const QVariantMap &parameters )
{
  QgsProcessingContext *context = processingContext();

  mExtraParameters.clear();
  const QgsProcessingParameterDefinitions defs = mAlgorithm->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *definition : std::as_const( defs ) )
  {
    auto paramIt = parameters.constFind( definition->name() );
    if ( definition->flags().testFlag( Qgis::ProcessingParameterFlag::Hidden ) )
    {
      if ( paramIt != parameters.constEnd() )
      {
        mExtraParameters.insert( definition->name(), paramIt.value() );
      }
      continue;
    }

    if ( paramIt == parameters.constEnd() )
    {
      continue;
    }

    const QVariant value = paramIt.value();

    auto wrapperIt = mWrappers.constFind( definition->name() );
    if ( wrapperIt != mWrappers.constEnd() )
    {
      wrapperIt.value()->setParameterValue( value, *context );
    }
  }
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingParametersWidget::wrapper( const QString &name ) const
{
  return mWrappers.value( name );
}

QList<QgsAbstractProcessingParameterWidgetWrapper *> QgsProcessingParametersWidget::wrappers() const
{
  return mWrappers.values();
}

void QgsProcessingParametersWidget::parameterChanged()
{
  auto wrapper = qobject_cast< QgsAbstractProcessingParameterWidgetWrapper * >( sender() );
  if ( !wrapper )
    return;

  const QVariantMap defaultValues
    = mAlgorithm->autogenerateParameterValues( createProcessingParameters( QgsProcessingParametersGenerator::Flag::SkipValidation ), wrapper->parameterDefinition()->name(), Qgis::ProcessingMode::Standard );

  if ( !defaultValues.empty() )
  {
    setParameters( defaultValues );
  }
}

void QgsProcessingParametersWidget::initWidgets()
{
  QgsProcessingContext *context = processingContext();

  // if there are advanced parameters - show corresponding groupbox
  const QgsProcessingParameterDefinitions defs = mAlgorithm->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *param : defs )
  {
    if ( param->flags() & Qgis::ProcessingParameterFlag::Advanced )
    {
      grpAdvanced->show();
      break;
    }
  }

  if ( !algorithm() )
  {
    return;
  }

  QgsProcessingParameterWidgetContext widgetContext = QgsGui::processingGuiRegistry()->createWidgetContext();
  widgetContext.setMessageBar( mMessageBar );

  if ( auto modelAlgorithm = dynamic_cast< const QgsProcessingModelAlgorithm * >( mAlgorithm ) )
  {
    widgetContext.setModel( const_cast< QgsProcessingModelAlgorithm * >( modelAlgorithm ) );
  }

  QString inPlaceInputParameterName = u"INPUT"_s;
  if ( auto featureAlgorithm = dynamic_cast< const QgsProcessingFeatureBasedAlgorithm * >( mAlgorithm ) )
  {
    inPlaceInputParameterName = featureAlgorithm->inputParameterName();
  }

  bool hasNonInputParams = false;
  for ( const QgsProcessingParameterDefinition *definition : std::as_const( defs ) )
  {
    if ( definition->name() != inPlaceInputParameterName && definition->name() != "OUTPUT"_L1 )
    {
      hasNonInputParams = true;
      break;
    }
  }
  // If there are no parameters to show because it's in-place, we add the info label.
  // Still, it needs the following steps to create the parameter widgets (even when hidden).
  if ( mInPlace && !hasNonInputParams )
  {
    auto widget = new QWidget( this );
    auto layout = new QVBoxLayout( widget );
    auto label = new QLabel( widget );
    label->setWordWrap( true );

    const QString layerName = mActiveLayer ? mActiveLayer->name() : QString();
    const QString infoText = tr( "<i>No additional parameters are required. This algorithm will activate edit mode and modify the features on layer <b>%1</b> in place.</i>" ).arg( layerName );
    label->setText( infoText );

    layout->addWidget( label );
    layout->addStretch();
    addExtraWidget( widget );
  }

  // Create widgets and put them in layouts
  for ( const QgsProcessingParameterDefinition *definition : defs )
  {
    if ( definition->flags().testFlag( Qgis::ProcessingParameterFlag::Hidden ) )
    {
      continue;
    }

    if ( definition->isDestination() )
    {
      continue;
    }

    if ( mInPlace && ( definition->name() == inPlaceInputParameterName || definition->name() == "OUTPUT"_L1 ) )
    {
      // don't show the input/output parameter widgets in in-place mode
      // we still need to CREATE them, because other wrappers may need to interact
      // with them (e.g. those parameters which need the input layer for field
      // selections/crs properties/etc)
      auto wrapper = new QgsProcessingHiddenWidgetWrapper( definition, Qgis::ProcessingMode::Standard, this );
      wrapper->setLinkedVectorLayer( qobject_cast< QgsVectorLayer * >( mActiveLayer.get() ) );
      mWrappers.insert( definition->name(), wrapper );
      continue;
    }

    QgsAbstractProcessingParameterWidgetWrapper *wrapper = QgsGui::processingGuiRegistry()->createParameterWidgetWrapper( definition, Qgis::ProcessingMode::Standard );
    if ( !wrapper )
    {
      continue;
    }

    wrapper->setDialog( parentWidget() );
    wrapper->setWidgetContext( widgetContext );
    wrapper->registerProcessingContextGenerator( this );
    wrapper->registerProcessingParametersGenerator( this );
    mWrappers.insert( definition->name(), wrapper );

    QWidget *widget = wrapper->createWrappedWidget( *context );
    connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, &QgsProcessingParametersWidget::parameterChanged );

    if ( widget )
    {
      if ( QLabel *label = wrapper->createWrappedLabel() )
      {
        addParameterLabel( definition, label );
      }
      addParameterWidget( definition, widget, wrapper->stretch() );
    }
  }

  const QgsProcessingParameterDefinitions destinationDefinitions = algorithm()->destinationParameterDefinitions();
  for ( const QgsProcessingParameterDefinition *output : destinationDefinitions )
  {
    if ( output->flags().testFlag( Qgis::ProcessingParameterFlag::Hidden ) )
    {
      continue;
    }

    if ( mInPlace && ( output->name() == inPlaceInputParameterName || output->name() == "OUTPUT"_L1 ) )
    {
      continue;
    }

    QgsAbstractProcessingParameterWidgetWrapper *wrapper = QgsGui::processingGuiRegistry()->createParameterWidgetWrapper( output, Qgis::ProcessingMode::Standard );
    if ( !wrapper )
    {
      continue;
    }

    wrapper->setWidgetContext( widgetContext );
    wrapper->registerProcessingContextGenerator( this );
    wrapper->registerProcessingParametersGenerator( this );
    mWrappers.insert( output->name(), wrapper );

    if ( QLabel *label = wrapper->createWrappedLabel() )
    {
      addOutputLabel( label );
    }

    QWidget *widget = wrapper->createWrappedWidget( *context );
    addOutputWidget( widget, wrapper->stretch() );
  }

  const QList< QgsAbstractProcessingParameterWidgetWrapper * > wrapperList = mWrappers.values();
  for ( auto it = mWrappers.constBegin(); it != mWrappers.constEnd(); ++it )
  {
    it.value()->postInitialize( wrapperList );
  }
}

void QgsProcessingParametersWidget::addParameterWidget( const QgsProcessingParameterDefinition *parameter, QWidget *widget, int stretch )
{
  if ( parameter->flags() & Qgis::ProcessingParameterFlag::Advanced )
    mAdvancedGroupLayout->addWidget( widget, stretch );
  else
    mScrollAreaLayout->insertWidget( mScrollAreaLayout->count() - 2, widget, stretch );
}

void QgsProcessingParametersWidget::addParameterLabel( const QgsProcessingParameterDefinition *parameter, QWidget *label )
{
  if ( parameter->flags() & Qgis::ProcessingParameterFlag::Advanced )
    mAdvancedGroupLayout->addWidget( label );
  else
    mScrollAreaLayout->insertWidget( mScrollAreaLayout->count() - 2, label );
}

void QgsProcessingParametersWidget::addOutputLabel( QWidget *label )
{
  mScrollAreaLayout->insertWidget( mScrollAreaLayout->count() - 1, label );
}

void QgsProcessingParametersWidget::addOutputWidget( QWidget *widget, int stretch )
{
  mScrollAreaLayout->insertWidget( mScrollAreaLayout->count() - 1, widget, stretch );
}

void QgsProcessingParametersWidget::addExtraWidget( QWidget *widget )
{
  mScrollAreaLayout->addWidget( widget );
}

///@endcond
