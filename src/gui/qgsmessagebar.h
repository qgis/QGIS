/***************************************************************************
                          qgsmessagebar.h  -  description
                             -------------------
    begin                : June 2012
    copyright            : (C) 2012 by Giuseppe Sucameli
    email                : sucameli at faunalia dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESSAGEBAR_H
#define QGSMESSAGEBAR_H

#include "qgsguiutils.h"
#include "qgis.h"

#include <QString>
#include <QFrame>
#include <QIcon>
#include <QColor>
#include <QList>
#include "qgis_gui.h"

class QWidget;
class QGridLayout;
class QMenu;
class QProgressBar;
class QToolButton;
class QLabel;
class QAction;
class QTimer;

class QgsMessageBarItem;

/**
 * \ingroup gui
 * \brief A bar for displaying non-blocking messages to the user.
 *
 * QgsMessageBar is a reusable widget which allows for providing feedback to users in
 * a non-intrusive way. Messages are shown in a horizontal bar widget, which is styled
 * automatically to reflect the severity ("message level") of the displayed message (e.g.
 * warning messages are styled in an orange color scheme, critical errors are shown in
 * red, etc).
 *
 * The message bar supports automatic stacking of multiple messages, so that
 * only the most recent message is shown to users. Users can then manually dismiss
 * individual messages to remove them from the stack, causing the next-most-recent
 * message to be shown. If no messages are available to show then the message bar
 * automatically hides.
 *
 * The class also supports pushing custom widgets to the notification stack via
 * the pushWidget() method.
 */
class GUI_EXPORT QgsMessageBar : public QFrame
{
    Q_OBJECT

