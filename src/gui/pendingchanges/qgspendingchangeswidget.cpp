
#include "qgspendingchangeswidget.h"

QgsPendingChangesWidget::QgsPendingChangesWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

}

void QgsPendingChangesWidget::setModel( QgsPendingChangesModel *model )
{
  this->mPendingTree->setModel( model );
}
