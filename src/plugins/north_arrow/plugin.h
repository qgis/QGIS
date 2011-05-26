/***************************************************************************
                          plugin.h
 Functions:
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNORTHARROWPLUGIN
#define QGSNORTHARROWPLUGIN

#include "../qgisplugin.h"

#include <QObject>
#include <QStringList>
class QgisInterface;
class QAction;
class QToolBar;
class QPainter;
/**
* \class Plugin
* \brief North Arrow plugin for QGIS
*
*/
class QgsNorthArrowPlugin: public QObject, public QgisPlugin
{
  Q_OBJECT public:
    /**
     * Constructor for a plugin. The QgisInterface pointer is passed by
     * QGIS when it attempts to instantiate the plugin.
     * @param qI Pointer to the QgisInterface object.
     */
    QgsNorthArrowPlugin( QgisInterface * );
    //! Destructor
    virtual ~QgsNorthArrowPlugin();
  public slots:
    //! init the gui
    virtual void initGui();
    //!set values on the gui when a project is read or the gui first loaded
    void projectRead();
    //! Show the dialog box
    void run();
    // draw some arbitary text to the screen
    void renderNorthArrow( QPainter * );
    //! Run when the user has set a new rotation
    void rotationChanged( int );
    //! Refresh the map display using the mapcanvas exported via the plugin interface
    void refreshCanvas();
    //! unload the plugin
    void unload();
    //! show the help document
    void help();
    //! set north arrow placement
    void setPlacement( int );
    //! enable or disable north arrow
    void setEnabled( bool );
    //! enable or disable the automatic setting of the arrow direction
    void setAutomatic( bool );
    //! try to calculate the direction for the north arrow. Sets the
    // private class rotation variable. If unable to calculate the
    // direction, the function returns false and leaves the rotation
    // variable as is.
    bool calculateNorthDirection();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );

  private:

    static const double PI;
    //  static const double DEG2RAD;
    static const double TOL;

    // The amount of rotation for the north arrow
    int mRotationInt;
    int pluginType;
    // enable or disable north arrow
    bool mEnable;
    //! enable or disable the automatic setting of the arrow direction
    bool mAutomatic;
    // The placement index and translated text
    int mPlacementIndex;
    QStringList mPlacementLabels;
    //! Pointer to the QGIS interface object
    QgisInterface *qGisInterface;
    //! Pointer to the QAction object used in the menu and toolbar
    QAction *myQActionPointer;
};

#endif
