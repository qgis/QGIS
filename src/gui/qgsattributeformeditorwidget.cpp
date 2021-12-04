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
#include "qgsattributeform.h"
#include "qgsmultiedittoolbutton.h"
#include "qgssearchwidgettoolbutton.h"
#include "qgseditorwidgetwrapper.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsattributeeditorcontext.h"
#include "qgseditorwidgetregistry.h"
#include "qgsaggregatetoolbutton.h"
#include "qgsgui.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvectorlayerutils.h"

#include <QLayout>
#include <QLabel>
#include <QStackedWidget>

QgsAttributeFormEditorWidget::QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper *editorWidget, const QString &widgetType, QgsAttributeForm *form )
  : QgsAttributeFormWidget( editorWidget, form )
  , mWidgetType( widgetType )
  , mEditorWidget( editorWidget )
  , mForm( form )
  , mMultiEditButton( new QgsMultiEditToolButton() )
  , mBlockValueUpdate( false )
  , mIsMixed( false )
  , mIsChanged( false )
{
  mConstraintResultLabel = new QLabel( this );
  mConstraintResultLabel->setObjectName( QStringLiteral( "ConstraintStatus" ) );
  mConstraintResultLabel->setSizePolicy( QSizePolicy::Fixed, mConstraintResultLabel->sizePolicy().verticalPolicy() );

  mMultiEditButton->setField( mEditorWidget->field() );
  mAggregateButton = new QgsAggregateToolButton();
  mAggregateButton->setType( mEditorWidget->field().type() );
  connect( mAggregateButton, &QgsAggregateToolButton::aggregateChanged, this, &QgsAttributeFormEditorWidget::onAggregateChanged );

  if ( mEditorWidget->widget() )
  {
    mEditorWidget->widget()->setObjectName( mEditorWidget->field().name() );
  }

  connect( mEditorWidget, &QgsEditorWidgetWrapper::valuesChanged, this, &QgsAttributeFormEditorWidget::editorWidgetValuesChanged );

  connect( mMultiEditButton, &QgsMultiEditToolButton::resetFieldValueTriggered, this, &QgsAttributeFormEditorWidget::resetValue );
  connect( mMultiEditButton, &QgsMultiEditToolButton::setFieldValueTriggered, this, &QgsAttributeFormEditorWidget::setFieldTriggered );

  mMultiEditButton->setField( mEditorWidget->field() );

  updateWidgets();
}

QgsAttributeFormEditorWidget::~QgsAttributeFormEditorWidget()
{
  //there's a chance these widgets are not currently added to the layout, so have no parent set
  delete mMultiEditButton;
}

void QgsAttributeFormEditorWidget::createSearchWidgetWrappers( const QgsAttributeEditorContext &context )
{
  Q_ASSERT( !mWidgetType.isEmpty() );
  const QVariantMap config = mEditorWidget->config();
  const int fieldIdx = mEditorWidget->fieldIdx();

  QgsSearchWidgetWrapper *sww = QgsGui::editorWidgetRegistry()->createSearchWidget( mWidgetType, layer(), fieldIdx, config,
                                searchWidgetFrame(), context );
  setSearchWidgetWrapper( sww );
  searchWidgetFrame()->layout()->addWidget( mAggregateButton );
  if ( sww->supportedFlags() & QgsSearchWidgetWrapper::Between ||
       sww->supportedFlags() & QgsSearchWidgetWrapper::IsNotBetween )
  {
    // create secondary widget for between type searches
    QgsSearchWidgetWrapper *sww2 = QgsGui::editorWidgetRegistry()->createSearchWidget( mWidgetType, layer(), fieldIdx, config,
                                   searchWidgetFrame(), context );
    addAdditionalSearchWidgetWrapper( sww2 );
  }
}

void QgsAttributeFormEditorWidget::setConstraintStatus( const QString &constraint, const QString &description, const QString &err, QgsEditorWidgetWrapper::ConstraintResult result )
{
  switch ( result )
  {
    case QgsEditorWidgetWrapper::ConstraintResultFailHard:
      mConstraintResultLabel->setText( QStringLiteral( "<font color=\"#FF9800\">%1</font>" ).arg( QChar( 0x2718 ) ) );
      mConstraintResultLabel->setToolTip( description.isEmpty() ? QStringLiteral( "<b>%1</b>: %2" ).arg( constraint, err ) : description );
      break;

    case QgsEditorWidgetWrapper::ConstraintResultFailSoft:
      mConstraintResultLabel->setText( QStringLiteral( "<font color=\"#FFC107\">%1</font>" ).arg( QChar( 0x2718 ) ) );
      mConstraintResultLabel->setToolTip( description.isEmpty() ? QStringLiteral( "<b>%1</b>: %2" ).arg( constraint, err ) : description );
      break;

    case QgsEditorWidgetWrapper::ConstraintResultPass:
      mConstraintResultLabel->setText( QStringLiteral( "<font color=\"#259B24\">%1</font>" ).arg( QChar( 0x2714 ) ) );
      mConstraintResultLabel->setToolTip( description );
      break;
  }
}

void QgsAttributeFormEditorWidget::setConstraintResultVisible( bool editable )
{
  mConstraintResultLabel->setHidden( !editable );
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

  switch ( mode() )
  {
    case DefaultMode:
    case MultiEditMode:
    {
      stack()->setCurrentWidget( editPage() );
      editPage()->layout()->addWidget( mConstraintResultLabel );
      break;
    }

    case AggregateSearchMode:
    {
      mAggregateButton->setVisible( true );
      stack()->setCurrentWidget( searchPage() );
      break;
    }

    case SearchMode:
    {
      mAggregateButton->setVisible( false );
      stack()->setCurrentWidget( searchPage() );
      break;
    }
  }
}
