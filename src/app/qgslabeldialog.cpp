/***************************************************************************
                         qgslabeldialog.cpp  -  render vector labels
                             -------------------
    begin                : August 2004
    copyright            : (C) 2004 by Radim Blazek & Tim Sutton
    email                : blazek@itc.it
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

#include <limits>

#include "qgslabeldialog.h"
#include "qgsfield.h"
#include "qgslabel.h"
#include "qgslabelattributes.h"
#include "qgslogger.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QTabWidget>
#include <QDoubleValidator>


const int PIXMAP_WIDTH = 200;
const int PIXMAP_HEIGHT = 20;

QgsLabelDialog::QgsLabelDialog( QgsLabel *label,  QWidget *parent )
    : QWidget( parent ),
    mLabel( label ),
    mFontColor( Qt::black ),
    mBufferColor( Qt::black ),
    mFont( "Helvetica" )
{
  setupUi( this );
  QgsDebugMsg( "entering." );

  Q_ASSERT( label );

  init();

  connect( btnDefaultFont, SIGNAL( clicked() ),
           this, SLOT( changeFont() ) );
  connect( pbnDefaultBufferColor, SIGNAL( clicked() ),
           this, SLOT( changeBufferColor() ) );
  connect( pbnDefaultFontColor, SIGNAL( clicked() ),
           this, SLOT( changeFontColor() ) );
}


void QgsLabelDialog::init( )
{
  QgsDebugMsg( "entering." );

  QgsLabelAttributes * myLabelAttributes = mLabel->labelAttributes();
  //populate a string list with all the field names which will be used to set up the
  //data bound combos
  QgsFieldMap& myFieldsMap = mLabel->fields();
  QStringList myFieldStringList;
  myFieldStringList.append( "" );
  for ( QgsFieldMap::iterator it = myFieldsMap.begin(); it != myFieldsMap.end(); ++it )
  {
    myFieldStringList.append( it->name() );
  }
  //
  //now set all the combos that need field lists using the string list
  //
  cboLabelField->clear();
  cboLabelField->addItems( myFieldStringList );
  cboLabelField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::Text ), myFieldStringList ) );

  cboFontField->clear();
  cboFontField->addItems( myFieldStringList );
  cboFontField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::Family ), myFieldStringList ) );

  cboBoldField->clear();
  cboBoldField->addItems( myFieldStringList );
  cboBoldField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::Bold ), myFieldStringList ) );


  cboItalicField->clear();
  cboItalicField->addItems( myFieldStringList );
  cboItalicField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::Italic ), myFieldStringList ) );

  cboUnderlineField->clear();
  cboUnderlineField->addItems( myFieldStringList );
  cboUnderlineField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::Underline ), myFieldStringList ) );

  cboStrikeOutField->clear();
  cboStrikeOutField->addItems( myFieldStringList );
  cboStrikeOutField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::StrikeOut ), myFieldStringList ) );

  cboFontSizeField->clear();
  cboFontSizeField->addItems( myFieldStringList );
  cboFontSizeField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::Size ), myFieldStringList ) );

  cboFontSizeTypeField->clear();
  cboFontSizeTypeField->addItems( myFieldStringList );
  cboFontSizeTypeField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::SizeType ), myFieldStringList ) );

#if 0
  cboFontTransparencyField->clear();
  cboFontTransparencyField->addItems( myFieldStringList );
  cboFontTransparencyField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::FontTransparency ), myFieldStringList ) );
#endif

  cboFontColorField->clear();
  cboFontColorField->addItems( myFieldStringList );
  cboFontColorField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::Color ), myFieldStringList ) );


  cboBufferSizeField->clear();
  cboBufferSizeField->addItems( myFieldStringList );
  cboBufferSizeField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::BufferSize ), myFieldStringList ) );

  cboBufferTransparencyField->clear();
  cboBufferTransparencyField->addItems( myFieldStringList );
  //cboBufferTransparencyField->setCurrentIndex(itemNoForField(mLabel->labelField(QgsLabel::BufferTransparency),myFieldStringList));

  cboXCoordinateField->clear();
  cboXCoordinateField->addItems( myFieldStringList );
  cboXCoordinateField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::XCoordinate ), myFieldStringList ) );

  cboYCoordinateField->clear();
  cboYCoordinateField->addItems( myFieldStringList );
  cboYCoordinateField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::YCoordinate ), myFieldStringList ) );

  cboXOffsetField->clear();
  cboXOffsetField->addItems( myFieldStringList );
  cboXOffsetField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::XOffset ), myFieldStringList ) );

  cboYOffsetField->clear();
  cboYOffsetField->addItems( myFieldStringList );
  cboYOffsetField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::YOffset ), myFieldStringList ) );

  cboAlignmentField->clear();
  cboAlignmentField->addItems( myFieldStringList );
  cboAlignmentField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::Alignment ), myFieldStringList ) );

  cboAngleField->clear();
  cboAngleField->addItems( myFieldStringList );
  cboAngleField->setCurrentIndex( itemNoForField( mLabel->labelField( QgsLabel::Angle ), myFieldStringList ) );

  // set up the scale based layer visibility stuff....
  chkUseScaleDependentRendering->setChecked( mLabel->scaleBasedVisibility() );
  leMinimumScale->setText( QString::number( mLabel->minScale(), 'f' ) );
  leMinimumScale->setValidator( new QDoubleValidator( 0, std::numeric_limits<float>::max(), 1000, this ) );
  leMaximumScale->setText( QString::number( mLabel->maxScale(), 'f' ) );
  leMaximumScale->setValidator( new QDoubleValidator( 0, std::numeric_limits<float>::max(), 1000, this ) );

  //
  //set the non-databound fields up now
  //
  leDefaultLabel->setText( myLabelAttributes->text() );
  mFont.setFamily( myLabelAttributes->family() );
  if ( myLabelAttributes->sizeIsSet() )
  {
    mFont.setPointSizeF( myLabelAttributes->size() );

    int myTypeInt = myLabelAttributes->sizeType();
    cboFontSizeUnits->setCurrentIndex( myTypeInt == QgsLabelAttributes::PointUnits ? 0 : 1 );
  }
  else //defaults for when no size has been set
  {
    mFont.setPointSizeF( myLabelAttributes->size() );
    cboFontSizeUnits->setCurrentIndex( 0 );
  }

  spinFontSize->setValue( myLabelAttributes->size() );

  if ( myLabelAttributes->boldIsSet() )
  {
    mFont.setBold( myLabelAttributes->bold() );
  }
  else
  {
    mFont.setBold( false );
  }
  if ( myLabelAttributes->italicIsSet() )
  {
    mFont.setItalic( myLabelAttributes->italic() );
  }
  else
  {
    mFont.setItalic( false );
  }
  if ( myLabelAttributes->underlineIsSet() )
  {
    mFont.setUnderline( myLabelAttributes->underline() );
  }
  else
  {
    mFont.setUnderline( false );
  }
  if ( myLabelAttributes->strikeOutIsSet() )
  {
    mFont.setStrikeOut( myLabelAttributes->strikeOut() );
  }
  else
  {
    mFont.setStrikeOut( false );
  }

  mFontColor = myLabelAttributes->color();

  if ( myLabelAttributes->offsetIsSet() )
  {
    int myTypeInt = myLabelAttributes->offsetType();
    cboOffsetUnits->setCurrentIndex( myTypeInt == QgsLabelAttributes::PointUnits ? 0 : 1 );
    spinXOffset->setValue( myLabelAttributes->xOffset() );
    spinYOffset->setValue( myLabelAttributes->yOffset() );
  }
  else //defaults for when no offset is defined
  {
    cboOffsetUnits->setCurrentIndex( 0 );
    spinXOffset->setValue( 0 );
    spinYOffset->setValue( 0 );
  }
  spinAngle->setRange( -1, 360 );
  spinAngle->setSpecialValueText( tr( "Auto" ) );
  if ( myLabelAttributes->angleIsAuto() )
  {
    spinAngle->setValue( -1 );
  }
  else
  {
    spinAngle->setValue( static_cast<int>( myLabelAttributes->angle() ) );
  }

  //the values here may seem a bit counterintuitive - basically everything
  //is the reverse of the way you think it should be...
  //TODO investigate in QgsLabel why this needs to be the case
  //TODO add support for corners (e.g. bottom right) to xml serialiser

  if ( myLabelAttributes->alignment() == ( Qt::AlignRight | Qt::AlignBottom ) ) radioAboveLeft->setChecked( true )   ;
  if ( myLabelAttributes->alignment() == ( Qt::AlignRight | Qt::AlignTop ) ) radioBelowLeft->setChecked( true )   ;
  if ( myLabelAttributes->alignment() == ( Qt::AlignLeft  | Qt::AlignBottom ) ) radioAboveRight->setChecked( true )  ;
  if ( myLabelAttributes->alignment() == ( Qt::AlignLeft  | Qt::AlignTop ) ) radioBelowRight->setChecked( true )  ;
  if ( myLabelAttributes->alignment() == ( Qt::AlignRight | Qt::AlignVCenter ) ) radioLeft->setChecked( true )        ;
  if ( myLabelAttributes->alignment() == ( Qt::AlignLeft  | Qt::AlignVCenter ) ) radioRight->setChecked( true )       ;
  if ( myLabelAttributes->alignment() == ( Qt::AlignBottom | Qt::AlignHCenter ) ) radioAbove->setChecked( true )       ;
  if ( myLabelAttributes->alignment() == ( Qt::AlignTop   | Qt::AlignHCenter ) ) radioBelow->setChecked( true )       ;
  if ( myLabelAttributes->alignment() == Qt::AlignCenter ) radioOver->setChecked( true )        ;

  mBufferColor = myLabelAttributes->bufferColor();
  //note that it could be that buffer properties are set, but the bufer is disabled
  if ( myLabelAttributes->bufferSizeIsSet() )
  {
    int myTypeInt = myLabelAttributes->bufferSizeType();
    cboBufferSizeUnits->setCurrentIndex( myTypeInt == QgsLabelAttributes::PointUnits ? 0 : 1 );
    spinBufferSize->setValue( myLabelAttributes->bufferSize() );
  }
  else //defaults for when no offset is defined
  {
    cboBufferSizeUnits->setCurrentIndex( 0 );
    spinBufferSize->setValue( 1 );
  }
  //set the state of the multiline enabled checkbox
  chkUseMultiline->setChecked( myLabelAttributes->multilineEnabled() );
  //set the state of the selected features only checkbox
  chkSelectedOnly->setChecked( myLabelAttributes->selectedOnly() );
  //set the state of the buffer enabled checkbox
  chkUseBuffer->setChecked( myLabelAttributes->bufferEnabled() );

  //NOTE: do we need this line too? TS
  spinBufferSize->setValue( myLabelAttributes->bufferSize() );
}



void QgsLabelDialog::changeFont( void )
{
  QgsDebugMsg( "entering." );

  qreal fontSize = mFont.pointSizeF();
  bool resultFlag;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && !defined(__LP64__)
  // Native Mac dialog works only for 64 bit Cocoa (observed in Qt 4.5.2, probably a Qt bug)
  mFont = QFontDialog::getFont( &resultFlag, mFont, this, QString(), QFontDialog::DontUseNativeDialog );
#else
  mFont = QFontDialog::getFont( &resultFlag, mFont, this );
#endif
  if ( resultFlag )
  {
    if ( mFont.pointSizeF() != fontSize )
    {
      // font is set to the font the user selected
      spinFontSize->setValue( mFont.pointSizeF() );
    }
  }
  else
  {
    // the user cancelled the dialog; font is set to the initial
    // value, in this case Helvetica [Cronyx], 10
  }
  lblSample->setFont( mFont );
}

void QgsLabelDialog::changeFontColor( void )
{
  QgsDebugMsg( "entering." );

  mFontColor = QColorDialog::getColor( mFontColor );
  QPalette palette = lblSample->palette();
  palette.setColor( lblSample->foregroundRole(), mFontColor );
  lblSample->setPalette( palette );
}

void QgsLabelDialog::changeBufferColor( void )
{
  QgsDebugMsg( "entering." );

  mBufferColor = QColorDialog::getColor( mBufferColor );
  QPalette palette = lblSample->palette();
  palette.setColor( lblSample->backgroundRole(), mBufferColor );
  lblSample->setPalette( palette );
}


int QgsLabelDialog::itemNoForField( QString theFieldName, QStringList theFieldList )
{
  int myItemInt = 0; for ( QStringList::Iterator it = theFieldList.begin(); it != theFieldList.end(); ++it )
  {
    if ( theFieldName == *it ) return myItemInt;
    ++myItemInt;
  }
  //if no matches assume first item in list is blank and return that
  return 0;
}

QgsLabelDialog::~QgsLabelDialog()
{
  QgsDebugMsg( "entering." );
}

void QgsLabelDialog::apply()
{
  QgsDebugMsg( "entering." );

  //set the label props that are NOT bound to a field in the attributes tbl
  //All of these are set in the labelAttributes member of the layer
  QgsLabelAttributes * myLabelAttributes = mLabel->labelAttributes();
  myLabelAttributes->setText( leDefaultLabel->text() );
  myLabelAttributes->setFamily( mFont.family() );
  int myTypeInt = cboFontSizeUnits->currentIndex() == 0 ? QgsLabelAttributes::PointUnits : QgsLabelAttributes::MapUnits;
  myLabelAttributes->setSize( mFont.pointSizeF(), myTypeInt );
  myLabelAttributes->setBold( mFont.bold() );
  myLabelAttributes->setItalic( mFont.italic() );
  myLabelAttributes->setUnderline( mFont.underline() );
  myLabelAttributes->setStrikeOut( mFont.strikeOut() );
  myLabelAttributes->setColor( mFontColor );
  myTypeInt = cboOffsetUnits->currentIndex() == 0 ?  QgsLabelAttributes::PointUnits : QgsLabelAttributes::MapUnits;
  myLabelAttributes->setOffset( spinXOffset->value(), spinYOffset->value(), myTypeInt );
  myLabelAttributes->setAutoAngle( spinAngle->value() == -1 );
  myLabelAttributes->setAngle( spinAngle->value() );

  //the values here may seem a bit counterintuitive - basically everything
  //is the reverse of the way you think it should be...
  //TODO investigate in QgsLabel why this needs to be the case
  if ( radioAboveLeft->isChecked() )   myLabelAttributes->setAlignment( Qt::AlignRight | Qt::AlignBottom );
  if ( radioBelowLeft->isChecked() )   myLabelAttributes->setAlignment( Qt::AlignRight | Qt::AlignTop );
  if ( radioAboveRight->isChecked() )  myLabelAttributes->setAlignment( Qt::AlignLeft  | Qt::AlignBottom );
  if ( radioBelowRight->isChecked() )  myLabelAttributes->setAlignment( Qt::AlignLeft  | Qt::AlignTop );
  if ( radioLeft->isChecked() )        myLabelAttributes->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  if ( radioRight->isChecked() )       myLabelAttributes->setAlignment( Qt::AlignLeft  | Qt::AlignVCenter );
  if ( radioAbove->isChecked() )       myLabelAttributes->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
  if ( radioBelow->isChecked() )       myLabelAttributes->setAlignment( Qt::AlignTop   | Qt::AlignHCenter );
  if ( radioOver->isChecked() )        myLabelAttributes->setAlignment( Qt::AlignCenter );

  myLabelAttributes->setMultilineEnabled( chkUseMultiline->isChecked() );
  myLabelAttributes->setSelectedOnly( chkSelectedOnly->isChecked() );
  myLabelAttributes->setBufferEnabled( chkUseBuffer->isChecked() );
  myLabelAttributes->setBufferColor( mBufferColor );
  myTypeInt = cboBufferSizeUnits->currentIndex() == 0 ? QgsLabelAttributes::PointUnits : QgsLabelAttributes::MapUnits;
  myLabelAttributes->setBufferSize( spinBufferSize->value(), myTypeInt );
  //TODO - transparency attributes for buffers

  //set the label props that are data bound to a field in the attributes tbl
  mLabel->setLabelField( QgsLabel::Text,  fieldIndexFromName( cboLabelField->currentText() ) );
  mLabel->setLabelField( QgsLabel::Family, fieldIndexFromName( cboFontField->currentText() ) );
  mLabel->setLabelField( QgsLabel::Bold,  fieldIndexFromName( cboBoldField->currentText() ) );
  mLabel->setLabelField( QgsLabel::Italic,  fieldIndexFromName( cboItalicField->currentText() ) );
  mLabel->setLabelField( QgsLabel::Underline,  fieldIndexFromName( cboUnderlineField->currentText() ) );
  mLabel->setLabelField( QgsLabel::StrikeOut,  fieldIndexFromName( cboStrikeOutField->currentText() ) );
  mLabel->setLabelField( QgsLabel::Size,  fieldIndexFromName( cboFontSizeField->currentText() ) );
  mLabel->setLabelField( QgsLabel::SizeType,  fieldIndexFromName( cboFontSizeTypeField->currentText() ) );
  mLabel->setLabelField( QgsLabel::Color,  fieldIndexFromName( cboFontColorField->currentText() ) );
  mLabel->setLabelField( QgsLabel::BufferSize,  fieldIndexFromName( cboBufferSizeField->currentText() ) );
  //mLabel->setLabelField( QgsLabel::BufferTransparency,  cboBufferTransparencyField->currentText() );
  mLabel->setLabelField( QgsLabel::XCoordinate,  fieldIndexFromName( cboXCoordinateField->currentText() ) );
  mLabel->setLabelField( QgsLabel::YCoordinate,  fieldIndexFromName( cboYCoordinateField->currentText() ) );
  mLabel->setLabelField( QgsLabel::XOffset,  fieldIndexFromName( cboXOffsetField->currentText() ) );
  mLabel->setLabelField( QgsLabel::YOffset,  fieldIndexFromName( cboYOffsetField->currentText() ) );
  mLabel->setLabelField( QgsLabel::Alignment,  fieldIndexFromName( cboAlignmentField->currentText() ) );
  mLabel->setLabelField( QgsLabel::Angle,  fieldIndexFromName( cboAngleField->currentText() ) );

  // set up the scale based layer visibility stuff....
  mLabel->setScaleBasedVisibility( chkUseScaleDependentRendering->isChecked() );
  mLabel->setMinScale( leMinimumScale->text().toFloat() );
  mLabel->setMaxScale( leMaximumScale->text().toFloat() );
}

int QgsLabelDialog::fieldIndexFromName( QString name )
{
  const QgsFieldMap& fields = mLabel->fields();
  for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); ++it )
  {
    if ( it->name() == name )
      return it.key();
  }
  return -1;
}
