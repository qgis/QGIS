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
#include "qgsprovidermetadata.h"
#include "qgsprojectionselectiondialog.h"
#include "layertree/qgslayertreemodel.h"
#include "layertree/qgslayertreegroup.h"
#include "layertree/qgslayertreelayer.h"
#include "layertree/qgslayertree.h"
#include "qgsproviderregistry.h"
#include "qgsiconutils.h"
#include "qgsembeddedlayerselectdialog.h"
#include "qgsgui.h"
#include "qgsdatasourceselectdialog.h"
#include "qgsfileutils.h"

#include <QUrl>
#include <QWidget>
#include <Qsci/qscilexer.h>
#include <QMessageBox>
#include <QTextStream>

QgsVirtualLayerSourceSelect::QgsVirtualLayerSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  mLayersTable->setColumnCount( 4 );
  mLayersTable->setHorizontalHeaderItem( LayerColumn::Name, new QTableWidgetItem( tr( "Local Name" ) ) );
  mLayersTable->setHorizontalHeaderItem( LayerColumn::Source, new QTableWidgetItem( tr( "Source" ) ) );
  mLayersTable->setHorizontalHeaderItem( LayerColumn::Provider, new QTableWidgetItem( tr( "Provider" ) ) );
  mLayersTable->setHorizontalHeaderItem( LayerColumn::Encoding, new QTableWidgetItem( tr( "Encoding" ) ) );

  // annoying dance to get nice default column sizes
  QgsSettings settings;
  const QByteArray ba = settings.value( "/Windows/VirtualLayer/layerTableHeaderState" ).toByteArray();
  if ( !ba.isNull() )
  {
    mLayersTable->horizontalHeader()->restoreState( ba );
  }
  else
  {
    const QFontMetrics fm( font() );
    mLayersTable->horizontalHeader()->setSectionResizeMode( LayerColumn::Name, QHeaderView::Interactive );
    mLayersTable->horizontalHeader()->resizeSection( LayerColumn::Name, fm.horizontalAdvance( 'X' ) * 25 );
    mLayersTable->horizontalHeader()->setSectionResizeMode( LayerColumn::Provider, QHeaderView::Interactive );
    mLayersTable->horizontalHeader()->resizeSection( LayerColumn::Provider, fm.horizontalAdvance( 'X' ) * 25 );
    mLayersTable->horizontalHeader()->setSectionResizeMode( LayerColumn::Encoding, QHeaderView::Interactive );
    mLayersTable->horizontalHeader()->resizeSection( LayerColumn::Encoding, fm.horizontalAdvance( 'X' ) * 15 );
    mLayersTable->horizontalHeader()->setSectionResizeMode( LayerColumn::Source, QHeaderView::Interactive );
    mLayersTable->horizontalHeader()->resizeSection( LayerColumn::Source, fm.horizontalAdvance( 'X' ) * 50 );
  }

  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVirtualLayerSourceSelect::showHelp );

  QPushButton *pbn = new QPushButton( tr( "Test" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::testQuery );

  mGeometryType->addItem( QgsIconUtils::iconForWkbType( QgsWkbTypes::Point ), tr( "Point" ), static_cast< long long >( QgsWkbTypes::Point ) );
  mGeometryType->addItem( QgsIconUtils::iconForWkbType( QgsWkbTypes::LineString ), tr( "LineString" ), static_cast< long long >( QgsWkbTypes::LineString ) );
  mGeometryType->addItem( QgsIconUtils::iconForWkbType( QgsWkbTypes::Polygon ), tr( "Polygon" ), static_cast< long long >( QgsWkbTypes::Polygon ) );
  mGeometryType->addItem( QgsIconUtils::iconForWkbType( QgsWkbTypes::MultiPoint ), tr( "MultiPoint" ), static_cast< long long >( QgsWkbTypes::MultiPoint ) );
  mGeometryType->addItem( QgsIconUtils::iconForWkbType( QgsWkbTypes::MultiLineString ), tr( "MultiLineString" ), static_cast< long long >( QgsWkbTypes::MultiLineString ) );
  mGeometryType->addItem( QgsIconUtils::iconForWkbType( QgsWkbTypes::MultiPolygon ), tr( "MultiPolygon" ), static_cast< long long >( QgsWkbTypes::MultiPolygon ) );

  mQueryEdit->setLineNumbersVisible( true );

  connect( mBrowseCRSBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::browseCRS );
  connect( mAddLayerBtn, &QAbstractButton::clicked, this, [ = ] { addLayer( true ); } );
  connect( mRemoveLayerBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::removeLayer );
  connect( mImportLayerBtn, &QAbstractButton::clicked, this, &QgsVirtualLayerSourceSelect::importLayer );
  connect( mLayersTable->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QgsVirtualLayerSourceSelect::tableRowChanged );

  // prepare provider list
  const QSet< QString > vectorLayerProviders = QgsProviderRegistry::instance()->providersForLayerType( QgsMapLayerType::VectorLayer );
  mProviderList = qgis::setToList( vectorLayerProviders );
  std::sort( mProviderList.begin(), mProviderList.end() );

  // It needs to find the layertree view without relying on the parent
  // being the main window
  const QList< QWidget * > widgets = qApp->allWidgets();
  for ( const QWidget *widget : widgets )
  {
    if ( !mTreeView )
    {
      mTreeView = widget->findChild<QgsLayerTreeView *>( QStringLiteral( "theLayerTreeView" ) );
    }
  }

  updateLayersList();
  connect( mLayerNameCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsVirtualLayerSourceSelect::layerComboChanged );
  layerComboChanged( mLayerNameCombo->currentIndex() );

  connect( QgsProject::instance(), &QgsProject::layersAdded, this, &QgsVirtualLayerSourceSelect::updateLayersList );
  connect( QgsProject::instance(), &QgsProject::layersRemoved, this, &QgsVirtualLayerSourceSelect::updateLayersList );

  // There is no validation logic to enable/disable the buttons
  // so they must be enabled by default
  emit enableButtons( true );
}

