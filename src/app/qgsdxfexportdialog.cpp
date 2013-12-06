#include "qgsdxfexportdialog.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgis.h"
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>

QgsDxfExportDialog::QgsDxfExportDialog( const QList<QgsMapLayer*>& layerKeys, QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  connect( mFileLineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( setOkEnabled() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( saveSettings() ) );

  QList<QgsMapLayer*>::const_iterator layerIt = layerKeys.constBegin();
  for ( ; layerIt != layerKeys.constEnd(); ++layerIt )
  {
    QgsMapLayer* layer = *layerIt;
    if ( layer )
    {
      if ( layer->type() == QgsMapLayer::VectorLayer )
      {
        QListWidgetItem* layerItem = new QListWidgetItem( layer->name() );
        layerItem->setData( Qt::UserRole, layer->id() );
        layerItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        layerItem->setCheckState( Qt::Checked );
        mLayersListWidget->addItem( layerItem );
      }
    }
  }

  //last dxf symbology mode
  QSettings s;
  mSymbologyModeComboBox->setCurrentIndex( s.value( "qgis/lastDxfSymbologyMode", "2" ).toInt() );
  //last symbol scale
  mSymbologyScaleLineEdit->setText( s.value( "qgis/lastSymbologyExportScale", "50000" ).toString() );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

QgsDxfExportDialog::~QgsDxfExportDialog()
{

}

QList<QString> QgsDxfExportDialog::layers() const
{
  QList<QString> layerKeyList;
  int nItems = mLayersListWidget->count();
  for ( int i = 0; i < nItems; ++i )
  {
    QListWidgetItem* currentItem = mLayersListWidget->item( i );
    if ( currentItem->checkState() == Qt::Checked )
    {
      layerKeyList.prepend( currentItem->data( Qt::UserRole ).toString() );
    }
  }
  return layerKeyList;
}

double QgsDxfExportDialog::symbologyScale() const
{
  double scale = mSymbologyScaleLineEdit->text().toDouble();
  if ( qgsDoubleNear( scale, 0.0 ) )
  {
    return 1.0;
  }
  return scale;
}

QString QgsDxfExportDialog::saveFile() const
{
  return mFileLineEdit->text();
}

QgsDxfExport::SymbologyExport QgsDxfExportDialog::symbologyMode() const
{
  return ( QgsDxfExport::SymbologyExport )mSymbologyModeComboBox->currentIndex();
}

void QgsDxfExportDialog::on_mFileSelectionButton_clicked()
{
  //get last dxf save directory
  QSettings s;
  QString lastSavePath = s.value( "qgis/lastDxfDir" ).toString();

  QString filePath = QFileDialog::getSaveFileName( 0, tr( "Export as DXF" ), lastSavePath, tr( "DXF files *.dxf *.DXF" ) );
  if ( !filePath.isEmpty() )
  {
    mFileLineEdit->setText( filePath );
  }
}

void QgsDxfExportDialog::setOkEnabled()
{
  QPushButton* btn = buttonBox->button( QDialogButtonBox::Ok );

  QString filePath = mFileLineEdit->text();
  if ( filePath.isEmpty() )
  {
    btn->setEnabled( false );
  }

  QFileInfo fi( filePath );
  btn->setEnabled( fi.absoluteDir().exists() );
}

void QgsDxfExportDialog::saveSettings()
{
  QSettings s;

  //last dxf dir
  QFileInfo dxfFileInfo( mFileLineEdit->text() );
  s.setValue( "qgis/lastDxfDir", dxfFileInfo.absolutePath() );

  //last dxf symbology mode
  s.setValue( "qgis/lastDxfSymbologyMode", mSymbologyModeComboBox->currentIndex() );
  s.setValue( "qgis/lastSymbologyExportScale", mSymbologyScaleLineEdit->text() );
}
