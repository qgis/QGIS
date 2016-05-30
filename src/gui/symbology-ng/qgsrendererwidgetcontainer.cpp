#include <QDialogButtonBox>
#include <QPushButton>

#include "qgsrendererwidgetcontainer.h"

QgsRendererWidgetContainer::QgsRendererWidgetContainer( QWidget *widget, const QString& title, QWidget *parent )
    : QWidget( parent )
{
  setupUi( this );
  mWidgetLayout->addWidget( widget );
  mWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
  mTitleText->setText( title );
  QPushButton* button = mButtonBox->button( QDialogButtonBox::Close );
  button->setDefault( true );
  connect( button, SIGNAL( pressed() ), this, SIGNAL( accepted() ) );
}

QWidget *QgsRendererWidgetContainer::widget()
{
  return mWidgetLayout->itemAt( 0 )->widget() ;
}

void QgsRendererWidgetContainer::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    emit accepted();
  }
}
