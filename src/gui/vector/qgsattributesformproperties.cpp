/***************************************************************************
    qgsattributesformproperties.cpp
    ---------------------
    begin                : August 2017
    copyright            : (C) 2017 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsactionmanager.h"
#include "qgsaddtaborgroup.h"
#include "qgsattributesformproperties.h"
#include "qgsattributetypedialog.h"
#include "qgsattributeformcontaineredit.h"
#include "qgsattributewidgetedit.h"
#include "qgsattributesforminitcode.h"
#include "qgsfieldcombobox.h"
#include "qgsqmlwidgetwrapper.h"
#include "qgshtmlwidgetwrapper.h"
#include "qgsapplication.h"
#include "qgscolorbutton.h"
#include "qgscodeeditorhtml.h"
#include "qgsexpressioncontextutils.h"
#include "qgsattributeeditoraction.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorqmlelement.h"
#include "qgsattributeeditorhtmlelement.h"
#include "qgssettingsregistrycore.h"


QgsAttributesFormProperties::QgsAttributesFormProperties( QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  if ( !layer )
    return;

  setupUi( this );

  mEditorLayoutComboBox->addItem( tr( "Autogenerate" ), QgsEditFormConfig::EditorLayout::GeneratedLayout );
  mEditorLayoutComboBox->addItem( tr( "Drag and Drop Designer" ), QgsEditFormConfig::EditorLayout::TabLayout );
  mEditorLayoutComboBox->addItem( tr( "Provide ui-file" ), QgsEditFormConfig::EditorLayout::UiFileLayout );

  // available widgets tree
  QGridLayout *availableWidgetsWidgetLayout = new QGridLayout;
  mAvailableWidgetsTree = new QgsAttributesDnDTree( mLayer );
  availableWidgetsWidgetLayout->addWidget( mAvailableWidgetsTree );
  availableWidgetsWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
  mAvailableWidgetsWidget->setLayout( availableWidgetsWidgetLayout );
  mAvailableWidgetsTree->setSelectionMode( QAbstractItemView::SelectionMode::ExtendedSelection );
  mAvailableWidgetsTree->setHeaderLabels( QStringList() << tr( "Available Widgets" ) );
  mAvailableWidgetsTree->setType( QgsAttributesDnDTree::Type::Drag );

  // form layout tree
  QGridLayout *formLayoutWidgetLayout = new QGridLayout;
  mFormLayoutTree = new QgsAttributesDnDTree( mLayer );
  mFormLayoutWidget->setLayout( formLayoutWidgetLayout );
  formLayoutWidgetLayout->addWidget( mFormLayoutTree );
  formLayoutWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
  mFormLayoutTree->setHeaderLabels( QStringList() << tr( "Form Layout" ) );
  mFormLayoutTree->setType( QgsAttributesDnDTree::Type::Drop );

  connect( mAvailableWidgetsTree, &QTreeWidget::itemSelectionChanged, this, &QgsAttributesFormProperties::onAttributeSelectionChanged );
  connect( mFormLayoutTree, &QTreeWidget::itemSelectionChanged, this, &QgsAttributesFormProperties::onFormLayoutSelectionChanged );
  connect( mAddTabOrGroupButton, &QAbstractButton::clicked, this, &QgsAttributesFormProperties::addTabOrGroupButton );
  connect( mRemoveTabOrGroupButton, &QAbstractButton::clicked, this, &QgsAttributesFormProperties::removeTabOrGroupButton );
  connect( mInvertSelectionButton, &QAbstractButton::clicked, this, &QgsAttributesFormProperties::onInvertSelectionButtonClicked );
  connect( mEditorLayoutComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsAttributesFormProperties::mEditorLayoutComboBox_currentIndexChanged );
  connect( pbnSelectEditForm, &QToolButton::clicked, this, &QgsAttributesFormProperties::pbnSelectEditForm_clicked );
  connect( mTbInitCode, &QPushButton::clicked, this, &QgsAttributesFormProperties::mTbInitCode_clicked );
}

void QgsAttributesFormProperties::init()
{
  initAvailableWidgetsTree();
  initFormLayoutTree();

  initLayoutConfig();
  initInitPython();
  initSuppressCombo();
}

void QgsAttributesFormProperties::initAvailableWidgetsTree()
{
  mAvailableWidgetsTree->clear();
  mAvailableWidgetsTree->setSortingEnabled( false );
  mAvailableWidgetsTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  mAvailableWidgetsTree->setAcceptDrops( false );
  mAvailableWidgetsTree->setDragDropMode( QAbstractItemView::DragOnly );

  //load Fields

  DnDTreeItemData catItemData = DnDTreeItemData( DnDTreeItemData::WidgetType, QStringLiteral( "Fields" ), QStringLiteral( "Fields" ) );
  QTreeWidgetItem *catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  const QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    const QgsField field = fields.at( i );
    DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Field, field.name(), field.name() );
    itemData.setShowLabel( true );

    FieldConfig cfg( mLayer, i );

    QTreeWidgetItem *item = mAvailableWidgetsTree->addItem( catitem, itemData, -1, fields.iconForField( i, true ) );

    item->setData( 0, FieldConfigRole, cfg );
    item->setData( 0, FieldNameRole, field.name() );

    QString tooltip;
    if ( !field.alias().isEmpty() )
      tooltip = tr( "%1 (%2)" ).arg( field.name(), field.alias() );
    else
      tooltip = field.name();
    item->setToolTip( 0, tooltip );
  }
  catitem->setExpanded( true );

  //load Relations
  catItemData = DnDTreeItemData( DnDTreeItemData::WidgetType, QStringLiteral( "Relations" ), tr( "Relations" ) );
  catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencedRelations( mLayer );

  for ( const QgsRelation &relation : relations )
  {
    DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Relation, relation.id(), relation.name() );
    itemData.setShowLabel( true );
    QTreeWidgetItem *item = mAvailableWidgetsTree->addItem( catitem, itemData );
    item->setData( 0, FieldNameRole, relation.id() );
  }
  catitem->setExpanded( true );

  // Form actions
  catItemData = DnDTreeItemData( DnDTreeItemData::WidgetType, QStringLiteral( "Actions" ), tr( "Actions" ) );
  catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  const QList<QgsAction> actions { mLayer->actions()->actions( ) };

  for ( const auto &action : std::as_const( actions ) )
  {
    if ( action.isValid() && action.runable() &&
         ( action.actionScopes().contains( QStringLiteral( "Feature" ) ) ||
           action.actionScopes().contains( QStringLiteral( "Layer" ) ) ) )
    {
      const QString actionTitle { action.shortTitle().isEmpty() ? action.name() : action.shortTitle() };
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Action, action.id().toString(), actionTitle );
      itemData.setShowLabel( true );
      mAvailableWidgetsTree->addItem( catitem, itemData );
    }
  }

  // QML/HTML widget
  catItemData = DnDTreeItemData( DnDTreeItemData::WidgetType, QStringLiteral( "Other" ), tr( "Other Widgets" ) );
  catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::QmlWidget, QStringLiteral( "QmlWidget" ), tr( "QML Widget" ) );
  itemData.setShowLabel( true );
  mAvailableWidgetsTree->addItem( catitem, itemData );

  auto itemDataHtml { DnDTreeItemData( DnDTreeItemData::HtmlWidget, QStringLiteral( "HtmlWidget" ), tr( "HTML Widget" ) ) };
  itemDataHtml.setShowLabel( true );
  mAvailableWidgetsTree->addItem( catitem, itemDataHtml );
  catitem ->setExpanded( true );
}

void QgsAttributesFormProperties::initFormLayoutTree()
{
  // tabs and groups info
  mFormLayoutTree->clear();
  mFormLayoutTree->setSortingEnabled( false );
  mFormLayoutTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  mFormLayoutTree->setSelectionMode( QAbstractItemView::SelectionMode::ExtendedSelection );
  mFormLayoutTree->setAcceptDrops( true );
  mFormLayoutTree->setDragDropMode( QAbstractItemView::DragDrop );

  const auto constTabs = mLayer->editFormConfig().tabs();
  for ( QgsAttributeEditorElement *wdg : constTabs )
  {
    loadAttributeEditorTreeItem( wdg, mFormLayoutTree->invisibleRootItem(), mFormLayoutTree );
  }
}


void QgsAttributesFormProperties::initSuppressCombo()
{
  if ( QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog.value() )
  {
    mFormSuppressCmbBx->addItem( tr( "Hide Form on Add Feature (global settings)" ) );
  }
  else
  {
    mFormSuppressCmbBx->addItem( tr( "Show Form on Add Feature (global settings)" ) );
  }
  mFormSuppressCmbBx->addItem( tr( "Hide Form on Add Feature" ) );
  mFormSuppressCmbBx->addItem( tr( "Show Form on Add Feature" ) );

  mFormSuppressCmbBx->setCurrentIndex( mLayer->editFormConfig().suppress() );
}

QgsExpressionContext QgsAttributesFormProperties::createExpressionContext() const
{
  QgsExpressionContext context;
  context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  return context;
}

void QgsAttributesFormProperties::initLayoutConfig()
{
  mEditorLayoutComboBox->setCurrentIndex( mEditorLayoutComboBox->findData( mLayer->editFormConfig().layout() ) );

  mEditorLayoutComboBox_currentIndexChanged( mEditorLayoutComboBox->currentIndex() );

  const QgsEditFormConfig cfg = mLayer->editFormConfig();
  mEditFormLineEdit->setText( cfg.uiForm() );
}

void QgsAttributesFormProperties::initInitPython()
{
  const QgsEditFormConfig cfg = mLayer->editFormConfig();

  mInitCodeSource = cfg.initCodeSource();
  mInitFunction = cfg.initFunction();
  mInitFilePath = cfg.initFilePath();
  mInitCode = cfg.initCode();

  if ( mInitCode.isEmpty() )
  {
    mInitCode.append( tr( "# -*- coding: utf-8 -*-\n\"\"\"\n"
                          "QGIS forms can have a Python function that is called when the form is\n"
                          "opened.\n"
                          "\n"
                          "Use this function to add extra logic to your forms.\n"
                          "\n"
                          "Enter the name of the function in the \"Python Init function\"\n"
                          "field.\n"
                          "An example follows:\n"
                          "\"\"\"\n"
                          "from qgis.PyQt.QtWidgets import QWidget\n\n"
                          "def my_form_open(dialog, layer, feature):\n"
                          "    geom = feature.geometry()\n"
                          "    control = dialog.findChild(QWidget, \"MyLineEdit\")\n" ) );
  }
}

void QgsAttributesFormProperties::loadAttributeTypeDialog()
{
  if ( mAvailableWidgetsTree->selectedItems().count() != 1 )
    return;

  QTreeWidgetItem *item = mAvailableWidgetsTree->selectedItems().at( 0 );

  const FieldConfig cfg = item->data( 0, FieldConfigRole ).value<FieldConfig>();
  const QString fieldName = item->data( 0, FieldNameRole ).toString();
  const int index = mLayer->fields().indexOf( fieldName );

  if ( index < 0 )
    return;

  mAttributeTypeDialog = new QgsAttributeTypeDialog( mLayer, index, mAttributeTypeFrame );

  const QgsFieldConstraints constraints = cfg.mFieldConstraints;

  mAttributeTypeDialog->setAlias( cfg.mAlias );
  mAttributeTypeDialog->setDataDefinedProperties( cfg.mDataDefinedProperties );
  mAttributeTypeDialog->setComment( cfg.mComment );
  mAttributeTypeDialog->setFieldEditable( cfg.mEditable );
  mAttributeTypeDialog->setLabelOnTop( cfg.mLabelOnTop );
  mAttributeTypeDialog->setReuseLastValues( cfg.mReuseLastValues );
  mAttributeTypeDialog->setNotNull( constraints.constraints() & QgsFieldConstraints::ConstraintNotNull );
  mAttributeTypeDialog->setNotNullEnforced( constraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintStrengthHard );
  mAttributeTypeDialog->setUnique( constraints.constraints() & QgsFieldConstraints::ConstraintUnique );
  mAttributeTypeDialog->setUniqueEnforced( constraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) == QgsFieldConstraints::ConstraintStrengthHard );

  QgsFieldConstraints::Constraints providerConstraints = QgsFieldConstraints::Constraints();
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintNotNull;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintUnique;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintExpression;
  mAttributeTypeDialog->setProviderConstraints( providerConstraints );

  mAttributeTypeDialog->setConstraintExpression( constraints.constraintExpression() );
  mAttributeTypeDialog->setConstraintExpressionDescription( constraints.constraintDescription() );
  mAttributeTypeDialog->setConstraintExpressionEnforced( constraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintStrengthHard );
  mAttributeTypeDialog->setDefaultValueExpression( mLayer->defaultValueDefinition( index ).expression() );
  mAttributeTypeDialog->setApplyDefaultValueOnUpdate( mLayer->defaultValueDefinition( index ).applyOnUpdate() );

  mAttributeTypeDialog->setEditorWidgetConfig( cfg.mEditorWidgetConfig );
  mAttributeTypeDialog->setEditorWidgetType( cfg.mEditorWidgetType );

  mAttributeTypeDialog->layout()->setContentsMargins( 0, 0, 0, 0 );
  mAttributeTypeFrame->layout()->setContentsMargins( 0, 0, 0, 0 );

  mAttributeTypeFrame->layout()->addWidget( mAttributeTypeDialog );
}


void QgsAttributesFormProperties::storeAttributeTypeDialog()
{
  if ( !mAttributeTypeDialog )
    return;

  if ( mAttributeTypeDialog->fieldIdx() < 0 || mAttributeTypeDialog->fieldIdx() >= mLayer->fields().count() )
    return;

  FieldConfig cfg;

  cfg.mComment = mLayer->fields().at( mAttributeTypeDialog->fieldIdx() ).comment();
  cfg.mEditable = mAttributeTypeDialog->fieldEditable();
  cfg.mLabelOnTop = mAttributeTypeDialog->labelOnTop();
  cfg.mReuseLastValues = mAttributeTypeDialog->reuseLastValues();
  cfg.mAlias = mAttributeTypeDialog->alias();
  cfg.mDataDefinedProperties = mAttributeTypeDialog->dataDefinedProperties();

  QgsFieldConstraints constraints;
  if ( mAttributeTypeDialog->notNull() )
  {
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull );
  }
  else if ( mAttributeTypeDialog->notNullFromProvider() )
  {
    constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
  }

  if ( mAttributeTypeDialog->unique() )
  {
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique );
  }
  else if ( mAttributeTypeDialog->uniqueFromProvider() )
  {
    constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
  }

  if ( !mAttributeTypeDialog->constraintExpression().isEmpty() )
  {
    constraints.setConstraint( QgsFieldConstraints::ConstraintExpression );
  }

  constraints.setConstraintExpression( mAttributeTypeDialog->constraintExpression(), mAttributeTypeDialog->constraintExpressionDescription() );

  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintNotNull, mAttributeTypeDialog->notNullEnforced() ?
                                     QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintUnique, mAttributeTypeDialog->uniqueEnforced() ?
                                     QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  constraints.setConstraintStrength( QgsFieldConstraints::ConstraintExpression, mAttributeTypeDialog->constraintExpressionEnforced() ?
                                     QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );

  cfg.mFieldConstraints = constraints;

  mLayer->setDefaultValueDefinition( mAttributeTypeDialog->fieldIdx(), QgsDefaultValue( mAttributeTypeDialog->defaultValueExpression(), mAttributeTypeDialog->applyDefaultValueOnUpdate() ) );

  cfg.mEditorWidgetType = mAttributeTypeDialog->editorWidgetType();
  cfg.mEditorWidgetConfig = mAttributeTypeDialog->editorWidgetConfig();

  const QString fieldName = mLayer->fields().at( mAttributeTypeDialog->fieldIdx() ).name();

  for ( auto itemIt = QTreeWidgetItemIterator( mAvailableWidgetsTree ); *itemIt; ++itemIt )
  {
    QTreeWidgetItem *item = *itemIt;
    if ( item->data( 0, FieldNameRole ).toString() == fieldName )
      item->setData( 0, FieldConfigRole, QVariant::fromValue<FieldConfig>( cfg ) );
  }
}

void QgsAttributesFormProperties::storeAttributeWidgetEdit()
{
  if ( !mAttributeWidgetEdit )
    return;

  mAttributeWidgetEdit->updateItemData();
}

void QgsAttributesFormProperties::loadAttributeWidgetEdit()
{
  if ( mFormLayoutTree->selectedItems().count() != 1 )
    return;

  QTreeWidgetItem *currentItem = mFormLayoutTree->selectedItems().at( 0 );
  mAttributeWidgetEdit = new QgsAttributeWidgetEdit( currentItem, this );
  mAttributeTypeFrame->layout()->setContentsMargins( 0, 0, 0, 0 );
  mAttributeTypeFrame->layout()->addWidget( mAttributeWidgetEdit );
}

void QgsAttributesFormProperties::loadInfoWidget( const QString &infoText )
{
  mInfoTextWidget = new QLabel( infoText );
  mAttributeTypeFrame->layout()->setContentsMargins( 0, 0, 0, 0 );
  mAttributeTypeFrame->layout()->addWidget( mInfoTextWidget );
}

void QgsAttributesFormProperties::storeAttributeContainerEdit()
{
  if ( !mAttributeContainerEdit )
    return;

  mAttributeContainerEdit->updateItemData();
}

void QgsAttributesFormProperties::loadAttributeContainerEdit()
{
  if ( mFormLayoutTree->selectedItems().count() != 1 )
    return;

  QTreeWidgetItem *currentItem = mFormLayoutTree->selectedItems().at( 0 );
  mAttributeContainerEdit = new QgsAttributeFormContainerEdit( currentItem, mLayer, this );
  mAttributeContainerEdit->registerExpressionContextGenerator( this );
  mAttributeContainerEdit->layout()->setContentsMargins( 0, 0, 0, 0 );
  mAttributeTypeFrame->layout()->setContentsMargins( 0, 0, 0, 0 );
  mAttributeTypeFrame->layout()->addWidget( mAttributeContainerEdit );
}

QTreeWidgetItem *QgsAttributesFormProperties::loadAttributeEditorTreeItem( QgsAttributeEditorElement *const widgetDef, QTreeWidgetItem *parent, QgsAttributesDnDTree *tree )
{
  QTreeWidgetItem *newWidget = nullptr;
  switch ( widgetDef->type() )
  {
    case QgsAttributeEditorElement::AeTypeField:
    {
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Field, widgetDef->name(), widgetDef->name() );
      itemData.setShowLabel( widgetDef->showLabel() );
      newWidget = tree->addItem( parent, itemData );
      break;
    }

    case QgsAttributeEditorElement::AeTypeAction:
    {
      const QgsAttributeEditorAction *actionEditor = static_cast<const QgsAttributeEditorAction *>( widgetDef );
      const QgsAction action { actionEditor->action( mLayer ) };
      if ( action.isValid() )
      {
        DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Action, action.id().toString(), action.shortTitle().isEmpty() ? action.name() : action.shortTitle() );
        itemData.setShowLabel( widgetDef->showLabel() );
        newWidget = tree->addItem( parent, itemData );
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Invalid form action" ) );
      }
      break;
    }

    case QgsAttributeEditorElement::AeTypeRelation:
    {
      const QgsAttributeEditorRelation *relationEditor = static_cast<const QgsAttributeEditorRelation *>( widgetDef );
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Relation, relationEditor->relation().id(), relationEditor->relation().name() );
      itemData.setShowLabel( widgetDef->showLabel() );

      RelationEditorConfiguration relEdConfig;
//      relEdConfig.buttons = relationEditor->visibleButtons();
      relEdConfig.mRelationWidgetType = relationEditor->relationWidgetTypeId();
      relEdConfig.mRelationWidgetConfig = relationEditor->relationEditorConfiguration();
      relEdConfig.nmRelationId = relationEditor->nmRelationId();
      relEdConfig.forceSuppressFormPopup = relationEditor->forceSuppressFormPopup();
      relEdConfig.label = relationEditor->label();
      itemData.setRelationEditorConfiguration( relEdConfig );
      newWidget = tree->addItem( parent, itemData );
      break;
    }

    case QgsAttributeEditorElement::AeTypeContainer:
    {
      DnDTreeItemData itemData( DnDTreeItemData::Container, widgetDef->name(), widgetDef->name() );
      itemData.setShowLabel( widgetDef->showLabel() );

      const QgsAttributeEditorContainer *container = static_cast<const QgsAttributeEditorContainer *>( widgetDef );
      if ( !container )
        break;

      itemData.setColumnCount( container->columnCount() );
      itemData.setShowAsGroupBox( container->isGroupBox() );
      itemData.setBackgroundColor( container->backgroundColor() );
      itemData.setVisibilityExpression( container->visibilityExpression() );
      itemData.setCollapsedExpression( container->collapsedExpression() );
      itemData.setCollapsed( container->collapsed() );
      newWidget = tree->addItem( parent, itemData );

      const QList<QgsAttributeEditorElement *> children = container->children();
      for ( QgsAttributeEditorElement *wdg : children )
      {
        loadAttributeEditorTreeItem( wdg, newWidget, tree );
      }
      break;
    }

    case QgsAttributeEditorElement::AeTypeQmlElement:
    {
      const QgsAttributeEditorQmlElement *qmlElementEditor = static_cast<const QgsAttributeEditorQmlElement *>( widgetDef );
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::QmlWidget, widgetDef->name(), widgetDef->name() );
      itemData.setShowLabel( widgetDef->showLabel() );
      QmlElementEditorConfiguration qmlEdConfig;
      qmlEdConfig.qmlCode = qmlElementEditor->qmlCode();
      itemData.setQmlElementEditorConfiguration( qmlEdConfig );
      newWidget = tree->addItem( parent, itemData );
      break;
    }

    case QgsAttributeEditorElement::AeTypeHtmlElement:
    {
      const QgsAttributeEditorHtmlElement *htmlElementEditor = static_cast<const QgsAttributeEditorHtmlElement *>( widgetDef );
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::HtmlWidget, widgetDef->name(), widgetDef->name() );
      itemData.setShowLabel( widgetDef->showLabel() );
      HtmlElementEditorConfiguration htmlEdConfig;
      htmlEdConfig.htmlCode = htmlElementEditor->htmlCode();
      itemData.setHtmlElementEditorConfiguration( htmlEdConfig );
      newWidget = tree->addItem( parent, itemData );
      break;
    }

    case QgsAttributeEditorElement::AeTypeInvalid:
    {
      QgsDebugMsg( QStringLiteral( "Not loading invalid attribute editor type..." ) );
      break;
    }
  }

  return newWidget;
}


void QgsAttributesFormProperties::onAttributeSelectionChanged()
{
  disconnect( mFormLayoutTree, &QTreeWidget::itemSelectionChanged, this, &QgsAttributesFormProperties::onFormLayoutSelectionChanged );
  loadAttributeSpecificEditor( mAvailableWidgetsTree, mFormLayoutTree );
  connect( mFormLayoutTree, &QTreeWidget::itemSelectionChanged, this, &QgsAttributesFormProperties::onFormLayoutSelectionChanged );
}

void QgsAttributesFormProperties::onFormLayoutSelectionChanged()
{
  // when the selection changes in the DnD layout, sync the main tree
  disconnect( mAvailableWidgetsTree, &QTreeWidget::itemSelectionChanged, this, &QgsAttributesFormProperties::onAttributeSelectionChanged );
  loadAttributeSpecificEditor( mFormLayoutTree, mAvailableWidgetsTree );
  connect( mAvailableWidgetsTree, &QTreeWidget::itemSelectionChanged, this, &QgsAttributesFormProperties::onAttributeSelectionChanged );
}

void QgsAttributesFormProperties::loadAttributeSpecificEditor( QgsAttributesDnDTree *emitter, QgsAttributesDnDTree *receiver )
{
  const QgsEditFormConfig::EditorLayout layout = mEditorLayoutComboBox->currentData().value<QgsEditFormConfig::EditorLayout>();

  if ( layout == QgsEditFormConfig::EditorLayout::TabLayout )
    storeAttributeWidgetEdit();
  storeAttributeTypeDialog();
  storeAttributeContainerEdit();

  clearAttributeTypeFrame();

  if ( emitter->selectedItems().count() != 1 )
  {
    receiver->clearSelection();
  }
  else
  {
    const DnDTreeItemData itemData = emitter->selectedItems().at( 0 )->data( 0, DnDTreeRole ).value<DnDTreeItemData>();
    switch ( itemData.type() )
    {
      case DnDTreeItemData::Relation:
      {
        receiver->selectFirstMatchingItem( itemData );
        if ( layout == QgsEditFormConfig::EditorLayout::TabLayout )
        {
          loadAttributeWidgetEdit();
        }
        else
        {
          loadInfoWidget( tr( "This configuration is available in the Drag and Drop Designer" ) );
        }
        break;
      }
      case DnDTreeItemData::Field:
      {
        receiver->selectFirstMatchingItem( itemData );
        if ( layout == QgsEditFormConfig::EditorLayout::TabLayout )
          loadAttributeWidgetEdit();
        loadAttributeTypeDialog();
        break;
      }
      case DnDTreeItemData::Container:
      {
        receiver->clearSelection();
        loadAttributeContainerEdit();
        break;
      }
      case DnDTreeItemData::Action:
      {
        receiver->selectFirstMatchingItem( itemData );
        const QgsAction action {mLayer->actions()->action( itemData.name() )};
        loadInfoWidget( action.html() );
        break;
      }
      case DnDTreeItemData::QmlWidget:
      case DnDTreeItemData::HtmlWidget:
      {
        if ( layout != QgsEditFormConfig::EditorLayout::TabLayout )
        {
          loadInfoWidget( tr( "This configuration is available with double-click in the Drag and Drop Designer" ) );
        }
        else
        {
          loadInfoWidget( tr( "This configuration is available with double-click" ) );
        }
        receiver->clearSelection();
        break;
      }
      case DnDTreeItemData::WidgetType:
      {
        receiver->clearSelection();
        break;
      }
    }
  }
}

void QgsAttributesFormProperties::clearAttributeTypeFrame()
{
  if ( mAttributeWidgetEdit )
  {
    mAttributeTypeFrame->layout()->removeWidget( mAttributeWidgetEdit );
    mAttributeWidgetEdit->deleteLater();
    mAttributeWidgetEdit = nullptr;
  }
  if ( mAttributeTypeDialog )
  {
    mAttributeTypeFrame->layout()->removeWidget( mAttributeTypeDialog );
    mAttributeTypeDialog->deleteLater();
    mAttributeTypeDialog = nullptr;
  }
  if ( mAttributeContainerEdit )
  {
    mAttributeTypeFrame->layout()->removeWidget( mAttributeContainerEdit );
    mAttributeContainerEdit->deleteLater();
    mAttributeContainerEdit = nullptr;
  }
  if ( mInfoTextWidget )
  {
    mAttributeTypeFrame->layout()->removeWidget( mInfoTextWidget );
    mInfoTextWidget->deleteLater();
    mInfoTextWidget = nullptr;
  }
}

void QgsAttributesFormProperties::onInvertSelectionButtonClicked( bool checked )
{
  Q_UNUSED( checked )
  const auto selectedItemList { mFormLayoutTree->selectedItems() };
  const auto rootItem { mFormLayoutTree->invisibleRootItem() };
  for ( int i = 0; i < rootItem->childCount(); ++i )
  {
    rootItem->child( i )->setSelected( ! selectedItemList.contains( rootItem->child( i ) ) );
  }
}

void QgsAttributesFormProperties::addTabOrGroupButton()
{
  QList<QgsAddTabOrGroup::TabPair> tabList;

  for ( QTreeWidgetItemIterator it( mFormLayoutTree ); *it; ++it )
  {
    const DnDTreeItemData itemData = ( *it )->data( 0, DnDTreeRole ).value<DnDTreeItemData>();
    if ( itemData.type() == DnDTreeItemData::Container )
    {
      tabList.append( QgsAddTabOrGroup::TabPair( itemData.name(), *it ) );
    }
  }
  QTreeWidgetItem *currentItem = mFormLayoutTree->selectedItems().value( 0 );
  QgsAddTabOrGroup addTabOrGroup( mLayer, tabList, currentItem, this );

  if ( !addTabOrGroup.exec() )
    return;

  const QString name = addTabOrGroup.name();
  if ( addTabOrGroup.tabButtonIsChecked() )
  {
    mFormLayoutTree->addContainer( mFormLayoutTree->invisibleRootItem(), name, addTabOrGroup.columnCount() );
  }
  else
  {
    QTreeWidgetItem *tabItem = addTabOrGroup.tab();
    mFormLayoutTree->addContainer( tabItem, name, addTabOrGroup.columnCount() );
  }
}

void QgsAttributesFormProperties::removeTabOrGroupButton()
{
  qDeleteAll( mFormLayoutTree->selectedItems() );
}


QgsAttributeEditorElement *QgsAttributesFormProperties::createAttributeEditorWidget( QTreeWidgetItem *item, QgsAttributeEditorElement *parent, bool forceGroup )
{
  QgsAttributeEditorElement *widgetDef = nullptr;

  const DnDTreeItemData itemData = item->data( 0, DnDTreeRole ).value<DnDTreeItemData>();

  switch ( itemData.type() )
  {
    //indexed here?
    case DnDTreeItemData::Field:
    {
      const int idx = mLayer->fields().lookupField( itemData.name() );
      widgetDef = new QgsAttributeEditorField( itemData.name(), idx, parent );
      break;
    }

    case DnDTreeItemData::Action:
    {
      const QgsAction action { mLayer->actions()->action( itemData.name() )};
      widgetDef = new QgsAttributeEditorAction( action, parent );
      break;
    }

    case DnDTreeItemData::Relation:
    {
      const QgsRelation relation = QgsProject::instance()->relationManager()->relation( itemData.name() );
      QgsAttributeEditorRelation *relDef = new QgsAttributeEditorRelation( relation, parent );
      const QgsAttributesFormProperties::RelationEditorConfiguration relationEditorConfig = itemData.relationEditorConfiguration();
      relDef->setRelationWidgetTypeId( relationEditorConfig.mRelationWidgetType );
      relDef->setRelationEditorConfiguration( relationEditorConfig.mRelationWidgetConfig );
      relDef->setNmRelationId( relationEditorConfig.nmRelationId );
      relDef->setForceSuppressFormPopup( relationEditorConfig.forceSuppressFormPopup );
      relDef->setLabel( relationEditorConfig.label );
      widgetDef = relDef;
      break;
    }

    case DnDTreeItemData::Container:
    {
      QgsAttributeEditorContainer *container = new QgsAttributeEditorContainer( item->text( 0 ), parent, itemData.backgroundColor() );
      container->setColumnCount( itemData.columnCount() );
      container->setIsGroupBox( forceGroup ? true : itemData.showAsGroupBox() );
      container->setCollapsed( itemData.collapsed() );
      container->setCollapsedExpression( itemData.collapsedExpression() );
      container->setVisibilityExpression( itemData.visibilityExpression() );
      container->setBackgroundColor( itemData.backgroundColor( ) );

      for ( int t = 0; t < item->childCount(); t++ )
      {
        QgsAttributeEditorElement *element { createAttributeEditorWidget( item->child( t ), container ) };
        if ( element )
          container->addChildElement( element );
      }

      widgetDef = container;
      break;
    }

    case DnDTreeItemData::QmlWidget:
    {
      QgsAttributeEditorQmlElement *element = new QgsAttributeEditorQmlElement( item->text( 0 ), parent );
      element->setQmlCode( itemData.qmlElementEditorConfiguration().qmlCode );
      widgetDef = element;
      break;
    }

    case DnDTreeItemData::HtmlWidget:
    {
      QgsAttributeEditorHtmlElement *element = new QgsAttributeEditorHtmlElement( item->text( 0 ), parent );
      element->setHtmlCode( itemData.htmlElementEditorConfiguration().htmlCode );
      widgetDef = element;
      break;
    }

    case DnDTreeItemData::WidgetType:
      break;

  }

  if ( widgetDef )
    widgetDef->setShowLabel( itemData.showLabel() );

  return widgetDef;
}

void QgsAttributesFormProperties::mEditorLayoutComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )

  const QgsEditFormConfig::EditorLayout layout = mEditorLayoutComboBox->currentData().value<QgsEditFormConfig::EditorLayout>();
  switch ( layout )
  {
    case QgsEditFormConfig::EditorLayout::GeneratedLayout:
      mFormLayoutWidget->setVisible( false );
      mUiFileFrame->setVisible( false );
      mAddTabOrGroupButton->setVisible( false );
      mRemoveTabOrGroupButton->setVisible( false );
      mInvertSelectionButton->setVisible( false );
      break;

    case QgsEditFormConfig::EditorLayout::TabLayout:
      mFormLayoutWidget->setVisible( true );
      mUiFileFrame->setVisible( false );
      mAddTabOrGroupButton->setVisible( true );
      mRemoveTabOrGroupButton->setVisible( true );
      mInvertSelectionButton->setVisible( true );
      break;

    case QgsEditFormConfig::EditorLayout::UiFileLayout:
      // ui file
      mFormLayoutWidget->setVisible( false );
      mUiFileFrame->setVisible( true );
      mAddTabOrGroupButton->setVisible( false );
      mRemoveTabOrGroupButton->setVisible( false );
      mInvertSelectionButton->setVisible( false );
      break;
  }
}

void QgsAttributesFormProperties::mTbInitCode_clicked()
{
  QgsAttributesFormInitCode attributesFormInitCode;

  attributesFormInitCode.setCodeSource( mInitCodeSource );
  attributesFormInitCode.setInitCode( mInitCode );
  attributesFormInitCode.setInitFilePath( mInitFilePath );
  attributesFormInitCode.setInitFunction( mInitFunction );

  if ( !attributesFormInitCode.exec() )
    return;

  mInitCodeSource = attributesFormInitCode.codeSource();
  mInitCode = attributesFormInitCode.initCode();
  mInitFilePath = attributesFormInitCode.initFilePath();
  mInitFunction = attributesFormInitCode.initFunction();

}

void QgsAttributesFormProperties::pbnSelectEditForm_clicked()
{
  QgsSettings myQSettings;
  const QString lastUsedDir = myQSettings.value( QStringLiteral( "style/lastUIDir" ), QDir::homePath() ).toString();
  const QString uifilename = QFileDialog::getOpenFileName( this, tr( "Select edit form" ), lastUsedDir, tr( "UI file" )  + " (*.ui)" );

  if ( uifilename.isNull() )
    return;

  const QFileInfo fi( uifilename );
  myQSettings.setValue( QStringLiteral( "style/lastUIDir" ), fi.path() );
  mEditFormLineEdit->setText( uifilename );
}

void QgsAttributesFormProperties::apply()
{
  storeAttributeWidgetEdit();
  storeAttributeContainerEdit();
  storeAttributeTypeDialog();

  QgsEditFormConfig editFormConfig = mLayer->editFormConfig();

  QTreeWidgetItem *fieldContainer = mAvailableWidgetsTree->invisibleRootItem()->child( 0 );

  for ( int i = 0; i < fieldContainer->childCount(); i++ )
  {
    QTreeWidgetItem *fieldItem = fieldContainer->child( i );
    const FieldConfig cfg = fieldItem->data( 0, FieldConfigRole ).value<FieldConfig>();

    const QString fieldName { fieldItem->data( 0, FieldNameRole ).toString() };
    const int idx = mLayer->fields().indexOf( fieldName );

    //continue in case field does not exist anymore
    if ( idx < 0 )
      continue;

    editFormConfig.setReadOnly( idx, !cfg.mEditable );
    editFormConfig.setLabelOnTop( idx, cfg.mLabelOnTop );
    editFormConfig.setReuseLastValue( idx, cfg.mReuseLastValues );

    if ( cfg.mDataDefinedProperties.count() > 0 )
    {
      editFormConfig.setDataDefinedFieldProperties( fieldName, cfg.mDataDefinedProperties );
    }

    mLayer->setEditorWidgetSetup( idx, QgsEditorWidgetSetup( cfg.mEditorWidgetType, cfg.mEditorWidgetConfig ) );

    const QgsFieldConstraints constraints = cfg.mFieldConstraints;
    mLayer->setConstraintExpression( idx, constraints.constraintExpression(), constraints.constraintDescription() );
    if ( constraints.constraints() & QgsFieldConstraints::ConstraintNotNull )
    {
      mLayer->setFieldConstraint( idx, QgsFieldConstraints::ConstraintNotNull, constraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) );
    }
    else
    {
      mLayer->removeFieldConstraint( idx, QgsFieldConstraints::ConstraintNotNull );
    }
    if ( constraints.constraints() & QgsFieldConstraints::ConstraintUnique )
    {
      mLayer->setFieldConstraint( idx, QgsFieldConstraints::ConstraintUnique, constraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) );
    }
    else
    {
      mLayer->removeFieldConstraint( idx, QgsFieldConstraints::ConstraintUnique );
    }
    if ( constraints.constraints() & QgsFieldConstraints::ConstraintExpression )
    {
      mLayer->setFieldConstraint( idx, QgsFieldConstraints::ConstraintExpression, constraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) );
    }
    else
    {
      mLayer->removeFieldConstraint( idx, QgsFieldConstraints::ConstraintExpression );
    }

    mLayer->setFieldAlias( idx, cfg.mAlias );
  }

  // tabs and groups
  editFormConfig.clearTabs();
  for ( int t = 0; t < mFormLayoutTree->invisibleRootItem()->childCount(); t++ )
  {
    QTreeWidgetItem *tabItem = mFormLayoutTree->invisibleRootItem()->child( t );
    QgsAttributeEditorElement *editorElement { createAttributeEditorWidget( tabItem, nullptr, false ) };
    if ( editorElement )
      editFormConfig.addTab( editorElement );
  }

  editFormConfig.setUiForm( mEditFormLineEdit->text() );

  editFormConfig.setLayout( static_cast<QgsEditFormConfig::EditorLayout>( mEditorLayoutComboBox->currentIndex() ) );

  editFormConfig.setInitCodeSource( mInitCodeSource );
  editFormConfig.setInitFunction( mInitFunction );
  editFormConfig.setInitFilePath( mInitFilePath );
  editFormConfig.setInitCode( mInitCode );

  editFormConfig.setSuppress( static_cast<QgsEditFormConfig::FeatureFormSuppress>( mFormSuppressCmbBx->currentIndex() ) );

  // write the legacy config of relation widgets to support settings read by the API
  QTreeWidgetItem *relationContainer = mAvailableWidgetsTree->invisibleRootItem()->child( 1 );

  for ( int i = 0; i < relationContainer->childCount(); i++ )
  {
    QTreeWidgetItem *relationItem = relationContainer->child( i );
    const DnDTreeItemData itemData = relationItem->data( 0, DnDTreeRole ).value<DnDTreeItemData>();

    for ( int t = 0; t < mFormLayoutTree->invisibleRootItem()->childCount(); t++ )
    {
      QTreeWidgetItem *tabItem = mFormLayoutTree->invisibleRootItem()->child( t );
      const DnDTreeItemData tabItemData = tabItem->data( 0, DnDTreeRole ).value<DnDTreeItemData>();

      if ( tabItemData.type() == itemData.type() && tabItemData.name() == itemData.name() )
      {
        QVariantMap cfg;

        cfg[QStringLiteral( "nm-rel" )] = tabItemData.relationEditorConfiguration().nmRelationId;
        cfg[QStringLiteral( "force-suppress-popup" )] = tabItemData.relationEditorConfiguration().forceSuppressFormPopup;

        editFormConfig.setWidgetConfig( tabItemData.name(), cfg );
        break;
      }
    }
  }

  mLayer->setEditFormConfig( editFormConfig );
}


/*
 * FieldConfig implementation
 */
