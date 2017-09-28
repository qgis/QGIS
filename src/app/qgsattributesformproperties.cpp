#include "qgsattributesformproperties.h"
#include "qgsattributetypedialog.h"

QgsAttributesFormProperties::QgsAttributesFormProperties( QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
  , mDragTree( nullptr )
  , mDropTree( nullptr )
  , mAttributeTypeDialog( nullptr )
{
  if ( !layer )
    return;

  setupUi( this );

  // drag tree (not mixup with drag queen)
  QGridLayout *mDragListWidgetLayout = new QGridLayout;
  mDragTree = new DnDTree( mLayer, mDragListWidget );
  mDragListWidgetLayout->addWidget( mDragTree );
  mDragListWidget->setLayout( mDragListWidgetLayout );
  mDragTree->setHeaderLabels( QStringList() << tr( "Label" ) );
  mDragTree->setType( DnDTree::Type::Drag );

  // drop tree
  QGridLayout *mDropListWidgetLayout = new QGridLayout;
  mDropTree = new DnDTree( mLayer, mDropListWidget );
  mDropListWidgetLayout->addWidget( mDropTree );
  mDropListWidgetLayout->setMargin( 0 );
  mDropListWidget->setLayout( mDropListWidgetLayout );
  mDropTree->setHeaderLabels( QStringList() << tr( "Label" ) );
  mDropTree->setType( DnDTree::Type::Drop );

  // AttributeTypeDialog
  mAttributeTypeDialog = new QgsAttributeTypeDialog( mLayer, 0, mAttributeTypeFrame );
  mAttributeTypeDialog->layout()->setMargin( 0 );
  mAttributeTypeFrame->setLayout( new QVBoxLayout( mAttributeTypeFrame ) );
  mAttributeTypeFrame->layout()->setMargin( 0 );
  mAttributeTypeFrame->layout()->addWidget( mAttributeTypeDialog );

  connect( mDragTree, &QTreeWidget::itemSelectionChanged, this, &QgsAttributesFormProperties::onAttributeSelectionChanged );
  connect( mDropTree, &QTreeWidget::itemSelectionChanged, this, &QgsAttributesFormProperties::onAttributeSelectionChanged );
  connect( mAddTabOrGroupButton, &QAbstractButton::clicked, this, &QgsAttributesFormProperties::addTabOrGroupButton );
  connect( mRemoveTabOrGroupButton, &QAbstractButton::clicked, this, &QgsAttributesFormProperties::removeTabOrGroupButton );
}


QgsAttributesFormProperties::~QgsAttributesFormProperties()
{

}

void QgsAttributesFormProperties::init()
{
  loadAttributeEditorTree( mDragTree );
  loadAttributeEditorTree( mDropTree );
  loadAttributeTypeDialog();
}

//Slots

void QgsAttributesFormProperties::loadAttributeTypeDialog()
{

  FieldConfig cfg;

  int index = mDragTree->mIndexedWidgets.indexOf( mDragTree->currentItem() );

  if ( index < 0 ) index = 0;

  Q_FOREACH ( QTreeWidgetItem *wdg, mDragTree->mIndexedWidgets )
  {
    if ( mDragTree->invisibleRootItem()->indexOfChild( wdg ) ==  index )
    {
      cfg = wdg->data( 0, FieldConfigRole ).value<FieldConfig>();
    }
  }

  // AttributeTypeDialog

  mAttributeTypeFrame->layout()->removeWidget( mAttributeTypeDialog );
  delete mAttributeTypeDialog;

  //
  mAttributeTypeDialog = new QgsAttributeTypeDialog( mLayer, index, mAttributeTypeFrame );

  mAttributeTypeDialog->setFieldEditable( cfg.mEditable );
  mAttributeTypeDialog->setLabelOnTop( cfg.mLabelOnTop );
  mAttributeTypeDialog->setNotNull( cfg.mConstraints & QgsFieldConstraints::ConstraintNotNull );
  mAttributeTypeDialog->setNotNullEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );
  mAttributeTypeDialog->setUnique( cfg.mConstraints & QgsFieldConstraints::ConstraintUnique );
  mAttributeTypeDialog->setUniqueEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );

  QgsFieldConstraints constraints = mLayer->fields().at( index ).constraints();
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
  mAttributeTypeDialog->setDefaultValueExpression( mLayer->defaultValueExpression( index ) );

  mAttributeTypeDialog->setEditorWidgetConfig( cfg.mEditorWidgetConfig );
  mAttributeTypeDialog->setEditorWidgetType( cfg.mEditorWidgetType );

  mAttributeTypeDialog->layout()->setMargin( 0 );
  mAttributeTypeFrame->setLayout( new QVBoxLayout( mAttributeTypeFrame ) );
  mAttributeTypeFrame->layout()->setMargin( 0 );

  mAttributeTypeFrame->layout()->addWidget( mAttributeTypeDialog );
}


