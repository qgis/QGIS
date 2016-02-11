/***************************************************************************
 * Copyright (C) 2008 Paolo L. Scala, Barbara Rita Barricelli, Marco Padula *
 * CNR, Milan Unit (Information Technology),                               *
 * Construction Technologies Institute.\n";                                *
 *                                                                         *
 * email : Paolo L. Scala <scala@itc.cnr.it>                               *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 * **************************************************************************/
#ifndef dxf2shpConverter_H
#define dxf2shpConverter_H

//QT4 includes
#include <QObject>

//QGIS includes
#include "../qgisplugin.h"

//forward declarations
class QAction;

class QgisInterface;

/**
 * \class dxf2shpConverter:
 * \brief DXF importer plugin for QGIS
 */
class dxf2shpConverter: public QObject, public QgisPlugin
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
    explicit dxf2shpConverter( QgisInterface *theInterface );
    //! Destructor
    virtual ~dxf2shpConverter();

  public slots:
    //! init the gui
    virtual void initGui() override;
    //! Show the dialog box
    void run();
    //! unload the plugin
    void unload() override;
    //! show the help document
    void help();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( const QString& theThemeName );

    void addMyLayer( const QString&, const QString& );

  private:

    ////////////////////////////////////////////////////////////////////
    //
    // MANDATORY PLUGIN PROPERTY DECLARATIONS  .....
    //
    ////////////////////////////////////////////////////////////////////

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //!pointer to the qaction for this plugin
    QAction *mQActionPointer;
    ////////////////////////////////////////////////////////////////////
    //
    // ADD YOUR OWN PROPERTY DECLARATIONS AFTER THIS POINT.....
    //
    ////////////////////////////////////////////////////////////////////
    QString mQString;
};

#endif //dxf2shpConverter_H
