#include "qgsattributesformproperties.h"
#include "qgsattributetypedialog.h"
#include "qgsattributerelationedit.h"

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
  mAttributeTypeDialog = new QgsAttributeTypeDialog( mLayer, 0, mAttributeTypeFrame );
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
}


QgsAttributesFormProperties::~QgsAttributesFormProperties()
{

}

void QgsAttributesFormProperties::init()
{
  initAvailableWidgetsTree();
  initFormLayoutTree();

  mAttributeTypeDialog->setEnabled( false );
  mAttributeRelationEdit->setEnabled( false );
}

void QgsAttributesFormProperties::loadAttributeTypeDialog()
{
  QTreeWidgetItem *currentItem = mAvailableWidgetsTree->currentItem();

  if ( !currentItem )
    mAttributeTypeDialog->setEnabled( false );
  else
  {
    FieldConfig cfg = mAvailableWidgetsTree->currentItem()->data( 0, FieldConfigRole ).value<FieldConfig>();
    QString fieldName = mAvailableWidgetsTree->currentItem()->data( 0, FieldNameRole ).toString();
    int index = mLayer->fields().indexOf( fieldName );

    mAttributeTypeDialog->setEnabled( true );

    // AttributeTypeDialog

    mAttributeTypeFrame->layout()->removeWidget( mAttributeTypeDialog );
    delete mAttributeTypeDialog;

    //
    mAttributeTypeDialog = new QgsAttributeTypeDialog( mLayer, index, mAttributeTypeFrame );
    mAttributeTypeDialog->setAlias( cfg.mAlias );
    mAttributeTypeDialog->setFieldEditable( cfg.mEditable );
    mAttributeTypeDialog->setLabelOnTop( cfg.mLabelOnTop );
    mAttributeTypeDialog->setNotNull( cfg.mConstraints & QgsFieldConstraints::ConstraintNotNull );
    mAttributeTypeDialog->setNotNullEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );
    mAttributeTypeDialog->setUnique( cfg.mConstraints & QgsFieldConstraints::ConstraintUnique );
    mAttributeTypeDialog->setUniqueEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );

    //confustion (will be removed): when we make this, the contraint stuff is allways reloaded from layer when we only change the widged-selection... : QgsFieldConstraints constraints = mLayer->fields().at( index ).constraints();
    QgsFieldConstraints constraints = cfg.mFieldConstraints;
    QgsFieldConstraints::Constraints providerConstraints = 0;
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
    //confustion (will be removed): das hier funktioniert nicht, es is neu, aber ich weiss nicht woher: mAttributeTypeDialog->setDefaultValueExpression( mLayer->defaultValueExpression( index ) );

    mAttributeTypeDialog->setEditorWidgetConfig( cfg.mEditorWidgetConfig );
    mAttributeTypeDialog->setEditorWidgetType( cfg.mEditorWidgetType );

    mAttributeTypeDialog->layout()->setMargin( 0 );
    mAttributeTypeFrame->layout()->setMargin( 0 );

    mAttributeTypeFrame->layout()->addWidget( mAttributeTypeDialog );
  }
}


