/***************************************************************************
     qgssearchwidgettoolbutton.h
     ---------------------------
    Date                 : May 2016
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSEARCHWIDGETTOOLBUTTON_H
#define QGSSEARCHWIDGETTOOLBUTTON_H

#include "editorwidgets/core/qgssearchwidgetwrapper.h"
#include <QToolButton>

/** \ingroup gui
 * \class QgsSearchWidgetToolButton
 * A tool button widget which is displayed next to search widgets in forms, and
 * allows for controlling how the widget behaves and how the filtering/searching
 * operates.
 * \note Added in version 2.16
 */
class GUI_EXPORT QgsSearchWidgetToolButton : public QToolButton
{
    Q_OBJECT

  public:

    /** Constructor for QgsSearchWidgetToolButton.
     * @param parent parent object
     */
    explicit QgsSearchWidgetToolButton( QWidget *parent = nullptr );

    /** Sets the available filter flags to show in the widget. Any active flags
     * (see activeFlags()) which are not present in the new available filter
     * flags will be cleared;
     * @param flags available flags to show in widget
     * @see availableFlags()
     * @see setActiveFlags()
     * @see setDefaultFlags()
     */
    void setAvailableFlags( QgsSearchWidgetWrapper::FilterFlags flags );

    /** Sets the default filter flags to show in the widget.
     * @param flags default flags to show in widget
     * @see setAvailableFlags()
     * @see setActiveFlags()
     */
    void setDefaultFlags( QgsSearchWidgetWrapper::FilterFlags flags );

    /** Returns the available filter flags shown in the widget.
     * @see setAvailableFlags()
     * @see activeFlags()
     */
    QgsSearchWidgetWrapper::FilterFlags availableFlags() const { return mAvailableFilterFlags; }

    /** Sets the current active filter flags for the widget. Any flags
     * which are not present in the available filter flags (see availableFlags())
     * will not be set.
     * @param flags active flags to show in widget
     * @see toggleFlag()
     * @see activeFlags()
     * @see setAvailableFlags()
     */
    void setActiveFlags( QgsSearchWidgetWrapper::FilterFlags flags );

    /** Toggles an individual active filter flag for the widget. Any flags
     * which are not present in the available filter flags (see availableFlags())
     * will be ignore. Other flags may be cleared if they conflict with the newly
     * toggled flag.
     * @param flag flag to toggle
     * @see setActiveFlags()
     * @see activeFlags()
     */
    void toggleFlag( QgsSearchWidgetWrapper::FilterFlag flag );

    /** Returns the active filter flags shown in the widget.
     * @see setActiveFlags()
     * @see toggleFlag()
     * @see availableFlags()
     */
    QgsSearchWidgetWrapper::FilterFlags activeFlags() const { return mFilterFlags; }

    /** Returns true if the widget is set to be included in the search.
     * @see setInactive()
     * @see setActive()
     */
    bool isActive() const;

  public slots:

    /** Sets the search widget as inactive, ie do not search the corresponding field.
     * @see isActive()
     * @see setActive()
     */
    void setInactive();

    /** Sets the search widget as active by selecting the first available search type.
     * @see isActive()
     * @see setInactive()
     */
    void setActive();

  signals:

    /** Emitted when the active flags selected in the widget is changed
     * @param flags active flags
     */
    void activeFlagsChanged( QgsSearchWidgetWrapper::FilterFlags flags );

  private slots:

    void aboutToShowMenu();

    void actionSelected();

    void searchWidgetValueChanged();

  private:

    QgsSearchWidgetWrapper::FilterFlags mAvailableFilterFlags;
    QgsSearchWidgetWrapper::FilterFlags mDefaultFilterFlags;
    QgsSearchWidgetWrapper::FilterFlags mFilterFlags;
    QMenu* mMenu;

    void updateState();

};

#endif // QGSSEARCHWIDGETTOOLBUTTON_H
