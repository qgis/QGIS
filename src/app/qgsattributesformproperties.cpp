#include "qgsattributesformproperties.h"
#include "qgsattributetypedialog.h"

QgsAttributesFormProperties::QgsAttributesFormProperties( QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
  , mDragTree( nullptr )
  , mDropTree( nullptr )
{
  if ( !layer )
    return;

  setupUi(this);

  // drag tree (not mixup with drag queen)
  QGridLayout *mDragListWidgetLayout = new QGridLayout;
  mDragTree = new DnDTree( mLayer, mDragListWidget );
  mDragListWidgetLayout->addWidget( mDragTree );
  mDragListWidget->setLayout(mDragListWidgetLayout);
  mDragTree->setHeaderLabels( QStringList() << tr( "Label" ) );
  mDragTree->type = DnDTree::Type::Drag;

  // drop tree
  QGridLayout *mDropListWidgetLayout = new QGridLayout;
  mDropTree = new DnDTree( mLayer, mDropListWidget );
  mDropListWidgetLayout->addWidget( mDropTree );
  mDropListWidget->setLayout(mDropListWidgetLayout);
  mDropTree->setHeaderLabels( QStringList() << tr( "Label" ) );
  mDropTree->type = DnDTree::Type::Drop;
  
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
  loadAttributeEditorTree(mDragTree);
  loadAttributeEditorTree(mDropTree);
}

//Slots
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


QTreeWidgetItem *QgsAttributesFormProperties::loadAttributeEditorTreeItem( QgsAttributeEditorElement *const widgetDef, QTreeWidgetItem *parent, DnDTree* mTree)
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

void QgsAttributesFormProperties::loadAttributeEditorTree(DnDTree* mTree)
{

  if( mTree->type==DnDTree::Type::Drop ){
    // tabs and groups info
    mTree->clear();
    mTree->setSortingEnabled( false );
    mTree->setSelectionBehavior( QAbstractItemView::SelectRows );
    mTree->setDragDropMode( QAbstractItemView::InternalMove );
    mTree->setAcceptDrops( true );
    mTree->setDragDropMode( QAbstractItemView::DragDrop );

    Q_FOREACH ( QgsAttributeEditorElement *wdg, mLayer->editFormConfig().tabs() )
    {
      loadAttributeEditorTreeItem( wdg, mTree->invisibleRootItem(), mTree);
    }

  }else{
    mTree->clear();
    mTree->setSortingEnabled( false );
    mTree->setSelectionBehavior( QAbstractItemView::SelectRows );
    mTree->setDragDropMode( QAbstractItemView::InternalMove );
    mTree->setAcceptDrops( false );
    mTree->setDragDropMode( QAbstractItemView::DragOnly );

    Q_FOREACH ( QgsAttributeEditorElement *wdg, mLayer->editFormConfig().tabs() )
    {
      loadAttributeEditorTreeItem( wdg, mTree->invisibleRootItem(), mTree);
    }

#if 0
    const QgsFields &fields = mLayer->fields();

    for ( int i = 0; i < fields.count(); ++i ){
      QTreeWidgetItem* item = new QTreeWidgetItem;
      item->setIcon(0, mLayer->fields().iconForField( i ));
      item->setText(0, QString::number( i+1 ) );
      item->setText(0, fields.at( i ).name() );
      mTree->addTopLevelItem(item);
    }
#endif
  }
}


void QgsAttributesFormProperties::onAttributeSelectionChanged()
{
  bool isAddPossible = false;
  if ( mDropTree->selectedItems().count() == 1 && !mDragTree->selectedItems().isEmpty() )
    if ( mDropTree->selectedItems()[0]->data( 0, DnDTreeRole ).value<DnDTreeItemData>().type() == DnDTreeItemData::Container )
      isAddPossible = true;
  //d mAddItemButton->setEnabled( isAddPossible );

  //updateButtons();
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
  connect( this, &QTreeWidget::itemDoubleClicked, this, &DnDTree::onItemDoubleClicked );
  connect( this, &QTreeWidget::itemSelectionChanged, this, &DnDTree::attributeTypeDialog );
}

QTreeWidgetItem *DnDTree::addItem( QTreeWidgetItem *parent, QgsAttributesFormProperties::DnDTreeItemData data )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( QStringList() << data.name() );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
  if ( data.type() == QgsAttributesFormProperties::DnDTreeItemData::Container )
  {
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
    newItem->setBackground( 0, QBrush( Qt::lightGray ) );

#if 0
    switch ( data.type() )
    {
      case DnDTreeItemData::Field:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( "/mFieldIcon.svg" ) );
        break;

      case DnDTreeItemData::Relation:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( "/mRelationIcon.svg" ) );
        break;

      case DnDTreeItemData::Container:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( "/mContainerIcon.svg" ) );
        break;
    }