void QgsAttributesFormProperties::storeAttributeTypeDialog()
{
  FieldConfig cfg;

  cfg.mEditable = mAttributeTypeDialog->fieldEditable();
  cfg.mLabelOnTop = mAttributeTypeDialog->labelOnTop();
  cfg.mAlias = mAttributeTypeDialog->alias();
  cfg.mComment = mAttributeTypeDialog->comment();

  //confustion (will be removed): wir laden teilweise sachen einfach beim store anstelle des applys auf die mLayer - eingie Sachen laden wir auch vom layer anstatt über das cfg. wieso
  QgsFieldConstraints constraints = mLayer->fields().at( mAttributeTypeDialog->fieldIdx() ).constraints();
  QgsFieldConstraints::Constraints providerConstraints = 0;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintNotNull;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintUnique;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintExpression;
  cfg.mConstraints = 0;
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
  //confustion (will be removed): das hier funktioniert nicht, es is neu, aber ich weiss nicht woher: mLayer->setDefaultValueExpression( mAttributeTypeDialog->fieldIdx(), mAttributeTypeDialog->defaultValueExpression() );
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

    //oder mit dem? RelationConfig relCfg = configForRelation( itemData.name() );
    RelationConfig cfg = currentItem->data( 0, RelationConfigRole).value<RelationConfig>();

    mAttributeRelationEdit = new QgsAttributeRelationEdit( currentItem->data( 0, FieldNameRole ).toString(), mAttributeTypeFrame );
    mAttributeRelationEdit->setCardinalityCombo( "testoption 1");
    mAttributeRelationEdit->setCardinalityCombo( "testoption 2");
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

    QTreeWidgetItem* relationContainer=mAvailableWidgetsTree->invisibleRootItem()->child(1);

    for ( int i = 0; i < relationContainer->childCount(); i++ )
    {
      QTreeWidgetItem *relationItem = relationContainer->child( i );
      DnDTreeItemData itemData= relationItem->data( 0, DnDTreeRole ).value<DnDTreeItemData>();

      if( itemData.name()==mAttributeRelationEdit->mRelationId ){
        relationItem->setData( 0, RelationConfigRole, QVariant::fromValue<RelationConfig>( cfg ) );
      }
    }
}

QgsAttributesFormProperties::FieldConfig QgsAttributesFormProperties::configForChild( int index )
{
  QString fieldName = mLayer->fields().at( index ).name();

  QTreeWidgetItemIterator itemIt( mAvailableWidgetsTree );
  while ( *itemIt )
  {
    QTreeWidgetItem *item = *itemIt;
    if ( item->data( 0, FieldNameRole ).toString() == fieldName )
      return item->data( 0, FieldConfigRole ).value<FieldConfig>();
    ++itemIt;
  }

  // Should never get here
  Q_ASSERT( false );
  return FieldConfig();
}

QgsAttributesFormProperties::RelationConfig QgsAttributesFormProperties::configForRelation( const QString &relationName )
{
  QTreeWidgetItemIterator itemIt( mAvailableWidgetsTree );
  while ( *itemIt )
  {
    QTreeWidgetItem *item = *itemIt;
    if ( item->data( 0, FieldNameRole ).toString() == relationName )
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
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Field, widgetDef->name() );
      itemData.setShowLabel( widgetDef->showLabel() );
      newWidget = tree->addItem( parent, itemData );
      break;
    }

    case QgsAttributeEditorElement::AeTypeRelation:
    {
      const QgsAttributeEditorRelation *relationEditor = static_cast<const QgsAttributeEditorRelation *>( widgetDef );
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Relation, widgetDef->name() );
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
      DnDTreeItemData itemData( DnDTreeItemData::Container, widgetDef->name() );
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
    }
    break;
  }
  return newWidget;
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


void QgsAttributesFormProperties::onAttributeSelectionChanged()
{
  bool isAddPossible = false;
  if ( mFormLayoutTree->selectedItems().count() == 1 && !mAvailableWidgetsTree->selectedItems().isEmpty() )
    if ( mFormLayoutTree->selectedItems()[0]->data( 0, DnDTreeRole ).value<DnDTreeItemData>().type() == DnDTreeItemData::Container )
      isAddPossible = true;

  storeAttributeTypeDialog();
  storeAttributeRelationEdit();

  switch ( mAvailableWidgetsTree->currentItem()->data( 0, DnDTreeRole ).value<DnDTreeItemData>().type() )
  {
    case DnDTreeItemData::Relation:
    {
      mAttributeTypeDialog->setEnabled( false );
      loadAttributeRelationEdit();
      break;
    }
    case DnDTreeItemData::Field:
    {
      mAttributeRelationEdit->setEnabled( false );
      loadAttributeTypeDialog();
      break;
    }
  }
}

