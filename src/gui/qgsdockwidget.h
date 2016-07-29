/***************************************************************************
                             qgsdockwidget.h
                             ---------------
    begin                : June 2016
    copyright            : (C) 2016 by Nyall Dawson
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
#ifndef QGSDOCKWIDGET_H
#define QGSDOCKWIDGET_H

#include <QDockWidget>

/** \ingroup gui
 * \class QgsDockWidget
 * QgsDockWidget subclass with more fine-grained control over how the widget is closed or opened.
 * \note added in 2.16
 */

class GUI_EXPORT QgsDockWidget : public QDockWidget
{
    Q_OBJECT

  public:

    /** Constructor for QgsDockWidget.
     * @param parent parent widget
     * @param flags window flags
     */
    explicit QgsDockWidget( QWidget* parent = nullptr, Qt::WindowFlags flags = 0 );

    /** Constructor for QgsDockWidget.
     * @param title dock title
     * @param parent parent widget
     * @param flags window flags
     */
    explicit QgsDockWidget( const QString &title, QWidget* parent = nullptr, Qt::WindowFlags flags = 0 );

  public slots:

    /** Sets the dock widget as visible to a user, ie both shown and raised to the front.
     * @param visible set to true to show the dock to the user, or false to hide the dock.
     * When setting a dock as user visible, the dock will be opened (if it is not already
     * opened) and raised to the front.
     * When setting as hidden, the following logic is used:
     * - hiding a dock which is open but not raised (ie hidden by another tab) will have no
     * effect, and the dock will still be opened and hidden by the other tab
     * - hiding a dock which is open and raised (ie, user visible) will cause the dock to
     * be closed
     * - hiding a dock which is closed has no effect and raises no signals
     * @see isUserVisible()
     */
    void setUserVisible( bool visible );

    /** Returns true if the dock is both opened and raised to the front (ie not hidden by
     * any other tabs.
     * @see setUserVisible()
     */
    bool isUserVisible() const;

  protected:

    virtual void closeEvent( QCloseEvent * ) override;
    virtual void showEvent( QShowEvent* event ) override;

  signals:

    /** Emitted when dock widget is closed.
     * @see closedStateChanged()
     * @see opened()
     */
    void closed();

    /** Emitted when dock widget is closed (or opened).
     * @param wasClosed will be true if dock widget was closed, or false if dock widget was opened
     * @see closed()
     * @see openedStateChanged()
     */
    void closedStateChanged( bool wasClosed );

    /** Emitted when dock widget is opened.
     * @see openedStateChanged()
     * @see closed()
     */
    void opened();

    /** Emitted when dock widget is opened (or closed).
     * @param wasOpened will be true if dock widget was opened, or false if dock widget was closed
     * @see closedStateChanged()
     * @see opened()
     */
    void openedStateChanged( bool wasOpened );

  private slots:

    void handleVisibilityChanged( bool visible );

  private:

    bool mVisibleAndActive;

};
#endif //QGSDOCKWIDGET_H
