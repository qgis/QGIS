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

#include "qgsattributesformproperties.h"
#include "qgsattributetypedialog.h"
#include "qgsattributerelationedit.h"
#include "qgsattributesforminitcode.h"
#include "qgisapp.h"
#include "qgsfieldcombobox.h"
#include "qgsqmlwidgetwrapper.h"

QgsAttributesFormProperties::QgsAttributesFormProperties( QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  if ( !layer )
    return;

  setupUi( this );

  // available widgets tree
  QGridLayout *availableWidgetsWidgetLayout = new QGridLayout;
  mAvailableWidgetsTree = new DnDTree( mLayer );
  availableWidgetsWidgetLayout->addWidget( mAvailableWidgetsTree );
  availableWidgetsWidgetLayout->setMargin( 0 );
  mAvailableWidgetsWidget->setLayout( availableWidgetsWidgetLayout );
  mAvailableWidgetsTree->setHeaderLabels( QStringList() << tr( "Available Widgets" ) );
  mAvailableWidgetsTree->setType( DnDTree::Type::Drag );

  // form layout tree
  QGridLayout *formLayoutWidgetLayout = new QGridLayout;
  mFormLayoutTree = new DnDTree( mLayer );
  mFormLayoutWidget->setLayout( formLayoutWidgetLayout );
  formLayoutWidgetLayout->addWidget( mFormLayoutTree );
  formLayoutWidgetLayout->setMargin( 0 );
  mFormLayoutTree->setHeaderLabels( QStringList() << tr( "Form Layout" ) );
  mFormLayoutTree->setType( DnDTree::Type::Drop );

  // AttributeTypeDialog
  mAttributeTypeDialog = new QgsAttributeTypeDialog( mLayer, -1, mAttributeTypeFrame );
  mAttributeTypeDialog->layout()->setMargin( 0 );
  mAttributeTypeFrame->layout()->setMargin( 0 );
  mAttributeTypeFrame->layout()->addWidget( mAttributeTypeDialog );

  // AttributeRelationEdit
  mAttributeRelationEdit = new QgsAttributeRelationEdit( "", mAttributeTypeFrame );
  mAttributeRelationEdit->layout()->setMargin( 0 );
  mAttributeTypeFrame->layout()->setMargin( 0 );
  mAttributeTypeFrame->layout()->addWidget( mAttributeRelationEdit );

  connect( mAvailableWidgetsTree, &QTreeWidget::itemSelectionChanged, this, &QgsAttributesFormProperties::onAttributeSelectionChanged );
  connect( mAddTabOrGroupButton, &QAbstractButton::clicked, this, &QgsAttributesFormProperties::addTabOrGroupButton );
  connect( mRemoveTabOrGroupButton, &QAbstractButton::clicked, this, &QgsAttributesFormProperties::removeTabOrGroupButton );
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

  mAttributeTypeDialog->setEnabled( false );
  mAttributeRelationEdit->setEnabled( false );
}

void QgsAttributesFormProperties::initAvailableWidgetsTree()
{
  mAvailableWidgetsTree->clear();
  mAvailableWidgetsTree->setSortingEnabled( false );
  mAvailableWidgetsTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  mAvailableWidgetsTree->setAcceptDrops( false );
  mAvailableWidgetsTree->setDragDropMode( QAbstractItemView::DragOnly );

  //load Fields

  DnDTreeItemData catItemData = DnDTreeItemData( DnDTreeItemData::Container, QStringLiteral( "Fields" ), QStringLiteral( "Fields" ) );
  QTreeWidgetItem *catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  const QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    const QgsField field = fields.at( i );
    DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Field, field.name(), field.name() );
    itemData.setShowLabel( true );

    FieldConfig cfg( mLayer, i );

    QTreeWidgetItem *item = mAvailableWidgetsTree->addItem( catitem, itemData );

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
  catItemData = DnDTreeItemData( DnDTreeItemData::Container, QStringLiteral( "Relations" ), tr( "Relations" ) );
  catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencedRelations( mLayer );

  for ( const QgsRelation &relation : relations )
  {
    DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Relation, QStringLiteral( "%1" ).arg( relation.id() ), QStringLiteral( "%1" ).arg( relation.name() ) );
    itemData.setShowLabel( true );

    RelationConfig cfg( mLayer, relation.id() );

    QTreeWidgetItem *item = mAvailableWidgetsTree->addItem( catitem, itemData );
    item->setData( 0, RelationConfigRole, cfg );
    item->setData( 0, FieldNameRole, relation.id() );
  }
  catitem->setExpanded( true );

  // QML widget
  catItemData = DnDTreeItemData( DnDTreeItemData::Container, QStringLiteral( "Other" ), tr( "Other Widgets" ) );
  catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::QmlWidget, QStringLiteral( "QmlWidget" ), tr( "QML Widget" ) );
  itemData.setShowLabel( true );

  mAvailableWidgetsTree->addItem( catitem, itemData );
  catitem ->setExpanded( true );
}

