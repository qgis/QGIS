/***************************************************************************
  qgsdatasourceselectdialog.cpp - QgsDataSourceSelectDialog

 ---------------------
 begin                : 1.11.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatasourceselectdialog.h"
#include "ui_qgsdatasourceselectdialog.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgis.h"
#include "qgsbrowsermodel.h"

#include <QPushButton>
#include <QMenu>

QgsDataSourceSelectDialog::QgsDataSourceSelectDialog(
  QgsBrowserModel *browserModel,
  bool setFilterByLayerType,
  const QgsMapLayer::LayerType &layerType,
  QWidget *parent )
  : QDialog( parent )
{
  if ( ! browserModel )
  {
    mBrowserModel = qgis::make_unique<QgsBrowserModel>();
    mBrowserModel->initialize();
    mOwnModel = true;
  }
  else
  {
    mBrowserModel.reset( browserModel );
    mOwnModel = false;
  }

  setupUi( this );
  setWindowTitle( tr( "Select a Data Source" ) );
  QgsGui::enableAutoGeometryRestore( this );

  mBrowserProxyModel.setBrowserModel( mBrowserModel.get() );
  mBrowserTreeView->setHeaderHidden( true );

  if ( setFilterByLayerType )
  {
    // This will also set the (proxy) model
    setLayerTypeFilter( layerType );
  }
  else
  {
    mBrowserTreeView->setModel( &mBrowserProxyModel );
    buttonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( false );
  }

  mBrowserTreeView->setBrowserModel( mBrowserModel.get() );

  mWidgetFilter->hide();
  mLeFilter->setPlaceholderText( tr( "Type here to filter visible items…" ) );
  // icons from http://www.fatcow.com/free-icons License: CC Attribution 3.0

  QMenu *menu = new QMenu( this );
  menu->setSeparatorsCollapsible( false );
  mBtnFilterOptions->setMenu( menu );
  QAction *action = new QAction( tr( "Case Sensitive" ), menu );
  action->setData( "case" );
  action->setCheckable( true );
  action->setChecked( false );
  connect( action, &QAction::toggled, this, &QgsDataSourceSelectDialog::setCaseSensitive );
  menu->addAction( action );
  QActionGroup *group = new QActionGroup( menu );
  action = new QAction( tr( "Filter Pattern Syntax" ), group );
  action->setSeparator( true );
  menu->addAction( action );
  action = new QAction( tr( "Normal" ), group );
  action->setData( QgsBrowserProxyModel::Normal );
  action->setCheckable( true );
  action->setChecked( true );
  menu->addAction( action );
  action = new QAction( tr( "Wildcard(s)" ), group );
  action->setData( QgsBrowserProxyModel::Wildcards );
  action->setCheckable( true );
  menu->addAction( action );
  action = new QAction( tr( "Regular Expression" ), group );
  action->setData( QgsBrowserProxyModel::RegularExpression );
  action->setCheckable( true );
  menu->addAction( action );

  mBrowserTreeView->setExpandsOnDoubleClick( false );

  connect( mActionRefresh, &QAction::triggered, [ = ] { refreshModel( QModelIndex() ); } );
  connect( mBrowserTreeView, &QgsBrowserTreeView::clicked, this, &QgsDataSourceSelectDialog::onLayerSelected );
  connect( mActionCollapse, &QAction::triggered, mBrowserTreeView, &QgsBrowserTreeView::collapseAll );
  connect( mActionShowFilter, &QAction::triggered, this, &QgsDataSourceSelectDialog::showFilterWidget );
  connect( mLeFilter, &QgsFilterLineEdit::returnPressed, this, &QgsDataSourceSelectDialog::setFilter );
  connect( mLeFilter, &QgsFilterLineEdit::cleared, this, &QgsDataSourceSelectDialog::setFilter );
  connect( mLeFilter, &QgsFilterLineEdit::textChanged, this, &QgsDataSourceSelectDialog::setFilter );
  connect( group, &QActionGroup::triggered, this, &QgsDataSourceSelectDialog::setFilterSyntax );

  if ( QgsSettings().value( QStringLiteral( "datasourceSelectFilterVisible" ), false, QgsSettings::Section::Gui ).toBool() )
  {
    mActionShowFilter->trigger();
  }
}

QgsDataSourceSelectDialog::~QgsDataSourceSelectDialog()
{
  if ( ! mOwnModel )
    mBrowserModel.release();
}


void QgsDataSourceSelectDialog::showEvent( QShowEvent *e )
{
  QDialog::showEvent( e );
  QString lastSelectedPath( QgsSettings().value( QStringLiteral( "datasourceSelectLastSelectedItem" ),
                            QString(), QgsSettings::Section::Gui ).toString() );
  if ( ! lastSelectedPath.isEmpty() )
  {
    QModelIndexList items = mBrowserProxyModel.match(
                              mBrowserProxyModel.index( 0, 0 ),
                              QgsBrowserModel::PathRole,
                              QVariant::fromValue( lastSelectedPath ),
                              1,
                              Qt::MatchRecursive );
    if ( items.count( ) > 0 )
    {
      QModelIndex expandIndex = items.at( 0 );
      if ( expandIndex.isValid() )
      {
        mBrowserTreeView->scrollTo( expandIndex, QgsBrowserTreeView::ScrollHint::PositionAtTop );
        mBrowserTreeView->expand( expandIndex );
      }
    }
  }
}

void QgsDataSourceSelectDialog::showFilterWidget( bool visible )
{
  QgsSettings().setValue( QStringLiteral( "datasourceSelectFilterVisible" ), visible, QgsSettings::Section::Gui );
  mWidgetFilter->setVisible( visible );
  if ( ! visible )
  {
    mLeFilter->setText( QString() );
    setFilter();
  }
  else
  {
    mLeFilter->setFocus();
  }
}

void QgsDataSourceSelectDialog::setFilter()
{
  QString filter = mLeFilter->text();
  mBrowserProxyModel.setFilterString( filter );
}


void QgsDataSourceSelectDialog::refreshModel( const QModelIndex &index )
{

  QgsDataItem *item = mBrowserModel->dataItem( index );
  if ( item )
  {
    QgsDebugMsg( "path = " + item->path() );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "invalid item" ) );
  }

  if ( item && ( item->capabilities2() & QgsDataItem::Fertile ) )
  {
    mBrowserModel->refresh( index );
  }

  for ( int i = 0; i < mBrowserModel->rowCount( index ); i++ )
  {
    QModelIndex idx = mBrowserModel->index( i, 0, index );
    QModelIndex proxyIdx = mBrowserProxyModel.mapFromSource( idx );
    QgsDataItem *child = mBrowserModel->dataItem( idx );

    // Check also expanded descendants so that the whole expanded path does not get collapsed if one item is collapsed.
    // Fast items (usually root items) are refreshed so that when collapsed, it is obvious they are if empty (no expand symbol).
    if ( mBrowserTreeView->isExpanded( proxyIdx ) || mBrowserTreeView->hasExpandedDescendant( proxyIdx ) || ( child && child->capabilities2() & QgsDataItem::Fast ) )
    {
      refreshModel( idx );
    }
    else
    {
      if ( child && ( child->capabilities2() & QgsDataItem::Fertile ) )
      {
        child->depopulate();
      }
    }
  }
}


void QgsDataSourceSelectDialog::setFilterSyntax( QAction *action )
{
  if ( !action )
    return;
  mBrowserProxyModel.setFilterSyntax( static_cast< QgsBrowserProxyModel::FilterSyntax >( action->data().toInt() ) );
}

void QgsDataSourceSelectDialog::setCaseSensitive( bool caseSensitive )
{
  mBrowserProxyModel.setFilterCaseSensitivity( caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive );
}

void QgsDataSourceSelectDialog::setLayerTypeFilter( QgsMapLayer::LayerType layerType )
{
  mBrowserProxyModel.setFilterByLayerType( true );
  mBrowserProxyModel.setLayerType( layerType );
  // reset model and button
  mBrowserTreeView->setModel( &mBrowserProxyModel );
  buttonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( false );
}

QgsMimeDataUtils::Uri QgsDataSourceSelectDialog::uri() const
{
  return mUri;
}

void QgsDataSourceSelectDialog::onLayerSelected( const QModelIndex &index )
{
  bool isLayerCompatible = false;
  mUri = QgsMimeDataUtils::Uri();
  if ( index.isValid() )
  {
    const QgsDataItem *dataItem( mBrowserProxyModel.dataItem( index ) );
    if ( dataItem )
    {
      const QgsLayerItem *layerItem = qobject_cast<const QgsLayerItem *>( dataItem );
      if ( layerItem && ( ! mBrowserProxyModel.filterByLayerType() ||
                          ( layerItem->mapLayerType() == mBrowserProxyModel.layerType() ) ) )
      {
        isLayerCompatible = true;
        mUri = layerItem->mimeUri();
        // Store last viewed item
        QgsSettings().setValue( QStringLiteral( "datasourceSelectLastSelectedItem" ),  mBrowserProxyModel.data( index, QgsBrowserModel::PathRole ).toString(), QgsSettings::Section::Gui );
      }
    }
  }
  buttonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( isLayerCompatible );
}

