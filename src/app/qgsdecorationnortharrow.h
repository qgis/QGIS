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

#include "qgsdecorationitem.h"

#include <QStringList>

class QAction;
class QToolBar;
class QPainter;

class APP_EXPORT QgsDecorationNorthArrow: public QgsDecorationItem
{
    Q_OBJECT

  public:
    //! Constructor
    QgsDecorationNorthArrow( QObject* parent = NULL );
    //! Destructor
    virtual ~QgsDecorationNorthArrow();

  public slots:
    //! set values on the gui when a project is read or the gui first loaded
    void projectRead() override;
    //! save values to the project
    void saveToProject() override;

    //! Show the dialog box
    void run() override;
    //! draw some arbitary text to the screen
    void render( QPainter * ) override;

    //! try to calculate the direction for the north arrow. Sets the
    //! private class rotation variable. If unable to calculate the
    //! direction, the function returns false and leaves the rotation
    //! variable as is.
    bool calculateNorthDirection();

  private:

    static const double PI;
    //  static const double DEG2RAD;
    static const double TOL;

    // The amount of rotation for the north arrow
    int mRotationInt;
    int pluginType;
    //! enable or disable the automatic setting of the arrow direction
    bool mAutomatic;
    // The placement index and translated text
    int mPlacementIndex;
    QStringList mPlacementLabels;

    friend class QgsDecorationNorthArrowDialog;
};

#endif