void QgsAttributesFormProperties::initFormLayoutTree()
{
  // tabs and groups info
  mFormLayoutTree->clear();
  mFormLayoutTree->setSortingEnabled( false );
  mFormLayoutTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  mFormLayoutTree->setAcceptDrops( true );
  mFormLayoutTree->setDragDropMode( QAbstractItemView::DragDrop );

  Q_FOREACH ( QgsAttributeEditorElement *wdg, mLayer->editFormConfig().tabs() )
  {
    loadAttributeEditorTreeItem( wdg, mFormLayoutTree->invisibleRootItem(), mFormLayoutTree );
  }
}


void QgsAttributesFormProperties::initSuppressCombo()
{
  QgsSettings settings;

  if ( settings.value( QStringLiteral( "qgis/digitizing/disable_enter_attribute_values_dialog" ), false ).toBool() )
  {
    mFormSuppressCmbBx->addItem( tr( "Hide form on add feature (global settings)" ) );
  }
  else
  {
    mFormSuppressCmbBx->addItem( tr( "Show form on add feature (global settings)" ) );
  }
  mFormSuppressCmbBx->addItem( tr( "Hide form on add feature" ) );
  mFormSuppressCmbBx->addItem( tr( "Show form on add feature" ) );

  mFormSuppressCmbBx->setCurrentIndex( mLayer->editFormConfig().suppress() );


}
void QgsAttributesFormProperties::initLayoutConfig()
{
  mEditorLayoutComboBox->setCurrentIndex( mLayer->editFormConfig().layout() );
  mEditorLayoutComboBox_currentIndexChanged( mEditorLayoutComboBox->currentIndex() );

  QgsEditFormConfig cfg = mLayer->editFormConfig();
  mEditFormLineEdit->setText( cfg.uiForm() );
}

void QgsAttributesFormProperties::initInitPython()
{
  QgsEditFormConfig cfg = mLayer->editFormConfig();

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
                          "\tgeom = feature.geometry()\n"
                          "\tcontrol = dialog.findChild(QWidget, \"MyLineEdit\")\n" ) );
  }
}

void QgsAttributesFormProperties::loadAttributeTypeDialog()
{
  //check if item or field with the items name for some reason not available anymore
  if ( !mAvailableWidgetsTree->currentItem() ||
       mLayer->fields().indexOf( mAvailableWidgetsTree->currentItem()->data( 0, FieldNameRole ).toString() ) < 0 )
    mAttributeTypeDialog->setEnabled( false );
  else
  {
    FieldConfig cfg = mAvailableWidgetsTree->currentItem()->data( 0, FieldConfigRole ).value<FieldConfig>();
    QString fieldName = mAvailableWidgetsTree->currentItem()->data( 0, FieldNameRole ).toString();
    int index = mLayer->fields().indexOf( fieldName );

    mAttributeTypeDialog->setEnabled( true );

    // AttributeTypeDialog delete and recreate
    mAttributeTypeFrame->layout()->removeWidget( mAttributeTypeDialog );
    delete mAttributeTypeDialog;
    mAttributeTypeDialog = new QgsAttributeTypeDialog( mLayer, index, mAttributeTypeFrame );

    mAttributeTypeDialog->setAlias( cfg.mAlias );
    mAttributeTypeDialog->setComment( cfg.mComment );
    mAttributeTypeDialog->setFieldEditable( cfg.mEditable );
    mAttributeTypeDialog->setLabelOnTop( cfg.mLabelOnTop );
    mAttributeTypeDialog->setNotNull( cfg.mConstraints & QgsFieldConstraints::ConstraintNotNull );
    mAttributeTypeDialog->setNotNullEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );
    mAttributeTypeDialog->setUnique( cfg.mConstraints & QgsFieldConstraints::ConstraintUnique );
    mAttributeTypeDialog->setUniqueEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );

    QgsFieldConstraints constraints = cfg.mFieldConstraints;
    QgsFieldConstraints::Constraints providerConstraints = nullptr;
    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintOriginProvider )
      providerConstraints |= QgsFieldConstraints::ConstraintNotNull;
    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) == QgsFieldConstraints::ConstraintOriginProvider )
      providerConstraints |= QgsFieldConstraints::ConstraintUnique;
    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintOriginProvider )
      providerConstraints |= QgsFieldConstraints::ConstraintExpression;
    mAttributeTypeDialog->setProviderConstraints( providerConstraints );

    mAttributeTypeDialog->setConstraintExpression( cfg.mConstraint );
    mAttributeTypeDialog->setConstraintExpressionDescription( cfg.mConstraintDescription );
    mAttributeTypeDialog->setConstraintExpressionEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );
    mAttributeTypeDialog->setDefaultValueExpression( mLayer->defaultValueDefinition( index ).expression() );
    mAttributeTypeDialog->setApplyDefaultValueOnUpdate( mLayer->defaultValueDefinition( index ).applyOnUpdate() );

    mAttributeTypeDialog->setEditorWidgetConfig( cfg.mEditorWidgetConfig );
    mAttributeTypeDialog->setEditorWidgetType( cfg.mEditorWidgetType );

    mAttributeTypeDialog->layout()->setMargin( 0 );
    mAttributeTypeFrame->layout()->setMargin( 0 );

    mAttributeTypeFrame->layout()->addWidget( mAttributeTypeDialog );
  }
}


