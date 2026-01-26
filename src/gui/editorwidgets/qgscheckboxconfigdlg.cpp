/***************************************************************************
    qgscheckboxconfigdlg.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscheckboxconfigdlg.h"

#include "qgscheckboxfieldformatter.h"
#include "qgscheckboxwidgetwrapper.h"

#include "moc_qgscheckboxconfigdlg.cpp"

QgsCheckBoxConfigDlg::QgsCheckBoxConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  mDisplayAsTextComboBox->addItem( tr( "\"True\" or \"False\"" ), QgsCheckBoxFieldFormatter::ShowTrueFalse );
  mDisplayAsTextComboBox->addItem( tr( "Stored Values" ), QgsCheckBoxFieldFormatter::ShowStoredValues );
  mDisplayAsTextComboBox->setCurrentIndex( 0 );

  connect( leCheckedState, &QLineEdit::textEdited, this, &QgsEditorConfigWidget::changed );
  connect( leUncheckedState, &QLineEdit::textEdited, this, &QgsEditorConfigWidget::changed );
  connect( mDisplayAsTextComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );

  if ( vl->fields().at( fieldIdx ).type() == QMetaType::Type::Bool )
  {
    leCheckedState->setEnabled( false );
    leUncheckedState->setEnabled( false );

    leCheckedState->setPlaceholderText( u"TRUE"_s );
    leUncheckedState->setPlaceholderText( u"FALSE"_s );
  }
}

QVariantMap QgsCheckBoxConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( u"CheckedState"_s, leCheckedState->text() );
  cfg.insert( u"UncheckedState"_s, leUncheckedState->text() );
  cfg.insert( u"TextDisplayMethod"_s, mDisplayAsTextComboBox->currentData().toInt() );
  cfg.insert( u"AllowNullState"_s, mAllowNullState->isChecked() );

  return cfg;
}

void QgsCheckBoxConfigDlg::setConfig( const QVariantMap &config )
{
  if ( layer()->fields().at( field() ).type() != QMetaType::Type::Bool )
  {
    leCheckedState->setText( config.value( u"CheckedState"_s ).toString() );
    leUncheckedState->setText( config.value( u"UncheckedState"_s ).toString() );
  }
  mDisplayAsTextComboBox->setCurrentIndex( mDisplayAsTextComboBox->findData( config.value( u"TextDisplayMethod"_s, QString::number( static_cast<int>( QgsCheckBoxFieldFormatter::ShowTrueFalse ) ) ).toInt() ) );
  mAllowNullState->setChecked( config.value( u"AllowNullState"_s ).toBool() );
}
