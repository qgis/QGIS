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

  connect( mTestButton, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::testQuery );
  connect( mBrowseCRSBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::browseCRS );
  connect( mAddLayerBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::addLayer );
  connect( mRemoveLayerBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::removeLayer );
  connect( mImportLayerBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::importLayer );
  connect( mLayersTable->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QgsVirtualLayerSourceSelect::tableRowChanged );

  // prepare provider list
  Q_FOREACH ( const QString &pk, QgsProviderRegistry::instance()->providerList() )
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

  QString lid = mLayerNameCombo->itemData( idx ).toString();
  QgsVectorLayer *l = static_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( lid ) );
  if ( !l )
    return;
  QgsVirtualLayerDefinition def = QgsVirtualLayerDefinition::fromUrl( QUrl::fromEncoded( l->source().toUtf8() ) );

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
    QgsCoordinateReferenceSystem crs( def.geometrySrid() );
    mCRS->setText( crs.authid() );
    mGeometryType->setCurrentIndex( static_cast<long>( def.geometryWkbType() ) - 1 );
    mGeometryField->setText( def.geometryField() );
  }

  // Clear embedded layers table
  mLayersTable->model()->removeRows( 0, mLayersTable->model()->rowCount() );
  // Add embedded layers
  Q_FOREACH ( const QgsVirtualLayerDefinition::SourceLayer &l, def.sourceLayers() )
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
  QgsCoordinateReferenceSystem crs( mSrid );
  crsSelector.setCrs( crs );
  crsSelector.setMessage( QString() );
  if ( crsSelector.exec() )
  {
    mCRS->setText( crsSelector.crs().authid() );
    QgsCoordinateReferenceSystem newCrs = crsSelector.crs();
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
  if ( ! mUIDField->text().isEmpty() )
  {
    def.setUid( mUIDField->text() );
  }
  if ( mNoGeometryRadio->isChecked() )
  {
    def.setGeometryWkbType( QgsWkbTypes::NoGeometry );
  }
  else if ( mGeometryRadio->isChecked() )
  {
    QgsWkbTypes::Type t = mGeometryType->currentIndex() > -1 ? static_cast<QgsWkbTypes::Type>( mGeometryType->currentIndex() + 1 ) : QgsWkbTypes::NoGeometry;
    def.setGeometryWkbType( t );
    def.setGeometryField( mGeometryField->text() );
    def.setGeometrySrid( mSrid );
  }

  // add embedded layers
  for ( int i = 0; i < mLayersTable->rowCount(); i++ )
  {
    QString name = mLayersTable->item( i, 0 )->text();
    QString provider = static_cast<QComboBox *>( mLayersTable->cellWidget( i, 1 ) )->currentText();
    QString encoding = static_cast<QComboBox *>( mLayersTable->cellWidget( i, 2 ) )->currentText();
    QString source = mLayersTable->item( i, 3 )->text();
    def.addSource( name, source, provider, encoding );
  }

  return def;
}

void QgsVirtualLayerSourceSelect::testQuery()
{
  QgsVirtualLayerDefinition def = getVirtualLayerDef();
  // If the definition is empty just do nothing.
  // TODO: a validation function that can enable/disable the test button
  //       according to the validity of the active layer definition
  if ( ! def.toString().isEmpty() )
  {
    std::unique_ptr<QgsVectorLayer> vl( new QgsVectorLayer( def.toString(), QStringLiteral( "test" ), QStringLiteral( "virtual" ) ) );
    if ( vl->isValid() )
    {
      QMessageBox::information( nullptr, tr( "Virtual layer test" ), tr( "No error" ) );
    }
    else
    {
      QMessageBox::critical( nullptr, tr( "Virtual layer test" ), vl->dataProvider()->error().summary() );
    }
  }
}

void QgsVirtualLayerSourceSelect::addLayer()
{
  mLayersTable->insertRow( mLayersTable->rowCount() );

  mLayersTable->setItem( mLayersTable->rowCount() - 1, 0, new QTableWidgetItem() );
  mLayersTable->setItem( mLayersTable->rowCount() - 1, 3, new QTableWidgetItem() );

  QComboBox *providerCombo = new QComboBox();
  providerCombo->addItems( mProviderList );
  mLayersTable->setCellWidget( mLayersTable->rowCount() - 1, 1, providerCombo );

  QComboBox *encodingCombo = new QComboBox();
  encodingCombo->addItems( QgsVectorDataProvider::availableEncodings() );
  QString defaultEnc = QgsSettings().value( QStringLiteral( "/UI/encoding" ), "System" ).toString();
  encodingCombo->setCurrentIndex( encodingCombo->findText( defaultEnc ) );
  mLayersTable->setCellWidget( mLayersTable->rowCount() - 1, 2, encodingCombo );
}

void QgsVirtualLayerSourceSelect::removeLayer()
{
  int currentRow = mLayersTable->selectionModel()->currentIndex().row();
  if ( currentRow != -1 )
    mLayersTable->removeRow( currentRow );
}

void QgsVirtualLayerSourceSelect::tableRowChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous );
  mRemoveLayerBtn->setEnabled( current.row() != -1 );
}

void QgsVirtualLayerSourceSelect::updateLayersList()
{
  mLayerNameCombo->clear();

  if ( mTreeView )
  {
    QgsLayerTreeModel *model = qobject_cast<QgsLayerTreeModel *>( mTreeView->model() );
    Q_FOREACH ( QgsLayerTreeLayer *layer, model->rootGroup()->findLayers() )
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
  Q_FOREACH ( QgsMapLayer *l, QgsProject::instance()->mapLayers() )
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
    QStringList ids = mEmbeddedSelectionDialog->layers();
    Q_FOREACH ( const QString &id, ids )
    {
      QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( id ) );
      addEmbeddedLayer( vl->name(), vl->providerType(), vl->dataProvider()->encoding(), vl->source() );
    }
  }
}

void QgsVirtualLayerSourceSelect::addButtonClicked()
{
  QString layerName = QStringLiteral( "virtual_layer" );
  QString id;
  bool replace = false;
  int idx = mLayerNameCombo->currentIndex();
  if ( idx != -1 && !mLayerNameCombo->currentText().isEmpty() )
  {
    layerName = mLayerNameCombo->currentText();
  }

  QgsVirtualLayerDefinition def = getVirtualLayerDef();


  if ( idx != -1 )
  {
    id = ( mLayerNameCombo->itemData( idx ).toString() );
    if ( !id.isEmpty() && mLayerNameCombo->currentText() == QgsProject::instance()->mapLayer( id )->name() )
    {
      int r = QMessageBox::warning( nullptr, tr( "Warning" ), tr( "A virtual layer of this name already exists, would you like to overwrite it?" ), QMessageBox::Yes | QMessageBox::No );
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

QGISEXTERN QgsVirtualLayerSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsVirtualLayerSourceSelect( parent, fl, widgetMode );
}