void QgsAttributesFormProperties::storeAttributeTypeDialog()
{
  if ( mAttributeTypeDialog->fieldIdx() < 0 )
    return;

  FieldConfig cfg;

  cfg.mEditable = mAttributeTypeDialog->fieldEditable();
  cfg.mLabelOnTop = mAttributeTypeDialog->labelOnTop();
  cfg.mAlias = mAttributeTypeDialog->alias();

  //confustion (will be removed): wir laden teilweise sachen einfach beim store anstelle des applys auf die mLayer - eingie Sachen laden wir auch vom layer anstatt über das cfg. wieso
  QgsFieldConstraints constraints = mLayer->fields().at( mAttributeTypeDialog->fieldIdx() ).constraints();
  QgsFieldConstraints::Constraints providerConstraints = nullptr;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintNotNull;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintUnique;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintExpression;
  cfg.mConstraints = nullptr;
  if ( mAttributeTypeDialog->notNull() && !( providerConstraints & QgsFieldConstraints::ConstraintNotNull ) )
  {
    cfg.mConstraints |= QgsFieldConstraints::ConstraintNotNull;
  }
  if ( mAttributeTypeDialog->unique() && !( providerConstraints & QgsFieldConstraints::ConstraintUnique ) )
  {
    cfg.mConstraints |= QgsFieldConstraints::ConstraintUnique;
  }
  if ( !mAttributeTypeDialog->constraintExpression().isEmpty() && !( providerConstraints & QgsFieldConstraints::ConstraintExpression ) )
  {
    cfg.mConstraints |= QgsFieldConstraints::ConstraintExpression;
  }

  cfg.mConstraintDescription = mAttributeTypeDialog->constraintExpressionDescription();
  cfg.mConstraint = mAttributeTypeDialog->constraintExpression();
  mLayer->setDefaultValueDefinition( mAttributeTypeDialog->fieldIdx(), QgsDefaultValue( mAttributeTypeDialog->defaultValueExpression(), mAttributeTypeDialog->applyDefaultValueOnUpdate() ) );

  cfg.mEditorWidgetType = mAttributeTypeDialog->editorWidgetType();
  cfg.mEditorWidgetConfig = mAttributeTypeDialog->editorWidgetConfig();

  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintNotNull, mAttributeTypeDialog->notNullEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintUnique, mAttributeTypeDialog->uniqueEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintExpression, mAttributeTypeDialog->constraintExpressionEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );

  QString fieldName = mLayer->fields().at( mAttributeTypeDialog->fieldIdx() ).name();

  QTreeWidgetItemIterator itemIt( mAvailableWidgetsTree );
  while ( *itemIt )
  {
    QTreeWidgetItem *item = *itemIt;
    if ( item->data( 0, FieldNameRole ).toString() == fieldName )
      item->setData( 0, FieldConfigRole, QVariant::fromValue<FieldConfig>( cfg ) );
    ++itemIt;
  }

}


void QgsAttributesFormProperties::loadAttributeRelationEdit()
{
  QTreeWidgetItem *currentItem = mAvailableWidgetsTree->currentItem();

  if ( !currentItem )
    mAttributeRelationEdit->setEnabled( false );
  else
  {
    mAttributeRelationEdit->setEnabled( true );
    mAttributeTypeFrame->layout()->removeWidget( mAttributeRelationEdit );
    delete mAttributeRelationEdit;

    RelationConfig cfg = currentItem->data( 0, RelationConfigRole ).value<RelationConfig>();

    mAttributeRelationEdit = new QgsAttributeRelationEdit( currentItem->data( 0, FieldNameRole ).toString(), mAttributeTypeFrame );

    mAttributeRelationEdit->setCardinalityCombo( tr( "Many to one relation" ) );

    QgsRelation relation = QgsProject::instance()->relationManager()->relation( currentItem->data( 0, FieldNameRole ).toString() );

    const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencingRelations( relation.referencingLayer() );
    for ( const QgsRelation &nmrel : relations )
    {
      if ( nmrel.fieldPairs().at( 0 ).referencingField() != relation.fieldPairs().at( 0 ).referencingField() )
        mAttributeRelationEdit->setCardinalityCombo( QStringLiteral( "%1 (%2)" ).arg( nmrel.referencedLayer()->name(), nmrel.fieldPairs().at( 0 ).referencedField() ), nmrel.id() );
    }

    mAttributeRelationEdit->setCardinality( cfg.mCardinality );

    mAttributeRelationEdit->layout()->setMargin( 0 );
    mAttributeTypeFrame->layout()->setMargin( 0 );

    mAttributeTypeFrame->layout()->removeWidget( mAttributeTypeDialog );
    mAttributeTypeFrame->layout()->addWidget( mAttributeTypeDialog );
    mAttributeTypeFrame->layout()->addWidget( mAttributeRelationEdit );
  }
}


