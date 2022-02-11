/***************************************************************************
    qgsrulebasedlabelingwidget.cpp
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrulebasedlabelingwidget.h"

#include "qgsapplication.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfeatureiterator.h"
#include "qgslabelinggui.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsrulebasedlabeling.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgslogger.h"
#include "qgsexpressioncontextutils.h"

#include <QAction>
#include <QClipboard>
#include <QMessageBox>

const double ICON_PADDING_FACTOR = 0.16;

static QList<QgsExpressionContextScope *> _globalProjectAtlasMapLayerScopes( QgsMapCanvas *mapCanvas, const QgsMapLayer *layer )
{
  QList<QgsExpressionContextScope *> scopes;
  scopes << QgsExpressionContextUtils::globalScope()
         << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
         << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( mapCanvas )
  {
    scopes << QgsExpressionContextUtils::mapSettingsScope( mapCanvas->mapSettings() )
           << new QgsExpressionContextScope( mapCanvas->expressionContextScope() );
  }
  else
  {
    scopes << QgsExpressionContextUtils::mapSettingsScope( QgsMapSettings() );
  }
  scopes << QgsExpressionContextUtils::layerScope( layer );
  return scopes;
}


QgsRuleBasedLabelingWidget::QgsRuleBasedLabelingWidget( QgsVectorLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsPanelWidget( parent )
  , mLayer( layer )
  , mCanvas( canvas )

{
  setupUi( this );

  btnAddRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnEditRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.svg" ) ) );
  btnRemoveRule->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

  mCopyAction = new QAction( tr( "Copy" ), this );
  mCopyAction->setShortcut( QKeySequence( QKeySequence::Copy ) );
  mPasteAction = new QAction( tr( "Paste" ), this );
  mPasteAction->setShortcut( QKeySequence( QKeySequence::Paste ) );
  mDeleteAction = new QAction( tr( "Remove Rule" ), this );
  mDeleteAction->setShortcut( QKeySequence( QKeySequence::Delete ) );

  viewRules->addAction( mCopyAction );
  viewRules->addAction( mPasteAction );
  viewRules->addAction( mDeleteAction );

  connect( viewRules, &QAbstractItemView::doubleClicked, this, static_cast<void ( QgsRuleBasedLabelingWidget::* )( const QModelIndex & )>( &QgsRuleBasedLabelingWidget::editRule ) );

  connect( btnAddRule, &QAbstractButton::clicked, this, &QgsRuleBasedLabelingWidget::addRule );
  connect( btnEditRule, &QAbstractButton::clicked, this, static_cast<void ( QgsRuleBasedLabelingWidget::* )()>( &QgsRuleBasedLabelingWidget::editRule ) );
  connect( btnRemoveRule, &QAbstractButton::clicked, this, &QgsRuleBasedLabelingWidget::removeRule );
  connect( mCopyAction, &QAction::triggered, this, &QgsRuleBasedLabelingWidget::copy );
  connect( mPasteAction, &QAction::triggered, this, &QgsRuleBasedLabelingWidget::paste );
  connect( mDeleteAction, &QAction::triggered, this, &QgsRuleBasedLabelingWidget::removeRule );

  if ( mLayer->labeling() && mLayer->labeling()->type() == QLatin1String( "rule-based" ) )
  {
    const QgsRuleBasedLabeling *rl = static_cast<const QgsRuleBasedLabeling *>( mLayer->labeling() );
    mRootRule = rl->rootRule()->clone();
  }
  else if ( mLayer->labeling() && mLayer->labeling()->type() == QLatin1String( "simple" ) )
  {
    // copy simple label settings to first rule
    mRootRule = new QgsRuleBasedLabeling::Rule( nullptr );
    std::unique_ptr< QgsPalLayerSettings > newSettings = std::make_unique< QgsPalLayerSettings >( mLayer->labeling()->settings() );
    newSettings->drawLabels = true; // otherwise we may be trying to copy a "blocking" setting to a rule - which is confusing for users!
    mRootRule->appendChild( new QgsRuleBasedLabeling::Rule( newSettings.release() ) );
  }
  else
  {
    mRootRule = new QgsRuleBasedLabeling::Rule( nullptr );
  }

  mModel = new QgsRuleBasedLabelingModel( mRootRule );
  viewRules->setModel( mModel );

  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsRuleBasedLabelingWidget::widgetChanged );
  connect( mModel, &QAbstractItemModel::rowsInserted, this, &QgsRuleBasedLabelingWidget::widgetChanged );
  connect( mModel, &QAbstractItemModel::rowsRemoved, this, &QgsRuleBasedLabelingWidget::widgetChanged );
}

QgsRuleBasedLabelingWidget::~QgsRuleBasedLabelingWidget()
{
  delete mRootRule;
}

void QgsRuleBasedLabelingWidget::setDockMode( bool dockMode )
{
  if ( dockMode )
  {
    // when in dock mode, these shortcuts conflict with the main window shortcuts and cannot be used
    if ( mCopyAction )
      mCopyAction->setShortcut( QKeySequence() );
    if ( mPasteAction )
      mPasteAction->setShortcut( QKeySequence() );
    if ( mDeleteAction )
      mDeleteAction->setShortcut( QKeySequence() );
  }
  QgsPanelWidget::setDockMode( dockMode );
}

void QgsRuleBasedLabelingWidget::addRule()
{
  QgsRuleBasedLabeling::Rule *newrule = new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( QgsAbstractVectorLayerLabeling::defaultSettingsForLayer( mLayer ) ) );

  QgsRuleBasedLabeling::Rule *current = currentRule();
  if ( current )
  {
    // add after this rule
    const QModelIndex currentIndex = viewRules->selectionModel()->currentIndex();
    mModel->insertRule( currentIndex.parent(), currentIndex.row() + 1, newrule );
    const QModelIndex newindex = mModel->index( currentIndex.row() + 1, 0, currentIndex.parent() );
    viewRules->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect );
  }
  else
  {
    // append to root rule
    const int rows = mModel->rowCount();
    mModel->insertRule( QModelIndex(), rows, newrule );
    const QModelIndex newindex = mModel->index( rows, 0 );
    viewRules->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect );
  }
  editRule();
}

void QgsRuleBasedLabelingWidget::ruleWidgetPanelAccepted( QgsPanelWidget *panel )
{
  QgsLabelingRulePropsWidget *widget = qobject_cast<QgsLabelingRulePropsWidget *>( panel );
  widget->apply();

  const QModelIndex index = viewRules->selectionModel()->currentIndex();
  mModel->updateRule( index.parent(), index.row() );
}

void QgsRuleBasedLabelingWidget::liveUpdateRuleFromPanel()
{
  ruleWidgetPanelAccepted( qobject_cast<QgsPanelWidget *>( sender() ) );
}


void QgsRuleBasedLabelingWidget::editRule()
{
  editRule( viewRules->selectionModel()->currentIndex() );
}

void QgsRuleBasedLabelingWidget::editRule( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  QgsRuleBasedLabeling::Rule *rule = mModel->ruleForIndex( index );
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );

  if ( panel && panel->dockMode() )
  {
    QgsLabelingRulePropsWidget *widget = new QgsLabelingRulePropsWidget( rule, mLayer, this, mCanvas );
    widget->setPanelTitle( tr( "Edit Rule" ) );
    connect( widget, &QgsPanelWidget::panelAccepted, this, &QgsRuleBasedLabelingWidget::ruleWidgetPanelAccepted );
    connect( widget, &QgsLabelingRulePropsWidget::widgetChanged, this, &QgsRuleBasedLabelingWidget::liveUpdateRuleFromPanel );
    openPanel( widget );
    return;
  }

  QgsLabelingRulePropsDialog dlg( rule, mLayer, this, mCanvas );
  if ( dlg.exec() )
  {
    mModel->updateRule( index.parent(), index.row() );
    emit widgetChanged();
  }
}

void QgsRuleBasedLabelingWidget::removeRule()
{
  const QItemSelection sel = viewRules->selectionModel()->selection();
  const auto constSel = sel;
  for ( const QItemSelectionRange &range : constSel )
  {
    if ( range.isValid() )
      mModel->removeRows( range.top(), range.bottom() - range.top() + 1, range.parent() );
  }
  // make sure that the selection is gone
  viewRules->selectionModel()->clear();
}

void QgsRuleBasedLabelingWidget::copy()
{
  const QModelIndexList indexlist = viewRules->selectionModel()->selectedRows();

  if ( indexlist.isEmpty() )
    return;

  QMimeData *mime = mModel->mimeData( indexlist );
  QApplication::clipboard()->setMimeData( mime );
}

void QgsRuleBasedLabelingWidget::paste()
{
  const QMimeData *mime = QApplication::clipboard()->mimeData();
  QModelIndexList indexlist = viewRules->selectionModel()->selectedRows();
  QModelIndex index;
  if ( indexlist.isEmpty() )
    index = mModel->index( mModel->rowCount(), 0 );
  else
    index = indexlist.first();
  mModel->dropMimeData( mime, Qt::CopyAction, index.row(), index.column(), index.parent() );
}

QgsRuleBasedLabeling::Rule *QgsRuleBasedLabelingWidget::currentRule()
{
  QItemSelectionModel *sel = viewRules->selectionModel();
  const QModelIndex idx = sel->currentIndex();
  if ( !idx.isValid() )
    return nullptr;
  return mModel->ruleForIndex( idx );
}

#include "qgsvscrollarea.h"

QgsLabelingRulePropsDialog::QgsLabelingRulePropsDialog( QgsRuleBasedLabeling::Rule *rule, QgsVectorLayer *layer, QWidget *parent, QgsMapCanvas *mapCanvas )
  : QDialog( parent )
{

#ifdef Q_OS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  QVBoxLayout *layout = new QVBoxLayout( this );
  QgsVScrollArea *scrollArea = new QgsVScrollArea( this );
  scrollArea->setFrameShape( QFrame::NoFrame );
  layout->addWidget( scrollArea );

  buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok );
  mPropsWidget = new QgsLabelingRulePropsWidget( rule, layer, this, mapCanvas );

  scrollArea->setWidget( mPropsWidget );
  layout->addWidget( buttonBox );
  this->setWindowTitle( "Edit Rule" );
  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsLabelingRulePropsDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsLabelingRulePropsDialog::showHelp );
}

void QgsLabelingRulePropsDialog::testFilter()
{
  mPropsWidget->testFilter();
}

void QgsLabelingRulePropsDialog::buildExpression()
{
  mPropsWidget->buildExpression();
}

void QgsLabelingRulePropsDialog::accept()
{
  mPropsWidget->apply();
  QDialog::accept();
}

void QgsLabelingRulePropsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#rule-based-labeling" ) );
}

////

QgsRuleBasedLabelingModel::QgsRuleBasedLabelingModel( QgsRuleBasedLabeling::Rule *rootRule, QObject *parent )
  : QAbstractItemModel( parent )
  , mRootRule( rootRule )
{
}

Qt::ItemFlags QgsRuleBasedLabelingModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemIsDropEnabled;

  // allow drop only at first column
  const Qt::ItemFlag drop = ( index.column() == 0 ? Qt::ItemIsDropEnabled : Qt::NoItemFlags );

  const Qt::ItemFlag checkable = ( index.column() == 0 ? Qt::ItemIsUserCheckable : Qt::NoItemFlags );

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
         Qt::ItemIsEditable | checkable |
         Qt::ItemIsDragEnabled | drop;
}

QVariant QgsRuleBasedLabelingModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsRuleBasedLabeling::Rule *rule = ruleForIndex( index );

  if ( role == Qt::DisplayRole || role == Qt::ToolTipRole )
  {
    switch ( index.column() )
    {
      case 0:
        return rule->description();
      case 1:
        if ( rule->isElse() )
        {
          return "ELSE";
        }
        else
        {
          return rule->filterExpression().isEmpty() ? tr( "(no filter)" ) : rule->filterExpression();
        }
      case 2:
        return rule->dependsOnScale() ? QgsScaleComboBox::toString( rule->minimumScale() ) : QVariant();
      case 3:
        return rule->dependsOnScale() ? QgsScaleComboBox::toString( rule->maximumScale() ) : QVariant();
      case 4:
        return rule->settings() ? rule->settings()->fieldName : QVariant();
      default:
        return QVariant();
    }
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 && rule->settings() )
  {
    const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
    return QgsPalLayerSettings::labelSettingsPreviewPixmap( *rule->settings(), QSize( iconSize, iconSize ), QString(),  static_cast< int >( iconSize * ICON_PADDING_FACTOR ) );
  }
  else if ( role == Qt::TextAlignmentRole )
  {
    return ( index.column() == 2 || index.column() == 3 ) ? static_cast<Qt::Alignment::Int>( Qt::AlignRight ) : static_cast<Qt::Alignment::Int>( Qt::AlignLeft );
  }
  else if ( role == Qt::FontRole && index.column() == 1 )
  {
    if ( rule->isElse() )
    {
      QFont italicFont;
      italicFont.setItalic( true );
      return italicFont;
    }
    return QVariant();
  }
  else if ( role == Qt::EditRole )
  {
    switch ( index.column() )
    {
      case 0:
        return rule->description();
      case 1:
        return rule->filterExpression();
      case 2:
        return rule->minimumScale();
      case 3:
        return rule->maximumScale();
      case 4:
        return rule->settings() ? rule->settings()->fieldName : QVariant();
      default:
        return QVariant();
    }
  }
  else if ( role == Qt::CheckStateRole )
  {
    if ( index.column() != 0 )
      return QVariant();
    return rule->active() ? Qt::Checked : Qt::Unchecked;
  }
  else
    return QVariant();
}

QVariant QgsRuleBasedLabelingModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < 5 )
  {
    QStringList lst;
    lst << tr( "Label" ) << tr( "Rule" ) << tr( "Min. Scale" ) << tr( "Max. Scale" ) << tr( "Text" ); // << tr( "Count" ) << tr( "Duplicate Count" );
    return lst[section];
  }

  return QVariant();
}

int QgsRuleBasedLabelingModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.column() > 0 )
    return 0;

  QgsRuleBasedLabeling::Rule *parentRule = ruleForIndex( parent );

  return parentRule->children().count();
}

int QgsRuleBasedLabelingModel::columnCount( const QModelIndex & ) const
{
  return 5;
}

QModelIndex QgsRuleBasedLabelingModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    QgsRuleBasedLabeling::Rule *parentRule = ruleForIndex( parent );
    QgsRuleBasedLabeling::Rule *childRule = parentRule->children()[row];
    return createIndex( row, column, childRule );
  }
  return QModelIndex();
}

QModelIndex QgsRuleBasedLabelingModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsRuleBasedLabeling::Rule *childRule = ruleForIndex( index );
  QgsRuleBasedLabeling::Rule *parentRule = childRule->parent();

  if ( parentRule == mRootRule )
    return QModelIndex();

  // this is right: we need to know row number of our parent (in our grandparent)
  const int row = parentRule->parent()->children().indexOf( parentRule );

  return createIndex( row, 0, parentRule );
}

bool QgsRuleBasedLabelingModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsRuleBasedLabeling::Rule *rule = ruleForIndex( index );

  if ( role == Qt::CheckStateRole )
  {
    rule->setActive( value.toInt() == Qt::Checked );
    emit dataChanged( index, index );
    return true;
  }

  if ( role != Qt::EditRole )
    return false;

  switch ( index.column() )
  {
    case 0: // description
      rule->setDescription( value.toString() );
      break;
    case 1: // filter
      rule->setFilterExpression( value.toString() );
      break;
    case 2: // scale min
      rule->setMinimumScale( value.toDouble() );
      break;
    case 3: // scale max
      rule->setMaximumScale( value.toDouble() );
      break;
    case 4: // label text
      if ( !rule->settings() )
        return false;
      rule->settings()->fieldName = value.toString();
      break;
    default:
      return false;
  }

  emit dataChanged( index, index );
  return true;
}

Qt::DropActions QgsRuleBasedLabelingModel::supportedDropActions() const
{
  return Qt::MoveAction; // | Qt::CopyAction
}

QStringList QgsRuleBasedLabelingModel::mimeTypes() const
{
  QStringList types;
  types << QStringLiteral( "application/vnd.text.list" );
  return types;
}

// manipulate DOM before dropping it so that rules are more useful
void _renderer2labelingRules( QDomElement &ruleElem )
{
  // labeling rules recognize only "description"
  if ( ruleElem.hasAttribute( QStringLiteral( "label" ) ) )
    ruleElem.setAttribute( QStringLiteral( "description" ), ruleElem.attribute( QStringLiteral( "label" ) ) );

  // run recursively
  QDomElement childRuleElem = ruleElem.firstChildElement( QStringLiteral( "rule" ) );
  while ( !childRuleElem.isNull() )
  {
    _renderer2labelingRules( childRuleElem );
    childRuleElem = childRuleElem.nextSiblingElement( QStringLiteral( "rule" ) );
  }
}

QMimeData *QgsRuleBasedLabelingModel::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );

  const auto constIndexes = indexes;
  for ( const QModelIndex &index : constIndexes )
  {
    // each item consists of several columns - let's add it with just first one
    if ( !index.isValid() || index.column() != 0 )
      continue;

    // we use a clone of the existing rule because it has a new unique rule key
    // non-unique rule keys would confuse other components using them (e.g. legend)
    QgsRuleBasedLabeling::Rule *rule = ruleForIndex( index )->clone();
    QDomDocument doc;

    QDomElement rootElem = doc.createElement( QStringLiteral( "rule_mime" ) );
    rootElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "labeling" ) ); // for determining whether rules are from renderer or labeling
    const QDomElement rulesElem = rule->save( doc, QgsReadWriteContext() );
    rootElem.appendChild( rulesElem );
    doc.appendChild( rootElem );

    delete rule;

    stream << doc.toString( -1 );
  }

  mimeData->setData( QStringLiteral( "application/vnd.text.list" ), encodedData );
  return mimeData;
}

bool QgsRuleBasedLabelingModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( column )

  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( QStringLiteral( "application/vnd.text.list" ) ) )
    return false;

  if ( parent.column() > 0 )
    return false;

  QByteArray encodedData = data->data( QStringLiteral( "application/vnd.text.list" ) );
  QDataStream stream( &encodedData, QIODevice::ReadOnly );
  int rows = 0;

  if ( row == -1 )
  {
    // the item was dropped at a parent - we may decide where to put the items - let's append them
    row = rowCount( parent );
  }

  while ( !stream.atEnd() )
  {
    QString text;
    stream >> text;

    QDomDocument doc;
    if ( !doc.setContent( text ) )
      continue;
    const QDomElement rootElem = doc.documentElement();
    if ( rootElem.tagName() != QLatin1String( "rule_mime" ) )
      continue;
    QDomElement ruleElem = rootElem.firstChildElement( QStringLiteral( "rule" ) );
    if ( rootElem.attribute( QStringLiteral( "type" ) ) == QLatin1String( "renderer" ) )
      _renderer2labelingRules( ruleElem ); // do some modifications so that we load the rules more nicely
    QgsRuleBasedLabeling::Rule *rule = QgsRuleBasedLabeling::Rule::create( ruleElem, QgsReadWriteContext() );

    insertRule( parent, row + rows, rule );

    ++rows;
  }
  return true;
}

bool QgsRuleBasedLabelingModel::removeRows( int row, int count, const QModelIndex &parent )
{
  QgsRuleBasedLabeling::Rule *parentRule = ruleForIndex( parent );

  if ( row < 0 || row >= parentRule->children().count() )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );

  for ( int i = 0; i < count; i++ )
  {
    if ( row < parentRule->children().count() )
    {
      parentRule->removeChildAt( row );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "trying to remove invalid index - this should not happen!" ) );
    }
  }

  endRemoveRows();

  return true;
}

QgsRuleBasedLabeling::Rule *QgsRuleBasedLabelingModel::ruleForIndex( const QModelIndex &index ) const
{
  if ( index.isValid() )
    return static_cast<QgsRuleBasedLabeling::Rule *>( index.internalPointer() );
  return mRootRule;
}

void QgsRuleBasedLabelingModel::insertRule( const QModelIndex &parent, int before, QgsRuleBasedLabeling::Rule *newrule )
{
  beginInsertRows( parent, before, before );

  QgsRuleBasedLabeling::Rule *parentRule = ruleForIndex( parent );
  parentRule->insertChild( before, newrule );

  endInsertRows();
}

void QgsRuleBasedLabelingModel::updateRule( const QModelIndex &parent, int row )
{
  emit dataChanged( index( row, 0, parent ),
                    index( row, columnCount( parent ), parent ) );
}

/////////

QgsLabelingRulePropsWidget::QgsLabelingRulePropsWidget( QgsRuleBasedLabeling::Rule *rule, QgsVectorLayer *layer, QWidget *parent, QgsMapCanvas *mapCanvas )
  : QgsPanelWidget( parent )
  , mRule( rule )
  , mLayer( layer )
  , mSettings( nullptr )
  , mMapCanvas( mapCanvas )
{
  setupUi( this );

  QButtonGroup *radioGroup = new QButtonGroup( this );
  radioGroup->addButton( mFilterRadio );
  radioGroup->addButton( mElseRadio );

  mElseRadio->setChecked( mRule->isElse() );
  mFilterRadio->setChecked( !mRule->isElse() );
  editFilter->setText( mRule->filterExpression() );
  editFilter->setToolTip( mRule->filterExpression() );
  editDescription->setText( mRule->description() );
  editDescription->setToolTip( mRule->description() );

  if ( mRule->dependsOnScale() )
  {
    groupScale->setChecked( true );
    // caution: rule uses scale denom, scale widget uses true scales
    mScaleRangeWidget->setScaleRange( std::max( rule->minimumScale(), 0.0 ),
                                      std::max( rule->maximumScale(), 0.0 ) );
  }
  mScaleRangeWidget->setMapCanvas( mMapCanvas );

  if ( mRule->settings() )
  {
    groupSettings->setChecked( true );
    mSettings = new QgsPalLayerSettings( *mRule->settings() ); // use a clone!
  }
  else
  {
    groupSettings->setChecked( false );
    mSettings = new QgsPalLayerSettings;
  }

  mLabelingGui = new QgsLabelingGui( nullptr, mMapCanvas, *mSettings, this );
  mLabelingGui->layout()->setContentsMargins( 0, 0, 0, 0 );
  QVBoxLayout *l = new QVBoxLayout;
  l->addWidget( mLabelingGui );
  groupSettings->setLayout( l );

  mLabelingGui->setLabelMode( QgsLabelingGui::Labels );
  mLabelingGui->setLayer( mLayer );

  connect( btnExpressionBuilder, &QAbstractButton::clicked, this, &QgsLabelingRulePropsWidget::buildExpression );
  connect( btnTestFilter, &QAbstractButton::clicked, this, &QgsLabelingRulePropsWidget::testFilter );
  connect( editFilter, &QLineEdit::textEdited, this, &QgsLabelingRulePropsWidget::widgetChanged );
  connect( editDescription, &QLineEdit::textChanged, this, &QgsLabelingRulePropsWidget::widgetChanged );
  connect( groupScale, &QGroupBox::toggled, this, &QgsLabelingRulePropsWidget::widgetChanged );
  connect( mScaleRangeWidget, &QgsScaleRangeWidget::rangeChanged, this, &QgsLabelingRulePropsWidget::widgetChanged );
  connect( groupSettings, &QGroupBox::toggled, this, &QgsLabelingRulePropsWidget::widgetChanged );
  connect( mLabelingGui, &QgsTextFormatWidget::widgetChanged, this, &QgsLabelingRulePropsWidget::widgetChanged );
  connect( mFilterRadio, &QRadioButton::toggled, this, [ = ]( bool toggled ) { filterFrame->setEnabled( toggled ) ; } );
  connect( mElseRadio, &QRadioButton::toggled, this, [ = ]( bool toggled ) { if ( toggled ) editFilter->setText( QStringLiteral( "ELSE" ) );} );
}

QgsLabelingRulePropsWidget::~QgsLabelingRulePropsWidget()
{
  delete mSettings;
}

void QgsLabelingRulePropsWidget::setDockMode( bool dockMode )
{
  QgsPanelWidget::setDockMode( dockMode );
  mLabelingGui->setDockMode( dockMode );
}

void QgsLabelingRulePropsWidget::testFilter()
{
  if ( !mFilterRadio->isChecked() )
    return;

  QgsExpression filter( editFilter->text() );
  if ( filter.hasParserError() )
  {
    QMessageBox::critical( this, tr( "Test Filter" ),  tr( "Filter expression parsing error:\n" ) + filter.parserErrorString() );
    return;
  }

  QgsExpressionContext context( _globalProjectAtlasMapLayerScopes( mMapCanvas, mLayer ) );

  if ( !filter.prepare( &context ) )
  {
    QMessageBox::critical( this, tr( "Test Filter" ), filter.evalErrorString() );
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsFeatureIterator fit = mLayer->getFeatures();

  int count = 0;
  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    context.setFeature( f );

    const QVariant value = filter.evaluate( &context );
    if ( value.toInt() != 0 )
      count++;
    if ( filter.hasEvalError() )
      break;
  }

  QApplication::restoreOverrideCursor();

  QMessageBox::information( this, tr( "Test Filter" ), tr( "Filter returned %n feature(s)", "number of filtered features", count ) );
}


void QgsLabelingRulePropsWidget::buildExpression()
{
  const QgsExpressionContext context( _globalProjectAtlasMapLayerScopes( mMapCanvas, mLayer ) );

  QgsExpressionBuilderDialog dlg( mLayer, editFilter->text(), this, QStringLiteral( "generic" ), context );

  if ( dlg.exec() )
    editFilter->setText( dlg.expressionText() );
}

void QgsLabelingRulePropsWidget::apply()
{
  const QString filter = mElseRadio->isChecked() ? QStringLiteral( "ELSE" ) : editFilter->text();
  mRule->setFilterExpression( filter );
  mRule->setDescription( editDescription->text() );
  mRule->setMinimumScale( groupScale->isChecked() ? mScaleRangeWidget->minimumScale() : 0 );
  mRule->setMaximumScale( groupScale->isChecked() ? mScaleRangeWidget->maximumScale() : 0 );
  mRule->setSettings( groupSettings->isChecked() ? new QgsPalLayerSettings( mLabelingGui->layerSettings() ) : nullptr );
}
