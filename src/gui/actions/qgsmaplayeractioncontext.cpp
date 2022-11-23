/***************************************************************************
    qgsmaplayeractioncontext.cpp
    -----------------------------
    begin                : January 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayeractioncontext.h"
#include "qgsattributedialog.h"
#include "qgsmessagebar.h"


QgsMapLayerActionContext::QgsMapLayerActionContext() = default;

QgsAttributeDialog *QgsMapLayerActionContext::attributeDialog() const
{
  return mAttributeDialog;
}

void QgsMapLayerActionContext::setAttributeDialog( QgsAttributeDialog *dialog )
{
  mAttributeDialog = dialog;
}

QgsMessageBar *QgsMapLayerActionContext::messageBar() const
{
  return mMessageBar;
}

void QgsMapLayerActionContext::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}
