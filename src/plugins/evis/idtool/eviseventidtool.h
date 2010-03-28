/*
** File: eviseventidtool.h
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-19
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
/*  $Id$ */
#ifndef EVISEVENTIDTOOL_H
#define EVISEVENTIDTOOL_H

#include <QMouseEvent>
#include <QWidget>

#include "qgsmaplayer.h"
#include "qgsmaptool.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"

#include "evisgenericeventbrowsergui.h"

/**
* \class eVisEventIdTool
* \brief Map tool for launching event browser
* The eVisEventIdTool is an id style map tool that is used to select point and launch the generic event browser
* to view the associated attributes for each selected feature, specifically features with associated photographic data
*/
class eVisEventIdTool : public QgsMapTool
{

  public:
    /** \brief Constructor */
    eVisEventIdTool( QgsMapCanvas* );

    /** \brief Method to handle mouse release, i.e., select, event */
    void canvasReleaseEvent( QMouseEvent* );

  private:

    /** \brief Pointer to a generic event browser */
    eVisGenericEventBrowserGui* mBrowser;

    /** \brief Selection routine called by the mouse release event */
    void select( QgsPoint );
};
#endif
