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

#include "qgis.h"
#include "qgsbrowsermodel.h"
#include "qgsgui.h"
#include "qgsguiutils.h"
#include "qgssettings.h"
#include "qgsnative.h"
#include "qgslayeritem.h"

#include <QPushButton>
#include <QMenu>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QUrl>
#include <QActionGroup>

QgsDataSourceSelectWidget::QgsDataSourceSelectWidget(
  QgsBrowserGuiModel *browserModel,
  bool setFilterByLayerType,
  QgsMapLayerType layerType,
  QWidget *parent )
  : QgsPanelWidget( parent )
{
  if ( ! browserModel )
  {
    mBrowserModel = new QgsBrowserGuiModel( this );
    mBrowserModel->initialize();
  }
  else
  {
    mBrowserModel = browserModel;
    mBrowserModel->initialize();
  }

  setupUi( this );

  mBrowserProxyModel.setBrowserModel( mBrowserModel );
  mBrowserTreeView->setHeaderHidden( true );

  if ( setFilterByLayerType )
  {
    // This will also set the (proxy) model
    setLayerTypeFilter( layerType );
  }
  else
  {
    mBrowserTreeView->setModel( &mBrowserProxyModel );
    setValid( false );
  }

  mBrowserTreeView->setBrowserModel( mBrowserModel );

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
  connect( action, &QAction::toggled, this, &QgsDataSourceSelectWidget::setCaseSensitive );
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

  connect( mActionRefresh, &QAction::triggered, this, [ = ] { refreshModel( QModelIndex() ); } );
  connect( mBrowserTreeView, &QgsBrowserTreeView::clicked, this, &QgsDataSourceSelectWidget::onLayerSelected );
  connect( mBrowserTreeView, &QgsBrowserTreeView::doubleClicked, this, &QgsDataSourceSelectWidget::itemDoubleClicked );
  connect( mActionCollapse, &QAction::triggered, mBrowserTreeView, &QgsBrowserTreeView::collapseAll );
  connect( mActionShowFilter, &QAction::triggered, this, &QgsDataSourceSelectWidget::showFilterWidget );
  connect( mLeFilter, &QgsFilterLineEdit::returnPressed, this, &QgsDataSourceSelectWidget::setFilter );
  connect( mLeFilter, &QgsFilterLineEdit::cleared, this, &QgsDataSourceSelectWidget::setFilter );
  connect( mLeFilter, &QgsFilterLineEdit::textChanged, this, &QgsDataSourceSelectWidget::setFilter );
  connect( group, &QActionGroup::triggered, this, &QgsDataSourceSelectWidget::setFilterSyntax );

  mBrowserToolbar->setIconSize( QgsGuiUtils::iconSize( true ) );

  if ( QgsSettings().value( QStringLiteral( "datasourceSelectFilterVisible" ), false, QgsSettings::Section::Gui ).toBool() )
  {
    mActionShowFilter->trigger();
  }
}

QgsDataSourceSelectWidget::~QgsDataSourceSelectWidget() = default;

void QgsDataSourceSelectWidget::showEvent( QShowEvent *e )
{
  QgsPanelWidget::showEvent( e );
  const QString lastSelectedPath( QgsSettings().value( QStringLiteral( "datasourceSelectLastSelectedItem" ),
                                  QString(), QgsSettings::Section::Gui ).toString() );
  if ( ! lastSelectedPath.isEmpty() )
  {
    const QModelIndexList items = mBrowserProxyModel.match(
                                    mBrowserProxyModel.index( 0, 0 ),
                                    QgsBrowserGuiModel::PathRole,
                                    QVariant::fromValue( lastSelectedPath ),
                                    1,
                                    Qt::MatchRecursive );
    if ( items.count( ) > 0 )
    {
      const QModelIndex expandIndex = items.at( 0 );
      if ( expandIndex.isValid() )
      {
        mBrowserTreeView->scrollTo( expandIndex, QgsBrowserTreeView::ScrollHint::PositionAtTop );
        mBrowserTreeView->expand( expandIndex );
      }
    }
  }
}

void QgsDataSourceSelectWidget::showFilterWidget( bool visible )
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

void QgsDataSourceSelectWidget::setDescription( const QString &description )
{
  if ( !description.isEmpty() )
  {
    if ( !mDescriptionLabel )
    {
      mDescriptionLabel = new QLabel();
      mDescriptionLabel->setWordWrap( true );
      mDescriptionLabel->setMargin( 4 );
      mDescriptionLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
      connect( mDescriptionLabel, &QLabel::linkActivated, this, [ = ]( const QString & link )
      {
        const QUrl url( link );
        const QFileInfo file( url.toLocalFile() );
        if ( file.exists() && !file.isDir() )
          QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
        else
          QDesktopServices::openUrl( url );
      } );
      verticalLayout->insertWidget( 1, mDescriptionLabel );
    }
    mDescriptionLabel->setText( description );
  }
  else
  {
    if ( mDescriptionLabel )
    {
      verticalLayout->removeWidget( mDescriptionLabel );
      delete mDescriptionLabel;
      mDescriptionLabel = nullptr;
    }
  }
}

void QgsDataSourceSelectWidget::setFilter()
{
  const QString filter = mLeFilter->text();
  mBrowserProxyModel.setFilterString( filter );
}


