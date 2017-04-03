/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CHARACTERWIDGET_H
#define CHARACTERWIDGET_H

#include <QFont>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QWidget>
#include "qgis.h"
#include "qgis_gui.h"

class QMouseEvent;
class QPaintEvent;

/**
 * \ingroup gui
 *
 * A widget for displaying characters available in a preset font, and allowing
 * users to select an individual character.
 */
class GUI_EXPORT CharacterWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QChar character READ character WRITE setCharacter NOTIFY characterSelected )
    Q_PROPERTY( int columns READ columns WRITE setColumns )
    Q_PROPERTY( QFont font READ font WRITE setFont )

  public:

    /**
     * Constructor for CharacterWidget.
     */
    CharacterWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QSize sizeHint() const override;

    /**
     * Returns the number of columns of characters shown in the widget.
     */
    int columns() const { return mColumns; }

    /**
     * Returns the size (in pixels) of the square used to render each character preview.
     */
    int squareSize() const { return mSquareSize; }

    /**
     * Returns the currently selected character in the widget.
     * \see setCharacter()
     * \since QGIS 3.0
     */
    QChar character() const { return QChar( mLastKey ); }

    /**
     * Returns the font shown in the widget
     * \see setFont()
     * \since QGIS 3.0
     */
    QFont font() const { return mDisplayFont; }

  public slots:

    /**
     * Sets the \a font to show in the widget.
     * \see font()
     * \since QGIS 3.0
     */
    void setFont( const QFont &font );

    /**
     * Sets the font size (in points) to render in the widget.
     * \since QGIS 3.0
     */
    void setFontSize( double fontSize );

    /**
     * Sets the font style to show in the widget.
     * \since QGIS 3.0
     */
    void setFontStyle( const QString &fontStyle );

    void updateFontMerging( bool enable );

    /**
     * Sets the number of columns of characters to show in the widget.
     * \since QGIS 3.0
     */
    void setColumns( int columns );

    /**
     * Sets the currently selected \a character in the widget.
     * \see character()
     * \see characterSelected()
     */
    void setCharacter( QChar character );

  signals:

    /**
     * Emitted when a character is selected in the widget.
     */
    void characterSelected( QChar character );

  protected:
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void paintEvent( QPaintEvent *event ) override;

  private:
    QFont mDisplayFont;
    int mColumns = 16;
    int mLastKey = -1;
    int mSquareSize = 24;
};

#endif