QgsAttributesFormProperties::FieldConfig::FieldConfig( QgsVectorLayer *layer, int idx )
{
  mAlias = layer->fields().at( idx ).alias();
  mDataDefinedProperties = layer->editFormConfig().dataDefinedFieldProperties( layer->fields().at( idx ).name() );
  mComment = layer->fields().at( idx ).comment();
  mEditable = !layer->editFormConfig().readOnly( idx );
  mEditableEnabled = layer->fields().fieldOrigin( idx ) != QgsFields::OriginJoin
                     && layer->fields().fieldOrigin( idx ) != QgsFields::OriginExpression;
  mLabelOnTop = layer->editFormConfig().labelOnTop( idx );
  mReuseLastValues = layer->editFormConfig().reuseLastValue( idx );
  mFieldConstraints = layer->fields().at( idx ).constraints();
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( layer, layer->fields().field( idx ).name() );
  mEditorWidgetType = setup.type();
  mEditorWidgetConfig = setup.config();
}

QgsAttributesFormProperties::FieldConfig::operator QVariant()
{
  return QVariant::fromValue<QgsAttributesFormProperties::FieldConfig>( *this );
}

/*
 * RelationEditorConfiguration implementation
 */

QgsAttributesFormProperties::RelationEditorConfiguration::operator QVariant()
{
  return QVariant::fromValue<QgsAttributesFormProperties::RelationEditorConfiguration>( *this );
}

