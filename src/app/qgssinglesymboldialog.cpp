/***************************************************************************
                         qgssinglesymboldialog.cpp  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgssinglesymboldialog.h"
#include "qgsmarkercatalogue.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfield.h"
#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgslogger.h"

#include <QColorDialog>
#include <QPainter>
#include <QImage>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QKeyEvent>

#define DO_NOT_USE_STR "<off>"

QgsSingleSymbolDialog::QgsSingleSymbolDialog(): QDialog(), mVectorLayer( 0 )
{
  setupUi( this );
  QgsDebugMsg( "entered." );
}

QgsSingleSymbolDialog::QgsSingleSymbolDialog( QgsVectorLayer * layer, bool disabled ): QDialog(), mVectorLayer( layer ), mDisabled( disabled )
{
  setupUi( this );
  QgsDebugMsg( "entered." );

  //
  //set point symbol list
  //

  // If this layer doesn't have points, break out of the following
  // two loops after the first iteration. This gives one point
  // symbol in the dialog, etc so that other code can rely on such a
  // fact, but avoids the long time required to load all of the
  // available symbols when they are not needed.

  // NOTE BY Tim: I think the note above and the break out in the
  // loops can be removed now with changes I have made to use combo
  // boxes for line style and fill style...test and remove if poss.

  QAction *refreshAction = new QAction( tr( "Refresh markers" ), lstSymbols );
  lstSymbols->addAction( refreshAction );
  connect( refreshAction, SIGNAL( triggered() ), QgsMarkerCatalogue::instance(), SLOT( refreshList() ) );
  connect( QgsMarkerCatalogue::instance(), SIGNAL( markersRefreshed() ), this, SLOT( refreshMarkers() ) );
  lstSymbols->setContextMenuPolicy( Qt::ActionsContextMenu );

  //do the signal/slot connections
  connect( btnOutlineColor, SIGNAL( clicked() ), this, SLOT( selectOutlineColor() ) );
  connect( btnFillColor, SIGNAL( clicked() ), this, SLOT( selectFillColor() ) );
  connect( outlinewidthspinbox, SIGNAL( valueChanged( double ) ), this, SLOT( resendSettingsChanged() ) );
  connect( mLabelEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( resendSettingsChanged() ) );
  connect( lstSymbols, SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem * ) ),
           this, SLOT( symbolChanged( QListWidgetItem *, QListWidgetItem * ) ) );
  connect( mPointSizeSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( resendSettingsChanged() ) );
  connect( mPointSizeUnitsCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( resendSettingsChanged() ) );
  connect( mRotationClassificationComboBox, SIGNAL( currentIndexChanged( const QString & ) ),
           this, SLOT( resendSettingsChanged() ) );
  connect( mScaleClassificationComboBox, SIGNAL( currentIndexChanged( const QString & ) ),
           this, SLOT( resendSettingsChanged() ) );
  connect( mSymbolComboBox, SIGNAL( currentIndexChanged( const QString & ) ),
           this, SLOT( resendSettingsChanged() ) );
  connect( cboOutlineStyle, SIGNAL(
             currentIndexChanged( const QString & ) ), this, SLOT( resendSettingsChanged() ) );
  connect( cboFillStyle, SIGNAL(
             currentIndexChanged( const QString & ) ), this, SLOT( resendSettingsChanged() ) );
  //need this to deal with when texture fill is selected or deselected
  connect( cboFillStyle, SIGNAL(
             currentIndexChanged( int ) ), this, SLOT( fillStyleChanged( int ) ) );
  connect( toolSelectTexture, SIGNAL( clicked() ), this, SLOT( selectTextureImage() ) );

  refreshMarkers();
}

void QgsSingleSymbolDialog::refreshMarkers()
{
  lstSymbols->blockSignals( true );
  lstSymbols->clear();

  QPen pen( QColor( 0, 0, 255 ) );
  QBrush brush( QColor( 220, 220, 220 ), Qt::SolidPattern );
  int size = 18;
  int myCounter = 0;
  QStringList ml = QgsMarkerCatalogue::instance()->list();
  for ( QStringList::iterator it = ml.begin(); it != ml.end(); ++it )
  {
    QPixmap myPixmap = QPixmap::fromImage( QgsMarkerCatalogue::instance()->imageMarker( *it, size, pen, brush ) );
    QListWidgetItem * mypItem = new QListWidgetItem( lstSymbols );
    QIcon myIcon;
    myIcon.addPixmap( myPixmap );
    mypItem->setIcon( myIcon );
    mypItem->setText( "" );
    mypItem->setToolTip( *it );
    //store the symbol offset in the UserData role for later retrieval
    mypItem->setData( Qt::UserRole, *it );
    mypItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    if ( mVectorLayer && mVectorLayer->geometryType() != QGis::Point )
    {
      break;
    }
    ++myCounter;
  }

  // Find out the numerical fields of mVectorLayer, and populate the ComboBoxes
  QgsVectorDataProvider *provider = mVectorLayer->dataProvider();
  if ( provider )
  {
    const QgsFieldMap & fields = provider->fields();
    QString str;

    mRotationClassificationComboBox->addItem( DO_NOT_USE_STR, -1 );
    mScaleClassificationComboBox->addItem( DO_NOT_USE_STR, -1 );
    mSymbolComboBox->addItem( DO_NOT_USE_STR, -1 );
    for ( QgsFieldMap::const_iterator it = fields.begin();
          it != fields.end();
          ++it )
    {
      QVariant::Type type = ( *it ).type();
      if ( type == QVariant::Int || type == QVariant::Double )
      {
        mRotationClassificationComboBox->addItem( it->name(), it.key() );
        mScaleClassificationComboBox->addItem( it->name(), it.key() );
      }
      else if ( type == QVariant::String )
      {
        mSymbolComboBox->addItem( it->name(), it.key() );
      }
    }
  }
  else
  {
    QgsDebugMsg( "Warning, data provider is null" );
    return;
  }
  //
  //set outline / line style
  //
  cboOutlineStyle->addItem( QIcon( QgsSymbologyUtils::char2LinePixmap( "SolidLine" ) ), "", "SolidLine" );
  cboOutlineStyle->addItem( QIcon( QgsSymbologyUtils::char2LinePixmap( "DashLine" ) ), "", "DashLine" );
  cboOutlineStyle->addItem( QIcon( QgsSymbologyUtils::char2LinePixmap( "DotLine" ) ), "", "DotLine" );
  cboOutlineStyle->addItem( QIcon( QgsSymbologyUtils::char2LinePixmap( "DashDotLine" ) ), "" , "DashDotLine" );
  cboOutlineStyle->addItem( QIcon( QgsSymbologyUtils::char2LinePixmap( "DashDotDotLine" ) ),"", "DashDotDotLine" );
  cboOutlineStyle->addItem( QIcon( QgsSymbologyUtils::char2LinePixmap( "NoPen" ) ), tr( "None" ), "NoPen" );

  //
  //set pattern icons and state
  //
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "SolidPattern" ) ), "", "SolidPattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "HorPattern" ) ), "", "HorPattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "VerPattern" ) ), "", "VerPattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "CrossPattern" ) ),"", "CrossPattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "BDiagPattern" ) ), "", "BDiagPattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "FDiagPattern" ) ), "", "FDiagPattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "DiagCrossPattern" ) ), "", "DiagCrossPattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "Dense1Pattern" ) ), "", "Dense1Pattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "Dense2Pattern" ) ), "", "Dense2Pattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "Dense3Pattern" ) ), "", "Dense3Pattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "Dense4Pattern" ) ), "", "Dense4Pattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "Dense5Pattern" ) ), "", "Dense5Pattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "Dense6Pattern" ) ), "", "Dense6Pattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "Dense7Pattern" ) ), "", "Dense7Pattern" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "NoBrush" ) ), tr( "None" ), "NoBrush" );
  cboFillStyle->addItem( QIcon( QgsSymbologyUtils::char2PatternPixmap( "TexturePattern" ) ), tr( "Texture" ), "TexturePattern" );

  if ( mVectorLayer && mVectorLayer->geometryType() != QGis::Point )
  {
    mGroupPoint->setVisible( false );
    mGroupPoint->setEnabled( false );
    mGroupDrawingByField->setVisible( false );
    mGroupDrawingByField->setEnabled( false );
  }

  if ( mDisabled )
  {
    unset();
  }
  else
  {
    if ( mVectorLayer )
    {
      const QgsSingleSymbolRenderer *renderer = dynamic_cast<const QgsSingleSymbolRenderer *>( mVectorLayer->renderer() );

      if ( renderer )
      {
        // Set from the existing renderer
        set( renderer->symbols().first() );
      }
      else
      {
        // Take values from an example instance
        QgsSingleSymbolRenderer exampleRenderer = QgsSingleSymbolRenderer( mVectorLayer->geometryType() );
        set( exampleRenderer.symbols().first() );
      }
    }
    else
    {
      QgsDebugMsg( "Warning, layer is a null pointer" );
    }
  }

  lstSymbols->blockSignals( false );
}

QgsSingleSymbolDialog::~QgsSingleSymbolDialog()
{
  QgsDebugMsg( "entered." );
}

void QgsSingleSymbolDialog::selectOutlineColor()
{
  QColor c = QColorDialog::getColor( btnOutlineColor->color(), this );

  if ( c.isValid() )
  {
    btnOutlineColor->setColor( c );
    emit settingsChanged();
  }

  activateWindow();
}

void QgsSingleSymbolDialog::selectFillColor()
{
  QColor c = QColorDialog::getColor( btnFillColor->color(), this );

  if ( c.isValid() )
  {
    btnFillColor->setColor( c );
    emit settingsChanged();
  }

  activateWindow();
}

//should this method have a different name?
void QgsSingleSymbolDialog::selectTextureImage()
{
  QString fileName = QFileDialog::getOpenFileName( this, "Open File",
                     mTexturePath,
                     "Images (*.png *.xpm *.jpg)" ); //should we allow other types of images?

  if ( fileName.isNull() == false )
  { //only process the string if the user clicked OK
    mTexturePath = fileName;
    resendSettingsChanged();
  }
}

void QgsSingleSymbolDialog::apply( QgsSymbol *sy )
{
  //query the values of the widgets and set the symbology of the vector layer
  if ( btnFillColor->isEnabled() )
    sy->setFillColor( btnFillColor->color() );

  if ( outlinewidthspinbox->isEnabled() )
    sy->setLineWidth( outlinewidthspinbox->value() );

  if ( btnOutlineColor->isEnabled() )
    sy->setColor( btnOutlineColor->color() );

  //
  // Apply point symbol
  //
  if ( lstSymbols->isEnabled() && lstSymbols->currentItem() )
  {
    sy->setNamedPointSymbol( lstSymbols->currentItem()->data( Qt::UserRole ).toString() ) ;
  }

  if ( mPointSizeSpinBox->isEnabled() )
    sy->setPointSize( mPointSizeSpinBox->value() );

  if ( mPointSizeUnitsCheckBox->isEnabled() )
    sy->setPointSizeUnits( mPointSizeUnitsCheckBox->isChecked() );

  std::map<QString, int>::iterator iter;
  if ( mRotationClassificationComboBox->isEnabled() )
  {
    sy->setRotationClassificationField( mRotationClassificationComboBox->itemData( mRotationClassificationComboBox->currentIndex() ).toInt() );
  }

  if ( mScaleClassificationComboBox->isEnabled() )
  {
    sy->setScaleClassificationField( mScaleClassificationComboBox->itemData( mScaleClassificationComboBox->currentIndex() ).toInt() );
  }

  if ( mSymbolComboBox->isEnabled() )
  {
    sy->setSymbolField( mSymbolComboBox->itemData( mSymbolComboBox->currentIndex() ).toInt() );
  }

  //
  // Apply the line style
  //
  if ( cboOutlineStyle->isEnabled() )
  {
    QString myLineStyle =
      cboOutlineStyle->itemData( cboOutlineStyle->currentIndex(), Qt::UserRole ).toString();
    sy->setLineStyle( QgsSymbologyUtils::qString2PenStyle( myLineStyle ) );
  }

  //
  // Apply the pattern
  //

  //Store the file path, and set the brush to TexturePattern.  If we have a different button selected,
  // the below code will override it, but leave the file path alone.

  sy->setCustomTexture( mTexturePath );

  if ( cboFillStyle->isEnabled() )
  {
    QString myFillStyle =
      cboFillStyle->itemData( cboFillStyle->currentIndex(), Qt::UserRole ).toString();
    sy->setFillStyle( QgsSymbologyUtils::qString2BrushStyle( myFillStyle ) );
  }

  if ( mLabelEdit->isEnabled() )
    sy->setLabel( mLabelEdit->text() );
}

void QgsSingleSymbolDialog::apply()
{
  QgsSymbol* sy = new QgsSymbol( mVectorLayer->geometryType() );
  apply( sy );

  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( mVectorLayer->geometryType() );
  renderer->addSymbol( sy );
  renderer->updateSymbolAttributes();

  mVectorLayer->setRenderer( renderer );
}

void QgsSingleSymbolDialog::unset()
{
  mLabelEdit->setEnabled( false );
  lstSymbols->setEnabled( false );
  mPointSizeSpinBox->setEnabled( false );
  mPointSizeUnitsCheckBox->setEnabled( false );
  mRotationClassificationComboBox->setEnabled( false );
  mScaleClassificationComboBox->setEnabled( false );
  mSymbolComboBox->setEnabled( false );
  outlinewidthspinbox->setEnabled( false );
  btnOutlineColor->setEnabled( false );
  cboOutlineStyle->setEnabled( false );

  cboFillStyle->setEnabled( false );
  btnFillColor->setEnabled( false );
}

void QgsSingleSymbolDialog::set( const QgsSymbol *sy )
{
  //set label
  mLabelEdit->setText( sy->label() );

  // Set point symbol
  QString mySymbolName = sy->pointSymbolName();
  for ( int i = 0; i < lstSymbols->count(); ++i )
  {
    if ( lstSymbols->item( i )->data( Qt::UserRole ).toString() == ( mySymbolName ) )
    {
      lstSymbols->setCurrentItem( lstSymbols->item( i ) );
      lstSymbols->item( i )->setBackground( QBrush( Qt::cyan ) );
      break;
    }
  }
  mPointSizeSpinBox->setValue( sy->pointSize() );
  mPointSizeUnitsCheckBox->setChecked( sy->pointSizeUnits() );

  int index;

  index = mRotationClassificationComboBox->findData( sy->rotationClassificationField() );
  mRotationClassificationComboBox->setCurrentIndex( index < 0 ? 0 : index );

  index = mScaleClassificationComboBox->findData( sy->scaleClassificationField() );
  mScaleClassificationComboBox->setCurrentIndex( index < 0 ? 0 : index );

  index = mSymbolComboBox->findData( sy->symbolField() );
  mSymbolComboBox->setCurrentIndex( index < 0 ? 0 : index );

  outlinewidthspinbox->setValue( sy->pen().widthF() );

  //set line width 1 as minimum to avoid confusion between line width 0 and no pen line style
  // ... but, drawLine is not correct with width > 0 -> until solved set to 0
  outlinewidthspinbox->setMinimum( 0 );

  btnFillColor->setColor( sy->brush().color() );

  btnOutlineColor->setColor( sy->pen().color() );

  //load the icons stored in QgsSymbologyUtils.cpp (to avoid redundancy)

  //
  // Set the line style combo
  //

  QPen myPen = sy->pen();
  QString myLineStyle = QgsSymbologyUtils::penStyle2QString( myPen.style() );
  for ( int i = 0; i < cboOutlineStyle->count(); ++i )
  {
    if ( cboOutlineStyle->itemData( i, Qt::UserRole ).toString() == myLineStyle )
    {
      cboOutlineStyle->setCurrentIndex( i );
      break;
    }
  }

  //
  // Set the brush combo
  //

  QBrush myBrush = sy->brush();
  QString myFillStyle =  QgsSymbologyUtils::brushStyle2QString( myBrush.style() );
  for ( int i = 0; i < cboFillStyle->count(); ++i )
  {
    if ( cboFillStyle->itemData( i, Qt::UserRole ).toString() == myFillStyle )
    {
      cboFillStyle->setCurrentIndex( i );
      break;
    }
  }

  //get and show the file path, even if we aren't using it.
  mTexturePath = sy->customTexture();
  //if the file path isn't empty, show the image on the button
  if ( sy->customTexture().size() > 0 )
  {
    //show the current texture image
    // texture->setPixmap(QPixmap(sy->customTexture()));
  }
  else
  {
    //show the default question mark
    //texture->setPixmap(QgsSymbologyUtils::char2PatternPixmap("TexturePattern"));
  }

  mLabelEdit->setEnabled( true );
  lstSymbols->setEnabled( true );
  mPointSizeSpinBox->setEnabled( true );
  mPointSizeUnitsCheckBox->setEnabled( true );
  mRotationClassificationComboBox->setEnabled( true );
  mScaleClassificationComboBox->setEnabled( true );
  mSymbolComboBox->setEnabled( true );
  outlinewidthspinbox->setEnabled( true );
  btnOutlineColor->setEnabled( true );
  cboOutlineStyle->setEnabled( true );

  if ( mVectorLayer && mVectorLayer->geometryType() != QGis::Line )
  {
    btnFillColor->setEnabled( true );
    cboFillStyle->setEnabled( true );
  }
}

void QgsSingleSymbolDialog::updateSet( const QgsSymbol *sy )
{
  if ( mLabelEdit->isEnabled() && mLabelEdit->text() != sy->label() )
    mLabelEdit->setEnabled( false );

  if ( lstSymbols->isEnabled() && lstSymbols->currentItem()->data( Qt::UserRole ).toString() != sy->pointSymbolName() )
    lstSymbols->setEnabled( false );

  if ( mPointSizeSpinBox->isEnabled() && !doubleNear( mPointSizeSpinBox->value(), sy->pointSize() ) )
    mPointSizeSpinBox->setEnabled( false );

  if ( mPointSizeUnitsCheckBox->isEnabled() && mPointSizeUnitsCheckBox->isChecked() != sy->pointSizeUnits() )
    mPointSizeUnitsCheckBox->setEnabled( false );

  if ( mRotationClassificationComboBox->isEnabled() &&
       mRotationClassificationComboBox->itemData( mRotationClassificationComboBox->currentIndex() ).toInt() != sy->rotationClassificationField() )
    mRotationClassificationComboBox->setEnabled( false );

  if ( mScaleClassificationComboBox->isEnabled() &&
       mScaleClassificationComboBox->itemData( mScaleClassificationComboBox->currentIndex() ).toInt() != sy->scaleClassificationField() )
    mScaleClassificationComboBox->setEnabled( false );

  if ( mSymbolComboBox->isEnabled() &&
       mSymbolComboBox->itemData( mSymbolComboBox->currentIndex() ).toInt() != sy->symbolField() )
    mSymbolComboBox->setEnabled( false );

  if ( outlinewidthspinbox->isEnabled() && !doubleNear( outlinewidthspinbox->value(), sy->pen().widthF() ) )
    outlinewidthspinbox->setEnabled( false );

  if ( btnFillColor->isEnabled() &&  btnFillColor->color() != sy->brush().color() )
    btnFillColor->setEnabled( false );

  if ( btnOutlineColor->isEnabled() &&  btnOutlineColor->color() != sy->pen().color() )
    btnOutlineColor->setEnabled( false );

  if ( cboOutlineStyle->isEnabled() )
  {
    QPen myPen = sy->pen();
    QString myLineStyle = QgsSymbologyUtils::penStyle2QString( myPen.style() );
    if ( cboOutlineStyle->itemData( cboOutlineStyle->currentIndex(), Qt::UserRole ).toString() != myLineStyle )
      cboOutlineStyle->setEnabled( false );
  }

  if ( cboFillStyle->isEnabled() )
  {
    QBrush myBrush = sy->brush();
    QString myFillStyle =  QgsSymbologyUtils::brushStyle2QString( myBrush.style() );
    if ( cboFillStyle->itemData( cboFillStyle->currentIndex(), Qt::UserRole ).toString() != myFillStyle )
      cboFillStyle->setEnabled( false );
  }
}

void QgsSingleSymbolDialog::setOutlineColor( QColor& c )
{
  btnOutlineColor->setColor( c );
}

void QgsSingleSymbolDialog::setOutlineStyle( Qt::PenStyle pstyle )
{
  QString myLineStyle = QgsSymbologyUtils::penStyle2QString( pstyle );
  for ( int i = 0; i < cboOutlineStyle->count(); ++i )
  {
    if ( cboOutlineStyle->itemData( i, Qt::UserRole ).toString() == myLineStyle )
    {
      cboOutlineStyle->setCurrentIndex( i );
      break;
    }
  }
}

void QgsSingleSymbolDialog::setFillColor( QColor& c )
{
  btnFillColor->setColor( c );
}

void QgsSingleSymbolDialog::setFillStyle( Qt::BrushStyle fstyle )
{
  QgsDebugMsg( QString( "Setting fill style: %1" ).arg( QgsSymbologyUtils::brushStyle2QString( fstyle ) ) );

  QString myFillStyle =  QgsSymbologyUtils::brushStyle2QString( fstyle );
  for ( int i = 0; i < cboFillStyle->count(); ++i )
  {
    if ( cboFillStyle->itemData( i, Qt::UserRole ).toString() == myFillStyle )
    {
      cboFillStyle->setCurrentIndex( i );
      break;
    }
  }
}

void QgsSingleSymbolDialog::setOutlineWidth( double width )
{
  outlinewidthspinbox->setValue( width );
}

QColor QgsSingleSymbolDialog::getOutlineColor()
{
  return btnOutlineColor->color();
}

Qt::PenStyle QgsSingleSymbolDialog::getOutlineStyle()
{
  QString myLineStyle =
    cboOutlineStyle->itemData( cboOutlineStyle->currentIndex(), Qt::UserRole ).toString();
  return QgsSymbologyUtils::qString2PenStyle( myLineStyle );
}

double QgsSingleSymbolDialog::getOutlineWidth()
{
  return outlinewidthspinbox->value();
}

QColor QgsSingleSymbolDialog::getFillColor()
{
  return btnFillColor->color();
}

Qt::BrushStyle QgsSingleSymbolDialog::getFillStyle()
{
  QString myFillStyle =
    cboFillStyle->itemData( cboFillStyle->currentIndex(), Qt::UserRole ).toString();
  return QgsSymbologyUtils::qString2BrushStyle( myFillStyle );
}

void QgsSingleSymbolDialog::resendSettingsChanged()
{
  emit settingsChanged();
}

QString QgsSingleSymbolDialog::label()
{
  return mLabelEdit->text();
}

void QgsSingleSymbolDialog::setLabel( QString label )
{
  mLabelEdit->setText( label );
}

void QgsSingleSymbolDialog::symbolChanged
( QListWidgetItem * current, QListWidgetItem * previous )
{
  current->setBackground( QBrush( Qt::cyan ) );
  if ( previous )
  {
    previous->setBackground( QBrush( Qt::white ) );
  }
  emit settingsChanged();
}

void QgsSingleSymbolDialog::fillStyleChanged( int theIndex )
{
  //if the new style is texture we need to enable the texture
  //selection button, otherwise disable it
  QString myFillStyle =
    cboFillStyle->itemData( theIndex, Qt::UserRole ).toString();
  if ( "TexturePattern" == myFillStyle )
  {
    toolSelectTexture->setEnabled( true );
  }
  else
  {
    toolSelectTexture->setEnabled( false );
  }

}

void QgsSingleSymbolDialog::keyPressEvent( QKeyEvent * e )
{
  // Ignore the ESC key to avoid close the dialog without the properties window
  if ( e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
}
