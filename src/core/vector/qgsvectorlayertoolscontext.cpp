/***************************************************************************
    qgsvectorlayertoolscontext.cpp
    ------------------------
    begin                : May 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayertoolscontext.h"
#include "qgsexpressioncontextutils.h"

QgsVectorLayerToolsContext::QgsVectorLayerToolsContext( const QgsVectorLayerToolsContext &other )
  : mParentWidget( other.mParentWidget )
  , mShowModal( other.mShowModal )
  , mHideParent( other.mHideParent )
{
  if ( other.mAdditionalExpressionContextScope )
  {
    mAdditionalExpressionContextScope.reset( new QgsExpressionContextScope( *other.mAdditionalExpressionContextScope ) );
  }
  if ( other.mExpressionContext )
  {
    mExpressionContext.reset( new QgsExpressionContext( *other.mExpressionContext ) );
  }
}

QgsVectorLayerToolsContext &QgsVectorLayerToolsContext::operator=( const QgsVectorLayerToolsContext &other )
{
  mParentWidget = other.mParentWidget;
  mShowModal = other.mShowModal;
  mHideParent = other.mHideParent;
  if ( other.mAdditionalExpressionContextScope )
  {
    mAdditionalExpressionContextScope.reset( new QgsExpressionContextScope( *other.mAdditionalExpressionContextScope ) );
  }
  if ( other.mExpressionContext )
  {
    mExpressionContext.reset( new QgsExpressionContext( *other.mExpressionContext ) );
  }
  else
  {
    mExpressionContext.reset();
  }
  return *this;
}

void QgsVectorLayerToolsContext::setExpressionContext( const QgsExpressionContext *context )
{
  if ( context )
    mExpressionContext.reset( new QgsExpressionContext( *context ) );
  else
    mExpressionContext.reset();
}

QgsExpressionContext *QgsVectorLayerToolsContext::expressionContext() const
{
  return mExpressionContext.get();
}

void QgsVectorLayerToolsContext::setAdditionalExpressionContextScope( const QgsExpressionContextScope *scope )
{
  if ( scope )
    mAdditionalExpressionContextScope.reset( new QgsExpressionContextScope( *scope ) );
  else
    mAdditionalExpressionContextScope.reset();
}

const QgsExpressionContextScope *QgsVectorLayerToolsContext::additionalExpressionContextScope() const
{
  return mAdditionalExpressionContextScope.get();
}
