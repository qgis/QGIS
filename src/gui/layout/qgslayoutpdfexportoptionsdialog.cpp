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

QgsLayoutPdfExportOptionsDialog::QgsLayoutPdfExportOptionsDialog( QWidget *parent, bool allowGeoPdfExport, const QString &geoPdfReason, const QStringList &geoPdfLayerOrder, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );

  mGeoPdfStructureTreeMenu = new QMenu( this );

  mTextRenderFormatComboBox->addItem( tr( "Always Export Text as Paths (Recommended)" ), static_cast< int >( Qgis::TextRenderFormat::AlwaysOutlines ) );
  mTextRenderFormatComboBox->addItem( tr( "Always Export Text as Text Objects" ), static_cast< int >( Qgis::TextRenderFormat::AlwaysText ) );

  mGeopdfAvailable = allowGeoPdfExport && QgsAbstractGeoPdfExporter::geoPDFCreationAvailable();
  mGeoPDFGroupBox->setEnabled( mGeopdfAvailable );
  mGeoPDFGroupBox->setChecked( false );
  if ( !mGeopdfAvailable )
  {
    mGeoPDFOptionsStackedWidget->setCurrentIndex( 0 );
    mGeoPdfUnavailableReason->setText( geoPdfReason.isEmpty() ? QgsAbstractGeoPdfExporter::geoPDFAvailabilityExplanation() : geoPdfReason );
    // avoid showing reason in disabled text color - we want it to stand out
    QPalette p = mGeoPdfUnavailableReason->palette();
    p.setColor( QPalette::Disabled, QPalette::WindowText, QPalette::WindowText );
    mGeoPdfUnavailableReason->setPalette( p );
    mGeoPDFOptionsStackedWidget->removeWidget( mGeoPDFOptionsStackedWidget->widget( 1 ) );
  }
  else
  {
    mGeoPDFOptionsStackedWidget->setCurrentIndex( 1 );
    mGeoPdfFormatComboBox->addItem( tr( "ISO 32000 Extension (recommended)" ) );
    mGeoPdfFormatComboBox->addItem( tr( "OGC Best Practice" ) );
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

  QList< QgsMapLayer * > order = QgsProject::instance()->layerTreeRoot()->layerOrder();
  for ( auto it = geoPdfLayerOrder.rbegin(); it != geoPdfLayerOrder.rend(); ++it )
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
  mGeoPdfStructureModel = new QgsGeoPdfLayerTreeModel( order, this );
  mGeoPdfStructureProxyModel = new QgsGeoPdfLayerFilteredTreeModel( mGeoPdfStructureModel, this );
  mGeoPdfStructureTree->setModel( mGeoPdfStructureProxyModel );
  mGeoPdfStructureTree->resizeColumnToContents( 0 );
  mGeoPdfStructureTree->header()->show();
  mGeoPdfStructureTree->setSelectionMode( QAbstractItemView::SingleSelection );
  mGeoPdfStructureTree->setSelectionBehavior( QAbstractItemView::SelectRows );

  mGeoPdfStructureTree->setDragEnabled( true );
  mGeoPdfStructureTree->setAcceptDrops( true );
  mGeoPdfStructureTree->setDragDropMode( QAbstractItemView::InternalMove );
  mGeoPdfStructureTree->setDefaultDropAction( Qt::MoveAction );

  mGeoPdfStructureTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mGeoPdfStructureTree, &QTreeView::customContextMenuRequested, this, [ = ]( const QPoint & point )
  {
    const QModelIndex index = mGeoPdfStructureTree->indexAt( point );
    if ( index.isValid() )
      showContextMenuForGeoPdfStructure( point, mGeoPdfStructureProxyModel->mapToSource( index ) );
  } );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsLayoutPdfExportOptionsDialog::showHelp );
  QgsGui::enableAutoGeometryRestore( this );
}

void QgsLayoutPdfExportOptionsDialog::setTextRenderFormat( Qgis::TextRenderFormat format )
{
  mTextRenderFormatComboBox->setCurrentIndex( mTextRenderFormatComboBox->findData( static_cast< int >( format ) ) );
}

Qgis::TextRenderFormat QgsLayoutPdfExportOptionsDialog::textRenderFormat() const
{
  return static_cast< Qgis::TextRenderFormat >( mTextRenderFormatComboBox->currentData().toInt() );
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

void QgsLayoutPdfExportOptionsDialog::setExportGeoPdf( bool enabled )
{
  if ( !mGeopdfAvailable )
    return;

  mGeoPDFGroupBox->setChecked( enabled );
}

bool QgsLayoutPdfExportOptionsDialog::exportGeoPdf() const
{
  if ( !mGeopdfAvailable )
    return false;

  return mGeoPDFGroupBox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::setUseOgcBestPracticeFormat( bool enabled )
{
  if ( !mGeopdfAvailable )
    return;

  if ( enabled )
    mGeoPdfFormatComboBox->setCurrentIndex( 1 );
  else
    mGeoPdfFormatComboBox->setCurrentIndex( 0 );
}

bool QgsLayoutPdfExportOptionsDialog::useOgcBestPracticeFormat() const
{
  if ( !mGeopdfAvailable )
    return false;

  return mGeoPdfFormatComboBox->currentIndex() == 1;
}


void QgsLayoutPdfExportOptionsDialog::setExportThemes( const QStringList &themes )
{
  if ( !mGeopdfAvailable )
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
  if ( !mGeopdfAvailable )
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

QStringList QgsLayoutPdfExportOptionsDialog::geoPdfLayerOrder() const
{
  QStringList order;
  for ( int row = 0; row < mGeoPdfStructureProxyModel->rowCount(); ++row )
  {
    order << mGeoPdfStructureProxyModel->data( mGeoPdfStructureProxyModel->index( row, 0 ), QgsGeoPdfLayerTreeModel::LayerIdRole ).toString();
  }
  return order;
}

void QgsLayoutPdfExportOptionsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "print_composer/create_output.html" ) );
}

void QgsLayoutPdfExportOptionsDialog::showContextMenuForGeoPdfStructure( QPoint point, const QModelIndex &index )
{
  mGeoPdfStructureTreeMenu->clear();

  switch ( index.column() )
  {
    case QgsGeoPdfLayerTreeModel::IncludeVectorAttributes:
    case QgsGeoPdfLayerTreeModel::InitiallyVisible:
    {
      QAction *selectAll = new QAction( tr( "Select All" ), mGeoPdfStructureTreeMenu );
      mGeoPdfStructureTreeMenu->addAction( selectAll );
      connect( selectAll, &QAction::triggered, this, [ = ]
      {
        mGeoPdfStructureModel->checkAll( true, QModelIndex(), index.column() );
      } );
      QAction *deselectAll = new QAction( tr( "Deselect All" ), mGeoPdfStructureTreeMenu );
      mGeoPdfStructureTreeMenu->addAction( deselectAll );
      connect( deselectAll, &QAction::triggered, this, [ = ]
      {
        mGeoPdfStructureModel->checkAll( false, QModelIndex(), index.column() );
      } );
      break;
    }

    default:
      break;
  }

  if ( !mGeoPdfStructureTreeMenu->actions().empty() )
  {
    mGeoPdfStructureTreeMenu->exec( mGeoPdfStructureTree->mapToGlobal( point ) );
  }
}
