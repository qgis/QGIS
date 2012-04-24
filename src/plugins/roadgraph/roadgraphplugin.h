/***************************************************************************
  roadgraphplugin.h
  --------------------------------------
  Date                 : 2010-10-10
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/
#ifndef ROADGRAPHPLUGIN
#define ROADGRAPHPLUGIN

//QT4 includes
#include <QObject>

//QGIS includes
#include <qgisplugin.h>
#include <qgspoint.h>

//forward declarations
class QAction;
class QToolBar;
class QPainter;
class QgisInterface;
class QDockWidget;

//forward declarations RoadGraph plugins classes
class QgsGraphDirector;
class RgShortestPathWidget;
class RgLineVectorLayerSettings;

/**
* \class RoadGraphPlugin
* \brief Road graph plugin for QGIS
* This plugin can solve the shotrest path problem and etc...
*/
class RoadGraphPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    /**
     * Constructor for a plugin. The QgisInterface pointer is passed by
     * QGIS when it attempts to instantiate the plugin.
     * @param theQgisInterface Pointer to the QgisInterface object.
     */
    RoadGraphPlugin( QgisInterface * theQgisInterface );
    //! Destructor
    virtual ~RoadGraphPlugin();
    /**
     * return pointer to my Interface
     */
    QgisInterface *iface();

    /**
     * return pointer to graph director
     */
    const QgsGraphDirector* director() const;

    /**
     * get time unit name
     */
    QString timeUnitName();

    /**
     * get distance unit name
     */
    QString distanceUnitName();

    /**
     * get topology tolerance factor
     */
    double topologyToleranceFactor();

  public slots:
    //! init the gui
    virtual void initGui();

    //!set values onthe gui when a project is read or the gui first loaded
    virtual void projectRead();

    //!set default values for new project
    void newProject();

    //! Show the property dialog box
    void property();

    //! unload the plugin
    void unload();

    //! show the help document
    void help();

  private slots:
    /**
     * set show roads direction
     */
    void onShowDirection();

  private:
    /**
     * set all gui elements to default status
     */
    void setGuiElementsToDefault( );

  private:

    ////////////////////////////////////////////////////////////////////
    //
    // MANDATORY PLUGIN PROPERTY DECLARATIONS  .....
    //
    ////////////////////////////////////////////////////////////////////
    int mPluginType;

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;

    ////////////////////////////////////////////////////////////////////
    // ADD YOUR OWN PROPERTY DECLARATIONS AFTER THIS POINT.....
    //
    ////////////////////////////////////////////////////////////////////
    /**
    * on show settings
    */
    QAction * mQSettingsAction;

    /**
     * GUI for use shortest path finder
     */
    RgShortestPathWidget *mQShortestPathDock;

    /**
     * My graph settings.
     * @note. Should be used RgSettings
     */
    RgLineVectorLayerSettings *mSettings;

    /**
     *  time unit for results presentation
     */
    QString mTimeUnitName;

    /**
     * distance unit for results presentation
     */
    QString mDistanceUnitName;

    /**
     * topology tolerance factor
     */
    double mTopologyToleranceFactor;

};

#endif //ROADGRAPHPLUGIN