QgsVirtualLayerSourceSelect::~QgsVirtualLayerSourceSelect()
{
  QgsSettings settings;
  settings.setValue( "/Windows/VirtualLayer/layerTableHeaderState", mLayersTable->horizontalHeader()->saveState() );
}

void QgsVirtualLayerSourceSelect::setBrowserModel( QgsBrowserModel *model )
{
  QgsAbstractDataSourceWidget::setBrowserModel( model );
  for ( int i = 0; i < mLayersTable->rowCount(); i++ )
  {
    qobject_cast< QgsVirtualLayerSourceWidget * >( mLayersTable->cellWidget( i, LayerColumn::Source ) )->setBrowserModel( model );
  }
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
    mGeometryType->setCurrentIndex( mGeometryType->findData( static_cast<long long>( def.geometryWkbType() ) ) );
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
    const QgsWkbTypes::Type t = mGeometryType->currentIndex() > -1 ? static_cast<QgsWkbTypes::Type>( mGeometryType->currentData().toLongLong() ) : QgsWkbTypes::NoGeometry;
    def.setGeometryWkbType( t );
    def.setGeometryField( mGeometryField->text() );
    def.setGeometrySrid( mSrid );
  }

  // add embedded layers
  for ( int i = 0; i < mLayersTable->rowCount(); i++ )
  {
    const QString name = mLayersTable->item( i, LayerColumn::Name )->text();
    const QString provider = qobject_cast<QComboBox *>( mLayersTable->cellWidget( i, LayerColumn::Provider ) )->currentData().toString();
    const QString encoding = qobject_cast<QComboBox *>( mLayersTable->cellWidget( i, LayerColumn::Encoding ) )->currentText();
    const QString source = qobject_cast< QgsVirtualLayerSourceWidget * >( mLayersTable->cellWidget( i, LayerColumn::Source ) )->source();
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

void QgsVirtualLayerSourceSelect::addLayer( bool browseForLayer )
{
  mLayersTable->insertRow( mLayersTable->rowCount() );

  mLayersTable->setItem( mLayersTable->rowCount() - 1, LayerColumn::Name, new QTableWidgetItem() );

  QgsVirtualLayerSourceWidget *sourceWidget = new QgsVirtualLayerSourceWidget();
  sourceWidget->setBrowserModel( browserModel() );
  mLayersTable->setCellWidget( mLayersTable->rowCount() - 1, LayerColumn::Source, sourceWidget );
  connect( sourceWidget, &QgsVirtualLayerSourceWidget::sourceChanged, this, &QgsVirtualLayerSourceSelect::rowSourceChanged );

  QComboBox *providerCombo = new QComboBox();
  for ( const QString &key : std::as_const( mProviderList ) )
  {
    QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( key );
    providerCombo->addItem( metadata->icon(), metadata->description(), key );
  }
  providerCombo->setCurrentIndex( providerCombo->findData( QStringLiteral( "ogr" ) ) );
  mLayersTable->setCellWidget( mLayersTable->rowCount() - 1, LayerColumn::Provider, providerCombo );

  QComboBox *encodingCombo = new QComboBox();
  encodingCombo->addItems( QgsVectorDataProvider::availableEncodings() );
  const QString defaultEnc = QgsSettings().value( QStringLiteral( "/UI/encoding" ), "System" ).toString();
  encodingCombo->setCurrentIndex( encodingCombo->findText( defaultEnc ) );
  mLayersTable->setCellWidget( mLayersTable->rowCount() - 1, LayerColumn::Encoding, encodingCombo );

  if ( browseForLayer )
  {
    sourceWidget->browseForLayer();
  }
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

  const QVector<QgsVectorLayer *> vectorLayers = QgsProject::instance()->layers<QgsVectorLayer *>();
  for ( QgsVectorLayer *vl : vectorLayers )
  {
    if ( vl && vl->providerType() == QLatin1String( "virtual" ) )
    {
      // store layer's id as user data
      mLayerNameCombo->addItem( vl->name(), vl->id() );
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
}

void QgsVirtualLayerSourceSelect::addEmbeddedLayer( const QString &name, const QString &provider, const QString &encoding, const QString &source )
{
  // insert a new row
  addLayer();
  const int n = mLayersTable->rowCount() - 1;
  // local name
  mLayersTable->item( n, LayerColumn::Name )->setText( name );
  // source
  QgsVirtualLayerSourceWidget *sourceWidget = qobject_cast<QgsVirtualLayerSourceWidget *>( mLayersTable->cellWidget( n, LayerColumn::Source ) );
  sourceWidget->setSource( source, provider );
  // provider
  QComboBox *providerCombo = qobject_cast<QComboBox *>( mLayersTable->cellWidget( n, LayerColumn::Provider ) );
  providerCombo->setCurrentIndex( providerCombo->findData( provider ) );
  // encoding
  QComboBox *encodingCombo = qobject_cast<QComboBox *>( mLayersTable->cellWidget( n, LayerColumn::Encoding ) );
  encodingCombo->setCurrentIndex( encodingCombo->findText( encoding ) );
}

void QgsVirtualLayerSourceSelect::importLayer()
{
  QgsEmbeddedLayerSelectDialog dialog( this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    const QStringList ids = dialog.layers();
    for ( const QString &id : ids )
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( id ) ) )
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
      emit addVectorLayer( def.toString(), layerName, QStringLiteral( "virtual" ) );
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

void QgsVirtualLayerSourceSelect::rowSourceChanged()
{
  QgsVirtualLayerSourceWidget *widget = qobject_cast< QgsVirtualLayerSourceWidget * >( sender() );
  // we have to find the matching row for the source widget which was changed
  for ( int row = 0; row < mLayersTable->rowCount(); row++ )
  {
    QgsVirtualLayerSourceWidget *rowSourceWidget = qobject_cast< QgsVirtualLayerSourceWidget * >( mLayersTable->cellWidget( row, LayerColumn::Source ) );
    if ( rowSourceWidget == widget )
    {
      // automatically update provider to match
      QComboBox *providerCombo = qobject_cast<QComboBox *>( mLayersTable->cellWidget( row, LayerColumn::Provider ) );
      providerCombo->setCurrentIndex( providerCombo->findData( widget->provider() ) );

      // if no layer name set yet, try to pick a good starting point
      if ( mLayersTable->item( row, LayerColumn::Name )->text().isEmpty() )
      {
        const QVariantMap sourceParts = QgsProviderRegistry::instance()->decodeUri( widget->provider(), widget->source() );
        if ( !sourceParts.value( QStringLiteral( "layerName" ) ).toString().isEmpty() )
        {
          const QString layerName = sourceParts.value( QStringLiteral( "layerName" ) ).toString();
          mLayersTable->item( row, LayerColumn::Name )->setText( layerName );
        }
        else if ( !sourceParts.value( QStringLiteral( "path" ) ).toString().isEmpty() )
        {
          const QFileInfo fi( sourceParts.value( QStringLiteral( "path" ) ).toString() );
          if ( !fi.baseName().isEmpty() )
          {
            mLayersTable->item( row, LayerColumn::Name )->setText( fi.baseName() );
          }
        }
      }
      break;
    }
  }
}

//
// QgsVirtualLayerSourceWidget
//
QgsVirtualLayerSourceWidget::QgsVirtualLayerSourceWidget( QWidget *parent )
  : QWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  layout->addWidget( mLineEdit, 1 );

  QPushButton *browseButton = new QPushButton( tr( "…" ) );
  browseButton->setToolTip( tr( "Browse for Layer" ) );
  connect( browseButton, &QPushButton::clicked, this, &QgsVirtualLayerSourceWidget::browseForLayer );
  layout->addWidget( browseButton );

  setLayout( layout );
}

void QgsVirtualLayerSourceWidget::setBrowserModel( QgsBrowserModel *model )
{
  mBrowserModel = model;
}

void QgsVirtualLayerSourceWidget::setSource( const QString &source, const QString &provider )
{
  mLineEdit->setText( source );
  mProvider = provider;
}

QString QgsVirtualLayerSourceWidget::source() const
{
  return mLineEdit->text();
}

QString QgsVirtualLayerSourceWidget::provider() const
{
  return mProvider;
}

void QgsVirtualLayerSourceWidget::browseForLayer()
{
  QgsDataSourceSelectDialog dlg( qobject_cast< QgsBrowserGuiModel * >( mBrowserModel ), true, QgsMapLayerType::VectorLayer, this );
  dlg.setWindowTitle( tr( "Select Layer Source" ) );

  QString source = mLineEdit->text();
  const QVariantMap sourceParts = QgsProviderRegistry::instance()->decodeUri( mProvider, source );
  if ( sourceParts.contains( QStringLiteral( "path" ) ) )
  {
    const QString path = sourceParts.value( QStringLiteral( "path" ) ).toString();
    const QString closestPath = QFile::exists( path ) ? path : QgsFileUtils::findClosestExistingPath( path );
    source.replace( path, QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( closestPath ).toString(),
                    path ) );
  }
  dlg.setDescription( tr( "Current source: %1" ).arg( source ) );

  if ( !dlg.exec() )
    return;

  mLineEdit->setText( dlg.uri().uri );
  mProvider = dlg.uri().providerKey;

  emit sourceChanged( dlg.uri().uri, dlg.uri().providerKey );
}
