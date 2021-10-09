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
#include <QColor>
#include "qgis_app.h"

class QAction;
class QToolBar;
class QPainter;

class APP_EXPORT QgsDecorationNorthArrow: public QgsDecorationItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDecorationNorthArrow, with the specified \a parent object.
     */
    QgsDecorationNorthArrow( QObject *parent = nullptr );

  public slots:
    //! Sets values on the gui when a project is read or the gui first loaded
    void projectRead() override;
    //! Save values to the project
    void saveToProject() override;

    //! Show the dialog box
    void run() override;
    //! Draw some arbitrary text to the screen
    void render( const QgsMapSettings &mapSettings, QgsRenderContext &context ) override;

    //! Returns the north arrow SVG path
    QString svgPath();

  private:

    //  static const double DEG2RAD;
    static const double TOL;

    //! The north arrow fill color
    QColor mColor;
    //! The north arrow outline color
    QColor mOutlineColor;
    //! The north arrow size in millimeter
    double mSize = 16.0;
    //! Custom north arrow svg path
    QString mSvgPath;

    // The amount of rotation for the north arrow
    int mRotationInt = 0;

    //! enable or disable the automatic setting of the arrow direction
    bool mAutomatic = true;
    //! margin values
    int mMarginHorizontal = 0;
    int mMarginVertical = 0;

    friend class QgsDecorationNorthArrowDialog;
};

#endif