void QgsAttributesFormProperties::storeAttributeTypeDialog()
{

  FieldConfig cfg;

  cfg.mEditable = mAttributeTypeDialog->fieldEditable();
  cfg.mLabelOnTop = mAttributeTypeDialog->labelOnTop();

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
  mLayer->setDefaultValueExpression( mAttributeTypeDialog->fieldIdx(), mAttributeTypeDialog->defaultValueExpression() );

  cfg.mEditorWidgetType = mAttributeTypeDialog->editorWidgetType();
  cfg.mEditorWidgetConfig = mAttributeTypeDialog->editorWidgetConfig();

  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintNotNull, mAttributeTypeDialog->notNullEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintUnique, mAttributeTypeDialog->uniqueEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintExpression, mAttributeTypeDialog->constraintExpressionEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );

  Q_FOREACH ( QTreeWidgetItem *wdg, mDragTree->mIndexedWidgets )
  {
    if ( mDragTree->invisibleRootItem()->indexOfChild( wdg ) ==  mAttributeTypeDialog->fieldIdx() )
    {
      wdg->setData( 0, FieldConfigRole, QVariant::fromValue<FieldConfig>( cfg ) );
    }
  }
}

QgsAttributesFormProperties::FieldConfig QgsAttributesFormProperties::configForChild( int index )
{
  Q_FOREACH ( QTreeWidgetItem *wdg, mDragTree->mIndexedWidgets )
  {
    if ( mDragTree->invisibleRootItem()->indexOfChild( wdg ) == index )
    {
      return wdg->data( 0, FieldConfigRole ).value<FieldConfig>();
    }
  }

  // Should never get here
  Q_ASSERT( false );
  return FieldConfig();
}


QTreeWidgetItem *QgsAttributesFormProperties::loadAttributeEditorTreeItem( QgsAttributeEditorElement *const widgetDef, QTreeWidgetItem *parent, DnDTree *mTree )
{
  QTreeWidgetItem *newWidget = nullptr;
  switch ( widgetDef->type() )
  {
    case QgsAttributeEditorElement::AeTypeField:
    {
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Field, widgetDef->name() );
      itemData.setShowLabel( widgetDef->showLabel() );
      newWidget = mTree->addItem( parent, itemData );
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

      newWidget = mTree->addItem( parent, itemData );
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
      newWidget = mTree->addItem( parent, itemData );

      Q_FOREACH ( QgsAttributeEditorElement *wdg, container->children() )
      {
        loadAttributeEditorTreeItem( wdg, newWidget, mTree );
      }
    }
    break;

    default:
      QgsDebugMsg( "Unknown attribute editor widget type encountered..." );
      break;
  }
  return newWidget;
}



void QgsAttributesFormProperties::loadAttributeEditorTree( DnDTree *mTree )
{
  if ( mTree->type() == DnDTree::Type::Drop )
  {
    // tabs and groups info
    mTree->clear();
    mTree->setSortingEnabled( false );
    mTree->setSelectionBehavior( QAbstractItemView::SelectRows );
    mTree->setDragDropMode( QAbstractItemView::InternalMove );
    mTree->setAcceptDrops( true );
    mTree->setDragDropMode( QAbstractItemView::DragDrop );

    Q_FOREACH ( QgsAttributeEditorElement *wdg, mLayer->editFormConfig().tabs() )
    {
      loadAttributeEditorTreeItem( wdg, mTree->invisibleRootItem(), mTree );
    }

  }
  else
  {
    mTree->clear();
    mTree->setSortingEnabled( false );
    mTree->setSelectionBehavior( QAbstractItemView::SelectRows );
    mTree->setDragDropMode( QAbstractItemView::InternalMove );
    mTree->setAcceptDrops( false );
    mTree->setDragDropMode( QAbstractItemView::DragOnly );

    mTree->mIndexedWidgets.clear();

    const QgsFields &fields = mLayer->fields();
    for ( int i = 0; i < fields.size(); ++i )
    {
      DnDTreeItemData itemData = DnDTreeItemData( DnDTreeItemData::Field, fields.at( i ).name() );
      itemData.setShowLabel( true );

      FieldConfig cfg( mLayer, i );
      QgsGui::editorWidgetRegistry()->name( cfg.mEditorWidgetType );


      itemData.setData( 0, FieldConfigRole, QVariant::fromValue<FieldConfig>( cfg ) );

      mTree->mIndexedWidgets.insert( i, mTree->addItem( mTree->invisibleRootItem(), itemData ) );
    }

    /*some stuff for containers
    //load Container Field
    DnDTreeItemData catItemData = DnDTreeItemData( DnDTreeItemData::Container, "Fields");
    catItemData.setShowLabel( true );
    QTreeWidgetItem *catWidget = nullptr;
    catWidget=mTree->addItem( mTree->invisibleRootItem(), catItemData );
    mTree->mIndexedWidgets.insert( i, mTree->addItem( catWidget, itemData ) );

    itemData.setIcon(i, mLayer->fields().iconForField( i ));
    itemData.setText(i, QString::number( i+1 ) );
    itemData.setText(i, fields.at( i ).name() );
    */
  }
}


