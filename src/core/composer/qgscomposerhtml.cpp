/***************************************************************************
                              qgscomposerhtml.cpp
    ------------------------------------------------------------
    begin                : Julli 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerhtml.h"

QgsComposerHtml::QgsComposerHtml( QgsComposition* c ): QgsComposerMultiFrame( c )
{
}

QgsComposerHtml::~QgsComposerHtml()
{
}

QSizeF QgsComposerHtml::totalSize() const
{
  return QSizeF(); //soon...
}
