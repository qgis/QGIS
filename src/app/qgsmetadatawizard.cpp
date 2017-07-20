/***************************************************************************
                          qgsmetadatawizard.h  -  description
                             -------------------
    begin                : 17/05/2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne at kartoza.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtWidgets>
#include <QIcon>
#include <QPushButton>
#include <QComboBox>
#include <QString>
#include <QInputDialog>

//#include "qgsmetadatalinkdelegate.h"
#include "qgsmetadatawizard.h"
#include "qgslogger.h"
#include "qgslayermetadatavalidator.h"
#include "qgisapp.h"
#include "qgsapplication.h"

QgsMetadataWizard::QgsMetadataWizard( QWidget *parent, QgsMapLayer *layer )
  : QDialog( parent ), mLayer( layer )
{
  setupUi( this );
  mMetadata = layer->metadata();

  tabWidget->setCurrentIndex( 0 );
//  tabWidget->setItemDelegate(new TableDelegate());
  backButton->setEnabled( false );
  nextButton->setEnabled( true );

  // Setup the link view
  mLinksModel = new QStandardItemModel();
  mLinksModel->setColumnCount( 7 );
  QStringList headers = QStringList();
  headers << tr( "Name" ) << tr( "Type" ) << tr( "URL" ) << tr( "Description" ) << tr( "Format" ) << tr( "MIME" ) << tr( "Size" );
  mLinksModel->setHorizontalHeaderLabels( headers );
  tabLinks->setModel( mLinksModel );
  tabLinks->setItemDelegate( new LinkItemDelegate() );

  connect( tabWidget, &QTabWidget::currentChanged, this, &QgsMetadataWizard::updatePanel );
  connect( cancelButton, &QPushButton::clicked, this, &QgsMetadataWizard::cancelClicked );
  connect( backButton, &QPushButton::clicked, this, &QgsMetadataWizard::backClicked );
  connect( nextButton, &QPushButton::clicked, this, &QgsMetadataWizard::nextClicked );
  connect( finishButton, &QPushButton::clicked, this, &QgsMetadataWizard::finishedClicked );
  connect( btnAutoSource, &QPushButton::clicked, this, &QgsMetadataWizard::setAutoSource );
  connect( btnAddVocabulary, &QPushButton::clicked, this, &QgsMetadataWizard::addVocabulary );
  connect( btnRemoveVocabulary, &QPushButton::clicked, this, &QgsMetadataWizard::removeVocabulary );
  connect( btnAddLicence, &QPushButton::clicked, this, &QgsMetadataWizard::addLicence );
  connect( btnRemoveLicence, &QPushButton::clicked, this, &QgsMetadataWizard::removeLicence );
  connect( btnAutoCrs, &QPushButton::clicked, this, &QgsMetadataWizard::setAutoCrs );
  connect( btnAddContact, &QPushButton::clicked, this, &QgsMetadataWizard::addContact );
  connect( btnRemoveContact, &QPushButton::clicked, this, &QgsMetadataWizard::removeContact );
  connect( tabContacts, &QTableWidget::itemSelectionChanged, this, &QgsMetadataWizard::updateContactDetails );
  connect( btnAddLink, &QPushButton::clicked, this, &QgsMetadataWizard::addLink );
  connect( btnRemoveLink, &QPushButton::clicked, this, &QgsMetadataWizard::removeLink );
  connect( btnCheckMetadata, &QPushButton::clicked, this, &QgsMetadataWizard::checkMetadata );

  fillComboBox();
  setPropertiesFromLayer();
  updateContactDetails();
}

QgsMetadataWizard::~QgsMetadataWizard()
{
}

void QgsMetadataWizard::setAutoSource()
{
  lineEditIdentifier->setText( mLayer->publicSource() );
}

void QgsMetadataWizard::addVocabulary()
{
  int row = tabKeywords->rowCount();
  tabKeywords->setRowCount( row + 1 );
  QTableWidgetItem *pCell;

  // Vocabulary
  pCell = new QTableWidgetItem( QString( "undefined %1" ).arg( row + 1 ) );
  tabKeywords->setItem( row, 0, pCell );

  // Keywords
  pCell = new QTableWidgetItem();
  tabKeywords->setItem( row, 1, pCell );
}

void QgsMetadataWizard::removeVocabulary()
{
  QItemSelectionModel *selectionModel = tabKeywords->selectionModel();
  QModelIndexList selectedRows = selectionModel->selectedRows();
  QgsDebugMsg( QString( "Remove: %1 " ).arg( selectedRows.count() ) );

  for ( int i = 0 ; i < selectedRows.size() ; i++ )
  {
    tabKeywords->model()->removeRow( selectedRows[i].row() );
  }
}

void QgsMetadataWizard::addLicence()
{
  QString newLicence = QInputDialog::getItem( this, tr( "New Licence" ), tr( "New Licence" ), parseLicenses(), 0, true );
  if ( tabLicenses->findItems( newLicence, Qt::MatchExactly ).isEmpty() )
  {
    int row = tabLicenses->rowCount();
    tabLicenses->setRowCount( row + 1 );
    QTableWidgetItem *pCell = new QTableWidgetItem( newLicence );
    tabLicenses->setItem( row, 0, pCell );
    QgsDebugMsg( QString( "Adding" ) );
  }
  else
  {
    QgsDebugMsg( QString( "Cant add" ) );
  }
}

void QgsMetadataWizard::removeLicence()
{
  QItemSelectionModel *selectionModel = tabLicenses->selectionModel();
  QModelIndexList selectedRows = selectionModel->selectedRows();
  QgsDebugMsg( QString( "Remove: %1 " ).arg( selectedRows.count() ) );

  for ( int i = 0 ; i < selectedRows.size() ; i++ )
  {
    tabLicenses->model()->removeRow( selectedRows[i].row() );
  }
}

void QgsMetadataWizard::setAutoCrs()
{
  selectionCrs->setCrs( mLayer->crs() );
}

void QgsMetadataWizard::addContact()
{
  int row = tabContacts->rowCount();
  tabContacts->setRowCount( row + 1 );
  QTableWidgetItem *pCell;

  // Name
  pCell = new QTableWidgetItem( QString( "unnamed %1" ).arg( row + 1 ) );
  tabContacts->setItem( row, 0, pCell );

  // Organization
  pCell = new QTableWidgetItem();
  tabContacts->setItem( row, 1, pCell );

  // Set last item selected
  tabContacts->selectRow( row );
}

void QgsMetadataWizard::removeContact()
{
  QItemSelectionModel *selectionModel = tabContacts->selectionModel();
  QModelIndexList selectedRows = selectionModel->selectedRows();
  QgsDebugMsg( QString( "Remove: %1 " ).arg( selectedRows.count() ) );

  for ( int i = 0 ; i < selectedRows.size() ; i++ )
  {
    tabContacts->model()->removeRow( selectedRows[i].row() );
  }
}

void QgsMetadataWizard::updateContactDetails()
{
  QItemSelectionModel *selectionModel = tabContacts->selectionModel();
  QModelIndexList selectedRows = selectionModel->selectedRows();

  if ( selectedRows.size() > 0 )
  {
    panelDetails->setDisabled( false );
    lineEditContactName->setText( tabContacts->item( selectedRows[0].row(), 0 )->text() );
    lineEditContactOrganization->setText( tabContacts->item( selectedRows[0].row(), 1 )->text() );
  }
  else
  {
    panelDetails->setDisabled( true );
    lineEditContactName->clear();
    lineEditContactOrganization->clear();
  }
}

void QgsMetadataWizard::addLink()
{
  int row = mLinksModel->rowCount();
  mLinksModel->setItem( row, 0, new QStandardItem( QString( "undefined %1" ).arg( row + 1 ) ) );
  mLinksModel->setItem( row, 1, new QStandardItem() );
  mLinksModel->setItem( row, 2, new QStandardItem() );
  mLinksModel->setItem( row, 3, new QStandardItem() );
  mLinksModel->setItem( row, 4, new QStandardItem() );
  mLinksModel->setItem( row, 5, new QStandardItem() );
  mLinksModel->setItem( row, 6, new QStandardItem() );
}

void QgsMetadataWizard::removeLink()
{
  QModelIndexList selectedRows = tabLinks->selectionModel()->selectedRows();
  mLinksModel->removeRow( selectedRows[0].row() );
}

void QgsMetadataWizard::fillComboBox()
{
  // Set default values in type combobox
  // It is advised to use the ISO 19115 MD_ScopeCode values. E.g. 'dataset' or 'series'.
  // http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml
  comboType->setEditable( true );
  QStringList types;
  types << "" << "attribute" << "attributeType" << "dataset" << "nonGeographicDataset" << "series" << "document";
  comboType->addItems( types );

  // Set default values in language combobox
  // It is advised to use the ISO 639.2 or ISO 3166 specifications, e.g. 'ENG' or 'SPA',
  comboLanguage->setEditable( true );
  types.clear();
  comboLanguage->addItems( parseLanguages() );
}

void QgsMetadataWizard::setPropertiesFromLayer()
{
  // Set all properties USING THE OLD API
  layerLabel->setText( mLayer->name() );
  lineEditTitle->setText( mLayer->name() );
  textEditAbstract->setText( mLayer->abstract() );

  // Set all properties USING THE NEW API
  // It will overwrite existing settings

  // Identifier
  if ( ! mMetadata.identifier().isEmpty() )
  {
    lineEditIdentifier->setText( mMetadata.identifier() );
  }

  // Title
  if ( ! mMetadata.title().isEmpty() )
  {
    lineEditTitle->setText( mMetadata.title() );
  }

  // Type
  if ( ! mMetadata.type().isEmpty() )
  {
    if ( comboType->findText( mMetadata.type() ) == -1 )
    {
      comboType->addItem( mMetadata.type() );
    }
    comboType->setCurrentIndex( comboType->findText( mMetadata.type() ) );
  }

  // Language
  if ( ! mMetadata.language().isEmpty() )
  {
    if ( comboLanguage->findText( mMetadata.language() ) == -1 )
    {
      comboLanguage->addItem( mMetadata.language() );
    }
    comboLanguage->setCurrentIndex( comboLanguage->findText( mMetadata.language() ) );
  }

  // Keywords
  tabKeywords->setRowCount( 0 );
  QMapIterator<QString, QStringList> i( mMetadata.keywords() );
  while ( i.hasNext() )
  {
    i.next();
    addVocabulary();
    int currentRow = tabKeywords->rowCount() - 1;
    tabKeywords->item( currentRow, 0 )->setText( i.key() );
    tabKeywords->item( currentRow, 1 )->setText( i.value().join( "," ) );
  }

  // Licenses
  tabLicenses->setRowCount( 0 );
  for ( QString licence : mMetadata.licenses() )
  {
    int currentRow = tabLicenses->rowCount();
    tabLicenses->setRowCount( currentRow + 1 );
    QTableWidgetItem *pCell = tabLicenses->item( currentRow, 0 );
    if ( !pCell )
    {
      pCell = new QTableWidgetItem;
      tabLicenses->setItem( currentRow, 0, pCell );
    }
    pCell->setText( licence );
  }

  // CRS
  if ( mMetadata.crs().isValid() )
  {
    selectionCrs->setCrs( mMetadata.crs() );
  }

  // Links
  for ( QgsLayerMetadata::Link link : mMetadata.links() )
  {
    int row = mLinksModel->rowCount();
    mLinksModel->setItem( row, 0, new QStandardItem( link.name ) );
    mLinksModel->setItem( row, 1, new QStandardItem( link.type ) );
    mLinksModel->setItem( row, 2, new QStandardItem( link.url ) );
    mLinksModel->setItem( row, 3, new QStandardItem( link.description ) );
    mLinksModel->setItem( row, 4, new QStandardItem( link.format ) );
    mLinksModel->setItem( row, 5, new QStandardItem( link.mimeType ) );
    mLinksModel->setItem( row, 6, new QStandardItem( link.size ) );
  }
}

void QgsMetadataWizard::saveMetadata( QgsLayerMetadata &layerMetadata )
{
  layerMetadata.setIdentifier( lineEditIdentifier->text() );
  layerMetadata.setTitle( lineEditTitle->text() );
  layerMetadata.setType( comboType->currentText() );
  layerMetadata.setLanguage( comboLanguage->currentText() );
  layerMetadata.setAbstract( textEditAbstract->toPlainText() );

  // Keywords
  QMap<QString, QStringList> keywords;
  for ( int i = 0 ; i < tabKeywords->rowCount() ; i++ )
  {
    keywords.insert( tabKeywords->item( i, 0 )->text(), tabKeywords->item( i, 1 )->text().split( "," ) );
  }
  layerMetadata.setKeywords( keywords );

  // Licenses
  QStringList licenses;
  for ( int i = 0 ; i < tabLicenses->rowCount() ; i++ )
  {
    licenses.append( tabLicenses->item( i, 0 )->text() );
  }
  layerMetadata.setLicenses( licenses );

  // CRS
  if ( selectionCrs->crs().isValid() )
  {
    layerMetadata.setCrs( selectionCrs->crs() );
  }

  // Links
  QList<QgsLayerMetadata::Link> links;
  for ( int row = 0 ; row < mLinksModel->rowCount() ; row++ )
  {
    struct QgsLayerMetadata::Link link = QgsLayerMetadata::Link();
    link.name = mLinksModel->item( row, 0 )->text();
    link.type = mLinksModel->item( row, 1 )->text();
    link.url = mLinksModel->item( row, 2 )->text();
    link.description = mLinksModel->item( row, 3 )->text();
    link.format = mLinksModel->item( row, 4 )->text();
    link.mimeType = mLinksModel->item( row, 5 )->text();
    link.size = mLinksModel->item( row, 6 )->text();
    links.append( link );
  }
  layerMetadata.setLinks( links );
}

bool QgsMetadataWizard::checkMetadata()
{
  QgsLayerMetadata metadata = QgsLayerMetadata();
  saveMetadata( metadata );
  QgsNativeMetadataValidator validator;
  QList<QgsMetadataValidator::ValidationResult> validationResults;
  bool results = validator.validate( metadata, validationResults );

  QString errors;
  if ( results == false )
  {
    errors = QStringLiteral();
    for ( QgsMetadataValidator::ValidationResult result : validationResults )
    {
      errors += QLatin1String( "<b>" ) % result.section;
      if ( ! result.identifier.isNull() )
      {
        errors += QLatin1String( " " ) % QVariant( result.identifier.toInt() + 1 ).toString();
      }
      errors += QLatin1String( "</b>: " ) % result.note % QLatin1String( "<br />" );
    }
  }
  else
  {
    errors = QStringLiteral( "Ok, it seems valid." );
  }

  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );
  resultsCheckMetadata->clear();
  resultsCheckMetadata->document()->setDefaultStyleSheet( myStyle );
  resultsCheckMetadata->setHtml( errors );

  return results;
}

QStringList QgsMetadataWizard::parseLanguages()
{
  QString path = QDir( QgsApplication::metadataPath() ).absoluteFilePath( QString( "country_code_ISO_3166.csv" ) );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QString( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
  }

  QStringList wordList;
  wordList.append( "" );
  // Skip the first line of the CSV
  file.readLine();
  while ( !file.atEnd() )
  {
    QByteArray line = file.readLine();
    wordList.append( line.split( ',' ).at( 2 ) );
  }
  return wordList;
}

QStringList QgsMetadataWizard::parseLicenses()
{
  QString path = QDir( QgsApplication::metadataPath() ).absoluteFilePath( QString( "licenses.csv" ) );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QString( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
  }

  QStringList wordList;
  // Skip the first line of the CSV
  file.readLine();
  while ( !file.atEnd() )
  {
    QByteArray line = file.readLine();
    wordList.append( line.split( ',' ).at( 0 ) );
  }
  return wordList;
}

QStringList QgsMetadataWizard::parseLinkTypes()
{
  QString path = QDir( QgsApplication::metadataPath() ).absoluteFilePath( QString( "LinkPropertyLookupTable.csv" ) );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QString( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
  }

  QStringList wordList;
  // Skip the first line of the CSV
  file.readLine();
  while ( !file.atEnd() )
  {
    QByteArray line = file.readLine();
    wordList.append( line.split( ',' ).at( 0 ) );
  }
  return wordList;
}

void QgsMetadataWizard::cancelClicked()
{
  hide();
}

void QgsMetadataWizard::backClicked()
{
  int index = tabWidget->currentIndex();
  if ( index > 0 )
    tabWidget->setCurrentIndex( index - 1 );
  updatePanel();
}

void QgsMetadataWizard::nextClicked()
{
  int index = tabWidget->currentIndex();
  if ( index < tabWidget->count() )
    tabWidget->setCurrentIndex( index + 1 );
  updatePanel();
}

void QgsMetadataWizard::finishedClicked()
{
  // OLD API (to remove later)
  mLayer->setName( lineEditTitle->text() );
  mLayer->setAbstract( textEditAbstract->toPlainText() );

  // New Metadata API
  saveMetadata( mMetadata );

  // Save layer metadata properties
  mLayer->setMetadata( mMetadata );

  QgsNativeMetadataValidator validator;
  QList<QgsMetadataValidator::ValidationResult> validationResults;
  bool results = validator.validate( mMetadata, validationResults );

  hide();

  if ( results )
  {
    QgisApp::instance()->messageBar()->pushInfo( tr( "Save metadata" ), tr( "Saving metadata successfull into the project" ) );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Save metadata" ), tr( "Saving metadata successfull but some fields were missing" ) );
  }
}

void QgsMetadataWizard::updatePanel()
{
  int index = tabWidget->currentIndex();
  if ( index == 0 )
  {
    backButton->setEnabled( false );
    nextButton->setEnabled( true );
  }
  else if ( index == tabWidget->count() - 1 )
  {
    backButton->setEnabled( true );
    nextButton->setEnabled( false );
  }
  else
  {
    backButton->setEnabled( true );
    nextButton->setEnabled( true );
  }
}

QWidget *LinkItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( index.column() == 1 )
  {
    QComboBox *typeEditor = new QComboBox( parent );
    typeEditor->setEditable( true );
    QStringListModel *model = new QStringListModel( parent );
    model->setStringList( QgsMetadataWizard::parseLinkTypes() );
    typeEditor->setModel( model );
    return typeEditor;
  }
  else if ( index.column() == 5 )
  {
    // See https://fr.wikipedia.org/wiki/Type_MIME
    QComboBox *mimeEditor = new QComboBox( parent );
    mimeEditor->setEditable( true );
    QStringListModel *model = new QStringListModel( parent );
    QStringList mime;
    mime << "" << "image/png" << "image/tiff" << "application/pdf";
    model->setStringList( mime );
    mimeEditor->setModel( model );
    return mimeEditor;
  }

  return QStyledItemDelegate::createEditor( parent, option, index );
}
