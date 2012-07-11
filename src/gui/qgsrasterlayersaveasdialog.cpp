#include "qgsrasterlayersaveasdialog.h"
#include "qgsrasterdataprovider.h"
#include <QFileDialog>

QgsRasterLayerSaveAsDialog::QgsRasterLayerSaveAsDialog( QgsRasterDataProvider* sourceProvider, const QgsRectangle& currentExtent,
    QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ),
    mDataProvider( sourceProvider ), mCurrentExtent( currentExtent )

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
      mColumnsLineEdit->setText( QString::number( 6000 ) );
      mRowsLineEdit->setText( QString::number( 6000 ) );
      mMaximumSizeXLineEdit->setText( QString::number( 2000 ) );
      mMaximumSizeYLineEdit->setText( QString::number( 2000 ) );
    }

    //extent
    setOutputExtent( mCurrentExtent );
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
  mXMinLineEdit->setValidator( new QDoubleValidator( this ) );
  mXMaxLineEdit->setValidator( new QDoubleValidator( this ) );
  mYMinLineEdit->setValidator( new QDoubleValidator( this ) );
  mYMaxLineEdit->setValidator( new QDoubleValidator( this ) );
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

  okButton->setEnabled( QFileInfo( text ).absoluteDir().exists() );
}

void QgsRasterLayerSaveAsDialog::on_mCurrentExtentButton_clicked()
{
  setOutputExtent( mCurrentExtent );
}

void QgsRasterLayerSaveAsDialog::on_mProviderExtentButton_clicked()
{
  if ( mDataProvider )
  {
    setOutputExtent( mDataProvider->extent() );
  }
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

QgsRectangle QgsRasterLayerSaveAsDialog::outputRectangle() const
{
  return QgsRectangle( mXMinLineEdit->text().toDouble(), mYMinLineEdit->text().toDouble(), mXMaxLineEdit->text().toDouble(), mYMaxLineEdit->text().toDouble() );
}

void QgsRasterLayerSaveAsDialog::setOutputExtent( const QgsRectangle& r )
{
  mXMinLineEdit->setText( QString::number( r.xMinimum() ) );
  mXMaxLineEdit->setText( QString::number( r.xMaximum() ) );
  mYMinLineEdit->setText( QString::number( r.yMinimum() ) );
  mYMaxLineEdit->setText( QString::number( r.yMaximum() ) );
}

void QgsRasterLayerSaveAsDialog::hideFormat()
{
  mFormatLabel->hide();
  mFormatComboBox->hide();
}

void QgsRasterLayerSaveAsDialog::hideOutput()
{
  mSaveAsLabel->hide();
  mSaveAsLineEdit->hide();
  mBrowseButton->hide();
}