/*
 * DnDTree implementation
 */

QTreeWidgetItem *QgsAttributesDnDTree::addContainer( QTreeWidgetItem *parent, const QString &title, int columnCount )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( QStringList() << title );
  newItem->setBackground( 0, QBrush( Qt::lightGray ) );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
  QgsAttributesFormProperties::DnDTreeItemData itemData( QgsAttributesFormProperties::DnDTreeItemData::Container, title, title );
  itemData.setColumnCount( columnCount );
  newItem->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData );
  parent->addChild( newItem );
  newItem->setExpanded( true );
  return newItem;
}

QgsAttributesDnDTree::QgsAttributesDnDTree( QgsVectorLayer *layer, QWidget *parent )
  : QTreeWidget( parent )
  , mLayer( layer )
{
  connect( this, &QTreeWidget::itemDoubleClicked, this, &QgsAttributesDnDTree::onItemDoubleClicked );
}

QTreeWidgetItem *QgsAttributesDnDTree::addItem( QTreeWidgetItem *parent, QgsAttributesFormProperties::DnDTreeItemData data, int index, const QIcon &icon )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( QStringList() << data.name() );

  switch ( data.type() )
  {
    case QgsAttributesFormProperties::DnDTreeItemData::Action:
    case QgsAttributesFormProperties::DnDTreeItemData::Field:
    case QgsAttributesFormProperties::DnDTreeItemData::Relation:
    case QgsAttributesFormProperties::DnDTreeItemData::QmlWidget:
    case QgsAttributesFormProperties::DnDTreeItemData::HtmlWidget:
      newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
      break;

    case QgsAttributesFormProperties::DnDTreeItemData::WidgetType:
    case QgsAttributesFormProperties::DnDTreeItemData::Container:
    {
      newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
      newItem->setBackground( 0, QBrush( Qt::lightGray ) );
    }
    break;
  }

  newItem->setData( 0, QgsAttributesFormProperties::DnDTreeRole, data );
  newItem->setText( 0, data.displayName() );
  newItem->setIcon( 0, icon );

  if ( index < 0 )
    parent->addChild( newItem );
  else
    parent->insertChild( index, newItem );

  return newItem;
}

