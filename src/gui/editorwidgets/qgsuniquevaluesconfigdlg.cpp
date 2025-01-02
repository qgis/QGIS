/***************************************************************************
    qgsuniquevaluesconfigdlg.cpp
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

#include "qgsuniquevaluesconfigdlg.h"
#include "moc_qgsuniquevaluesconfigdlg.cpp"

QgsUniqueValuesConfigDlg::QgsUniqueValuesConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  connect( editableUniqueValues, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
}


QVariantMap QgsUniqueValuesConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "Editable" ), editableUniqueValues->isChecked() );

  return cfg;
}

void QgsUniqueValuesConfigDlg::setConfig( const QVariantMap &config )
{
  editableUniqueValues->setChecked( config.value( QStringLiteral( "Editable" ), false ).toBool() );
}
