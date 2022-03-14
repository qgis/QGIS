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
#include "qgsattributeeditoraction.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditorrelation.h"
#include "qgsattributeeditorqmlelement.h"
#include "qgsattributeeditorhtmlelement.h"
#include "qgseditorwidgetregistry.h"
#include "qgsfeatureiterator.h"
#include "qgsgui.h"
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
#include "qgsvectorlayerjoinbuffer.h"
#include "qgsvectorlayerutils.h"
#include "qgsactionwidgetwrapper.h"
#include "qgsqmlwidgetwrapper.h"
#include "qgshtmlwidgetwrapper.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeaturerequest.h"
#include "qgstexteditwrapper.h"
#include "qgsfieldmodel.h"
#include "qgscollapsiblegroupbox.h"

#include <QDir>
#include <QTextStream>
#include <QFileInfo>
#include <QFile>
#include <QFormLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QUiLoader>
#include <QMessageBox>
#include <QToolButton>
#include <QMenu>
#include <QSvgWidget>

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
  updateLabels();

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
  for ( QgsAttributeFormWidget *w : std::as_const( mFormWidgets ) )
  {
    switch ( mode )
    {
      case QgsAttributeEditorContext::SingleEditMode:
        w->setMode( QgsAttributeFormWidget::DefaultMode );
        break;

      case QgsAttributeEditorContext::AddFeatureMode:
        w->setMode( QgsAttributeFormWidget::DefaultMode );
        break;

      case QgsAttributeEditorContext::FixAttributeMode:
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
  for ( QgsWidgetWrapper *w : std::as_const( mWidgets ) )
  {
    QgsAttributeEditorContext newContext = w->context();
    newContext.setAttributeFormMode( mMode );
    w->setContext( newContext );
  }

  bool relationWidgetsVisible = ( mMode != QgsAttributeEditorContext::AggregateSearchMode );
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
      synchronizeState();
      mSearchButtonBox->setVisible( false );
      break;

    case QgsAttributeEditorContext::FixAttributeMode:
      synchronizeState();
      mSearchButtonBox->setVisible( false );
      break;

    case QgsAttributeEditorContext::MultiEditMode:
      resetMultiEdit( false );
      synchronizeState();
      mSearchButtonBox->setVisible( false );
      break;

    case QgsAttributeEditorContext::SearchMode:
      mSearchButtonBox->setVisible( true );
      synchronizeState();
      hideButtonBox();
      break;

    case QgsAttributeEditorContext::AggregateSearchMode:
      mSearchButtonBox->setVisible( false );
      synchronizeState();
      hideButtonBox();
      break;

    case QgsAttributeEditorContext::IdentifyMode:
      setFeature( mFeature );
      synchronizeState();
      mSearchButtonBox->setVisible( false );
      break;
  }

  emit modeChanged( mMode );
}

void QgsAttributeForm::changeAttribute( const QString &field, const QVariant &value, const QString &hintText )
{
  const auto constMWidgets = mWidgets;
  for ( QgsWidgetWrapper *ww : constMWidgets )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( eww )
    {
      if ( eww->field().name() == field )
      {
        eww->setValues( value, QVariantList() );
        eww->setHint( hintText );
      }
      // see if the field is present in additional fields of the editor widget
      int index = eww->additionalFields().indexOf( field );
      if ( index >= 0 )
      {
        QVariant mainValue = eww->value();
        QVariantList additionalFieldValues = eww->additionalFieldValues();
        additionalFieldValues[index] = value;
        eww->setValues( mainValue, additionalFieldValues );
        eww->setHint( hintText );
      }
    }
  }
}

