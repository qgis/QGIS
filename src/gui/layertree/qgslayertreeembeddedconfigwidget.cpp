#include "qgslayertreeembeddedconfigwidget.h"

#include "qgslayertree.h"
#include "qgslayertreeembeddedwidgetregistry.h"

#include <QStringListModel>
#include <QStandardItemModel>

QgsLayerTreeEmbeddedConfigWidget::QgsLayerTreeEmbeddedConfigWidget( QgsLayerTreeLayer* nodeLayer, QWidget* parent )
    : QWidget( parent )
    , mNodeLayer( nodeLayer )
{
  setupUi( this );

  connect( mBtnAdd, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
  connect( mBtnRemove, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );

  QStandardItemModel* modelAvailable = new QStandardItemModel( this );
  QStandardItemModel* modelUsed = new QStandardItemModel( this );

  // populate available
  Q_FOREACH ( const QString& providerId, QgsLayerTreeEmbeddedWidgetRegistry::instance()->providers() )
  {
    QgsLayerTreeEmbeddedWidgetProvider* provider = QgsLayerTreeEmbeddedWidgetRegistry::instance()->provider( providerId );
    QStandardItem* item = new QStandardItem( provider->name() );
    item->setData( provider->id(), Qt::UserRole + 1 );
    modelAvailable->appendRow( item );
  }
  mListAvailable->setModel( modelAvailable );

  // populate used
  int widgetsCount = nodeLayer->customProperty( "embeddedWidgets/count", 0 ).toInt();
  for ( int i = 0; i < widgetsCount; ++i )
  {
    QString providerId = nodeLayer->customProperty( QString( "embeddedWidgets/%1/id" ).arg( i ) ).toString();
    if ( QgsLayerTreeEmbeddedWidgetProvider* provider = QgsLayerTreeEmbeddedWidgetRegistry::instance()->provider( providerId ) )
    {
      QStandardItem* item = new QStandardItem( provider->name() );
      item->setData( provider->id(), Qt::UserRole + 1 );
      modelUsed->appendRow( item );
    }
  }
  mListUsed->setModel( modelUsed );
}

void QgsLayerTreeEmbeddedConfigWidget::onAddClicked()
{
  if ( !mListAvailable->currentIndex().isValid() )
    return;

  QString providerId = mListAvailable->model()->data( mListAvailable->currentIndex(), Qt::UserRole + 1 ).toString();
  QgsLayerTreeEmbeddedWidgetProvider* provider = QgsLayerTreeEmbeddedWidgetRegistry::instance()->provider( providerId );
  if ( !provider )
    return;

  if ( QStandardItemModel* model = qobject_cast<QStandardItemModel*>( mListUsed->model() ) )
  {
    QStandardItem* item = new QStandardItem( provider->name() );
    item->setData( provider->id(), Qt::UserRole + 1 );
    model->appendRow( item );
  }

  updateCustomProperties();
}

void QgsLayerTreeEmbeddedConfigWidget::onRemoveClicked()
{
  if ( !mListUsed->currentIndex().isValid() )
    return;

  int row = mListUsed->currentIndex().row();
  mListUsed->model()->removeRow( row );

  updateCustomProperties();
}

void QgsLayerTreeEmbeddedConfigWidget::updateCustomProperties()
{
  // clear old properties
  int widgetsCount = mNodeLayer->customProperty( "embeddedWidgets/count", 0 ).toInt();
  for ( int i = 0; i < widgetsCount; ++i )
  {
    mNodeLayer->removeCustomProperty( QString( "embeddedWidgets/%1/id" ).arg( i ) );
  }

  // setup new properties
  int newCount = mListUsed->model()->rowCount();
  mNodeLayer->setCustomProperty( "embeddedWidgets/count", newCount );
  for ( int i = 0; i < newCount; ++i )
  {
    QString providerId = mListUsed->model()->data( mListUsed->model()->index( i, 0 ), Qt::UserRole + 1 ).toString();
    mNodeLayer->setCustomProperty( QString( "embeddedWidgets/%1/id" ).arg( i ), providerId );
  }
}
