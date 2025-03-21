/***************************************************************************
    qgsmaplayerserverpropertieswidget.cpp
    ---------------------
    begin                : 2025/02/20
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerserverpropertieswidget.h"
#include "moc_qgsmaplayerserverpropertieswidget.cpp"
#include "qgsmaplayerserverproperties.h"
#include "qgsapplication.h"
#include "qgsmetadataurlitemdelegate.h"

#include <QRegularExpressionValidator>
#include <QStandardItemModel>

QgsMapLayerServerPropertiesWidget::QgsMapLayerServerPropertiesWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  connect( buttonRemoveMetadataUrl, &QPushButton::clicked, this, &QgsMapLayerServerPropertiesWidget::removeSelectedMetadataUrl );
  connect( buttonAddMetadataUrl, &QPushButton::clicked, this, &QgsMapLayerServerPropertiesWidget::addMetadataUrl );
}

void QgsMapLayerServerPropertiesWidget::setServerProperties( QgsMapLayerServerProperties *serverProperties )
{
  mServerProperties = serverProperties;
  sync();
}

void QgsMapLayerServerPropertiesWidget::setHasWfsTitle( bool hasWfsTitle )
{
  mHasWfsTitle = hasWfsTitle;
  mLayerOptWfsTitleLineEdit->setVisible( mHasWfsTitle );
  mLayerOptWfsTitleLabel->setVisible( mHasWfsTitle );
}

bool QgsMapLayerServerPropertiesWidget::hasWfsTitle() const
{
  return mHasWfsTitle;
}

bool QgsMapLayerServerPropertiesWidget::save()
{
  if ( !mServerProperties )
    return false;

  bool hasChanged = mServerProperties->shortName() != mLayerShortNameLineEdit->text()
                    || mServerProperties->title() != mLayerTitleLineEdit->text()
                    || mServerProperties->abstract() != mLayerAbstractTextEdit->toPlainText()
                    || mServerProperties->keywordList() != mLayerKeywordListLineEdit->text()
                    || mServerProperties->dataUrl() != mLayerDataUrlLineEdit->text()
                    || mServerProperties->dataUrlFormat() != mLayerDataUrlFormatComboBox->currentText()
                    || mServerProperties->attribution() != mLayerAttributionLineEdit->text()
                    || mServerProperties->attributionUrl() != mLayerAttributionUrlLineEdit->text()
                    || mServerProperties->legendUrl() != mLayerLegendUrlLineEdit->text()
                    || mServerProperties->legendUrlFormat() != mLayerLegendUrlFormatComboBox->currentText();

  mServerProperties->setShortName( mLayerShortNameLineEdit->text() );
  mServerProperties->setTitle( mLayerTitleLineEdit->text() );
  mServerProperties->setAbstract( mLayerAbstractTextEdit->toPlainText() );
  mServerProperties->setKeywordList( mLayerKeywordListLineEdit->text() );
  mServerProperties->setDataUrl( mLayerDataUrlLineEdit->text() );
  mServerProperties->setDataUrlFormat( mLayerDataUrlFormatComboBox->currentText() );
  mServerProperties->setAttribution( mLayerAttributionLineEdit->text() );
  mServerProperties->setAttributionUrl( mLayerAttributionUrlLineEdit->text() );
  mServerProperties->setLegendUrl( mLayerLegendUrlLineEdit->text() );
  mServerProperties->setLegendUrlFormat( mLayerLegendUrlFormatComboBox->currentText() );

  if ( !mLayerOptWfsTitleLineEdit->text().isEmpty() && mLayerOptWfsTitleLineEdit->text() != mLayerTitleLineEdit->text() )
  {
    mServerProperties->setWfsTitle( mLayerOptWfsTitleLineEdit->text() );
    hasChanged = true;
  }
  else
  {
    mServerProperties->setWfsTitle( QString() );
  }

  // Metadata URL
  QList<QgsMapLayerServerProperties::MetadataUrl> metaUrls;
  for ( int row = 0; row < mMetadataUrlModel->rowCount(); row++ )
  {
    QgsMapLayerServerProperties::MetadataUrl metaUrl;
    metaUrl.url = mMetadataUrlModel->item( row, 0 )->text();
    metaUrl.type = mMetadataUrlModel->item( row, 1 )->text();
    metaUrl.format = mMetadataUrlModel->item( row, 2 )->text();
    metaUrls.append( metaUrl );

    // TODO has not necessary changed
    hasChanged = true;
  }
  mServerProperties->setMetadataUrls( metaUrls );

  return hasChanged;
}

void QgsMapLayerServerPropertiesWidget::sync()
{
  if ( !mServerProperties )
    return;

  // WMS Name as layer short name
  mLayerShortNameLineEdit->setText( mServerProperties->shortName() );
  // WMS Name validator
  QValidator *shortNameValidator = new QRegularExpressionValidator( QgsApplication::shortNameRegularExpression(), this );
  mLayerShortNameLineEdit->setValidator( shortNameValidator );

  //layer title and abstract
  mLayerTitleLineEdit->setText( mServerProperties->title() );

  if ( mServerProperties->wfsTitle() != mServerProperties->title() )
    mLayerOptWfsTitleLineEdit->setText( mServerProperties->wfsTitle() );

  mLayerAbstractTextEdit->setPlainText( mServerProperties->abstract() );
  mLayerKeywordListLineEdit->setText( mServerProperties->keywordList() );
  mLayerDataUrlLineEdit->setText( mServerProperties->dataUrl() );
  mLayerDataUrlFormatComboBox->setCurrentIndex(
    mLayerDataUrlFormatComboBox->findText(
      mServerProperties->dataUrlFormat()
    )
  );
  //layer attribution
  mLayerAttributionLineEdit->setText( mServerProperties->attribution() );
  mLayerAttributionUrlLineEdit->setText( mServerProperties->attributionUrl() );

  // Setup the layer metadata URL
  tableViewMetadataUrl->setSelectionMode( QAbstractItemView::SingleSelection );
  tableViewMetadataUrl->setSelectionBehavior( QAbstractItemView::SelectRows );
  tableViewMetadataUrl->horizontalHeader()->setStretchLastSection( true );
  tableViewMetadataUrl->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

  mMetadataUrlModel = new QStandardItemModel( tableViewMetadataUrl );
  mMetadataUrlModel->clear();
  mMetadataUrlModel->setColumnCount( 3 );
  QStringList metadataUrlHeaders;
  metadataUrlHeaders << tr( "URL" ) << tr( "Type" ) << tr( "Format" );
  mMetadataUrlModel->setHorizontalHeaderLabels( metadataUrlHeaders );
  tableViewMetadataUrl->setModel( mMetadataUrlModel );
  tableViewMetadataUrl->setItemDelegate( new MetadataUrlItemDelegate( this ) );

  const QList<QgsMapLayerServerProperties::MetadataUrl> &metaUrls = mServerProperties->metadataUrls();
  for ( const QgsMapLayerServerProperties::MetadataUrl &metaUrl : metaUrls )
  {
    const int row = mMetadataUrlModel->rowCount();
    mMetadataUrlModel->setItem( row, 0, new QStandardItem( metaUrl.url ) );
    mMetadataUrlModel->setItem( row, 1, new QStandardItem( metaUrl.type ) );
    mMetadataUrlModel->setItem( row, 2, new QStandardItem( metaUrl.format ) );
  }

  // layer legend url
  mLayerLegendUrlLineEdit->setText( mServerProperties->legendUrl() );
  mLayerLegendUrlFormatComboBox->setCurrentIndex(
    mLayerLegendUrlFormatComboBox->findText(
      mServerProperties->legendUrlFormat()
    )
  );
}

void QgsMapLayerServerPropertiesWidget::addMetadataUrl()
{
  const int row = mMetadataUrlModel->rowCount();
  mMetadataUrlModel->setItem( row, 0, new QStandardItem( QLatin1String() ) );
  mMetadataUrlModel->setItem( row, 1, new QStandardItem( QLatin1String() ) );
  mMetadataUrlModel->setItem( row, 2, new QStandardItem( QLatin1String() ) );
}

void QgsMapLayerServerPropertiesWidget::removeSelectedMetadataUrl()
{
  const QModelIndexList selectedRows = tableViewMetadataUrl->selectionModel()->selectedRows();
  if ( selectedRows.empty() )
    return;
  mMetadataUrlModel->removeRow( selectedRows[0].row() );
}
