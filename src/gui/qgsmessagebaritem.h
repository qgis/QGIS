/***************************************************************************
                          qgsmessagebaritem.h  -  description
                             -------------------
    begin                : August 2013
    copyright            : (C) 2013 by Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESSAGEBARITEM_H
#define QGSMESSAGEBARITEM_H

#include "qgis.h"

#include <QWidget>
#include <QIcon>
#include <QHBoxLayout>
#include "qgis_gui.h"

class QTextBrowser;
class QLabel;
class QgsMessageBar;

/**
 * \ingroup gui
 * \class QgsMessageBarItem
 * \brief Represents an item shown within a QgsMessageBar widget.
 *
 * QgsMessageBarItem represents a single item (or message) which can be shown in a QgsMessageBar widget.
 */
class GUI_EXPORT QgsMessageBarItem : public QWidget
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsMessageBarItem, containing a message with the specified \a text to be displayed on the bar.
     *
     * The \a level argument specifies the desired message level (severity) of the message, which controls
     * how the message bar is styled when the item is displayed.
     *
     * The optional \a duration argument can be used to specify the message timeout in seconds. If \a duration
     * is set to 0, then the message must be manually dismissed by the user. Since QGIS 3.18, a duration of -1 indicates that
     * the default timeout for the message \a level should be used.
     */
    QgsMessageBarItem( const QString &text, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = 0, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsMessageBarItem, containing a \a title and message with the specified \a text to be displayed on the bar.
     *
     * The \a level argument specifies the desired message level (severity) of the message, which controls
     * how the message bar is styled when the item is displayed.
     *
     * The optional \a duration argument can be used to specify the message timeout in seconds. If \a duration
     * is set to 0, then the message must be manually dismissed by the user. Since QGIS 3.18, a duration of -1 indicates that
     * the default timeout for the message \a level should be used.
     */
    QgsMessageBarItem( const QString &title, const QString &text, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = 0, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsMessageBarItem, containing a \a title, message with the specified \a text, and a custom \a widget to be displayed on the bar.
     *
     * The \a level argument specifies the desired message level (severity) of the message, which controls
     * how the message bar is styled when the item is displayed.
     *
     * The optional \a duration argument can be used to specify the message timeout in seconds. If \a duration
     * is set to 0, then the message must be manually dismissed by the user. Since QGIS 3.18, a duration of -1 indicates that
     * the default timeout for the message \a level should be used.
     */
    QgsMessageBarItem( const QString &title, const QString &text, QWidget *widget, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = 0, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsMessageBarItem, containing a custom \a widget to be displayed on the bar.
     *
     * The \a level argument specifies the desired message level (severity) of the message, which controls
     * how the message bar is styled when the item is displayed.
     *
     * The optional \a duration argument can be used to specify the message timeout in seconds. If \a duration
     * is set to 0, then the message must be manually dismissed by the user. Since QGIS 3.18, a duration of -1 indicates that
     * the default timeout for the message \a level should be used.
     */
    QgsMessageBarItem( QWidget *widget, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = 0, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the message \a text to show in the item.
     *
     * \see text()
     */
    QgsMessageBarItem *setText( const QString &text );

    /**
     * Returns the text for the message.
     *
     * \see setText()
     */
    QString text() const;

    /**
     * Sets the \a title for in the item.
     *
     * \see title()
     */
    QgsMessageBarItem *setTitle( const QString &title );

    /**
     * Returns the title for the message.
     *
     * \see setTitle()
     */
    QString title() const;

    /**
     * Sets the message \a level for the item, which controls how the message bar is styled
     * when the item is displayed.
     *
     * \see level()
     */
    QgsMessageBarItem *setLevel( Qgis::MessageLevel level );

    /**
     * Returns the message level for the message.
     *
     * \see setLevel()
     */
    Qgis::MessageLevel level() const;

    /**
     * Sets a custom \a widget to show in the item.
     *
     * \see widget()
     */
    QgsMessageBarItem *setWidget( QWidget *widget );

    /**
     * Returns the widget for the message.
     *
     * \see setWidget()
     */
    QWidget *widget() const;

    /**
     * Sets the \a icon associated with the message.
     *
     * \see icon()
     */
    QgsMessageBarItem *setIcon( const QIcon &icon );

    /**
     * Returns the icon for the message.
     *
     * \see setIcon()
     */
    QIcon icon() const;

    /**
     * Sets the \a duration (in seconds) to show the message for. If \a duration
     * is 0 then the message will not automatically timeout and instead must be
     * manually dismissed by the user.
     *
     * \see duration()
     */
    QgsMessageBarItem *setDuration( int duration );

    /**
     * Returns the duration (in seconds) of the message.
     *
     * If the duration is 0 then the message will not automatically timeout and instead must be
     * manually dismissed by the user.
     *
     * \see setDuration()
     */
    int duration() const { return mDuration; }

    /**
     * Returns the styleSheet which should be used to style a QgsMessageBar object when
     * this item is displayed.
     */
    QString getStyleSheet() { return mStyleSheet; }

  public slots:

    /**
     * Dismisses the item, removing it from the message bar and deleting
     * it. Calling this on items which have not been added to a message bar
     * has no effect.
     *
     * \since QGIS 3.4
     */
    void dismiss();

  signals:

    /**
     * Emitted when the item's message level has changed and the message bar style
     * will need to be updated as a result.
     */
    void styleChanged( const QString &styleSheet );

  private slots:

    void urlClicked( const QUrl &url );

  private:
    void writeContent();

    QString mTitle;
    QString mText;
    Qgis::MessageLevel mLevel;
    int mDuration;
    QWidget *mWidget = nullptr;
    QIcon mUserIcon;
    QHBoxLayout *mLayout = nullptr;
    QLabel *mLblIcon = nullptr;
    QString mStyleSheet;
    QTextBrowser *mTextBrowser = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    friend class QgsMessageBar;
};

#endif // QGSMESSAGEBARITEM_H
