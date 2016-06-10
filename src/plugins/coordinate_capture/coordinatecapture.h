/***************************************************************************
    coordinatecapture.h
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
/***************************************************************************
 *   QGIS Programming conventions:
 *
 *   mVariableName - a class level member variable
 *   sVariableName - a static class level member variable
 *   variableName() - accessor for a class member (no 'get' in front of name)
 *   setVariableName() - mutator for a class member (prefix with 'set')
 *
 *   Additional useful conventions:
 *
 *   theVariableName - a method parameter (prefix with 'the')
 *   myVariableName - a locally declared variable within a method ('my' prefix)
 *
 *   DO: Use mixed case variable names - myVariableName
 *   DON'T: separate variable names using underscores: my_variable_name (NO!)
 *
 * **************************************************************************/
#ifndef COORDINATECAPTURE_H
#define COORDINATECAPTURE_H

//QT4 includes
#include <QObject>
#include <QPointer>

//QGIS includes
#include "../qgisplugin.h"
#include "coordinatecapturemaptool.h"
#include <qgscoordinatereferencesystem.h>
#include <qgscoordinatetransform.h>

//forward declarations
class QAction;
class QToolBar;
class QToolButton;
class QPushButton;
class QgsDockWidget;
class QLineEdit;
class QIcon;
class QLabel;

class QgisInterface;
class QgsPoint;

/**
* \class Plugin
* \brief [name] plugin for QGIS
* [description]
*/
class CoordinateCapture: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:

    //////////////////////////////////////////////////////////////////////
    //
    //                MANDATORY PLUGIN METHODS FOLLOW
    //
    //////////////////////////////////////////////////////////////////////

    /**
    * Constructor for a plugin. The QgisInterface pointer is passed by
    * QGIS when it attempts to instantiate the plugin.
    * @param theInterface Pointer to the QgisInterface object.
     */
    explicit CoordinateCapture( QgisInterface * theInterface );
    //! Destructor
    virtual ~CoordinateCapture();

  public slots:
    //! init the gui
    virtual void initGui() override;
    //! Show the dialog box
    void run();
    //! unload the plugin
    void unload() override;
    //! Show/hide the dockwidget
    void showOrHide();
    //! show the help document
    void help();
    //! Set the Coordinate Reference System used for displaying non canvas CRS coord
    void setCRS();
    //! Called when mouse clicks on the canvas. Will populate text box with coords.
    void mouseClicked( const QgsPoint& thePoint );
    /** Called when mouse moved over the canvas. If the tracking button is toggled,
     * the text box coords will be updated. */
    void mouseMoved( const QgsPoint& thePoint );
    //! Called when mouse is clicked on the canvas
    void update( const QgsPoint& thePoint );
    //! Called when user clicks the copy button
    void copy();
    //! called when the project's CRS is changed
    void setSourceCrs();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( const QString& theThemeName );

  private:
    //! Container for the coordinate info
    QPointer<QgsDockWidget> mpDockWidget;

    //!output display for user defined Coordinate Reference System
    QPointer<QLineEdit> mpUserCrsEdit;

    //!output display for CRS  coord
    QPointer<QLineEdit> mpCanvasEdit;

    //!Our custom map tool to capture clicks
    CoordinateCaptureMapTool * mpMapTool;

    //!A two buttons to track and capture coordinates
    QToolButton * mpTrackMouseButton;
    QPushButton * mpCaptureButton;

    //! A toolbutton to select crs to display the coordinates
    QToolButton * mypUserCrsToolButton;

    //! A label for coordinates in the project crs
    QLabel * mypCRSLabel;

    //! transform object
    QgsCoordinateTransform mTransform;

    //! map coordinate display precision
    int mCanvasDisplayPrecision;

    //! user CRS object
    QgsCoordinateReferenceSystem mCrs;

    //! user coordinate display precision
    int mUserCrsDisplayPrecision;

    //! Get the path to the icon from the best available theme
    QString getIconPath( const QString& theName );

    ////////////////////////////////////////////////////////////////////
    //
    // MANDATORY PLUGIN PROPERTY DECLARATIONS  .....
    //
    ////////////////////////////////////////////////////////////////////

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //!pointer to the qaction for this plugin
    QAction * mQActionPointer;
    ////////////////////////////////////////////////////////////////////
    //
    // ADD YOUR OWN PROPERTY DECLARATIONS AFTER THIS POINT.....
    //
    ////////////////////////////////////////////////////////////////////
};

#endif //CoordinateCapture_H
