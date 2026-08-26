/***************************************************************************
                         qgsmodelchildalgorithmwidgets.cpp
                         ------------------------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
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


#include "qgsmodelchildalgorithmwidgets.h"

#include "qgscollapsiblegroupbox.h"
#include "qgscolorbutton.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsmessagebar.h"
#include "qgsmodeldesignerdialog.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingmodelerparameterwidget.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgsscrollarea.h"

#include <QDesktopServices>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QVBoxLayout>

#include "moc_qgsmodelchildalgorithmwidgets.cpp"

using namespace Qt::StringLiterals;

//
// QgsProcessingModelerParametersPanelWidget
//

QgsProcessingModelerParametersPanelWidget::QgsProcessingModelerParametersPanelWidget(
  const QgsProcessingAlgorithm *childAlgorithm, QgsProcessingModelAlgorithm *model, QgsProcessingContext &context, const QString &childId, const QVariantMap &configuration, QWidget *parent, QWidget *dialog
)
  : QgsPanelWidget( parent )
  , mAlgorithm( childAlgorithm ? childAlgorithm->create() : nullptr )
  , mModel( model )
  , mChildId( childId )
  , mConfiguration( configuration )
  , mContext( context )
  , mDialog( dialog )
{
  setupUi();

  setStateFromChildAlgorithm();
}

QgsProcessingModelerParametersPanelWidget::~QgsProcessingModelerParametersPanelWidget() = default;

const QgsProcessingAlgorithm *QgsProcessingModelerParametersPanelWidget::algorithm() const
{
  return mAlgorithm.get();
}

void QgsProcessingModelerParametersPanelWidget::setupUi()
{
  auto mainLayout = new QVBoxLayout();
  mainLayout->setContentsMargins( 0, 0, 0, 0 );

  auto verticalLayout = new QVBoxLayout();

  mMessageBar = new QgsMessageBar();
  mMessageBar->setSizePolicy( QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed );
  verticalLayout->addWidget( mMessageBar );

  auto hLayout = new QHBoxLayout();
  hLayout->setContentsMargins( 0, 0, 0, 0 );
  hLayout->addWidget( new QLabel( tr( "Description" ) ) );
  mDescriptionBox = new QLineEdit();
  mDescriptionBox->setObjectName( u"mDescriptionBox"_s );
  if ( mAlgorithm )
  {
    mDescriptionBox->setText( mAlgorithm->displayName() );
  }
  hLayout->addWidget( mDescriptionBox );
  connect( mDescriptionBox, &QLineEdit::textChanged, this, &QgsProcessingModelerParametersPanelWidget::emitChangedSignal );

  verticalLayout->addLayout( hLayout );

  auto line = new QFrame();
  line->setFrameShape( QFrame::Shape::HLine );
  line->setFrameShadow( QFrame::Shadow::Sunken );
  verticalLayout->addWidget( line );

  if ( mAlgorithm )
  {
    mAlgorithmItem = QgsGui::processingGuiRegistry()->algorithmConfigurationWidget( mAlgorithm.get() );
    if ( mAlgorithmItem )
    {
      mAlgorithmItem->registerProcessingContextGenerator( this );
      if ( !mConfiguration.isEmpty() )
        mAlgorithmItem->setConfiguration( mConfiguration );

      verticalLayout->addWidget( mAlgorithmItem );
    }
  }

  if ( mAlgorithm )
  {
    const QList<const QgsProcessingParameterDefinition *> parameters = mAlgorithm->parameterDefinitions();
    // only create advanced group when there are SOME advanced parameters:
    bool hasAdvancedParameters = false;
    for ( const QgsProcessingParameterDefinition *parameter : parameters )
    {
      if ( parameter->flags() & Qgis::ProcessingParameterFlag::Advanced )
      {
        hasAdvancedParameters = true;
        break;
      }
    }
    QgsCollapsibleGroupBox *advancedGroup = nullptr;
    QVBoxLayout *advancedGroupLayout = nullptr;
    if ( hasAdvancedParameters )
    {
      advancedGroup = new QgsCollapsibleGroupBox( tr( "Advanced Parameters" ) );
      advancedGroupLayout = new QVBoxLayout();
      advancedGroup->setLayout( advancedGroupLayout );
      verticalLayout->addWidget( advancedGroup );
    }

    for ( const QgsProcessingParameterDefinition *parameter : parameters )
    {
      if ( parameter->isDestination() || ( parameter->flags() & Qgis::ProcessingParameterFlag::Hidden ) )
        continue;

      // note: ownership of widget is transferred to parent layout below
      QgsProcessingModelerParameterWidget *widget = QgsGui::processingGuiRegistry()->createModelerParameterWidget( mModel, mChildId, parameter, mContext );
      if ( !widget )
        continue;

      widget->setDialog( mDialog );
      widget->registerProcessingContextGenerator( this );
      connect( widget, &QgsProcessingModelerParameterWidget::changed, this, &QgsProcessingModelerParametersPanelWidget::emitChangedSignal );
      mWrappers.insert( parameter->name(), widget );

      QLabel *label = widget->createLabel();
      // advanced parameters get inserted to a different layout:
      if ( advancedGroupLayout && parameter->flags() & Qgis::ProcessingParameterFlag::Advanced )
      {
        if ( label )
          advancedGroupLayout->addWidget( label );
        advancedGroupLayout->addWidget( widget );
      }
      else
      {
        if ( label )
          verticalLayout->insertWidget( verticalLayout->count() - 1, label );
        verticalLayout->insertWidget( verticalLayout->count() - 1, widget );
      }
    }

    const QList< const QgsProcessingParameterDefinition * > destinationsParameters = mAlgorithm->destinationParameterDefinitions();
    for ( const QgsProcessingParameterDefinition *output : destinationsParameters )
    {
      if ( output->flags() & Qgis::ProcessingParameterFlag::Hidden )
        continue;

      // note: ownership of widget is transferred to parent layout below
      QgsProcessingModelerParameterWidget *widget = QgsGui::processingGuiRegistry()->createModelerParameterWidget( mModel, mChildId, output, mContext );
      if ( !widget )
        continue;

      widget->setDialog( mDialog );
      widget->registerProcessingContextGenerator( this );
      connect( widget, &QgsProcessingModelerParameterWidget::changed, this, &QgsProcessingModelerParametersPanelWidget::emitChangedSignal );

      mWrappers.insert( output->name(), widget );

      if ( QLabel *label = widget->createLabel() )
        verticalLayout->addWidget( label );

      verticalLayout->addWidget( widget );
    }
  }

  const QFontMetrics fm( font() );
  auto spacer = new QSpacerItem( 20, fm.height() );
  verticalLayout->addItem( spacer );

  auto dependenciesLabel = new QLabel( tr( "Dependencies" ) );
  mDependenciesPanel = new QgsModelChildDependenciesWidget( this, mModel, mChildId );
  verticalLayout->addWidget( dependenciesLabel );
  verticalLayout->addWidget( mDependenciesPanel );
  connect( mDependenciesPanel, &QgsModelChildDependenciesWidget::changed, this, &QgsProcessingModelerParametersPanelWidget::emitChangedSignal );
  verticalLayout->addStretch( 1 );

  auto scrollAreaContainer = new QVBoxLayout();
  scrollAreaContainer->setSpacing( 2 );
  scrollAreaContainer->setContentsMargins( 0, 0, 0, 0 );

  auto paramPanel = new QWidget();
  paramPanel->setLayout( verticalLayout );

  auto scrollArea = new QgsScrollArea();
  scrollArea->setWidget( paramPanel );
  scrollArea->setWidgetResizable( true );
  scrollArea->setFrameStyle( QFrame::Shape::NoFrame );

  scrollAreaContainer->addWidget( scrollArea );

  auto scrollAreaContainerWidget = new QWidget();
  scrollAreaContainerWidget->setLayout( scrollAreaContainer );
  mainLayout->addWidget( scrollAreaContainerWidget );
  setLayout( mainLayout );
}

void QgsProcessingModelerParametersPanelWidget::emitChangedSignal()
{
  if ( !mBlockChangesSignal )
    emit widgetChanged();
}

void QgsProcessingModelerParametersPanelWidget::setStateFromChildAlgorithm()
{
  if ( mChildId.isEmpty() || !mModel->childAlgorithms().contains( mChildId ) )
    return;

  const QgsProcessingModelChildAlgorithm childAlgorithm = mModel->childAlgorithm( mChildId );
  const QgsProcessingAlgorithm *sourceAlgorithm = mAlgorithm.get();
  std::unique_ptr< QgsProcessingAlgorithm > tempAlgorithm;
  if ( mAlgorithmItem )
  {
    // for algorithms with a custom config widget, we need to iterate over parameters defined
    // when that algorithm is created respecting the custom config widget.

    // WARNING: we CANNOT overwrite mAlgorithm here, as all the existing wrappers have already
    // been created with references to that algorithm instance!
    tempAlgorithm.reset( childAlgorithm.algorithm()->create( mAlgorithmItem->configuration() ) );
    sourceAlgorithm = tempAlgorithm.get();
  }

  if ( !sourceAlgorithm )
    return;

  const QList< const QgsProcessingParameterDefinition * > parameters = sourceAlgorithm->parameterDefinitions();
  const QList< const QgsProcessingParameterDefinition * > destinationParameters = sourceAlgorithm->destinationParameterDefinitions();

  mBlockChangesSignal++;

  mDescriptionBox->setText( childAlgorithm.description() );

  for ( const QgsProcessingParameterDefinition *param : parameters )
  {
    if ( param->isDestination() || ( param->flags() & Qgis::ProcessingParameterFlag::Hidden ) )
      continue;

    QList< QgsProcessingModelChildParameterSource > valueList;
    std::optional< QgsProcessingModelChildParameterSource > singleValue;
    if ( childAlgorithm.parameterSources().contains( param->name() ) )
    {
      valueList = childAlgorithm.parameterSources().value( param->name() );
      if ( valueList.size() == 1 )
      {
        singleValue = valueList.at( 0 );
        valueList.clear();
      }
    }

    if ( valueList.isEmpty() && !singleValue.has_value() )
    {
      singleValue = QgsProcessingModelChildParameterSource::fromStaticValue( param->defaultValue() );
    }

    if ( mWrappers.contains( param->name() ) )
    {
      if ( !valueList.isEmpty() )
      {
        mWrappers.value( param->name() )->setWidgetValue( valueList );
      }
      else if ( singleValue.has_value() )
      {
        mWrappers.value( param->name() )->setWidgetValue( singleValue.value() );
      }
    }
  }

  for ( const QgsProcessingParameterDefinition *output : destinationParameters )
  {
    if ( output->flags() & Qgis::ProcessingParameterFlag::Hidden )
      continue;

    QString modelOutputName;
    const QMap< QString, QgsProcessingModelOutput > outputs = childAlgorithm.modelOutputs();
    for ( auto it = outputs.constBegin(); it != outputs.constEnd(); ++it )
    {
      if ( it.value().childId() == mChildId && it.value().childOutputName() == output->name() )
      {
        // this destination parameter is linked to a model output
        modelOutputName = it.value().name();
        mPreviousOutputDefinitions.insert( output->name(), it.value() );
        break;
      }
    }

    QList< QgsProcessingModelChildParameterSource > valueList;
    std::optional< QgsProcessingModelChildParameterSource > singleValue;
    if ( modelOutputName.isEmpty() && childAlgorithm.parameterSources().contains( output->name() ) )
    {
      valueList = childAlgorithm.parameterSources().value( output->name() );
      if ( valueList.size() == 1 )
      {
        singleValue = valueList.at( 0 );
        valueList.clear();
      }
    }

    if ( mWrappers.contains( output->name() ) )
    {
      QgsProcessingModelerParameterWidget *wrapper = mWrappers.value( output->name() );
      if ( !modelOutputName.isEmpty() )
      {
        wrapper->setToModelOutput( modelOutputName );
      }
      else if ( !valueList.empty() )
      {
        wrapper->setWidgetValue( valueList );
      }
      else if ( singleValue.has_value() )
      {
        wrapper->setWidgetValue( singleValue.value() );
      }
      else if ( output->defaultValue().isValid() )
      {
        wrapper->setWidgetValue( QgsProcessingModelChildParameterSource::fromStaticValue( output->defaultValue() ) );
      }
    }
  }

  mDependenciesPanel->setValue( childAlgorithm.dependencies() );

  mBlockChangesSignal--;
}

std::unique_ptr< QgsProcessingModelChildAlgorithm > QgsProcessingModelerParametersPanelWidget::createAlgorithm()
{
  if ( !mAlgorithm )
    return nullptr;

  auto newChildAlgorithm = std::make_unique< QgsProcessingModelChildAlgorithm >( mAlgorithm->id() );
  if ( mChildId.isEmpty() )
    newChildAlgorithm->generateChildId( *mModel );
  else
    newChildAlgorithm->setChildId( mChildId );

  newChildAlgorithm->setDescription( mDescriptionBox->text() );
  if ( mAlgorithmItem )
  {
    newChildAlgorithm->setConfiguration( mAlgorithmItem->configuration() );
    mAlgorithm.reset( newChildAlgorithm->algorithm()->create( mAlgorithmItem->configuration() ) );
  }

  const QList<const QgsProcessingParameterDefinition * > parameterDefinitions = mAlgorithm->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *parameter : parameterDefinitions )
  {
    if ( parameter->isDestination() || ( parameter->flags() & Qgis::ProcessingParameterFlag::Hidden ) )
      continue;

    if ( !mWrappers.contains( parameter->name() ) )
      continue;

    const QgsProcessingModelerParameterWidget *wrapper = mWrappers.value( parameter->name() );
    QVariant value = wrapper->value();

    QList< QgsProcessingModelChildParameterSource > sources;
    if ( value.userType() == qMetaTypeId< QgsProcessingModelChildParameterSource >() )
    {
      sources.append( value.value< QgsProcessingModelChildParameterSource >() );
    }
    else if ( value.userType() == QMetaType::Type::QVariantList )
    {
      const QVariantList list = value.toList();
      for ( const QVariant &subValue : list )
      {
        if ( subValue.userType() == qMetaTypeId< QgsProcessingModelChildParameterSource >() )
          sources.append( subValue.value< QgsProcessingModelChildParameterSource >() );
        else
          sources.append( QgsProcessingModelChildParameterSource::fromStaticValue( subValue ) );
      }
    }
    else
    {
      sources.append( QgsProcessingModelChildParameterSource::fromStaticValue( value ) );
    }

    bool valid = true;
    for ( const QgsProcessingModelChildParameterSource &subValue : std::as_const( sources ) )
    {
      if ( subValue.source() == Qgis::ProcessingModelChildParameterSource::StaticValue && !parameter->checkValueIsAcceptable( subValue.staticValue() ) )
      {
        valid = false;
        break;
      }
    }

    if ( valid )
      newChildAlgorithm->addParameterSources( parameter->name(), sources );
  }

  QMap< QString, QgsProcessingModelOutput > outputs;
  const QList< const QgsProcessingParameterDefinition * > destinationParameters = mAlgorithm->destinationParameterDefinitions();
  for ( const QgsProcessingParameterDefinition *output : destinationParameters )
  {
    if ( !( output->flags() & Qgis::ProcessingParameterFlag::Hidden ) )
    {
      if ( mWrappers.contains( output->name() ) )
      {
        QgsProcessingModelerParameterWidget *wrapper = mWrappers.value( output->name() );
        if ( wrapper->isModelOutput() )
        {
          const QString name = wrapper->modelOutputName();
          if ( !name.isEmpty() )
          {
            // if there was a previous output definition already for this output, we start with it,
            // otherwise we'll lose any existing output comments, coloring, position, etc
            QgsProcessingModelOutput modelOutput = mPreviousOutputDefinitions.value( output->name(), QgsProcessingModelOutput( name, name ) );
            modelOutput.setDescription( name );
            modelOutput.setChildId( newChildAlgorithm->childId() );
            modelOutput.setChildOutputName( output->name() );
            outputs.insert( name, modelOutput );
          }
        }
        else
        {
          const QVariant val = wrapper->value();
          QList< QgsProcessingModelChildParameterSource > sources;
          if ( val.userType() == qMetaTypeId< QgsProcessingModelChildParameterSource >() )
          {
            sources.append( val.value< QgsProcessingModelChildParameterSource >() );
          }
          else if ( val.userType() == QMetaType::Type::QVariantList )
          {
            const QVariantList list = val.toList();
            for ( const QVariant &subValue : list )
            {
              if ( subValue.userType() == qMetaTypeId< QgsProcessingModelChildParameterSource >() )
                sources.append( subValue.value< QgsProcessingModelChildParameterSource >() );
              else
                sources.append( QgsProcessingModelChildParameterSource::fromStaticValue( subValue ) );
            }
          }

          newChildAlgorithm->addParameterSources( output->name(), sources );
        }
      }
    }

    if ( output->flags() & Qgis::ProcessingParameterFlag::IsModelOutput )
    {
      if ( !outputs.contains( output->name() ) )
      {
        QgsProcessingModelOutput modelOutput( output->name(), output->name() );
        modelOutput.setChildId( newChildAlgorithm->childId() );
        modelOutput.setChildOutputName( output->name() );
        outputs.insert( output->name(), modelOutput );
      }
    }
  }

  newChildAlgorithm->setModelOutputs( outputs );
  newChildAlgorithm->setDependencies( mDependenciesPanel->value() );

  return newChildAlgorithm;
}

QgsProcessingContext *QgsProcessingModelerParametersPanelWidget::processingContext() const
{
  return &mContext;
}

void QgsProcessingModelerParametersPanelWidget::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  if ( mAlgorithmItem )
  {
    mAlgorithmItem->setWidgetContext( context );
  }
  for ( auto it = mWrappers.constBegin(); it != mWrappers.constEnd(); ++it )
  {
    it.value()->setWidgetContext( context );
  }
}


//
// QgsProcessingModelerParametersWidget
//

QgsProcessingModelerParametersWidget::QgsProcessingModelerParametersWidget(
  const QgsProcessingAlgorithm *childAlgorithm, QgsProcessingModelAlgorithm *model, QgsProcessingContext &context, const QString &childId, const QVariantMap &configuration, QWidget *parent, QWidget *dialog
)
  : QgsProcessingModelConfigWidget( parent )
{
  mParametersPanel = new QgsProcessingModelerParametersPanelWidget( childAlgorithm, model, context, childId, configuration, this, dialog );
  connect( mParametersPanel, &QgsProcessingModelerParametersPanelWidget::widgetChanged, this, &QgsProcessingModelConfigWidget::widgetChanged );

  setupUi();
}

QgsProcessingModelerParametersWidget::~QgsProcessingModelerParametersWidget() = default;

const QgsProcessingAlgorithm *QgsProcessingModelerParametersWidget::algorithm() const
{
  return mParametersPanel->algorithm();
}

void QgsProcessingModelerParametersWidget::switchToCommentTab()
{
  mTabWidget->setCurrentIndex( 1 );
  mCommentEdit->setFocus();
  mCommentEdit->selectAll();
}

void QgsProcessingModelerParametersWidget::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  mParametersPanel->setWidgetContext( context );
}

void QgsProcessingModelerParametersWidget::setupUi()
{
  auto mainLayout = new QVBoxLayout();
  mainLayout->setContentsMargins( 0, 0, 0, 0 );

  mTabWidget = new QTabWidget();
  mainLayout->addWidget( mTabWidget );

  mPanelWidgetStack = new QgsPanelWidgetStack();
  mParametersPanel->setDockMode( true );
  mPanelWidgetStack->setMainPanel( mParametersPanel );

  mTabWidget->addTab( mPanelWidgetStack, tr( "Properties" ) );

  auto commentLayout = new QVBoxLayout();
  mCommentEdit = new QTextEdit();
  mCommentEdit->setAcceptRichText( false );
  commentLayout->addWidget( mCommentEdit, 1 );
  connect( mCommentEdit, &QTextEdit::textChanged, this, &QgsProcessingModelerParametersWidget::widgetChanged );

  auto hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );
  hl->addWidget( new QLabel( tr( "Color" ) ) );

  mCommentColorButton = new QgsColorButton();
  mCommentColorButton->setAllowOpacity( true );
  mCommentColorButton->setWindowTitle( tr( "Comment Color" ) );
  mCommentColorButton->setShowNull( true, tr( "Default" ) );
  hl->addWidget( mCommentColorButton );
  commentLayout->addLayout( hl );
  connect( mCommentColorButton, &QgsColorButton::colorChanged, this, &QgsProcessingModelerParametersWidget::widgetChanged );

  auto commentWidget = new QWidget();
  commentWidget->setLayout( commentLayout );
  mTabWidget->addTab( commentWidget, tr( "Comments" ) );

  setLayout( mainLayout );
}

void QgsProcessingModelerParametersWidget::setComments( const QString &text )
{
  mCommentEdit->setPlainText( text );
}

QString QgsProcessingModelerParametersWidget::comments() const
{
  return mCommentEdit->toPlainText();
}

void QgsProcessingModelerParametersWidget::setCommentColor( const QColor &color )
{
  if ( color.isValid() )
    mCommentColorButton->setColor( color );
  else
    mCommentColorButton->setToNull();
}

QColor QgsProcessingModelerParametersWidget::commentColor() const
{
  return !mCommentColorButton->isNull() ? mCommentColorButton->color() : QColor();
}

void QgsProcessingModelerParametersWidget::setStateFromChildAlgorithm()
{
  mParametersPanel->setStateFromChildAlgorithm();
}

std::unique_ptr< QgsProcessingModelChildAlgorithm > QgsProcessingModelerParametersWidget::createAlgorithm()
{
  std::unique_ptr< QgsProcessingModelChildAlgorithm > alg = mParametersPanel->createAlgorithm();
  if ( alg )
  {
    alg->comment()->setDescription( comments() );
    alg->comment()->setColor( commentColor() );
  }
  return alg;
}

//
// QgsProcessingModelerParametersDialog
//

QgsProcessingModelerParametersDialog::QgsProcessingModelerParametersDialog(
  const QgsProcessingAlgorithm *childAlgorithm, QgsProcessingModelAlgorithm *model, QgsProcessingContext &context, const QString &childId, const QVariantMap &configuration, QWidget *parent
)
  : QDialog( parent )
{
  setObjectName( u"QgsProcessingModelerParametersDialog"_s );

  setStyleSheet( QgsGui::applicationStyleSheet() );
  connect( QgsGui::instance(), &QgsGui::applicationStyleSheetChanged, this, &QgsProcessingModelerParametersDialog::setStyleSheet );

  mWidget = new QgsProcessingModelerParametersWidget( childAlgorithm, model, context, childId, configuration, this, this );

  setWindowTitle( childAlgorithm ? ( childAlgorithm->group().isEmpty() ? childAlgorithm->displayName() : u"%1 - %2"_s.arg( childAlgorithm->group(), childAlgorithm->displayName() ) ) : QString() );

  QgsGui::enableAutoGeometryRestore( this );

  mButtonBox = new QDialogButtonBox();
  mButtonBox->setOrientation( Qt::Orientation::Horizontal );
  mButtonBox->setStandardButtons( QDialogButtonBox::StandardButton::Cancel | QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Help );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsProcessingModelerParametersDialog::okPressed );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProcessingModelerParametersDialog::openHelp );

  auto mainLayout = new QVBoxLayout();
  mainLayout->addWidget( mWidget, 1 );
  mainLayout->addWidget( mButtonBox );
  setLayout( mainLayout );
}

QgsProcessingModelerParametersDialog::~QgsProcessingModelerParametersDialog() = default;

const QgsProcessingAlgorithm *QgsProcessingModelerParametersDialog::algorithm() const
{
  return mWidget->algorithm();
}

void QgsProcessingModelerParametersDialog::setComments( const QString &text )
{
  mWidget->setComments( text );
}

QString QgsProcessingModelerParametersDialog::comments() const
{
  return mWidget->comments();
}

void QgsProcessingModelerParametersDialog::setCommentColor( const QColor &color )
{
  mWidget->setCommentColor( color );
}

QColor QgsProcessingModelerParametersDialog::commentColor() const
{
  return mWidget->commentColor();
}

void QgsProcessingModelerParametersDialog::switchToCommentTab()
{
  mWidget->switchToCommentTab();
}

void QgsProcessingModelerParametersDialog::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  mWidget->setWidgetContext( context );
}

void QgsProcessingModelerParametersDialog::setStateFromChildAlgorithm()
{
  mWidget->setStateFromChildAlgorithm();
}

std::unique_ptr< QgsProcessingModelChildAlgorithm > QgsProcessingModelerParametersDialog::createAlgorithm()
{
  return mWidget->createAlgorithm();
}

void QgsProcessingModelerParametersDialog::okPressed()
{
  if ( createAlgorithm() )
    accept();
}

void QgsProcessingModelerParametersDialog::openHelp()
{
  if ( !algorithm() )
    return;

  QString algHelp = algorithm()->helpUrl();
  if ( algHelp.isEmpty() && algorithm()->provider() )
  {
    algHelp = QgsHelp::helpUrl( u"processing_algs/%1/%2.html#%3"_s
                                  .arg( algorithm()->provider()->helpId(), algorithm()->groupId(), u"%1%2"_s.arg( algorithm()->provider()->helpId(), QString( algorithm()->name() ).replace( '_', '-' ) ) ) )
                .toString();
  }

  if ( !algHelp.isEmpty() )
    QDesktopServices::openUrl( QUrl( algHelp ) );
}
