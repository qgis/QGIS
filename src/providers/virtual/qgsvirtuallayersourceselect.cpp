/***************************************************************************
           qgsvirtuallayersourceselect.cpp
      Virtual layer data provider selection widget

begin                : Jan 2016
copyright            : (C) 2016 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtuallayersourceselect.h"

#include "layertree/qgslayertreeview.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsprojectionselectiondialog.h"
#include "layertree/qgslayertreemodel.h"
#include "layertree/qgslayertreegroup.h"
#include "layertree/qgslayertreelayer.h"
#include "layertree/qgslayertree.h"
#include "qgsproviderregistry.h"

#include "qgsembeddedlayerselectdialog.h"

#include <QUrl>
#include <QWidget>
#include <Qsci/qscilexer.h>
#include <QMessageBox>
#include <QTextStream>

QgsVirtualLayerSourceSelect::QgsVirtualLayerSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVirtualLayerSourceSelect::showHelp );

  mQueryEdit->setLineNumbersVisible( true );

  connect( mTestButton, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::testQuery );
  connect( mBrowseCRSBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::browseCRS );
  connect( mAddLayerBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::addLayer );
  connect( mRemoveLayerBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::removeLayer );
  connect( mImportLayerBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::importLayer );
  connect( mLayersTable->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QgsVirtualLayerSourceSelect::tableRowChanged );

  // prepare provider list
  const auto constProviderList = QgsProviderRegistry::instance()->providerList();
  for ( const QString &pk : constProviderList )
  {
    // we cannot know before trying to actually load a dataset
    // if the provider is raster or vector
    // so we manually exclude well-known raster providers
    if ( pk != QLatin1String( "gdal" ) && pk != QLatin1String( "ows" ) && pk != QLatin1String( "wcs" ) && pk != QLatin1String( "wms" ) )
    {
      mProviderList << pk;
    }
  }
  // It needs to find the layertree view without relying on the parent
  // being the main window
  const QList< QWidget * > widgets = qApp->allWidgets();
  for ( const QWidget *widget : widgets )
  {
    if ( ! mTreeView )
    {
      mTreeView = widget->findChild<QgsLayerTreeView *>( QStringLiteral( "theLayerTreeView" ) );
    }
  }
  updateLayersList();
  connect( mLayerNameCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsVirtualLayerSourceSelect::layerComboChanged );
  layerComboChanged( mLayerNameCombo->currentIndex() );

  // Prepare embedded layer selection dialog and
  // connect to model changes in the treeview
  if ( mTreeView )
  {
    mEmbeddedSelectionDialog = new QgsEmbeddedLayerSelectDialog( this, mTreeView );
    // Queued connection here prevents the updateLayerList to run before the tree layer
    // pointer points to the effective layer.
    connect( mTreeView->model(), &QAbstractItemModel::rowsInserted, this, &QgsVirtualLayerSourceSelect::updateLayersList, Qt::QueuedConnection );
    connect( mTreeView->model(), &QAbstractItemModel::rowsRemoved, this, &QgsVirtualLayerSourceSelect::updateLayersList );
    connect( mTreeView->model(), &QAbstractItemModel::dataChanged, this, &QgsVirtualLayerSourceSelect::updateLayersList );
  }
  // There is no validation logic to enable/disable the buttons
  // so they must be enabled by default
  emit enableButtons( true );
}

void QgsVirtualLayerSourceSelect::refresh()
{
  // TODO: check that this really works
  updateLayersList();
}

void QgsVirtualLayerSourceSelect::layerComboChanged( int idx )
{
  if ( idx == -1 )
    return;

  const QString lid = mLayerNameCombo->itemData( idx ).toString();
  QgsVectorLayer *l = static_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( lid ) );
  if ( !l )
    return;
  const QgsVirtualLayerDefinition def = QgsVirtualLayerDefinition::fromUrl( QUrl::fromEncoded( l->source().toUtf8() ) );

  if ( !def.query().isEmpty() )
  {
    mQueryEdit->setText( def.query() );
  }

  if ( !def.uid().isEmpty() )
  {
    mUIDColumnNameChck->setChecked( true );
    mUIDField->setText( def.uid() );
  }

  if ( def.geometryWkbType() == QgsWkbTypes::NoGeometry )
  {
    mNoGeometryRadio->setChecked( true );
  }
  else if ( def.hasDefinedGeometry() )
  {
    mGeometryRadio->setChecked( true );
    mSrid = def.geometrySrid();
    Q_NOWARN_DEPRECATED_PUSH
    const QgsCoordinateReferenceSystem crs( def.geometrySrid() );
    Q_NOWARN_DEPRECATED_POP
    mCRS->setText( crs.authid() );
    mGeometryType->setCurrentIndex( static_cast<long>( def.geometryWkbType() ) - 1 );
    mGeometryField->setText( def.geometryField() );
  }

  // Clear embedded layers table
  mLayersTable->model()->removeRows( 0, mLayersTable->model()->rowCount() );
  // Add embedded layers
  const auto constSourceLayers = def.sourceLayers();
  for ( const QgsVirtualLayerDefinition::SourceLayer &l : constSourceLayers )
  {
    if ( ! l.isReferenced() )
    {
      addEmbeddedLayer( l.name(), l.provider(), l.encoding(), l.source() );
    }
  }
}

void QgsVirtualLayerSourceSelect::browseCRS()
{
  QgsProjectionSelectionDialog crsSelector( this );
  Q_NOWARN_DEPRECATED_PUSH
  const QgsCoordinateReferenceSystem crs( mSrid );
  Q_NOWARN_DEPRECATED_POP
  crsSelector.setCrs( crs );
  if ( !crs.isValid() )
    crsSelector.showNoCrsForLayerMessage();

  if ( crsSelector.exec() )
  {
    mCRS->setText( crsSelector.crs().authid() );
    const QgsCoordinateReferenceSystem newCrs = crsSelector.crs();
    mSrid = newCrs.postgisSrid();
  }
}

QgsVirtualLayerDefinition QgsVirtualLayerSourceSelect::getVirtualLayerDef()
{
  QgsVirtualLayerDefinition def;

  if ( ! mQueryEdit->text().isEmpty() )
  {
    def.setQuery( mQueryEdit->text() );
  }
  if ( mUIDColumnNameChck->isChecked() && ! mUIDField->text().isEmpty() )
  {
    def.setUid( mUIDField->text() );
  }
  if ( mNoGeometryRadio->isChecked() )
  {
    def.setGeometryWkbType( QgsWkbTypes::NoGeometry );
  }
  else if ( mGeometryRadio->isChecked() )
  {
    const QgsWkbTypes::Type t = mGeometryType->currentIndex() > -1 ? static_cast<QgsWkbTypes::Type>( mGeometryType->currentIndex() + 1 ) : QgsWkbTypes::NoGeometry;
    def.setGeometryWkbType( t );
    def.setGeometryField( mGeometryField->text() );
    def.setGeometrySrid( mSrid );
  }

  // add embedded layers
  for ( int i = 0; i < mLayersTable->rowCount(); i++ )
  {
    const QString name = mLayersTable->item( i, 0 )->text();
    const QString provider = static_cast<QComboBox *>( mLayersTable->cellWidget( i, 1 ) )->currentText();
    const QString encoding = static_cast<QComboBox *>( mLayersTable->cellWidget( i, 2 ) )->currentText();
    const QString source = mLayersTable->item( i, 3 )->text();
    def.addSource( name, source, provider, encoding );
  }

  return def;
}

bool QgsVirtualLayerSourceSelect::preFlight()
{
  const QgsVirtualLayerDefinition def = getVirtualLayerDef();
  // If the definition is empty just do nothing.
  // TODO: a validation function that can enable/disable the test button
  //       according to the validity of the active layer definition
  if ( ! def.toString().isEmpty() )
  {
    const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
    std::unique_ptr<QgsVectorLayer> vl( new QgsVectorLayer( def.toString(), QStringLiteral( "test" ), QStringLiteral( "virtual" ), options ) );
    if ( vl->isValid() )
    {
      const QStringList fieldNames = vl->fields().names();
      if ( mUIDColumnNameChck->isChecked() && mUIDField->text().isEmpty() )
      {
        QMessageBox::warning( nullptr, tr( "Test Virtual Layer " ), tr( "Checkbox 'Unique identifier column' is checked, but no field given" ) );
      }
      else if ( mUIDColumnNameChck->isChecked() && !mUIDField->text().isEmpty() && !vl->fields().names().contains( mUIDField->text() ) )
      {
        QStringList bulletedFieldNames;
        for ( const QString &fieldName : fieldNames )
        {
          bulletedFieldNames.append( QLatin1String( "<li>" ) + fieldName + QLatin1String( "</li>" ) );
        }
        QMessageBox::warning( nullptr, tr( "Test Virtual Layer " ), tr( "The unique identifier field <b>%1</b> was not found in list of fields:<ul>%2</ul>" ).arg( mUIDField->text(), bulletedFieldNames.join( ' ' ) ) );
      }
      else
      {
        if ( mGeometryRadio->isChecked() && mCRS->text().isEmpty() )
        {
          // warning when the geometryRadio is checked, but the user did not set a proper crs
          // old implementation did NOT set a crs then...
          if ( QMessageBox::Yes == QMessageBox::question( nullptr, tr( "Test Virtual Layer " ), tr( "No CRS defined, are you sure you want to create a layer without a crs?" ), QMessageBox::Yes | QMessageBox::No ) )
          {
            return true;
          }
          else
          {
            return false;
          }
        }
        return true;
      }
    }
    else
    {
      QMessageBox::critical( nullptr, tr( "Test Virtual Layer" ), vl->dataProvider()->error().summary() );
    }
  }
  return false;
}

void QgsVirtualLayerSourceSelect::testQuery()
{
  if ( preFlight() )
  {
    QMessageBox::information( nullptr, tr( "Test Virtual Layer" ), tr( "No error" ) );
  }
}

void QgsVirtualLayerSourceSelect::addLayer()
{
  mLayersTable->insertRow( mLayersTable->rowCount() );

  mLayersTable->setItem( mLayersTable->rowCount() - 1, 0, new QTableWidgetItem() );
  mLayersTable->setItem( mLayersTable->rowCount() - 1, 3, new QTableWidgetItem() );

  QComboBox *providerCombo = new QComboBox();
  providerCombo->addItems( mProviderList );
  providerCombo->setCurrentText( QStringLiteral( "ogr" ) );
  mLayersTable->setCellWidget( mLayersTable->rowCount() - 1, 1, providerCombo );

  QComboBox *encodingCombo = new QComboBox();
  encodingCombo->addItems( QgsVectorDataProvider::availableEncodings() );
  const QString defaultEnc = QgsSettings().value( QStringLiteral( "/UI/encoding" ), "System" ).toString();
  encodingCombo->setCurrentIndex( encodingCombo->findText( defaultEnc ) );
  mLayersTable->setCellWidget( mLayersTable->rowCount() - 1, 2, encodingCombo );
}

void QgsVirtualLayerSourceSelect::removeLayer()
{
  const int currentRow = mLayersTable->selectionModel()->currentIndex().row();
  if ( currentRow != -1 )
    mLayersTable->removeRow( currentRow );
}

void QgsVirtualLayerSourceSelect::tableRowChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous )
  mRemoveLayerBtn->setEnabled( current.row() != -1 );
}

void QgsVirtualLayerSourceSelect::updateLayersList()
{
  mLayerNameCombo->clear();

  if ( mTreeView )
  {
    QgsLayerTreeProxyModel *proxyModel = qobject_cast<QgsLayerTreeProxyModel *>( mTreeView->model( ) );
    QgsLayerTreeModel *model = qobject_cast<QgsLayerTreeModel *>( proxyModel->sourceModel() );
    const auto constFindLayers = model->rootGroup()->findLayers();
    for ( QgsLayerTreeLayer *layer : constFindLayers )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer->layer() );
      if ( vl && vl->providerType() == QLatin1String( "virtual" ) )
      {
        // store layer's id as user data
        mLayerNameCombo->addItem( layer->layer()->name(), layer->layer()->id() );
      }
    }
  }

  if ( mLayerNameCombo->count() == 0 )
    mLayerNameCombo->addItem( QStringLiteral( "virtual_layer" ) );

  // select the current layer, if any
  if ( mTreeView )
  {
    QList<QgsMapLayer *> selected = mTreeView->selectedLayers();
    if ( selected.size() == 1 && selected[0]->type() == QgsMapLayerType::VectorLayer && static_cast<QgsVectorLayer *>( selected[0] )->providerType() == QLatin1String( "virtual" ) )
    {
      mLayerNameCombo->setCurrentIndex( mLayerNameCombo->findData( selected[0]->id() ) );
    }
  }

  // configure auto completion with SQL functions
  QsciAPIs *apis = new QsciAPIs( mQueryEdit->lexer() );

  Q_INIT_RESOURCE( sqlfunctionslist );
  QFile fFile( QStringLiteral( ":/sqlfunctions/list.txt" ) );
  if ( fFile.open( QIODevice::ReadOnly ) )
  {
    QTextStream in( &fFile );
    while ( !in.atEnd() )
    {
      apis->add( in.readLine().toLower() + "()" );
    }
    fFile.close();
  }

  // configure auto completion with table and column names
  const auto constMapLayers = QgsProject::instance()->mapLayers();
  for ( QgsMapLayer *l : constMapLayers )
  {
    if ( l->type() == QgsMapLayerType::VectorLayer )
    {
      apis->add( l->name() );
      QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( l );
      const QgsFields fields = vl->fields();
      for ( const QgsField &f : fields )
      {
        apis->add( f.name() );
      }
    }
  }

  apis->prepare();
  mQueryEdit->lexer()->setAPIs( apis );
  mQueryEdit->setWrapMode( QsciScintilla::WrapWord );

  // Update the layer selection list
  if ( mEmbeddedSelectionDialog )
  {
    mEmbeddedSelectionDialog->updateLayersList();
  }
}

void QgsVirtualLayerSourceSelect::addEmbeddedLayer( const QString &name, const QString &provider, const QString &encoding, const QString &source )
{
  // insert a new row
  addLayer();
  const int n = mLayersTable->rowCount() - 1;
  // local name
  mLayersTable->item( n, 0 )->setText( name );
  // source
  mLayersTable->item( n, 3 )->setText( source );
  // provider
  QComboBox *providerCombo = static_cast<QComboBox *>( mLayersTable->cellWidget( n, 1 ) );
  providerCombo->setCurrentIndex( providerCombo->findText( provider ) );
  // encoding
  QComboBox *encodingCombo = static_cast<QComboBox *>( mLayersTable->cellWidget( n, 2 ) );
  encodingCombo->setCurrentIndex( encodingCombo->findText( encoding ) );
}

void QgsVirtualLayerSourceSelect::importLayer()
{
  if ( mEmbeddedSelectionDialog && mEmbeddedSelectionDialog->exec() == QDialog::Accepted )
  {
    const QStringList ids = mEmbeddedSelectionDialog->layers();
    const auto constIds = ids;
    for ( const QString &id : constIds )
    {
      QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( id ) );
      addEmbeddedLayer( vl->name(), vl->providerType(), vl->dataProvider()->encoding(), vl->source() );
    }
  }
}

void QgsVirtualLayerSourceSelect::addButtonClicked()
{
  if ( ! preFlight() )
  {
    return;
  }

  QString layerName = QStringLiteral( "virtual_layer" );
  QString id;
  bool replace = false;
  const int idx = mLayerNameCombo->currentIndex();
  if ( idx != -1 && !mLayerNameCombo->currentText().isEmpty() )
  {
    layerName = mLayerNameCombo->currentText();
  }

  const QgsVirtualLayerDefinition def = getVirtualLayerDef();


  if ( idx != -1 )
  {
    id = ( mLayerNameCombo->itemData( idx ).toString() );
    if ( !id.isEmpty() && mLayerNameCombo->currentText() == QgsProject::instance()->mapLayer( id )->name() )
    {
      const int r = QMessageBox::warning( nullptr, tr( "Warning" ), tr( "A virtual layer of this name already exists, would you like to overwrite it?" ), QMessageBox::Yes | QMessageBox::No );
      if ( r == QMessageBox::Yes )
      {
        replace = true;
      }
    }
  }
  // This check is to prevent a crash, a proper implementation should handle
  // the Add button state when a virtual layer definition is available
  if ( ! def.toString().isEmpty() )
  {
    if ( replace )
    {
      emit replaceVectorLayer( id, def.toString(), layerName, QStringLiteral( "virtual" ) );
    }
    else
    {
      emit addVectorLayer( def.toString(), layerName );
    }
  }
  if ( widgetMode() == QgsProviderRegistry::WidgetMode::None )
  {
    accept();
  }
}

void QgsVirtualLayerSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-virtual-layers" ) );
}
