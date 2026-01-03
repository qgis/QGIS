/***************************************************************************
    qgsattributeformeditorwidget.cpp
     -------------------------------
    Date                 : March 2016
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeformeditorwidget.h"

#include "qgsaggregatetoolbutton.h"
#include "qgsapplication.h"
#include "qgsattributeeditorcontext.h"
#include "qgsattributeform.h"
#include "qgseditorwidgetregistry.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsgui.h"
#include "qgsmultiedittoolbutton.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsvectorlayerutils.h"

#include <QLabel>
#include <QLayout>
#include <QStackedWidget>

#include "moc_qgsattributeformeditorwidget.cpp"

QgsAttributeFormEditorWidget::QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper *editorWidget, const QString &widgetType, QgsAttributeForm *form )
  : QgsAttributeFormWidget( editorWidget, form )
  , mWidgetType( widgetType )
  , mEditorWidget( editorWidget )
  , mForm( form )
  , mMultiEditButton( new QgsMultiEditToolButton() )
{
  mRememberLastValueButton = new QToolButton();
  mRememberLastValueButton->setAutoRaise( true );
  mRememberLastValueButton->setCheckable( true );
  mRememberLastValueButton->setIcon( QgsApplication::getThemeIcon( u"/mIconRememberDisabled.svg"_s ) );
  mRememberLastValueButton->setToolTip( tr( "When enabled, the entered value will be remembered and reused for the next feature additions" ) );
  updateRememberWidget();

  connect( mRememberLastValueButton, &QAbstractButton::toggled, this, [this]( bool checked ) {
    mRememberLastValueButton->setIcon( QgsApplication::getThemeIcon( checked ? u"/mIconRememberEnabled.svg"_s : u"/mIconRememberDisabled.svg"_s ) );
    emit rememberLastValueChanged( mEditorWidget->fieldIdx(), checked );
  } );
  connect( mForm, &QgsAttributeForm::modeChanged, this, [this]( QgsAttributeEditorContext::Mode ) {
    updateRememberWidget();
  } );

  mConstraintResultLabel = new QLabel();
  mConstraintResultLabel->setObjectName( u"ConstraintStatus"_s );
  mConstraintResultLabel->setSizePolicy( QSizePolicy::Fixed, mConstraintResultLabel->sizePolicy().verticalPolicy() );
  mConstraintResultLabel->setAlignment( Qt::AlignCenter );
  mConstraintResultLabel->setFixedWidth( 24 );

  mAggregateButton = new QgsAggregateToolButton();
  mAggregateButton->setType( mEditorWidget->field().type() );
  connect( mAggregateButton, &QgsAggregateToolButton::aggregateChanged, this, &QgsAttributeFormEditorWidget::onAggregateChanged );

  if ( mEditorWidget->widget() )
  {
    mEditorWidget->widget()->setObjectName( mEditorWidget->field().name() );
  }

  connect( mEditorWidget, &QgsEditorWidgetWrapper::valuesChanged, this, &QgsAttributeFormEditorWidget::editorWidgetValuesChanged );

  mMultiEditButton->setField( mEditorWidget->field() );
  connect( mMultiEditButton, &QgsMultiEditToolButton::resetFieldValueTriggered, this, &QgsAttributeFormEditorWidget::resetValue );
  connect( mMultiEditButton, &QgsMultiEditToolButton::setFieldValueTriggered, this, &QgsAttributeFormEditorWidget::setFieldTriggered );

  updateWidgets();
}

QgsAttributeFormEditorWidget::~QgsAttributeFormEditorWidget()
{
  //there's a chance these widgets are not currently added to the layout, so have no parent set
  delete mMultiEditButton;
  delete mRememberLastValueButton;
  delete mConstraintResultLabel;
}

void QgsAttributeFormEditorWidget::createSearchWidgetWrappers( const QgsAttributeEditorContext &context )
{
  Q_ASSERT( !mWidgetType.isEmpty() );
  const QVariantMap config = mEditorWidget->config();
  const int fieldIdx = mEditorWidget->fieldIdx();

  QgsSearchWidgetWrapper *sww = QgsGui::editorWidgetRegistry()->createSearchWidget( mWidgetType, layer(), fieldIdx, config, searchWidgetFrame(), context );
  setSearchWidgetWrapper( sww );
  searchWidgetFrame()->layout()->addWidget( mAggregateButton );
  if ( sww->supportedFlags() & QgsSearchWidgetWrapper::Between || sww->supportedFlags() & QgsSearchWidgetWrapper::IsNotBetween )
  {
    // create secondary widget for between type searches
    QgsSearchWidgetWrapper *sww2 = QgsGui::editorWidgetRegistry()->createSearchWidget( mWidgetType, layer(), fieldIdx, config, searchWidgetFrame(), context );
    addAdditionalSearchWidgetWrapper( sww2 );
  }
}

void QgsAttributeFormEditorWidget::setConstraintStatus( const QString &constraint, const QString &description, const QString &err, QgsEditorWidgetWrapper::ConstraintResult result )
{
  switch ( result )
  {
    case QgsEditorWidgetWrapper::ConstraintResultFailHard:
      mConstraintResultLabel->setText( u"<font color=\"#FF9800\">%1</font>"_s.arg( QChar( 0x2718 ) ) );
      mConstraintResultLabel->setToolTip( description.isEmpty() ? u"<b>%1</b>: %2"_s.arg( constraint, err ) : description );
      break;

    case QgsEditorWidgetWrapper::ConstraintResultFailSoft:
      mConstraintResultLabel->setText( u"<font color=\"#FFC107\">%1</font>"_s.arg( QChar( 0x2718 ) ) );
      mConstraintResultLabel->setToolTip( description.isEmpty() ? u"<b>%1</b>: %2"_s.arg( constraint, err ) : description );
      break;

    case QgsEditorWidgetWrapper::ConstraintResultPass:
      mConstraintResultLabel->setText( u"<font color=\"#259B24\">%1</font>"_s.arg( QChar( 0x2714 ) ) );
      mConstraintResultLabel->setToolTip( description );
      break;
  }
}

void QgsAttributeFormEditorWidget::setConstraintResultVisible( bool editable )
{
  mIsConstraintResultVisible = editable;

  switch ( mode() )
  {
    case SearchMode:
    case AggregateSearchMode:
      return;
    case DefaultMode:
    case MultiEditMode:
      break;
  }

  if ( !layer() || QgsVectorLayerUtils::attributeHasConstraints( layer(), mEditorWidget->fieldIdx() ) )
  {
    const bool hasConstraintResultLabel = ( editPage()->layout()->indexOf( mConstraintResultLabel ) >= 0 );
    if ( editable && !hasConstraintResultLabel )
    {
      editPage()->layout()->addWidget( mConstraintResultLabel );
    }
    else if ( !editable && hasConstraintResultLabel )
    {
      editPage()->layout()->removeWidget( mConstraintResultLabel );
      mConstraintResultLabel->setParent( nullptr );
    }
  }
}

QgsEditorWidgetWrapper *QgsAttributeFormEditorWidget::editorWidget() const
{
  return mEditorWidget;
}

void QgsAttributeFormEditorWidget::setIsMixed( bool mixed )
{
  if ( mEditorWidget && mixed )
    mEditorWidget->showIndeterminateState();
  mMultiEditButton->setIsMixed( mixed );
  mIsMixed = mixed;
}

void QgsAttributeFormEditorWidget::setRememberLastValue( bool remember )
{
  mRememberLastValueButton->setChecked( remember );
  mRememberLastValueButton->setIcon( QgsApplication::getThemeIcon( remember ? u"/mIconRememberEnabled.svg"_s : u"/mIconRememberDisabled.svg"_s ) );
}

void QgsAttributeFormEditorWidget::changesCommitted()
{
  if ( mEditorWidget )
  {
    mPreviousValue = mEditorWidget->value();
    mPreviousAdditionalValues = mEditorWidget->additionalFieldValues();
  }

  setIsMixed( false );
  mMultiEditButton->changesCommitted();
  mIsChanged = false;
}


void QgsAttributeFormEditorWidget::initialize( const QVariant &initialValue, bool mixedValues, const QVariantList &additionalFieldValues )
{
  if ( mEditorWidget )
  {
    mBlockValueUpdate = true;
    mEditorWidget->setValues( initialValue, additionalFieldValues );
    mBlockValueUpdate = false;
  }
  mPreviousValue = initialValue;
  mPreviousAdditionalValues = additionalFieldValues;
  setIsMixed( mixedValues );
  mMultiEditButton->setIsChanged( false );
  mIsChanged = false;
  updateWidgets();
}

QVariant QgsAttributeFormEditorWidget::currentValue() const
{
  return mEditorWidget->value();
}


void QgsAttributeFormEditorWidget::editorWidgetValuesChanged( const QVariant &value, const QVariantList &additionalFieldValues )
{
  if ( mBlockValueUpdate )
    return;

  mIsChanged = true;

  switch ( mode() )
  {
    case DefaultMode:
    case SearchMode:
    case AggregateSearchMode:
      break;
    case MultiEditMode:
      mMultiEditButton->setIsChanged( true );
  }

  Q_NOWARN_DEPRECATED_PUSH
  emit valueChanged( value );
  Q_NOWARN_DEPRECATED_POP
  emit valuesChanged( value, additionalFieldValues );
}

void QgsAttributeFormEditorWidget::resetValue()
{
  mIsChanged = false;
  mBlockValueUpdate = true;
  if ( mEditorWidget )
    mEditorWidget->setValues( mPreviousValue, mPreviousAdditionalValues );
  mBlockValueUpdate = false;

  switch ( mode() )
  {
    case DefaultMode:
    case SearchMode:
    case AggregateSearchMode:
      break;
    case MultiEditMode:
    {
      mMultiEditButton->setIsChanged( false );
      if ( mEditorWidget && mIsMixed )
        mEditorWidget->showIndeterminateState();
      break;
    }
  }
}

void QgsAttributeFormEditorWidget::setFieldTriggered()
{
  mIsChanged = true;
}

void QgsAttributeFormEditorWidget::onAggregateChanged()
{
  const auto constWigets( searchWidgetWrappers() );
  for ( QgsSearchWidgetWrapper *searchWidget : constWigets )
    searchWidget->setAggregate( mAggregateButton->aggregate() );
}

void QgsAttributeFormEditorWidget::updateRememberWidget()
{
  const bool hasRememberButton = ( editPage()->layout()->indexOf( mRememberLastValueButton ) >= 0 );
  const int idx = mEditorWidget->fieldIdx();
  if ( !hasRememberButton && form() && form()->mode() == QgsAttributeEditorContext::AddFeatureMode )
  {
    if ( layer() && layer()->editFormConfig().reuseLastValuePolicy( idx ) != Qgis::AttributeFormReuseLastValuePolicy::NotAllowed )
    {
      editPage()->layout()->addWidget( mRememberLastValueButton );
    }
  }
  else if ( hasRememberButton )
  {
    editPage()->layout()->removeWidget( mRememberLastValueButton );
    mRememberLastValueButton->setParent( nullptr );
  }
}

void QgsAttributeFormEditorWidget::updateWidgets()
{
  //first update the tool buttons
  const bool hasMultiEditButton = ( editPage()->layout()->indexOf( mMultiEditButton ) >= 0 );

  bool shouldShowMultiEditButton = false;
  switch ( mode() )
  {
    case QgsAttributeFormWidget::DefaultMode:
    case QgsAttributeFormWidget::SearchMode:
    case QgsAttributeFormWidget::AggregateSearchMode:
      // in these modes we don't show the multi edit button
      shouldShowMultiEditButton = false;
      break;

    case QgsAttributeFormWidget::MultiEditMode:
    {
      // in multi-edit mode we need to know upfront whether or not to allow add the multiedit buttons
      // for this field.
      // if the field is always read only regardless of the feature, no need to dig further. But otherwise
      // we may need to test editability for the actual selected features...
      if ( mEditorWidget )
      {
        const int fieldIndex = mEditorWidget->fieldIdx();
        shouldShowMultiEditButton = !QgsVectorLayerUtils::fieldIsReadOnly( layer(), fieldIndex );
        if ( shouldShowMultiEditButton )
        {
          // depending on the field type, the editability of the field may vary feature by feature (e.g. for joined
          // fields coming from joins without the upsert on edit capabilities).
          // But this feature-by-feature check is EXPENSIVE!!! (see https://github.com/qgis/QGIS/issues/41366), so
          // avoid it whenever we can...
          const bool fieldEditabilityDependsOnFeature = QgsVectorLayerUtils::fieldEditabilityDependsOnFeature( layer(), fieldIndex );
          if ( fieldEditabilityDependsOnFeature )
          {
            QgsFeature feature;
            QgsFeatureIterator it = layer()->getSelectedFeatures();
            while ( it.nextFeature( feature ) )
            {
              const bool isEditable = QgsVectorLayerUtils::fieldIsEditable( layer(), fieldIndex, feature );
              if ( !isEditable )
              {
                // as soon as we find one read-only feature for the field, we can break early...
                shouldShowMultiEditButton = false;
                break;
              }
            }
          }
        }
      }
    }
    break;
  }

  if ( hasMultiEditButton && !shouldShowMultiEditButton )
  {
    editPage()->layout()->removeWidget( mMultiEditButton );
    mMultiEditButton->setParent( nullptr );
  }
  else if ( !hasMultiEditButton && shouldShowMultiEditButton )
  {
    editPage()->layout()->addWidget( mMultiEditButton );
  }

  setVisiblePageForMode( mode() );

  switch ( mode() )
  {
    case DefaultMode:
    case MultiEditMode:
    {
      if ( mIsConstraintResultVisible && editPage()->layout()->indexOf( mConstraintResultLabel ) == -1 )
      {
        if ( !layer() || ( mEditorWidget && QgsVectorLayerUtils::attributeHasConstraints( layer(), mEditorWidget->fieldIdx() ) ) )
        {
          editPage()->layout()->addWidget( mConstraintResultLabel );
        }
      }
      break;
    }

    case AggregateSearchMode:
    {
      mAggregateButton->setVisible( true );
      break;
    }

    case SearchMode:
    {
      mAggregateButton->setVisible( false );
      break;
    }
  }
}
