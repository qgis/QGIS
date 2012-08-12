/***************************************************************************
                          qgslabelpropertydialog.cpp
                          --------------------------
    begin                : 2010-11-12
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelpropertydialog.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsvectorlayer.h"
#include <QColorDialog>
#include <QFontDialog>

QgsLabelPropertyDialog::QgsLabelPropertyDialog( const QString& layerId, int featureId, QgsMapRenderer* renderer, QWidget * parent, Qt::WindowFlags f ):
    QDialog( parent, f ), mMapRenderer( renderer ), mCurrentLabelField( -1 )
{
  setupUi( this );
  fillHaliComboBox();
  fillValiComboBox();
  init( layerId, featureId );
}

QgsLabelPropertyDialog::~QgsLabelPropertyDialog()
{
}

void QgsLabelPropertyDialog::init( const QString& layerId, int featureId )
{
  if ( !mMapRenderer )
  {
    return;
  }

  //get feature attributes
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
  if ( !vlayer )
  {
    return;
  }

  QgsFeature f;
  if ( !vlayer->featureAtId( featureId, f, false, true ) )
  {
    return;
  }
  const QgsAttributeMap& attributeValues = f.attributeMap();

  //get layerproperties. Problem: only for pallabeling...
  QgsPalLabeling* lbl = dynamic_cast<QgsPalLabeling*>( mMapRenderer->labelingEngine() );
  if ( !lbl )
  {
    return;
  }

  blockElementSignals( true );

  //get label field and fill line edit
  QString labelFieldName = vlayer->customProperty( "labeling/fieldName" ).toString();
  if ( !labelFieldName.isEmpty() )
  {
    mCurrentLabelField = vlayer->fieldNameIndex( labelFieldName );
    mLabelTextLineEdit->setText( attributeValues[mCurrentLabelField].toString() );
    const QgsFieldMap& layerFields = vlayer->pendingFields();
    switch ( layerFields[mCurrentLabelField].type() )
    {
      case QVariant::Double:
        mLabelTextLineEdit->setValidator( new QDoubleValidator( this ) );
        break;
      case QVariant::Int:
      case QVariant::UInt:
      case QVariant::LongLong:
        mLabelTextLineEdit->setValidator( new QIntValidator( this ) );
        break;
      default:
        break;
    }
  }

  //get attributes of the feature and fill data defined values
  QgsPalLayerSettings& layerSettings = lbl->layer( layerId );
  mLabelFont = layerSettings.textFont;

  //set all the gui elements to the default values
  mFontSizeSpinBox->setValue( layerSettings.textFont.pointSizeF() );
  mBufferColorButton->setColor( layerSettings.textColor );
  mLabelDistanceSpinBox->setValue( layerSettings.dist );
  mBufferSizeSpinBox->setValue( layerSettings.bufferSize );
  mHaliComboBox->setCurrentIndex( mHaliComboBox->findText( "Left" ) );
  mValiComboBox->setCurrentIndex( mValiComboBox->findText( "Bottom" ) );

  disableGuiElements();

  mDataDefinedProperties = layerSettings.dataDefinedProperties;
  QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator propIt = mDataDefinedProperties.constBegin();

  for ( ; propIt != mDataDefinedProperties.constEnd(); ++propIt )
  {
    switch ( propIt.key() )
    {
      case QgsPalLayerSettings::Show:
        mShowLabelChkbx->setEnabled( true );
        mShowLabelChkbx->setChecked( attributeValues[propIt.value()].toInt() != 0 );
        break;
      case QgsPalLayerSettings::Size:
        mFontSizeSpinBox->setEnabled( true );
        mLabelFont.setPointSizeF( attributeValues[propIt.value()].toDouble() );
        mFontSizeSpinBox->setValue( attributeValues[propIt.value()].toDouble() );
        break;
      case QgsPalLayerSettings::BufferSize:
        mBufferSizeSpinBox->setEnabled( true );
        mBufferSizeSpinBox->setValue( attributeValues[propIt.value()].toDouble() );
        break;
      case QgsPalLayerSettings::PositionX:
        mXCoordSpinBox->setEnabled( true );
        mXCoordSpinBox->setValue( attributeValues[propIt.value()].toDouble() );
        break;
      case QgsPalLayerSettings::PositionY:
        mYCoordSpinBox->setEnabled( true );
        mYCoordSpinBox->setValue( attributeValues[propIt.value()].toDouble() );
        break;
      case QgsPalLayerSettings::LabelDistance:
        mLabelDistanceSpinBox->setEnabled( true );
        mLabelDistanceSpinBox->setValue( attributeValues[propIt.value()].toDouble() );
        break;
      case QgsPalLayerSettings::Hali:
        mHaliComboBox->setEnabled( true );
        mHaliComboBox->setCurrentIndex( mHaliComboBox->findText( attributeValues[propIt.value()].toString() ) );
        break;
      case QgsPalLayerSettings::Vali:
        mValiComboBox->setEnabled( true );
        mValiComboBox->setCurrentIndex( mValiComboBox->findText( attributeValues[propIt.value()].toString() ) );
        break;
      case QgsPalLayerSettings::BufferColor:
        mBufferColorButton->setEnabled( true );
        mBufferColorButton->setColor( QColor( attributeValues[propIt.value()].toString() ) );
        break;
      case QgsPalLayerSettings::Color:
        mFontColorButton->setEnabled( true );
        mFontColorButton->setColor( QColor( attributeValues[propIt.value()].toString() ) );
        break;
      case QgsPalLayerSettings::Rotation:
        mRotationSpinBox->setEnabled( true );
        mRotationSpinBox->setValue( attributeValues[propIt.value()].toDouble() );
        break;

        //font related properties
      case QgsPalLayerSettings::Bold:
        mLabelFont.setBold( attributeValues[propIt.value()].toBool() );
        break;
      case QgsPalLayerSettings::Italic:
        mLabelFont.setItalic( attributeValues[propIt.value()].toBool() );
        break;
      case QgsPalLayerSettings::Underline:
        mLabelFont.setUnderline( attributeValues[propIt.value()].toBool() );
        break;
      case QgsPalLayerSettings::Strikeout:
        mLabelFont.setStrikeOut( attributeValues[propIt.value()].toBool() );
        break;
      case QgsPalLayerSettings::Family:
        mLabelFont.setFamily( attributeValues[propIt.value()].toString() );
        break;
      default:
        break;
    }
  }
  mFontPushButton->setEnabled( labelFontEditingPossible() );
  blockElementSignals( false );
}

void QgsLabelPropertyDialog::disableGuiElements()
{
  mShowLabelChkbx->setEnabled( false );
  mFontSizeSpinBox->setEnabled( false );
  mBufferSizeSpinBox->setEnabled( false );
  mFontPushButton->setEnabled( false );
  mFontColorButton->setEnabled( false );
  mBufferColorButton->setEnabled( false );
  mLabelDistanceSpinBox->setEnabled( false );
  mXCoordSpinBox->setEnabled( false );
  mYCoordSpinBox->setEnabled( false );
  mHaliComboBox->setEnabled( false );
  mValiComboBox->setEnabled( false );
  mRotationSpinBox->setEnabled( false );
}

void QgsLabelPropertyDialog::blockElementSignals( bool block )
{
  mShowLabelChkbx->blockSignals( block );
  mFontSizeSpinBox->blockSignals( block );
  mBufferSizeSpinBox->blockSignals( block );
  mFontPushButton->blockSignals( block );
  mBufferColorButton->blockSignals( block );
  mLabelDistanceSpinBox->blockSignals( block );
  mXCoordSpinBox->blockSignals( block );
  mYCoordSpinBox->blockSignals( block );
  mHaliComboBox->blockSignals( block );
  mValiComboBox->blockSignals( block );
  mRotationSpinBox->blockSignals( block );
}

void QgsLabelPropertyDialog::fillHaliComboBox()
{
  mHaliComboBox->addItem( "Left" );
  mHaliComboBox->addItem( "Center" );
  mHaliComboBox->addItem( "Right" );
}

void QgsLabelPropertyDialog::fillValiComboBox()
{
  mValiComboBox->addItem( "Bottom" );
  mValiComboBox->addItem( "Base" );
  mValiComboBox->addItem( "Half" );
  mValiComboBox->addItem( "Top" );
}

void QgsLabelPropertyDialog::on_mShowLabelChkbx_toggled( bool chkd )
{
  insertChangedValue( QgsPalLayerSettings::Show, ( chkd ? 1 : 0 ) );
}

void QgsLabelPropertyDialog::on_mLabelDistanceSpinBox_valueChanged( double d )
{
  insertChangedValue( QgsPalLayerSettings::LabelDistance, d );
}

void QgsLabelPropertyDialog::on_mXCoordSpinBox_valueChanged( double d )
{
  insertChangedValue( QgsPalLayerSettings::PositionX, d );
}

void QgsLabelPropertyDialog::on_mYCoordSpinBox_valueChanged( double d )
{
  insertChangedValue( QgsPalLayerSettings::PositionY, d );
}

void QgsLabelPropertyDialog::on_mFontSizeSpinBox_valueChanged( double d )
{
  insertChangedValue( QgsPalLayerSettings::Size, d );
}

void QgsLabelPropertyDialog::on_mBufferSizeSpinBox_valueChanged( double d )
{
  insertChangedValue( QgsPalLayerSettings::PositionX, d );
}

void QgsLabelPropertyDialog::on_mRotationSpinBox_valueChanged( double d )
{
  insertChangedValue( QgsPalLayerSettings::Rotation, d );
}

void QgsLabelPropertyDialog::on_mFontPushButton_clicked()
{
  bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  mLabelFont = QFontDialog::getFont( &ok, mLabelFont, 0, tr( "Label font" ), QFontDialog::DontUseNativeDialog );
#else
  mLabelFont = QFontDialog::getFont( &ok, mLabelFont, 0, tr( "Label font" ) );
#endif
  if ( ok )
  {
    insertChangedValue( QgsPalLayerSettings::Size, mLabelFont.pointSizeF() );
    insertChangedValue( QgsPalLayerSettings::Bold, mLabelFont.bold() );
    insertChangedValue( QgsPalLayerSettings::Italic, mLabelFont.italic() );
    insertChangedValue( QgsPalLayerSettings::Underline, mLabelFont.underline() );
  }
}

void QgsLabelPropertyDialog::on_mFontColorButton_clicked()
{
#if QT_VERSION >= 0x040500
  QColor c = QColorDialog::getColor( mFontColorButton->color(), 0, tr( "Font color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor c = QColorDialog::getColor( mFontColorButton->color() );
#endif
  if ( c.isValid() )
  {
    mFontColorButton->setColor( c );
    insertChangedValue( QgsPalLayerSettings::Color, c.name() );
  }
}

void QgsLabelPropertyDialog::on_mBufferColorButton_clicked()
{
#if QT_VERSION >= 0x040500
  QColor c = QColorDialog::getColor( mBufferColorButton->color(), 0, tr( "Buffer color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor c = QColorDialog::getColor( mBufferColorButton->color() );
#endif
  if ( c.isValid() )
  {
    mFontColorButton->setColor( c );
    insertChangedValue( QgsPalLayerSettings::BufferColor, c.name() );
  }
}

void QgsLabelPropertyDialog::on_mHaliComboBox_currentIndexChanged( const QString& text )
{
  insertChangedValue( QgsPalLayerSettings::Hali, text );
}

void QgsLabelPropertyDialog::on_mValiComboBox_currentIndexChanged( const QString& text )
{
  insertChangedValue( QgsPalLayerSettings::Vali, text );
}

void QgsLabelPropertyDialog::on_mLabelTextLineEdit_textChanged( const QString& text )
{
  if ( mCurrentLabelField != -1 )
  {
    mChangedProperties.insert( mCurrentLabelField, text );
  }
}

void QgsLabelPropertyDialog::insertChangedValue( QgsPalLayerSettings::DataDefinedProperties p, QVariant value )
{
  QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator ddIt = mDataDefinedProperties.find( p );
  if ( ddIt != mDataDefinedProperties.constEnd() )
  {
    mChangedProperties.insert( ddIt.value(), value );
  }
}

bool QgsLabelPropertyDialog::labelFontEditingPossible() const
{
  return( mDataDefinedProperties.contains( QgsPalLayerSettings:: Size ) || mDataDefinedProperties.contains( QgsPalLayerSettings::Bold )
          || mDataDefinedProperties.contains( QgsPalLayerSettings::Italic ) || mDataDefinedProperties.contains( QgsPalLayerSettings::Underline )
          ||  mDataDefinedProperties.contains( QgsPalLayerSettings::Strikeout )
          || mDataDefinedProperties.contains( QgsPalLayerSettings::Family ) );
}

