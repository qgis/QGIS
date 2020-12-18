/***************************************************************************
                         qgsbasicrelationconfigwidget.cpp
                         ----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsbasicrelationconfigwidget.h"
#include "qgsbasicrelationwidget.h"


QgsBasicRelationConfigWidget::QgsBasicRelationConfigWidget( const QgsRelation &relation, QWidget *parent )
  : QgsRelationConfigWidget( relation, parent )
{
  setupUi( this );
}

QVariantMap QgsBasicRelationConfigWidget::config()
{
  QgsBasicRelationWidget::Buttons buttons;
  buttons.setFlag( QgsBasicRelationWidget::Button::Link, mRelationShowLinkCheckBox->isChecked() );
  buttons.setFlag( QgsBasicRelationWidget::Button::Unlink, mRelationShowUnlinkCheckBox->isChecked() );
  buttons.setFlag( QgsBasicRelationWidget::Button::AddChildFeature, mRelationShowAddChildCheckBox->isChecked() );
  buttons.setFlag( QgsBasicRelationWidget::Button::DuplicateChildFeature, mRelationShowDuplicateChildFeatureCheckBox->isChecked() );
  buttons.setFlag( QgsBasicRelationWidget::Button::ZoomToChildFeature, mRelationShowZoomToFeatureCheckBox->isChecked() );
  buttons.setFlag( QgsBasicRelationWidget::Button::DeleteChildFeature, mRelationDeleteChildFeatureCheckBox->isChecked() );
  buttons.setFlag( QgsBasicRelationWidget::Button::SaveChildEdits, mRelationShowSaveChildEditsCheckBox->isChecked() );

  return QVariantMap(
  {
    {"buttons", qgsFlagValueToKeys( buttons )},
  } );
}

void QgsBasicRelationConfigWidget::setConfig( const QVariantMap &config )
{
  const QgsBasicRelationWidget::Buttons buttons = qgsFlagKeysToValue( config.value( QStringLiteral( "buttons" ) ).toString(), QgsBasicRelationWidget::Button::AllButtons );

  mRelationShowLinkCheckBox->setChecked( buttons.testFlag( QgsBasicRelationWidget::Button::Link ) );
  mRelationShowUnlinkCheckBox->setChecked( buttons.testFlag( QgsBasicRelationWidget::Button::Unlink ) );
  mRelationShowAddChildCheckBox->setChecked( buttons.testFlag( QgsBasicRelationWidget::Button::AddChildFeature ) );
  mRelationShowDuplicateChildFeatureCheckBox->setChecked( buttons.testFlag( QgsBasicRelationWidget::Button::DuplicateChildFeature ) );
  mRelationShowZoomToFeatureCheckBox->setChecked( buttons.testFlag( QgsBasicRelationWidget::Button::ZoomToChildFeature ) );
  mRelationDeleteChildFeatureCheckBox->setChecked( buttons.testFlag( QgsBasicRelationWidget::Button::DeleteChildFeature ) );
  mRelationShowSaveChildEditsCheckBox->setChecked( buttons.testFlag( QgsBasicRelationWidget::Button::SaveChildEdits ) );
}
