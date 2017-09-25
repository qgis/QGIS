/***************************************************************************
  qgsactionscope.cpp - QgsActionScope

 ---------------------
 begin                : 25.09.2017
 copyright            : (C) 2017 by Clément MARCEL
 email                : clement.marcel@nwanda.fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscontextaction.h"

QgsContextAction::QgsContextAction( QObject *parent ):
  QAction( parent )
{
}

QgsContextAction::QgsContextAction( const QString &text, QObject *parent ):
  QAction( text, parent )
{
}

QgsContextAction::QgsContextAction( const QIcon &icon, const QString &text, QObject *parent ):
  QAction( icon, text, parent )
{
}

QgsExpressionContextScope QgsContextAction::expressionContextScope() const
{
  return mExpressionContextScope;
}

void QgsContextAction::setExpressionContextScope( const QgsExpressionContextScope &expressionContextScope )
{
  mExpressionContextScope = expressionContextScope;
}