#endif
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


void DnDTree::attributeTypeDialog()
{

  //mOptionsWidget
#if 0
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  FieldConfig cfg;
  int index = -1;
  int row = -1;

  Q_FOREACH ( QTableWidgetItem *wdg, mIndexedWidgets )
  {
    cfg = wdg->data( FieldConfigRole ).value<FieldConfig>();
    if ( cfg.mButton == pb )
    {
      index = mIndexedWidgets.indexOf( wdg );
      row = wdg->row();
      break;
    }
  }

  if ( index == -1 )
    return;

  QgsAttributeTypeDialog attributeTypeDialog( mLayer, index );

  attributeTypeDialog.setFieldEditable( cfg.mEditable );
  attributeTypeDialog.setLabelOnTop( cfg.mLabelOnTop );
  attributeTypeDialog.setNotNull( cfg.mConstraints & QgsFieldConstraints::ConstraintNotNull );
  attributeTypeDialog.setNotNullEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );
  attributeTypeDialog.setUnique( cfg.mConstraints & QgsFieldConstraints::ConstraintUnique );
  attributeTypeDialog.setUniqueEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );

  QgsFieldConstraints constraints = mLayer->fields().at( index ).constraints();
  QgsFieldConstraints::Constraints providerConstraints = 0;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintNotNull;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintUnique;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintExpression;
  attributeTypeDialog.setProviderConstraints( providerConstraints );

  attributeTypeDialog.setConstraintExpression( cfg.mConstraint );
  attributeTypeDialog.setConstraintExpressionDescription( cfg.mConstraintDescription );
  attributeTypeDialog.setConstraintExpressionEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );
  attributeTypeDialog.setDefaultValueExpression( mLayer->defaultValueExpression( index ) );

  attributeTypeDialog.setEditorWidgetConfig( cfg.mEditorWidgetConfig );
  attributeTypeDialog.setEditorWidgetType( cfg.mEditorWidgetType );

  if ( !attributeTypeDialog.exec() )
    return;

  cfg.mEditable = attributeTypeDialog.fieldEditable();
  cfg.mLabelOnTop = attributeTypeDialog.labelOnTop();

  cfg.mConstraints = 0;
  if ( attributeTypeDialog.notNull() && !( providerConstraints & QgsFieldConstraints::ConstraintNotNull ) )
  {
    cfg.mConstraints |= QgsFieldConstraints::ConstraintNotNull;
  }
  if ( attributeTypeDialog.unique() && !( providerConstraints & QgsFieldConstraints::ConstraintUnique ) )
  {
    cfg.mConstraints |= QgsFieldConstraints::ConstraintUnique;
  }
  if ( !attributeTypeDialog.constraintExpression().isEmpty() && !( providerConstraints & QgsFieldConstraints::ConstraintExpression ) )
  {
    cfg.mConstraints |= QgsFieldConstraints::ConstraintExpression;
  }

  cfg.mConstraintDescription = attributeTypeDialog.constraintExpressionDescription();
  cfg.mConstraint = attributeTypeDialog.constraintExpression();
  mLayer->setDefaultValueExpression( index, attributeTypeDialog.defaultValueExpression() );

  cfg.mEditorWidgetType = attributeTypeDialog.editorWidgetType();
  cfg.mEditorWidgetConfig = attributeTypeDialog.editorWidgetConfig();

  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintNotNull, attributeTypeDialog.notNullEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintUnique, attributeTypeDialog.uniqueEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintExpression, attributeTypeDialog.constraintExpressionEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );

  pb->setText( attributeTypeDialog.editorWidgetText() );

  setConfigForRow( row, cfg );

#endif
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

