#include "qgsdatumtransformdialog.h"
#include "qgscoordinatetransform.h"

QgsDatumTransformDialog::QgsDatumTransformDialog( const QList< QList< int > >& dt, QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  QList< QList< int > >::const_iterator it = dt.constBegin();
  for ( ; it != dt.constEnd(); ++it )
  {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    QString itemText;
    for ( int i = 0; i < 2; ++i )
    {
      int nr = it->at( i );
      if ( nr != -1 )
      {
        item->setData( i, Qt::UserRole, nr );
        item->setText( i, QgsCoordinateTransform::datumTransformString( nr ) );
      }
    }
    mDatumTransformTreeWidget->addTopLevelItem( item );
  }
}

QgsDatumTransformDialog::~QgsDatumTransformDialog()
{
}

QgsDatumTransformDialog::QgsDatumTransformDialog(): QDialog()
{
  setupUi( this );
}

QList< int > QgsDatumTransformDialog::selectedDatumTransform()
{
  QList<int> list;
  QTreeWidgetItem * item = mDatumTransformTreeWidget->currentItem();
  if ( item )
  {
    for ( int i = 0; i < 2; ++i )
    {
      int transformNr = item->data( i, Qt::UserRole ).toInt();
      if ( transformNr != -1 )
      {
        list << transformNr;
      }
    }
  }
  return list;
}
