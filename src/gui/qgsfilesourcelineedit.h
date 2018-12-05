/***************************************************************************
 qgssvgsourcelineedit.h
 ---------------------
 begin                : July 2018
 copyright            : (C) 2018 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSVGSOURCELINEEDIT_H
#define QGSSVGSOURCELINEEDIT_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QWidget>

class QLineEdit;
class QToolButton;

/**
 * \ingroup gui
 * \class QgsSvgSourceLineEdit
 * A line edit widget with toolbutton for setting an SVG image path.
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsSvgSourceLineEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString source READ source WRITE setSource NOTIFY sourceChanged )

  public:

    /**
     * Constructor for QgsSvgSourceLineEdit, with the specified \a parent widget.
     */
    QgsSvgSourceLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the current SVG source.
     * \see setSource()
     * \see sourceChanged()
     */
    QString source() const;

    /**
     * Sets a specific settings \a key to use when storing the last
     * used path for the SVG source.
     */
    void setLastPathSettingsKey( const QString &key );

  public slots:

    /**
     * Sets a new \a source to show in the widget.
     * \see source()
     * \see sourceChanged()
     */
    void setSource( const QString &source );

  signals:

    /**
     * Emitted whenever the SVG source is changed in the widget.
     */
    void sourceChanged( const QString &source );

  private slots:
    void selectFile();
    void selectUrl();
    void embedFile();
    void extractFile();
    void mFileLineEdit_textEdited( const QString &text );
    void mFileLineEdit_editingFinished();

  private:

    QLineEdit *mFileLineEdit = nullptr;
    QToolButton *mFileToolButton = nullptr;
    QString mLastPathKey;

    QString defaultPath() const;
    QString settingsKey() const;

};

#endif
