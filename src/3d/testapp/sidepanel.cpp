#include "sidepanel.h"

#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>

SidePanel::SidePanel( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->setAlignment( Qt::AlignTop );
  vLayout->setMargin( 0 );

  labelFps = new QLabel( this );
  vLayout->addWidget( labelFps );

  //QLineEdit* e = new QLineEdit(this);
  //e->setText("hello");
  //vLayout->addWidget(e);

  setLayout( vLayout );
}

void SidePanel::setFps( float fps )
{
  labelFps->setText( QString( "fps %1" ).arg( fps ) );
}
