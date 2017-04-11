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

QgsCheckBoxConfigDlg::QgsCheckBoxConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  connect( leCheckedState, &QLineEdit::textEdited, this, &QgsEditorConfigWidget::changed );
  connect( leUncheckedState, &QLineEdit::textEdited, this, &QgsEditorConfigWidget::changed );

  if ( vl->fields().at( fieldIdx ).type() == QVariant::Bool )
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

  return cfg;
}

void QgsCheckBoxConfigDlg::setConfig( const QVariantMap &config )
{
  if ( layer()->fields().at( field() ).type() != QVariant::Bool )
  {
    leCheckedState->setText( config.value( QStringLiteral( "CheckedState" ) ).toString() );
    leUncheckedState->setText( config.value( QStringLiteral( "UncheckedState" ) ).toString() );
  }
}