void QgsAttributesFormProperties::storeAttributeRelationEdit()
{
  RelationConfig cfg;

  cfg.mCardinality = mAttributeRelationEdit->cardinality();

  QTreeWidgetItem *relationContainer = mAvailableWidgetsTree->invisibleRootItem()->child( 1 );

  for ( int i = 0; i < relationContainer->childCount(); i++ )
  {
    QTreeWidgetItem *relationItem = relationContainer->child( i );
    DnDTreeItemData itemData = relationItem->data( 0, DnDTreeRole ).value<DnDTreeItemData>();

    if ( itemData.name() == mAttributeRelationEdit->mRelationId )
    {
      relationItem->setData( 0, RelationConfigRole, QVariant::fromValue<RelationConfig>( cfg ) );
    }
  }
}

QgsAttributesFormProperties::RelationConfig QgsAttributesFormProperties::configForRelation( const QString &relationId )
{
  QTreeWidgetItemIterator itemIt( mAvailableWidgetsTree );
  while ( *itemIt )
  {
    QTreeWidgetItem *item = *itemIt;

    if ( item->data( 0, FieldNameRole ).toString() == relationId )
      return item->data( 0, RelationConfigRole ).value<RelationConfig>();
    ++itemIt;
  }

  // Should never get here
  Q_ASSERT( false );
  return RelationConfig();
}


QTreeWidgetItem *QgsAttributesFormProperties::loadAttributeEditorTreeItem( QgsAttributeEditorElement *const widgetDef, QTreeWidgetItem *parent, DnDTree *tree )
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

    case QgsAttributeEditorElement::AeTypeRelation:
    {
      const QgsAttributeEditorRelation *relationEditor = static_cast<const QgsAttributeEditorRelation *>( widgetDef );
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Relation, relationEditor->relation().id(), relationEditor->relation().name() );
      itemData.setShowLabel( widgetDef->showLabel() );
      RelationEditorConfiguration relEdConfig;
      relEdConfig.showLinkButton = relationEditor->showLinkButton();
      relEdConfig.showUnlinkButton = relationEditor->showUnlinkButton();
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
      itemData.setVisibilityExpression( container->visibilityExpression() );
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
  storeAttributeTypeDialog();
  storeAttributeRelationEdit();

  switch ( mAvailableWidgetsTree->currentItem()->data( 0, DnDTreeRole ).value<DnDTreeItemData>().type() )
  {
    case DnDTreeItemData::Relation:
    {
      mAttributeTypeDialog->setVisible( false );
      loadAttributeRelationEdit();
      break;
    }
    case DnDTreeItemData::Field:
    {
      mAttributeRelationEdit->setVisible( false );
      loadAttributeTypeDialog();
      break;
    }
    case DnDTreeItemData::Container:
    {
      mAttributeRelationEdit->setVisible( false );
      mAttributeTypeDialog->setVisible( false );
      break;
    }
    case DnDTreeItemData::QmlWidget:
    {
      mAttributeRelationEdit->setVisible( false );
      mAttributeTypeDialog->setVisible( false );
      break;
    }

  }
}