void QgsAttributesFormProperties::initAvailableWidgetsTree()
{
  mAvailableWidgetsTree->clear();
  mAvailableWidgetsTree->setSortingEnabled( false );
  mAvailableWidgetsTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  mAvailableWidgetsTree->setAcceptDrops( false );
  mAvailableWidgetsTree->setDragDropMode( QAbstractItemView::DragOnly );

  //load Fields

  DnDTreeItemData catItemData = DnDTreeItemData( DnDTreeItemData::Container, "Fields");
  QTreeWidgetItem *catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  const QgsFields fields = mLayer->fields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    const QgsField field = fields.at( i );
    DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Field, field.name() );
    //should we load here stuff like in im loadAttributeEditorTreeItem other stuff like itemData.setShowLabel( true );?
    itemData.setShowLabel( true );

    FieldConfig cfg( mLayer, i );

    QTreeWidgetItem *item = mAvailableWidgetsTree->addItem( catitem, itemData );
    //QTreeWidgetItem *item = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), itemData );

    item->setData( 0, FieldConfigRole, cfg );
    item->setData( 0, FieldNameRole, field.name() );
  }

  /* stuff
  itemData.setIcon(i, mLayer->fields().iconForField( i ));
  itemData.setText(i, QString::number( i+1 ) );
  itemData.setText(i, fields.at( i ).name() );
  */


  //load Relations
  catItemData = DnDTreeItemData( DnDTreeItemData::Container, "Relations");
  catitem = mAvailableWidgetsTree->addItem( mAvailableWidgetsTree->invisibleRootItem(), catItemData );

  const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencedRelations( mLayer );

  for ( const QgsRelation &relation : relations )
  {
    DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Relation, QStringLiteral( "%1" ).arg( relation.id() )); //relation.name() );
    itemData.setShowLabel( true );

    //daveRelation
    RelationConfig cfg( mLayer, relation.id() );

    QTreeWidgetItem *item = mAvailableWidgetsTree->addItem( catitem, itemData );
    item->setData( 0, RelationConfigRole, cfg );
    item->setData( 0, FieldNameRole, QStringLiteral( "%1" ).arg( relation.id() ) ); //relation.name() );
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
      QgsAttributeEditorRelation *relDef = new QgsAttributeEditorRelation( itemData.name(), relation, parent );
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
  }

  widgetDef->setShowLabel( itemData.showLabel() );

  return widgetDef;
}


void QgsAttributesFormProperties::apply()
{
  if( mAttributeTypeDialog )
  {
    storeAttributeTypeDialog();
  }else
  {
    storeAttributeRelationEdit();
  }

  QgsEditFormConfig editFormConfig = mLayer->editFormConfig();

  QTreeWidgetItem* fieldContainer=mAvailableWidgetsTree->invisibleRootItem()->child(0);

  int idx;

  for ( int i = 0; i < fieldContainer->childCount(); i++ )
  {
    QTreeWidgetItem *fieldItem = fieldContainer->child( i );
    idx=fieldContainer->indexOfChild( fieldItem );

    QString name = mLayer->fields().at( idx ).name();
    FieldConfig cfg = configForChild( idx );

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
    //wo?? ( idx, cfg.mComment );
  }

  // tabs and groups
  editFormConfig.clearTabs();
  for ( int t = 0; t < mFormLayoutTree->invisibleRootItem()->childCount(); t++ )
  {
    QTreeWidgetItem *tabItem = mFormLayoutTree->invisibleRootItem()->child( t );

    editFormConfig.addTab( createAttributeEditorWidget( tabItem, nullptr, false ) );
  }

  /*
  editFormConfig.setUiForm( mEditFormLineEdit->text() );
  editFormConfig.setLayout( ( QgsEditFormConfig::EditorLayout ) mEditorLayoutComboBox->currentIndex() );

  // Init function configuration
  editFormConfig.setInitFunction( mInitFunctionLineEdit->text() );
  editFormConfig.setInitCode( mInitCodeEditorPython->text() );
  editFormConfig.setInitFilePath( mInitFilePathLineEdit->text() );
  editFormConfig.setInitCodeSource( ( QgsEditFormConfig::PythonInitCodeSource )mInitCodeSourceComboBox->currentIndex() );
  editFormConfig.setSuppress( ( QgsEditFormConfig::FeatureFormSuppress )mFormSuppressCmbBx->currentIndex() );
  */


  // relations
  QTreeWidgetItem* relationContainer=mAvailableWidgetsTree->invisibleRootItem()->child(1);

  for ( int i = 0; i < relationContainer->childCount(); i++ )
  {
    QTreeWidgetItem *relationItem = relationContainer->child( i );
    DnDTreeItemData itemData= relationItem->data( 0, DnDTreeRole ).value<DnDTreeItemData>();

    RelationConfig relCfg = configForRelation( itemData.name() );

    QVariantMap cfg;
    cfg[QStringLiteral( "nm-rel" )]=relCfg.mCardinality;

    editFormConfig.setWidgetConfig( itemData.name(), cfg );
  }

/*

  for ( int i = 0; i < mRelationsList->rowCount(); ++i )
  {
    QVariantMap cfg;

    QComboBox *cb = qobject_cast<QComboBox *>( mRelationsList->cellWidget( i, RelNmCol ) );
    QVariant otherRelation = cb->currentData();

    if ( otherRelation.isValid() )
    {
      cfg[QStringLiteral( "nm-rel" )] = otherRelation.toString();
    }

    DesignerTreeItemData itemData = mRelationsList->item( i, RelNameCol )->data( DesignerTreeRole ).value<DesignerTreeItemData>();

    QString relationName = itemData.name();

    editFormConfig.setWidgetConfig( relationName, cfg );
  }
*/

  mLayer->setEditFormConfig( editFormConfig );
}


