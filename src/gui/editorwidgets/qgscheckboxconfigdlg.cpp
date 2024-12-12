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
#include "moc_qgscheckboxconfigdlg.cpp"
#include "qgscheckboxwidgetwrapper.h"
#include "qgscheckboxfieldformatter.h"

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

    leCheckedState->setPlaceholderText( QStringLiteral( "TRUE" ) );
    leUncheckedState->setPlaceholderText( QStringLiteral( "FALSE" ) );
  }
}

QVariantMap QgsCheckBoxConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "CheckedState" ), leCheckedState->text() );
  cfg.insert( QStringLiteral( "UncheckedState" ), leUncheckedState->text() );
  cfg.insert( QStringLiteral( "TextDisplayMethod" ), mDisplayAsTextComboBox->currentData().toInt() );
  cfg.insert( QStringLiteral( "AllowNullState" ), mAllowNullState->isChecked() );

  return cfg;
}

void QgsCheckBoxConfigDlg::setConfig( const QVariantMap &config )
{
  if ( layer()->fields().at( field() ).type() != QMetaType::Type::Bool )
  {
    leCheckedState->setText( config.value( QStringLiteral( "CheckedState" ) ).toString() );
    leUncheckedState->setText( config.value( QStringLiteral( "UncheckedState" ) ).toString() );
  }
  mDisplayAsTextComboBox->setCurrentIndex( mDisplayAsTextComboBox->findData( config.value( QStringLiteral( "TextDisplayMethod" ), QString::number( static_cast<int>( QgsCheckBoxFieldFormatter::ShowTrueFalse ) ) ).toInt() ) );
  mAllowNullState->setChecked( config.value( QStringLiteral( "AllowNullState" ) ).toBool() );
}
