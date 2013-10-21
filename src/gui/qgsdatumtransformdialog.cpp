#include "qgsdatumtransformdialog.h"

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
      itemText = QString::number( it->at( i ) );
      if ( itemText.compare( "-1" ) != 0 )
      {
        item->setText( i, itemText );
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
      bool conversionOk = false;
      QString itemText = item->text( i );
      int transformNr = itemText.toInt( &conversionOk );
      if ( !itemText.isEmpty() && conversionOk )
      {
        list << transformNr;
      }
    }
  }

  return list;
}
