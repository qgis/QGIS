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

#include <layertree/qgslayertreeview.h>
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsmaplayerregistry.h>
#include <qgsgenericprojectionselector.h>
#include <layertree/qgslayertreemodel.h>
#include <layertree/qgslayertreegroup.h>
#include <layertree/qgslayertreelayer.h>

#include <QUrl>
#include <Qsci/qscilexer.h>
#include <QMessageBox>
#include <QTextStream>

QgsVirtualLayerSourceSelect::QgsVirtualLayerSourceSelect( QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mSrid( 0 )
{
  setupUi( this );

  QObject::connect( mTestButton, SIGNAL( clicked() ), this, SLOT( onTestQuery() ) );
  QObject::connect( mBrowseCRSBtn, SIGNAL( clicked() ), this, SLOT( onBrowseCRS() ) );

  QgsLayerTreeView* treeView = parent->findChild<QgsLayerTreeView*>( "theLayerTreeView" );
  if ( treeView )
  {
    QgsLayerTreeModel* model = qobject_cast<QgsLayerTreeModel*>( treeView->model() );
    foreach ( QgsLayerTreeLayer* layer, model->rootGroup()->findLayers() )
    {
      if ( layer->layer()->type() == QgsMapLayer::VectorLayer && static_cast<QgsVectorLayer*>( layer->layer() )->providerType() == "virtual" )
      {
        // store layer's id as user data
        mLayerNameCombo->addItem( layer->layer()->name(), layer->layer()->id() );
      }
    }
  }

  if ( mLayerNameCombo->count() == 0 )
    mLayerNameCombo->addItem( "virtual_layer" );

  // select the current layer, if any
  if ( treeView )
  {
    QList<QgsMapLayer*> selected = treeView->selectedLayers();
    if ( selected.size() == 1 && selected[0]->type() == QgsMapLayer::VectorLayer && static_cast<QgsVectorLayer*>( selected[0] )->providerType() == "virtual" )
    {
      mLayerNameCombo->setCurrentIndex( mLayerNameCombo->findData( selected[0]->id() ) );
    }
  }

  QObject::connect( mLayerNameCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onLayerComboChanged( int ) ) );
  onLayerComboChanged( mLayerNameCombo->currentIndex() );

  // configure auto completion with SQL functions
  QsciAPIs* apis = new QsciAPIs( mQueryEdit->lexer() );

  Q_INIT_RESOURCE( sqlfunctionslist );
  QFile fFile( ":/sqlfunctions/list.txt" );
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
  foreach ( QgsMapLayer* l, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    if ( l->type() == QgsMapLayer::VectorLayer )
    {
      apis->add( l->name() );
      QgsVectorLayer* vl = static_cast<QgsVectorLayer*>( l );
      foreach ( const QgsField& f, vl->fields().toList() )
      {
        apis->add( f.name() );
      }
    }
  }

  apis->prepare();
  mQueryEdit->lexer()->setAPIs( apis );

  mQueryEdit->setWrapMode( QsciScintilla::WrapWord );
}

QgsVirtualLayerSourceSelect::~QgsVirtualLayerSourceSelect()
{
}

void QgsVirtualLayerSourceSelect::onLayerComboChanged( int idx )
{
  if ( idx == -1 )
    return;

  QString lid = mLayerNameCombo->itemData( idx ).toString();
  QgsVectorLayer* l = static_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( lid ) );
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

  if ( def.geometryWkbType() == QgsWKBTypes::NoGeometry )
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
}

void QgsVirtualLayerSourceSelect::onBrowseCRS()
{
  QgsGenericProjectionSelector crsSelector( this );
  QgsCoordinateReferenceSystem crs( mSrid );
  crsSelector.setSelectedCrsId( crs.srsid() ); // convert postgis srid to internal id
  crsSelector.setMessage();
  if ( crsSelector.exec() )
  {
    mCRS->setText( crsSelector.selectedAuthId() );
    QgsCoordinateReferenceSystem newCrs( crsSelector.selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
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
    def.setGeometryWkbType( QgsWKBTypes::NoGeometry );
  }
  else if ( mGeometryRadio->isChecked() )
  {
    QgsWKBTypes::Type t = mGeometryType->currentIndex() > -1 ? static_cast<QgsWKBTypes::Type>( mGeometryType->currentIndex() + 1 ) : QgsWKBTypes::NoGeometry;
    def.setGeometryWkbType( t );
    def.setGeometryField( mGeometryField->text() );
    def.setGeometrySrid( mSrid );
  }
  return def;
}

void QgsVirtualLayerSourceSelect::onTestQuery()
{
  QgsVirtualLayerDefinition def = getVirtualLayerDef();

  QScopedPointer<QgsVectorLayer> vl( new QgsVectorLayer( def.toString(), "test", "virtual" ) );
  if ( vl->isValid() )
  {
    QMessageBox::information( nullptr, tr( "Virtual layer test" ), tr( "No error" ) );
  }
  else
  {
    QMessageBox::critical( nullptr, tr( "Virtual layer test" ), vl->dataProvider()->error().summary() );
  }
}

void QgsVirtualLayerSourceSelect::on_buttonBox_accepted()
{
  QString layerName = "virtual_layer";
  int idx = mLayerNameCombo->currentIndex();
  if ( idx != -1 && !mLayerNameCombo->currentText().isEmpty() )
  {
    layerName = mLayerNameCombo->currentText();
  }

  QgsVirtualLayerDefinition def = getVirtualLayerDef();

  if ( idx != -1 )
  {
    QString id( mLayerNameCombo->itemData( idx ).toString() );
    if ( !id.isEmpty() && mLayerNameCombo->currentText() == QgsMapLayerRegistry::instance()->mapLayer( id )->name() )
    {
      int r = QMessageBox::warning( nullptr, tr( "Warning" ), tr( "A virtual layer of this name already exists, would you like to overwrite it ?" ), QMessageBox::Yes | QMessageBox::No );
      if ( r == QMessageBox::Yes )
      {
        emit replaceVectorLayer( id, def.toString(), layerName, "virtual" );
        return;
      }
    }
  }
  emit addVectorLayer( def.toString(), layerName, "virtual" );
}

QGISEXTERN QgsVirtualLayerSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl )
{
  return new QgsVirtualLayerSourceSelect( parent, fl );
}

