/***************************************************************************
    qgsattributeform.cpp
     --------------------------------------
    Date                 : 3.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeform.h"

#include "qgsattributeforminterface.h"
#include "qgsattributeformlegacyinterface.h"
#include "qgsattributeformrelationeditorwidget.h"
#include "qgseditorwidgetregistry.h"
#include "qgsfeatureiterator.h"
#include "qgsproject.h"
#include "qgspythonrunner.h"
#include "qgsrelationwidgetwrapper.h"
#include "qgsvectordataprovider.h"
#include "qgsattributeformeditorwidget.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsnetworkcontentfetcherregistry.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsrelationmanager.h"
#include "qgslogger.h"
#include "qgstabwidget.h"
#include "qgssettings.h"
#include "qgsscrollarea.h"
#include "qgsgui.h"
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvectorlayerutils.h"
#include "qgsqmlwidgetwrapper.h"
#include "qgshtmlwidgetwrapper.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"

#include <QDir>
#include <QTextStream>
#include <QFileInfo>
#include <QFile>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QUiLoader>
#include <QMessageBox>
#include <QToolButton>
#include <QMenu>

int QgsAttributeForm::sFormCounter = 0;

QgsAttributeForm::QgsAttributeForm( QgsVectorLayer *vl, const QgsFeature &feature, const QgsAttributeEditorContext &context, QWidget *parent )
  : QWidget( parent )
  , mLayer( vl )
  , mOwnsMessageBar( true )
  , mContext( context )
  , mFormNr( sFormCounter++ )
  , mIsSaving( false )
  , mPreventFeatureRefresh( false )
  , mIsSettingMultiEditFeatures( false )
  , mUnsavedMultiEditChanges( false )
  , mEditCommandMessage( tr( "Attributes changed" ) )
  , mMode( QgsAttributeEditorContext::SingleEditMode )
{
  init();
  initPython();
  setFeature( feature );

  connect( vl, &QgsVectorLayer::updatedFields, this, &QgsAttributeForm::onUpdatedFields );
  connect( vl, &QgsVectorLayer::beforeAddingExpressionField, this, &QgsAttributeForm::preventFeatureRefresh );
  connect( vl, &QgsVectorLayer::beforeRemovingExpressionField, this, &QgsAttributeForm::preventFeatureRefresh );
  connect( vl, &QgsVectorLayer::selectionChanged, this, &QgsAttributeForm::layerSelectionChanged );
  connect( this, &QgsAttributeForm::modeChanged, this, &QgsAttributeForm::updateContainersVisibility );

  updateContainersVisibility();
}

QgsAttributeForm::~QgsAttributeForm()
{
  cleanPython();
  qDeleteAll( mInterfaces );
}

void QgsAttributeForm::hideButtonBox()
{
  mButtonBox->hide();

  // Make sure that changes are taken into account if somebody tries to figure out if there have been some
  if ( mMode == QgsAttributeEditorContext::SingleEditMode )
    connect( mLayer, &QgsVectorLayer::beforeModifiedCheck, this, &QgsAttributeForm::save );
}

void QgsAttributeForm::showButtonBox()
{
  mButtonBox->show();

  disconnect( mLayer, &QgsVectorLayer::beforeModifiedCheck, this, &QgsAttributeForm::save );
}

void QgsAttributeForm::disconnectButtonBox()
{
  disconnect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsAttributeForm::save );
  disconnect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsAttributeForm::resetValues );
}

void QgsAttributeForm::addInterface( QgsAttributeFormInterface *iface )
{
  mInterfaces.append( iface );
}

bool QgsAttributeForm::editable()
{
  return mFeature.isValid() && mLayer->isEditable();
}

void QgsAttributeForm::setMode( QgsAttributeEditorContext::Mode mode )
{
  if ( mode == mMode )
    return;

  if ( mMode == QgsAttributeEditorContext::MultiEditMode )
  {
    //switching out of multi edit mode triggers a save
    if ( mUnsavedMultiEditChanges )
    {
      // prompt for save
      int res = QMessageBox::question( this, tr( "Multiedit Attributes" ),
                                       tr( "Apply changes to edited features?" ), QMessageBox::Yes | QMessageBox::No );
      if ( res == QMessageBox::Yes )
      {
        save();
      }
    }
    clearMultiEditMessages();
  }
  mUnsavedMultiEditChanges = false;

  mMode = mode;

  if ( mButtonBox->isVisible() && mMode == QgsAttributeEditorContext::SingleEditMode )
  {
    connect( mLayer, &QgsVectorLayer::beforeModifiedCheck, this, &QgsAttributeForm::save );
  }
  else
  {
    disconnect( mLayer, &QgsVectorLayer::beforeModifiedCheck, this, &QgsAttributeForm::save );
  }

  //update all form editor widget modes to match
  for ( QgsAttributeFormWidget *w : qgis::as_const( mFormWidgets ) )
  {
    switch ( mode )
    {
      case QgsAttributeEditorContext::SingleEditMode:
        w->setMode( QgsAttributeFormWidget::DefaultMode );
        break;

      case QgsAttributeEditorContext::AddFeatureMode:
        w->setMode( QgsAttributeFormWidget::DefaultMode );
        break;

      case QgsAttributeEditorContext::MultiEditMode:
        w->setMode( QgsAttributeFormWidget::MultiEditMode );
        break;

      case QgsAttributeEditorContext::SearchMode:
        w->setMode( QgsAttributeFormWidget::SearchMode );
        break;

      case QgsAttributeEditorContext::AggregateSearchMode:
        w->setMode( QgsAttributeFormWidget::AggregateSearchMode );
        break;

      case QgsAttributeEditorContext::IdentifyMode:
        w->setMode( QgsAttributeFormWidget::DefaultMode );
        break;
    }
  }
  //update all form editor widget modes to match
  for ( QgsWidgetWrapper *w : qgis::as_const( mWidgets ) )
  {
    QgsAttributeEditorContext newContext = w->context();
    newContext.setAttributeFormMode( mMode );
    w->setContext( newContext );
  }

  bool relationWidgetsVisible = ( mMode != QgsAttributeEditorContext::MultiEditMode && mMode != QgsAttributeEditorContext::AggregateSearchMode );
  for ( QgsAttributeFormRelationEditorWidget *w : findChildren<  QgsAttributeFormRelationEditorWidget * >() )
  {
    w->setVisible( relationWidgetsVisible );
  }

  switch ( mode )
  {
    case QgsAttributeEditorContext::SingleEditMode:
      setFeature( mFeature );
      mSearchButtonBox->setVisible( false );
      break;

    case QgsAttributeEditorContext::AddFeatureMode:
      synchronizeEnabledState();
      mSearchButtonBox->setVisible( false );
      break;

    case QgsAttributeEditorContext::MultiEditMode:
      resetMultiEdit( false );
      synchronizeEnabledState();
      mSearchButtonBox->setVisible( false );
      break;

    case QgsAttributeEditorContext::SearchMode:
      mSearchButtonBox->setVisible( true );
      hideButtonBox();
      break;

    case QgsAttributeEditorContext::AggregateSearchMode:
      mSearchButtonBox->setVisible( false );
      hideButtonBox();
      break;

    case QgsAttributeEditorContext::IdentifyMode:
      setFeature( mFeature );
      mSearchButtonBox->setVisible( false );
      break;
  }

  emit modeChanged( mMode );
}

void QgsAttributeForm::changeAttribute( const QString &field, const QVariant &value, const QString &hintText )
{
  Q_FOREACH ( QgsWidgetWrapper *ww, mWidgets )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( eww && eww->field().name() == field )
    {
      eww->setValue( value );
      eww->setHint( hintText );
    }
  }
}

void QgsAttributeForm::setFeature( const QgsFeature &feature )
{
  mIsSettingFeature = true;
  mFeature = feature;

  switch ( mMode )
  {
    case QgsAttributeEditorContext::SingleEditMode:
    case QgsAttributeEditorContext::IdentifyMode:
    case QgsAttributeEditorContext::AddFeatureMode:
    {
      resetValues();

      synchronizeEnabledState();

      Q_FOREACH ( QgsAttributeFormInterface *iface, mInterfaces )
      {
        iface->featureChanged();
      }
      break;
    }
    case QgsAttributeEditorContext::SearchMode:
    case QgsAttributeEditorContext::AggregateSearchMode:
    {
      resetValues();
      break;
    }
    case QgsAttributeEditorContext::MultiEditMode:
    {
      //ignore setFeature
      break;
    }
  }
  mIsSettingFeature = false;
}

bool QgsAttributeForm::saveEdits()
{
  bool success = true;
  bool changedLayer = false;

  QgsFeature updatedFeature = QgsFeature( mFeature );

  if ( mFeature.isValid() || mMode == QgsAttributeEditorContext::AddFeatureMode )
  {
    bool doUpdate = false;

    // An add dialog should perform an action by default
    // and not only if attributes have "changed"
    if ( mMode == QgsAttributeEditorContext::AddFeatureMode )
      doUpdate = true;

    QgsAttributes src = mFeature.attributes();
    QgsAttributes dst = mFeature.attributes();

    for ( QgsWidgetWrapper *ww : qgis::as_const( mWidgets ) )
    {
      QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
      if ( eww )
      {
        QVariant dstVar = dst.at( eww->fieldIdx() );
        QVariant srcVar = eww->value();

        // need to check dstVar.isNull() != srcVar.isNull()
        // otherwise if dstVar=NULL and scrVar=0, then dstVar = srcVar
        // be careful- sometimes two null qvariants will be reported as not equal!! (e.g., different types)
        bool changed = ( dstVar != srcVar && !dstVar.isNull() && !srcVar.isNull() )
                       || ( dstVar.isNull() != srcVar.isNull() );
        if ( changed && srcVar.isValid() && fieldIsEditable( eww->fieldIdx() ) )
        {
          dst[eww->fieldIdx()] = srcVar;

          doUpdate = true;
        }
      }
    }

    updatedFeature.setAttributes( dst );

    Q_FOREACH ( QgsAttributeFormInterface *iface, mInterfaces )
    {
      if ( !iface->acceptChanges( updatedFeature ) )
      {
        doUpdate = false;
      }
    }

    if ( doUpdate )
    {
      if ( mMode == QgsAttributeEditorContext::AddFeatureMode )
      {
        mFeature.setValid( true );
        mLayer->beginEditCommand( mEditCommandMessage );
        bool res = mLayer->addFeature( updatedFeature );
        if ( res )
        {
          mFeature.setAttributes( updatedFeature.attributes() );
          mLayer->endEditCommand();
          setMode( QgsAttributeEditorContext::SingleEditMode );
          changedLayer = true;
        }
        else
          mLayer->destroyEditCommand();
      }
      else
      {
        mLayer->beginEditCommand( mEditCommandMessage );

        QgsAttributeMap newValues;
        QgsAttributeMap oldValues;

        int n = 0;
        for ( int i = 0; i < dst.count(); ++i )
        {
          if ( qgsVariantEqual( dst.at( i ), src.at( i ) ) // If field is not changed...
               || !dst.at( i ).isValid()                 // or the widget returns invalid (== do not change)
               || !fieldIsEditable( i ) )                // or the field cannot be edited ...
          {
            continue;
          }

          QgsDebugMsg( QStringLiteral( "Updating field %1" ).arg( i ) );
          QgsDebugMsg( QStringLiteral( "dst:'%1' (type:%2, isNull:%3, isValid:%4)" )
                       .arg( dst.at( i ).toString(), dst.at( i ).typeName() ).arg( dst.at( i ).isNull() ).arg( dst.at( i ).isValid() ) );
          QgsDebugMsg( QStringLiteral( "src:'%1' (type:%2, isNull:%3, isValid:%4)" )
                       .arg( src.at( i ).toString(), src.at( i ).typeName() ).arg( src.at( i ).isNull() ).arg( src.at( i ).isValid() ) );

          newValues[i] = dst.at( i );
          oldValues[i] = src.at( i );

          n++;
        }

        success = mLayer->changeAttributeValues( mFeature.id(), newValues, oldValues );

        if ( success && n > 0 )
        {
          mLayer->endEditCommand();
          mFeature.setAttributes( dst );
          changedLayer = true;
        }
        else
        {
          mLayer->destroyEditCommand();
        }
      }
    }
  }

  emit featureSaved( updatedFeature );

  // [MD] Refresh canvas only when absolutely necessary - it interferes with other stuff (#11361).
  // This code should be revisited - and the signals should be fired (+ layer repainted)
  // only when actually doing any changes. I am unsure if it is actually a good idea
  // to call save() whenever some code asks for vector layer's modified status
  // (which is the case when attribute table is open)
  if ( changedLayer )
    mLayer->triggerRepaint();

  return success;
}

void QgsAttributeForm::resetMultiEdit( bool promptToSave )
{
  if ( promptToSave )
    save();

  mUnsavedMultiEditChanges = false;
  setMultiEditFeatureIds( mLayer->selectedFeatureIds() );
}

void QgsAttributeForm::multiEditMessageClicked( const QString &link )
{
  clearMultiEditMessages();
  resetMultiEdit( link == QLatin1String( "#apply" ) );
}

void QgsAttributeForm::filterTriggered()
{
  QString filter = createFilterExpression();
  emit filterExpressionSet( filter, ReplaceFilter );
  if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    setMode( QgsAttributeEditorContext::SingleEditMode );
}

void QgsAttributeForm::searchZoomTo()
{
  QString filter = createFilterExpression();
  if ( filter.isEmpty() )
    return;

  emit zoomToFeatures( filter );
}

void QgsAttributeForm::searchFlash()
{
  QString filter = createFilterExpression();
  if ( filter.isEmpty() )
    return;

  emit flashFeatures( filter );
}

void QgsAttributeForm::filterAndTriggered()
{
  QString filter = createFilterExpression();
  if ( filter.isEmpty() )
    return;

  if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    setMode( QgsAttributeEditorContext::SingleEditMode );
  emit filterExpressionSet( filter, FilterAnd );
}

void QgsAttributeForm::filterOrTriggered()
{
  QString filter = createFilterExpression();
  if ( filter.isEmpty() )
    return;

  if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    setMode( QgsAttributeEditorContext::SingleEditMode );
  emit filterExpressionSet( filter, FilterOr );
}

void QgsAttributeForm::pushSelectedFeaturesMessage()
{
  int count = mLayer->selectedFeatureCount();
  if ( count > 0 )
  {
    mMessageBar->pushMessage( QString(),
                              tr( "%n matching feature(s) selected", "matching features", count ),
                              Qgis::Info,
                              messageTimeout() );
  }
  else
  {
    mMessageBar->pushMessage( QString(),
                              tr( "No matching features found" ),
                              Qgis::Warning,
                              messageTimeout() );
  }
}

void QgsAttributeForm::runSearchSelect( QgsVectorLayer::SelectBehavior behavior )
{
  QString filter = createFilterExpression();
  if ( filter.isEmpty() )
    return;

  mLayer->selectByExpression( filter, behavior );
  pushSelectedFeaturesMessage();
  if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    setMode( QgsAttributeEditorContext::SingleEditMode );
}

void QgsAttributeForm::searchSetSelection()
{
  runSearchSelect( QgsVectorLayer::SetSelection );
}

void QgsAttributeForm::searchAddToSelection()
{
  runSearchSelect( QgsVectorLayer::AddToSelection );
}

void QgsAttributeForm::searchRemoveFromSelection()
{
  runSearchSelect( QgsVectorLayer::RemoveFromSelection );
}

void QgsAttributeForm::searchIntersectSelection()
{
  runSearchSelect( QgsVectorLayer::IntersectSelection );
}

bool QgsAttributeForm::saveMultiEdits()
{
  //find changed attributes
  QgsAttributeMap newAttributeValues;
  QMap< int, QgsAttributeFormEditorWidget * >::const_iterator wIt = mFormEditorWidgets.constBegin();
  for ( ; wIt != mFormEditorWidgets.constEnd(); ++ wIt )
  {
    QgsAttributeFormEditorWidget *w = wIt.value();
    if ( !w->hasChanged() )
      continue;

    if ( !w->currentValue().isValid() // if the widget returns invalid (== do not change)
         || mLayer->editFormConfig().readOnly( wIt.key() ) ) // or the field cannot be edited ...
    {
      continue;
    }

    // let editor know we've accepted the changes
    w->changesCommitted();

    newAttributeValues.insert( wIt.key(), w->currentValue() );
  }

  if ( newAttributeValues.isEmpty() )
  {
    //nothing to change
    return true;
  }

#if 0
  // prompt for save
  int res = QMessageBox::information( this, tr( "Multiedit Attributes" ),
                                      tr( "Edits will be applied to all selected features." ), QMessageBox::Ok | QMessageBox::Cancel );
  if ( res != QMessageBox::Ok )
  {
    resetMultiEdit();
    return false;
  }
#endif

  mLayer->beginEditCommand( tr( "Updated multiple feature attributes" ) );

  bool success = true;

  Q_FOREACH ( QgsFeatureId fid, mMultiEditFeatureIds )
  {
    QgsAttributeMap::const_iterator aIt = newAttributeValues.constBegin();
    for ( ; aIt != newAttributeValues.constEnd(); ++aIt )
    {
      success &= mLayer->changeAttributeValue( fid, aIt.key(), aIt.value() );
    }
  }

  clearMultiEditMessages();
  if ( success )
  {
    mLayer->endEditCommand();
    mLayer->triggerRepaint();
    mMultiEditMessageBarItem = new QgsMessageBarItem( tr( "Attribute changes for multiple features applied." ), Qgis::Success, messageTimeout() );
  }
  else
  {
    mLayer->destroyEditCommand();
    mMultiEditMessageBarItem = new QgsMessageBarItem( tr( "Changes could not be applied." ), Qgis::Warning, messageTimeout() );
  }

  if ( !mButtonBox->isVisible() )
    mMessageBar->pushItem( mMultiEditMessageBarItem );
  return success;
}

bool QgsAttributeForm::save()
{
  if ( mIsSaving )
    return true;

  for ( QgsWidgetWrapper *wrapper : qgis::as_const( mWidgets ) )
  {
    wrapper->notifyAboutToSave();
  }

  // only do the dirty checks when editing an existing feature - for new
  // features we need to add them even if the attributes are unchanged from the initial
  // default values
  switch ( mMode )
  {
    case QgsAttributeEditorContext::SingleEditMode:
    case QgsAttributeEditorContext::IdentifyMode:
    case QgsAttributeEditorContext::MultiEditMode:
      if ( !mDirty )
        return true;
      break;

    case QgsAttributeEditorContext::AddFeatureMode:
    case QgsAttributeEditorContext::SearchMode:
    case QgsAttributeEditorContext::AggregateSearchMode:
      break;
  }

  mIsSaving = true;

  bool success = true;

  emit beforeSave( success );

  // Somebody wants to prevent this form from saving
  if ( !success )
    return false;

  switch ( mMode )
  {
    case QgsAttributeEditorContext::SingleEditMode:
    case QgsAttributeEditorContext::IdentifyMode:
    case QgsAttributeEditorContext::AddFeatureMode:
    case QgsAttributeEditorContext::SearchMode:
    case QgsAttributeEditorContext::AggregateSearchMode:
      success = saveEdits();
      break;

    case QgsAttributeEditorContext::MultiEditMode:
      success = saveMultiEdits();
      break;
  }

  mIsSaving = false;
  mUnsavedMultiEditChanges = false;
  mDirty = false;

  return success;
}

void QgsAttributeForm::resetValues()
{
  mValuesInitialized = false;
  Q_FOREACH ( QgsWidgetWrapper *ww, mWidgets )
  {
    ww->setFeature( mFeature );
  }
  mValuesInitialized = true;
  mDirty = false;
}

void QgsAttributeForm::resetSearch()
{
  Q_FOREACH ( QgsAttributeFormEditorWidget *w, findChildren<  QgsAttributeFormEditorWidget * >() )
  {
    w->resetSearch();
  }
}

void QgsAttributeForm::clearMultiEditMessages()
{
  if ( mMultiEditUnsavedMessageBarItem )
  {
    if ( !mButtonBox->isVisible() )
      mMessageBar->popWidget( mMultiEditUnsavedMessageBarItem );
    mMultiEditUnsavedMessageBarItem = nullptr;
  }
  if ( mMultiEditMessageBarItem )
  {
    if ( !mButtonBox->isVisible() )
      mMessageBar->popWidget( mMultiEditMessageBarItem );
    mMultiEditMessageBarItem = nullptr;
  }
}

QString QgsAttributeForm::createFilterExpression() const
{
  QStringList filters;
  for ( QgsAttributeFormWidget *w : qgis::as_const( mFormWidgets ) )
  {
    QString filter = w->currentFilterExpression();
    if ( !filter.isEmpty() )
      filters << filter;
  }

  if ( filters.isEmpty() )
    return QString();

  QString filter = filters.join( QStringLiteral( ") AND (" ) ).prepend( '(' ).append( ')' );
  return filter;
}


void QgsAttributeForm::onAttributeChanged( const QVariant &value )
{
  QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( sender() );
  Q_ASSERT( eww );

  bool signalEmitted = false;

  if ( mValuesInitialized )
    mDirty = true;

  switch ( mMode )
  {
    case QgsAttributeEditorContext::SingleEditMode:
    case QgsAttributeEditorContext::IdentifyMode:
    case QgsAttributeEditorContext::AddFeatureMode:
    {
      Q_NOWARN_DEPRECATED_PUSH
      emit attributeChanged( eww->field().name(), value );
      Q_NOWARN_DEPRECATED_PUSH
      emit widgetValueChanged( eww->field().name(), value, !mIsSettingFeature );

      signalEmitted = true;

      updateJoinedFields( *eww );

      break;
    }
    case QgsAttributeEditorContext::MultiEditMode:
    {
      if ( !mIsSettingMultiEditFeatures )
      {
        mUnsavedMultiEditChanges = true;

        QLabel *msgLabel = new QLabel( tr( "Unsaved multiedit changes: <a href=\"#apply\">apply changes</a> or <a href=\"#reset\">reset changes</a>." ), mMessageBar );
        msgLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        msgLabel->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
        connect( msgLabel, &QLabel::linkActivated, this, &QgsAttributeForm::multiEditMessageClicked );
        clearMultiEditMessages();

        mMultiEditUnsavedMessageBarItem = new QgsMessageBarItem( msgLabel, Qgis::Warning );
        if ( !mButtonBox->isVisible() )
          mMessageBar->pushItem( mMultiEditUnsavedMessageBarItem );
      }
      break;
    }
    case QgsAttributeEditorContext::SearchMode:
    case QgsAttributeEditorContext::AggregateSearchMode:
      //nothing to do
      break;
  }

  updateConstraints( eww );

  if ( !signalEmitted )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit attributeChanged( eww->field().name(), value );
    Q_NOWARN_DEPRECATED_PUSH
    emit widgetValueChanged( eww->field().name(), value, !mIsSettingFeature );
  }
}

void QgsAttributeForm::updateAllConstraints()
{
  Q_FOREACH ( QgsWidgetWrapper *ww, mWidgets )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( eww )
      updateConstraints( eww );
  }
}

void QgsAttributeForm::updateConstraints( QgsEditorWidgetWrapper *eww )
{
  // get the current feature set in the form
  QgsFeature ft;
  if ( currentFormFeature( ft ) )
  {
    // if the layer is NOT being edited then we only check layer based constraints, and not
    // any constraints enforced by the provider. Because:
    // 1. we want to keep browsing features nice and responsive. It's nice to give feedback as to whether
    // the value checks out, but not if it's too slow to do so. Some constraints (e.g., unique) can be
    // expensive to test. A user can freely remove a layer-based constraint if it proves to be too slow
    // to test, but they are unlikely to have any control over provider-side constraints
    // 2. the provider has already accepted the value, so presumably it doesn't violate the constraint
    // and there's no point rechecking!

    // update eww constraint
    updateConstraint( ft, eww );

    // update eww dependencies constraint
    const QList<QgsEditorWidgetWrapper *> deps = constraintDependencies( eww );

    for ( QgsEditorWidgetWrapper *depsEww : deps )
      updateConstraint( ft, depsEww );

    // sync OK button status
    synchronizeEnabledState();

    mExpressionContext.setFeature( ft );

    mExpressionContext << QgsExpressionContextUtils::formScope( ft, mContext.attributeFormModeString() );

    // Recheck visibility for all containers which are controlled by this value
    const QVector<ContainerInformation *> infos = mContainerInformationDependency.value( eww->field().name() );
    for ( ContainerInformation *info : infos )
    {
      info->apply( &mExpressionContext );
    }
  }
}

void QgsAttributeForm::updateContainersVisibility()
{
  mExpressionContext << QgsExpressionContextUtils::formScope( QgsFeature( mFeature ), mContext.attributeFormModeString() );

  const QVector<ContainerInformation *> infos = mContainerVisibilityInformation;

  for ( ContainerInformation *info : infos )
  {
    info->apply( &mExpressionContext );
  }

  //and update the constraints
  updateAllConstraints();
}

void QgsAttributeForm::updateConstraint( const QgsFeature &ft, QgsEditorWidgetWrapper *eww )
{
  QgsFieldConstraints::ConstraintOrigin constraintOrigin = mLayer->isEditable() ? QgsFieldConstraints::ConstraintOriginNotSet : QgsFieldConstraints::ConstraintOriginLayer;

  if ( eww->layer()->fields().fieldOrigin( eww->fieldIdx() ) == QgsFields::OriginJoin )
  {
    int srcFieldIdx;
    const QgsVectorLayerJoinInfo *info = eww->layer()->joinBuffer()->joinForFieldIndex( eww->fieldIdx(), eww->layer()->fields(), srcFieldIdx );

    if ( info && info->joinLayer() && info->isDynamicFormEnabled() )
    {
      if ( mJoinedFeatures.contains( info ) )
      {
        eww->updateConstraint( info->joinLayer(), srcFieldIdx, mJoinedFeatures[info], constraintOrigin );
        return;
      }
      else // if we are here, it means there's not joined field for this feature
      {
        eww->updateConstraint( QgsFeature() );
        return;
      }
    }
  }

  // default constraint update
  eww->updateConstraint( ft, constraintOrigin );
}

bool QgsAttributeForm::currentFormFeature( QgsFeature &feature )
{
  bool rc = true;
  feature = QgsFeature( mFeature );
  QgsAttributes dst = feature.attributes();

  for ( QgsWidgetWrapper *ww : qgis::as_const( mWidgets ) )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );

    if ( !eww )
      continue;

    if ( dst.count() > eww->fieldIdx() )
    {
      QVariant dstVar = dst.at( eww->fieldIdx() );
      QVariant srcVar = eww->value();
      // need to check dstVar.isNull() != srcVar.isNull()
      // otherwise if dstVar=NULL and scrVar=0, then dstVar = srcVar
      if ( ( dstVar != srcVar || dstVar.isNull() != srcVar.isNull() ) && srcVar.isValid() )
        dst[eww->fieldIdx()] = srcVar;
    }
    else
    {
      rc = false;
      break;
    }
  }

  feature.setAttributes( dst );

  return rc;
}


void QgsAttributeForm::registerContainerInformation( QgsAttributeForm::ContainerInformation *info )
{
  mContainerVisibilityInformation.append( info );

  const QSet<QString> referencedColumns = info->expression.referencedColumns();

  for ( const QString &col : referencedColumns )
  {
    mContainerInformationDependency[ col ].append( info );
  }
}

bool QgsAttributeForm::currentFormValidConstraints( QStringList &invalidFields, QStringList &descriptions )
{
  bool valid( true );

  for ( QgsWidgetWrapper *ww : qgis::as_const( mWidgets ) )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( eww )
    {
      if ( ! eww->isValidConstraint() )
      {
        invalidFields.append( eww->field().displayName() );

        descriptions.append( eww->constraintFailureReason() );

        if ( eww->isBlockingCommit() )
          valid = false; // continue to get all invalid fields
      }
    }
  }

  return valid;
}

void QgsAttributeForm::onAttributeAdded( int idx )
{
  mPreventFeatureRefresh = false;
  if ( mFeature.isValid() )
  {
    QgsAttributes attrs = mFeature.attributes();
    attrs.insert( idx, QVariant( layer()->fields().at( idx ).type() ) );
    mFeature.setFields( layer()->fields() );
    mFeature.setAttributes( attrs );
  }
  init();
  setFeature( mFeature );
}

void QgsAttributeForm::onAttributeDeleted( int idx )
{
  mPreventFeatureRefresh = false;
  if ( mFeature.isValid() )
  {
    QgsAttributes attrs = mFeature.attributes();
    attrs.remove( idx );
    mFeature.setFields( layer()->fields() );
    mFeature.setAttributes( attrs );
  }
  init();
  setFeature( mFeature );
}

void QgsAttributeForm::onUpdatedFields()
{
  mPreventFeatureRefresh = false;
  if ( mFeature.isValid() )
  {
    QgsAttributes attrs( layer()->fields().size() );
    for ( int i = 0; i < layer()->fields().size(); i++ )
    {
      int idx = mFeature.fields().indexFromName( layer()->fields().at( i ).name() );
      if ( idx != -1 )
      {
        attrs[i] = mFeature.attributes().at( idx );
        if ( mFeature.attributes().at( idx ).type() != layer()->fields().at( i ).type() )
        {
          attrs[i].convert( layer()->fields().at( i ).type() );
        }
      }
      else
      {
        attrs[i] = QVariant( layer()->fields().at( i ).type() );
      }
    }
    mFeature.setFields( layer()->fields() );
    mFeature.setAttributes( attrs );
  }
  init();
  setFeature( mFeature );
}

void QgsAttributeForm::onConstraintStatusChanged( const QString &constraint,
    const QString &description, const QString &err, QgsEditorWidgetWrapper::ConstraintResult result )
{
  QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( sender() );
  Q_ASSERT( eww );

  QgsAttributeFormEditorWidget *formEditorWidget = mFormEditorWidgets.value( eww->fieldIdx() );

  if ( formEditorWidget )
    formEditorWidget->setConstraintStatus( constraint, description, err, result );
}

QList<QgsEditorWidgetWrapper *> QgsAttributeForm::constraintDependencies( QgsEditorWidgetWrapper *w )
{
  QList<QgsEditorWidgetWrapper *> wDeps;
  QString name = w->field().name();

  // for each widget in the current form
  for ( QgsWidgetWrapper *ww : qgis::as_const( mWidgets ) )
  {
    // get the wrapper
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( eww )
    {
      // compare name to not compare w to itself
      QString ewwName = eww->field().name();
      if ( name != ewwName )
      {
        // get expression and referencedColumns
        QgsExpression expr = eww->layer()->fields().at( eww->fieldIdx() ).constraints().constraintExpression();

        const auto referencedColumns = expr.referencedColumns();

        for ( const QString &colName : referencedColumns )
        {
          if ( name == colName )
          {
            wDeps.append( eww );
            break;
          }
        }
      }
    }
  }

  return wDeps;
}

QgsRelationWidgetWrapper *QgsAttributeForm::setupRelationWidgetWrapper( const QgsRelation &rel, const QgsAttributeEditorContext &context )
{
  QgsRelationWidgetWrapper *rww = new QgsRelationWidgetWrapper( mLayer, rel, nullptr, this );
  const QVariantMap config = mLayer->editFormConfig().widgetConfig( rel.id() );
  rww->setConfig( config );
  rww->setContext( context );

  return rww;
}

void QgsAttributeForm::preventFeatureRefresh()
{
  mPreventFeatureRefresh = true;
}

void QgsAttributeForm::refreshFeature()
{
  if ( mPreventFeatureRefresh || mLayer->isEditable() || !mFeature.isValid() )
    return;

  // reload feature if layer changed although not editable
  // (datasource probably changed bypassing QgsVectorLayer)
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeature.id() ) ).nextFeature( mFeature ) )
    return;

  init();
  setFeature( mFeature );
}

void QgsAttributeForm::synchronizeEnabledState()
{
  bool isEditable = ( mFeature.isValid()
                      || mMode == QgsAttributeEditorContext::AddFeatureMode
                      || mMode == QgsAttributeEditorContext::MultiEditMode ) && mLayer->isEditable();

  for ( QgsWidgetWrapper *ww : qgis::as_const( mWidgets ) )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( eww )
    {
      QgsAttributeFormEditorWidget *formWidget = mFormEditorWidgets.value( eww->fieldIdx() );

      if ( formWidget )
        formWidget->setConstraintResultVisible( isEditable );

      eww->setConstraintResultVisible( isEditable );

      bool enabled = isEditable && fieldIsEditable( eww->fieldIdx() );
      ww->setEnabled( enabled );

      updateIcon( eww );
    }
  }

  if ( mMode != QgsAttributeEditorContext::SearchMode )
  {
    QStringList invalidFields, descriptions;
    bool validConstraint = currentFormValidConstraints( invalidFields, descriptions );

    isEditable = isEditable & validConstraint;
  }

  // change OK button status
  QPushButton *okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
    okButton->setEnabled( isEditable );
}

void QgsAttributeForm::init()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  // Cleanup of any previously shown widget, we start from scratch
  QWidget *formWidget = nullptr;

  bool buttonBoxVisible = true;
  // Cleanup button box but preserve visibility
  if ( mButtonBox )
  {
    buttonBoxVisible = mButtonBox->isVisible();
    delete mButtonBox;
    mButtonBox = nullptr;
  }

  if ( mSearchButtonBox )
  {
    delete mSearchButtonBox;
    mSearchButtonBox = nullptr;
  }

  qDeleteAll( mWidgets );
  mWidgets.clear();

  while ( QWidget *w = this->findChild<QWidget *>() )
  {
    delete w;
  }
  delete layout();

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setMargin( 0 );
  vl->setContentsMargins( 0, 0, 0, 0 );
  mMessageBar = new QgsMessageBar( this );
  mMessageBar->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
  vl->addWidget( mMessageBar );

  setLayout( vl );

  // Get a layout
  QGridLayout *layout = new QGridLayout();
  QWidget *container = new QWidget();
  container->setLayout( layout );
  vl->addWidget( container );

  mFormEditorWidgets.clear();
  mFormWidgets.clear();

  // a bar to warn the user with non-blocking messages
  setContentsMargins( 0, 0, 0, 0 );

  // Try to load Ui-File for layout
  if ( mContext.allowCustomUi() && mLayer->editFormConfig().layout() == QgsEditFormConfig::UiFileLayout &&
       !mLayer->editFormConfig().uiForm().isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "loading form: %1" ).arg( mLayer->editFormConfig().uiForm() ) );
    const QString path = mLayer->editFormConfig().uiForm();
    QFile *file = QgsApplication::instance()->networkContentFetcherRegistry()->localFile( path );
    if ( file && file->open( QFile::ReadOnly ) )
    {
      QUiLoader loader;

      QFileInfo fi( file->fileName() );
      loader.setWorkingDirectory( fi.dir() );
      formWidget = loader.load( file, this );
      if ( formWidget )
      {
        formWidget->setWindowFlags( Qt::Widget );
        layout->addWidget( formWidget );
        formWidget->show();
        file->close();
        mButtonBox = findChild<QDialogButtonBox *>();
        createWrappers();

        formWidget->installEventFilter( this );
      }
    }
  }

  QgsTabWidget *tabWidget = nullptr;

  // Tab layout
  if ( !formWidget && mLayer->editFormConfig().layout() == QgsEditFormConfig::TabLayout )
  {
    int row = 0;
    int column = 0;
    int columnCount = 1;

    const QList<QgsAttributeEditorElement *> tabs = mLayer->editFormConfig().tabs();

    for ( QgsAttributeEditorElement *widgDef : tabs )
    {
      if ( widgDef->type() == QgsAttributeEditorElement::AeTypeContainer )
      {
        QgsAttributeEditorContainer *containerDef = dynamic_cast<QgsAttributeEditorContainer *>( widgDef );
        if ( !containerDef )
          continue;

        if ( containerDef->isGroupBox() )
        {
          tabWidget = nullptr;
          WidgetInfo widgetInfo = createWidgetFromDef( widgDef, formWidget, mLayer, mContext );
          layout->addWidget( widgetInfo.widget, row, column, 1, 2 );
          if ( containerDef->visibilityExpression().enabled() )
          {
            registerContainerInformation( new ContainerInformation( widgetInfo.widget, containerDef->visibilityExpression().data() ) );
          }
          column += 2;
        }
        else
        {
          if ( !tabWidget )
          {
            tabWidget = new QgsTabWidget();
            layout->addWidget( tabWidget, row, column, 1, 2 );
            column += 2;
          }

          QWidget *tabPage = new QWidget( tabWidget );

          tabWidget->addTab( tabPage, widgDef->name() );

          if ( containerDef->visibilityExpression().enabled() )
          {
            registerContainerInformation( new ContainerInformation( tabWidget, tabPage, containerDef->visibilityExpression().data() ) );
          }
          QGridLayout *tabPageLayout = new QGridLayout();
          tabPage->setLayout( tabPageLayout );

          WidgetInfo widgetInfo = createWidgetFromDef( widgDef, tabPage, mLayer, mContext );
          tabPageLayout->addWidget( widgetInfo.widget );
        }
      }
      else
      {
        tabWidget = nullptr;
        WidgetInfo widgetInfo = createWidgetFromDef( widgDef, container, mLayer, mContext );
        QLabel *label = new QLabel( widgetInfo.labelText );
        label->setToolTip( widgetInfo.toolTip );
        if ( columnCount > 1 && !widgetInfo.labelOnTop )
        {
          label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
        }

        label->setBuddy( widgetInfo.widget );

        if ( !widgetInfo.showLabel )
        {
          QVBoxLayout *c = new QVBoxLayout();
          label->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
          c->addWidget( widgetInfo.widget );
          layout->addLayout( c, row, column, 1, 2 );
          column += 2;
        }
        else if ( widgetInfo.labelOnTop )
        {
          QVBoxLayout *c = new QVBoxLayout();
          label->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
          c->addWidget( label );
          c->addWidget( widgetInfo.widget );
          layout->addLayout( c, row, column, 1, 2 );
          column += 2;
        }
        else
        {
          layout->addWidget( label, row, column++ );
          layout->addWidget( widgetInfo.widget, row, column++ );
        }
      }

      if ( column >= columnCount * 2 )
      {
        column = 0;
        row += 1;
      }
    }
    formWidget = container;
  }

  // Autogenerate Layout
  // If there is still no layout loaded (defined as autogenerate or other methods failed)
  mIconMap.clear();

  if ( !formWidget )
  {
    formWidget = new QWidget( this );
    QGridLayout *gridLayout = new QGridLayout( formWidget );
    formWidget->setLayout( gridLayout );

    if ( mContext.formMode() != QgsAttributeEditorContext::Embed )
    {
      // put the form into a scroll area to nicely handle cases with lots of attributes
      QgsScrollArea *scrollArea = new QgsScrollArea( this );
      scrollArea->setWidget( formWidget );
      scrollArea->setWidgetResizable( true );
      scrollArea->setFrameShape( QFrame::NoFrame );
      scrollArea->setFrameShadow( QFrame::Plain );
      scrollArea->setFocusProxy( this );
      layout->addWidget( scrollArea );
    }
    else
    {
      layout->addWidget( formWidget );
    }

    int row = 0;

    const QgsFields fields = mLayer->fields();

    for ( const QgsField &field : fields )
    {
      int idx = fields.lookupField( field.name() );
      if ( idx < 0 )
        continue;

      //show attribute alias if available
      QString fieldName = mLayer->attributeDisplayName( idx );
      QString labelText = fieldName;
      labelText.replace( '&', QStringLiteral( "&&" ) ); // need to escape '&' or they'll be replace by _ in the label text

      const QgsEditorWidgetSetup widgetSetup = QgsGui::editorWidgetRegistry()->findBest( mLayer, field.name() );

      if ( widgetSetup.type() == QLatin1String( "Hidden" ) )
        continue;

      bool labelOnTop = mLayer->editFormConfig().labelOnTop( idx );

      // This will also create the widget
      QLabel *l = new QLabel( labelText );
      l->setToolTip( QStringLiteral( "<b>%1</b><p>%2</p>" ).arg( fieldName, field.comment() ) );
      QSvgWidget *i = new QSvgWidget();
      i->setFixedSize( 18, 18 );

      QgsEditorWidgetWrapper *eww = QgsGui::editorWidgetRegistry()->create( widgetSetup.type(), mLayer, idx, widgetSetup.config(), nullptr, this, mContext );

      QWidget *w = nullptr;
      if ( eww )
      {
        QgsAttributeFormEditorWidget *formWidget = new QgsAttributeFormEditorWidget( eww, widgetSetup.type(), this );
        w = formWidget;
        mFormEditorWidgets.insert( idx, formWidget );
        mFormWidgets.append( formWidget );
        formWidget->createSearchWidgetWrappers( mContext );

        l->setBuddy( eww->widget() );
      }
      else
      {
        w = new QLabel( QStringLiteral( "<p style=\"color: red; font-style: italic;\">%1</p>" ).arg( tr( "Failed to create widget with type '%1'" ).arg( widgetSetup.type() ) ) );
      }


      if ( w )
        w->setObjectName( field.name() );

      if ( eww )
      {
        addWidgetWrapper( eww );
        mIconMap[eww->widget()] = i;
      }

      if ( labelOnTop )
      {
        gridLayout->addWidget( l, row++, 0, 1, 2 );
        gridLayout->addWidget( w, row++, 0, 1, 2 );
        gridLayout->addWidget( i, row++, 0, 1, 2 );
      }
      else
      {
        gridLayout->addWidget( l, row, 0 );
        gridLayout->addWidget( w, row, 1 );
        gridLayout->addWidget( i, row++, 2 );
      }
    }

    const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencedRelations( mLayer );
    for ( const QgsRelation &rel : relations )
    {
      QgsRelationWidgetWrapper *rww = setupRelationWidgetWrapper( rel, mContext );

      QgsAttributeFormRelationEditorWidget *formWidget = new QgsAttributeFormRelationEditorWidget( rww, this );
      formWidget->createSearchWidgetWrappers( mContext );
      gridLayout->addWidget( formWidget, row++, 0, 1, 2 );

      mWidgets.append( rww );
      mFormWidgets.append( formWidget );
    }

    if ( QgsProject::instance()->relationManager()->referencedRelations( mLayer ).isEmpty() )
    {
      QSpacerItem *spacerItem = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
      gridLayout->addItem( spacerItem, row, 0 );
      gridLayout->setRowStretch( row, 1 );
      row++;
    }
  }

  if ( !mButtonBox )
  {
    mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    mButtonBox->setObjectName( QStringLiteral( "buttonBox" ) );
    layout->addWidget( mButtonBox, layout->rowCount(), 0, 1, layout->columnCount() );
  }
  mButtonBox->setVisible( buttonBoxVisible );

  if ( !mSearchButtonBox )
  {
    mSearchButtonBox = new QWidget();
    QHBoxLayout *boxLayout = new QHBoxLayout();
    boxLayout->setMargin( 0 );
    boxLayout->setContentsMargins( 0, 0, 0, 0 );
    mSearchButtonBox->setLayout( boxLayout );
    mSearchButtonBox->setObjectName( QStringLiteral( "searchButtonBox" ) );

    QPushButton *clearButton = new QPushButton( tr( "&Reset Form" ), mSearchButtonBox );
    connect( clearButton, &QPushButton::clicked, this, &QgsAttributeForm::resetSearch );
    boxLayout->addWidget( clearButton );
    boxLayout->addStretch( 1 );

    QPushButton *flashButton = new QPushButton();
    flashButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    flashButton->setText( tr( "&Flash Features" ) );
    connect( flashButton, &QToolButton::clicked, this, &QgsAttributeForm::searchFlash );
    boxLayout->addWidget( flashButton );

    QPushButton *zoomButton = new QPushButton();
    zoomButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    zoomButton->setText( tr( "&Zoom to Features" ) );
    connect( zoomButton, &QToolButton::clicked, this, &QgsAttributeForm::searchZoomTo );
    boxLayout->addWidget( zoomButton );

    QToolButton *selectButton = new QToolButton();
    selectButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    selectButton->setText( tr( "&Select Features" ) );
    selectButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFormSelect.svg" ) ) );
    selectButton->setPopupMode( QToolButton::MenuButtonPopup );
    selectButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    connect( selectButton, &QToolButton::clicked, this, &QgsAttributeForm::searchSetSelection );
    QMenu *selectMenu = new QMenu( selectButton );
    QAction *selectAction = new QAction( tr( "Select Features" ), selectMenu );
    selectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFormSelect.svg" ) ) );
    connect( selectAction, &QAction::triggered, this, &QgsAttributeForm::searchSetSelection );
    selectMenu->addAction( selectAction );
    QAction *addSelectAction = new QAction( tr( "Add to Current Selection" ), selectMenu );
    addSelectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSelectAdd.svg" ) ) );
    connect( addSelectAction, &QAction::triggered, this, &QgsAttributeForm::searchAddToSelection );
    selectMenu->addAction( addSelectAction );
    QAction *deselectAction = new QAction( tr( "Remove from Current Selection" ), selectMenu );
    deselectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSelectRemove.svg" ) ) );
    connect( deselectAction, &QAction::triggered, this, &QgsAttributeForm::searchRemoveFromSelection );
    selectMenu->addAction( deselectAction );
    QAction *filterSelectAction = new QAction( tr( "Filter Current Selection" ), selectMenu );
    filterSelectAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSelectIntersect.svg" ) ) );
    connect( filterSelectAction, &QAction::triggered, this, &QgsAttributeForm::searchIntersectSelection );
    selectMenu->addAction( filterSelectAction );
    selectButton->setMenu( selectMenu );
    boxLayout->addWidget( selectButton );

    if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    {
      QToolButton *filterButton = new QToolButton();
      filterButton->setText( tr( "Filter features" ) );
      filterButton->setPopupMode( QToolButton::MenuButtonPopup );
      filterButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
      connect( filterButton, &QToolButton::clicked, this, &QgsAttributeForm::filterTriggered );
      QMenu *filterMenu = new QMenu( filterButton );
      QAction *filterAndAction = new QAction( tr( "Filter Within (\"AND\")" ), filterMenu );
      connect( filterAndAction, &QAction::triggered, this, &QgsAttributeForm::filterAndTriggered );
      filterMenu->addAction( filterAndAction );
      QAction *filterOrAction = new QAction( tr( "Extend Filter (\"OR\")" ), filterMenu );
      connect( filterOrAction, &QAction::triggered, this, &QgsAttributeForm::filterOrTriggered );
      filterMenu->addAction( filterOrAction );
      filterButton->setMenu( filterMenu );
      boxLayout->addWidget( filterButton );
    }
    else
    {
      QPushButton *closeButton = new QPushButton( tr( "Close" ), mSearchButtonBox );
      connect( closeButton, &QPushButton::clicked, this, &QgsAttributeForm::closed );
      closeButton->setShortcut( Qt::Key_Escape );
      boxLayout->addWidget( closeButton );
    }

    layout->addWidget( mSearchButtonBox );
  }
  mSearchButtonBox->setVisible( mMode == QgsAttributeEditorContext::SearchMode );

  afterWidgetInit();

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsAttributeForm::save );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsAttributeForm::resetValues );

  connect( mLayer, &QgsVectorLayer::editingStarted, this, &QgsAttributeForm::synchronizeEnabledState );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsAttributeForm::synchronizeEnabledState );

  // This triggers a refresh of the form widget and gives a chance to re-format the
  // value to those widgets that have a different representation when in edit mode
  connect( mLayer, &QgsVectorLayer::editingStarted, this, &QgsAttributeForm::resetValues );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsAttributeForm::resetValues );


  Q_FOREACH ( QgsAttributeFormInterface *iface, mInterfaces )
  {
    iface->initForm();
  }

  if ( mContext.formMode() == QgsAttributeEditorContext::Embed || mMode == QgsAttributeEditorContext::SearchMode )
  {
    hideButtonBox();
  }

  QApplication::restoreOverrideCursor();
}

void QgsAttributeForm::cleanPython()
{
  if ( !mPyFormVarName.isNull() )
  {
    QString expr = QStringLiteral( "if '%1' in locals(): del %1\n" ).arg( mPyFormVarName );
    QgsPythonRunner::run( expr );
  }
}

void QgsAttributeForm::initPython()
{
  cleanPython();

  // Init Python, if init function is not empty and the combo indicates
  // the source for the function code
  if ( !mLayer->editFormConfig().initFunction().isEmpty()
       && mLayer->editFormConfig().initCodeSource() != QgsEditFormConfig::CodeSourceNone )
  {

    QString initFunction = mLayer->editFormConfig().initFunction();
    QString initFilePath = mLayer->editFormConfig().initFilePath();
    QString initCode;

    switch ( mLayer->editFormConfig().initCodeSource() )
    {
      case QgsEditFormConfig::CodeSourceFile:
        if ( ! initFilePath.isEmpty() )
        {
          QFile inputFile( initFilePath );

          if ( inputFile.open( QFile::ReadOnly ) )
          {
            // Read it into a string
            QTextStream inf( &inputFile );
            initCode = inf.readAll();
            inputFile.close();
          }
          else // The file couldn't be opened
          {
            QgsLogger::warning( QStringLiteral( "The external python file path %1 could not be opened!" ).arg( initFilePath ) );
          }
        }
        else
        {
          QgsLogger::warning( QStringLiteral( "The external python file path is empty!" ) );
        }
        break;

      case QgsEditFormConfig::CodeSourceDialog:
        initCode = mLayer->editFormConfig().initCode();
        if ( initCode.isEmpty() )
        {
          QgsLogger::warning( QStringLiteral( "The python code provided in the dialog is empty!" ) );
        }
        break;

      case QgsEditFormConfig::CodeSourceEnvironment:
      case QgsEditFormConfig::CodeSourceNone:
      default:
        // Nothing to do: the function code should be already in the environment
        break;
    }

    // If we have a function code, run it
    if ( ! initCode.isEmpty() )
    {
      QgsPythonRunner::run( initCode );
    }

    QgsPythonRunner::run( QStringLiteral( "import inspect" ) );
    QString numArgs;

    // Check for eval result
    if ( QgsPythonRunner::eval( QStringLiteral( "len(inspect.getargspec(%1)[0])" ).arg( initFunction ), numArgs ) )
    {
      static int sFormId = 0;
      mPyFormVarName = QStringLiteral( "_qgis_featureform_%1_%2" ).arg( mFormNr ).arg( sFormId++ );

      QString form = QStringLiteral( "%1 = sip.wrapinstance( %2, qgis.gui.QgsAttributeForm )" )
                     .arg( mPyFormVarName )
                     .arg( ( quint64 ) this );

      QgsPythonRunner::run( form );

      QgsDebugMsg( QStringLiteral( "running featureForm init: %1" ).arg( mPyFormVarName ) );

      // Legacy
      if ( numArgs == QLatin1String( "3" ) )
      {
        addInterface( new QgsAttributeFormLegacyInterface( initFunction, mPyFormVarName, this ) );
      }
      else
      {
        // If we get here, it means that the function doesn't accept three arguments
        QMessageBox msgBox;
        msgBox.setText( tr( "The python init function (<code>%1</code>) does not accept three arguments as expected!<br>Please check the function name in the <b>Fields</b> tab of the layer properties." ).arg( initFunction ) );
        msgBox.exec();
#if 0
        QString expr = QString( "%1(%2)" )
                       .arg( mLayer->editFormInit() )
                       .arg( mPyFormVarName );
        QgsAttributeFormInterface *iface = QgsPythonRunner::evalToSipObject<QgsAttributeFormInterface *>( expr, "QgsAttributeFormInterface" );
        if ( iface )
          addInterface( iface );
#endif
      }
    }
    else
    {
      // If we get here, it means that inspect couldn't find the function
      QMessageBox msgBox;
      msgBox.setText( tr( "The python init function (<code>%1</code>) could not be found!<br>Please check the function name in the <b>Fields</b> tab of the layer properties." ).arg( initFunction ) );
      msgBox.exec();
    }
  }
}

QgsAttributeForm::WidgetInfo QgsAttributeForm::createWidgetFromDef( const QgsAttributeEditorElement *widgetDef, QWidget *parent, QgsVectorLayer *vl, QgsAttributeEditorContext &context )
{
  WidgetInfo newWidgetInfo;

  switch ( widgetDef->type() )
  {
    case QgsAttributeEditorElement::AeTypeField:
    {
      const QgsAttributeEditorField *fieldDef = dynamic_cast<const QgsAttributeEditorField *>( widgetDef );
      if ( !fieldDef )
        break;

      const QgsFields fields = vl->fields();
      int fldIdx = fields.lookupField( fieldDef->name() );
      if ( fldIdx < fields.count() && fldIdx >= 0 )
      {
        const QgsEditorWidgetSetup widgetSetup = QgsGui::editorWidgetRegistry()->findBest( mLayer, fieldDef->name() );

        QgsEditorWidgetWrapper *eww = QgsGui::editorWidgetRegistry()->create( widgetSetup.type(), mLayer, fldIdx, widgetSetup.config(), nullptr, this, mContext );
        QgsAttributeFormEditorWidget *formWidget = new QgsAttributeFormEditorWidget( eww, widgetSetup.type(), this );
        mFormEditorWidgets.insert( fldIdx, formWidget );
        mFormWidgets.append( formWidget );

        formWidget->createSearchWidgetWrappers( mContext );

        newWidgetInfo.widget = formWidget;
        addWidgetWrapper( eww );

        newWidgetInfo.widget->setObjectName( fields.at( fldIdx ).name() );
        newWidgetInfo.hint = fields.at( fldIdx ).comment();
      }

      newWidgetInfo.labelOnTop = mLayer->editFormConfig().labelOnTop( fldIdx );
      newWidgetInfo.labelText = mLayer->attributeDisplayName( fldIdx );
      newWidgetInfo.labelText.replace( '&', QStringLiteral( "&&" ) ); // need to escape '&' or they'll be replace by _ in the label text
      newWidgetInfo.toolTip = QStringLiteral( "<b>%1</b><p>%2</p>" ).arg( mLayer->attributeDisplayName( fldIdx ), newWidgetInfo.hint );
      newWidgetInfo.showLabel = widgetDef->showLabel();

      break;
    }

    case QgsAttributeEditorElement::AeTypeRelation:
    {
      const QgsAttributeEditorRelation *relDef = static_cast<const QgsAttributeEditorRelation *>( widgetDef );

      QgsRelationWidgetWrapper *rww = setupRelationWidgetWrapper( relDef->relation(), context );

      rww->setShowLabel( relDef->showLabel() );
      rww->setShowLinkButton( relDef->showLinkButton() );
      rww->setShowUnlinkButton( relDef->showUnlinkButton() );

      QgsAttributeFormRelationEditorWidget *formWidget = new QgsAttributeFormRelationEditorWidget( rww, this );
      formWidget->createSearchWidgetWrappers( mContext );

      mWidgets.append( rww );
      mFormWidgets.append( formWidget );

      newWidgetInfo.widget = formWidget;
      newWidgetInfo.labelText = QString();
      newWidgetInfo.labelOnTop = true;
      break;
    }

    case QgsAttributeEditorElement::AeTypeContainer:
    {
      const QgsAttributeEditorContainer *container = dynamic_cast<const QgsAttributeEditorContainer *>( widgetDef );
      if ( !container )
        break;

      int columnCount = container->columnCount();

      if ( columnCount <= 0 )
        columnCount = 1;

      QString widgetName;
      QWidget *myContainer = nullptr;
      if ( container->isGroupBox() )
      {
        QGroupBox *groupBox = new QGroupBox( parent );
        widgetName = QStringLiteral( "QGroupBox" );
        if ( container->showLabel() )
          groupBox->setTitle( container->name() );
        myContainer = groupBox;
        newWidgetInfo.widget = myContainer;
      }
      else
      {
        myContainer = new QWidget();

        if ( context.formMode() != QgsAttributeEditorContext::Embed )
        {
          QgsScrollArea *scrollArea = new QgsScrollArea( parent );

          scrollArea->setWidget( myContainer );
          scrollArea->setWidgetResizable( true );
          scrollArea->setFrameShape( QFrame::NoFrame );
          widgetName = QStringLiteral( "QScrollArea QWidget" );

          newWidgetInfo.widget = scrollArea;
        }
        else
        {
          newWidgetInfo.widget = myContainer;
          widgetName = QStringLiteral( "QWidget" );
        }
      }

      if ( container->backgroundColor().isValid() )
      {
        QString style {QStringLiteral( "background-color: %1;" ).arg( container->backgroundColor().name() )};
        newWidgetInfo.widget->setStyleSheet( style );
      }

      QGridLayout *gbLayout = new QGridLayout();
      myContainer->setLayout( gbLayout );

      int row = 0;
      int column = 0;

      const QList<QgsAttributeEditorElement *> children = container->children();

      for ( QgsAttributeEditorElement *childDef : children )
      {
        WidgetInfo widgetInfo = createWidgetFromDef( childDef, myContainer, vl, context );

        if ( childDef->type() == QgsAttributeEditorElement::AeTypeContainer )
        {
          QgsAttributeEditorContainer *containerDef = static_cast<QgsAttributeEditorContainer *>( childDef );
          if ( containerDef->visibilityExpression().enabled() )
          {
            registerContainerInformation( new ContainerInformation( widgetInfo.widget, containerDef->visibilityExpression().data() ) );
          }
        }

        if ( widgetInfo.labelText.isNull() )
        {
          gbLayout->addWidget( widgetInfo.widget, row, column, 1, 2 );
          column += 2;
        }
        else
        {
          QLabel *mypLabel = new QLabel( widgetInfo.labelText );
          mypLabel->setToolTip( widgetInfo.toolTip );
          if ( columnCount > 1 && !widgetInfo.labelOnTop )
          {
            mypLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
          }

          mypLabel->setBuddy( widgetInfo.widget );

          if ( widgetInfo.labelOnTop )
          {
            QVBoxLayout *c = new QVBoxLayout();
            mypLabel->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
            c->layout()->addWidget( mypLabel );
            c->layout()->addWidget( widgetInfo.widget );
            gbLayout->addLayout( c, row, column, 1, 2 );
            column += 2;
          }
          else
          {
            gbLayout->addWidget( mypLabel, row, column++ );
            gbLayout->addWidget( widgetInfo.widget, row, column++ );
          }
        }

        if ( column >= columnCount * 2 )
        {
          column = 0;
          row += 1;
        }
      }
      QWidget *spacer = new QWidget();
      spacer->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
      gbLayout->addWidget( spacer, ++row, 0 );
      gbLayout->setRowStretch( row, 1 );

      newWidgetInfo.labelText = QString();
      newWidgetInfo.labelOnTop = true;
      break;
    }

    case QgsAttributeEditorElement::AeTypeQmlElement:
    {
      const QgsAttributeEditorQmlElement *elementDef = static_cast<const QgsAttributeEditorQmlElement *>( widgetDef );

      QgsQmlWidgetWrapper *qmlWrapper = new QgsQmlWidgetWrapper( mLayer, nullptr, this );
      qmlWrapper->setQmlCode( elementDef->qmlCode() );
      qmlWrapper->setConfig( mLayer->editFormConfig().widgetConfig( elementDef->name() ) );
      context.setAttributeFormMode( mMode );
      qmlWrapper->setContext( context );

      mWidgets.append( qmlWrapper );

      newWidgetInfo.widget = qmlWrapper->widget();
      newWidgetInfo.labelText = elementDef->name();
      newWidgetInfo.labelOnTop = true;
      newWidgetInfo.showLabel = widgetDef->showLabel();
      break;
    }

    case QgsAttributeEditorElement::AeTypeHtmlElement:
    {
      const QgsAttributeEditorHtmlElement *elementDef = static_cast<const QgsAttributeEditorHtmlElement *>( widgetDef );

      QgsHtmlWidgetWrapper *htmlWrapper = new QgsHtmlWidgetWrapper( mLayer, nullptr, this );
      context.setAttributeFormMode( mMode );
      htmlWrapper->setHtmlCode( elementDef->htmlCode() );
      htmlWrapper->reinitWidget();
      htmlWrapper->setConfig( mLayer->editFormConfig().widgetConfig( elementDef->name() ) );

      mWidgets.append( htmlWrapper );

      newWidgetInfo.widget = htmlWrapper->widget();
      newWidgetInfo.labelText = elementDef->name();
      newWidgetInfo.labelOnTop = true;
      newWidgetInfo.showLabel = widgetDef->showLabel();
      break;
    }

    default:
      QgsDebugMsg( QStringLiteral( "Unknown attribute editor widget type encountered..." ) );
      break;
  }

  newWidgetInfo.showLabel = widgetDef->showLabel();

  return newWidgetInfo;
}

void QgsAttributeForm::addWidgetWrapper( QgsEditorWidgetWrapper *eww )
{
  Q_FOREACH ( QgsWidgetWrapper *ww, mWidgets )
  {
    QgsEditorWidgetWrapper *meww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( meww )
    {
      if ( meww->field() == eww->field() )
      {
        connect( meww, static_cast<void ( QgsEditorWidgetWrapper::* )( const QVariant & )>( &QgsEditorWidgetWrapper::valueChanged ), eww, &QgsEditorWidgetWrapper::setValue );
        connect( eww, static_cast<void ( QgsEditorWidgetWrapper::* )( const QVariant & )>( &QgsEditorWidgetWrapper::valueChanged ), meww, &QgsEditorWidgetWrapper::setValue );
        break;
      }
    }
  }

  mWidgets.append( eww );
}

void QgsAttributeForm::createWrappers()
{
  QList<QWidget *> myWidgets = findChildren<QWidget *>();
  const QList<QgsField> fields = mLayer->fields().toList();

  Q_FOREACH ( QWidget *myWidget, myWidgets )
  {
    // Check the widget's properties for a relation definition
    QVariant vRel = myWidget->property( "qgisRelation" );
    if ( vRel.isValid() )
    {
      QgsRelationManager *relMgr = QgsProject::instance()->relationManager();
      QgsRelation relation = relMgr->relation( vRel.toString() );
      if ( relation.isValid() )
      {
        QgsRelationWidgetWrapper *rww = new QgsRelationWidgetWrapper( mLayer, relation, myWidget, this );
        rww->setConfig( mLayer->editFormConfig().widgetConfig( relation.id() ) );
        rww->setContext( mContext );
        rww->widget(); // Will initialize the widget
        mWidgets.append( rww );
      }
    }
    else
    {
      Q_FOREACH ( const QgsField &field, fields )
      {
        if ( field.name() == myWidget->objectName() )
        {
          int idx = mLayer->fields().lookupField( field.name() );

          QgsEditorWidgetWrapper *eww = QgsGui::editorWidgetRegistry()->create( mLayer, idx, myWidget, this, mContext );
          addWidgetWrapper( eww );
        }
      }
    }
  }
}

void QgsAttributeForm::afterWidgetInit()
{
  bool isFirstEww = true;

  Q_FOREACH ( QgsWidgetWrapper *ww, mWidgets )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );

    if ( eww )
    {
      if ( isFirstEww )
      {
        setFocusProxy( eww->widget() );
        isFirstEww = false;
      }

      connect( eww, static_cast<void ( QgsEditorWidgetWrapper::* )( const QVariant & )>( &QgsEditorWidgetWrapper::valueChanged ), this, &QgsAttributeForm::onAttributeChanged );
      connect( eww, &QgsEditorWidgetWrapper::constraintStatusChanged, this, &QgsAttributeForm::onConstraintStatusChanged );
    }
  }
}


bool QgsAttributeForm::eventFilter( QObject *object, QEvent *e )
{
  Q_UNUSED( object )

  if ( e->type() == QEvent::KeyPress )
  {
    QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>( e );
    if ( keyEvent && keyEvent->key() == Qt::Key_Escape )
    {
      // Re-emit to this form so it will be forwarded to parent
      event( e );
      return true;
    }
  }

  return false;
}

void QgsAttributeForm::scanForEqualAttributes( QgsFeatureIterator &fit, QSet< int > &mixedValueFields, QHash< int, QVariant > &fieldSharedValues ) const
{
  mixedValueFields.clear();
  fieldSharedValues.clear();

  QgsFeature f;
  bool first = true;
  while ( fit.nextFeature( f ) )
  {
    for ( int i = 0; i < mLayer->fields().count(); ++i )
    {
      if ( mixedValueFields.contains( i ) )
        continue;

      if ( first )
      {
        fieldSharedValues[i] = f.attribute( i );
      }
      else
      {
        if ( fieldSharedValues.value( i ) != f.attribute( i ) )
        {
          fieldSharedValues.remove( i );
          mixedValueFields.insert( i );
        }
      }
    }
    first = false;

    if ( mixedValueFields.count() == mLayer->fields().count() )
    {
      // all attributes are mixed, no need to keep scanning
      break;
    }
  }
}


void QgsAttributeForm::layerSelectionChanged()
{
  switch ( mMode )
  {
    case QgsAttributeEditorContext::SingleEditMode:
    case QgsAttributeEditorContext::IdentifyMode:
    case QgsAttributeEditorContext::AddFeatureMode:
    case QgsAttributeEditorContext::SearchMode:
    case QgsAttributeEditorContext::AggregateSearchMode:
      break;

    case QgsAttributeEditorContext::MultiEditMode:
      resetMultiEdit( true );
      break;
  }
}

void QgsAttributeForm::setMultiEditFeatureIds( const QgsFeatureIds &fids )
{
  mIsSettingMultiEditFeatures = true;
  mMultiEditFeatureIds = fids;

  if ( fids.isEmpty() )
  {
    // no selected features
    QMap< int, QgsAttributeFormEditorWidget * >::const_iterator wIt = mFormEditorWidgets.constBegin();
    for ( ; wIt != mFormEditorWidgets.constEnd(); ++ wIt )
    {
      wIt.value()->initialize( QVariant() );
    }
    mIsSettingMultiEditFeatures = false;
    return;
  }

  QgsFeatureIterator fit = mLayer->getFeatures( QgsFeatureRequest().setFilterFids( fids ) );

  // Scan through all features to determine which attributes are initially the same
  QSet< int > mixedValueFields;
  QHash< int, QVariant > fieldSharedValues;
  scanForEqualAttributes( fit, mixedValueFields, fieldSharedValues );

  // also fetch just first feature
  fit = mLayer->getFeatures( QgsFeatureRequest().setFilterFid( *fids.constBegin() ) );
  QgsFeature firstFeature;
  fit.nextFeature( firstFeature );

  Q_FOREACH ( int field, mixedValueFields )
  {
    if ( QgsAttributeFormEditorWidget *w = mFormEditorWidgets.value( field, nullptr ) )
    {
      w->initialize( firstFeature.attribute( field ), true );
    }
  }
  QHash< int, QVariant >::const_iterator sharedValueIt = fieldSharedValues.constBegin();
  for ( ; sharedValueIt != fieldSharedValues.constEnd(); ++sharedValueIt )
  {
    if ( QgsAttributeFormEditorWidget *w = mFormEditorWidgets.value( sharedValueIt.key(), nullptr ) )
    {
      w->initialize( sharedValueIt.value(), false );
    }
  }
  mIsSettingMultiEditFeatures = false;
}

void QgsAttributeForm::setMessageBar( QgsMessageBar *messageBar )
{
  if ( mOwnsMessageBar )
    delete mMessageBar;
  mOwnsMessageBar = false;
  mMessageBar = messageBar;
}

QString QgsAttributeForm::aggregateFilter() const
{
  if ( mMode != QgsAttributeEditorContext::AggregateSearchMode )
  {
    Q_ASSERT( false );
  }

  QStringList filters;
  for ( QgsAttributeFormWidget *widget : mFormWidgets )
  {
    QString filter = widget->currentFilterExpression();
    if ( !filter.isNull() )
      filters << '(' + filter + ')';
  }

  return filters.join( QStringLiteral( " AND " ) );
}

int QgsAttributeForm::messageTimeout()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/messageTimeout" ), 5 ).toInt();
}

void QgsAttributeForm::ContainerInformation::apply( QgsExpressionContext *expressionContext )
{
  bool newVisibility = expression.evaluate( expressionContext ).toBool();

  if ( newVisibility != isVisible )
  {
    if ( tabWidget )
    {
      tabWidget->setTabVisible( widget, newVisibility );
    }
    else
    {
      widget->setVisible( newVisibility );
    }

    isVisible = newVisibility;
  }
}

void QgsAttributeForm::updateJoinedFields( const QgsEditorWidgetWrapper &eww )
{
  if ( !eww.layer()->fields().exists( eww.fieldIdx() ) )
    return;

  QgsFeature formFeature;
  QgsField field = eww.layer()->fields().field( eww.fieldIdx() );
  QList<const QgsVectorLayerJoinInfo *> infos = eww.layer()->joinBuffer()->joinsWhereFieldIsId( field );

  if ( infos.count() == 0 || !currentFormFeature( formFeature ) )
    return;

  const QString hint = tr( "No feature joined" );
  Q_FOREACH ( const QgsVectorLayerJoinInfo *info, infos )
  {
    if ( !info->isDynamicFormEnabled() )
      continue;

    QgsFeature joinFeature = mLayer->joinBuffer()->joinedFeatureOf( info, formFeature );

    mJoinedFeatures[info] = joinFeature;

    if ( info->hasSubset() )
    {
      const QStringList subsetNames = QgsVectorLayerJoinInfo::joinFieldNamesSubset( *info );

      Q_FOREACH ( const QString &field, subsetNames )
      {
        QString prefixedName = info->prefixedFieldName( field );
        QVariant val;
        QString hintText = hint;

        if ( joinFeature.isValid() )
        {
          val = joinFeature.attribute( field );
          hintText.clear();
        }

        changeAttribute( prefixedName, val, hintText );
      }
    }
    else
    {
      const QgsFields joinFields = joinFeature.fields();
      for ( const QgsField &field : joinFields )
      {
        QString prefixedName = info->prefixedFieldName( field );
        QVariant val;
        QString hintText = hint;

        if ( joinFeature.isValid() )
        {
          val = joinFeature.attribute( field.name() );
          hintText.clear();
        }

        changeAttribute( prefixedName, val, hintText );
      }
    }
  }
}

bool QgsAttributeForm::fieldIsEditable( int fieldIndex ) const
{
  bool editable = false;

  if ( mLayer->fields().fieldOrigin( fieldIndex ) == QgsFields::OriginJoin )
  {
    int srcFieldIndex;
    const QgsVectorLayerJoinInfo *info = mLayer->joinBuffer()->joinForFieldIndex( fieldIndex, mLayer->fields(), srcFieldIndex );

    if ( info && !info->hasUpsertOnEdit() && mMode == QgsAttributeEditorContext::AddFeatureMode )
      editable = false;
    else if ( info && info->isEditable() && info->joinLayer()->isEditable() )
      editable = fieldIsEditable( *( info->joinLayer() ), srcFieldIndex, mFeature.id() );
  }
  else
    editable = fieldIsEditable( *mLayer, fieldIndex, mFeature.id() );

  return editable;
}

bool QgsAttributeForm::fieldIsEditable( const QgsVectorLayer &layer, int fieldIndex,  QgsFeatureId fid ) const
{
  return !layer.editFormConfig().readOnly( fieldIndex ) &&
         ( ( layer.dataProvider() && layer.dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) || FID_IS_NEW( fid ) );
}

void QgsAttributeForm::updateIcon( QgsEditorWidgetWrapper *eww )
{
  if ( !eww->widget() || !mIconMap[eww->widget()] )
    return;

  // no icon by default
  mIconMap[eww->widget()]->hide();

  if ( !eww->widget()->isEnabled() && mLayer->isEditable() )
  {
    if ( mLayer->fields().fieldOrigin( eww->fieldIdx() ) == QgsFields::OriginJoin )
    {
      int srcFieldIndex;
      const QgsVectorLayerJoinInfo *info = mLayer->joinBuffer()->joinForFieldIndex( eww->fieldIdx(), mLayer->fields(), srcFieldIndex );

      if ( !info )
        return;

      if ( !info->isEditable() )
      {
        const QString file = QStringLiteral( "/mIconJoinNotEditable.svg" );
        const QString tooltip = tr( "Join settings do not allow editing" );
        reloadIcon( file, tooltip, mIconMap[eww->widget()] );
      }
      else if ( mMode == QgsAttributeEditorContext::AddFeatureMode && !info->hasUpsertOnEdit() )
      {
        const QString file = QStringLiteral( "mIconJoinHasNotUpsertOnEdit.svg" );
        const QString tooltip = tr( "Join settings do not allow upsert on edit" );
        reloadIcon( file, tooltip, mIconMap[eww->widget()] );
      }
      else if ( !info->joinLayer()->isEditable() )
      {
        const QString file = QStringLiteral( "/mIconJoinedLayerNotEditable.svg" );
        const QString tooltip = tr( "Joined layer is not toggled editable" );
        reloadIcon( file, tooltip, mIconMap[eww->widget()] );
      }
    }
  }
}

void QgsAttributeForm::reloadIcon( const QString &file, const QString &tooltip, QSvgWidget *sw )
{
  sw->load( QgsApplication::iconPath( file ) );
  sw->setToolTip( tooltip );
  sw->show();
}
