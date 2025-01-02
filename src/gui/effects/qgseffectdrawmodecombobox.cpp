/***************************************************************************
    qgseffectdrawmodecombobox.cpp
    -----------------------------
    begin                : March 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseffectdrawmodecombobox.h"
#include "moc_qgseffectdrawmodecombobox.cpp"
#include "qgsapplication.h"

#include <QList>
#include <QPair>

QgsEffectDrawModeComboBox::QgsEffectDrawModeComboBox( QWidget *parent )
  : QComboBox( parent )
{
  QList<QPair<QgsPaintEffect::DrawMode, QString>> modes;
  modes << qMakePair( QgsPaintEffect::Render, tr( "Render only" ) )
        << qMakePair( QgsPaintEffect::Modifier, tr( "Modifier only" ) )
        << qMakePair( QgsPaintEffect::ModifyAndRender, tr( "Render and modify" ) );

  for ( int i = 0; i < modes.count(); i++ )
  {
    const QgsPaintEffect::DrawMode mode = modes.at( i ).first;
    const QString name = modes.at( i ).second;
    addItem( name, QVariant( ( int ) mode ) );
  }
}

QgsPaintEffect::DrawMode QgsEffectDrawModeComboBox::drawMode() const
{
  return ( QgsPaintEffect::DrawMode ) currentData().toInt();
}

void QgsEffectDrawModeComboBox::setDrawMode( QgsPaintEffect::DrawMode drawMode )
{
  const int idx = findData( QVariant( ( int ) drawMode ) );
  setCurrentIndex( idx == -1 ? 0 : idx );
}