void QgsAttributeForm::setFeature( const QgsFeature &feature )
{
  mIsSettingFeature = true;
  mFeature = feature;
  mCurrentFormFeature = feature;

  switch ( mMode )
  {
    case QgsAttributeEditorContext::SingleEditMode:
    case QgsAttributeEditorContext::IdentifyMode:
    case QgsAttributeEditorContext::AddFeatureMode:
    case QgsAttributeEditorContext::FixAttributeMode:
    {
      resetValues();

      synchronizeState();

      // Settings of feature is done when we trigger the attribute form interface
      // Issue https://github.com/qgis/QGIS/issues/29667
      mIsSettingFeature = false;
      const auto constMInterfaces = mInterfaces;
      for ( QgsAttributeFormInterface *iface : constMInterfaces )
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

bool QgsAttributeForm::saveEdits( QString *error )
{
  bool success = true;
  bool changedLayer = false;

  QgsFeature updatedFeature = QgsFeature( mFeature );
  if ( mFeature.isValid() || mMode == QgsAttributeEditorContext::AddFeatureMode )
  {
    bool doUpdate = false;

    // An add dialog should perform an action by default
    // and not only if attributes have "changed"
    if ( mMode == QgsAttributeEditorContext::AddFeatureMode || mMode == QgsAttributeEditorContext::FixAttributeMode )
      doUpdate = true;

    QgsAttributes src = mFeature.attributes();
    QgsAttributes dst = mFeature.attributes();

    for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
    {
      QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
      if ( eww )
      {
        // check for invalid JSON values
        QgsTextEditWrapper *textEdit = qobject_cast<QgsTextEditWrapper *>( eww );
        if ( textEdit && textEdit->isInvalidJSON() )
        {
          if ( error )
            *error = tr( "JSON value for %1 is invalid and has not been saved" ).arg( eww->field().name() );
          return false;
        }
        QVariantList dstVars = QVariantList() << dst.at( eww->fieldIdx() );
        QVariantList srcVars = QVariantList() << eww->value();
        QList<int> fieldIndexes = QList<int>() << eww->fieldIdx();

        // append additional fields
        const QStringList additionalFields = eww->additionalFields();
        for ( const QString &fieldName : additionalFields )
        {
          int idx = eww->layer()->fields().lookupField( fieldName );
          fieldIndexes << idx;
          dstVars << dst.at( idx );
        }
        srcVars.append( eww->additionalFieldValues() );

        Q_ASSERT( dstVars.count() == srcVars.count() );

        for ( int i = 0; i < dstVars.count(); i++ )
        {

          if ( !qgsVariantEqual( dstVars[i], srcVars[i] ) && srcVars[i].isValid() )
          {
            dst[fieldIndexes[i]] = srcVars[i];

            doUpdate = true;
          }
        }
      }
    }

    updatedFeature.setAttributes( dst );

    const auto constMInterfaces = mInterfaces;
    for ( QgsAttributeFormInterface *iface : constMInterfaces )
    {
      if ( !iface->acceptChanges( updatedFeature ) )
      {
        doUpdate = false;
      }
    }

    if ( doUpdate )
    {
      if ( mMode == QgsAttributeEditorContext::FixAttributeMode )
      {
        mFeature = updatedFeature;
      }
      else if ( mMode == QgsAttributeEditorContext::AddFeatureMode )
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

          QgsDebugMsgLevel( QStringLiteral( "Updating field %1" ).arg( i ), 2 );
          QgsDebugMsgLevel( QStringLiteral( "dst:'%1' (type:%2, isNull:%3, isValid:%4)" )
                            .arg( dst.at( i ).toString(), dst.at( i ).typeName() ).arg( dst.at( i ).isNull() ).arg( dst.at( i ).isValid() ), 2 );
          QgsDebugMsgLevel( QStringLiteral( "src:'%1' (type:%2, isNull:%3, isValid:%4)" )
                            .arg( src.at( i ).toString(), src.at( i ).typeName() ).arg( src.at( i ).isNull() ).arg( src.at( i ).isValid() ), 2 );

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

QgsFeature QgsAttributeForm::getUpdatedFeature() const
{
  // create updated Feature
  QgsFeature updatedFeature = QgsFeature( mFeature );

  QgsAttributes featureAttributes = mFeature.attributes();
  for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( !eww )
      continue;

    QVariantList dstVars = QVariantList() << featureAttributes.at( eww->fieldIdx() );
    QVariantList srcVars = QVariantList() << eww->value();
    QList<int> fieldIndexes = QList<int>() << eww->fieldIdx();

    // append additional fields
    const QStringList additionalFields = eww->additionalFields();
    for ( const QString &fieldName : additionalFields )
    {
      int idx = eww->layer()->fields().lookupField( fieldName );
      fieldIndexes << idx;
      dstVars << featureAttributes.at( idx );
    }
    srcVars.append( eww->additionalFieldValues() );

    Q_ASSERT( dstVars.count() == srcVars.count() );

    for ( int i = 0; i < dstVars.count(); i++ )
    {
      if ( !qgsVariantEqual( dstVars[i], srcVars[i] ) && srcVars[i].isValid() )
        featureAttributes[fieldIndexes[i]] = srcVars[i];
    }
  }
  updatedFeature.setAttributes( featureAttributes );

  return updatedFeature;
}

void QgsAttributeForm::updateValuesDependencies( const int originIdx )
{
  updateFieldDependencies();

  updateValuesDependenciesDefaultValues( originIdx );
  updateValuesDependenciesVirtualFields( originIdx );
}

void QgsAttributeForm::updateValuesDependenciesDefaultValues( const int originIdx )
{
  if ( !mDefaultValueDependencies.contains( originIdx ) )
    return;

  if ( !mFeature.isValid()
       && mMode != QgsAttributeEditorContext::AddFeatureMode )
    return;

  // create updated Feature
  QgsFeature updatedFeature = getUpdatedFeature();

  // go through depending fields and update the fields with defaultexpression
  QList<QgsWidgetWrapper *> relevantWidgets = mDefaultValueDependencies.values( originIdx );
  for ( QgsWidgetWrapper *ww : std::as_const( relevantWidgets ) )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( eww )
    {
      //do not update when when mMode is not AddFeatureMode and it's not applyOnUpdate
      if ( mMode != QgsAttributeEditorContext::AddFeatureMode && !eww->field().defaultValueDefinition().applyOnUpdate() )
      {
        continue;
      }

      //do not update when this widget is already updating (avoid recursions)
      if ( mAlreadyUpdatedFields.contains( eww->fieldIdx() ) )
        continue;

      QgsExpressionContext context = createExpressionContext( updatedFeature );
      const QVariant value = mLayer->defaultValue( eww->fieldIdx(), updatedFeature, &context );
      eww->setValue( value );
      mCurrentFormFeature.setAttribute( eww->field().name(), value );
    }
  }
}

void QgsAttributeForm::updateValuesDependenciesVirtualFields( const int originIdx )
{
  if ( !mVirtualFieldsDependencies.contains( originIdx ) )
    return;

  if ( !mFeature.isValid() )
    return;

  // create updated Feature
  QgsFeature updatedFeature = getUpdatedFeature();

  // go through depending fields and update the virtual field with its expression
  const QList<QgsWidgetWrapper *> relevantWidgets = mVirtualFieldsDependencies.values( originIdx );
  for ( QgsWidgetWrapper *ww : relevantWidgets )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( !eww )
      continue;

    //do not update when this widget is already updating (avoid recursions)
    if ( mAlreadyUpdatedFields.contains( eww->fieldIdx() ) )
      continue;

    // Update value
    QgsExpressionContext context = createExpressionContext( updatedFeature );
    QgsExpression exp( mLayer->expressionField( eww->fieldIdx() ) );
    const QVariant value = exp.evaluate( &context );
    updatedFeature.setAttribute( eww->fieldIdx(), value );
    eww->setValue( value );
  }
}

void QgsAttributeForm::updateRelatedLayerFields()
{
  // Synchronize dependencies
  updateRelatedLayerFieldsDependencies();

  if ( mRelatedLayerFieldsDependencies.isEmpty() )
    return;

  if ( !mFeature.isValid() )
    return;

  // create updated Feature
  QgsFeature updatedFeature = getUpdatedFeature();

  // go through depending fields and update the fields with virtual field
  const QSet<QgsEditorWidgetWrapper *> relevantWidgets = mRelatedLayerFieldsDependencies;
  for ( QgsEditorWidgetWrapper *eww : relevantWidgets )
  {
    //do not update when this widget is already updating (avoid recursions)
    if ( mAlreadyUpdatedFields.contains( eww->fieldIdx() ) )
      continue;

    // Update value
    QgsExpressionContext context = createExpressionContext( updatedFeature );
    QgsExpression exp( mLayer->expressionField( eww->fieldIdx() ) );
    QVariant value = exp.evaluate( &context );
    eww->setValue( value );
  }
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
                              Qgis::MessageLevel::Info );
  }
  else
  {
    mMessageBar->pushMessage( QString(),
                              tr( "No matching features found" ),
                              Qgis::MessageLevel::Info );
  }
}

void QgsAttributeForm::displayWarning( const QString &message )
{
  mMessageBar->pushMessage( QString(),
                            message,
                            Qgis::MessageLevel::Warning );
}

void QgsAttributeForm::runSearchSelect( Qgis::SelectBehavior behavior )
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
  runSearchSelect( Qgis::SelectBehavior::SetSelection );
}

