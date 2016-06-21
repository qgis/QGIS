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

#include "qgsattributeeditor.h"
#include "qgsattributeforminterface.h"
#include "qgsattributeformlegacyinterface.h"
#include "qgseditorwidgetregistry.h"
#include "qgsproject.h"
#include "qgspythonrunner.h"
#include "qgsrelationwidgetwrapper.h"
#include "qgsvectordataprovider.h"
#include "qgsattributeformeditorwidget.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"

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
#include <QScrollArea>
#include <QTabWidget>
#include <QUiLoader>
#include <QMessageBox>
#include <QSettings>
#include <QToolButton>
#include <QMenu>

int QgsAttributeForm::sFormCounter = 0;

QgsAttributeForm::QgsAttributeForm( QgsVectorLayer* vl, const QgsFeature &feature, const QgsAttributeEditorContext &context, QWidget* parent )
    : QWidget( parent )
    , mLayer( vl )
    , mMessageBar( nullptr )
    , mOwnsMessageBar( true )
    , mMultiEditUnsavedMessageBarItem( nullptr )
    , mMultiEditMessageBarItem( nullptr )
    , mInvalidConstraintMessage( nullptr )
    , mContext( context )
    , mButtonBox( nullptr )
    , mSearchButtonBox( nullptr )
    , mFormNr( sFormCounter++ )
    , mIsSaving( false )
    , mPreventFeatureRefresh( false )
    , mIsSettingFeature( false )
    , mIsSettingMultiEditFeatures( false )
    , mUnsavedMultiEditChanges( false )
    , mEditCommandMessage( tr( "Attributes changed" ) )
    , mMode( SingleEditMode )
{
  init();
  initPython();
  setFeature( feature );

  connect( vl, SIGNAL( updatedFields() ), this, SLOT( onUpdatedFields() ) );
  connect( vl, SIGNAL( beforeAddingExpressionField( QString ) ), this, SLOT( preventFeatureRefresh() ) );
  connect( vl, SIGNAL( beforeRemovingExpressionField( int ) ), this, SLOT( preventFeatureRefresh() ) );
  connect( vl, SIGNAL( selectionChanged() ), this, SLOT( layerSelectionChanged() ) );

  // constraints management
  updateAllConstaints();
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
  if ( mMode == SingleEditMode )
    connect( mLayer, SIGNAL( beforeModifiedCheck() ), this, SLOT( save() ) );
}

void QgsAttributeForm::showButtonBox()
{
  mButtonBox->show();

  disconnect( mLayer, SIGNAL( beforeModifiedCheck() ), this, SLOT( save() ) );
}

void QgsAttributeForm::disconnectButtonBox()
{
  disconnect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  disconnect( mButtonBox, SIGNAL( rejected() ), this, SLOT( resetValues() ) );
}

void QgsAttributeForm::addInterface( QgsAttributeFormInterface* iface )
{
  mInterfaces.append( iface );
}

bool QgsAttributeForm::editable()
{
  return mFeature.isValid() && mLayer->isEditable();
}