void QgsAttributesFormProperties::addTabOrGroupButton()
{
  QList<QgsAddTabOrGroup::TabPair> tabList;

  for ( QTreeWidgetItemIterator it( mFormLayoutTree ); *it; ++it )
  {
    DnDTreeItemData itemData = ( *it )->data( 0, DnDTreeRole ).value<DnDTreeItemData>();
    if ( itemData.type() == DnDTreeItemData::Container )
    {
      tabList.append( QgsAddTabOrGroup::TabPair( itemData.name(), *it ) );
    }
  }
  QgsAddTabOrGroup addTabOrGroup( mLayer, tabList, this );

  if ( !addTabOrGroup.exec() )
    return;

  QString name = addTabOrGroup.name();
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

  DnDTreeItemData itemData = item->data( 0, DnDTreeRole ).value<DnDTreeItemData>();

  switch ( itemData.type() )
  {
    //indexed here?
    case DnDTreeItemData::Field:
    {
      int idx = mLayer->fields().lookupField( itemData.name() );
      widgetDef = new QgsAttributeEditorField( itemData.name(), idx, parent );
      break;
    }

    case DnDTreeItemData::Relation:
    {
      QgsRelation relation = QgsProject::instance()->relationManager()->relation( itemData.name() );
      QgsAttributeEditorRelation *relDef = new QgsAttributeEditorRelation( relation, parent );
      relDef->setShowLinkButton( itemData.relationEditorConfiguration().showLinkButton );
      relDef->setShowUnlinkButton( itemData.relationEditorConfiguration().showUnlinkButton );
      widgetDef = relDef;
      break;
    }

    case DnDTreeItemData::Container:
    {
      QgsAttributeEditorContainer *container = new QgsAttributeEditorContainer( item->text( 0 ), parent );
      container->setColumnCount( itemData.columnCount() );
      container->setIsGroupBox( forceGroup ? true : itemData.showAsGroupBox() );
      container->setVisibilityExpression( itemData.visibilityExpression() );

      for ( int t = 0; t < item->childCount(); t++ )
      {
        container->addChildElement( createAttributeEditorWidget( item->child( t ), container ) );
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
  }

  widgetDef->setShowLabel( itemData.showLabel() );

  return widgetDef;
}

void QgsAttributesFormProperties::mEditorLayoutComboBox_currentIndexChanged( int index )
{
  switch ( index )
  {
    case 0:
      mFormLayoutWidget->setVisible( false );
      mUiFileFrame->setVisible( false );
      mAddTabOrGroupButton->setVisible( false );
      mRemoveTabOrGroupButton->setVisible( false );
      break;

    case 1:
      mFormLayoutWidget->setVisible( true );
      mUiFileFrame->setVisible( false );
      mAddTabOrGroupButton->setVisible( true );
      mRemoveTabOrGroupButton->setVisible( true );
      break;

    case 2:
      mFormLayoutWidget->setVisible( false );
      mUiFileFrame->setVisible( true );
      mAddTabOrGroupButton->setVisible( false );
      mRemoveTabOrGroupButton->setVisible( false );
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
  QString lastUsedDir = myQSettings.value( QStringLiteral( "style/lastUIDir" ), QDir::homePath() ).toString();
  QString uifilename = QFileDialog::getOpenFileName( this, tr( "Select edit form" ), lastUsedDir, tr( "UI file" )  + " (*.ui)" );

  if ( uifilename.isNull() )
    return;

  QFileInfo fi( uifilename );
  myQSettings.setValue( QStringLiteral( "style/lastUIDir" ), fi.path() );
  mEditFormLineEdit->setText( uifilename );
}

void QgsAttributesFormProperties::apply()
{

  storeAttributeTypeDialog();
  storeAttributeRelationEdit();

  QgsEditFormConfig editFormConfig = mLayer->editFormConfig();

  QTreeWidgetItem *fieldContainer = mAvailableWidgetsTree->invisibleRootItem()->child( 0 );

  for ( int i = 0; i < fieldContainer->childCount(); i++ )
  {
    QTreeWidgetItem *fieldItem = fieldContainer->child( i );
    FieldConfig cfg = fieldItem->data( 0, FieldConfigRole ).value<FieldConfig>();

    int idx = mLayer->fields().indexOf( fieldItem->data( 0, FieldNameRole ).toString() );

    //continue in case field does not exist anymore
    if ( idx < 0 )
      continue;

    editFormConfig.setReadOnly( idx, !cfg.mEditable );
    editFormConfig.setLabelOnTop( idx, cfg.mLabelOnTop );
    mLayer->setConstraintExpression( idx, cfg.mConstraint, cfg.mConstraintDescription );
    mLayer->setEditorWidgetSetup( idx, QgsEditorWidgetSetup( cfg.mEditorWidgetType, cfg.mEditorWidgetConfig ) );

    if ( cfg.mConstraints & QgsFieldConstraints::ConstraintNotNull )
    {
      mLayer->setFieldConstraint( idx, QgsFieldConstraints::ConstraintNotNull, cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintStrengthHard ) );
    }
    else
    {
      mLayer->removeFieldConstraint( idx, QgsFieldConstraints::ConstraintNotNull );
    }
    if ( cfg.mConstraints & QgsFieldConstraints::ConstraintUnique )
    {
      mLayer->setFieldConstraint( idx, QgsFieldConstraints::ConstraintUnique, cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthHard ) );
    }
    else
    {
      mLayer->removeFieldConstraint( idx, QgsFieldConstraints::ConstraintUnique );
    }
    if ( cfg.mConstraints & QgsFieldConstraints::ConstraintExpression )
    {
      mLayer->setFieldConstraint( idx, QgsFieldConstraints::ConstraintExpression, cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthHard ) );
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

    editFormConfig.addTab( createAttributeEditorWidget( tabItem, nullptr, false ) );
  }

  editFormConfig.setUiForm( mEditFormLineEdit->text() );

  editFormConfig.setLayout( ( QgsEditFormConfig::EditorLayout ) mEditorLayoutComboBox->currentIndex() );

  editFormConfig.setInitCodeSource( mInitCodeSource );
  editFormConfig.setInitFunction( mInitFunction );
  editFormConfig.setInitFilePath( mInitFilePath );
  editFormConfig.setInitCode( mInitCode );

  editFormConfig.setSuppress( ( QgsEditFormConfig::FeatureFormSuppress )mFormSuppressCmbBx->currentIndex() );

  // relations
  QTreeWidgetItem *relationContainer = mAvailableWidgetsTree->invisibleRootItem()->child( 1 );

  for ( int i = 0; i < relationContainer->childCount(); i++ )
  {
    QTreeWidgetItem *relationItem = relationContainer->child( i );
    DnDTreeItemData itemData = relationItem->data( 0, DnDTreeRole ).value<DnDTreeItemData>();

    RelationConfig relCfg = configForRelation( itemData.name() );

    QVariantMap cfg;
    cfg[QStringLiteral( "nm-rel" )] = relCfg.mCardinality.toString();

    editFormConfig.setWidgetConfig( itemData.name(), cfg );
  }

  mLayer->setEditFormConfig( editFormConfig );
}


/*
 * FieldConfig implementation
 */

QgsAttributesFormProperties::FieldConfig::FieldConfig( QgsVectorLayer *layer, int idx )
{
  mAlias = layer->fields().at( idx ).alias();
  mComment = layer->fields().at( idx ).comment();
  mEditable = !layer->editFormConfig().readOnly( idx );
  mEditableEnabled = layer->fields().fieldOrigin( idx ) != QgsFields::OriginJoin
                     && layer->fields().fieldOrigin( idx ) != QgsFields::OriginExpression;
  mLabelOnTop = layer->editFormConfig().labelOnTop( idx );
  mFieldConstraints = layer->fields().at( idx ).constraints();
  mConstraints = mFieldConstraints.constraints();
  mConstraint = mFieldConstraints.constraintExpression();
  mConstraintStrength.insert( QgsFieldConstraints::ConstraintNotNull, mFieldConstraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) );
  mConstraintStrength.insert( QgsFieldConstraints::ConstraintUnique, mFieldConstraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) );
  mConstraintStrength.insert( QgsFieldConstraints::ConstraintExpression, mFieldConstraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) );
  mConstraintDescription = mFieldConstraints.constraintDescription();
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( layer, layer->fields().field( idx ).name() );
  mEditorWidgetType = setup.type();
  mEditorWidgetConfig = setup.config();
}

