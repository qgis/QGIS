#include "qgsrelationadddlg.h"
#include "qgsvectorlayer.h"

QgsRelationAddDlg::QgsRelationAddDlg( QWidget *parent ) :
    QDialog( parent )
{
  setupUi( this );

  mTxtRelationId->setPlaceholderText( tr( "[Generated automatically]" ) );
}

void QgsRelationAddDlg::addLayers( QList< QgsVectorLayer* > layers )
{
  mCbxReferencingLayer->addItem( "", "" );
  mCbxReferencedLayer->addItem( "", "" );

  Q_FOREACH ( QgsVectorLayer* layer, layers )
  {
    mCbxReferencingLayer->addItem( layer->name(), layer->id() );
    mCbxReferencedLayer->addItem( layer->name(), layer->id() );

    mLayers.insert( layer->id(), layer );
  }
}

QString QgsRelationAddDlg::referencingLayerId()
{
  return mCbxReferencingLayer->itemData( mCbxReferencingLayer->currentIndex() ).toString();
}

QString QgsRelationAddDlg::referencedLayerId()
{
  return mCbxReferencedLayer->itemData( mCbxReferencedLayer->currentIndex() ).toString();
}

QList< QPair< QString, QString > > QgsRelationAddDlg::references()
{
  QList< QPair< QString, QString > > references;

  QString referencingField = mCbxReferencingField->itemData( mCbxReferencingField->currentIndex() ).toString();
  QString referencedField = mCbxReferencedField->itemData( mCbxReferencedField->currentIndex() ).toString();

  references.append( QPair<QString, QString> ( referencingField, referencedField ) );

  return references;
}

QString QgsRelationAddDlg::relationId()
{
  return mTxtRelationId->text();
}

QString QgsRelationAddDlg::relationName()
{
  return mTxtRelationName->text();
}

void QgsRelationAddDlg::on_mCbxReferencingLayer_currentIndexChanged( int index )
{
  loadLayerAttributes( mCbxReferencingField, mLayers[mCbxReferencingLayer->itemData( index ).toString()] );
}

void QgsRelationAddDlg::on_mCbxReferencedLayer_currentIndexChanged( int index )
{
  loadLayerAttributes( mCbxReferencedField, mLayers[mCbxReferencedLayer->itemData( index ).toString()] );
}

void QgsRelationAddDlg::loadLayerAttributes( QComboBox* cbx, QgsVectorLayer* layer )
{
  cbx->clear();

  if ( !layer )
  {
    return;
  }

  Q_FOREACH ( const QgsField& fld, layer->fields().toList() )
  {
    cbx->addItem( fld.name(), fld.name() );
  }
}
