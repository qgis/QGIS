/***************************************************************************
                         qgsprocessingmodelcomment.cpp
                         --------------------------
    begin                : February 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
