#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgscoordinatetransform.h"
#include "qgsrasterlayersaveasdialog.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterformatsaveoptionswidget.h"
#include "qgsgenericprojectionselector.h"

#include <QFileDialog>
#include <QSettings>


QgsRasterLayerSaveAsDialog::QgsRasterLayerSaveAsDialog( QgsRasterDataProvider* sourceProvider, const QgsRectangle& currentExtent,
    const QgsCoordinateReferenceSystem& layerCrs,
    const QgsCoordinateReferenceSystem& currentCrs,
    QWidget* parent, Qt::WindowFlags f ):
    QDialog( parent, f )
    , mDataProvider( sourceProvider )
    , mCurrentExtent( currentExtent )
    , mLayerCrs( layerCrs )
    , mCurrentCrs( currentCrs )
    , mExtentState( OriginalExtent )
    , mResolutionState( OriginalResolution )
{
  setupUi( this );
  mAddNoDataManuallyToolButton->setIcon( QgsApplication::getThemeIcon( "/mActionNewAttribute.png" ) );
  mLoadTransparentNoDataToolButton->setIcon( QgsApplication::getThemeIcon( "/mActionCopySelected.png" ) );
  mRemoveSelectedNoDataToolButton->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  mRemoveAllNoDataToolButton->setIcon( QgsApplication::getThemeIcon( "/mActionRemove.png" ) );
  // mNoDataGroupBox->setEnabled( false ); // not yet implemented

  setValidators();
  // Translated labels + EPSG are updated later
  mCrsComboBox->addItem( "Layer", OriginalCrs );
  mCrsComboBox->addItem( "Project", CurrentCrs );
  mCrsComboBox->addItem( "Selected", UserCrs );

  toggleResolutionSize();
  mUserCrs.createFromOgcWmsCrs( "EPSG:4326" );

  //only one hardcoded format at the moment
  QStringList myFormats;
  myFormats << "GTiff";
  foreach ( QString myFormat, myFormats )
  {
    mFormatComboBox->addItem( myFormat );
  }

  //fill reasonable default values depending on the provider
  if ( mDataProvider )
  {
    //extent
    setOutputExtent( mDataProvider->extent(), mLayerCrs, OriginalExtent );

    if ( mDataProvider->capabilities() & QgsRasterDataProvider::ExactResolution )
    {
      setOriginalResolution();
      int xSize = mDataProvider->xSize();
      int ySize = mDataProvider->ySize();
      mMaximumSizeXLineEdit->setText( QString::number( xSize ) );
      mMaximumSizeYLineEdit->setText( QString::number( ySize ) );
    }
    else //wms, sometimes wcs
    {
      mTileModeCheckBox->setChecked( true );
      mMaximumSizeXLineEdit->setText( QString::number( 2000 ) );
      mMaximumSizeYLineEdit->setText( QString::number( 2000 ) );
    }

    mOptionsWidget->setProvider( mDataProvider->name() );
    if ( mDataProvider->name() == "gdal" )
    {
      mOptionsWidget->setFormat( myFormats[0] );
    }
    mOptionsWidget->update();

  }
  updateCrsGroup();

  QPushButton* okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
  {
    okButton->setEnabled( false );
  }

  // this should scroll down to make widget visible, but it's not happening
  // (at least part of it is visible)...
  connect( mCreateOptionsGroupBox, SIGNAL( expanded( QWidget* ) ),
           this, SLOT( groupBoxExpanded( QWidget* ) ) );

}

void QgsRasterLayerSaveAsDialog::setValidators()
{
  mXResolutionLineEdit->setValidator( new QDoubleValidator( this ) );
  mYResolutionLineEdit->setValidator( new QDoubleValidator( this ) );
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
    fileName = QFileDialog::getSaveFileName( this, tr( "Select output file" ) );
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
  setOutputExtent( mCurrentExtent, mCurrentCrs, CurrentExtent );
}

void QgsRasterLayerSaveAsDialog::on_mOriginalExtentButton_clicked()
{
  if ( mDataProvider )
  {
    setOutputExtent( mDataProvider->extent(), mLayerCrs, OriginalExtent );
  }
}