QgsAttributesFormProperties::FieldConfig::operator QVariant()
{
  return QVariant::fromValue<QgsAttributesFormProperties::FieldConfig>( *this );
}

/*
 * RelationConfig implementation
 */
QgsAttributesFormProperties::RelationConfig::RelationConfig() = default;

QgsAttributesFormProperties::RelationConfig::RelationConfig( QgsVectorLayer *layer, const QString &relationId )
{
  const QVariant nmrelcfg = layer->editFormConfig().widgetConfig( relationId ).value( QStringLiteral( "nm-rel" ) );

  mCardinality = nmrelcfg;
}

QgsAttributesFormProperties::RelationConfig::operator QVariant()
{
  return QVariant::fromValue<QgsAttributesFormProperties::RelationConfig>( *this );
}

/*
 * DnDTree implementation
 */

QTreeWidgetItem *DnDTree::addContainer( QTreeWidgetItem *parent, const QString &title, int columnCount )
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

DnDTree::DnDTree( QgsVectorLayer *layer, QWidget *parent )
  : QTreeWidget( parent )
  , mLayer( layer )
{
  connect( this, &QTreeWidget::itemDoubleClicked, this, &DnDTree::onItemDoubleClicked );
}

QTreeWidgetItem *DnDTree::addItem( QTreeWidgetItem *parent, QgsAttributesFormProperties::DnDTreeItemData data, int index )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( QStringList() << data.name() );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
  if ( data.type() == QgsAttributesFormProperties::DnDTreeItemData::Container )
  {
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
    newItem->setBackground( 0, QBrush( Qt::lightGray ) );

    switch ( data.type() )
    {
      case QgsAttributesFormProperties::DnDTreeItemData::Field:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mFieldIcon.svg" ) ) );
        break;

      case QgsAttributesFormProperties::DnDTreeItemData::Relation:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mRelationIcon.svg" ) ) );
        break;

      case QgsAttributesFormProperties::DnDTreeItemData::Container:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mContainerIcon.svg" ) ) );
        break;

      case QgsAttributesFormProperties::DnDTreeItemData::QmlWidget:
        //no icon for QmlWidget
        break;
    }
  }
  newItem->setData( 0, QgsAttributesFormProperties::DnDTreeRole, data );
  newItem->setText( 0, data.displayName() );

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