/*
 * FieldConfig implementation
 */

QgsAttributesFormProperties::FieldConfig::FieldConfig()
  : mEditable( true )
  , mEditableEnabled( true )
  , mLabelOnTop( false )
  , mConstraints( 0 )
  , mConstraintDescription( QString() )
  , mButton( nullptr )
{
}

QgsAttributesFormProperties::FieldConfig::FieldConfig( QgsVectorLayer *layer, int idx )
  : mButton( nullptr )
{
  mAlias=layer->fields().at( idx ).alias();
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
QgsAttributesFormProperties::RelationConfig::RelationConfig()
  : mCardinality( QString() )
{
}

QgsAttributesFormProperties::RelationConfig::RelationConfig( QgsVectorLayer *layer, const QString &relationId )
{
  const QVariant nmrelcfg = layer->editFormConfig().widgetConfig( relationId ).value( QStringLiteral( "nm-rel" ) );

  mCardinality=nmrelcfg.toString();
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
  QgsAttributesFormProperties::DnDTreeItemData itemData( QgsAttributesFormProperties::DnDTreeItemData::Container, title );
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

QTreeWidgetItem *DnDTree::addItem( QTreeWidgetItem *parent, QgsAttributesFormProperties::DnDTreeItemData data )
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
        newItem->setIcon( 0, QgsApplication::getThemeIcon( "/mFieldIcon.svg" ) );
        break;

      case QgsAttributesFormProperties::DnDTreeItemData::Relation:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( "/mRelationIcon.svg" ) );
        break;

      case QgsAttributesFormProperties::DnDTreeItemData::Container:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( "/mContainerIcon.svg" ) );
        break;
    }
  }
  newItem->setData( 0, QgsAttributesFormProperties::DnDTreeRole, data );
  parent->addChild( newItem );

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
  Q_UNUSED( index )
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

      if ( parent )
      {
        addItem( parent, itemElement );
        bDropSuccessful = true;
      }
      else
      {
        addItem( invisibleRootItem(), itemElement );
        bDropSuccessful = true;
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

  if ( itemData.type() == QgsAttributesFormProperties::DnDTreeItemData::Container )
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
  else if ( itemData.type() == QgsAttributesFormProperties::DnDTreeItemData::Relation )
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
  else
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
}

DnDTree::Type DnDTree::type() const
{
  return mType;
}

void DnDTree::setType( const Type &value )
{
  mType = value;
}


/*
 * Serialization helpers for DesigerTreeItemData so we can stuff this easily into QMimeData
 */

QDataStream &operator<<( QDataStream &stream, const QgsAttributesFormProperties::DnDTreeItemData &data )
{
  stream << ( quint32 )data.type() << data.name();
  return stream;
}

QDataStream &operator>>( QDataStream &stream, QgsAttributesFormProperties::DnDTreeItemData &data )
{
  QString name;
  quint32 type;

  stream >> type >> name;

  data.setType( ( QgsAttributesFormProperties::DnDTreeItemData::Type )type );
  data.setName( name );

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