/**
 * Is called when mouse is moved over attributes tree before a
 * drop event. Used to inhibit dropping fields onto the root item.
 */

void QgsAttributesDnDTree::dragMoveEvent( QDragMoveEvent *event )
{
  const QMimeData *data = event->mimeData();

  if ( data->hasFormat( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) ) )
  {
    QgsAttributesFormProperties::DnDTreeItemData itemElement;

    QByteArray itemData = data->data( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) );
    QDataStream stream( &itemData, QIODevice::ReadOnly );
    stream >> itemElement;

    // Inner drag and drop actions are always MoveAction
    if ( event->source() == this )
    {
      event->setDropAction( Qt::MoveAction );
    }
  }
  else
  {
    event->ignore();
  }

  QTreeWidget::dragMoveEvent( event );
}


bool QgsAttributesDnDTree::dropMimeData( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action )
{
  bool bDropSuccessful = false;

  if ( action == Qt::IgnoreAction )
  {
    bDropSuccessful = true;
  }
  else if ( data->hasFormat( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) ) )
  {
    QByteArray itemData = data->data( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) );
    QDataStream stream( &itemData, QIODevice::ReadOnly );
    QgsAttributesFormProperties::DnDTreeItemData itemElement;

    while ( !stream.atEnd() )
    {
      stream >> itemElement;

      QTreeWidgetItem *newItem;

      if ( parent )
      {
        newItem = addItem( parent, itemElement, index++ );
        bDropSuccessful = true;
      }
      else
      {
        newItem = addItem( invisibleRootItem(), itemElement, index++ );
        bDropSuccessful = true;
      }

      if ( itemElement.type() == QgsAttributesFormProperties::DnDTreeItemData::QmlWidget )
      {
        onItemDoubleClicked( newItem, 0 );
      }

      if ( itemElement.type() == QgsAttributesFormProperties::DnDTreeItemData::HtmlWidget )
      {
        onItemDoubleClicked( newItem, 0 );
      }
      clearSelection();
      newItem->setSelected( true );
    }
  }

  return bDropSuccessful;
}

