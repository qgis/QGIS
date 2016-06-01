#include "qgslayertreeembeddedconfigwidget.h"

#include "qgslayertree.h"
#include "qgslayertreeembeddedwidgetregistry.h"

#include <QStringListModel>

QgsLayerTreeEmbeddedConfigWidget::QgsLayerTreeEmbeddedConfigWidget( QgsLayerTreeLayer* nodeLayer, QWidget* parent )
    : QWidget( parent )
    , mNodeLayer( nodeLayer )
{
  setupUi( this );

  connect( mBtnAdd, SIGNAL(clicked(bool)), this, SLOT(onAddClicked()));
  connect( mBtnRemove, SIGNAL(clicked(bool)), this, SLOT(onRemoveClicked()));

  // populate available
  QStringList lst;
  Q_FOREACH ( const QString& providerId, QgsLayerTreeEmbeddedWidgetRegistry::instance()->providers() )
  {
    QgsLayerTreeEmbeddedWidgetProvider* provider = QgsLayerTreeEmbeddedWidgetRegistry::instance()->provider( providerId );
    lst << provider->id();
  }
  mListAvailable->setModel( new QStringListModel( lst, this ) );

  // populate used
  QStringList lstUsed;
  int widgetsCount = nodeLayer->customProperty( "embeddedWidgets/count", 0 ).toInt();
  for ( int i = 0; i < widgetsCount; ++i )
  {
    QString providerId = nodeLayer->customProperty( QString( "embeddedWidgets/%1/id" ).arg( i ) ).toString();
    if ( /*QgsLayerTreeEmbeddedWidgetProvider* provider =*/ QgsLayerTreeEmbeddedWidgetRegistry::instance()->provider( providerId ) )
    {
      lstUsed << providerId;
    }
  }
  mListUsed->setModel( new QStringListModel( lstUsed, this ) );
}

void QgsLayerTreeEmbeddedConfigWidget::onAddClicked()
{
  if (!mListAvailable->currentIndex().isValid())
    return;

  QString providerId = mListAvailable->model()->data( mListAvailable->currentIndex() ).toString();
  if (QStringListModel* model = qobject_cast<QStringListModel*>(mListUsed->model()))
  {
    int row = model->rowCount();
    model->insertRow( row );
    model->setData( model->index(row, 0), providerId );
    //QStringList lst = model->stringList();
    //lst << providerId;
    //model->setStringList( lst );
  }

  updateCustomProperties();
}

void QgsLayerTreeEmbeddedConfigWidget::onRemoveClicked()
{
  if (!mListUsed->currentIndex().isValid())
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
    QString providerId = mListUsed->model()->data( mListUsed->model()->index(i, 0) ).toString();
    mNodeLayer->setCustomProperty( QString( "embeddedWidgets/%1/id" ).arg( i ), providerId );
  }
}