void QgsRasterLayerSaveAsDialog::on_mFormatComboBox_currentIndexChanged( const QString & text )
{
  //gdal-specific
  if ( mDataProvider && mDataProvider->name() == "gdal" )
  {
    mOptionsWidget->setFormat( text );
    mOptionsWidget->update();
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

double QgsRasterLayerSaveAsDialog::xResolution() const
{
  return mXResolutionLineEdit->text().toDouble();
}

double QgsRasterLayerSaveAsDialog::yResolution() const
{
  return mYResolutionLineEdit->text().toDouble();
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
  return mFormatComboBox->currentText();
}

QStringList QgsRasterLayerSaveAsDialog::createOptions() const
{
  return mOptionsWidget ? mOptionsWidget->options() : QStringList();
}

QgsRectangle QgsRasterLayerSaveAsDialog::outputRectangle() const
{
  return QgsRectangle( mXMinLineEdit->text().toDouble(), mYMinLineEdit->text().toDouble(), mXMaxLineEdit->text().toDouble(), mYMaxLineEdit->text().toDouble() );
}

void QgsRasterLayerSaveAsDialog::setOutputExtent( const QgsRectangle& r, const QgsCoordinateReferenceSystem& srcCrs, ExtentState state )
{
  QgsRectangle extent;
  if ( outputCrs() == srcCrs )
  {
    extent = r;
  }
  else
  {
    QgsCoordinateTransform ct( srcCrs, outputCrs() );
    extent = ct.transformBoundingBox( r );
  }

  mXMinLineEdit->setText( QString::number( extent.xMinimum() ) );
  mXMaxLineEdit->setText( QString::number( extent.xMaximum() ) );
  mYMinLineEdit->setText( QString::number( extent.yMinimum() ) );
  mYMaxLineEdit->setText( QString::number( extent.yMaximum() ) );

  mExtentState = state;
  extentChanged();
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
  QPushButton* okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
  {
    okButton->setEnabled( true );
  }
}

void QgsRasterLayerSaveAsDialog::toggleResolutionSize()
{
  bool hasResolution = mDataProvider && mDataProvider->capabilities() & QgsRasterDataProvider::ExactResolution;

  bool on = mResolutionRadioButton->isChecked();
  mXResolutionLineEdit->setEnabled( on );
  mYResolutionLineEdit->setEnabled( on );
  mOriginalResolutionPushButton->setEnabled( on && hasResolution );
  mColumnsLineEdit->setEnabled( !on );
  mRowsLineEdit->setEnabled( !on );
  mOriginalSizePushButton->setEnabled( !on && hasResolution );
}

void QgsRasterLayerSaveAsDialog::setOriginalResolution()
{
  double xRes, yRes;

  if ( mDataProvider->capabilities() & QgsRasterDataProvider::ExactResolution )
  {
    xRes = mDataProvider->extent().width() / mDataProvider->xSize();
    yRes = mDataProvider->extent().height() / mDataProvider->ySize();
  }
  else
  {
    // Init to something if no original resolution is available
    xRes = yRes = mDataProvider->extent().width() / 100;
  }
  setResolution( xRes, yRes, mLayerCrs );
  mResolutionState = OriginalResolution;
  recalcSize();
}

void QgsRasterLayerSaveAsDialog::setResolution( double xRes, double yRes, const QgsCoordinateReferenceSystem& srcCrs )
{
  if ( srcCrs != outputCrs() )
  {
    // We reproject pixel rectangle from center of selected extent, of course, it gives
    // bigger xRes,yRes than reprojected edges (envelope), it may also be that
    // close to margins are higher resolutions (even very, too high)
    // TODO: consider more precise resolution calculation

    QgsPoint center = outputRectangle().center();
    QgsCoordinateTransform ct( srcCrs, outputCrs() );
    QgsPoint srsCenter = ct.transform( center, QgsCoordinateTransform::ReverseTransform );

    QgsRectangle srcExtent( srsCenter.x() - xRes / 2, srsCenter.y() - yRes / 2, srsCenter.x() + xRes / 2, srsCenter.y() + yRes / 2 );

    QgsRectangle extent =  ct.transform( srcExtent );
    xRes = extent.width();
    yRes = extent.height();
  }
  mXResolutionLineEdit->setText( QString::number( xRes ) );
  mYResolutionLineEdit->setText( QString::number( yRes ) );
}

void QgsRasterLayerSaveAsDialog::recalcSize()
{
  QgsDebugMsg( "Entered" );
  QgsRectangle extent = outputRectangle();
  int xSize =  xResolution() != 0 ? static_cast<int>( qRound( extent.width() / xResolution() ) ) : 0;
  int ySize =  yResolution() != 0 ? static_cast<int>( qRound( extent.height() / yResolution() ) ) : 0;
  mColumnsLineEdit->setText( QString::number( xSize ) );
  mRowsLineEdit->setText( QString::number( ySize ) );
  updateResolutionStateMsg();
}

void QgsRasterLayerSaveAsDialog::setOriginalSize()
{
  mColumnsLineEdit->setText( QString::number( mDataProvider->xSize() ) );
  mRowsLineEdit->setText( QString::number( mDataProvider->ySize() ) );
  recalcResolution();
}

void QgsRasterLayerSaveAsDialog::recalcResolution()
{
  QgsDebugMsg( "Entered" );
  QgsRectangle extent = outputRectangle();
  double xRes = nColumns() != 0 ? extent.width() / nColumns() : 0;
  double yRes = nRows() != 0 ? extent.height() / nRows() : 0;
  mXResolutionLineEdit->setText( QString::number( xRes ) );
  mYResolutionLineEdit->setText( QString::number( yRes ) );
  updateResolutionStateMsg();
}

void QgsRasterLayerSaveAsDialog::recalcResolutionSize()
{
  QgsDebugMsg( "Entered" );
  if ( mResolutionRadioButton->isChecked() )
  {
    recalcSize();
  }
  else
  {
    mResolutionState = UserResolution;
    recalcResolution();
  }
}

void QgsRasterLayerSaveAsDialog::updateResolutionStateMsg()
{
  QString msg;
  switch ( mResolutionState )
  {
    case OriginalResolution:
      msg = tr( "layer" );
      break;
    case UserResolution:
      msg = tr( "user defined" );
      break;
    default:
      break;
  }
  msg = tr( "Resolution" ) + " (" + tr( "current" ) + ": " + msg + ")";
  mResolutionGroupBox->setTitle( msg );
}

void QgsRasterLayerSaveAsDialog::extentChanged()
{
  updateExtentStateMsg();
  // Whenever extent changes with fixed size, original resolution is lost
  if ( mSizeRadioButton->isChecked() )
  {
    mResolutionState = UserResolution;
  }
  recalcResolutionSize();
}

void QgsRasterLayerSaveAsDialog::updateExtentStateMsg()
{
  QString msg;
  switch ( mExtentState )
  {
    case OriginalExtent:
      msg = tr( "layer" );
      break;
    case CurrentExtent:
      msg = tr( "map view" );
      break;
    case UserExtent:
      msg = tr( "user defined" );
      break;
    default:
      break;
  }
  msg = tr( "Extent" ) + " (" + tr( "current" ) + ": " + msg + ")";
  mExtentGroupBox->setTitle( msg );
}

void QgsRasterLayerSaveAsDialog::on_mChangeCrsPushButton_clicked()
{
  QgsGenericProjectionSelector * selector = new QgsGenericProjectionSelector( this );
  selector->setMessage();
  selector->setSelectedCrsId( mUserCrs.srsid() );
  if ( selector->exec() )
  {
    mUserCrs.createFromId( selector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( UserCrs ) );
  }
  delete selector;
  crsChanged();
}

void QgsRasterLayerSaveAsDialog::crsChanged()
{
  QgsDebugMsg( "Entered" );
  if ( outputCrs() != mPreviousCrs )
  {
    // Reset extent
    QgsRectangle previousExtent;
    QgsCoordinateReferenceSystem previousCrs;
    // We could reproject previous but that would add additional space also if
    // it is was not necessary or at leas it could decrease accuracy
    if ( mExtentState == OriginalExtent )
    {
      previousExtent = mDataProvider->extent();
      previousCrs = mLayerCrs;
    }
    else if ( mExtentState == CurrentExtent )
    {
      previousExtent = mCurrentExtent;
      previousCrs = mCurrentCrs;
    }
    else
    {
      previousExtent = outputRectangle();
      previousCrs = mPreviousCrs;
    }
    setOutputExtent( previousExtent, previousCrs, mExtentState );

    // Reset resolution
    if ( mResolutionRadioButton->isChecked() )
    {
      if ( mResolutionState == OriginalResolution )
      {
        setOriginalResolution();
      }
      else
      {
        // reset from present resolution and present crs
        setResolution( xResolution(), yResolution(), mPreviousCrs );
      }
    }
    else
    {
      // Size does not change, we just recalc resolution from new extent
      recalcResolution();
    }
  }
  mPreviousCrs = outputCrs();
  updateCrsGroup();
}

void QgsRasterLayerSaveAsDialog::updateCrsGroup()
{
  QgsDebugMsg( "Entered" );

  mCrsComboBox->setItemText( mCrsComboBox->findData( OriginalCrs ),
                             tr( "Layer" ) + " (" + mLayerCrs.description() + ", " + mLayerCrs.authid() + ")" );

  mCrsComboBox->setItemText( mCrsComboBox->findData( CurrentCrs ),
                             tr( "Project" ) + " (" + mCurrentCrs.description() + ", " + mCurrentCrs.authid() + ")" );

  mCrsComboBox->setItemText( mCrsComboBox->findData( UserCrs ),
                             tr( "Selected" ) + " (" + mUserCrs.description() + ", " + mUserCrs.authid() + ")" );
}

QgsCoordinateReferenceSystem QgsRasterLayerSaveAsDialog::outputCrs()
{
  int state = mCrsComboBox->itemData( mCrsComboBox->currentIndex() ).toInt();
  if ( state == OriginalCrs )
  {
    return mLayerCrs;
  }
  else if ( state == CurrentCrs )
  {
    return mCurrentCrs;
  }
  return mUserCrs;
}

QgsRasterLayerSaveAsDialog::Mode QgsRasterLayerSaveAsDialog::mode() const
{
  if ( mRenderedModeRadioButton->isChecked() ) return RenderedImageMode;
  return RawDataMode;
}
