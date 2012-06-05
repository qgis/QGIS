#include "qgsrasterlayersaveasdialog.h"
#include "qgsrasterdataprovider.h"
#include <QFileDialog>

QgsRasterLayerSaveAsDialog::QgsRasterLayerSaveAsDialog( QgsRasterDataProvider* sourceProvider,
    QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ),
    mDataProvider( sourceProvider )

{
  setupUi( this );
  setValidators();

  //fill reasonable default values depending on the provider
  if ( mDataProvider )
  {
    if ( mDataProvider->capabilities() & QgsRasterDataProvider::ExactResolution )
    {
      int xSize = mDataProvider->xSize();
      int ySize = mDataProvider->ySize();
      mColumnsLineEdit->setText( QString::number( xSize ) );
      mRowsLineEdit->setText( QString::number( ySize ) );
      mMaximumSizeXLineEdit->setText( QString::number( xSize ) );
      mMaximumSizeYLineEdit->setText( QString::number( ySize ) );
    }
    else //wms
    {
      mTileModeCheckBox->setChecked( true );
      mColumnsLineEdit->setText( QString::number( 10000 ) );
      mRowsLineEdit->setText( QString::number( 10000 ) );
      mMaximumSizeXLineEdit->setText( QString::number( 2000 ) );
      mMaximumSizeYLineEdit->setText( QString::number( 2000 ) );
    }
  }

  QPushButton* okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
  {
    okButton->setEnabled( false );
  }
}

void QgsRasterLayerSaveAsDialog::setValidators()
{
  mColumnsLineEdit->setValidator( new QIntValidator( this ) );
  mRowsLineEdit->setValidator( new QIntValidator( this ) );
  mMaximumSizeXLineEdit->setValidator( new QIntValidator( this ) );
  mMaximumSizeYLineEdit->setValidator( new QIntValidator( this ) );
}

QgsRasterLayerSaveAsDialog::~QgsRasterLayerSaveAsDialog()
{
}

void QgsRasterLayerSaveAsDialog::on_mBrowseButton_clicked()
{
  QString fileName;
  if ( mTileModeCheckBox->isChecked() )
  {
    fileName = QFileDialog::getExistingDirectory( this, tr( "Select output directory" ) );
  }
  else
  {
    fileName = QFileDialog::getOpenFileName( this, tr( "Select output file" ) );
  }

  if ( !fileName.isEmpty() )
  {
    mSaveAsLineEdit->setText( fileName );
  }
}

void QgsRasterLayerSaveAsDialog::on_mSaveAsLineEdit_textChanged( const QString& text )
{
  QPushButton* okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( !okButton )
  {
    return;
  }

  okButton->setEnabled( QFile::exists( text ) );
}

int QgsRasterLayerSaveAsDialog::nColumns() const
{
  return mColumnsLineEdit->text().toInt();
}

int QgsRasterLayerSaveAsDialog::nRows() const
{
  return mRowsLineEdit->text().toInt();
}

int QgsRasterLayerSaveAsDialog::maximumTileSizeX() const
{
  return mMaximumSizeXLineEdit->text().toInt();
}

int QgsRasterLayerSaveAsDialog::maximumTileSizeY() const
{
  return mMaximumSizeYLineEdit->text().toInt();
}

bool QgsRasterLayerSaveAsDialog::tileMode() const
{
  return mTileModeCheckBox->isChecked();
}

QString QgsRasterLayerSaveAsDialog::outputFileName() const
{
  return mSaveAsLineEdit->text();
}

QString QgsRasterLayerSaveAsDialog::outputFormat() const
{
  return ""; //soon...
}