void DnDTree::dragMoveEvent( QDragMoveEvent *event )
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


bool DnDTree::dropMimeData( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action )
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
        newItem = addItem( parent, itemElement, index );
        bDropSuccessful = true;
      }
      else
      {
        newItem = addItem( invisibleRootItem(), itemElement, index );
        bDropSuccessful = true;
      }

      if ( itemElement.type() == QgsAttributesFormProperties::DnDTreeItemData::QmlWidget )
      {
        onItemDoubleClicked( newItem, 0 );
      }
    }
  }

  return bDropSuccessful;
}

void DnDTree::dropEvent( QDropEvent *event )
{
  if ( !event->mimeData()->hasFormat( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) ) )
    return;

  if ( event->source() == this )
  {
    event->setDropAction( Qt::MoveAction );
  }

  QTreeWidget::dropEvent( event );
}

QStringList DnDTree::mimeTypes() const
{
  return QStringList() << QStringLiteral( "application/x-qgsattributetabledesignerelement" );
}

QMimeData *DnDTree::mimeData( const QList<QTreeWidgetItem *> items ) const
{
  if ( items.count() <= 0 )
    return nullptr;

  QStringList types = mimeTypes();

  if ( types.isEmpty() )
    return nullptr;

  QMimeData *data = new QMimeData();
  QString format = types.at( 0 );
  QByteArray encoded;
  QDataStream stream( &encoded, QIODevice::WriteOnly );

  Q_FOREACH ( const QTreeWidgetItem *item, items )
  {
    if ( item )
    {
      // Relevant information is always in the DnDTreeRole of the first column
      QgsAttributesFormProperties::DnDTreeItemData itemData = item->data( 0, QgsAttributesFormProperties::DnDTreeRole ).value<QgsAttributesFormProperties::DnDTreeItemData>();
      stream << itemData;
    }
  }

  data->setData( format, encoded );

  return data;
}

