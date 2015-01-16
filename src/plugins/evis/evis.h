/*
** File: evis.h
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-06
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
** This work was made possible through a grant by the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/

//This file was created using the plugin generator distributed with QGIS evis.h
//is based on and a modification of the default plugin.h file which carried the
//following header
/***************************************************************************
                          plugin.h
 Functions:
                             -------------------
    begin                : Jan 21, 2004
    copyright            : ( C ) 2004 by Tim Sutton
    email                : tim at linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *   QGIS Programming conventions:
 *
 *   mVariableName - a class level member variable
 *   sVariableName - a static class level member variable
 *   variableName() - accessor for a class member ( no 'get' in front of name )
 *   setVariableName() - mutator for a class member ( prefix with 'set' )
 *
 *   Additional useful conventions:
 *
 *   theVariableName - a method parameter ( prefix with 'the' )
 *   myVariableName - a locally declared variable within a method ( 'my' prefix )
 *
 *   DO: Use mixed case variable names - myVariableName
 *   DON'T: separate variable names using underscores: my_variable_name ( NO! )
 *
 * **************************************************************************/
#ifndef eVis_H
#define eVis_H

#include <QTemporaryFile>

#include <QObject>

#include <qgisplugin.h>
#include <qgisinterface.h>

//forward declarations
class QAction;
class QToolBar;
class eVisEventIdTool;

/**
* \class eVis
* \brief eVis plugin for QGIS
* This plugin allows for generic database connectivity and also provides a browser in which
* users can load and view photos referenced as attributes for a feature. The browser also
* has the ability to interact with the map canvas and display an arrow point in the direction
* in which a photograph was taken
*/
class eVis: public QObject, public QgisPlugin
{
    Q_OBJECT;
  public:

    //! Constructor
    eVis( QgisInterface * theInterface );

    //! Destructor */
    ~eVis();

  public slots:
    //! init the gui
    virtual void initGui() override;

    //! Main button actions
    void launchDatabaseConnection();
    void launchEventIdTool();
    void launchEventBrowser();

    //! unload the plugin
    void unload() override;

    //! show the help document
    void help();

    //! Add a vector layer given vectorLayerPath, baseName, providerKey ( "ogr" or "postgres" );
    void drawVectorLayer( QString, QString, QString );

  private:

    QgisInterface *mQGisIface;

    QAction* mDatabaseConnectionActionPointer;

    QAction* mEventIdToolActionPointer;

    QAction* mEventBrowserActionPointer;

    eVisEventIdTool* mIdTool;

    //! List of pointers to temporary files, files are created by database queries */
    QList<QTemporaryFile*> mTemporaryFileList;
};
#endif //eVis_H