void QgsAttributeForm::setMode( QgsAttributeForm::Mode mode )
{
  if ( mode == mMode )
    return;

  if ( mMode == MultiEditMode )
  {
    //switching out of multi edit mode triggers a save
    if ( mUnsavedMultiEditChanges )
    {
      // prompt for save
      int res = QMessageBox::information( this, tr( "Multiedit attributes" ),
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

  if ( mButtonBox->isVisible() && mMode == SingleEditMode )
  {
    connect( mLayer, SIGNAL( beforeModifiedCheck() ), this, SLOT( save() ) );
  }
  else
  {
    disconnect( mLayer, SIGNAL( beforeModifiedCheck() ), this, SLOT( save() ) );
  }

  //update all form editor widget modes to match
  Q_FOREACH ( QgsAttributeFormEditorWidget* w, findChildren<  QgsAttributeFormEditorWidget* >() )
  {
    switch ( mode )
    {
      case QgsAttributeForm::SingleEditMode:
        w->setMode( QgsAttributeFormEditorWidget::DefaultMode );
        break;

      case QgsAttributeForm::AddFeatureMode:
        w->setMode( QgsAttributeFormEditorWidget::DefaultMode );
        break;

      case QgsAttributeForm::MultiEditMode:
        w->setMode( QgsAttributeFormEditorWidget::MultiEditMode );
        break;

      case QgsAttributeForm::SearchMode:
        w->setMode( QgsAttributeFormEditorWidget::SearchMode );
        break;
    }
  }

  bool relationWidgetsVisible = ( mMode == QgsAttributeForm::SingleEditMode || mMode == QgsAttributeForm::AddFeatureMode );
  Q_FOREACH ( QgsRelationWidgetWrapper* w, findChildren<  QgsRelationWidgetWrapper* >() )
  {
    w->setVisible( relationWidgetsVisible );
  }

  switch ( mode )
  {
    case QgsAttributeForm::SingleEditMode:
      setFeature( mFeature );
      mSearchButtonBox->setVisible( false );
      mInvalidConstraintMessage->show();
      break;

    case QgsAttributeForm::AddFeatureMode:
      synchronizeEnabledState();
      mSearchButtonBox->setVisible( false );
      mInvalidConstraintMessage->show();
      break;

    case QgsAttributeForm::MultiEditMode:
      resetMultiEdit( false );
      synchronizeEnabledState();
      mSearchButtonBox->setVisible( false );
      mInvalidConstraintMessage->show();
      break;

    case QgsAttributeForm::SearchMode:
      mSearchButtonBox->setVisible( true );
      hideButtonBox();
      if ( mContext.formMode() != QgsAttributeEditorContext::Embed )
      {
        delete mInvalidConstraintMessage;
        mInvalidConstraintMessage = nullptr;
      }
      else
      {
        mInvalidConstraintMessage->hide();
      }
      break;
  }

  emit modeChanged( mMode );
}

void QgsAttributeForm::setIsAddDialog( bool isAddDialog )
{
  setMode( isAddDialog ? AddFeatureMode : SingleEditMode );
}

void QgsAttributeForm::changeAttribute( const QString& field, const QVariant& value )
{
  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
    if ( eww && eww->field().name() == field )
    {
      eww->setValue( value );
    }
  }
}

void QgsAttributeForm::setFeature( const QgsFeature& feature )
{
  mIsSettingFeature = true;
  mFeature = feature;

  switch ( mMode )
  {
    case SingleEditMode:
    case AddFeatureMode:
    {
      resetValues();

      synchronizeEnabledState();

      Q_FOREACH ( QgsAttributeFormInterface* iface, mInterfaces )
      {
        iface->featureChanged();
      }
      break;
    }
    case MultiEditMode:
    case SearchMode:
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

  if ( mFeature.isValid() || mMode == AddFeatureMode )
  {
    bool doUpdate = false;

    // An add dialog should perform an action by default
    // and not only if attributes have "changed"
    if ( mMode == AddFeatureMode )
      doUpdate = true;

    QgsAttributes src = mFeature.attributes();
    QgsAttributes dst = mFeature.attributes();

    Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
    {
      QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
      if ( eww )
      {
        QVariant dstVar = dst.at( eww->fieldIdx() );
        QVariant srcVar = eww->value();

        // need to check dstVar.isNull() != srcVar.isNull()
        // otherwise if dstVar=NULL and scrVar=0, then dstVar = srcVar
        // be careful- sometimes two null qvariants will be reported as not equal!! (eg different types)
        bool changed = ( dstVar != srcVar && !dstVar.isNull() && !srcVar.isNull() )
                       || ( dstVar.isNull() != srcVar.isNull() );
        if ( changed && srcVar.isValid()
             && !mLayer->editFormConfig()->readOnly( eww->fieldIdx() ) )
        {
          dst[eww->fieldIdx()] = srcVar;

          doUpdate = true;
        }
      }
    }

    updatedFeature.setAttributes( dst );

    Q_FOREACH ( QgsAttributeFormInterface* iface, mInterfaces )
    {
      if ( !iface->acceptChanges( updatedFeature ) )
      {
        doUpdate = false;
      }
    }

    if ( doUpdate )
    {
      if ( mMode == AddFeatureMode )
      {
        mFeature.setValid( true );
        mLayer->beginEditCommand( mEditCommandMessage );
        bool res = mLayer->addFeature( updatedFeature );
        if ( res )
        {
          mFeature.setAttributes( updatedFeature.attributes() );
          mLayer->endEditCommand();
          setMode( SingleEditMode );
          changedLayer = true;
        }
        else
          mLayer->destroyEditCommand();
      }
      else
      {
        mLayer->beginEditCommand( mEditCommandMessage );

        int n = 0;
        for ( int i = 0; i < dst.count(); ++i )
        {
          if (( dst.at( i ) == src.at( i ) && dst.at( i ).isNull() == src.at( i ).isNull() )  // If field is not changed...
              || !dst.at( i ).isValid()                                     // or the widget returns invalid (== do not change)
              || mLayer->editFormConfig()->readOnly( i ) )                           // or the field cannot be edited ...
          {
            continue;
          }

          QgsDebugMsg( QString( "Updating field %1" ).arg( i ) );
          QgsDebugMsg( QString( "dst:'%1' (type:%2, isNull:%3, isValid:%4)" )
                       .arg( dst.at( i ).toString(), dst.at( i ).typeName() ).arg( dst.at( i ).isNull() ).arg( dst.at( i ).isValid() ) );
          QgsDebugMsg( QString( "src:'%1' (type:%2, isNull:%3, isValid:%4)" )
                       .arg( src.at( i ).toString(), src.at( i ).typeName() ).arg( src.at( i ).isNull() ).arg( src.at( i ).isValid() ) );

          success &= mLayer->changeAttributeValue( mFeature.id(), i, dst.at( i ), src.at( i ) );
          n++;
        }

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
  setMultiEditFeatureIds( mLayer->selectedFeaturesIds() );
}

void QgsAttributeForm::multiEditMessageClicked( const QString& link )
{
  clearMultiEditMessages();
  resetMultiEdit( link == "#apply" );
}

void QgsAttributeForm::filterTriggered()
{
  QString filter = createFilterExpression();
  emit filterExpressionSet( filter, ReplaceFilter );
  if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    setMode( SingleEditMode );
}

void QgsAttributeForm::filterAndTriggered()
{
  QString filter = createFilterExpression();
  if ( filter.isEmpty() )
    return;

  if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    setMode( SingleEditMode );
  emit filterExpressionSet( filter, FilterAnd );
}

void QgsAttributeForm::filterOrTriggered()
{
  QString filter = createFilterExpression();
  if ( filter.isEmpty() )
    return;

  if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    setMode( SingleEditMode );
  emit filterExpressionSet( filter, FilterOr );
}

void QgsAttributeForm::pushSelectedFeaturesMessage()
{
  int count = mLayer->selectedFeatureCount();
  if ( count > 0 )
  {
    mMessageBar->pushMessage( QString(),
                              tr( "%1 matching %2 selected" ).arg( count )
                              .arg( count == 1 ? tr( "feature" ) : tr( "features" ) ),
                              QgsMessageBar::INFO,
                              messageTimeout() );
  }
  else
  {
    mMessageBar->pushMessage( QString(),
                              tr( "No matching features found" ),
                              QgsMessageBar::WARNING,
                              messageTimeout() );
  }
}

void QgsAttributeForm::runSearchSelect( QgsVectorLayer::SelectBehaviour behaviour )
{
  QString filter = createFilterExpression();
  if ( filter.isEmpty() )
    return;

  mLayer->selectByExpression( filter, behaviour );
  pushSelectedFeaturesMessage();
  if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    setMode( SingleEditMode );
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
  QMap< int, QgsAttributeFormEditorWidget* >::const_iterator wIt = mFormEditorWidgets.constBegin();
  for ( ; wIt != mFormEditorWidgets.constEnd(); ++ wIt )
  {
    QgsAttributeFormEditorWidget* w = wIt.value();
    if ( !w->hasChanged() )
      continue;

    if ( !w->currentValue().isValid() // if the widget returns invalid (== do not change)
         || mLayer->editFormConfig()->readOnly( wIt.key() ) ) // or the field cannot be edited ...
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
  int res = QMessageBox::information( this, tr( "Multiedit attributes" ),
                                      tr( "Edits will be applied to all selected features" ), QMessageBox::Ok | QMessageBox::Cancel );
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
    mMultiEditMessageBarItem = new QgsMessageBarItem( tr( "Attribute changes for multiple features applied" ), QgsMessageBar::SUCCESS, messageTimeout() );
  }
  else
  {
    mLayer->destroyEditCommand();
    mMultiEditMessageBarItem = new QgsMessageBarItem( tr( "Changes could not be applied" ), QgsMessageBar::WARNING, messageTimeout() );
  }

  if ( !mButtonBox->isVisible() )
    mMessageBar->pushItem( mMultiEditMessageBarItem );
  return success;
}

bool QgsAttributeForm::save()
{
  if ( mIsSaving )
    return true;

  mIsSaving = true;

  bool success = true;

  emit beforeSave( success );

  // Somebody wants to prevent this form from saving
  if ( !success )
    return false;

  switch ( mMode )
  {
    case SingleEditMode:
    case AddFeatureMode:
    case SearchMode:
      success = saveEdits();
      break;

    case MultiEditMode:
      success = saveMultiEdits();
      break;
  }

  mIsSaving = false;
  mUnsavedMultiEditChanges = false;

  return success;
}

void QgsAttributeForm::resetValues()
{
  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    ww->setFeature( mFeature );
  }
}

void QgsAttributeForm::resetSearch()
{
  Q_FOREACH ( QgsAttributeFormEditorWidget* w, findChildren<  QgsAttributeFormEditorWidget* >() )
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
  Q_FOREACH ( QgsAttributeFormEditorWidget* w, findChildren<  QgsAttributeFormEditorWidget* >() )
  {
    QString filter = w->currentFilterExpression();
    if ( !filter.isEmpty() )
      filters << filter;
  }

  if ( filters.isEmpty() )
    return QString();

  QString filter = filters.join( ") AND (" ).prepend( '(' ).append( ')' );
  return filter;
}

void QgsAttributeForm::onAttributeChanged( const QVariant& value )
{
  QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( sender() );

  Q_ASSERT( eww );

  switch ( mMode )
  {
    case SingleEditMode:
    case AddFeatureMode:
    {
      // don't emit signal if it was triggered by a feature change
      if ( !mIsSettingFeature )
      {
        emit attributeChanged( eww->field().name(), value );
      }
      break;
    }
    case MultiEditMode:
    {
      if ( !mIsSettingMultiEditFeatures )
      {
        mUnsavedMultiEditChanges = true;

        QLabel *msgLabel = new QLabel( tr( "Unsaved multiedit changes: <a href=\"#apply\">apply changes</a> or <a href=\"#reset\">reset changes</a>." ), mMessageBar );
        msgLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        msgLabel->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
        connect( msgLabel, SIGNAL( linkActivated( QString ) ), this, SLOT( multiEditMessageClicked( QString ) ) );
        clearMultiEditMessages();

        mMultiEditUnsavedMessageBarItem = new QgsMessageBarItem( msgLabel, QgsMessageBar::WARNING );
        if ( !mButtonBox->isVisible() )
          mMessageBar->pushItem( mMultiEditUnsavedMessageBarItem );
      }
      break;
    }
    case SearchMode:
      //nothing to do
      break;
  }

  if ( eww->layer()->editFormConfig()->notNull( eww->fieldIdx() ) )
  {
    QLabel* buddy = mBuddyMap.value( eww->widget() );

    if ( buddy )
    {
      if ( !buddy->property( "originalText" ).isValid() )
        buddy->setProperty( "originalText", buddy->text() );

      QString text = buddy->property( "originalText" ).toString();

      if ( value.isNull() )
      {
        // not good
#if QT_VERSION >= 0x050000
        buddy->setText( QString( "%1<font color=\"red\">❌</font>" ).arg( text ) );
#else
        buddy->setText( QString( "%1<font color=\"red\">*</font>" ).arg( text ) );
#endif
      }
      else
      {
        // good
#if QT_VERSION >= 0x050000
        buddy->setText( QString( "%1<font color=\"green\">✔</font>" ).arg( text ) );
#else
        buddy->setText( QString( "%1<font color=\"green\">*</font>" ).arg( text ) );
#endif
      }
    }
  }

  updateConstraints( eww );

  // emit
  emit attributeChanged( eww->field().name(), value );
}

void QgsAttributeForm::updateAllConstaints()
{
  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
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
    // update eww constraint
    eww->updateConstraint( ft );

    // update eww dependencies constraint
    QList<QgsEditorWidgetWrapper*> deps;
    constraintDependencies( eww, deps );

    Q_FOREACH ( QgsEditorWidgetWrapper* depsEww, deps )
      depsEww->updateConstraint( ft );

    // sync ok button status
    synchronizeEnabledState();
  }
}

bool QgsAttributeForm::currentFormFeature( QgsFeature &feature )
{
  bool rc = true;
  feature = QgsFeature( mFeature );
  QgsAttributes src = feature.attributes();
  QgsAttributes dst = feature.attributes();

  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
    if ( eww && dst.count() > eww->fieldIdx() )
    {
      QVariant dstVar = dst.at( eww->fieldIdx() );
      QVariant srcVar = eww->value();
      // need to check dstVar.isNull() != srcVar.isNull()
      // otherwise if dstVar=NULL and scrVar=0, then dstVar = srcVar
      if (( dstVar != srcVar || dstVar.isNull() != srcVar.isNull() ) && srcVar.isValid() && !mLayer->editFormConfig()->readOnly( eww->fieldIdx() ) )
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

void QgsAttributeForm::clearInvalidConstraintsMessage()
{
  mInvalidConstraintMessage->clear();
  mInvalidConstraintMessage->setStyleSheet( QString() );
}

void QgsAttributeForm::displayInvalidConstraintMessage( const QStringList &f,
    const QStringList &d )
{
  clearInvalidConstraintsMessage();

  // show only the third first errors (to avoid a too long label)
  int max = 3;
  int size = f.size() > max ? max : f.size();
  QString descriptions;
  for ( int i = 0; i < size; i++ )
    descriptions += QString( "<li>%1: <i>%2</i></li>" ).arg( f[i] ).arg( d[i] );

  QString icPath = QgsApplication::iconPath( "/mIconWarn.png" );

  QString title = QString( "<img src=\"%1\">     <b>%2:" ).arg( icPath ).arg( tr( "Invalid fields" ) );
  QString msg = QString( "%1</b><ul>%2</ul>" ).arg( title ).arg( descriptions ) ;

  mInvalidConstraintMessage->setText( msg );
  mInvalidConstraintMessage->setStyleSheet( "QLabel { background-color : #ffc800; }" );
}

bool QgsAttributeForm::currentFormValidConstraints( QStringList &invalidFields,
    QStringList &descriptions )
{
  bool valid( true );

  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
    if ( eww )
    {
      if ( ! eww->isValidConstraint() )
      {
        invalidFields.append( eww->field().name() );

        QString desc = eww->layer()->editFormConfig()->expressionDescription( eww->fieldIdx() );
        descriptions.append( desc );

        valid = false; // continue to get all invalif fields
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
      int idx = mFeature.fields()->indexFromName( layer()->fields().at( i ).name() );
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

void QgsAttributeForm::onConstraintStatusChanged( const QString& constraint,
    const QString &description, const QString& err, bool ok )
{
  QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( sender() );
  Q_ASSERT( eww );

  QLabel* buddy = mBuddyMap.value( eww->widget() );

  if ( buddy )
  {
    QString tooltip = tr( "Description: " ) + description + "\n" +
                      tr( "Raw expression: " ) + constraint + "\n" + tr( "Constraint: " ) + err;
    buddy->setToolTip( tooltip );

    if ( !buddy->property( "originalText" ).isValid() )
      buddy->setProperty( "originalText", buddy->text() );

    QString text = buddy->property( "originalText" ).toString();

    if ( !ok )
    {
      // not good
      buddy->setText( QString( "%1<font color=\"red\">*</font>" ).arg( text ) );
    }
    else
    {
      // good
      buddy->setText( QString( "%1<font color=\"green\">*</font>" ).arg( text ) );
    }
  }
}

void QgsAttributeForm::constraintDependencies( QgsEditorWidgetWrapper *w,
    QList<QgsEditorWidgetWrapper*> &wDeps )
{
  QString name =  w->field().name();

  // for each widget in the current form
  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    // get the wrapper
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
    if ( eww )
    {
      // compare name to not compare w to itself
      QString ewwName = eww->field().name();
      if ( name != ewwName )
      {
        // get expression and referencedColumns
        QgsExpression expr = eww->layer()->editFormConfig()->expression( eww->fieldIdx() );

        Q_FOREACH ( const QString& colName, expr.referencedColumns() )
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
                      || mMode == AddFeatureMode
                      || mMode == MultiEditMode ) && mLayer->isEditable();

  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    bool fieldEditable = true;
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
    if ( eww )
    {
      fieldEditable = !mLayer->editFormConfig()->readOnly( eww->fieldIdx() ) &&
                      (( mLayer->dataProvider() && layer()->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) ||
                       FID_IS_NEW( mFeature.id() ) );
    }
    ww->setEnabled( isEditable && fieldEditable );
  }

  // push a message and disable the OK button if constraints are invalid
  clearInvalidConstraintsMessage();

  if ( mMode != SearchMode )
  {
    QStringList invalidFields, descriptions;
    bool validConstraint = currentFormValidConstraints( invalidFields, descriptions );

    if ( ! validConstraint )
      displayInvalidConstraintMessage( invalidFields, descriptions );

    isEditable = isEditable & validConstraint;
  }

  // change ok button status
  QPushButton* okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
    okButton->setEnabled( isEditable );
}

void QgsAttributeForm::init()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  // Cleanup of any previously shown widget, we start from scratch
  QWidget* formWidget = nullptr;

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

  while ( QWidget* w = this->findChild<QWidget*>() )
  {
    delete w;
  }
  delete layout();

  QVBoxLayout* vl = new QVBoxLayout();
  vl->setMargin( 0 );
  vl->setContentsMargins( 0, 0, 0, 0 );
  mMessageBar = new QgsMessageBar( this );
  mMessageBar->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
  vl->addWidget( mMessageBar );

  mInvalidConstraintMessage = new QLabel( this );
  vl->addWidget( mInvalidConstraintMessage );

  setLayout( vl );

  // Get a layout
  QGridLayout* layout = new QGridLayout();
  QWidget* container = new QWidget();
  container->setLayout( layout );
  vl->addWidget( container );

  mFormEditorWidgets.clear();

  // a bar to warn the user with non-blocking messages
  setContentsMargins( 0, 0, 0, 0 );

  // Try to load Ui-File for layout
  if ( mContext.allowCustomUi() && mLayer->editFormConfig()->layout() == QgsEditFormConfig::UiFileLayout &&
       !mLayer->editFormConfig()->uiForm().isEmpty() )
  {
    QFile file( mLayer->editFormConfig()->uiForm() );

    if ( file.open( QFile::ReadOnly ) )
    {
      QUiLoader loader;

      QFileInfo fi( mLayer->editFormConfig()->uiForm() );
      loader.setWorkingDirectory( fi.dir() );
      formWidget = loader.load( &file, this );
      formWidget->setWindowFlags( Qt::Widget );
      layout->addWidget( formWidget );
      formWidget->show();
      file.close();
      mButtonBox = findChild<QDialogButtonBox*>();
      createWrappers();

      formWidget->installEventFilter( this );
    }
  }

  QTabWidget* tabWidget = nullptr;

  // Tab layout
  if ( !formWidget && mLayer->editFormConfig()->layout() == QgsEditFormConfig::TabLayout )
  {
    int row = 0;
    int column = 0;
    int columnCount = 1;

    Q_FOREACH ( QgsAttributeEditorElement* widgDef, mLayer->editFormConfig()->tabs() )
    {
      if ( widgDef->type() == QgsAttributeEditorElement::AeTypeContainer )
      {
        QgsAttributeEditorContainer* containerDef = dynamic_cast<QgsAttributeEditorContainer*>( widgDef );
        if ( !containerDef )
          continue;

        if ( containerDef->isGroupBox() )
        {
          tabWidget = nullptr;
          WidgetInfo widgetInfo = createWidgetFromDef( widgDef, formWidget, mLayer, mContext );
          layout->addWidget( widgetInfo.widget, row, column, 1, 2 );
          column += 2;
        }
        else
        {
          if ( !tabWidget )
          {
            tabWidget = new QTabWidget();
            layout->addWidget( tabWidget, row, column, 1, 2 );
            column += 2;
          }

          QWidget* tabPage = new QWidget( tabWidget );

          tabWidget->addTab( tabPage, widgDef->name() );
          QGridLayout* tabPageLayout = new QGridLayout();
          tabPage->setLayout( tabPageLayout );

          WidgetInfo widgetInfo = createWidgetFromDef( widgDef, tabPage, mLayer, mContext );
          tabPageLayout->addWidget( widgetInfo.widget );
        }
      }
      else
      {
        tabWidget = nullptr;
        WidgetInfo widgetInfo = createWidgetFromDef( widgDef, container, mLayer, mContext );
        QLabel* label = new QLabel( widgetInfo.labelText );
        if ( columnCount > 1 && !widgetInfo.labelOnTop )
        {
          label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
        }

        label->setBuddy( widgetInfo.widget );

        if ( widgetInfo.labelOnTop )
        {
          QVBoxLayout* c = new QVBoxLayout();
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
  if ( !formWidget )
  {
    formWidget = new QWidget( this );
    QGridLayout* gridLayout = new QGridLayout( formWidget );
    formWidget->setLayout( gridLayout );

    if ( mContext.formMode() != QgsAttributeEditorContext::Embed )
    {
      // put the form into a scroll area to nicely handle cases with lots of attributes
      QScrollArea* scrollArea = new QScrollArea( this );
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
    Q_FOREACH ( const QgsField& field, mLayer->fields().toList() )
    {
      int idx = mLayer->fieldNameIndex( field.name() );
      if ( idx < 0 )
        continue;

      //show attribute alias if available
      QString fieldName = mLayer->attributeDisplayName( idx );

      const QString widgetType = mLayer->editFormConfig()->widgetType( idx );

      if ( widgetType == "Hidden" )
        continue;

      const QgsEditorWidgetConfig widgetConfig = mLayer->editFormConfig()->widgetConfig( idx );
      bool labelOnTop = mLayer->editFormConfig()->labelOnTop( idx );

      // This will also create the widget
      QLabel *l = new QLabel( fieldName );
      QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, idx, widgetConfig, nullptr, this, mContext );

      QWidget* w = nullptr;
      if ( eww )
      {
        QgsAttributeFormEditorWidget* formWidget = new QgsAttributeFormEditorWidget( eww, this );
        w = formWidget;
        mFormEditorWidgets.insert( idx, formWidget );
        formWidget->createSearchWidgetWrappers( widgetType, idx, widgetConfig, mContext );

        l->setBuddy( eww->widget() );
      }
      else
      {
        w = new QLabel( QString( "<p style=\"color: red; font-style: italic;\">Failed to create widget with type '%1'</p>" ).arg( widgetType ) );
      }


      if ( w )
        w->setObjectName( field.name() );

      if ( eww )
        addWidgetWrapper( eww );

      if ( labelOnTop )
      {
        gridLayout->addWidget( l, row++, 0, 1, 2 );
        gridLayout->addWidget( w, row++, 0, 1, 2 );
      }
      else
      {
        gridLayout->addWidget( l, row, 0 );
        gridLayout->addWidget( w, row++, 1 );
      }
    }

    Q_FOREACH ( const QgsRelation& rel, QgsProject::instance()->relationManager()->referencedRelations( mLayer ) )
    {
      QgsRelationWidgetWrapper* rww = new QgsRelationWidgetWrapper( mLayer, rel, nullptr, this );
      QgsEditorWidgetConfig cfg = mLayer->editFormConfig()->widgetConfig( rel.id() );
      rww->setConfig( cfg );
      rww->setContext( mContext );
      gridLayout->addWidget( rww->widget(), row++, 0, 1, 2 );
      mWidgets.append( rww );
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
    mButtonBox->setObjectName( "buttonBox" );
    layout->addWidget( mButtonBox, layout->rowCount(), 0, 1, layout->columnCount() );
  }
  mButtonBox->setVisible( buttonBoxVisible );

  if ( !mSearchButtonBox )
  {
    mSearchButtonBox = new QWidget();
    QHBoxLayout* boxLayout = new QHBoxLayout();
    boxLayout->setMargin( 0 );
    boxLayout->setContentsMargins( 0, 0, 0, 0 );
    mSearchButtonBox->setLayout( boxLayout );
    mSearchButtonBox->setObjectName( "searchButtonBox" );

    QPushButton* clearButton = new QPushButton( tr( "&Reset form" ), mSearchButtonBox );
    connect( clearButton, SIGNAL( clicked( bool ) ), this, SLOT( resetSearch() ) );
    boxLayout->addWidget( clearButton );
    boxLayout->addStretch( 1 );

    QToolButton* selectButton = new QToolButton();
    selectButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    selectButton->setText( tr( "&Select features" ) );
    selectButton->setPopupMode( QToolButton::MenuButtonPopup );
    connect( selectButton, SIGNAL( clicked( bool ) ), this, SLOT( searchSetSelection() ) );
    QMenu* selectMenu = new QMenu( selectButton );
    QAction* selectAction = new QAction( tr( "Select features" ), selectMenu );
    connect( selectAction, SIGNAL( triggered( bool ) ), this, SLOT( searchSetSelection() ) );
    selectMenu->addAction( selectAction );
    QAction* addSelectAction = new QAction( tr( "Add to current selection" ), selectMenu );
    connect( addSelectAction, SIGNAL( triggered( bool ) ), this, SLOT( searchAddToSelection() ) );
    selectMenu->addAction( addSelectAction );
    QAction* filterSelectAction = new QAction( tr( "Filter current selection" ), selectMenu );
    connect( filterSelectAction, SIGNAL( triggered( bool ) ), this, SLOT( searchIntersectSelection() ) );
    selectMenu->addAction( filterSelectAction );
    QAction* deselectAction = new QAction( tr( "Remove from current selection" ), selectMenu );
    connect( deselectAction, SIGNAL( triggered( bool ) ), this, SLOT( searchRemoveFromSelection() ) );
    selectMenu->addAction( deselectAction );
    selectButton->setMenu( selectMenu );
    boxLayout->addWidget( selectButton );

    if ( mContext.formMode() == QgsAttributeEditorContext::Embed )
    {
      QToolButton* filterButton = new QToolButton();
      filterButton->setText( tr( "Filter features" ) );
      filterButton->setPopupMode( QToolButton::MenuButtonPopup );
      filterButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
      connect( filterButton, SIGNAL( clicked( bool ) ), this, SLOT( filterTriggered() ) );
      QMenu* filterMenu = new QMenu( filterButton );
      QAction* filterAndAction = new QAction( tr( "Filter within (\"AND\")" ), filterMenu );
      connect( filterAndAction, SIGNAL( triggered( bool ) ), this, SLOT( filterAndTriggered() ) );
      filterMenu->addAction( filterAndAction );
      QAction* filterOrAction = new QAction( tr( "Extend filter (\"OR\")" ), filterMenu );
      connect( filterOrAction, SIGNAL( triggered( bool ) ), this, SLOT( filterOrTriggered() ) );
      filterMenu->addAction( filterOrAction );
      filterButton->setMenu( filterMenu );
      boxLayout->addWidget( filterButton );
    }
    else
    {
      QPushButton* closeButton = new QPushButton( tr( "Close" ), mSearchButtonBox );
      connect( closeButton, SIGNAL( clicked( bool ) ), this, SIGNAL( closed() ) );
      closeButton->setShortcut( Qt::Key_Escape );
      boxLayout->addWidget( closeButton );
    }

    layout->addWidget( mSearchButtonBox );
  }
  mSearchButtonBox->setVisible( mMode == SearchMode );

  afterWidgetInit();

  connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( resetValues() ) );

  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( synchronizeEnabledState() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( synchronizeEnabledState() ) );

  Q_FOREACH ( QgsAttributeFormInterface* iface, mInterfaces )
  {
    iface->initForm();
  }

  if ( mContext.formMode() == QgsAttributeEditorContext::Embed || mMode == SearchMode )
  {
    hideButtonBox();
  }

  QApplication::restoreOverrideCursor();
}

void QgsAttributeForm::cleanPython()
{
  if ( !mPyFormVarName.isNull() )
  {
    QString expr = QString( "if locals().has_key('%1'): del %1\n" ).arg( mPyFormVarName );
    QgsPythonRunner::run( expr );
  }
}

void QgsAttributeForm::initPython()
{
  cleanPython();

  // Init Python, if init function is not empty and the combo indicates
  // the source for the function code
  if ( !mLayer->editFormConfig()->initFunction().isEmpty()
       && mLayer->editFormConfig()->initCodeSource() != QgsEditFormConfig::CodeSourceNone )
  {

    QString initFunction = mLayer->editFormConfig()->initFunction();
    QString initFilePath = mLayer->editFormConfig()->initFilePath();
    QString initCode;

    switch ( mLayer->editFormConfig()->initCodeSource() )
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
            QgsLogger::warning( QString( "The external python file path %1 could not be opened!" ).arg( initFilePath ) );
          }
        }
        else
        {
          QgsLogger::warning( QString( "The external python file path is empty!" ) );
        }
        break;

      case QgsEditFormConfig::CodeSourceDialog:
        initCode = mLayer->editFormConfig()->initCode();
        if ( initCode.isEmpty() )
        {
          QgsLogger::warning( QString( "The python code provided in the dialog is empty!" ) );
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

    QgsPythonRunner::run( "import inspect" );
    QString numArgs;

    // Check for eval result
    if ( QgsPythonRunner::eval( QString( "len(inspect.getargspec(%1)[0])" ).arg( initFunction ), numArgs ) )
    {
      static int sFormId = 0;
      mPyFormVarName = QString( "_qgis_featureform_%1_%2" ).arg( mFormNr ).arg( sFormId++ );

      QString form = QString( "%1 = sip.wrapinstance( %2, qgis.gui.QgsAttributeForm )" )
                     .arg( mPyFormVarName )
                     .arg(( unsigned long ) this );

      QgsPythonRunner::run( form );

      QgsDebugMsg( QString( "running featureForm init: %1" ).arg( mPyFormVarName ) );

      // Legacy
      if ( numArgs == "3" )
      {
        addInterface( new QgsAttributeFormLegacyInterface( initFunction, mPyFormVarName, this ) );
      }
      else
      {
        // If we get here, it means that the function doesn't accept three arguments
        QMessageBox msgBox;
        msgBox.setText( tr( "The python init function (<code>%1</code>) does not accept three arguments as expected!<br>Please check the function name in the  <b>Fields</b> tab of the layer properties." ).arg( initFunction ) );
        msgBox.exec();
#if 0
        QString expr = QString( "%1(%2)" )
                       .arg( mLayer->editFormInit() )
                       .arg( mPyFormVarName );
        QgsAttributeFormInterface* iface = QgsPythonRunner::evalToSipObject<QgsAttributeFormInterface*>( expr, "QgsAttributeFormInterface" );
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

QgsAttributeForm::WidgetInfo QgsAttributeForm::createWidgetFromDef( const QgsAttributeEditorElement* widgetDef, QWidget* parent, QgsVectorLayer* vl, QgsAttributeEditorContext& context )
{
  WidgetInfo newWidgetInfo;

  switch ( widgetDef->type() )
  {
    case QgsAttributeEditorElement::AeTypeField:
    {
      const QgsAttributeEditorField* fieldDef = dynamic_cast<const QgsAttributeEditorField*>( widgetDef );
      if ( !fieldDef )
        break;

      int fldIdx = vl->fieldNameIndex( fieldDef->name() );
      if ( fldIdx < vl->fields().count() && fldIdx >= 0 )
      {
        const QString widgetType = mLayer->editFormConfig()->widgetType( fldIdx );
        const QgsEditorWidgetConfig widgetConfig = mLayer->editFormConfig()->widgetConfig( fldIdx );

        QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, fldIdx, widgetConfig, nullptr, this, mContext );
        QgsAttributeFormEditorWidget* w = new QgsAttributeFormEditorWidget( eww, this );
        mFormEditorWidgets.insert( fldIdx, w );

        w->createSearchWidgetWrappers( widgetType, fldIdx, widgetConfig, mContext );

        newWidgetInfo.widget = w;
        addWidgetWrapper( eww );

        newWidgetInfo.widget->setObjectName( mLayer->fields().at( fldIdx ).name() );
      }

      newWidgetInfo.labelOnTop = mLayer->editFormConfig()->labelOnTop( fieldDef->idx() );
      newWidgetInfo.labelText = mLayer->attributeDisplayName( fieldDef->idx() );

      break;
    }

    case QgsAttributeEditorElement::AeTypeRelation:
    {
      const QgsAttributeEditorRelation* relDef = dynamic_cast<const QgsAttributeEditorRelation*>( widgetDef );

      QgsRelationWidgetWrapper* rww = new QgsRelationWidgetWrapper( mLayer, relDef->relation(), nullptr, this );
      QgsEditorWidgetConfig cfg = mLayer->editFormConfig()->widgetConfig( relDef->relation().id() );
      rww->setConfig( cfg );
      rww->setContext( context );
      newWidgetInfo.widget = rww->widget();
      mWidgets.append( rww );
      newWidgetInfo.labelText = QString::null;
      newWidgetInfo.labelOnTop = true;
      break;
    }

    case QgsAttributeEditorElement::AeTypeContainer:
    {
      const QgsAttributeEditorContainer* container = dynamic_cast<const QgsAttributeEditorContainer*>( widgetDef );
      if ( !container )
        break;

      int columnCount = container->columnCount();

      if ( columnCount <= 0 )
        columnCount = 1;

      QWidget* myContainer;
      if ( container->isGroupBox() )
      {
        QGroupBox* groupBox = new QGroupBox( parent );
        groupBox->setTitle( container->name() );
        myContainer = groupBox;
        newWidgetInfo.widget = myContainer;
      }
      else
      {
        QScrollArea *scrollArea = new QScrollArea( parent );

        myContainer = new QWidget( scrollArea );

        scrollArea->setWidget( myContainer );
        scrollArea->setWidgetResizable( true );
        scrollArea->setFrameShape( QFrame::NoFrame );

        newWidgetInfo.widget = scrollArea;
      }

      QGridLayout* gbLayout = new QGridLayout();
      myContainer->setLayout( gbLayout );

      int row = 0;
      int column = 0;

      QList<QgsAttributeEditorElement*> children = container->children();

      Q_FOREACH ( QgsAttributeEditorElement* childDef, children )
      {
        WidgetInfo widgetInfo = createWidgetFromDef( childDef, myContainer, vl, context );

        if ( widgetInfo.labelText.isNull() )
        {
          gbLayout->addWidget( widgetInfo.widget, row, column, 1, 2 );
          column += 2;
        }
        else
        {
          QLabel* mypLabel = new QLabel( widgetInfo.labelText );
          if ( columnCount > 1 && !widgetInfo.labelOnTop )
          {
            mypLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
          }

          mypLabel->setBuddy( widgetInfo.widget );

          if ( widgetInfo.labelOnTop )
          {
            QVBoxLayout* c = new QVBoxLayout();
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
      QWidget* spacer = new QWidget();
      spacer->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
      gbLayout->addWidget( spacer, ++row, 0 );
      gbLayout->setRowStretch( row, 1 );

      newWidgetInfo.labelText = QString::null;
      newWidgetInfo.labelOnTop = true;
      break;
    }

    default:
      QgsDebugMsg( "Unknown attribute editor widget type encountered..." );
      break;
  }

  return newWidgetInfo;
}

void QgsAttributeForm::addWidgetWrapper( QgsEditorWidgetWrapper* eww )
{
  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* meww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
    if ( meww )
    {
      if ( meww->field() == eww->field() )
      {
        connect( meww, SIGNAL( valueChanged( QVariant ) ), eww, SLOT( setValue( QVariant ) ) );
        connect( eww, SIGNAL( valueChanged( QVariant ) ), meww, SLOT( setValue( QVariant ) ) );
        break;
      }
    }
  }

  mWidgets.append( eww );
}

void QgsAttributeForm::createWrappers()
{
  QList<QWidget*> myWidgets = findChildren<QWidget*>();
  const QList<QgsField> fields = mLayer->fields().toList();

  Q_FOREACH ( QWidget* myWidget, myWidgets )
  {
    // Check the widget's properties for a relation definition
    QVariant vRel = myWidget->property( "qgisRelation" );
    if ( vRel.isValid() )
    {
      QgsRelationManager* relMgr = QgsProject::instance()->relationManager();
      QgsRelation relation = relMgr->relation( vRel.toString() );
      if ( relation.isValid() )
      {
        QgsRelationWidgetWrapper* rww = new QgsRelationWidgetWrapper( mLayer, relation, myWidget, this );
        rww->setConfig( mLayer->editFormConfig()->widgetConfig( relation.id() ) );
        rww->setContext( mContext );
        rww->widget(); // Will initialize the widget
        mWidgets.append( rww );
      }
    }
    else
    {
      Q_FOREACH ( const QgsField& field, fields )
      {
        if ( field.name() == myWidget->objectName() )
        {
          const QString widgetType = mLayer->editFormConfig()->widgetType( field.name() );
          const QgsEditorWidgetConfig widgetConfig = mLayer->editFormConfig()->widgetConfig( field.name() );
          int idx = mLayer->fieldNameIndex( field.name() );

          QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, idx, widgetConfig, myWidget, this, mContext );
          addWidgetWrapper( eww );
        }
      }
    }
  }
}

void QgsAttributeForm::afterWidgetInit()
{
  bool isFirstEww = true;

  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );

    if ( eww )
    {
      if ( isFirstEww )
      {
        setFocusProxy( eww->widget() );
        isFirstEww = false;
      }

      connect( eww, SIGNAL( valueChanged( const QVariant& ) ), this, SLOT( onAttributeChanged( const QVariant& ) ) );
      connect( eww, SIGNAL( constraintStatusChanged( QString, QString, QString, bool ) ),
               this, SLOT( onConstraintStatusChanged( QString, QString, QString, bool ) ) );
    }
  }

  // Update buddy widget list
  mBuddyMap.clear();
  QList<QLabel*> labels = findChildren<QLabel*>();

  Q_FOREACH ( QLabel* label, labels )
  {
    if ( label->buddy() )
      mBuddyMap.insert( label->buddy(), label );
  }
}


bool QgsAttributeForm::eventFilter( QObject* object, QEvent* e )
{
  Q_UNUSED( object )

  if ( e->type() == QEvent::KeyPress )
  {
    QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>( e );
    if ( keyEvent && keyEvent->key() == Qt::Key_Escape )
    {
      // Re-emit to this form so it will be forwarded to parent
      event( e );
      return true;
    }
  }

  return false;
}

void QgsAttributeForm::scanForEqualAttributes( QgsFeatureIterator& fit, QSet< int >& mixedValueFields, QHash< int, QVariant >& fieldSharedValues ) const
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
    case SingleEditMode:
    case AddFeatureMode:
    case SearchMode:
      break;

    case MultiEditMode:
      resetMultiEdit( true );
      break;
  }
}

void QgsAttributeForm::setMultiEditFeatureIds( const QgsFeatureIds& fids )
{
  mIsSettingMultiEditFeatures = true;
  mMultiEditFeatureIds = fids;

  if ( fids.isEmpty() )
  {
    // no selected features
    QMap< int, QgsAttributeFormEditorWidget* >::const_iterator wIt = mFormEditorWidgets.constBegin();
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
    if ( QgsAttributeFormEditorWidget* w = mFormEditorWidgets.value( field, nullptr ) )
    {
      w->initialize( firstFeature.attribute( field ), true );
    }
  }
  QHash< int, QVariant >::const_iterator sharedValueIt = fieldSharedValues.constBegin();
  for ( ; sharedValueIt != fieldSharedValues.constEnd(); ++sharedValueIt )
  {
    if ( QgsAttributeFormEditorWidget* w = mFormEditorWidgets.value( sharedValueIt.key(), nullptr ) )
    {
      w->initialize( sharedValueIt.value(), false );
    }
  }
  mIsSettingMultiEditFeatures = false;
}

void QgsAttributeForm::setMessageBar( QgsMessageBar* messageBar )
{
  if ( mOwnsMessageBar )
    delete mMessageBar;
  mOwnsMessageBar = false;
  mMessageBar = messageBar;
}

int QgsAttributeForm::messageTimeout()
{
  QSettings settings;
  return settings.value( "/qgis/messageTimeout", 5 ).toInt();
}