void DnDTree::onItemDoubleClicked( QTreeWidgetItem *item, int column )
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
    case QgsAttributesFormProperties::DnDTreeItemData::Container:
    {
      QDialog dlg;
      dlg.setWindowTitle( tr( "Configure Container" ) );
      QFormLayout *layout = new QFormLayout() ;
      dlg.setLayout( layout );
      layout->addRow( baseWidget );

      QCheckBox *showAsGroupBox = nullptr;
      QLineEdit *title = new QLineEdit( itemData.name() );
      QSpinBox *columnCount = new QSpinBox();
      QGroupBox *visibilityExpressionGroupBox = new QGroupBox( tr( "Control visibility by expression" ) );
      visibilityExpressionGroupBox->setCheckable( true );
      visibilityExpressionGroupBox->setChecked( itemData.visibilityExpression().enabled() );
      visibilityExpressionGroupBox->setLayout( new QGridLayout );
      QgsFieldExpressionWidget *visibilityExpressionWidget = new QgsFieldExpressionWidget;
      visibilityExpressionWidget->setLayer( mLayer );
      visibilityExpressionWidget->setExpressionDialogTitle( tr( "Visibility Expression" ) );
      visibilityExpressionWidget->setExpression( itemData.visibilityExpression()->expression() );
      visibilityExpressionGroupBox->layout()->addWidget( visibilityExpressionWidget );

      columnCount->setRange( 1, 5 );
      columnCount->setValue( itemData.columnCount() );

      layout->addRow( tr( "Title" ), title );
      layout->addRow( tr( "Column count" ), columnCount );
      layout->addRow( visibilityExpressionGroupBox );

      if ( !item->parent() )
      {
        showAsGroupBox = new QCheckBox( tr( "Show as group box" ) );
        showAsGroupBox->setChecked( itemData.showAsGroupBox() );
        layout->addRow( showAsGroupBox );
      }

      QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok
          | QDialogButtonBox::Cancel );

      connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
      connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );

      layout->addWidget( buttonBox );

      if ( dlg.exec() )
      {
        itemData.setColumnCount( columnCount->value() );
        itemData.setShowAsGroupBox( showAsGroupBox ? showAsGroupBox->isChecked() : true );
        itemData.setName( title->text() );
        itemData.setShowLabel( showLabelCheckbox->isChecked() );

        QgsOptionalExpression visibilityExpression;
        visibilityExpression.setData( QgsExpression( visibilityExpressionWidget->expression() ) );
        visibilityExpression.setEnabled( visibilityExpressionGroupBox->isChecked() );
        itemData.setVisibilityExpression( visibilityExpression );

        item->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData );
        item->setText( 0, title->text() );
      }
    }
    break;

    case  QgsAttributesFormProperties::DnDTreeItemData::Relation:
    {
      QDialog dlg;
      dlg.setWindowTitle( tr( "Configure Relation Editor" ) );
      QFormLayout *layout = new QFormLayout() ;
      dlg.setLayout( layout );
      layout->addWidget( baseWidget );

      QCheckBox *showLinkButton = new QCheckBox( tr( "Show link button" ) );
      showLinkButton->setChecked( itemData.relationEditorConfiguration().showLinkButton );
      QCheckBox *showUnlinkButton = new QCheckBox( tr( "Show unlink button" ) );
      showUnlinkButton->setChecked( itemData.relationEditorConfiguration().showUnlinkButton );
      layout->addRow( showLinkButton );
      layout->addRow( showUnlinkButton );

      QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

      connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
      connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );

      dlg.layout()->addWidget( buttonBox );

      if ( dlg.exec() )
      {
        QgsAttributesFormProperties::RelationEditorConfiguration relEdCfg;
        relEdCfg.showLinkButton = showLinkButton->isChecked();
        relEdCfg.showUnlinkButton = showUnlinkButton->isChecked();
        itemData.setShowLabel( showLabelCheckbox->isChecked() );
        itemData.setRelationEditorConfiguration( relEdCfg );

        item->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData );
      }
    }
    break;

    case QgsAttributesFormProperties::DnDTreeItemData::QmlWidget:
    {
      QDialog dlg;
      dlg.setWindowTitle( tr( "Configure QML Widget" ) );

      QVBoxLayout *mainLayout = new QVBoxLayout();
      QHBoxLayout *qmlLayout = new QHBoxLayout();
      QFormLayout *layout = new QFormLayout();
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
      qmlObjectTemplate->addItem( tr( "Free text…" ) );
      qmlObjectTemplate->addItem( tr( "Rectangle" ) );
      qmlObjectTemplate->addItem( tr( "Pie chart" ) );
      qmlObjectTemplate->addItem( tr( "Bar chart" ) );
      connect( qmlObjectTemplate, qgis::overload<int>::of( &QComboBox::activated ), qmlCode, [ = ]( int index )
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
                                      "\n"
                                      "    BarSeries {\n"
                                      "        id: mySeries\n"
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

      layout->addRow( tr( "Title" ), title );
      QGroupBox *qmlCodeBox = new QGroupBox( tr( "QML Code" ) );
      qmlCodeBox->setLayout( new QGridLayout );
      qmlCodeBox->layout()->addWidget( qmlObjectTemplate );
      QGroupBox *expressionWidgetBox = new QGroupBox();
      qmlCodeBox->layout()->addWidget( expressionWidgetBox );
      expressionWidgetBox->setLayout( new QHBoxLayout );
      expressionWidgetBox->layout()->addWidget( expressionWidget );
      expressionWidgetBox->layout()->addWidget( addExpressionButton );
      qmlCodeBox->layout()->addWidget( qmlCode );
      layout->addRow( qmlCodeBox );
      QScrollArea *qmlPreviewBox = new QScrollArea();
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

    case QgsAttributesFormProperties::DnDTreeItemData::Field:
    {
      QDialog dlg;
      dlg.setWindowTitle( tr( "Configure Field" ) );
      dlg.setLayout( new QGridLayout() );
      dlg.layout()->addWidget( baseWidget );

      QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok
          | QDialogButtonBox::Cancel );

      connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
      connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );

      dlg.layout()->addWidget( buttonBox );

      if ( dlg.exec() )
      {
        itemData.setShowLabel( showLabelCheckbox->isChecked() );

        item->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData );
      }
    }
    break;
  }
}

DnDTree::Type DnDTree::type() const
{
  return mType;
}

void DnDTree::setType( DnDTree::Type value )
{
  mType = value;
}


/*
 * Serialization helpers for DesigerTreeItemData so we can stuff this easily into QMimeData
 */

QDataStream &operator<<( QDataStream &stream, const QgsAttributesFormProperties::DnDTreeItemData &data )
{
  stream << ( quint32 )data.type() << data.name() << data.displayName();
  return stream;
}

QDataStream &operator>>( QDataStream &stream, QgsAttributesFormProperties::DnDTreeItemData &data )
{
  QString name;
  QString displayName;
  quint32 type;

  stream >> type >> name >> displayName;

  data.setType( ( QgsAttributesFormProperties::DnDTreeItemData::Type )type );
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