void QgsAttributesDnDTree::dropEvent( QDropEvent *event )
{
  if ( !event->mimeData()->hasFormat( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) ) )
    return;

  if ( event->source() == this )
  {
    event->setDropAction( Qt::MoveAction );
  }

  QTreeWidget::dropEvent( event );
}

QStringList QgsAttributesDnDTree::mimeTypes() const
{
  return QStringList() << QStringLiteral( "application/x-qgsattributetabledesignerelement" );
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QMimeData *QgsAttributesDnDTree::mimeData( const QList<QTreeWidgetItem *> items ) const
#else
QMimeData *QgsAttributesDnDTree::mimeData( const QList<QTreeWidgetItem *> &items ) const
#endif
{
  if ( items.count() <= 0 )
    return nullptr;

  const QStringList types = mimeTypes();

  if ( types.isEmpty() )
    return nullptr;

  QMimeData *data = new QMimeData();
  const QString format = types.at( 0 );
  QByteArray encoded;
  QDataStream stream( &encoded, QIODevice::WriteOnly );

  const auto constItems = items;
  for ( const QTreeWidgetItem *item : constItems )
  {
    if ( item )
    {
      // Relevant information is always in the DnDTreeRole of the first column
      const QgsAttributesFormProperties::DnDTreeItemData itemData = item->data( 0, QgsAttributesFormProperties::DnDTreeRole ).value<QgsAttributesFormProperties::DnDTreeItemData>();
      stream << itemData;
    }
  }

  data->setData( format, encoded );

  return data;
}

void QgsAttributesDnDTree::onItemDoubleClicked( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( column )

  QgsAttributesFormProperties::DnDTreeItemData itemData = item->data( 0, QgsAttributesFormProperties::DnDTreeRole ).value<QgsAttributesFormProperties::DnDTreeItemData>();

  QGroupBox *baseData = new QGroupBox( tr( "Base configuration" ) );

  QFormLayout *baseLayout = new QFormLayout();
  baseData->setLayout( baseLayout );
  QCheckBox *showLabelCheckbox = new QCheckBox( QStringLiteral( "Show label" ) );
  showLabelCheckbox->setChecked( itemData.showLabel() );
  baseLayout->addRow( showLabelCheckbox );
  QWidget *baseWidget = new QWidget();
  baseWidget->setLayout( baseLayout );

  switch ( itemData.type() )
  {
    case QgsAttributesFormProperties::DnDTreeItemData::Action:
    case QgsAttributesFormProperties::DnDTreeItemData::Container:
    case QgsAttributesFormProperties::DnDTreeItemData::WidgetType:
    case QgsAttributesFormProperties::DnDTreeItemData::Relation:
    case QgsAttributesFormProperties::DnDTreeItemData::Field:
      break;

    case QgsAttributesFormProperties::DnDTreeItemData::QmlWidget:
    {
      if ( mType == QgsAttributesDnDTree::Type::Drag )
        return;

      QDialog dlg;
      dlg.setWindowTitle( tr( "Configure QML Widget" ) );

      QVBoxLayout *mainLayout = new QVBoxLayout();
      QHBoxLayout *qmlLayout = new QHBoxLayout();
      QVBoxLayout *layout = new QVBoxLayout();
      mainLayout->addLayout( qmlLayout );
      qmlLayout->addLayout( layout );
      dlg.setLayout( mainLayout );
      layout->addWidget( baseWidget );

      QLineEdit *title = new QLineEdit( itemData.name() );

      //qmlCode
      QPlainTextEdit *qmlCode = new QPlainTextEdit( itemData.qmlElementEditorConfiguration().qmlCode );
      qmlCode->setPlaceholderText( tr( "Insert QML code here…" ) );

      QgsQmlWidgetWrapper *qmlWrapper = new QgsQmlWidgetWrapper( mLayer, nullptr, this );
      QgsFeature previewFeature;
      mLayer->getFeatures().nextFeature( previewFeature );

      //update preview on text change
      connect( qmlCode, &QPlainTextEdit::textChanged, this, [ = ]
      {
        qmlWrapper->setQmlCode( qmlCode->toPlainText() );
        qmlWrapper->reinitWidget();
        qmlWrapper->setFeature( previewFeature );
      } );

      //templates
      QComboBox *qmlObjectTemplate = new QComboBox();
      qmlObjectTemplate->addItem( tr( "Free Text…" ) );
      qmlObjectTemplate->addItem( tr( "Rectangle" ) );
      qmlObjectTemplate->addItem( tr( "Pie Chart" ) );
      qmlObjectTemplate->addItem( tr( "Bar Chart" ) );
      connect( qmlObjectTemplate, qOverload<int>( &QComboBox::activated ), qmlCode, [ = ]( int index )
      {
        qmlCode->clear();
        switch ( index )
        {
          case 0:
          {
            qmlCode->setPlaceholderText( tr( "Insert QML code here…" ) );
            break;
          }
          case 1:
          {
            qmlCode->insertPlainText( QStringLiteral( "import QtQuick 2.0\n"
                                      "\n"
                                      "Rectangle {\n"
                                      "    width: 100\n"
                                      "    height: 100\n"
                                      "    color: \"steelblue\"\n"
                                      "    Text{ text: \"A rectangle\" }\n"
                                      "}\n" ) );
            break;
          }
          case 2:
          {
            qmlCode->insertPlainText( QStringLiteral( "import QtQuick 2.0\n"
                                      "import QtCharts 2.0\n"
                                      "\n"
                                      "ChartView {\n"
                                      "    width: 400\n"
                                      "    height: 400\n"
                                      "\n"
                                      "    PieSeries {\n"
                                      "        id: pieSeries\n"
                                      "        PieSlice { label: \"First slice\"; value: 25 }\n"
                                      "        PieSlice { label: \"Second slice\"; value: 45 }\n"
                                      "        PieSlice { label: \"Third slice\"; value: 30 }\n"
                                      "    }\n"
                                      "}\n" ) );
            break;
          }
          case 3:
          {
            qmlCode->insertPlainText( QStringLiteral( "import QtQuick 2.0\n"
                                      "import QtCharts 2.0\n"
                                      "\n"
                                      "ChartView {\n"
                                      "    title: \"Bar series\"\n"
                                      "    width: 600\n"
                                      "    height:400\n"
                                      "    legend.alignment: Qt.AlignBottom\n"
                                      "    antialiasing: true\n"
                                      "    ValueAxis{\n"
                                      "        id: valueAxisY\n"
                                      "        min: 0\n"
                                      "        max: 15\n"
                                      "    }\n"
                                      "\n"
                                      "    BarSeries {\n"
                                      "        id: mySeries\n"
                                      "        axisY: valueAxisY\n"
                                      "        axisX: BarCategoryAxis { categories: [\"2007\", \"2008\", \"2009\", \"2010\", \"2011\", \"2012\" ] }\n"
                                      "        BarSet { label: \"Bob\"; values: [2, 2, 3, 4, 5, 6] }\n"
                                      "        BarSet { label: \"Susan\"; values: [5, 1, 2, 4, 1, 7] }\n"
                                      "        BarSet { label: \"James\"; values: [3, 5, 8, 13, 5, 8] }\n"
                                      "    }\n"
                                      "}\n" ) );
            break;
          }
          default:
            break;
        }
      } );

      QgsFieldExpressionWidget *expressionWidget = new QgsFieldExpressionWidget;
      expressionWidget->setLayer( mLayer );
      QToolButton *addExpressionButton = new QToolButton();
      addExpressionButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyAdd.svg" ) ) );

      connect( addExpressionButton, &QAbstractButton::clicked, this, [ = ]
      {
        qmlCode->insertPlainText( QStringLiteral( "expression.evaluate(\"%1\")" ).arg( expressionWidget->expression().replace( '"', QLatin1String( "\\\"" ) ) ) );
      } );

      layout->addWidget( new QLabel( tr( "Title" ) ) );
      layout->addWidget( title );
      QGroupBox *qmlCodeBox = new QGroupBox( tr( "QML Code" ) );
      qmlCodeBox->setLayout( new QGridLayout );
      qmlCodeBox->layout()->addWidget( qmlObjectTemplate );
      QGroupBox *expressionWidgetBox = new QGroupBox();
      qmlCodeBox->layout()->addWidget( expressionWidgetBox );
      expressionWidgetBox->setLayout( new QHBoxLayout );
      expressionWidgetBox->layout()->addWidget( expressionWidget );
      expressionWidgetBox->layout()->addWidget( addExpressionButton );
      qmlCodeBox->layout()->addWidget( qmlCode );
      layout->addWidget( qmlCodeBox );
      QScrollArea *qmlPreviewBox = new QgsScrollArea();
      qmlPreviewBox->setLayout( new QGridLayout );
      qmlPreviewBox->setMinimumWidth( 400 );
      qmlPreviewBox->layout()->addWidget( qmlWrapper->widget() );
      //emit to load preview for the first time
      emit qmlCode->textChanged();
      qmlLayout->addWidget( qmlPreviewBox );

      QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

      connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
      connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );

      mainLayout->addWidget( buttonBox );

      if ( dlg.exec() )
      {
        QgsAttributesFormProperties::QmlElementEditorConfiguration qmlEdCfg;
        qmlEdCfg.qmlCode = qmlCode->toPlainText();
        itemData.setName( title->text() );
        itemData.setQmlElementEditorConfiguration( qmlEdCfg );
        itemData.setShowLabel( showLabelCheckbox->isChecked() );

        item->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData );
        item->setText( 0, title->text() );
      }
    }
    break;

    case QgsAttributesFormProperties::DnDTreeItemData::HtmlWidget:
    {
      if ( mType == QgsAttributesDnDTree::Type::Drag )
        return;
      QDialog dlg;
      dlg.setWindowTitle( tr( "Configure HTML Widget" ) );

      QVBoxLayout *mainLayout = new QVBoxLayout();
      QHBoxLayout *htmlLayout = new QHBoxLayout();
      QVBoxLayout *layout = new QVBoxLayout();
      mainLayout->addLayout( htmlLayout );
      htmlLayout->addLayout( layout );
      dlg.setLayout( mainLayout );
      layout->addWidget( baseWidget );

      QLineEdit *title = new QLineEdit( itemData.name() );

      //htmlCode
      QgsCodeEditorHTML *htmlCode = new QgsCodeEditorHTML( );
      htmlCode->setSizePolicy( QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding );
      htmlCode->setText( itemData.htmlElementEditorConfiguration().htmlCode );

      QgsHtmlWidgetWrapper *htmlWrapper = new QgsHtmlWidgetWrapper( mLayer, nullptr, this );
      QgsFeature previewFeature;
      mLayer->getFeatures().nextFeature( previewFeature );

      //update preview on text change
      connect( htmlCode, &QgsCodeEditorHTML::textChanged, this, [ = ]
      {
        htmlWrapper->setHtmlCode( htmlCode->text( ) );
        htmlWrapper->reinitWidget();
        htmlWrapper->setFeature( previewFeature );
      } );

      QgsFieldExpressionWidget *expressionWidget = new QgsFieldExpressionWidget;
      expressionWidget->setLayer( mLayer );
      QToolButton *addExpressionButton = new QToolButton();
      addExpressionButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyAdd.svg" ) ) );

      connect( addExpressionButton, &QAbstractButton::clicked, this, [ = ]
      {
        htmlCode->insertText( QStringLiteral( "<script>document.write(expression.evaluate(\"%1\"));</script>" ).arg( expressionWidget->expression().replace( '"', QLatin1String( "\\\"" ) ) ) );
      } );

      layout->addWidget( new QLabel( tr( "Title" ) ) );
      layout->addWidget( title );
      QGroupBox *expressionWidgetBox = new QGroupBox( tr( "HTML Code" ) );
      layout->addWidget( expressionWidgetBox );
      expressionWidgetBox->setLayout( new QHBoxLayout );
      expressionWidgetBox->layout()->addWidget( expressionWidget );
      expressionWidgetBox->layout()->addWidget( addExpressionButton );
      layout->addWidget( htmlCode );
      QScrollArea *htmlPreviewBox = new QgsScrollArea();
      htmlPreviewBox->setLayout( new QGridLayout );
      htmlPreviewBox->setMinimumWidth( 400 );
      htmlPreviewBox->layout()->addWidget( htmlWrapper->widget() );
      //emit to load preview for the first time
      emit htmlCode->textChanged();
      htmlLayout->addWidget( htmlPreviewBox );

      QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

      connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
      connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );

      mainLayout->addWidget( buttonBox );

      if ( dlg.exec() )
      {
        QgsAttributesFormProperties::HtmlElementEditorConfiguration htmlEdCfg;
        htmlEdCfg.htmlCode = htmlCode->text();
        itemData.setName( title->text() );
        itemData.setHtmlElementEditorConfiguration( htmlEdCfg );
        itemData.setShowLabel( showLabelCheckbox->isChecked() );

        item->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData );
        item->setText( 0, title->text() );
      }
    }
    break;
  }
}

