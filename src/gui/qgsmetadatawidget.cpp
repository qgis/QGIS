/***************************************************************************
                          qgsmetadatawidget.h  -  description
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
#include <QStringListModel>

#include "qgsmetadatawidget.h"
#include "qgslogger.h"
#include "qgslayermetadatavalidator.h"
#include "qgsapplication.h"

QgsMetadataWidget::QgsMetadataWidget( QWidget *parent, QgsMapLayer *layer )
  : QWidget( parent ),
    mLayer( layer )
{
  setupUi( this );
  mMetadata = layer->metadata();
  tabWidget->setCurrentIndex( 0 );

  // Disable the encoding and contacts
  encodingFrame->setHidden( true );
  tabWidget->removeTab( 5 );

  // Default categories, we want them translated, so we are not using a CSV.
  mDefaultCategories << tr( "Farming" ) << tr( "Climatology Meteorology Atmosphere" ) << tr( "Location" ) << tr( "Intelligence Military" ) << tr( "Transportation" ) << tr( "Structure" ) << tr( "Boundaries" );
  mDefaultCategories << tr( "Inland Waters" ) << tr( "Planning Cadastre" ) << tr( "Geoscientific Information" ) << tr( "Elevation" ) << tr( "Health" ) << tr( "Biota" ) << tr( "Oceans" ) << tr( "Environment" );
  mDefaultCategories << tr( "Utilities Communication" ) << tr( "Economy" ) << tr( "Society" ) << tr( "Imagery Base Maps Earth Cover" );
  mDefaultCategoriesModel = new QStringListModel( mDefaultCategories );
  mDefaultCategoriesModel->sort( 0 );  // Sorting using translations
  listDefaultCategories->setModel( mDefaultCategoriesModel );

  // Categories
  mCategoriesModel = new QStringListModel( listCategories );
  listCategories->setModel( mCategoriesModel );

  // Rights
  mRightsModel = new QStringListModel( listRights );
  listRights->setModel( mRightsModel );

  // Setup the constraints view
  mConstraintsModel = new QStandardItemModel( tabConstraints );
  mConstraintsModel->setColumnCount( 2 );
  QStringList constraintheaders;
  constraintheaders << tr( "Type" ) << tr( "Constraint" );
  mConstraintsModel->setHorizontalHeaderLabels( constraintheaders );
  tabConstraints->setModel( mConstraintsModel );
  tabConstraints->setItemDelegate( new ConstraintItemDelegate() );

  // Setup the link view
  mLinksModel = new QStandardItemModel( tabLinks );
  mLinksModel->setColumnCount( 7 );
  QStringList headers = QStringList();
  headers << tr( "Name" ) << tr( "Type" ) << tr( "URL" ) << tr( "Description" ) << tr( "Format" ) << tr( "MIME" ) << tr( "Size" );
  mLinksModel->setHorizontalHeaderLabels( headers );
  tabLinks->setModel( mLinksModel );
  tabLinks->setItemDelegate( new LinkItemDelegate() );

  // History
  mHistoryModel = new QStringListModel( listHistory );
  listHistory->setModel( mHistoryModel );

  // Connect signals and slots
  connect( tabWidget, &QTabWidget::currentChanged, this, &QgsMetadataWidget::updatePanel );
  connect( btnAutoSource, &QPushButton::clicked, this, &QgsMetadataWidget::fillSourceFromLayer );
  connect( btnAddVocabulary, &QPushButton::clicked, this, &QgsMetadataWidget::addVocabulary );
  connect( btnRemoveVocabulary, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedVocabulary );
  connect( btnAddRight, &QPushButton::clicked, this, &QgsMetadataWidget::addRight );
  connect( btnRemoveRight, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedRight );
  connect( btnAddLicence, &QPushButton::clicked, this, &QgsMetadataWidget::addLicence );
  connect( btnRemoveLicence, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedLicence );
  connect( btnAddConstraint, &QPushButton::clicked, this, &QgsMetadataWidget::addConstraint );
  connect( btnRemoveConstraint, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedConstraint );
  connect( btnAutoCrs, &QPushButton::clicked, this, &QgsMetadataWidget::fillCrsFromLayer );
  connect( btnAddContact, &QPushButton::clicked, this, &QgsMetadataWidget::addContact );
  connect( btnRemoveContact, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedContact );
  connect( tabContacts, &QTableWidget::itemSelectionChanged, this, &QgsMetadataWidget::updateContactDetails );
  connect( btnAddLink, &QPushButton::clicked, this, &QgsMetadataWidget::addLink );
  connect( btnRemoveLink, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedLink );
  connect( btnAddHistory, &QPushButton::clicked, this, &QgsMetadataWidget::addHistory );
  connect( btnRemoveHistory, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedHistory );
  connect( btnNewCategory, &QPushButton::clicked, this, &QgsMetadataWidget::addNewCategory );
  connect( btnAddDefaultCategory, &QPushButton::clicked, this, &QgsMetadataWidget::addDefaultCategory );
  connect( btnRemoveCategory, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedCategory );

  fillComboBox();
  setPropertiesFromLayer();
  updateContactDetails();
}

void QgsMetadataWidget::fillSourceFromLayer() const
{
  lineEditIdentifier->setText( mLayer->publicSource() );
}

void QgsMetadataWidget::addVocabulary() const
{
  int row = tabKeywords->rowCount();
  tabKeywords->setRowCount( row + 1 );
  QTableWidgetItem *pCell = nullptr;

  // Vocabulary
  pCell = new QTableWidgetItem( QString( tr( "undefined %1" ) ).arg( row + 1 ) );
  tabKeywords->setItem( row, 0, pCell );

  // Keywords
  pCell = new QTableWidgetItem();
  tabKeywords->setItem( row, 1, pCell );
}

void QgsMetadataWidget::removeSelectedVocabulary() const
{
  QItemSelectionModel *selectionModel = tabKeywords->selectionModel();
  const QModelIndexList selectedRows = selectionModel->selectedRows();
  for ( int i = 0; i < selectedRows.size() ; i++ )
  {
    tabKeywords->model()->removeRow( selectedRows[i].row() );
  }
}

void QgsMetadataWidget::addLicence()
{
  QString newLicence = QInputDialog::getItem( this, tr( "New Licence" ), tr( "New Licence" ), parseLicenses(), 0, true );
  if ( tabLicenses->findItems( newLicence, Qt::MatchExactly ).isEmpty() )
  {
    int row = tabLicenses->rowCount();
    tabLicenses->setRowCount( row + 1 );
    QTableWidgetItem *pCell = new QTableWidgetItem( newLicence );
    tabLicenses->setItem( row, 0, pCell );
  }
}

void QgsMetadataWidget::removeSelectedLicence() const
{
  QItemSelectionModel *selectionModel = tabLicenses->selectionModel();
  const QModelIndexList selectedRows = selectionModel->selectedRows();
  for ( int i = 0; i < selectedRows.size() ; i++ )
  {
    tabLicenses->model()->removeRow( selectedRows[i].row() );
  }
}

void QgsMetadataWidget::addRight()
{
  QString newRight = QInputDialog::getText( this, tr( "New Right" ), tr( "New Right" ) );
  QStringList existingRights = mRightsModel->stringList();
  if ( ! existingRights.contains( newRight ) )
  {
    existingRights.append( newRight );
    mRightsModel->setStringList( existingRights );
  }
}

void QgsMetadataWidget::removeSelectedRight() const
{
  QItemSelectionModel *selection = listRights->selectionModel();
  if ( selection->hasSelection() )
  {
    QModelIndex indexElementSelectionne = selection->currentIndex();

    QVariant item = mRightsModel->data( indexElementSelectionne, Qt::DisplayRole );
    QStringList list = mRightsModel->stringList();
    list.removeOne( item.toString() );
    mRightsModel->setStringList( list );
  }
}

void QgsMetadataWidget::addConstraint() const
{
  int row = mConstraintsModel->rowCount();
  mConstraintsModel->setItem( row, 0, new QStandardItem( QString( tr( "undefined %1" ) ).arg( row + 1 ) ) );
  mConstraintsModel->setItem( row, 1, new QStandardItem( QString( tr( "undefined %1" ) ).arg( row + 1 ) ) );
}

void QgsMetadataWidget::removeSelectedConstraint() const
{
  const QModelIndexList selectedRows = tabConstraints->selectionModel()->selectedRows();
  mConstraintsModel->removeRow( selectedRows[0].row() );
}

void QgsMetadataWidget::fillCrsFromLayer() const
{
  selectionCrs->setCrs( mLayer->crs() );
}

void QgsMetadataWidget::addContact() const
{
  int row = tabContacts->rowCount();
  tabContacts->setRowCount( row + 1 );
  QTableWidgetItem *pCell = nullptr;

  // Name
  pCell = new QTableWidgetItem( QString( tr( "unnamed %1" ) ).arg( row + 1 ) );
  tabContacts->setItem( row, 0, pCell );

  // Organization
  pCell = new QTableWidgetItem();
  tabContacts->setItem( row, 1, pCell );

  // Set last item selected
  tabContacts->selectRow( row );
}

void QgsMetadataWidget::removeSelectedContact() const
{
  QItemSelectionModel *selectionModel = tabContacts->selectionModel();
  const QModelIndexList selectedRows = selectionModel->selectedRows();
  for ( int i = 0; i < selectedRows.size() ; i++ )
  {
    tabContacts->model()->removeRow( selectedRows[i].row() );
  }
}

void QgsMetadataWidget::updateContactDetails() const
{
  QItemSelectionModel *selectionModel = tabContacts->selectionModel();
  const QModelIndexList selectedRows = selectionModel->selectedRows();

  if ( ! selectedRows.isEmpty() )
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

void QgsMetadataWidget::addLink() const
{
  int row = mLinksModel->rowCount();
  mLinksModel->setItem( row, 0, new QStandardItem( QString( tr( "undefined %1" ) ).arg( row + 1 ) ) );
  mLinksModel->setItem( row, 1, new QStandardItem() );
  mLinksModel->setItem( row, 2, new QStandardItem() );
  mLinksModel->setItem( row, 3, new QStandardItem() );
  mLinksModel->setItem( row, 4, new QStandardItem() );
  mLinksModel->setItem( row, 5, new QStandardItem() );
  mLinksModel->setItem( row, 6, new QStandardItem() );
}

void QgsMetadataWidget::removeSelectedLink() const
{
  const QModelIndexList selectedRows = tabLinks->selectionModel()->selectedRows();
  mLinksModel->removeRow( selectedRows[0].row() );
}

void QgsMetadataWidget::addHistory()
{
  QString newHistory = QInputDialog::getText( this, tr( "New History" ), tr( "New History" ) );
  QStringList existingHistory = mHistoryModel->stringList();
  if ( ! existingHistory.contains( newHistory ) )
  {
    existingHistory.append( newHistory );
    mHistoryModel->setStringList( existingHistory );
  }
}

void QgsMetadataWidget::removeSelectedHistory() const
{
  QItemSelectionModel *selection = listHistory->selectionModel();
  if ( selection->hasSelection() )
  {
    QModelIndex indexElementSelectionne = selection->currentIndex();

    QVariant item = mHistoryModel->data( indexElementSelectionne, Qt::DisplayRole );
    QStringList list = mHistoryModel->stringList();
    list.removeOne( item.toString() );
    mHistoryModel->setStringList( list );
  }
}

void QgsMetadataWidget::fillComboBox() const
{
  // Set default values in type combobox
  // It is advised to use the ISO 19115 MD_ScopeCode values. E.g. 'dataset' or 'series'.
  // http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml
  comboType->setEditable( true );
  comboType->clear();
  QMap<QString, QString> types = parseTypes();
  const QStringList &keys = types.keys();
  int i = 0;
  for ( const QString &type : keys )
  {
    comboType->insertItem( i, type );
    comboType->setItemData( i, types.value( type ), Qt::ToolTipRole );
    i++;
  }

  // Set default values in language combobox
  // It is advised to use the ISO 639.2 or ISO 3166 specifications, e.g. 'ENG' or 'SPA',
  comboLanguage->setEditable( true );
  comboLanguage->clear();
  QMap<QString, QString> countries = parseLanguages();
  const QStringList &k = countries.keys();
  i = 0;
  for ( const QString &countryCode : k )
  {
    comboLanguage->insertItem( i, countryCode );
    comboLanguage->setItemData( i, countries.value( countryCode ), Qt::ToolTipRole );
    i++;
  }
}

void QgsMetadataWidget::setPropertiesFromLayer() const
{
  // Parent ID
  lineEditParentId->setText( mMetadata.parentIdentifier() );

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

  // Abstract
  textEditAbstract->setPlainText( mMetadata.abstract() );

  // Categories
  mCategoriesModel->setStringList( mMetadata.categories() );

  // Keywords
  tabKeywords->setRowCount( 0 );
  QMapIterator<QString, QStringList> i( mMetadata.keywords() );
  while ( i.hasNext() )
  {
    i.next();
    addVocabulary();
    int currentRow = tabKeywords->rowCount() - 1;
    tabKeywords->item( currentRow, 0 )->setText( i.key() );
    tabKeywords->item( currentRow, 1 )->setText( i.value().join( QStringLiteral( "," ) ) );
  }

  // Fees
  lineEditFees->setText( mMetadata.fees() );

  // Licenses
  tabLicenses->setRowCount( 0 );
  const QStringList &licenses = mMetadata.licenses();
  for ( const QString &licence : licenses )
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

  // Rights
  mRightsModel->setStringList( mMetadata.rights() );

  // Constraints
  const QList<QgsLayerMetadata::Constraint> &constraints = mMetadata.constraints();
  for ( const QgsLayerMetadata::Constraint &constraint : constraints )
  {
    int row = mConstraintsModel->rowCount();
    mConstraintsModel->setItem( row, 0, new QStandardItem( constraint.type ) );
    mConstraintsModel->setItem( row, 1, new QStandardItem( constraint.constraint ) );
  }

  // CRS
  if ( mMetadata.crs().isValid() )
  {
    selectionCrs->setCrs( mMetadata.crs() );
  }

  // Links
  const QList<QgsLayerMetadata::Link> &links = mMetadata.links();
  for ( const QgsLayerMetadata::Link &link : links )
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

  // History
  mHistoryModel->setStringList( mMetadata.history() );
}

void QgsMetadataWidget::saveMetadata( QgsLayerMetadata &layerMetadata ) const
{
  layerMetadata.setParentIdentifier( lineEditParentId->text() );
  layerMetadata.setIdentifier( lineEditIdentifier->text() );
  layerMetadata.setTitle( lineEditTitle->text() );
  layerMetadata.setType( comboType->currentText() );
  layerMetadata.setLanguage( comboLanguage->currentText() );
  layerMetadata.setAbstract( textEditAbstract->toPlainText() );

  // Keywords, it will save categories too.
  syncFromCategoriesTabToKeywordsTab();
  QMap<QString, QStringList> keywords;
  for ( int i = 0; i < tabKeywords->rowCount() ; i++ )
  {
    keywords.insert( tabKeywords->item( i, 0 )->text(), tabKeywords->item( i, 1 )->text().split( ',' ) );
  }
  layerMetadata.setKeywords( keywords );

  // Fees
  layerMetadata.setFees( lineEditFees->text() );

  // Licenses
  QStringList licenses;
  for ( int i = 0; i < tabLicenses->rowCount() ; i++ )
  {
    licenses.append( tabLicenses->item( i, 0 )->text() );
  }
  layerMetadata.setLicenses( licenses );

  // Rights
  layerMetadata.setRights( mRightsModel->stringList() );

  // Constraints
  QList<QgsLayerMetadata::Constraint> constraints;
  for ( int row = 0; row < mConstraintsModel->rowCount() ; row++ )
  {
    struct QgsLayerMetadata::Constraint constraint = QgsLayerMetadata::Constraint();
    constraint.type = mConstraintsModel->item( row, 0 )->text();
    constraint.constraint = mConstraintsModel->item( row, 1 )->text();
    constraints.append( constraint );
  }
  layerMetadata.setConstraints( constraints );

  // CRS
  layerMetadata.setCrs( selectionCrs->crs() );

  // Links
  QList<QgsLayerMetadata::Link> links;
  for ( int row = 0; row < mLinksModel->rowCount() ; row++ )
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

  // History
  layerMetadata.setHistory( mHistoryModel->stringList() );
}

bool QgsMetadataWidget::checkMetadata() const
{
  QgsLayerMetadata metadata = QgsLayerMetadata();
  saveMetadata( metadata );
  QgsNativeMetadataValidator validator;
  QList<QgsMetadataValidator::ValidationResult> validationResults;
  bool results = validator.validate( metadata, validationResults );

  QString errors;
  if ( !results )
  {
    for ( const QgsMetadataValidator::ValidationResult &result : qgis::as_const( validationResults ) )
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
    errors = QString( tr( "Ok, it seems valid according to the QGIS Schema." ) );
  }

  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );
  resultsCheckMetadata->clear();
  resultsCheckMetadata->document()->setDefaultStyleSheet( myStyle );
  resultsCheckMetadata->setHtml( errors );

  return results;
}

QMap<QString, QString> QgsMetadataWidget::parseLanguages()
{
  QMap<QString, QString> countries;
  countries.insert( QString(), QString() ); // We add an empty line, because it's not compulsory.

  QString path = QDir( QgsApplication::metadataPath() ).absoluteFilePath( QStringLiteral( "language_codes_ISO_639.csv" ) );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QString( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
    return countries;
  }

  // Skip the first line of the CSV
  file.readLine();
  while ( !file.atEnd() )
  {
    QByteArray line = file.readLine();
    QList<QByteArray> items = line.split( ',' );
    countries.insert( QString( items.at( 0 ).constData() ).trimmed(), QString( items.at( 1 ).constData() ).trimmed() );
  }
  file.close();

  path = QDir( QgsApplication::metadataPath() ).absoluteFilePath( QStringLiteral( "country_code_ISO_3166.csv" ) );
  QFile secondFile( path );
  if ( !secondFile.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QString( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
    return countries;
  }

  // Skip the first line of the CSV
  secondFile.readLine();
  while ( !secondFile.atEnd() )
  {
    QByteArray line = secondFile.readLine();
    QList<QByteArray> items = line.split( ',' );
    countries.insert( QString( items.at( 2 ).constData() ).trimmed(), QString( items.at( 0 ).constData() ).trimmed() );
  }
  secondFile.close();
  return countries;
}

QStringList QgsMetadataWidget::parseLicenses()
{
  QStringList wordList;
  wordList.append( QString() ); // We add an empty line, because it's not compulsory.

  QString path = QDir( QgsApplication::metadataPath() ).absoluteFilePath( QStringLiteral( "licenses.csv" ) );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QString( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
    return wordList;
  }

  // Skip the first line of the CSV
  file.readLine();
  while ( !file.atEnd() )
  {
    QByteArray line = file.readLine();
    wordList.append( line.split( ',' ).at( 0 ).trimmed() );
  }
  file.close();
  return wordList;
}

QStringList QgsMetadataWidget::parseLinkTypes()
{
  QStringList wordList;
  wordList.append( QString() ); // We add an empty line, because it's not compulsory.

  QString path = QDir( QgsApplication::metadataPath() ).absoluteFilePath( QStringLiteral( "LinkPropertyLookupTable.csv" ) );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QString( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
    return wordList;
  }

  // Skip the first line of the CSV
  file.readLine();
  while ( !file.atEnd() )
  {
    QByteArray line = file.readLine();
    wordList.append( line.split( ',' ).at( 0 ).trimmed() );
  }
  file.close();
  return wordList;
}

QStringList QgsMetadataWidget::parseMimeTypes()
{
  QStringList wordList;
  wordList.append( QString() ); // We add an empty line, because it's not compulsory.

  QString path = QDir( QgsApplication::metadataPath() ).absoluteFilePath( QStringLiteral( "mime.csv" ) );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QString( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
    return wordList;
  }

  while ( !file.atEnd() )
  {
    QByteArray line = file.readLine();
    wordList.append( line.split( ',' ).at( 0 ).trimmed() );
  }
  file.close();
  return wordList;
}

QMap<QString, QString> QgsMetadataWidget::parseTypes()
{
  QMap<QString, QString> types;
  types.insert( QString(), QString() ); // We add an empty line, because it's not compulsory.
  QString path = QDir( QgsApplication::metadataPath() ).absoluteFilePath( QStringLiteral( "md_scope_codes.csv" ) );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( QString( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
    return types;
  }

  types.insert( QString(), QString() ); // We add an empty line, because it's not compulsory.
  while ( !file.atEnd() )
  {
    QByteArray line = file.readLine();
    QList<QByteArray> items = line.split( ';' );
    types.insert( items.at( 0 ).constData(), items.at( 1 ).constData() );
  }
  file.close();
  return types;
}

void QgsMetadataWidget::acceptMetadata()
{
  saveMetadata( mMetadata );

  // Save layer metadata properties
  mLayer->setMetadata( mMetadata );
}

void QgsMetadataWidget::syncFromCategoriesTabToKeywordsTab() const
{
  if ( mCategoriesModel->rowCount() > 0 )
  {
    QList<QTableWidgetItem *> categories = tabKeywords->findItems( QStringLiteral( "gmd:topicCategory" ), Qt::MatchExactly );
    int row;
    if ( !categories.isEmpty() )
    {
      row = categories.at( 0 )->row();
    }
    else
    {
      // Create a new line with 'gmd:topicCategory'
      addVocabulary();
      row = tabKeywords->rowCount() - 1;
      tabKeywords->item( row, 0 )->setText( QStringLiteral( "gmd:topicCategory" ) );
    }
    tabKeywords->item( row, 1 )->setText( mCategoriesModel->stringList().join( QStringLiteral( "," ) ) );
  }
}

void QgsMetadataWidget::updatePanel() const
{
  int index = tabWidget->currentIndex();
  QString currentTabText = tabWidget->widget( index )->objectName();
  if ( currentTabText == QStringLiteral( "tabCategoriesDialog" ) )
  {
    // Categories tab
    // We need to take keywords and insert them into the list
    QList<QTableWidgetItem *> categories = tabKeywords->findItems( QStringLiteral( "gmd:topicCategory" ), Qt::MatchExactly );
    if ( !categories.isEmpty() )
    {
      int row = categories.at( 0 )->row();
      mCategoriesModel->setStringList( tabKeywords->item( row, 1 )->text().split( QStringLiteral( "," ) ) );
    }
    else
    {
      mCategoriesModel->setStringList( QStringList() );
    }
  }
  else if ( currentTabText == QStringLiteral( "tabKeywordsDialog" ) )
  {
    // Keywords tab
    // We need to take categories and insert them into the table
    syncFromCategoriesTabToKeywordsTab();
  }
  else if ( currentTabText == QStringLiteral( "tabValidationDialog" ) )
  {
    checkMetadata();
  }
}

void QgsMetadataWidget::addNewCategory()
{
  bool ok;
  QString text = QInputDialog::getText( this, tr( "New Category" ),
                                        tr( "New Category:" ), QLineEdit::Normal,
                                        QString(), &ok );
  if ( ok && !text.isEmpty() )
  {
    QStringList list = mCategoriesModel->stringList();
    if ( ! list.contains( text ) )
    {
      list.append( text );
      mCategoriesModel->setStringList( list );
      mCategoriesModel->sort( 0 );
    }
  }
}

void QgsMetadataWidget::addDefaultCategory() const
{
  QItemSelectionModel *selection = listDefaultCategories->selectionModel();
  if ( selection->hasSelection() )
  {
    QModelIndex indexElementSelectionne = selection->currentIndex();

    QVariant item = mDefaultCategoriesModel->data( indexElementSelectionne, Qt::DisplayRole );
    QStringList list = mDefaultCategoriesModel->stringList();
    list.removeOne( item.toString() );
    mDefaultCategoriesModel->setStringList( list );

    list = mCategoriesModel->stringList();
    list.append( item.toString() );
    mCategoriesModel->setStringList( list );
    mCategoriesModel->sort( 0 );
  }
}


void QgsMetadataWidget::removeSelectedCategory() const
{
  QItemSelectionModel *selection = listCategories->selectionModel();
  if ( selection->hasSelection() )
  {
    QModelIndex indexElementSelectionne = listCategories->selectionModel()->currentIndex();

    QVariant item = mCategoriesModel->data( indexElementSelectionne, Qt::DisplayRole );
    QStringList list = mCategoriesModel->stringList();
    list.removeOne( item.toString() );
    mCategoriesModel->setStringList( list );

    if ( mDefaultCategories.contains( item.toString() ) )
    {
      list = mDefaultCategoriesModel->stringList();
      list.append( item.toString() );
      mDefaultCategoriesModel->setStringList( list );
      mDefaultCategoriesModel->sort( 0 );
    }
  }
}

QWidget *LinkItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( index.column() == 1 )
  {
    // Link type
    QComboBox *typeEditor = new QComboBox( parent );
    typeEditor->setEditable( true );
    QStringListModel *model = new QStringListModel( parent );
    model->setStringList( QgsMetadataWidget::parseLinkTypes() );
    typeEditor->setModel( model );
    return typeEditor;
  }
  else if ( index.column() == 5 )
  {
    // MIME
    QComboBox *mimeEditor = new QComboBox( parent );
    mimeEditor->setEditable( true );
    QStringListModel *model = new QStringListModel( parent );
    model->setStringList( QgsMetadataWidget::parseMimeTypes() );
    mimeEditor->setModel( model );
    return mimeEditor;
  }

  return QStyledItemDelegate::createEditor( parent, option, index );
}

QWidget *ConstraintItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( index.column() == 0 )
  {
    // Constraint type
    QComboBox *typeEditor = new QComboBox( parent );
    typeEditor->setEditable( true );
    QStringList types;
    types << QStringLiteral( "access" ) << QStringLiteral( "use" ) << QStringLiteral( "other" );
    QStringListModel *model = new QStringListModel( parent );
    model->setStringList( types );
    typeEditor->setModel( model );
    return typeEditor;
  }

  return QStyledItemDelegate::createEditor( parent, option, index );
}
