/***************************************************************************
                              qgscheckablecombobox.cpp
                              ------------------------
  begin                : May 25, 2023
  copyright            : (C) 2017 by Mathieu Pellerin
  email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstooltipcombobox.h"
#include "moc_qgstooltipcombobox.cpp"

#include <QEvent>
#include <QHelpEvent>
#include <QPoint>
#include <QToolTip>


QgsToolTipComboBox::QgsToolTipComboBox( QWidget *parent )
  : QComboBox( parent )
{
}

bool QgsToolTipComboBox::event( QEvent *event )
{
  if ( event->type() == QEvent::ToolTip )
  {
    const QString description = currentData( Qt::ToolTipRole ).toString();
    if ( !description.isEmpty() )
    {
      QHelpEvent *helpEvent = static_cast<QHelpEvent *>( event );
      QPoint pos = mapToGlobal( helpEvent->pos() );
      QToolTip::showText( pos, description );
    }
    return true;
  }
  return QComboBox::event( event );
}