QgsAttributesDnDTree::Type QgsAttributesDnDTree::type() const
{
  return mType;
}

void QgsAttributesDnDTree::setType( QgsAttributesDnDTree::Type value )
{
  mType = value;
}

void QgsAttributesDnDTree::selectFirstMatchingItem( const QgsAttributesFormProperties::DnDTreeItemData &data )
{
  QTreeWidgetItemIterator it( this );
  while ( *it )
  {
    const QgsAttributesFormProperties::DnDTreeItemData rowData = ( *it )->data( 0, QgsAttributesFormProperties::DnDTreeRole ).value<QgsAttributesFormProperties::DnDTreeItemData>();
    if ( data.type() == rowData.type() && data.name() == rowData.name() )
    {
      if ( selectedItems().count() == 1 && ( *it )->isSelected() == true )
      {
        // the selection is already good
      }
      else
      {
        clearSelection();
        ( *it )->setSelected( true );
      }
      return;
    }
    ++it;
  }
  clearSelection();
}


/*
 * Serialization helpers for DesigerTreeItemData so we can stuff this easily into QMimeData
 */

QDataStream &operator<<( QDataStream &stream, const QgsAttributesFormProperties::DnDTreeItemData &data )
{
  stream << static_cast<quint32>( data.type() ) << data.name() << data.displayName();
  return stream;
}