void QgsDataSourceSelectWidget::refreshModel( const QModelIndex &index )
{

  QgsDataItem *item = mBrowserModel->dataItem( index );
  if ( item )
  {
    QgsDebugMsgLevel( "path = " + item->path(), 2 );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "invalid item" ) );
  }

  if ( item && ( item->capabilities2() & Qgis::BrowserItemCapability::Fertile ) )
  {
    mBrowserModel->refresh( index );
  }

  for ( int i = 0; i < mBrowserModel->rowCount( index ); i++ )
  {
    const QModelIndex idx = mBrowserModel->index( i, 0, index );
    const QModelIndex proxyIdx = mBrowserProxyModel.mapFromSource( idx );
    QgsDataItem *child = mBrowserModel->dataItem( idx );

    // Check also expanded descendants so that the whole expanded path does not get collapsed if one item is collapsed.
    // Fast items (usually root items) are refreshed so that when collapsed, it is obvious they are if empty (no expand symbol).
    if ( mBrowserTreeView->isExpanded( proxyIdx ) || mBrowserTreeView->hasExpandedDescendant( proxyIdx ) || ( child && child->capabilities2() & Qgis::BrowserItemCapability::Fast ) )
    {
      refreshModel( idx );
    }
    else
    {
      if ( child && ( child->capabilities2() & Qgis::BrowserItemCapability::Fertile ) )
      {
        child->depopulate();
      }
    }
  }
}

void QgsDataSourceSelectWidget::setValid( bool valid )
{
  const bool prev = mIsValid;
  mIsValid = valid;
  if ( prev != mIsValid )
    emit validationChanged( mIsValid );

}


void QgsDataSourceSelectWidget::setFilterSyntax( QAction *action )
{
  if ( !action )
    return;
  mBrowserProxyModel.setFilterSyntax( static_cast< QgsBrowserProxyModel::FilterSyntax >( action->data().toInt() ) );
}

void QgsDataSourceSelectWidget::setCaseSensitive( bool caseSensitive )
{
  mBrowserProxyModel.setFilterCaseSensitivity( caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive );
}

void QgsDataSourceSelectWidget::setLayerTypeFilter( QgsMapLayerType layerType )
{
  mBrowserProxyModel.setFilterByLayerType( true );
  mBrowserProxyModel.setLayerType( layerType );
  // reset model and button
  mBrowserTreeView->setModel( &mBrowserProxyModel );
  setValid( false );
}

QgsMimeDataUtils::Uri QgsDataSourceSelectWidget::uri() const
{
  return mUri;
}

void QgsDataSourceSelectWidget::onLayerSelected( const QModelIndex &index )
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
        mUri = layerItem->mimeUris().isEmpty() ? QgsMimeDataUtils::Uri() : layerItem->mimeUris().first();
        // Store last viewed item
        QgsSettings().setValue( QStringLiteral( "datasourceSelectLastSelectedItem" ),  mBrowserProxyModel.data( index, QgsBrowserGuiModel::PathRole ).toString(), QgsSettings::Section::Gui );
      }
    }
  }
  setValid( isLayerCompatible );
  emit selectionChanged();
}

void QgsDataSourceSelectWidget::itemDoubleClicked( const QModelIndex &index )
{
  onLayerSelected( index );
  if ( mIsValid )
    emit itemTriggered( uri() );
}

//
// QgsDataSourceSelectDialog
//

QgsDataSourceSelectDialog::QgsDataSourceSelectDialog( QgsBrowserGuiModel *browserModel, bool setFilterByLayerType, QgsMapLayerType layerType, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "Select a Data Source" ) );
  setObjectName( QStringLiteral( "QgsDataSourceSelectDialog" ) );
  QgsGui::enableAutoGeometryRestore( this );

  mWidget = new QgsDataSourceSelectWidget( browserModel, setFilterByLayerType, layerType );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->addWidget( mWidget, 1 );
  vl->setContentsMargins( 4, 4, 4, 4 );
  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  connect( mWidget, &QgsDataSourceSelectWidget::validationChanged, buttonBox->button( QDialogButtonBox::Ok ), &QWidget::setEnabled );
  connect( mWidget, &QgsDataSourceSelectWidget::itemTriggered, this, &QDialog::accept );

  // pressing escape should reject the dialog
  connect( mWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );

  vl->addWidget( buttonBox );
  setLayout( vl );
}

void QgsDataSourceSelectDialog::setLayerTypeFilter( QgsMapLayerType layerType )
{
  mWidget->setLayerTypeFilter( layerType );
}

void QgsDataSourceSelectDialog::setDescription( const QString &description )
{
  mWidget->setDescription( description );
}

QgsMimeDataUtils::Uri QgsDataSourceSelectDialog::uri() const
{
  return mWidget->uri();
}

void QgsDataSourceSelectDialog::showFilterWidget( bool visible )
{
  mWidget->showFilterWidget( visible );
}

void QgsDataSourceSelectDialog::setFilterSyntax( QAction *syntax )
{
  mWidget->setFilterSyntax( syntax );
}

void QgsDataSourceSelectDialog::setCaseSensitive( bool caseSensitive )
{
  mWidget->setCaseSensitive( caseSensitive );
}

void QgsDataSourceSelectDialog::setFilter()
{
  mWidget->setFilter();

}
