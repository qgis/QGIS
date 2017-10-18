/***************************************************************************
    qgscomposerinterface.h
     ---------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTDESIGNERINTERFACE_H
#define QGSLAYOUTDESIGNERINTERFACE_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QObject>

class QgsLayout;
class QgsLayoutView;

/**
 * \ingroup gui
 * \class QgsLayoutDesignerInterface
 * A common interface for layout designer dialogs and widgets.
 *
 * Provides a common interface and stable API for layout designer dialogs and widgets.
 * This interface can be used by plugins and scripts to interact with
 * open layout designer dialogs.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutDesignerInterface: public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutDesignerInterface.
     */
    QgsLayoutDesignerInterface( QObject *parent SIP_TRANSFERTHIS = 0 )
      : QObject( parent )
    {}

    virtual ~QgsLayoutDesignerInterface() = default;

    /**
     * Returns the layout displayed in the designer.
     * \see view()
     */
    virtual QgsLayout *layout() = 0;


    /**
     * Returns the layout view utilized by the designer.
     * \see layout()
     */
    virtual QgsLayoutView *view() = 0;

  public slots:

    /**
     * Closes the layout designer.
     */
    virtual void close() = 0;

};

#endif // QGSLAYOUTDESIGNERINTERFACE_H
