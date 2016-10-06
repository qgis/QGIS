/***************************************************************************
    qgsfocuswatcher.h
    -----------------
    Date                 : April 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFOCUSWATCHER_H
#define QGSFOCUSWATCHER_H

#include <QObject>

/** \ingroup gui
 * \class QgsFocusWatcher
 * A event filter for watching for focus events on a parent object. Usually QObjects must
 * subclass and override methods like focusOutEvent to handle focus events. Using this class
 * as an event filter avoids the need to subclass objects and the focus events can be directly
 * caught using the emitted signals.
 * \note added in 2.16
 */

class GUI_EXPORT QgsFocusWatcher : public QObject
{
    Q_OBJECT

  public:

    /** Constructor for QgsFocusWatcher.
     * @param parent parent widget to catch focus events for. This class will automatically be
     * installed as an event filter for parent.
     */
    explicit QgsFocusWatcher( QObject* parent );

    virtual bool eventFilter( QObject* obj, QEvent* event ) override;

  signals:

    /** Emitted when parent object's focus changes.
     * @param focused true if object gained focus, false if object lost focus
     */
    void focusChanged( bool focused );

    //! Emitted when parent object gains focus.
    void focusIn();

    //! Emitted when parent object loses focus.
    void focusOut();

};

#endif //QGSFOCUSWATCHER_H
