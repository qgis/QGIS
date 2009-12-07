/***************************************************************************
    qgsapplegendinterface.h
     --------------------------------------
    Date                 : 23-Nov-2009
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

#ifndef QGSLEGENDAPPIFACE_H
#define QGSLEGENDAPPIFACE_H

#include "qgslegendinterface.h"

#include <QModelIndex>

class QgsLegend;
class QgsMapLayer;

/** \ingroup gui
 * QgsLegendInterface
 * Abstract base class to make QgsLegend available to plugins.
 */
class QgsAppLegendInterface : public QgsLegendInterface
{
    Q_OBJECT

  public:

    /** Constructor */
    explicit QgsAppLegendInterface( QgsLegend * legend );

    /** Virtual destructor */
    ~QgsAppLegendInterface();

    //! Return a string list of groups
    QStringList groups();

  public slots:

    //! Add a new group
    int addGroup( QString name, bool expand = true );

    //! Remove all groups with the given name
    void removeGroup( int groupIndex );

    //! Move a layer to a group
    void moveLayer( QgsMapLayer * ml, int groupIndex );

    //! Update an index
    void updateIndex( QModelIndex oldIndex, QModelIndex newIndex );

  private:

    //! Pointer to QgsLegend object
    QgsLegend *mLegend;
};

#endif //QGSLEGENDAPPIFACE_H