void QgsAttributeForm::searchAddToSelection()
{
  runSearchSelect( Qgis::SelectBehavior::AddToSelection );
}

void QgsAttributeForm::searchRemoveFromSelection()
{
  runSearchSelect( Qgis::SelectBehavior::RemoveFromSelection );
}

void QgsAttributeForm::searchIntersectSelection()
{
  runSearchSelect( Qgis::SelectBehavior::IntersectSelection );
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
         || !fieldIsEditable( wIt.key() ) ) // or the field cannot be edited ...
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

  const auto constMultiEditFeatureIds = mMultiEditFeatureIds;
  for ( QgsFeatureId fid : constMultiEditFeatureIds )
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
    mMultiEditMessageBarItem = new QgsMessageBarItem( tr( "Attribute changes for multiple features applied." ), Qgis::MessageLevel::Success, -1 );
  }
  else
  {
    mLayer->destroyEditCommand();
    mMultiEditMessageBarItem = new QgsMessageBarItem( tr( "Changes could not be applied." ), Qgis::MessageLevel::Warning, 0 );
  }

  if ( !mButtonBox->isVisible() )
    mMessageBar->pushItem( mMultiEditMessageBarItem );
  return success;
}

bool QgsAttributeForm::save()
{
  return saveWithDetails( nullptr );
}

bool QgsAttributeForm::saveWithDetails( QString *error )
{
  if ( error )
    error->clear();

  if ( mIsSaving )
    return true;

  if ( mContext.formMode() == QgsAttributeEditorContext::Embed && !mValidConstraints )
  {
    // the feature isn't saved (as per the warning provided), but we return true
    // so switching features still works
    return true;
  }

  for ( QgsWidgetWrapper *wrapper : std::as_const( mWidgets ) )
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
    case QgsAttributeEditorContext::FixAttributeMode:
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
    case QgsAttributeEditorContext::FixAttributeMode:
    case QgsAttributeEditorContext::SearchMode:
    case QgsAttributeEditorContext::AggregateSearchMode:
      success = saveEdits( error );
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
  const auto constMWidgets = mWidgets;
  for ( QgsWidgetWrapper *ww : constMWidgets )
  {
    ww->setFeature( mFeature );
  }
  mValuesInitialized = true;
  mDirty = false;
}

void QgsAttributeForm::resetSearch()
{
  const auto widgets { findChildren<  QgsAttributeFormEditorWidget * >() };
  for ( QgsAttributeFormEditorWidget *w : widgets )
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
  for ( QgsAttributeFormWidget *w : std::as_const( mFormWidgets ) )
  {
    QString filter = w->currentFilterExpression();
    if ( !filter.isEmpty() )
      filters << filter;
  }

  if ( filters.isEmpty() )
    return QString();

  QString filter = filters.join( QLatin1String( ") AND (" ) ).prepend( '(' ).append( ')' );
  return filter;
}

QgsExpressionContext QgsAttributeForm::createExpressionContext( const QgsFeature &feature ) const
{
  QgsExpressionContext context;
  context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  context.appendScope( QgsExpressionContextUtils::formScope( feature, mContext.attributeFormModeString() ) );
  if ( mExtraContextScope )
    context.appendScope( new QgsExpressionContextScope( *mExtraContextScope.get() ) );
  context.setFeature( feature );
  return context;
}


void QgsAttributeForm::onAttributeChanged( const QVariant &value, const QVariantList &additionalFieldValues )
{
  QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( sender() );
  Q_ASSERT( eww );

  bool signalEmitted = false;

  if ( mValuesInitialized )
    mDirty = true;

  mCurrentFormFeature.setAttribute( eww->field().name(), value );

  switch ( mMode )
  {
    case QgsAttributeEditorContext::SingleEditMode:
    case QgsAttributeEditorContext::IdentifyMode:
    case QgsAttributeEditorContext::AddFeatureMode:
    case QgsAttributeEditorContext::FixAttributeMode:
    {
      Q_NOWARN_DEPRECATED_PUSH
      emit attributeChanged( eww->field().name(), value );
      Q_NOWARN_DEPRECATED_POP
      emit widgetValueChanged( eww->field().name(), value, !mIsSettingFeature );

      // also emit the signal for additional values
      const QStringList additionalFields = eww->additionalFields();
      for ( int i = 0; i < additionalFields.count(); i++ )
      {
        const QString fieldName = additionalFields.at( i );
        const QVariant value = additionalFieldValues.at( i );
        emit widgetValueChanged( fieldName, value, !mIsSettingFeature );
      }

      signalEmitted = true;

      if ( mValuesInitialized )
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

        mMultiEditUnsavedMessageBarItem = new QgsMessageBarItem( msgLabel, Qgis::MessageLevel::Warning );
        if ( !mButtonBox->isVisible() )
          mMessageBar->pushItem( mMultiEditUnsavedMessageBarItem );

        emit widgetValueChanged( eww->field().name(), value, !mIsSettingFeature );
      }
      break;
    }
    case QgsAttributeEditorContext::SearchMode:
    case QgsAttributeEditorContext::AggregateSearchMode:
      //nothing to do
      break;
  }

  updateConstraints( eww );

  //append field index here, so it's not updated recursive
  mAlreadyUpdatedFields.append( eww->fieldIdx() );
  updateValuesDependencies( eww->fieldIdx() );
  mAlreadyUpdatedFields.removeAll( eww->fieldIdx() );

  // Updates expression controlled labels
  updateLabels();

  if ( !signalEmitted )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit attributeChanged( eww->field().name(), value );
    Q_NOWARN_DEPRECATED_POP
    emit widgetValueChanged( eww->field().name(), value, !mIsSettingFeature );
  }
}

