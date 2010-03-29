#include "qgsformannotationdialog.h"
#include "qgsannotationwidget.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>

QgsFormAnnotationDialog::QgsFormAnnotationDialog( QgsFormAnnotationItem* item, QWidget * parent, Qt::WindowFlags f ): \
    QDialog( parent, f ), mItem( item ), mEmbeddedWidget( 0 )
{
  setupUi( this );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mEmbeddedWidget->show();
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );

  if ( item )
  {
    mFileLineEdit->setText( item->designerForm() );
  }

  QObject::connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( applySettingsToItem() ) );
  QPushButton* deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, SIGNAL( clicked() ), this, SLOT( deleteItem() ) );
  mButtonBox->addButton( deleteButton, QDialogButtonBox::RejectRole );
}

QgsFormAnnotationDialog::~QgsFormAnnotationDialog()
{

}

void QgsFormAnnotationDialog::applySettingsToItem()
{
  //apply settings from embedded item widget
  if ( mEmbeddedWidget )
  {
    mEmbeddedWidget->apply();
  }

  if ( mItem )
  {
    mItem->setDesignerForm( mFileLineEdit->text() );
    mItem->update();
  }
}

void QgsFormAnnotationDialog::on_mBrowseToolButton_clicked()
{
  QString directory;
  QFileInfo fi( mFileLineEdit->text() );
  if ( fi.exists() )
  {
    directory = fi.absolutePath();
  }
  QString filename = QFileDialog::getOpenFileName( 0, tr( "Qt designer file" ), directory, "*.ui" );
  mFileLineEdit->setText( filename );
}

void QgsFormAnnotationDialog::deleteItem()
{
  QGraphicsScene* scene = mItem->scene();
  if ( scene )
  {
    scene->removeItem( mItem );
  }
  delete mItem;
  mItem = 0;
}

