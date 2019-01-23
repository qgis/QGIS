/***************************************************************************
                         qgsprocessingmodelerparameterwidget.cpp
                         ---------------------------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
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


#include "qgsprocessingmodelerparameterwidget.h"
#include "qgsprocessingparameters.h"
#include "qgsexpressionlineedit.h"
#include "qgsprocessingguiregistry.h"
#include "models/qgsprocessingmodelalgorithm.h"
#include "qgsgui.h"
#include "qgsguiutils.h"
#include "qgsexpressioncontext.h"
#include "qgsapplication.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QStackedWidget>
#include <QMenu>
#include <QLabel>
#include <QComboBox>

QgsProcessingModelerParameterWidget::QgsProcessingModelerParameterWidget( QgsProcessingModelAlgorithm *model,
    const QString &childId,
    const QgsProcessingParameterDefinition *parameter, QgsProcessingContext &context,
    QWidget *parent )
  : QWidget( parent )
  , mModel( model )
  , mChildId( childId )
  , mParameterDefinition( parameter )
  , mContext( context )
{
  setFocusPolicy( Qt::StrongFocus );

  // icon size is a bit bigger than text, but minimum size of 24 so that we get pixel-aligned rendering on low-dpi screens
  int iconSize = QgsGuiUtils::scaleIconSize( 24 );

  QHBoxLayout *hLayout = new QHBoxLayout();

  mSourceButton = new QToolButton();
  mSourceButton->setFocusPolicy( Qt::StrongFocus );

  // button width is 1.25 * icon size, height 1.1 * icon size. But we round to ensure even pixel sizes for equal margins
  mSourceButton->setFixedSize( 2 * static_cast< int >( 1.25 * iconSize / 2.0 ), 2 * static_cast< int >( iconSize * 1.1 / 2.0 ) );
  mSourceButton->setIconSize( QSize( iconSize, iconSize ) );
  mSourceButton->setPopupMode( QToolButton::InstantPopup );

  mSourceMenu = new QMenu( this );
  connect( mSourceMenu, &QMenu::aboutToShow, this, &QgsProcessingModelerParameterWidget::sourceMenuAboutToShow );
  connect( mSourceMenu, &QMenu::triggered, this, &QgsProcessingModelerParameterWidget::sourceMenuActionTriggered );
  mSourceButton->setMenu( mSourceMenu );

  hLayout->addWidget( mSourceButton );

  mStackedWidget = new QStackedWidget();

  mStaticWidgetWrapper.reset( QgsGui::processingGuiRegistry()->createParameterWidgetWrapper( mParameterDefinition, QgsProcessingGui::Modeler ) );
  if ( mStaticWidgetWrapper )
  {
    QWidget *widget = mStaticWidgetWrapper->createWrappedWidget( context );
    if ( widget )
    {
      mHasStaticWrapper = true;
      mStackedWidget->addWidget( widget );
    }
    else
      mStackedWidget->addWidget( new QWidget() );
  }
  else
  {
    mStackedWidget->addWidget( new QWidget() );
  }

  mExpressionWidget = new QgsExpressionLineEdit();
  mExpressionWidget->registerExpressionContextGenerator( this );
  mStackedWidget->addWidget( mExpressionWidget );

  mModelInputCombo = new QComboBox();
  QHBoxLayout *hLayout2 = new QHBoxLayout();
  hLayout2->setMargin( 0 );
  hLayout2->setContentsMargins( 0, 0, 0, 0 );
  hLayout2->addWidget( new QLabel( tr( "Using model input" ) ) );
  hLayout2->addWidget( mModelInputCombo, 1 );
  QWidget *hWidget2 = new QWidget();
  hWidget2->setLayout( hLayout2 );
  mStackedWidget->addWidget( hWidget2 );

  mChildOutputCombo = new QComboBox();
  QHBoxLayout *hLayout3 = new QHBoxLayout();
  hLayout3->setMargin( 0 );
  hLayout3->setContentsMargins( 0, 0, 0, 0 );
  hLayout3->addWidget( new QLabel( tr( "Using algorithm output" ) ) );
  hLayout3->addWidget( mChildOutputCombo, 1 );
  QWidget *hWidget3 = new QWidget();
  hWidget3->setLayout( hLayout3 );
  mStackedWidget->addWidget( hWidget3 );

  hLayout->setMargin( 0 );
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->addWidget( mStackedWidget, 1 );

  setLayout( hLayout );
  setSourceType( QgsProcessingModelChildParameterSource::StaticValue );
}

QgsProcessingModelerParameterWidget::~QgsProcessingModelerParameterWidget() = default;

void QgsProcessingModelerParameterWidget::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  if ( mStaticWidgetWrapper )
    mStaticWidgetWrapper->setWidgetContext( context );
}

void QgsProcessingModelerParameterWidget::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  if ( mStaticWidgetWrapper )
    mStaticWidgetWrapper->registerProcessingContextGenerator( generator );
}

const QgsProcessingParameterDefinition *QgsProcessingModelerParameterWidget::parameterDefinition() const
{
  return mParameterDefinition;
}

QLabel *QgsProcessingModelerParameterWidget::createLabel()
{
  if ( mStaticWidgetWrapper )
    return mStaticWidgetWrapper->createWrappedLabel();
  else
    return nullptr;
}

void QgsProcessingModelerParameterWidget::setWidgetValue( const QgsProcessingModelChildParameterSource &value )
{
  // we make a copy of all attributes and store locally, so that users can flick between
  // sources without losing their current value
  mStaticValue = value.staticValue();
  mModelInputParameterName = value.parameterName();
  mOutputChildId = value.outputChildId();
  mOutputName = value.outputName();
  mExpression = value.expression();

  updateUi();
  setSourceType( value.source() );
}

QgsProcessingModelChildParameterSource QgsProcessingModelerParameterWidget::value() const
{
  switch ( currentSourceType() )
  {
    case StaticValue:
      return QgsProcessingModelChildParameterSource::fromStaticValue( mStaticWidgetWrapper->parameterValue() );

    case Expression:
      return QgsProcessingModelChildParameterSource::fromExpression( mExpressionWidget->expression() );

    case ModelParameter:
      return QgsProcessingModelChildParameterSource::fromModelParameter( mModelInputCombo->currentData().toString() );

    case ChildOutput:
    {
      const QStringList parts = mChildOutputCombo->currentData().toStringList();
      return QgsProcessingModelChildParameterSource::fromChildOutput( parts.value( 0, QString() ), parts.value( 1, QString() ) );
    }
  }

  return QgsProcessingModelChildParameterSource();
}

QgsExpressionContext QgsProcessingModelerParameterWidget::createExpressionContext() const
{
  QgsExpressionContext c = mContext.expressionContext();
  if ( mModel )
  {
    const QgsProcessingAlgorithm *alg = nullptr;
    if ( mModel->childAlgorithms().contains( mChildId ) )
      alg = mModel->childAlgorithm( mChildId ).algorithm();
    QgsExpressionContextScope *algorithmScope = QgsExpressionContextUtils::processingAlgorithmScope( alg, QVariantMap(), mContext );
    c << algorithmScope;
    QgsExpressionContextScope *childScope = mModel->createExpressionContextScopeForChildAlgorithm( mChildId, mContext, QVariantMap(), QVariantMap() );
    c << childScope;

    QStringList highlightedVariables = childScope->variableNames();
    QStringList highlightedFunctions = childScope->functionNames();
    highlightedVariables += algorithmScope->variableNames();
    highlightedFunctions += algorithmScope->functionNames();
    c.setHighlightedVariables( highlightedVariables );
    c.setHighlightedFunctions( highlightedFunctions );
  }

  return c;
}

void QgsProcessingModelerParameterWidget::sourceMenuAboutToShow()
{
  mSourceMenu->clear();

  const SourceType currentSource = currentSourceType();

  if ( mHasStaticWrapper )
  {
    QAction *fixedValueAction = mSourceMenu->addAction( tr( "Value" ) );
    fixedValueAction->setCheckable( currentSource == StaticValue );
    fixedValueAction->setChecked( currentSource == StaticValue );
    fixedValueAction->setData( QgsProcessingModelChildParameterSource::StaticValue );
  }

  QAction *calculatedValueAction = mSourceMenu->addAction( tr( "Pre-calculated Value" ) );
  calculatedValueAction->setCheckable( currentSource == Expression );
  calculatedValueAction->setChecked( currentSource == Expression );
  calculatedValueAction->setData( QgsProcessingModelChildParameterSource::Expression );

  QAction *inputValueAction = mSourceMenu->addAction( tr( "Model Input" ) );
  inputValueAction->setCheckable( currentSource == ModelParameter );
  inputValueAction->setChecked( currentSource == ModelParameter );
  inputValueAction->setData( QgsProcessingModelChildParameterSource::ModelParameter );

  QAction *childOutputValueAction = mSourceMenu->addAction( tr( "Algorithm Output" ) );
  childOutputValueAction->setCheckable( currentSource == ChildOutput );
  childOutputValueAction->setChecked( currentSource == ChildOutput );
  childOutputValueAction->setData( QgsProcessingModelChildParameterSource::ChildOutput );

  // TODO - expression text item?
}

void QgsProcessingModelerParameterWidget::sourceMenuActionTriggered( QAction *action )
{
  QgsProcessingModelChildParameterSource::Source sourceType = static_cast< QgsProcessingModelChildParameterSource::Source  >( action->data().toInt() );
  setSourceType( sourceType );
}

QgsProcessingModelerParameterWidget::SourceType QgsProcessingModelerParameterWidget::currentSourceType() const
{
  return static_cast< SourceType >( mStackedWidget->currentIndex() );
}

void QgsProcessingModelerParameterWidget::setSourceType( QgsProcessingModelChildParameterSource::Source type )
{
  switch ( type )
  {
    case QgsProcessingModelChildParameterSource::StaticValue:
      mStackedWidget->setCurrentIndex( static_cast< int >( StaticValue ) );
      mSourceButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconFieldInteger.svg" ) ) );
      mSourceButton->setToolTip( tr( "Value" ) );
      break;

    case QgsProcessingModelChildParameterSource::Expression:
      mSourceButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconExpression.svg" ) ) );
      mStackedWidget->setCurrentIndex( static_cast< int >( Expression ) );
      mSourceButton->setToolTip( tr( "Pre-calculated Value" ) );
      break;

    case QgsProcessingModelChildParameterSource::ModelParameter:
    {
      mSourceButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "processingModel.svg" ) ) );
      mStackedWidget->setCurrentIndex( static_cast< int >( ModelParameter ) );
      mSourceButton->setToolTip( tr( "Model Input" ) );
      break;
    }

    case QgsProcessingModelChildParameterSource::ChildOutput:
    {
      mSourceButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "processingAlgorithm.svg" ) ) );
      mStackedWidget->setCurrentIndex( static_cast< int >( ChildOutput ) );
      mSourceButton->setToolTip( tr( "Algorithm Output" ) );
      break;
    }

    case QgsProcessingModelChildParameterSource::ExpressionText:
      break;
  }
}

void QgsProcessingModelerParameterWidget::updateUi()
{
  mStaticWidgetWrapper->setParameterValue( mStaticValue, mContext );

  mExpressionWidget->setExpression( mExpression );

  int currentIndex = mModelInputCombo->findData( mModelInputParameterName );
  if ( currentIndex == -1 && mModelInputCombo->count() > 0 )
    currentIndex = 0;
  mModelInputCombo->setCurrentIndex( currentIndex );

  const QStringList parts = QStringList() << mOutputChildId << mOutputName;
  currentIndex = mChildOutputCombo->findData( parts );
  if ( currentIndex == -1 && mChildOutputCombo->count() > 0 )
    currentIndex = 0;
  mChildOutputCombo->setCurrentIndex( currentIndex );
}

void QgsProcessingModelerParameterWidget::populateSources( const QStringList &compatibleParameterTypes, const QStringList &compatibleOutputTypes, const QList<int> &compatibleDataTypes )
{
  const QList< QgsProcessingModelChildParameterSource > sources = mModel->availableSourcesForChild( mChildId,
      compatibleParameterTypes, compatibleOutputTypes, compatibleDataTypes );

  for ( const QgsProcessingModelChildParameterSource &source : sources )
  {
    switch ( source.source() )
    {
      case QgsProcessingModelChildParameterSource::ModelParameter:
        mModelInputCombo->addItem( mModel->parameterDefinition( source.parameterName() )->description(), source.parameterName() );
        break;

      case QgsProcessingModelChildParameterSource::ChildOutput:
      {
        if ( !mModel->childAlgorithms().contains( source.outputChildId() ) )
          continue;

        const QgsProcessingModelChildAlgorithm &alg = mModel->childAlgorithm( source.outputChildId() );
        if ( !alg.algorithm() )
          continue;
        const QString outputDescription = alg.algorithm()->outputDefinition( source.outputName() )->description();
        const QString childDescription = alg.description();

        mChildOutputCombo->addItem( tr( "“%1” from algorithm “%2”" ).arg( outputDescription, childDescription ), QStringList() << source.outputChildId() << source.outputName() );
        break;
      }

      case QgsProcessingModelChildParameterSource::StaticValue:
      case QgsProcessingModelChildParameterSource::Expression:
      case QgsProcessingModelChildParameterSource::ExpressionText:
        break;
    }

  }
}

void QgsProcessingModelerParameterWidget::setExpressionHelpText( const QString &text )
{
  mExpressionWidget->setExpectedOutputFormat( text );
}
