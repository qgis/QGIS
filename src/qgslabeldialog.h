/***************************************************************************
                         qgslabeldialog.h  -  render vector labels
                             -------------------
    begin                : August 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELDIALOG_H
#define QGSLABELDIALOG_H

#include <qobject.h>
#include <qcolor.h>

class QWidget;
class QPoint;
class QTable;

class QgsLabel;

/** QgsLabelDialog is the dialog for label. */
class QgsLabelDialog: public QObject
{
    Q_OBJECT;

public:
    QgsLabelDialog( QgsLabel *label,  QWidget * parent = 0 );
    ~QgsLabelDialog();

    /* Attributes in order used in the table */
    enum Attribute {
	Text = 0,
	Family,
	Size,
	Bold,
	Italic,
	Underline,
	Color,
	XCoordinate,
	YCoordinate,
	XOffset,
	YOffset,
	Angle,
	Alignment,
	BufferSize,
	BufferColor,
	BufferBrush,
	BorderWidth,
	BorderColor,
	BorderStyle,
	AttributeCount
    };

public slots:
    /** applies the changes to the label class */
    void apply ( void );

    /** Table clicked */
    void tableClicked ( int row, int col );
	
    /** Change color */
    void changeColor ( void );

    /** Reset dialog to vector layer values */
    void reset ( void );

protected slots:

private:
    QTable   *mTable;
    QgsLabel *mLabel;
    QColor    mColor;
};

#endif
