/***************************************************************************
    qgseffectdrawmodecombobox.cpp
    -----------------------------
    begin                : March 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgseffectdrawmodecombobox.h"
#include "qgsapplication.h"

#include <QList>
#include <QPair>

QgsEffectDrawModeComboBox::QgsEffectDrawModeComboBox( QWidget *parent )
  : QComboBox( parent )
{
  QList < QPair<QgsPaintEffect::DrawMode, QString> > modes;
  modes << qMakePair( QgsPaintEffect::Render, tr( "Render only" ) )
        << qMakePair( QgsPaintEffect::Modifier, tr( "Modifier only" ) )
        << qMakePair( QgsPaintEffect::ModifyAndRender, tr( "Render and modify" ) );

  for ( int i = 0; i < modes.count(); i++ )
  {
    QgsPaintEffect::DrawMode mode = modes.at( i ).first;
    QString name = modes.at( i ).second;
    addItem( name, QVariant( ( int ) mode ) );
  }
}

QgsPaintEffect::DrawMode QgsEffectDrawModeComboBox::drawMode() const
{
  return ( QgsPaintEffect::DrawMode ) currentData().toInt();
}

void QgsEffectDrawModeComboBox::setDrawMode( QgsPaintEffect::DrawMode drawMode )
{
  int idx = findData( QVariant( ( int ) drawMode ) );
  setCurrentIndex( idx == -1 ? 0 : idx );
}
