/***************************************************************************
    qgslegendinterface.h
     --------------------------------------
    Date                 : 19-Nov-2009
    Copyright            : (C) 2009 by Andres Manz
    Email                : manz dot andres at gmail dot com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSLEGENDINTERFACE_H
#define QGSLEGENDINTERFACE_H

#include <QObject>
#include <QStringList>

class QgsMapLayer;

/** \ingroup gui
 * QgsLegendInterface
 * Abstract base class to make QgsLegend available to plugins.
 *
 * \note added in 1.4
 */
class GUI_EXPORT QgsLegendInterface : public QObject
{
    Q_OBJECT

  public:

    /** Constructor */
    QgsLegendInterface();

    /** Virtual destructor */
    virtual ~QgsLegendInterface();

    //! Return a string list of groups
    virtual QStringList groups() = 0;

  signals:

    //! emitted when a group index has changed
    void groupIndexChanged( int oldIndex, int newIndex );

  public slots:

    //! Add a new group
    virtual int addGroup( QString name, bool expand = true ) = 0;

    //! Remove group on index
    virtual void removeGroup( int groupIndex ) = 0;

    //! Move a layer to a group
    virtual void moveLayer( QgsMapLayer * ml, int groupIndex ) = 0;
};

#endif
