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

int QgsAttributeForm::sFormCounter = 0;

QgsAttributeForm::QgsAttributeForm( QgsVectorLayer* vl, const QgsFeature &feature, const QgsAttributeEditorContext &context, QWidget* parent )
    : QWidget( parent )
    , mLayer( vl )
    , mMessageBar( nullptr )
    , mMultiEditUnsavedMessageBarItem( nullptr )
    , mMultiEditMessageBarItem( nullptr )
    , mContext( context )
    , mButtonBox( nullptr )
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

  // Using attributeAdded() attributeDeleted() are not emitted on all fields changes (e.g. layer fields changed,
  // joined fields changed) -> use updatedFields() instead
#if 0
  connect( vl, SIGNAL( attributeAdded( int ) ), this, SLOT( onAttributeAdded( int ) ) );
  connect( vl, SIGNAL( attributeDeleted( int ) ), this, SLOT( onAttributeDeleted( int ) ) );
#endif
  connect( vl, SIGNAL( updatedFields() ), this, SLOT( onUpdatedFields() ) );
  connect( vl, SIGNAL( beforeAddingExpressionField( QString ) ), this, SLOT( preventFeatureRefresh() ) );
  connect( vl, SIGNAL( beforeRemovingExpressionField( int ) ), this, SLOT( preventFeatureRefresh() ) );
  connect( vl, SIGNAL( selectionChanged() ), this, SLOT( layerSelectionChanged() ) );
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

#if 0
      case QgsAttributeForm::SearchMode:
        w->setMode( QgsAttributeFormEditorWidget::SearchMode );
        break;
#endif
    }
  }

  switch ( mode )
  {
    case QgsAttributeForm::SingleEditMode:
      setFeature( mFeature );
      break;

    case QgsAttributeForm::AddFeatureMode:
      synchronizeEnabledState();
      break;

    case QgsAttributeForm::MultiEditMode:
      resetMultiEdit( false );
      synchronizeEnabledState();
      break;
  }

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
        if (( dstVar != srcVar || dstVar.isNull() != srcVar.isNull() ) && srcVar.isValid() && !mLayer->editFormConfig()->readOnly( eww->fieldIdx() ) )
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
  }
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

  qDeleteAll( mWidgets );
  mWidgets.clear();

  while ( QWidget* w = this->findChild<QWidget*>() )
  {
    delete w;
  }
  delete layout();

  // Get a layout
  QGridLayout* layout = new QGridLayout();
  setLayout( layout );

  mFormEditorWidgets.clear();

  // a bar to warn the user with non-blocking messages
  setContentsMargins( 0, 0, 0, 0 );
  mMessageBar = new QgsMessageBar( this );
  mMessageBar->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
  layout->addWidget( mMessageBar, 0, 0, 1, 1 );

  // Try to load Ui-File for layout
  if ( mLayer->editFormConfig()->layout() == QgsEditFormConfig::UiFileLayout && !mLayer->editFormConfig()->uiForm().isEmpty() )
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

  // Tab layout
  if ( !formWidget && mLayer->editFormConfig()->layout() == QgsEditFormConfig::TabLayout )
  {
    QTabWidget* tabWidget = new QTabWidget();
    layout->addWidget( tabWidget );

    Q_FOREACH ( QgsAttributeEditorElement* widgDef, mLayer->editFormConfig()->tabs() )
    {
      QWidget* tabPage = new QWidget( tabWidget );

      tabWidget->addTab( tabPage, widgDef->name() );
      QGridLayout* tabPageLayout = new QGridLayout();
      tabPage->setLayout( tabPageLayout );

      if ( widgDef->type() == QgsAttributeEditorElement::AeTypeContainer )
      {
        QgsAttributeEditorContainer* containerDef = dynamic_cast<QgsAttributeEditorContainer*>( widgDef );
        if ( !containerDef )
          continue;

        containerDef->setIsGroupBox( false ); // Toplevel widgets are tabs not groupboxes
        WidgetInfo widgetInfo = createWidgetFromDef( widgDef, tabPage, mLayer, mContext );
        tabPageLayout->addWidget( widgetInfo.widget );
      }
      else
      {
        QgsDebugMsg( "No support for fields in attribute editor on top level" );
      }
    }
    formWidget = tabWidget;
  }

  // Autogenerate Layout
  // If there is still no layout loaded (defined as autogenerate or other methods failed)
  if ( !formWidget )
  {
    formWidget = new QWidget( this );
    QGridLayout* gridLayout = new QGridLayout( formWidget );
    formWidget->setLayout( gridLayout );

    // put the form into a scroll area to nicely handle cases with lots of attributes

    QScrollArea* scrollArea = new QScrollArea( this );
    scrollArea->setWidget( formWidget );
    scrollArea->setWidgetResizable( true );
    scrollArea->setFrameShape( QFrame::NoFrame );
    scrollArea->setFrameShadow( QFrame::Plain );
    scrollArea->setFocusProxy( this );
    layout->addWidget( scrollArea );

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
      QWidget *l = new QLabel( fieldName );
      QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, idx, widgetConfig, nullptr, this, mContext );

      QWidget* w = nullptr;
      if ( eww )
      {
        w = new QgsAttributeFormEditorWidget( eww, this );
        mFormEditorWidgets.insert( idx, static_cast< QgsAttributeFormEditorWidget* >( w ) );
      }
      else
      {
        w = new QLabel( QString( "<p style=\"color: red; font-style: italic;\">Failed to create widget with type '%1'</p>" ).arg( widgetType ) );
      }

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
      gridLayout->addItem( spacerItem, row++, 0 );
    }
  }

  if ( !mButtonBox )
  {
    mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    mButtonBox->setObjectName( "buttonBox" );
    layout->addWidget( mButtonBox );
  }

  mButtonBox->setVisible( buttonBoxVisible );

  connectWrappers();

  connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( resetValues() ) );

  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( synchronizeEnabledState() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( synchronizeEnabledState() ) );

  Q_FOREACH ( QgsAttributeFormInterface* iface, mInterfaces )
  {
    iface->initForm();
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

        QWidget* w = new QgsAttributeFormEditorWidget( eww, this );
        mFormEditorWidgets.insert( fldIdx, static_cast< QgsAttributeFormEditorWidget* >( w ) );

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

void QgsAttributeForm::connectWrappers()
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
    }
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

int QgsAttributeForm::messageTimeout()
{
  QSettings settings;
  return settings.value( "/qgis/messageTimeout", 5 ).toInt();
}
