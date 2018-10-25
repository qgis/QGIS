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
#ifndef qgsmessagebaritem_H
#define qgsmessagebaritem_H

#include "qgsmessagebaritem.h"
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
 */
class GUI_EXPORT QgsMessageBarItem : public QWidget
{
    Q_OBJECT
  public:
    //! make out a widget containing a message to be displayed on the bar
    QgsMessageBarItem( const QString &text, Qgis::MessageLevel level = Qgis::Info, int duration = 0, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! make out a widget containing title and message to be displayed on the bar
    QgsMessageBarItem( const QString &title, const QString &text, Qgis::MessageLevel level = Qgis::Info, int duration = 0, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! make out a widget containing title, message and widget to be displayed on the bar
    QgsMessageBarItem( const QString &title, const QString &text, QWidget *widget, Qgis::MessageLevel level = Qgis::Info, int duration = 0, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! make out a widget containing a widget to be displayed on the bar
    QgsMessageBarItem( QWidget *widget, Qgis::MessageLevel level = Qgis::Info, int duration = 0, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QgsMessageBarItem *setText( const QString &text );

    /**
     * Returns the text for the message.
     */
    QString text() const;

    QgsMessageBarItem *setTitle( const QString &title );

    /**
     * Returns the title for the message.
     */
    QString title() const;

    QgsMessageBarItem *setLevel( Qgis::MessageLevel level );

    /**
     * Returns the message level for the message.
     */
    Qgis::MessageLevel level() const;

    QgsMessageBarItem *setWidget( QWidget *widget );

    /**
     * Returns the widget for the message.
     */
    QWidget *widget() const;

    QgsMessageBarItem *setIcon( const QIcon &icon );

    /**
     * Returns the icon for the message.
     */
    QIcon icon() const;

    QgsMessageBarItem *setDuration( int duration );

    //! returns the duration in second of the message
    int duration() const { return mDuration; }

    //! returns the styleSheet
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
    //! emitted when the message level has changed
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

#endif // qgsmessagebaritem_H