void QgsAttributeForm::updateAllConstraints()
{
  const auto constMWidgets = mWidgets;
  for ( QgsWidgetWrapper *ww : constMWidgets )
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
  if ( currentFormValuesFeature( ft ) )
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
    synchronizeState();

    QgsExpressionContext context = createExpressionContext( ft );

    // Recheck visibility/collapsed state for all containers which are controlled by this value
    const QVector<ContainerInformation *> infos = mContainerInformationDependency.value( eww->field().name() );
    for ( ContainerInformation *info : infos )
    {
      info->apply( &context );
    }
  }
}

void QgsAttributeForm::updateContainersVisibility()
{
  QgsExpressionContext context = createExpressionContext( mFeature );

  const QVector<ContainerInformation *> infos = mContainerVisibilityCollapsedInformation;

  for ( ContainerInformation *info : infos )
  {
    info->apply( &context );
  }

  //and update the constraints
  updateAllConstraints();
}

void QgsAttributeForm::updateConstraint( const QgsFeature &ft, QgsEditorWidgetWrapper *eww )
{

  if ( mContext.attributeFormMode() != QgsAttributeEditorContext::Mode::MultiEditMode )
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

}

void QgsAttributeForm::updateLabels()
{
  if ( ! mLabelDataDefinedProperties.isEmpty() )
  {
    QgsFeature currentFeature;
    if ( currentFormValuesFeature( currentFeature ) )
    {
      QgsExpressionContext context = createExpressionContext( currentFeature );

      for ( auto it = mLabelDataDefinedProperties.constBegin() ; it != mLabelDataDefinedProperties.constEnd(); ++it )
      {
        QLabel *label { it.key() };
        bool ok;
        const QString value { it->valueAsString( context, QString(), &ok ) };
        if ( ok && ! value.isEmpty() )
        {
          label->setText( value );
        }
      }
    }
  }
}

bool QgsAttributeForm::currentFormValuesFeature( QgsFeature &feature )
{
  bool rc = true;
  feature = QgsFeature( mFeature );
  QgsAttributes dst = feature.attributes();

  for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );

    if ( !eww )
      continue;

    if ( dst.count() > eww->fieldIdx() )
    {
      QVariantList dstVars = QVariantList() << dst.at( eww->fieldIdx() );
      QVariantList srcVars = QVariantList() << eww->value();
      QList<int> fieldIndexes = QList<int>() << eww->fieldIdx();

      // append additional fields
      const QStringList additionalFields = eww->additionalFields();
      for ( const QString &fieldName : additionalFields )
      {
        int idx = eww->layer()->fields().lookupField( fieldName );
        fieldIndexes << idx;
        dstVars << dst.at( idx );
      }
      srcVars.append( eww->additionalFieldValues() );

      Q_ASSERT( dstVars.count() == srcVars.count() );

      for ( int i = 0; i < dstVars.count(); i++ )
      {
        // need to check dstVar.isNull() != srcVar.isNull()
        // otherwise if dstVar=NULL and scrVar=0, then dstVar = srcVar
        if ( ( !qgsVariantEqual( dstVars[i], srcVars[i] ) || dstVars[i].isNull() != srcVars[i].isNull() ) && srcVars[i].isValid() )
        {
          dst[fieldIndexes[i]] = srcVars[i];
        }
      }
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
  mContainerVisibilityCollapsedInformation.append( info );

  const QSet<QString> referencedColumns = info->expression.referencedColumns().unite( info->collapsedExpression.referencedColumns() );

  for ( const QString &col : referencedColumns )
  {
    mContainerInformationDependency[ col ].append( info );
  }
}

