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
/*  $Id$ */
#ifndef QGSCOPYRIGHTLABELPLUGIN
#define QGSCOPYRIGHTLABELPLUGIN
#include "../qgisplugin.h"

#include <QColor>
#include <QFont>
#include <QObject>
class QAction;
class QPainter;

class QgisInterface;

/**
* \class Plugin
* \brief OpenModeller plugin for QGIS
*
*/
class QgsCopyrightLabelPlugin: public QObject, public QgisPlugin
{
  Q_OBJECT public:
    /**
     * Constructor for a plugin. The QgisInterface pointer is passed by
     * QGIS when it attempts to instantiate the plugin.
     * @param qI Pointer to the QgisInterface object.
     */
    QgsCopyrightLabelPlugin( QgisInterface * );
    //! Destructor
    virtual ~ QgsCopyrightLabelPlugin();
    void writeEntry( QString theScope, QString theProperty, QVariant theValue );
  public slots:
    //! init the gui
    void initGui();
    //!set values on the gui when a project is read or the gui first loaded
    void projectRead();
    //! Show the dialog box
    void run();
    void renderLabel( QPainter * );
    //! Refresh the map display using the mapcanvas exported via the plugin interface
    void refreshCanvas();
    //! unload the plugin
    void unload();
    //! show the help document
    void help();
    //! change the copyright font
    void setFont( QFont );
    //! change the copyright text
    void setLabel( QString );
    //! change the copyright font color
    void setColor( QColor );
    //! set copyright label placement
    void setPlacement( int );
    //! set copyright label enabled
    void setEnable( bool );
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );



  private:
    //! This is the font that will be used for the copyright label
    QFont mQFont;
    //! This is the string that will be used for the copyright label
    QString mLabelQString;
    //! This is the color for the copyright label
    QColor mLabelQColor;
    //! Placement of the copyright label - index and translated label names
    int mPlacementIndex;
    QStringList mPlacementLabels;
    //! Copyright label enabled
    bool mEnable;

    int pluginType;
    //! Pointer to the QGIS interface object
    QgisInterface *qGisInterface;
    //! Pointer to the QAction object used in the menu and toolbar
    QAction *myQActionPointer;
};

#endif
