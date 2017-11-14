/***************************************************************************
    qgscomposerinterface.h
     ---------------------
    Date                 : March 2017
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

#ifndef QGSCOMPOSERINTERFACE_H
#define QGSCOMPOSERINTERFACE_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QObject>

class QgsComposerView;
class QgsComposition;

/**
 * \ingroup gui
 * \class QgsComposerInterface
 * A common interface for composer dialogs.
 *
 * Provides a common interface and stable API for composer editor dialogs.
 * This interface can be used by plugins and scripts to interact with
 * open composer dialogs.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsComposerInterface: public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsComposerInterface.
     */
    QgsComposerInterface( QObject *parent SIP_TRANSFERTHIS = 0 )
      : QObject( parent )
    {}

    virtual ~QgsComposerInterface() = default;

    /**
     * Returns the composer's QgsComposerView editor widget.
     */
    virtual QgsComposerView *view() = 0;

    /**
     * Returns the composition displated in the composer.
     */
    virtual QgsComposition *composition() = 0;

    /**
     * Closes the composer window.
     */
    virtual void close() = 0;

};

#endif // QGSCOMPOSERINTERFACE_H