QDataStream &operator>>( QDataStream &stream, QgsAttributesFormProperties::DnDTreeItemData &data )
{
  QString name;
  QString displayName;
  quint32 type;

  stream >> type >> name >> displayName;

  data.setType( static_cast<QgsAttributesFormProperties::DnDTreeItemData::Type>( type ) );
  data.setName( name );
  data.setDisplayName( displayName );

  return stream;
}

bool QgsAttributesFormProperties::DnDTreeItemData::showAsGroupBox() const
{
  return mShowAsGroupBox;
}

void QgsAttributesFormProperties::DnDTreeItemData::setShowAsGroupBox( bool showAsGroupBox )
{
  mShowAsGroupBox = showAsGroupBox;
}

bool QgsAttributesFormProperties::DnDTreeItemData::showLabel() const
{
  return mShowLabel;
}

void QgsAttributesFormProperties::DnDTreeItemData::setShowLabel( bool showLabel )
{
  mShowLabel = showLabel;
}

QgsOptionalExpression QgsAttributesFormProperties::DnDTreeItemData::visibilityExpression() const
{
  return mVisibilityExpression;
}

void QgsAttributesFormProperties::DnDTreeItemData::setVisibilityExpression( const QgsOptionalExpression &visibilityExpression )
{
  mVisibilityExpression = visibilityExpression;
}

