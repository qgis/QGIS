/***************************************************************************
                         qgsprocessingmodelcomment.cpp
                         --------------------------
    begin                : February 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmodelcomment.h"

///@cond NOT_STABLE

QgsProcessingModelComment::QgsProcessingModelComment( const QString &description )
  : QgsProcessingModelComponent( description )
{
  setSize( QSizeF( 100, 60 ) );
}

QgsProcessingModelComment *QgsProcessingModelComment::clone() const
{
  return new QgsProcessingModelComment( *this );
}

QVariant QgsProcessingModelComment::toVariant() const
{
  QVariantMap map;
  saveCommonProperties( map );
  return map;
}

bool QgsProcessingModelComment::loadVariant( const QVariantMap &map )
{
  restoreCommonProperties( map );
  return true;
}


///@endcond
