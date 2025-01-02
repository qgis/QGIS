/***************************************************************************
                         qgslayoutpdfexportoptionsdialog.cpp
                         -------------------------------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutpdfexportoptionsdialog.h"
#include "moc_qgslayoutpdfexportoptionsdialog.cpp"
#include "qgis.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsabstractgeopdfexporter.h"
#include "qgsproject.h"
#include "qgsmapthemecollection.h"
#include "qgsgeopdflayertreemodel.h"
#include "qgslayertree.h"

#include <QCheckBox>
#include <QPushButton>
#include <QMenu>

QgsLayoutPdfExportOptionsDialog::QgsLayoutPdfExportOptionsDialog( QWidget *parent, bool allowGeospatialPdfExport, const QString &geospatialPdfReason, const QStringList &geospatialPdfLayerOrder, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );

  mGeospatialPdfStructureTreeMenu = new QMenu( this );

  mTextRenderFormatComboBox->addItem( tr( "Always Export Text as Paths (Recommended)" ), static_cast<int>( Qgis::TextRenderFormat::AlwaysOutlines ) );
  mTextRenderFormatComboBox->addItem( tr( "Always Export Text as Text Objects" ), static_cast<int>( Qgis::TextRenderFormat::AlwaysText ) );
  mTextRenderFormatComboBox->addItem( tr( "Prefer Exporting Text as Text Objects" ), static_cast<int>( Qgis::TextRenderFormat::PreferText ) );

  mGeospatialPdfAvailable = allowGeospatialPdfExport && QgsAbstractGeospatialPdfExporter::geospatialPDFCreationAvailable();
  mGeospatialPDFGroupBox->setEnabled( mGeospatialPdfAvailable );
  mGeospatialPDFGroupBox->setChecked( false );
  if ( !mGeospatialPdfAvailable )
  {
    mGeospatialPDFOptionsStackedWidget->setCurrentIndex( 0 );
    mGeospatialPdfUnavailableReason->setText( geospatialPdfReason.isEmpty() ? QgsAbstractGeospatialPdfExporter::geospatialPDFAvailabilityExplanation() : geospatialPdfReason );
    // avoid showing reason in disabled text color - we want it to stand out
    QPalette p = mGeospatialPdfUnavailableReason->palette();
    p.setColor( QPalette::Disabled, QPalette::WindowText, QPalette::WindowText );
    mGeospatialPdfUnavailableReason->setPalette( p );
    mGeospatialPDFOptionsStackedWidget->removeWidget( mGeospatialPDFOptionsStackedWidget->widget( 1 ) );
  }
  else
  {
    mGeospatialPDFOptionsStackedWidget->setCurrentIndex( 1 );
    mGeospatialPdfFormatComboBox->addItem( tr( "ISO 32000 Extension (recommended)" ) );
    mGeospatialPdfFormatComboBox->addItem( tr( "OGC Best Practice" ) );
  }

  mComboImageCompression->addItem( tr( "Lossy (JPEG)" ), false );
  mComboImageCompression->addItem( tr( "Lossless" ), true );

  const QStringList themes = QgsProject::instance()->mapThemeCollection()->mapThemes();
  for ( const QString &theme : themes )
  {
    QListWidgetItem *item = new QListWidgetItem( theme );
    item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
    item->setCheckState( Qt::Unchecked );
    mThemesList->addItem( item );
  }

  QList<QgsMapLayer *> order = QgsProject::instance()->layerTreeRoot()->layerOrder();
  for ( auto it = geospatialPdfLayerOrder.rbegin(); it != geospatialPdfLayerOrder.rend(); ++it )
  {
    for ( int i = 0; i < order.size(); ++i )
    {
      if ( order.at( i )->id() == *it )
      {
        order.move( i, 0 );
        break;
      }
    }
  }
  mGeospatialPdfStructureModel = new QgsGeospatialPdfLayerTreeModel( order, this );
  mGeospatialPdfStructureProxyModel = new QgsGeospatialPdfLayerFilteredTreeModel( mGeospatialPdfStructureModel, this );
  mGeospatialPdfStructureTree->setModel( mGeospatialPdfStructureProxyModel );
  mGeospatialPdfStructureTree->resizeColumnToContents( 0 );
  mGeospatialPdfStructureTree->header()->show();
  mGeospatialPdfStructureTree->setSelectionMode( QAbstractItemView::SingleSelection );
  mGeospatialPdfStructureTree->setSelectionBehavior( QAbstractItemView::SelectRows );

  mGeospatialPdfStructureTree->setDragEnabled( true );
  mGeospatialPdfStructureTree->setAcceptDrops( true );
  mGeospatialPdfStructureTree->setDragDropMode( QAbstractItemView::InternalMove );
  mGeospatialPdfStructureTree->setDefaultDropAction( Qt::MoveAction );

  mGeospatialPdfStructureTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mGeospatialPdfStructureTree, &QTreeView::customContextMenuRequested, this, [=]( const QPoint &point ) {
    const QModelIndex index = mGeospatialPdfStructureTree->indexAt( point );
    if ( index.isValid() )
      showContextMenuForGeospatialPdfStructure( point, mGeospatialPdfStructureProxyModel->mapToSource( index ) );
  } );

  connect( mHelpButtonBox, &QDialogButtonBox::helpRequested, this, &QgsLayoutPdfExportOptionsDialog::showHelp );
  QgsGui::enableAutoGeometryRestore( this );
}

void QgsLayoutPdfExportOptionsDialog::setTextRenderFormat( Qgis::TextRenderFormat format )
{
  mTextRenderFormatComboBox->setCurrentIndex( mTextRenderFormatComboBox->findData( static_cast<int>( format ) ) );
}

Qgis::TextRenderFormat QgsLayoutPdfExportOptionsDialog::textRenderFormat() const
{
  return static_cast<Qgis::TextRenderFormat>( mTextRenderFormatComboBox->currentData().toInt() );
}

void QgsLayoutPdfExportOptionsDialog::setForceVector( bool force )
{
  mForceVectorCheckBox->setChecked( force );
}

bool QgsLayoutPdfExportOptionsDialog::forceVector() const
{
  return mForceVectorCheckBox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::enableGeoreferencingOptions( bool enabled )
{
  mAppendGeoreferenceCheckbox->setEnabled( enabled );
}

void QgsLayoutPdfExportOptionsDialog::setGeoreferencingEnabled( bool enabled )
{
  mAppendGeoreferenceCheckbox->setChecked( enabled );
}

bool QgsLayoutPdfExportOptionsDialog::georeferencingEnabled() const
{
  return mAppendGeoreferenceCheckbox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::setMetadataEnabled( bool enabled )
{
  mIncludeMetadataCheckbox->setChecked( enabled );
}

bool QgsLayoutPdfExportOptionsDialog::metadataEnabled() const
{
  return mIncludeMetadataCheckbox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::setRasterTilingDisabled( bool disabled )
{
  mDisableRasterTilingCheckBox->setChecked( disabled );
}

bool QgsLayoutPdfExportOptionsDialog::rasterTilingDisabled() const
{
  return mDisableRasterTilingCheckBox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::setGeometriesSimplified( bool enabled )
{
  mSimplifyGeometriesCheckbox->setChecked( enabled );
}

bool QgsLayoutPdfExportOptionsDialog::geometriesSimplified() const
{
  return mSimplifyGeometriesCheckbox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::setLosslessImageExport( bool enabled )
{
  mComboImageCompression->setCurrentIndex( mComboImageCompression->findData( enabled ) );
}

bool QgsLayoutPdfExportOptionsDialog::losslessImageExport() const
{
  return mComboImageCompression->currentData().toBool();
}

void QgsLayoutPdfExportOptionsDialog::setExportGeospatialPdf( bool enabled )
{
  if ( !mGeospatialPdfAvailable )
    return;

  mGeospatialPDFGroupBox->setChecked( enabled );
}

bool QgsLayoutPdfExportOptionsDialog::exportGeospatialPdf() const
{
  if ( !mGeospatialPdfAvailable )
    return false;

  return mGeospatialPDFGroupBox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::setUseOgcBestPracticeFormat( bool enabled )
{
  if ( !mGeospatialPdfAvailable )
    return;

  if ( enabled )
    mGeospatialPdfFormatComboBox->setCurrentIndex( 1 );
  else
    mGeospatialPdfFormatComboBox->setCurrentIndex( 0 );
}

bool QgsLayoutPdfExportOptionsDialog::useOgcBestPracticeFormat() const
{
  if ( !mGeospatialPdfAvailable )
    return false;

  return mGeospatialPdfFormatComboBox->currentIndex() == 1;
}


void QgsLayoutPdfExportOptionsDialog::setExportThemes( const QStringList &themes )
{
  if ( !mGeospatialPdfAvailable )
    return;

  mIncludeMapThemesCheck->setChecked( !themes.isEmpty() );
  for ( int i = 0; i < mThemesList->count(); ++i )
  {
    QListWidgetItem *item = mThemesList->item( i );
    item->setCheckState( themes.contains( item->text() ) ? Qt::Checked : Qt::Unchecked );
  }
}

QStringList QgsLayoutPdfExportOptionsDialog::exportThemes() const
{
  QStringList res;
  if ( !mGeospatialPdfAvailable )
    return res;

  if ( !mIncludeMapThemesCheck || !mIncludeMapThemesCheck->isChecked() )
    return res;

  res.reserve( mThemesList->count() );
  for ( int i = 0; i < mThemesList->count(); ++i )
  {
    QListWidgetItem *item = mThemesList->item( i );
    if ( item->checkState() == Qt::Checked )
      res << item->text();
  }
  return res;
}

QStringList QgsLayoutPdfExportOptionsDialog::geospatialPdfLayerOrder() const
{
  QStringList order;
  for ( int row = 0; row < mGeospatialPdfStructureProxyModel->rowCount(); ++row )
  {
    order << mGeospatialPdfStructureProxyModel->data( mGeospatialPdfStructureProxyModel->index( row, 0 ), static_cast<int>( QgsMapLayerModel::CustomRole::LayerId ) ).toString();
  }
  return order;
}

QStringList QgsLayoutPdfExportOptionsDialog::geospatialPdfGroupOrder() const
{
  // we don't explicitly expose a "group order" widget in the dialog -- rather
  // we use the ordering of the layers, and build the group ordering based
  // on grouped layers which appear first
  QStringList groupOrder;
  for ( int row = 0; row < mGeospatialPdfStructureProxyModel->rowCount(); ++row )
  {
    const QString group = mGeospatialPdfStructureProxyModel->data( mGeospatialPdfStructureProxyModel->index( row, QgsGeospatialPdfLayerTreeModel::GroupColumn ), Qt::DisplayRole ).toString().trimmed();
    if ( !group.isEmpty() && !groupOrder.contains( group ) )
      groupOrder << group;
  }
  return groupOrder;
}

void QgsLayoutPdfExportOptionsDialog::setOpenAfterExporting( bool enabled )
{
  mOpenAfterExportingCheckBox->setChecked( enabled );
}

bool QgsLayoutPdfExportOptionsDialog::openAfterExporting() const
{
  return mOpenAfterExportingCheckBox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "print_composer/create_output.html" ) );
}

void QgsLayoutPdfExportOptionsDialog::showContextMenuForGeospatialPdfStructure( QPoint point, const QModelIndex &index )
{
  mGeospatialPdfStructureTreeMenu->clear();

  switch ( index.column() )
  {
    case QgsGeospatialPdfLayerTreeModel::IncludeVectorAttributes:
    case QgsGeospatialPdfLayerTreeModel::InitiallyVisible:
    {
      QAction *selectAll = new QAction( tr( "Select All" ), mGeospatialPdfStructureTreeMenu );
      mGeospatialPdfStructureTreeMenu->addAction( selectAll );
      connect( selectAll, &QAction::triggered, this, [=] {
        mGeospatialPdfStructureModel->checkAll( true, QModelIndex(), index.column() );
      } );
      QAction *deselectAll = new QAction( tr( "Deselect All" ), mGeospatialPdfStructureTreeMenu );
      mGeospatialPdfStructureTreeMenu->addAction( deselectAll );
      connect( deselectAll, &QAction::triggered, this, [=] {
        mGeospatialPdfStructureModel->checkAll( false, QModelIndex(), index.column() );
      } );
      break;
    }

    default:
      break;
  }

  if ( !mGeospatialPdfStructureTreeMenu->actions().empty() )
  {
    mGeospatialPdfStructureTreeMenu->exec( mGeospatialPdfStructureTree->mapToGlobal( point ) );
  }
}
