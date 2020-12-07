/***************************************************************************
    qgsoptionsdialoghighlightwidget.h
     -------------------------------
    Date                 : February 2018
    Copyright            : (C) 2018 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOPTIONSDIALOGHIGHLIGHTWIDGET_H
#define QGSOPTIONSDIALOGHIGHLIGHTWIDGET_H

#include <QObject>
#include <QPointer>
#include <QWidget>

#include "qgis_gui.h"
#include "qgis_sip.h"

/**
 * \ingroup gui
 * \class QgsOptionsDialogHighlightWidget
 * Container for a widget to be used to search text in the option dialog
 * If the widget type is handled, it is valid.
 * It can perform a text search in the widget and highlight it in case of success.
 * This uses stylesheets.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsOptionsDialogHighlightWidget : public QObject
{

    Q_OBJECT
  public:

    /**
     * create a highlight widget implementation for the proper widget type.
     * For instance a QgsOptionsDialogHighlightButton for button.
     * \return a QgsOptionsDialogHighlightWidget or NULLPTR if there is no implementation
     * for the given widget.
     */
    static QgsOptionsDialogHighlightWidget *createWidget( QWidget *widget ) SIP_FACTORY;

    /**
     * Returns if it valid: if the widget type is handled and if the widget is not still available
     */
    bool isValid() { return !mWidget.isNull(); }

    /**
     * search for a text pattern and highlight the widget if the text is found
     * \returns TRUE if the text pattern is found
     */
    bool searchHighlight( const QString &text );

    /**
     * Returns the widget
     */
    QWidget *widget() {return mWidget;}


    bool eventFilter( QObject *obj, QEvent *event ) override;


  protected:

    /**
     * Search for the \a text in the widget and return TRUE if it was found
     */
    virtual bool searchText( const QString &text ) = 0;

    /**
     * Highlight the \a text in the widget.
     * \return TRUE if the text could be highlighted.
     */
    virtual bool highlightText( const QString &text ) = 0;

    /**
     *  reset the style of the widgets to its original state
     */
    virtual void reset() = 0;

    /**
     * Constructor
     * \param widget the widget used to search text into
     */
    explicit QgsOptionsDialogHighlightWidget( QWidget *widget = nullptr );

    //! Pointer to the widget
    QPointer< QWidget > mWidget;

  private:
    QString mSearchText = QString();
    bool mChangedStyle = false;
    bool mInstalledFilter = false;
};

#endif // QGSOPTIONSDIALOGHIGHLIGHTWIDGET_H
