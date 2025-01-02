/***************************************************************************
    qgslabelingengineruleswidget.cpp
    ------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelingengineruleswidget.h"
#include "moc_qgslabelingengineruleswidget.cpp"
#include "qgsapplication.h"
#include "qgslabelingengineruleregistry.h"
#include "qgslabelingenginerule.h"
#include "qgslabelingenginerulewidget.h"
#include "qgsgui.h"
#include "qgshelp.h"

#include <QMenu>
#include <QAction>
#include <QDialogButtonBox>
#include <QPushButton>

//
// QgsLabelingEngineRulesModel
//

QgsLabelingEngineRulesModel::QgsLabelingEngineRulesModel( QObject *parent )
  : QAbstractItemModel( parent )
{
}

Qt::ItemFlags QgsLabelingEngineRulesModel::flags( const QModelIndex &index ) const
{
  const QgsAbstractLabelingEngineRule *rule = ruleAtIndex( index );
  if ( !rule )
    return Qt::ItemFlags();

  Qt::ItemFlags res = Qt::ItemIsSelectable;
  if ( rule->isAvailable() )
  {
    res |= Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

  if ( index.column() == 0 )
  {
    res |= Qt::ItemIsUserCheckable;
  }
  return res;
}

QgsLabelingEngineRulesModel::~QgsLabelingEngineRulesModel() = default;

QModelIndex QgsLabelingEngineRulesModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsLabelingEngineRulesModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return static_cast<int>( mRules.size() );
}

int QgsLabelingEngineRulesModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsLabelingEngineRulesModel::data( const QModelIndex &index, int role ) const
{
  const QgsAbstractLabelingEngineRule *rule = ruleAtIndex( index );
  if ( !rule )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      return rule->name().isEmpty() ? rule->displayType() : rule->name();
    }

    case Qt::ToolTipRole:
    {
      if ( !rule->isAvailable() )
        return tr( "This rule is not available for use on this system." );

      return rule->description();
    }

    case Qt::CheckStateRole:
    {
      if ( index.column() != 0 )
        return QVariant();
      return rule->active() ? Qt::Checked : Qt::Unchecked;
    }

    default:
      break;
  }

  return QVariant();
}

QModelIndex QgsLabelingEngineRulesModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

bool QgsLabelingEngineRulesModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 || row >= static_cast<int>( mRules.size() ) )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );
  for ( int i = 0; i < count; i++ )
  {
    if ( row < static_cast<int>( mRules.size() ) )
    {
      mRules.erase( mRules.begin() + row );
    }
  }
  endRemoveRows();
  return true;
}

bool QgsLabelingEngineRulesModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsAbstractLabelingEngineRule *rule = ruleAtIndex( index );
  if ( !rule )
    return false;

  switch ( role )
  {
    case Qt::CheckStateRole:
    {
      rule->setActive( value.toInt() == Qt::Checked );
      emit dataChanged( index, index, { role } );
      return true;
    }

    case Qt::EditRole:
    {
      if ( index.column() == 0 )
      {
        rule->setName( value.toString() );
        emit dataChanged( index, index );
        return true;
      }
      break;
    }

    default:
      break;
  }

  return false;
}

void QgsLabelingEngineRulesModel::setRules( const QList<QgsAbstractLabelingEngineRule *> &rules )
{
  beginResetModel();
  mRules.clear();
  for ( const QgsAbstractLabelingEngineRule *rule : rules )
  {
    mRules.emplace_back( rule->clone() );
  }
  endResetModel();
}

void QgsLabelingEngineRulesModel::addRule( std::unique_ptr<QgsAbstractLabelingEngineRule> &rule )
{
  beginInsertRows( QModelIndex(), static_cast<int>( mRules.size() ), static_cast<int>( mRules.size() ) );
  mRules.emplace_back( std::move( rule ) );
  endInsertRows();
}

QgsAbstractLabelingEngineRule *QgsLabelingEngineRulesModel::ruleAtIndex( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return nullptr;

  if ( index.row() < 0 || index.row() >= static_cast<int>( mRules.size() ) )
    return nullptr;

  return mRules[index.row()].get();
}

void QgsLabelingEngineRulesModel::changeRule( const QModelIndex &index, std::unique_ptr<QgsAbstractLabelingEngineRule> &rule )
{
  if ( !index.isValid() )
    return;

  if ( index.row() < 0 || index.row() >= static_cast<int>( mRules.size() ) )
    return;

  mRules[index.row()] = std::move( rule );
  emit dataChanged( index, index );
}

QList<QgsAbstractLabelingEngineRule *> QgsLabelingEngineRulesModel::rules() const
{
  QList<QgsAbstractLabelingEngineRule *> res;
  res.reserve( static_cast<int>( mRules.size() ) );
  for ( auto &it : mRules )
  {
    res.append( it->clone() );
  }
  return res;
}


//
// QgsLabelingEngineRulesWidget
//

QgsLabelingEngineRulesWidget::QgsLabelingEngineRulesWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Labeling Rules" ) );

  mModel = new QgsLabelingEngineRulesModel( this );
  connect( mModel, &QAbstractItemModel::dataChanged, this, &QgsLabelingEngineRulesWidget::changed );
  viewRules->setModel( mModel );
  viewRules->setHeaderHidden( true );

  mAddRuleMenu = new QMenu( this );
  connect( mAddRuleMenu, &QMenu::aboutToShow, this, &QgsLabelingEngineRulesWidget::createTypesMenu );

  btnAddRule->setMenu( mAddRuleMenu );
  btnAddRule->setPopupMode( QToolButton::InstantPopup );

  connect( btnEditRule, &QToolButton::clicked, this, &QgsLabelingEngineRulesWidget::editSelectedRule );
  connect( btnRemoveRule, &QToolButton::clicked, this, &QgsLabelingEngineRulesWidget::removeRules );

  connect( viewRules, &QAbstractItemView::doubleClicked, this, &QgsLabelingEngineRulesWidget::editRule );
}

void QgsLabelingEngineRulesWidget::setRules( const QList<QgsAbstractLabelingEngineRule *> &rules )
{
  mModel->setRules( rules );
}

QList<QgsAbstractLabelingEngineRule *> QgsLabelingEngineRulesWidget::rules() const
{
  return mModel->rules();
}

void QgsLabelingEngineRulesWidget::createTypesMenu()
{
  mAddRuleMenu->clear();

  const QStringList ruleIds = QgsApplication::labelingEngineRuleRegistry()->ruleIds();
  QList<QAction *> actions;
  for ( const QString &id : ruleIds )
  {
    if ( !QgsApplication::labelingEngineRuleRegistry()->isAvailable( id ) )
      continue;

    QAction *action = new QAction( QgsApplication::labelingEngineRuleRegistry()->create( id )->displayType() );
    connect( action, &QAction::triggered, this, [this, id] {
      createRule( id );
    } );
    actions << action;
  }
  std::sort( actions.begin(), actions.end(), []( const QAction *a, const QAction *b ) -> bool {
    return QString::localeAwareCompare( a->text(), b->text() ) < 0;
  } );
  mAddRuleMenu->addActions( actions );
}

void QgsLabelingEngineRulesWidget::createRule( const QString &id )
{
  std::unique_ptr<QgsAbstractLabelingEngineRule> rule( QgsApplication::labelingEngineRuleRegistry()->create( id ) );
  if ( rule )
  {
    rule->setName( rule->displayType() );
    mModel->addRule( rule );
    const QModelIndex newRuleIndex = mModel->index( mModel->rowCount() - 1, 0, QModelIndex() );
    viewRules->selectionModel()->setCurrentIndex( newRuleIndex, QItemSelectionModel::SelectionFlag::ClearAndSelect );
    editRule( newRuleIndex );
  }
}

void QgsLabelingEngineRulesWidget::editSelectedRule()
{
  const QItemSelection selection = viewRules->selectionModel()->selection();
  for ( const QItemSelectionRange &range : selection )
  {
    if ( range.isValid() )
    {
      const QModelIndex index = range.indexes().value( 0 );
      editRule( index );
      return;
    }
  }
}

void QgsLabelingEngineRulesWidget::editRule( const QModelIndex &index )
{
  const QgsAbstractLabelingEngineRule *rule = mModel->ruleAtIndex( index );
  if ( !rule || !rule->isAvailable() )
    return;

  // TODO -- move to a registry when there's a need
  QgsLabelingEngineRuleWidget *widget = nullptr;
  if ( rule->id() == "minimumDistanceLabelToFeature" )
  {
    widget = new QgsLabelingEngineRuleMinimumDistanceLabelToFeatureWidget();
  }
  else if ( rule->id() == "minimumDistanceLabelToLabel" )
  {
    widget = new QgsLabelingEngineRuleMinimumDistanceLabelToLabelWidget();
  }
  else if ( rule->id() == "maximumDistanceLabelToFeature" )
  {
    widget = new QgsLabelingEngineRuleMaximumDistanceLabelToFeatureWidget();
  }
  else if ( rule->id() == "avoidLabelOverlapWithFeature" )
  {
    widget = new QgsLabelingEngineRuleAvoidLabelOverlapWithFeatureWidget();
  }

  if ( !widget )
    return;

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    widget->setPanelTitle( rule->name().isEmpty() ? tr( "Configure Rule" ) : rule->name() );
    widget->setRule( rule );
    connect( widget, &QgsLabelingEngineRuleWidget::changed, this, [=] {
      std::unique_ptr<QgsAbstractLabelingEngineRule> updatedRule( widget->rule() );
      mModel->changeRule( index, updatedRule );
      emit changed();
    } );
    panel->openPanel( widget );
  }
  else
  {
    QgsLabelingEngineRuleDialog dialog( widget, this );
    dialog.setRule( rule );
    if ( dialog.exec() )
    {
      std::unique_ptr<QgsAbstractLabelingEngineRule> updatedRule( dialog.rule() );
      mModel->changeRule( index, updatedRule );
      emit changed();
    }
  }
}

void QgsLabelingEngineRulesWidget::removeRules()
{
  const QItemSelection selection = viewRules->selectionModel()->selection();
  QList<int> rows;
  for ( const QItemSelectionRange &range : selection )
  {
    if ( range.isValid() )
    {
      for ( int row = range.top(); row <= range.bottom(); ++row )
      {
        if ( !rows.contains( row ) )
          rows << row;
      }
    }
  }

  std::sort( rows.begin(), rows.end() );
  std::reverse( rows.begin(), rows.end() );
  for ( int row : std::as_const( rows ) )
  {
    mModel->removeRow( row );
  }
  emit changed();
}

//
// QgsLabelingEngineRulesDialog
//

QgsLabelingEngineRulesDialog::QgsLabelingEngineRulesDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setWindowTitle( tr( "Configure Rules" ) );
  setObjectName( QStringLiteral( "QgsLabelingEngineRulesDialog" ) );

  mWidget = new QgsLabelingEngineRulesWidget();

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mWidget );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, Qt::Horizontal, this );
  layout->addWidget( mButtonBox );

  setLayout( layout );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mButtonBox->button( QDialogButtonBox::Ok ), &QAbstractButton::clicked, this, &QDialog::accept );
  connect( mButtonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [=] {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#labeling-rules" ) );
  } );
}

void QgsLabelingEngineRulesDialog::setRules( const QList<QgsAbstractLabelingEngineRule *> &rules )
{
  mWidget->setRules( rules );
}

QList<QgsAbstractLabelingEngineRule *> QgsLabelingEngineRulesDialog::rules() const
{
  return mWidget->rules();
}