void QgsAttributesFormProperties::onAttributeSelectionChanged()
{
  bool isAddPossible = false;
  if ( mDropTree->selectedItems().count() == 1 && !mDragTree->selectedItems().isEmpty() )
    if ( mDropTree->selectedItems()[0]->data( 0, DnDTreeRole ).value<DnDTreeItemData>().type() == DnDTreeItemData::Container )
      isAddPossible = true;

  storeAttributeTypeDialog();
  loadAttributeTypeDialog();
}


void QgsAttributesFormProperties::addTabOrGroupButton()
{
  QList<QgsAddTabOrGroup::TabPair> tabList;

  for ( QTreeWidgetItemIterator it( mDropTree ); *it; ++it )
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
    mDropTree->addContainer( mDropTree->invisibleRootItem(), name, addTabOrGroup.columnCount() );
  }
  else
  {
    QTreeWidgetItem *tabItem = addTabOrGroup.tab();
    mDropTree->addContainer( tabItem, name, addTabOrGroup.columnCount() );
  }
}

void QgsAttributesFormProperties::removeTabOrGroupButton()
{
  qDeleteAll( mDropTree->selectedItems() );
}


QgsAttributeEditorElement *QgsAttributesFormProperties::createAttributeEditorWidget( QTreeWidgetItem *item, QgsAttributeEditorElement *parent, bool forceGroup )
{
  QgsAttributeEditorElement *widgetDef = nullptr;

  DnDTreeItemData itemData = item->data( 0, DnDTreeRole ).value<DnDTreeItemData>();

  switch ( itemData.type() )
  {
    //dave hier auch indexed rein?
    case DnDTreeItemData::Field:
    {
      int idx = mLayer->fields().lookupField( itemData.name() );
      widgetDef = new QgsAttributeEditorField( itemData.name(), idx, parent );
      break;
    }

    /*dave to do
    case DnDTreeItemData::Relation:
    {
      QgsRelation relation = QgsProject::instance()->relationManager()->relation( itemData.name() );
      QgsAttributeEditorRelation *relDef = new QgsAttributeEditorRelation( itemData.name(), relation, parent );
      relDef->setShowLinkButton( itemData.relationEditorConfiguration().showLinkButton );
      relDef->setShowUnlinkButton( itemData.relationEditorConfiguration().showUnlinkButton );
      widgetDef = relDef;
      break;
    }
    */

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
  QgsEditFormConfig editFormConfig = mLayer->editFormConfig();

  for ( QTreeWidgetItemIterator it( mDragTree ); *it; ++it )
  {
    int idx = mDragTree->invisibleRootItem()->indexOfChild( ( *it ) );
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
  }

  // tabs and groups
  editFormConfig.clearTabs();
  for ( int t = 0; t < mDropTree->invisibleRootItem()->childCount(); t++ )
  {
    QTreeWidgetItem *tabItem = mDropTree->invisibleRootItem()->child( t );

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

  // relations
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
  mEditable = !layer->editFormConfig().readOnly( idx );
  mEditableEnabled = layer->fields().fieldOrigin( idx ) != QgsFields::OriginJoin
                     && layer->fields().fieldOrigin( idx ) != QgsFields::OriginExpression;
  mLabelOnTop = layer->editFormConfig().labelOnTop( idx );
  QgsFieldConstraints constraints = layer->fields().at( idx ).constraints();
  mConstraints = constraints.constraints();
  mConstraint = constraints.constraintExpression();
  mConstraintStrength.insert( QgsFieldConstraints::ConstraintNotNull, constraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) );
  mConstraintStrength.insert( QgsFieldConstraints::ConstraintUnique, constraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) );
  mConstraintStrength.insert( QgsFieldConstraints::ConstraintExpression, constraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) );
  mConstraintDescription = constraints.constraintDescription();
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( layer, layer->fields().field( idx ).name() );
  mEditorWidgetType = setup.type();
  mEditorWidgetConfig = setup.config();
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
  newItem->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData.asQVariant() );
  parent->addChild( newItem );
  newItem->setExpanded( true );
  return newItem;
}

DnDTree::DnDTree( QgsVectorLayer *layer, QWidget *parent )
  : QTreeWidget( parent )
  , mLayer( layer )
{
  //connect( this, &QTreeWidget::itemDoubleClicked, this, &DnDTree::onItemDoubleClicked );
  //connect( this, &QTreeWidget::itemDoubleClicked, this, &DnDTree::attributeTypeDialog );
  //connect( this, &QTreeWidget::itemSelectionChanged, this, &DnDTree::attributeTypeDialog );
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
  newItem->setData( 0, QgsAttributesFormProperties::DnDTreeRole, data.asQVariant() );
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

      item->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData.asQVariant() );
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

      item->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData.asQVariant() );
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

      item->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData.asQVariant() );
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

