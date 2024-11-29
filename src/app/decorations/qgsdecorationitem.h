/***************************************************************************
                         qgsdecorationitem.h
                         ----------------------
    begin                : May 10, 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDECORATIONITEM_H
#define QGSDECORATIONITEM_H

#include <QObject>

#include "qgsmapdecoration.h"
#include "qgsunittypes.h"
#include "qgis_app.h"

class QPainter;

#define INCHES_TO_MM 0.0393700787402

class APP_EXPORT QgsDecorationItem : public QObject, public QgsMapDecoration
{
    Q_OBJECT

  public:
    //! Item placements
    enum Placement
    {
      BottomLeft = 0,
      TopLeft,
      TopRight,
      BottomRight,
      TopCenter,
      BottomCenter
    };

    //! Constructor
    QgsDecorationItem( QObject *parent = nullptr );

    void setEnabled( bool enabled ) { mEnabled = enabled; }
    bool enabled() const { return mEnabled; }

    /**
     * Returns the current placement for the item.
     * \see setPlacement()
     */
    Placement placement() const { return mPlacement; }

    /**
     * Sets the placement of the item.
     * \see placement()
     */
    void setPlacement( Placement placement ) { mPlacement = placement; }

  signals:
    void toggled( bool t );

  public slots:
    //! Sets values on the gui when a project is read or the gui first loaded
    virtual void projectRead();
    //! save values to the project
    virtual void saveToProject();

    //! Show the dialog box
    virtual void run() {}

    //! Redraws the decoration
    void update();

  protected:
    //! True if decoration item has to be displayed
    bool mEnabled = false;

    //! Placement of the decoration
    Placement mPlacement = TopLeft;
    //! Units used for the decoration placement margin
    Qgis::RenderUnit mMarginUnit = Qgis::RenderUnit::Millimeters;

    QString mConfigurationName;
};

#endif
