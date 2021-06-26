/***************************************************************************
    qgsdevtoolwidgetfactory.cpp
     --------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsdevtoolwidgetfactory.h"

QgsDevToolWidgetFactory::QgsDevToolWidgetFactory( const QString &title, const QIcon &icon )
  : mIcon( icon )
  , mTitle( title )
{
}
