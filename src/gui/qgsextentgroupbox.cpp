#include "qgsextentgroupbox.h"

#include "qgscoordinatetransform.h"
#include "qgsrasterblock.h"

QgsExtentGroupBox::QgsExtentGroupBox( QWidget* parent )
    : QgsCollapsibleGroupBox( parent )
    , mTitleBase( tr( "Extent" ) )
    , mExtentState( OriginalExtent )
{
  setupUi( this );

  mXMinLineEdit->setValidator( new QDoubleValidator( this ) );
  mXMaxLineEdit->setValidator( new QDoubleValidator( this ) );
  mYMinLineEdit->setValidator( new QDoubleValidator( this ) );
  mYMaxLineEdit->setValidator( new QDoubleValidator( this ) );

  connect( mCurrentExtentButton, SIGNAL( clicked() ), this, SLOT( setOutputExtentFromCurrent() ) );
  connect( mOriginalExtentButton, SIGNAL( clicked() ), this, SLOT( setOutputExtentFromOriginal() ) );
  connect( this, SIGNAL( clicked( bool ) ), this, SLOT( groupBoxClicked() ) );
}


void QgsExtentGroupBox::setOriginalExtent( const QgsRectangle& originalExtent, const QgsCoordinateReferenceSystem& originalCrs )
{
  mOriginalExtent = originalExtent;
  mOriginalCrs = originalCrs;
}


void QgsExtentGroupBox::setCurrentExtent( const QgsRectangle& currentExtent, const QgsCoordinateReferenceSystem& currentCrs )
{
  mCurrentExtent = currentExtent;
  mCurrentCrs = currentCrs;
}

void QgsExtentGroupBox::setOutputCrs( const QgsCoordinateReferenceSystem& outputCrs )
{
  mOutputCrs = outputCrs;
}


void QgsExtentGroupBox::setOutputExtent( const QgsRectangle& r, const QgsCoordinateReferenceSystem& srcCrs, ExtentState state )
{
  QgsRectangle extent;
  if ( mOutputCrs == srcCrs )
  {
    extent = r;
  }
  else
  {
    QgsCoordinateTransform ct( srcCrs, mOutputCrs );
    extent = ct.transformBoundingBox( r );
  }

  mXMinLineEdit->setText( QgsRasterBlock::printValue( extent.xMinimum() ) );
  mXMaxLineEdit->setText( QgsRasterBlock::printValue( extent.xMaximum() ) );
  mYMinLineEdit->setText( QgsRasterBlock::printValue( extent.yMinimum() ) );
  mYMaxLineEdit->setText( QgsRasterBlock::printValue( extent.yMaximum() ) );

  mExtentState = state;

  if ( isCheckable() && !isChecked() )
    setChecked( true );

  updateTitle();

  emit extentChanged( extent );
}


void QgsExtentGroupBox::setOutputExtentFromLineEdit()
{
  mExtentState = UserExtent;

  updateTitle();

  emit extentChanged( outputExtent() );
}


void QgsExtentGroupBox::updateTitle()
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
  if ( isCheckable() && !isChecked() )
    msg = tr( "none" );
  msg = tr( "%1 (current: %2)" ).arg( mTitleBase ).arg( msg );

  setTitle( msg );
}


void QgsExtentGroupBox::setOutputExtentFromCurrent()
{
  setOutputExtent( mCurrentExtent, mCurrentCrs, CurrentExtent );
}


void QgsExtentGroupBox::setOutputExtentFromOriginal()
{
  setOutputExtent( mOriginalExtent, mOriginalCrs, OriginalExtent );
}


void QgsExtentGroupBox::setOutputExtentFromUser( const QgsRectangle& extent, const QgsCoordinateReferenceSystem& crs )
{
  setOutputExtent( extent, crs, UserExtent );
}

void QgsExtentGroupBox::groupBoxClicked()
{
  if ( !isCheckable() )
    return;

  updateTitle();

  // output extent just went from null to something (or vice versa)
  emit extentChanged( outputExtent() );
}


QgsRectangle QgsExtentGroupBox::outputExtent() const
{
  if ( isCheckable() && !isChecked() )
    return QgsRectangle();

  return QgsRectangle( mXMinLineEdit->text().toDouble(), mYMinLineEdit->text().toDouble(),
                       mXMaxLineEdit->text().toDouble(), mYMaxLineEdit->text().toDouble() );
}

void QgsExtentGroupBox::setTitleBase(const QString& title)
{
  mTitleBase = title;
  updateTitle();
}

QString QgsExtentGroupBox::titleBase() const
{
  return mTitleBase;
}
