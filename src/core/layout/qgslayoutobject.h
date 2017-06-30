/***************************************************************************
                         qgslayoutobject.h
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTOBJECT_H
#define QGSLAYOUTOBJECT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>
#include <QDomNode>
#include <QMap>

class QgsLayout;
class QPainter;

/**
 * \ingroup core
 * A base class for objects which belong to a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutObject: public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsLayoutObject, with the specified parent \a layout.
     * \note While ownership of a QgsLayoutObject is not passed to the layout,
     * classes which are derived from QgsLayoutObject (such as QgsLayoutItem)
     * may transfer their ownership to a layout upon construction.
     */
    QgsLayoutObject( QgsLayout *layout );

    /**
     * Returns the layout the object is attached to.
     */
    SIP_SKIP const QgsLayout *layout() const { return mLayout; }

    /**
     * Returns the layout the object is attached to.
     */
    QgsLayout *layout() { return mLayout; }

  protected:

    QgsLayout *mLayout = nullptr;

  private:

    friend class TestQgsLayoutObject;
};

#endif //QGSLAYOUTOBJECT_H