  public:
    //! Constructor for QgsMessageBar
    QgsMessageBar( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Display a message \a item on the bar, after hiding the currently visible one
     * and putting it in a stack.
     *
     * The message bar will take ownership of \a item.
     */
    void pushItem( QgsMessageBarItem *item SIP_TRANSFER );

    /**
     * Display a \a widget as a message on the bar, after hiding the currently visible one
     * and putting it in a stack.
     *
     * \param widget message widget to display
     * \param level is Qgis::MessageLevel::Info, Warning, Critical or Success
     * \param duration timeout duration of message in seconds, 0 value indicates no timeout (i.e.
     * the message must be manually cleared by the user).
     */
    QgsMessageBarItem *pushWidget( QWidget *widget SIP_TRANSFER, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = 0 );

    /**
     * Remove the specified \a item from the bar, and display the next most recent one in the stack.
     * If no messages remain in the stack, then the bar will be hidden.
     *
     * \param item previously added item to remove.
     * \returns TRUE if \a item was removed, FALSE otherwise
     */
    bool popWidget( QgsMessageBarItem *item );

    /**
     * Creates message bar item widget containing a message \a text to be displayed on the bar.
     *
     * The caller takes ownership of the returned item.
     *
     * \note This is a low-level API call. Users are recommended to use the high-level pushMessage() API call
     * instead.
     */
    static QgsMessageBarItem *createMessage( const QString &text, QWidget *parent = nullptr ) SIP_FACTORY;

    /**
     * Creates message bar item widget containing a \a title and message \a text to be displayed on the bar.
     *
     * The caller takes ownership of the returned item.
     *
     * \note This is a low-level API call. Users are recommended to use the high-level pushMessage() API call
     * instead.
     */
    static QgsMessageBarItem *createMessage( const QString &title, const QString &text, QWidget *parent = nullptr ) SIP_FACTORY;

    /**
     * Creates message bar item widget containing a custom \a widget to be displayed on the bar.
     *
     * The caller takes ownership of the returned item.
     *
     * \note This is a low-level API call. Users are recommended to use the high-level pushWidget() API call
     * instead.
     */
    static QgsMessageBarItem *createMessage( QWidget *widget, QWidget *parent = nullptr ) SIP_FACTORY;

    /**
     * A convenience method for pushing a message with the specified \a text to the bar.
     *
     * The \a level argument specifies the desired message level (severity) of the message, which controls
     * how the message bar is styled.
     *
     * The optional \a duration argument can be used to specify the message timeout in seconds. If \a duration
     * is set to 0, then the message must be manually dismissed by the user. Since QGIS 3.18, a duration of -1 indicates that
     * the default timeout for the message \a level should be used.
     */
    void pushMessage( const QString &text, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = -1 );

    /**
     * A convenience method for pushing a message with the specified \a title and \a text to the bar.
     *
     * The \a level argument specifies the desired message level (severity) of the message, which controls
     * how the message bar is styled.
     *
     * The optional \a duration argument can be used to specify the message timeout in seconds. If \a duration
     * is set to 0, then the message must be manually dismissed by the user. Since QGIS 3.18, a duration of -1 indicates that
     * the default timeout for the message \a level should be used.
     */
    void pushMessage( const QString &title, const QString &text, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = -1 );

    /**
     * A convenience method for pushing a message with the specified \a title and \a text to the bar. Additional
     * message content specified via \a showMore will be shown when the user presses a "more" button.
     *
     * The \a level argument specifies the desired message level (severity) of the message, which controls
     * how the message bar is styled.
     *
     * The optional \a duration argument can be used to specify the message timeout in seconds. If \a duration
     * is set to 0, then the message must be manually dismissed by the user. Since QGIS 3.18, a duration of -1 indicates that
     * the default timeout for the message \a level should be used.
     */
    void pushMessage( const QString &title, const QString &text, const QString &showMore, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = -1 );

    /**
     * Returns the current visible item, or NULLPTR if no item is shown.
     */
    QgsMessageBarItem *currentItem();

    /**
     * Returns a list of all items currently visible or queued for the bar.
     *
     * \since QGIS 3.14
     */
    QList<QgsMessageBarItem *> items();

    /**
     * Returns the default timeout in seconds for timed messages of the specified \a level.
     * \since QGIS 3.18
     */
    static int defaultMessageTimeout( Qgis::MessageLevel level = Qgis::MessageLevel::NoLevel );

  signals:

    /**
     * Emitted whenever an \a item is added to the bar.
     */
    void widgetAdded( QgsMessageBarItem *item );

    /**
     * Emitted whenever an \a item was removed from the bar.
     */
    void widgetRemoved( QgsMessageBarItem *item );

  public slots:

    /**
     * Remove the currently displayed item from the bar and display the next item
     * in the stack. If no remaining items are present, the bar will be hidden.
     *
     * \returns TRUE if the widget was removed, FALSE otherwise
     */
    bool popWidget();

    /**
     * Removes all items from the bar.
     *
     * \returns TRUE if all items were removed, FALSE otherwise
     */
    bool clearWidgets();

    /**
     * Pushes a success \a message with default timeout to the message bar.
     *
     * \param title title string for message
     * \param message The message to be displayed
     *
     */
    void pushSuccess( const QString &title, const QString &message );

    /**
     * Pushes a information \a message with default timeout to the message bar.
     *
     * \param title title string for message
     * \param message The message to be displayed
     *
     */
    void pushInfo( const QString &title, const QString &message );

    /**
     * Pushes a warning \a message that must be manually dismissed by the user. Before QGIS 3.18 the default timeout was used.
     *
     * \param title title string for message
     * \param message The message to be displayed
     *
     */
    void pushWarning( const QString &title, const QString &message );

    /**
     * Pushes a critical warning \a message that must be manually dismissed by the user. Before QGIS 3.18 the default timeout was used.
     *
     * \param title title string for message
     * \param message The message to be displayed
     *
     */
    void pushCritical( const QString &title, const QString &message );

  protected:
    void mousePressEvent( QMouseEvent *e ) override;

  private:
    void popItem( QgsMessageBarItem *item );
    void showItem( QgsMessageBarItem *item );
    QgsMessageBarItem *mCurrentItem = nullptr;
    QList<QgsMessageBarItem *> mItems;
    QMenu *mCloseMenu = nullptr;
    QToolButton *mCloseBtn = nullptr;
    QGridLayout *mLayout = nullptr;
    QLabel *mItemCount = nullptr;
    QAction *mActionCloseAll = nullptr;
    QTimer *mCountdownTimer = nullptr;
    QProgressBar *mCountProgress = nullptr;
    QString mCountStyleSheet;
    Qgis::MessageLevel mPrevLevel = Qgis::MessageLevel::NoLevel;

    static constexpr int MAX_ITEMS = 100;

    void removeLowestPriorityOldestItem();

  private slots:
    //! updates count of items in widget list
    void updateItemCount();

    //! updates the countdown for widgets that have a timeout duration
    void updateCountdown();
    void resetCountdown();

    friend class TestQgsMessageBar;
};

#endif