bool QgsAttributeForm::currentFormValidConstraints( QStringList &invalidFields, QStringList &descriptions ) const
{
  bool valid{ true };

  for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
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

bool QgsAttributeForm::currentFormValidHardConstraints( QStringList &invalidFields, QStringList &descriptions ) const
{
  bool valid{ true };

  for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( eww )
    {
      if ( eww->isBlockingCommit() )
      {
        invalidFields.append( eww->field().displayName() );
        descriptions.append( eww->constraintFailureReason() );
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

void QgsAttributeForm::onRelatedFeaturesChanged()
{
  updateRelatedLayerFields();
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
  for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
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
  return setupRelationWidgetWrapper( QString(), rel, context );
}

QgsRelationWidgetWrapper *QgsAttributeForm::setupRelationWidgetWrapper( const QString &relationWidgetTypeId, const QgsRelation &rel, const QgsAttributeEditorContext &context )
{
  QgsRelationWidgetWrapper *rww = new QgsRelationWidgetWrapper( relationWidgetTypeId, mLayer, rel, nullptr, this );
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

void QgsAttributeForm::parentFormValueChanged( const QString &attribute, const QVariant &newValue )
{
  for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( eww )
    {
      eww->parentFormValueChanged( attribute, newValue );
    }
  }
}

bool QgsAttributeForm::needsGeometry() const
{
  return mNeedsGeometry;
}

void QgsAttributeForm::synchronizeState()
{
  bool isEditable = ( mFeature.isValid()
                      || mMode == QgsAttributeEditorContext::AddFeatureMode
                      || mMode == QgsAttributeEditorContext::MultiEditMode ) && mLayer->isEditable();

  for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
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
    else  // handle QgsWidgetWrapper different than QgsEditorWidgetWrapper
    {
      ww->setEnabled( isEditable );
    }

  }


  if ( mMode != QgsAttributeEditorContext::SearchMode )
  {
    QStringList invalidFields, descriptions;
    mValidConstraints = currentFormValidHardConstraints( invalidFields, descriptions );

    if ( isEditable && mContext.formMode() == QgsAttributeEditorContext::Embed )
    {
      if ( !mValidConstraints && !mConstraintsFailMessageBarItem )
      {
        mConstraintsFailMessageBarItem = new QgsMessageBarItem( tr( "Changes to this form will not be saved. %n field(s) don't meet their constraints.", "invalid fields", invalidFields.size() ), Qgis::MessageLevel::Warning, -1 );
        mMessageBar->pushItem( mConstraintsFailMessageBarItem );
      }
      else if ( mValidConstraints && mConstraintsFailMessageBarItem )
      {
        mMessageBar->popWidget( mConstraintsFailMessageBarItem );
        mConstraintsFailMessageBarItem = nullptr;
      }
    }
    else if ( mConstraintsFailMessageBarItem )
    {
      mMessageBar->popWidget( mConstraintsFailMessageBarItem );
      mConstraintsFailMessageBarItem = nullptr;
    }

    isEditable = isEditable & mValidConstraints;
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
  mNeedsGeometry = false;

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
    QFile *file = QgsApplication::networkContentFetcherRegistry()->localFile( path );
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
    bool hasRootFields = false;
    bool addSpacer = true;

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
          if ( containerDef->visibilityExpression().enabled() || containerDef->collapsedExpression().enabled() )
          {
            registerContainerInformation( new ContainerInformation( widgetInfo.widget, containerDef->visibilityExpression().enabled() ? containerDef->visibilityExpression().data() : QgsExpression(), containerDef->collapsed(), containerDef->collapsedExpression().enabled() ? containerDef->collapsedExpression().data() : QgsExpression() ) );
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
      else if ( widgDef->type() == QgsAttributeEditorElement::AeTypeRelation )
      {
        hasRootFields = true;
        tabWidget = nullptr;
        WidgetInfo widgetInfo = createWidgetFromDef( widgDef, container, mLayer, mContext );
        QgsCollapsibleGroupBox *collapsibleGroupBox = new QgsCollapsibleGroupBox();

        if ( widgetInfo.showLabel )
          collapsibleGroupBox->setTitle( widgetInfo.labelText );

        QVBoxLayout *collapsibleGroupBoxLayout = new QVBoxLayout();
        collapsibleGroupBoxLayout->addWidget( widgetInfo.widget );
        collapsibleGroupBox->setLayout( collapsibleGroupBoxLayout );

        QVBoxLayout *c = new QVBoxLayout();
        c->addWidget( collapsibleGroupBox );
        layout->addLayout( c, row, column, 1, 2 );
        column += 2;

        // we consider all relation editors should be expanding
        addSpacer = false;
      }
      else
      {
        hasRootFields = true;
        tabWidget = nullptr;
        WidgetInfo widgetInfo = createWidgetFromDef( widgDef, container, mLayer, mContext );
        QLabel *label = new QLabel( widgetInfo.labelText );
        label->setToolTip( widgetInfo.toolTip );
        if ( columnCount > 1 && !widgetInfo.labelOnTop )
        {
          label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
        }

        label->setBuddy( widgetInfo.widget );

        // If at least one expanding widget is present do not add a spacer
        if ( widgetInfo.widget
             && widgetInfo.widget->sizePolicy().verticalPolicy() != QSizePolicy::Fixed
             && widgetInfo.widget->sizePolicy().verticalPolicy() != QSizePolicy::Maximum
             && widgetInfo.widget->sizePolicy().verticalPolicy() != QSizePolicy::Preferred )
          addSpacer = false;

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

        // Alias DD overrides
        if ( widgDef->type() == QgsAttributeEditorElement::AttributeEditorType::AeTypeField )
        {
          const QgsAttributeEditorField *fieldElement { static_cast<QgsAttributeEditorField *>( widgDef ) };
          const int fieldIdx = fieldElement->idx();
          if ( fieldIdx >= 0 && fieldIdx < mLayer->fields().count() )
          {
            const QString fieldName { mLayer->fields().at( fieldIdx ).name() };
            if ( mLayer->editFormConfig().dataDefinedFieldProperties( fieldName ).hasProperty( QgsEditFormConfig::DataDefinedProperty::Alias ) )
            {
              const QgsProperty property { mLayer->editFormConfig().dataDefinedFieldProperties( fieldName ).property( QgsEditFormConfig::DataDefinedProperty::Alias ) };
              if ( property.isActive() && ! property.expressionString().isEmpty() )
              {
                mLabelDataDefinedProperties[ label ] = property;
              }
            }
          }
        }
      }

      if ( column >= columnCount * 2 )
      {
        column = 0;
        row += 1;
      }
    }

    if ( hasRootFields && addSpacer )
    {
      QSpacerItem *spacerItem = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
      layout->addItem( spacerItem, row, 0 );
      layout->setRowStretch( row, 1 );
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
      labelText.replace( '&', QLatin1String( "&&" ) ); // need to escape '&' or they'll be replace by _ in the label text

      const QgsEditorWidgetSetup widgetSetup = QgsGui::editorWidgetRegistry()->findBest( mLayer, field.name() );

      if ( widgetSetup.type() == QLatin1String( "Hidden" ) )
        continue;

      bool labelOnTop = mLayer->editFormConfig().labelOnTop( idx );

      // This will also create the widget
      QLabel *label = new QLabel( labelText );
      label->setToolTip( QgsFieldModel::fieldToolTipExtended( field, mLayer ) );
      QSvgWidget *i = new QSvgWidget();
      i->setFixedSize( 18, 18 );

      if ( mLayer->editFormConfig().dataDefinedFieldProperties( fieldName ).hasProperty( QgsEditFormConfig::DataDefinedProperty::Alias ) )
      {
        const QgsProperty property { mLayer->editFormConfig().dataDefinedFieldProperties( fieldName ).property( QgsEditFormConfig::DataDefinedProperty::Alias ) };
        if ( property.isActive() && ! property.expressionString().isEmpty() )
        {
          mLabelDataDefinedProperties[ label ] = property;
        }
      }

      QgsEditorWidgetWrapper *eww = QgsGui::editorWidgetRegistry()->create( widgetSetup.type(), mLayer, idx, widgetSetup.config(), nullptr, this, mContext );

      QWidget *w = nullptr;
      if ( eww )
      {
        QgsAttributeFormEditorWidget *formWidget = new QgsAttributeFormEditorWidget( eww, widgetSetup.type(), this );
        w = formWidget;
        mFormEditorWidgets.insert( idx, formWidget );
        mFormWidgets.append( formWidget );
        formWidget->createSearchWidgetWrappers( mContext );

        label->setBuddy( eww->widget() );
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
        gridLayout->addWidget( label, row++, 0, 1, 2 );
        gridLayout->addWidget( w, row++, 0, 1, 2 );
        gridLayout->addWidget( i, row++, 0, 1, 2 );
      }
      else
      {
        gridLayout->addWidget( label, row, 0 );
        gridLayout->addWidget( w, row, 1 );
        gridLayout->addWidget( i, row++, 2 );
      }

    }

    const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencedRelations( mLayer );
    for ( const QgsRelation &rel : relations )
    {
      QgsRelationWidgetWrapper *rww = setupRelationWidgetWrapper( QStringLiteral( "relation_editor" ), rel, mContext );

      QgsAttributeFormRelationEditorWidget *formWidget = new QgsAttributeFormRelationEditorWidget( rww, this );
      formWidget->createSearchWidgetWrappers( mContext );

      QgsCollapsibleGroupBox *collapsibleGroupBox = new QgsCollapsibleGroupBox( rel.name() );
      QVBoxLayout *collapsibleGroupBoxLayout = new QVBoxLayout();
      collapsibleGroupBoxLayout->addWidget( formWidget );
      collapsibleGroupBox->setLayout( collapsibleGroupBoxLayout );

      gridLayout->addWidget( collapsibleGroupBox, row++, 0, 1, 2 );

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

  updateFieldDependencies();

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

    QPushButton *openAttributeTableButton = new QPushButton();
    openAttributeTableButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    openAttributeTableButton->setText( tr( "Show in &Table" ) );
    openAttributeTableButton->setToolTip( tr( "Open the attribute table editor with the filtered features" ) );
    connect( openAttributeTableButton, &QToolButton::clicked, this, [ = ]
    {
      emit openFilteredFeaturesAttributeTable( createFilterExpression() );
    } );
    boxLayout->addWidget( openAttributeTableButton );

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
      filterButton->setText( tr( "Filter Features" ) );
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

  connect( mLayer, &QgsVectorLayer::editingStarted, this, &QgsAttributeForm::synchronizeState );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsAttributeForm::synchronizeState );

  // This triggers a refresh of the form widget and gives a chance to re-format the
  // value to those widgets that have a different representation when in edit mode
  connect( mLayer, &QgsVectorLayer::editingStarted, this, &QgsAttributeForm::resetValues );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsAttributeForm::resetValues );


  const auto constMInterfaces = mInterfaces;
  for ( QgsAttributeFormInterface *iface : constMInterfaces )
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
        if ( !initFilePath.isEmpty() )
        {
          QFile *inputFile = QgsApplication::networkContentFetcherRegistry()->localFile( initFilePath );

          if ( inputFile && inputFile->open( QFile::ReadOnly ) )
          {
            // Read it into a string
            QTextStream inf( inputFile );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            inf.setCodec( "UTF-8" );
#endif
            initCode = inf.readAll();
            inputFile->close();
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
        // Nothing to do: the function code should be already in the environment
        break;
    }

    // If we have a function code, run it
    if ( !initCode.isEmpty() )
    {
      if ( QgsGui::pythonMacroAllowed() )
        QgsPythonRunner::run( initCode );
      else
        mMessageBar->pushMessage( QString(),
                                  tr( "Python macro could not be run due to missing permissions." ),
                                  Qgis::MessageLevel::Warning );
    }

    QgsPythonRunner::run( QStringLiteral( "import inspect" ) );
    QString numArgs;

    // Check for eval result
    if ( QgsPythonRunner::eval( QStringLiteral( "len(inspect.getfullargspec(%1)[0])" ).arg( initFunction ), numArgs ) )
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
    case QgsAttributeEditorElement::AeTypeAction:
    {
      const QgsAttributeEditorAction *elementDef = dynamic_cast<const QgsAttributeEditorAction *>( widgetDef );
      if ( !elementDef )
        break;

      QgsActionWidgetWrapper *actionWrapper = new QgsActionWidgetWrapper( mLayer, nullptr, this );
      actionWrapper->setAction( elementDef->action( vl ) );
      context.setAttributeFormMode( mMode );
      actionWrapper->setContext( context );
      mWidgets.append( actionWrapper );
      newWidgetInfo.widget = actionWrapper->widget();
      newWidgetInfo.showLabel = false;

      break;
    }

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
      newWidgetInfo.labelText.replace( '&', QLatin1String( "&&" ) ); // need to escape '&' or they'll be replace by _ in the label text
      newWidgetInfo.toolTip = QStringLiteral( "<b>%1</b><p>%2</p>" ).arg( mLayer->attributeDisplayName( fldIdx ), newWidgetInfo.hint );
      newWidgetInfo.showLabel = widgetDef->showLabel();

      break;
    }

    case QgsAttributeEditorElement::AeTypeRelation:
    {
      const QgsAttributeEditorRelation *relDef = static_cast<const QgsAttributeEditorRelation *>( widgetDef );

      QgsRelationWidgetWrapper *rww = setupRelationWidgetWrapper( relDef->relationWidgetTypeId(), relDef->relation(), context );

      QgsAttributeFormRelationEditorWidget *formWidget = new QgsAttributeFormRelationEditorWidget( rww, this );
      formWidget->createSearchWidgetWrappers( mContext );

      // This needs to be after QgsAttributeFormRelationEditorWidget creation, because the widget
      // does not exists yet until QgsAttributeFormRelationEditorWidget is created and the setters
      // below directly alter the widget and check for it.
      rww->setWidgetConfig( relDef->relationEditorConfiguration() );
      rww->setNmRelationId( relDef->nmRelationId() );
      rww->setForceSuppressFormPopup( relDef->forceSuppressFormPopup() );

      mWidgets.append( rww );
      mFormWidgets.append( formWidget );

      newWidgetInfo.widget = formWidget;
      newWidgetInfo.showLabel = relDef->showLabel();
      newWidgetInfo.labelText = relDef->label();
      if ( newWidgetInfo.labelText.isEmpty() )
        newWidgetInfo.labelText = rww->relation().name();
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
        QgsCollapsibleGroupBoxBasic *groupBox = new QgsCollapsibleGroupBoxBasic();
        widgetName = QStringLiteral( "QGroupBox" );
        if ( container->showLabel() )
          groupBox->setTitle( container->name() );
        myContainer = groupBox;
        newWidgetInfo.widget = myContainer;
        groupBox->setCollapsed( container->collapsed() );
      }
      else
      {
        myContainer = new QWidget();

        QgsScrollArea *scrollArea = new QgsScrollArea( parent );

        scrollArea->setWidget( myContainer );
        scrollArea->setWidgetResizable( true );
        scrollArea->setFrameShape( QFrame::NoFrame );
        widgetName = QStringLiteral( "QScrollArea QWidget" );

        newWidgetInfo.widget = scrollArea;
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
          if ( containerDef->visibilityExpression().enabled() || containerDef->collapsedExpression().enabled() )
          {
            registerContainerInformation( new ContainerInformation( widgetInfo.widget, containerDef->visibilityExpression().enabled() ? containerDef->visibilityExpression().data() : QgsExpression(), containerDef->collapsed(), containerDef->collapsedExpression().enabled() ? containerDef->collapsedExpression().data() : QgsExpression() ) );
          }
        }

        if ( widgetInfo.labelText.isNull() || ! widgetInfo.showLabel )
        {
          gbLayout->addWidget( widgetInfo.widget, row, column, 1, 2 );
          column += 2;
        }
        else
        {
          QLabel *mypLabel = new QLabel( widgetInfo.labelText );

          // Alias DD overrides
          if ( childDef->type() == QgsAttributeEditorElement::AeTypeField )
          {
            const QgsAttributeEditorField *fieldDef { static_cast<QgsAttributeEditorField *>( childDef ) };
            const QgsFields fields = vl->fields();
            const int fldIdx = fieldDef->idx();
            if ( fldIdx < fields.count() && fldIdx >= 0 )
            {
              const QString fieldName { fields.at( fldIdx ).name() };
              if ( mLayer->editFormConfig().dataDefinedFieldProperties( fieldName ).hasProperty( QgsEditFormConfig::DataDefinedProperty::Alias ) )
              {
                const QgsProperty property { mLayer->editFormConfig().dataDefinedFieldProperties( fieldName ).property( QgsEditFormConfig::DataDefinedProperty::Alias ) };
                if ( property.isActive() && ! property.expressionString().isEmpty() )
                {
                  mLabelDataDefinedProperties[ mypLabel ] = property;
                }
              }
            }
          }

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
      newWidgetInfo.showLabel = widgetDef->showLabel();
      break;
    }

    case QgsAttributeEditorElement::AeTypeQmlElement:
    {
      const QgsAttributeEditorQmlElement *elementDef = static_cast<const QgsAttributeEditorQmlElement *>( widgetDef );

      QgsQmlWidgetWrapper *qmlWrapper = new QgsQmlWidgetWrapper( mLayer, nullptr, this );
      qmlWrapper->setQmlCode( elementDef->qmlCode() );
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
      mWidgets.append( htmlWrapper );

      newWidgetInfo.widget = htmlWrapper->widget();
      newWidgetInfo.labelText = elementDef->name();
      newWidgetInfo.labelOnTop = true;
      newWidgetInfo.showLabel = widgetDef->showLabel();
      mNeedsGeometry |= htmlWrapper->needsGeometry();
      break;
    }

    default:
      QgsDebugMsg( QStringLiteral( "Unknown attribute editor widget type encountered..." ) );
      break;
  }

  return newWidgetInfo;
}

void QgsAttributeForm::addWidgetWrapper( QgsEditorWidgetWrapper *eww )
{
  for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
  {
    QgsEditorWidgetWrapper *meww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( meww )
    {
      // if another widget wrapper exists for the same field
      // synchronise them
      if ( meww->field() == eww->field() )
      {
        connect( meww, &QgsEditorWidgetWrapper::valuesChanged, eww, &QgsEditorWidgetWrapper::setValues );
        connect( eww, &QgsEditorWidgetWrapper::valuesChanged, meww, &QgsEditorWidgetWrapper::setValues );
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

  const auto constMyWidgets = myWidgets;
  for ( QWidget *myWidget : constMyWidgets )
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
      const auto constFields = fields;
      for ( const QgsField &field : constFields )
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

  const auto constMWidgets = mWidgets;
  for ( QgsWidgetWrapper *ww : constMWidgets )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );

    if ( eww )
    {
      if ( isFirstEww )
      {
        setFocusProxy( eww->widget() );
        isFirstEww = false;
      }

      connect( eww, &QgsEditorWidgetWrapper::valuesChanged, this, &QgsAttributeForm::onAttributeChanged, Qt::UniqueConnection );
      connect( eww, &QgsEditorWidgetWrapper::constraintStatusChanged, this, &QgsAttributeForm::onConstraintStatusChanged, Qt::UniqueConnection );
    }
    else
    {
      QgsRelationWidgetWrapper *relationWidgetWrapper = qobject_cast<QgsRelationWidgetWrapper *>( ww );
      if ( relationWidgetWrapper )
      {
        connect( relationWidgetWrapper, &QgsRelationWidgetWrapper::relatedFeaturesChanged, this, &QgsAttributeForm::onRelatedFeaturesChanged, static_cast<Qt::ConnectionType>( Qt::UniqueConnection | Qt::QueuedConnection ) );
      }
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

void QgsAttributeForm::scanForEqualAttributes( QgsFeatureIterator &fit,
    QSet< int > &mixedValueFields,
    QHash< int, QVariant > &fieldSharedValues ) const
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
    case QgsAttributeEditorContext::FixAttributeMode:
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

  const auto constMixedValueFields = mixedValueFields;
  for ( int fieldIndex : std::as_const( mixedValueFields ) )
  {
    if ( QgsAttributeFormEditorWidget *w = mFormEditorWidgets.value( fieldIndex, nullptr ) )
    {
      const QStringList additionalFields = w->editorWidget()->additionalFields();
      QVariantList additionalFieldValues;
      for ( const QString &additionalField : additionalFields )
        additionalFieldValues << firstFeature.attribute( additionalField );
      w->initialize( firstFeature.attribute( fieldIndex ), true, additionalFieldValues );
    }
  }
  QHash< int, QVariant >::const_iterator sharedValueIt = fieldSharedValues.constBegin();
  for ( ; sharedValueIt != fieldSharedValues.constEnd(); ++sharedValueIt )
  {
    if ( QgsAttributeFormEditorWidget *w = mFormEditorWidgets.value( sharedValueIt.key(), nullptr ) )
    {
      bool mixed = false;
      const QStringList additionalFields = w->editorWidget()->additionalFields();
      for ( const QString &additionalField : additionalFields )
      {
        int index = mLayer->fields().indexFromName( additionalField );
        if ( constMixedValueFields.contains( index ) )
        {
          // if additional field are mixed, it is considered as mixed
          mixed = true;
          break;
        }
      }
      QVariantList additionalFieldValues;
      if ( mixed )
      {
        for ( const QString &additionalField : additionalFields )
          additionalFieldValues << firstFeature.attribute( additionalField );
        w->initialize( firstFeature.attribute( sharedValueIt.key() ), true, additionalFieldValues );
      }
      else
      {
        for ( const QString &additionalField : additionalFields )
        {
          int index = mLayer->fields().indexFromName( additionalField );
          Q_ASSERT( fieldSharedValues.contains( index ) );
          additionalFieldValues << fieldSharedValues.value( index );
        }
        w->initialize( sharedValueIt.value(), false, additionalFieldValues );
      }
    }
  }

  setMultiEditFeatureIdsRelations( fids );

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

  return filters.join( QLatin1String( " AND " ) );
}

void QgsAttributeForm::setExtraContextScope( QgsExpressionContextScope *extraScope )
{
  mExtraContextScope.reset( extraScope );
}

void QgsAttributeForm::ContainerInformation::apply( QgsExpressionContext *expressionContext )
{

  const bool newVisibility = expression.evaluate( expressionContext ).toBool();

  if ( expression.isValid() && ! expression.hasEvalError() && newVisibility != isVisible )
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

  const bool newCollapsedState = collapsedExpression.evaluate( expressionContext ).toBool();

  if ( collapsedExpression.isValid() && ! collapsedExpression.hasEvalError() && newCollapsedState != isCollapsed )
  {

    if ( QgsCollapsibleGroupBoxBasic * collapsibleGroupBox { qobject_cast<QgsCollapsibleGroupBoxBasic *>( widget ) } )
    {
      collapsibleGroupBox->setCollapsed( newCollapsedState );
      isCollapsed = newCollapsedState;
    }
  }
}

void QgsAttributeForm::updateJoinedFields( const QgsEditorWidgetWrapper &eww )
{
  if ( !eww.layer()->fields().exists( eww.fieldIdx() ) )
    return;

  QgsFeature formFeature;
  QgsField field = eww.layer()->fields().field( eww.fieldIdx() );
  QList<const QgsVectorLayerJoinInfo *> infos = eww.layer()->joinBuffer()->joinsWhereFieldIsId( field );

  if ( infos.count() == 0 || !currentFormValuesFeature( formFeature ) )
    return;

  const QString hint = tr( "No feature joined" );
  const auto constInfos = infos;
  for ( const QgsVectorLayerJoinInfo *info : constInfos )
  {
    if ( !info->isDynamicFormEnabled() )
      continue;

    QgsFeature joinFeature = mLayer->joinBuffer()->joinedFeatureOf( info, formFeature );

    mJoinedFeatures[info] = joinFeature;

    if ( info->hasSubset() )
    {
      const QStringList subsetNames = QgsVectorLayerJoinInfo::joinFieldNamesSubset( *info );

      const auto constSubsetNames = subsetNames;
      for ( const QString &field : constSubsetNames )
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
  return QgsVectorLayerUtils::fieldIsEditable( mLayer, fieldIndex, mFeature );
}

void QgsAttributeForm::updateFieldDependencies()
{
  mDefaultValueDependencies.clear();
  mVirtualFieldsDependencies.clear();
  mRelatedLayerFieldsDependencies.clear();

  //create defaultValueDependencies
  for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
  {
    QgsEditorWidgetWrapper *eww = qobject_cast<QgsEditorWidgetWrapper *>( ww );
    if ( ! eww )
      continue;

    updateFieldDependenciesDefaultValue( eww );
    updateFieldDependenciesVirtualFields( eww );
    updateRelatedLayerFieldsDependencies( eww );
  }
}

void QgsAttributeForm::updateFieldDependenciesDefaultValue( QgsEditorWidgetWrapper *eww )
{
  QgsExpression exp( eww->field().defaultValueDefinition().expression() );
  const QSet<QString> referencedColumns = exp.referencedColumns();
  for ( const QString &referencedColumn : referencedColumns )
  {
    if ( referencedColumn == QgsFeatureRequest::ALL_ATTRIBUTES )
    {
      const QList<int> allAttributeIds( mLayer->fields().allAttributesList() );

      for ( const int id : allAttributeIds )
      {
        mDefaultValueDependencies.insertMulti( id, eww );
      }
    }
    else
    {
      mDefaultValueDependencies.insertMulti( mLayer->fields().lookupField( referencedColumn ), eww );
    }
  }
}

void QgsAttributeForm::updateFieldDependenciesVirtualFields( QgsEditorWidgetWrapper *eww )
{
  QString expressionField = eww->layer()->expressionField( eww->fieldIdx() );
  if ( expressionField.isEmpty() )
    return;

  QgsExpression exp( expressionField );
  const QSet<QString> referencedColumns = exp.referencedColumns();
  for ( const QString &referencedColumn : referencedColumns )
  {
    if ( referencedColumn == QgsFeatureRequest::ALL_ATTRIBUTES )
    {
      const QList<int> allAttributeIds( mLayer->fields().allAttributesList() );
      for ( const int id : allAttributeIds )
      {
        mVirtualFieldsDependencies.insertMulti( id, eww );
      }
    }
    else
    {
      mVirtualFieldsDependencies.insertMulti( mLayer->fields().lookupField( referencedColumn ), eww );
    }
  }
}

void QgsAttributeForm::updateRelatedLayerFieldsDependencies( QgsEditorWidgetWrapper *eww )
{
  if ( eww )
  {
    QString expressionField = eww->layer()->expressionField( eww->fieldIdx() );
    if ( expressionField.contains( QStringLiteral( "relation_aggregate" ) )
         || expressionField.contains( QStringLiteral( "get_features" ) ) )
      mRelatedLayerFieldsDependencies.insert( eww );
  }
  else
  {
    mRelatedLayerFieldsDependencies.clear();
    //create defaultValueDependencies
    for ( QgsWidgetWrapper *ww : std::as_const( mWidgets ) )
    {
      QgsEditorWidgetWrapper *editorWidgetWrapper = qobject_cast<QgsEditorWidgetWrapper *>( ww );
      if ( ! editorWidgetWrapper )
        continue;

      updateRelatedLayerFieldsDependencies( editorWidgetWrapper );
    }
  }
}

void QgsAttributeForm::setMultiEditFeatureIdsRelations( const QgsFeatureIds &fids )
{
  for ( QgsAttributeFormWidget *formWidget : mFormWidgets )
  {
    QgsAttributeFormRelationEditorWidget *relationEditorWidget = dynamic_cast<QgsAttributeFormRelationEditorWidget *>( formWidget );
    if ( !relationEditorWidget )
      continue;

    relationEditorWidget->setMultiEditFeatureIds( fids );
  }
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