QgsOptionalExpression QgsAttributesFormProperties::DnDTreeItemData::collapsedExpression() const
{
  return mCollapsedExpression;
}

void QgsAttributesFormProperties::DnDTreeItemData::setCollapsedExpression( const QgsOptionalExpression &collapsedExpression )
{
  mCollapsedExpression = collapsedExpression;
}

QgsAttributesFormProperties::RelationEditorConfiguration QgsAttributesFormProperties::DnDTreeItemData::relationEditorConfiguration() const
{
  return mRelationEditorConfiguration;
}

void QgsAttributesFormProperties::DnDTreeItemData::setRelationEditorConfiguration( QgsAttributesFormProperties::RelationEditorConfiguration relationEditorConfiguration )
{
  mRelationEditorConfiguration = relationEditorConfiguration;
}

QgsAttributesFormProperties::QmlElementEditorConfiguration QgsAttributesFormProperties::DnDTreeItemData::qmlElementEditorConfiguration() const
{
  return mQmlElementEditorConfiguration;
}

void QgsAttributesFormProperties::DnDTreeItemData::setQmlElementEditorConfiguration( QgsAttributesFormProperties::QmlElementEditorConfiguration qmlElementEditorConfiguration )
{
  mQmlElementEditorConfiguration = qmlElementEditorConfiguration;
}


QgsAttributesFormProperties::HtmlElementEditorConfiguration QgsAttributesFormProperties::DnDTreeItemData::htmlElementEditorConfiguration() const
{
  return mHtmlElementEditorConfiguration;
}

void QgsAttributesFormProperties::DnDTreeItemData::setHtmlElementEditorConfiguration( QgsAttributesFormProperties::HtmlElementEditorConfiguration htmlElementEditorConfiguration )
{
  mHtmlElementEditorConfiguration = htmlElementEditorConfiguration;
}

QColor QgsAttributesFormProperties::DnDTreeItemData::backgroundColor() const
{
  return mBackgroundColor;
}

void QgsAttributesFormProperties::DnDTreeItemData::setBackgroundColor( const QColor &backgroundColor )
{
  mBackgroundColor = backgroundColor;
}
