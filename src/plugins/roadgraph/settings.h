/***************************************************************************
  settings.h
  --------------------------------------
  Date                 : 2010-10-18
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
#ifndef ROADGRAPH_SETTINGS
#define ROADGRAPH_SETTINGS

//QT4 includes

//QGIS includes

//forward declarations
class QWidget;
class QgsProject;

/**
* \class RgGraphDirector
* \brief Determine making the graph
* contained the settings
*/
class RgSettings
{
  public:
    //! Destructor
    virtual ~RgSettings() { }

    /**
     * write settings to the poject file
     */
    virtual void write( QgsProject * ) = 0;
    /**
     * read settings form project file
     */
    virtual void read( const QgsProject * ) = 0;
    /**
     * This function test settings and return true if setting correct
     */
    virtual bool test() = 0;
    /**
     * Make settings widget
     * use it for GUI setting
     */
    virtual QWidget* getGui( QWidget* parent ) = 0;
    /**
     * Load settings from widget
     */
    virtual void setFromGui( QWidget * ) = 0;
};
#endif //ROADGRAPH_SETTIGNS
