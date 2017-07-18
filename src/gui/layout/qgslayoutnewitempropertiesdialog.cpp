/***************************************************************************
                             qgslayoutnewitempropertiesdialog.cpp
                             ------------------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutnewitempropertiesdialog.h"
#include "qgssettings.h"

QgsLayoutItemPropertiesDialog::QgsLayoutItemPropertiesDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );

  //make button exclusive
  QButtonGroup *buttonGroup = new QButtonGroup( this );
  buttonGroup->addButton( mUpperLeftCheckBox );
  buttonGroup->addButton( mUpperMiddleCheckBox );
  buttonGroup->addButton( mUpperRightCheckBox );
  buttonGroup->addButton( mMiddleLeftCheckBox );
  buttonGroup->addButton( mMiddleCheckBox );
  buttonGroup->addButton( mMiddleRightCheckBox );
  buttonGroup->addButton( mLowerLeftCheckBox );
  buttonGroup->addButton( mLowerMiddleCheckBox );
  buttonGroup->addButton( mLowerRightCheckBox );
  buttonGroup->setExclusive( true );

  QgsSettings settings;
  double lastWidth = settings.value( QStringLiteral( "LayoutDesigner/lastItemWidth" ), QStringLiteral( "50" ) ).toDouble();
  double lastHeight = settings.value( QStringLiteral( "LayoutDesigner/lastItemHeight" ), QStringLiteral( "50" ) ).toDouble();
  QgsUnitTypes::LayoutUnit lastSizeUnit = static_cast< QgsUnitTypes::LayoutUnit >( settings.value( QStringLiteral( "LayoutDesigner/lastSizeUnit" ) ).toInt() );
  setItemSize( QgsLayoutSize( lastWidth, lastHeight, lastSizeUnit ) );
}

void QgsLayoutItemPropertiesDialog::setItemPosition( QgsLayoutPoint position )
{
  mXPosSpin->setValue( position.x() );
  mYPosSpin->setValue( position.y() );
  mPosUnitsComboBox->setUnit( position.units() );
}

QgsLayoutPoint QgsLayoutItemPropertiesDialog::itemPosition() const
{
  return QgsLayoutPoint( mXPosSpin->value(), mYPosSpin->value(), mPosUnitsComboBox->unit() );
}

void QgsLayoutItemPropertiesDialog::setItemSize( QgsLayoutSize size )
{
  mWidthSpin->setValue( size.width() );
  mHeightSpin->setValue( size.height() );
  mSizeUnitsComboBox->setUnit( size.units() );
}

QgsLayoutSize QgsLayoutItemPropertiesDialog::itemSize() const
{
  return QgsLayoutSize( mWidthSpin->value(), mHeightSpin->value(), mSizeUnitsComboBox->unit() );
}

QgsLayoutItem::ReferencePoint QgsLayoutItemPropertiesDialog::referencePoint() const
{
  if ( mUpperLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::UpperLeft;
  }
  else if ( mUpperMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::UpperMiddle;
  }
  else if ( mUpperRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::UpperRight;
  }
  else if ( mMiddleLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::MiddleLeft;
  }
  else if ( mMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::Middle;
  }
  else if ( mMiddleRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::MiddleRight;
  }
  else if ( mLowerLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::LowerLeft;
  }
  else if ( mLowerMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::LowerMiddle;
  }
  else if ( mLowerRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsLayoutItem::LowerRight;
  }
  return QgsLayoutItem::UpperLeft;
}

void QgsLayoutItemPropertiesDialog::setReferencePoint( QgsLayoutItem::ReferencePoint point )
{
  switch ( point )
  {
    case QgsLayoutItem::UpperLeft:
      mUpperLeftCheckBox->setChecked( true );
      break;

    case QgsLayoutItem::UpperMiddle:
      mUpperMiddleCheckBox->setChecked( true );
      break;

    case QgsLayoutItem::UpperRight:
      mUpperRightCheckBox->setChecked( true );
      break;

    case QgsLayoutItem::MiddleLeft:
      mMiddleLeftCheckBox->setChecked( true );
      break;

    case QgsLayoutItem::Middle:
      mMiddleCheckBox->setChecked( true );
      break;

    case QgsLayoutItem::MiddleRight:
      mMiddleRightCheckBox->setChecked( true );
      break;

    case QgsLayoutItem::LowerLeft:
      mLowerLeftCheckBox->setChecked( true );
      break;

    case QgsLayoutItem::LowerMiddle:
      mLowerMiddleCheckBox->setChecked( true );
      break;

    case QgsLayoutItem::LowerRight:
      mLowerRightCheckBox->setChecked( true );
      break;
  }
}
