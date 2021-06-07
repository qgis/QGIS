/***************************************************************************
    qgsattributeeditorcontext.cpp
     --------------------------------------
    Date                 : 19.8.2019
    Copyright            : (C) 201 Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeeditorcontext.h"

void QgsAttributeEditorContext::setCadDockWidget( QgsAdvancedDigitizingDockWidget *cadDockWidget )
{
  mCadDockWidget = cadDockWidget;
}
