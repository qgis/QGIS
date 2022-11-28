/***************************************************************************
                          QgsAbstractMetadataBasewidget.h  -  description
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

#include <QIcon>
#include <QPushButton>
#include <QComboBox>
#include <QString>
#include <QInputDialog>
#include <QStringListModel>

#include "qgsbox3d.h"
#include "qgsmetadatawidget.h"
#include "qgslogger.h"
#include "qgslayermetadatavalidator.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsprojectmetadata.h"
#include "qgsproject.h"

QgsMetadataWidget::QgsMetadataWidget( QWidget *parent, QgsMapLayer *layer )
  : QWidget( parent ),
    mLayer( layer )
{
  setupUi( this );
  tabWidget->setCurrentIndex( 0 );

  // Disable the encoding
  encodingFrame->setHidden( true );

  // Default categories, we want them translated, so we are not using a CSV.
  mDefaultCategories << tr( "Farming" ) << tr( "Climatology Meteorology Atmosphere" ) << tr( "Location" ) << tr( "Intelligence Military" ) << tr( "Transportation" ) << tr( "Structure" ) << tr( "Boundaries" );
  mDefaultCategories << tr( "Inland Waters" ) << tr( "Planning Cadastre" ) << tr( "Geoscientific Information" ) << tr( "Elevation" ) << tr( "Health" ) << tr( "Biota" ) << tr( "Oceans" ) << tr( "Environment" );
  mDefaultCategories << tr( "Utilities Communication" ) << tr( "Economy" ) << tr( "Society" ) << tr( "Imagery Base Maps Earth Cover" );
  mDefaultCategoriesModel = new QStringListModel( mDefaultCategories, this );
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
  tabConstraints->setItemDelegate( new ConstraintItemDelegate( this ) );

  // Extent
  dateTimeFrom->setAllowNull( true );
  dateTimeTo->setAllowNull( true );

  // Setup the link view
  mLinksModel = new QStandardItemModel( tabLinks );
  mLinksModel->setColumnCount( 7 );
  QStringList headers = QStringList();
  headers << tr( "Name" ) << tr( "Type" ) << tr( "URL" ) << tr( "Description" ) << tr( "Format" ) << tr( "MIME" ) << tr( "Size" );
  mLinksModel->setHorizontalHeaderLabels( headers );
  tabLinks->setModel( mLinksModel );
  tabLinks->setItemDelegate( new LinkItemDelegate( this ) );

  // History
  mHistoryModel = new QStringListModel( listHistory );
  listHistory->setModel( mHistoryModel );

  for ( QgsDateTimeEdit *w :
        {
          mCreationDateTimeEdit,
          mCreationDateTimeEdit2,
          mPublishedDateTimeEdit,
          mRevisedDateTimeEdit,
          mSupersededDateTimeEdit
        } )
  {
    w->setAllowNull( true );
    w->setNullRepresentation( tr( "Not set" ) );
  }

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
  connect( btnSetCrsFromLayer, &QPushButton::clicked, this, &QgsMetadataWidget::fillCrsFromLayer );
  connect( btnSetCrsFromProvider, &QPushButton::clicked, this, &QgsMetadataWidget::fillCrsFromProvider );
  connect( btnAddAddress, &QPushButton::clicked, this, &QgsMetadataWidget::addAddress );
  connect( btnRemoveAddress, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedAddress );
  connect( btnAddLink, &QPushButton::clicked, this, &QgsMetadataWidget::addLink );
  connect( btnRemoveLink, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedLink );
  connect( btnAddHistory, &QPushButton::clicked, this, &QgsMetadataWidget::addHistory );
  connect( btnRemoveHistory, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedHistory );
  connect( btnNewCategory, &QPushButton::clicked, this, &QgsMetadataWidget::addNewCategory );
  connect( btnAddDefaultCategory, &QPushButton::clicked, this, &QgsMetadataWidget::addDefaultCategories );
  connect( btnRemoveCategory, &QPushButton::clicked, this, &QgsMetadataWidget::removeSelectedCategories );

  fillComboBox();
  if ( !mLayer )
  {
    btnAutoSource->setEnabled( false );
    btnAutoEncoding->setEnabled( false );
    btnSetCrsFromLayer->setEnabled( false );
  }

  if ( mLayer )
  {
    mMetadata.reset( mLayer->metadata().clone() );
    setMode( LayerMetadata );
    setUiFromMetadata();
  }

  connect( lineEditTitle, &QLineEdit::textChanged, this, &QgsMetadataWidget::titleChanged );
}

void QgsMetadataWidget::setMode( QgsMetadataWidget::Mode mode )
{
  QString type;
  QString typeUpper;
  mMode = mode;
  switch ( mMode )
  {
    case LayerMetadata:
      type = tr( "dataset" );
      typeUpper = tr( "Dataset" );
      mEncodingFrame->show();
      mAuthorFrame->hide();
      btnAutoSource->setEnabled( mLayer );
      break;

    case ProjectMetadata:
      type = tr( "project" );
      typeUpper = tr( "Project" );
      mEncodingFrame->hide();
      mAuthorFrame->show();
      tabWidget->removeTab( 4 );
      tabWidget->removeTab( 3 );
      btnAutoSource->setEnabled( true );

      // these two widgets should be kept in sync
      connect( mCreationDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, [ = ]( const QDateTime & value )
      {
        if ( value.isValid() )
          mCreationDateTimeEdit2->setDateTime( value );
        else if ( mCreationDateTimeEdit2->dateTime().isValid() )
          mCreationDateTimeEdit2->clear();
      } );
      connect( mCreationDateTimeEdit2, &QDateTimeEdit::dateTimeChanged, this, [ = ]( const QDateTime & value )
      {
        if ( value.isValid() )
          mCreationDateTimeEdit->setDateTime( value );
        else if ( mCreationDateTimeEdit->dateTime().isValid() )
          mCreationDateTimeEdit->clear();
      } );

      break;
  }

  mIdLabel->setText( tr( "This page describes the basic attribution of the %1. Please use the tooltips for more information." ).arg( type ) );
  mLabelCategories->setText( tr( "%1 categories." ).arg( typeUpper ) );
  mLabelContact->setText( tr( "Contacts related to the %1." ).arg( type ) );
  mLabelLinks->setText( tr( "Links describe ancillary resources and information related to this %1." ).arg( type ) );
  mLabelHistory->setText( tr( "History about the %1." ).arg( type ) );
  labelKeywords->setText( tr( "<html><head/><body><p>Keywords are optional, and provide a way to provide additional descriptive information about "
                              "the %1. Edits made in the categories tab will update the category entry below. For the concept, we suggest "
                              "to use a standard based vocabulary such as <a href=\"https://www.eionet.europa.eu/gemet/en/inspire-themes/\">"
                              "<span style=\" text-decoration: underline; color:#0000ff;\">GEMET.</span></a></p></body></html>" ).arg( type ) );
  btnAutoSource->setText( tr( "Set from %1" ).arg( mMode == LayerMetadata ? tr( "layer" ) : tr( "project" ) ) );
}

void QgsMetadataWidget::setMetadata( const QgsAbstractMetadataBase *metadata )
{
  if ( !metadata )
    return;

  if ( dynamic_cast< const QgsLayerMetadata * >( metadata ) && mMode != LayerMetadata )
    setMode( LayerMetadata );
  else if ( dynamic_cast< const QgsProjectMetadata * >( metadata ) && mMode != ProjectMetadata )
    setMode( ProjectMetadata );

  mMetadata.reset( metadata->clone() );
  setUiFromMetadata();
}

QgsAbstractMetadataBase *QgsMetadataWidget::metadata()
{
  std::unique_ptr< QgsAbstractMetadataBase > md;
  switch ( mMode )
  {
    case LayerMetadata:
      md = std::make_unique< QgsLayerMetadata >();
      break;

    case ProjectMetadata:
      md = std::make_unique< QgsProjectMetadata >();
      break;

  }
  saveMetadata( md.get() );
  return md.release();
}

void QgsMetadataWidget::fillSourceFromLayer()
{
  switch ( mMode )
  {
    case LayerMetadata:
      if ( mLayer )
      {
        lineEditIdentifier->setText( mLayer->publicSource() );
      }
      break;

    case ProjectMetadata:
      lineEditIdentifier->setText( QgsProject::instance()->fileName() );
      break;
  }

}

void QgsMetadataWidget::addVocabulary()
{
  const int row = tabKeywords->rowCount();
  tabKeywords->setRowCount( row + 1 );
  QTableWidgetItem *pCell = nullptr;

  // Vocabulary
  pCell = new QTableWidgetItem( tr( "undefined %1" ).arg( row + 1 ) );
  tabKeywords->setItem( row, 0, pCell );

  // Keywords
  pCell = new QTableWidgetItem();
  tabKeywords->setItem( row, 1, pCell );
}

void QgsMetadataWidget::removeSelectedVocabulary()
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
    const int row = tabLicenses->rowCount();
    tabLicenses->setRowCount( row + 1 );
    QTableWidgetItem *pCell = new QTableWidgetItem( newLicence );
    tabLicenses->setItem( row, 0, pCell );
  }
}

void QgsMetadataWidget::removeSelectedLicence()
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

void QgsMetadataWidget::removeSelectedRight()
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

void QgsMetadataWidget::addConstraint()
{
  const int row = mConstraintsModel->rowCount();
  mConstraintsModel->setItem( row, 0, new QStandardItem( tr( "undefined %1" ).arg( row + 1 ) ) );
  mConstraintsModel->setItem( row, 1, new QStandardItem( tr( "undefined %1" ).arg( row + 1 ) ) );
}

void QgsMetadataWidget::removeSelectedConstraint()
{
  const QModelIndexList selectedRows = tabConstraints->selectionModel()->selectedRows();
  if ( selectedRows.empty() )
    return;
  mConstraintsModel->removeRow( selectedRows[0].row() );
}

void QgsMetadataWidget::crsChanged()
{
  if ( ( mCrs.isValid() ) && ( mLayer ) )
  {
    lblCurrentCrs->setText( tr( "CRS: %1" ).arg( mCrs.userFriendlyIdentifier() ) );
    spatialExtentSelector->setEnabled( true );
    spatialExtentSelector->setOutputCrs( mCrs );

    if ( mCrs == mLayer->crs() && mCrs == mLayer->dataProvider()->crs() )
    {
      lblCurrentCrsStatus->setText( tr( "Same as layer properties and provider." ) );
    }
    else if ( mCrs == mLayer->crs() && mCrs != mLayer->dataProvider()->crs() )
    {
      lblCurrentCrsStatus->setText( tr( "Same as layer properties but different than the provider." ) );
    }
    else if ( mCrs != mLayer->crs() && mCrs == mLayer->dataProvider()->crs() )
    {
      lblCurrentCrsStatus->setText( tr( "Same as the provider but different than the layer properties." ) );
    }
    else
    {
      lblCurrentCrsStatus->setText( tr( "Does not match either layer properties or the provider." ) );
    }
  }
  else
  {
    lblCurrentCrs->setText( tr( "CRS: Not set." ) );
    lblCurrentCrsStatus->setText( QString() );
    spatialExtentSelector->setEnabled( false );
  }
}

void QgsMetadataWidget::addAddress()
{
  const int row = tabAddresses->rowCount();
  tabAddresses->setRowCount( row + 1 );
  QTableWidgetItem *pCell = nullptr;

  // Type
  pCell = new QTableWidgetItem( tr( "postal" ) );
  tabAddresses->setItem( row, 0, pCell );

  // Address
  tabAddresses->setItem( row, 1, new QTableWidgetItem() );

  // postal code
  tabAddresses->setItem( row, 2, new QTableWidgetItem() );

  // City
  tabAddresses->setItem( row, 3, new QTableWidgetItem() );

  // Admin area
  tabAddresses->setItem( row, 4, new QTableWidgetItem() );

  // Country
  tabAddresses->setItem( row, 5, new QTableWidgetItem() );
}

void QgsMetadataWidget::removeSelectedAddress()
{
  QItemSelectionModel *selectionModel = tabAddresses->selectionModel();
  const QModelIndexList selectedRows = selectionModel->selectedRows();
  for ( int i = 0; i < selectedRows.size() ; i++ )
  {
    tabAddresses->model()->removeRow( selectedRows[i].row() );
  }
}

void QgsMetadataWidget::fillCrsFromLayer()
{
  mCrs = mLayer->crs();
  crsChanged();
}

void QgsMetadataWidget::fillCrsFromProvider()
{
  mCrs = mLayer->dataProvider()->crs();
  crsChanged();
}

void QgsMetadataWidget::addLink()
{
  const int row = mLinksModel->rowCount();
  mLinksModel->setItem( row, 0, new QStandardItem( tr( "undefined %1" ).arg( row + 1 ) ) );
  mLinksModel->setItem( row, 1, new QStandardItem() );
  mLinksModel->setItem( row, 2, new QStandardItem() );
  mLinksModel->setItem( row, 3, new QStandardItem() );
  mLinksModel->setItem( row, 4, new QStandardItem() );
  mLinksModel->setItem( row, 5, new QStandardItem() );
  mLinksModel->setItem( row, 6, new QStandardItem() );
}

void QgsMetadataWidget::removeSelectedLink()
{
  const QModelIndexList selectedRows = tabLinks->selectionModel()->selectedRows();
  if ( selectedRows.empty() )
    return;

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

void QgsMetadataWidget::removeSelectedHistory()
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

void QgsMetadataWidget::fillComboBox()
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

void QgsMetadataWidget::setUiFromMetadata()
{
  // Parent ID
  lineEditParentId->setText( mMetadata->parentIdentifier() );

  // Identifier
  if ( ! mMetadata->identifier().isEmpty() )
  {
    lineEditIdentifier->setText( mMetadata->identifier() );
  }

  // Title
  if ( ! mMetadata->title().isEmpty() )
  {
    whileBlocking( lineEditTitle )->setText( mMetadata->title() );
  }

  // Type
  if ( ! mMetadata->type().isEmpty() )
  {
    if ( comboType->findText( mMetadata->type() ) == -1 )
    {
      comboType->addItem( mMetadata->type() );
    }
    comboType->setCurrentIndex( comboType->findText( mMetadata->type() ) );
  }

  // Language
  if ( ! mMetadata->language().isEmpty() )
  {
    if ( comboLanguage->findText( mMetadata->language() ) == -1 )
    {
      comboLanguage->addItem( mMetadata->language() );
    }
    comboLanguage->setCurrentIndex( comboLanguage->findText( mMetadata->language() ) );
  }

  // Abstract
  textEditAbstract->setPlainText( mMetadata->abstract() );

  // Categories
  mCategoriesModel->setStringList( mMetadata->categories() );

  // Keywords
  tabKeywords->setRowCount( 0 );
  QMapIterator<QString, QStringList> i( mMetadata->keywords() );
  while ( i.hasNext() )
  {
    i.next();
    addVocabulary();
    int currentRow = tabKeywords->rowCount() - 1;
    tabKeywords->item( currentRow, 0 )->setText( i.key() );
    tabKeywords->item( currentRow, 1 )->setText( i.value().join( QLatin1Char( ',' ) ) );
  }

  if ( QgsLayerMetadata *layerMetadata = dynamic_cast< QgsLayerMetadata * >( mMetadata.get() ) )
  {
    // Encoding
    comboEncoding->setCurrentText( layerMetadata->encoding() );

    // Fees
    lineEditFees->setText( layerMetadata->fees() );

    // Licenses
    tabLicenses->setRowCount( 0 );
    const QStringList &licenses = layerMetadata->licenses();
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
    mRightsModel->setStringList( layerMetadata->rights() );

    // Constraints
    mConstraintsModel->clear();
    const QList<QgsLayerMetadata::Constraint> &constraints = layerMetadata->constraints();
    for ( const QgsLayerMetadata::Constraint &constraint : constraints )
    {
      const int row = mConstraintsModel->rowCount();
      mConstraintsModel->setItem( row, 0, new QStandardItem( constraint.type ) );
      mConstraintsModel->setItem( row, 1, new QStandardItem( constraint.constraint ) );
    }

    // CRS
    mCrs = layerMetadata->crs();
    crsChanged();

    // Spatial extent
    const QList<QgsLayerMetadata::SpatialExtent> &spatialExtents = layerMetadata->extent().spatialExtents();
    if ( ! spatialExtents.isEmpty() )
    {
      // Even if it's a list, it's supposed to store the same extent in different CRS.
      spatialExtentSelector->setOutputCrs( spatialExtents.at( 0 ).extentCrs );
      spatialExtentSelector->setOriginalExtent( spatialExtents.at( 0 ).bounds.toRectangle(), spatialExtents.at( 0 ).extentCrs );
      spatialExtentSelector->setOutputExtentFromOriginal();
      spinBoxZMaximum->setValue( spatialExtents.at( 0 ).bounds.zMaximum() );
      spinBoxZMinimum->setValue( spatialExtents.at( 0 ).bounds.zMinimum() );
    }

    // Temporal extent
    const QList<QgsDateTimeRange> &temporalExtents = layerMetadata->extent().temporalExtents();
    if ( ! temporalExtents.isEmpty() )
    {
      // Even if it's a list, it seems we use only one for now (cf discussion with Tom)
      dateTimeFrom->setDateTime( temporalExtents.at( 0 ).begin() );
      dateTimeTo->setDateTime( temporalExtents.at( 0 ).end() );
    }
    else
    {
      dateTimeFrom->clear();
      dateTimeTo->clear();
    }
  }
  else if ( QgsProjectMetadata *projectMetadata = dynamic_cast< QgsProjectMetadata * >( mMetadata.get() ) )
  {
    mLineEditAuthor->setText( projectMetadata->author() );
  }

  // Contacts
  const QList<QgsAbstractMetadataBase::Contact> &contacts = mMetadata->contacts();
  if ( ! contacts.isEmpty() )
  {
    // Only one contact supported in the UI for now
    const QgsAbstractMetadataBase::Contact &contact = contacts.at( 0 );
    lineEditContactName->setText( contact.name );
    lineEditContactEmail->setText( contact.email );
    lineEditContactFax->setText( contact.fax );
    lineEditContactOrganization->setText( contact.organization );
    lineEditContactPosition->setText( contact.position );
    lineEditContactVoice->setText( contact.voice );
    if ( comboContactRole->findText( contact.role ) == -1 )
    {
      comboContactRole->addItem( contact.role );
    }
    comboContactRole->setCurrentIndex( comboContactRole->findText( contact.role ) );
    tabAddresses->setRowCount( 0 );
    const QList<QgsAbstractMetadataBase::Address> &addresses = contact.addresses;
    for ( const QgsAbstractMetadataBase::Address &address : addresses )
    {
      int currentRow = tabAddresses->rowCount();
      tabAddresses->setRowCount( currentRow + 1 );
      tabAddresses->setItem( currentRow, 0,  new QTableWidgetItem( address.type ) );
      tabAddresses->setItem( currentRow, 1,  new QTableWidgetItem( address.address ) );
      tabAddresses->setItem( currentRow, 2,  new QTableWidgetItem( address.postalCode ) );
      tabAddresses->setItem( currentRow, 3,  new QTableWidgetItem( address.city ) );
      tabAddresses->setItem( currentRow, 4,  new QTableWidgetItem( address.administrativeArea ) );
      tabAddresses->setItem( currentRow, 5,  new QTableWidgetItem( address.country ) );
    }
  }

  // Links
  const QList<QgsAbstractMetadataBase::Link> &links = mMetadata->links();
  mLinksModel->setRowCount( 0 );
  for ( const QgsAbstractMetadataBase::Link &link : links )
  {
    const int row = mLinksModel->rowCount();
    mLinksModel->setItem( row, 0, new QStandardItem( link.name ) );
    mLinksModel->setItem( row, 1, new QStandardItem( link.type ) );
    mLinksModel->setItem( row, 2, new QStandardItem( link.url ) );
    mLinksModel->setItem( row, 3, new QStandardItem( link.description ) );
    mLinksModel->setItem( row, 4, new QStandardItem( link.format ) );
    mLinksModel->setItem( row, 5, new QStandardItem( link.mimeType ) );
    mLinksModel->setItem( row, 6, new QStandardItem( link.size ) );
  }

  // History
  mHistoryModel->setStringList( mMetadata->history() );

  // dates
  if ( mMetadata->dateTime( Qgis::MetadataDateType::Created ).isValid() )
    mCreationDateTimeEdit2->setDateTime( mMetadata->dateTime( Qgis::MetadataDateType::Created ) );
  else
    mCreationDateTimeEdit2->clear();

  if ( mMetadata->dateTime( Qgis::MetadataDateType::Published ).isValid() )
    mPublishedDateTimeEdit->setDateTime( mMetadata->dateTime( Qgis::MetadataDateType::Published ) );
  else
    mPublishedDateTimeEdit->clear();

  if ( mMetadata->dateTime( Qgis::MetadataDateType::Revised ).isValid() )
    mRevisedDateTimeEdit->setDateTime( mMetadata->dateTime( Qgis::MetadataDateType::Revised ) );
  else
    mRevisedDateTimeEdit->clear();

  if ( mMetadata->dateTime( Qgis::MetadataDateType::Superseded ).isValid() )
    mSupersededDateTimeEdit->setDateTime( mMetadata->dateTime( Qgis::MetadataDateType::Superseded ) );
  else
    mSupersededDateTimeEdit->clear();
}

void QgsMetadataWidget::saveMetadata( QgsAbstractMetadataBase *metadata )
{
  if ( !metadata )
    return;

  metadata->setParentIdentifier( lineEditParentId->text() );
  metadata->setIdentifier( lineEditIdentifier->text() );
  metadata->setTitle( lineEditTitle->text() );
  metadata->setType( comboType->currentText() );
  metadata->setLanguage( comboLanguage->currentText() );
  metadata->setAbstract( textEditAbstract->toPlainText() );

  // Keywords, it will save categories too.
  syncFromCategoriesTabToKeywordsTab();
  QMap<QString, QStringList> keywords;
  for ( int i = 0; i < tabKeywords->rowCount() ; i++ )
  {
    keywords.insert( tabKeywords->item( i, 0 )->text(), tabKeywords->item( i, 1 )->text().split( ',' ) );
  }
  metadata->setKeywords( keywords );

  switch ( mMode )
  {
    case LayerMetadata:
    {
      QgsLayerMetadata *layerMetadata = static_cast< QgsLayerMetadata * >( metadata );
      // Fees
      layerMetadata->setFees( lineEditFees->text() );

      // Licenses
      QStringList licenses;
      for ( int i = 0; i < tabLicenses->rowCount() ; i++ )
      {
        licenses.append( tabLicenses->item( i, 0 )->text() );
      }
      layerMetadata->setLicenses( licenses );

      // Rights
      layerMetadata->setRights( mRightsModel->stringList() );

      // Encoding
      layerMetadata->setEncoding( comboEncoding->currentText() );

      // Constraints
      QList<QgsLayerMetadata::Constraint> constraints;
      for ( int row = 0; row < mConstraintsModel->rowCount() ; row++ )
      {
        QgsLayerMetadata::Constraint constraint;
        constraint.type = mConstraintsModel->item( row, 0 )->text();
        constraint.constraint = mConstraintsModel->item( row, 1 )->text();
        constraints.append( constraint );
      }
      layerMetadata->setConstraints( constraints );

      // CRS
      if ( mCrs.isValid() )
      {
        layerMetadata->setCrs( mCrs );
      }

      // Extent
      QgsLayerMetadata::SpatialExtent spatialExtent;
      spatialExtent.bounds = QgsBox3d( spatialExtentSelector->outputExtent() );
      spatialExtent.bounds.setZMinimum( spinBoxZMinimum->value() );
      spatialExtent.bounds.setZMaximum( spinBoxZMaximum->value() );
      spatialExtent.extentCrs = spatialExtentSelector->outputCrs();
      QList<QgsLayerMetadata::SpatialExtent> spatialExtents;
      spatialExtents.append( spatialExtent );
      QList<QgsDateTimeRange> temporalExtents;
      temporalExtents.append( QgsDateTimeRange( dateTimeFrom->dateTime(), dateTimeTo->dateTime() ) );
      QgsLayerMetadata::Extent extent;
      extent.setSpatialExtents( spatialExtents );
      extent.setTemporalExtents( temporalExtents );
      layerMetadata->setExtent( extent );
      break;
    }

    case ProjectMetadata:
    {
      QgsProjectMetadata *projectMetadata = static_cast< QgsProjectMetadata * >( metadata );
      projectMetadata->setAuthor( mLineEditAuthor->text() );
      break;
    }
  }

  // Contacts, only one contact supported in the UI for now.
  // We don't want to lost data if more than one contact, so we update only the first one.
  QList<QgsAbstractMetadataBase::Contact> contacts = mMetadata->contacts();
  if ( contacts.size() > 0 )
    contacts.removeFirst();
  QgsAbstractMetadataBase::Contact contact;
  contact.email = lineEditContactEmail->text();
  contact.position = lineEditContactPosition->text();
  contact.fax = lineEditContactFax->text();
  contact.voice = lineEditContactVoice->text();
  contact.name = lineEditContactName->text();
  contact.organization = lineEditContactOrganization->text();
  contact.role = comboContactRole->currentText();
  QList<QgsAbstractMetadataBase::Address> addresses;
  for ( int i = 0; i < tabAddresses->rowCount() ; i++ )
  {
    QgsAbstractMetadataBase::Address address;
    address.type = tabAddresses->item( i, 0 )->text();
    address.address = tabAddresses->item( i, 1 )->text();
    address.postalCode = tabAddresses->item( i, 2 )->text();
    address.city = tabAddresses->item( i, 3 )->text();
    address.administrativeArea = tabAddresses->item( i, 4 )->text();
    address.country = tabAddresses->item( i, 5 )->text();
    addresses.append( address );
  }
  contact.addresses = addresses;
  contacts.insert( 0, contact );
  metadata->setContacts( contacts );

  // Links
  QList<QgsAbstractMetadataBase::Link> links;
  for ( int row = 0; row < mLinksModel->rowCount() ; row++ )
  {
    QgsAbstractMetadataBase::Link link;
    link.name = mLinksModel->item( row, 0 )->text();
    link.type = mLinksModel->item( row, 1 )->text();
    link.url = mLinksModel->item( row, 2 )->text();
    link.description = mLinksModel->item( row, 3 )->text();
    link.format = mLinksModel->item( row, 4 )->text();
    link.mimeType = mLinksModel->item( row, 5 )->text();
    link.size = mLinksModel->item( row, 6 )->text();
    links.append( link );
  }
  metadata->setLinks( links );

  // History
  metadata->setHistory( mHistoryModel->stringList() );

  // dates
  metadata->setDateTime( Qgis::MetadataDateType::Created, mCreationDateTimeEdit2->dateTime() );
  metadata->setDateTime( Qgis::MetadataDateType::Published, mPublishedDateTimeEdit->dateTime() );
  metadata->setDateTime( Qgis::MetadataDateType::Revised, mRevisedDateTimeEdit->dateTime() );
  metadata->setDateTime( Qgis::MetadataDateType::Superseded, mSupersededDateTimeEdit->dateTime() );
}

bool QgsMetadataWidget::checkMetadata()
{
  std::unique_ptr< QgsAbstractMetadataBase > md( metadata() );

  std::unique_ptr< QgsNativeMetadataBaseValidator > validator;
  switch ( mMode )
  {
    case LayerMetadata:
      validator = std::make_unique< QgsNativeMetadataValidator>();
      break;

    case ProjectMetadata:
      validator = std::make_unique< QgsNativeProjectMetadataValidator>();
      break;
  }

  QList<QgsAbstractMetadataBaseValidator::ValidationResult> validationResults;
  bool results = validator->validate( md.get(), validationResults );

  QString errors;
  if ( !results )
  {
    for ( const QgsAbstractMetadataBaseValidator::ValidationResult &result : std::as_const( validationResults ) )
    {
      errors += QLatin1String( "<b>" ) % result.section;
      if ( ! QgsVariantUtils::isNull( result._identifier() ) )
      {
        errors += QLatin1Char( ' ' ) % QVariant( result._identifier().toInt() + 1 ).toString();
      }
      errors += QLatin1String( "</b>: " ) % result.note % QLatin1String( "<br />" );
    }
  }
  else
  {
    errors = tr( "Ok, it seems valid according to the QGIS Schema." );
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
    QgsDebugMsg( QStringLiteral( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
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
    QgsDebugMsg( QStringLiteral( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
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
    QgsDebugMsg( QStringLiteral( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
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
    QgsDebugMsg( QStringLiteral( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
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
    QgsDebugMsg( QStringLiteral( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
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
    QgsDebugMsg( QStringLiteral( "Error while opening the CSV file: %1, %2 " ).arg( path, file.errorString() ) );
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

void QgsMetadataWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  if ( canvas )
    spatialExtentSelector->setCurrentExtent( canvas->extent(), canvas->mapSettings().destinationCrs() );
}

QString QgsMetadataWidget::title() const
{
  return lineEditTitle->text();
}

void QgsMetadataWidget::setTitle( const QString &title )
{
  if ( title != lineEditTitle->text() )
  {
    whileBlocking( lineEditTitle )->setText( title );
    emit titleChanged( title );
  }
}

void QgsMetadataWidget::acceptMetadata()
{
  saveMetadata( mMetadata.get() );
  switch ( mMode )
  {
    case LayerMetadata:
      if ( mLayer )
      {
        // Save layer metadata properties
        mLayer->setMetadata( *static_cast< QgsLayerMetadata * >( mMetadata.get() ) );
      }
      break;

    case ProjectMetadata:
      QgsProject::instance()->setMetadata( *static_cast< QgsProjectMetadata * >( mMetadata.get() ) );
      break;
  }
}

void QgsMetadataWidget::syncFromCategoriesTabToKeywordsTab()
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
    tabKeywords->item( row, 1 )->setText( mCategoriesModel->stringList().join( QLatin1Char( ',' ) ) );
  }
}

void QgsMetadataWidget::updatePanel()
{
  int index = tabWidget->currentIndex();
  QString currentTabText = tabWidget->widget( index )->objectName();
  if ( currentTabText == QLatin1String( "tabCategoriesDialog" ) )
  {
    // Categories tab
    // We need to take keywords and insert them into the list
    QList<QTableWidgetItem *> categories = tabKeywords->findItems( QStringLiteral( "gmd:topicCategory" ), Qt::MatchExactly );
    if ( !categories.isEmpty() )
    {
      const int row = categories.at( 0 )->row();
      mCategoriesModel->setStringList( tabKeywords->item( row, 1 )->text().split( ',' ) );
    }
    else
    {
      mCategoriesModel->setStringList( QStringList() );
    }
  }
  else if ( currentTabText == QLatin1String( "tabKeywordsDialog" ) )
  {
    // Keywords tab
    // We need to take categories and insert them into the table
    syncFromCategoriesTabToKeywordsTab();
  }
  else if ( currentTabText == QLatin1String( "tabValidationDialog" ) )
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

void QgsMetadataWidget::addDefaultCategories()
{
  const QModelIndexList selectedIndexes = listDefaultCategories->selectionModel()->selectedIndexes();
  QStringList defaultCategoriesList = mDefaultCategoriesModel->stringList();
  QStringList selectedCategories = mCategoriesModel->stringList();

  for ( const QModelIndex &selection : selectedIndexes )
  {
    QVariant item = mDefaultCategoriesModel->data( selection, Qt::DisplayRole );
    defaultCategoriesList.removeOne( item.toString() );

    selectedCategories.append( item.toString() );
  }

  mDefaultCategoriesModel->setStringList( defaultCategoriesList );
  mCategoriesModel->setStringList( selectedCategories );
  mCategoriesModel->sort( 0 );
}

void QgsMetadataWidget::removeSelectedCategories()
{
  const QModelIndexList selectedIndexes = listCategories->selectionModel()->selectedIndexes();
  QStringList categories = mCategoriesModel->stringList();
  QStringList defaultList = mDefaultCategoriesModel->stringList();

  for ( const QModelIndex &selection : selectedIndexes )
  {
    QVariant item = mCategoriesModel->data( selection, Qt::DisplayRole );
    categories.removeOne( item.toString() );

    if ( mDefaultCategories.contains( item.toString() ) )
    {
      defaultList.append( item.toString() );
    }
  }
  mCategoriesModel->setStringList( categories );

  mDefaultCategoriesModel->setStringList( defaultList );
  mDefaultCategoriesModel->sort( 0 );
}

///@cond PRIVATE
LinkItemDelegate::LinkItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{

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

ConstraintItemDelegate::ConstraintItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{

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
///@endcond
