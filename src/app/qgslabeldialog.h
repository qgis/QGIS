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
/* $Id$ */

#ifndef QGSLABELDIALOG_H
#define QGSLABELDIALOG_H

#include "ui_qgslabeldialogbase.h"

class QgsLabel;


/** QgsLabelDialog is the dialog for label. */
class QgsLabelDialog: public QWidget, private Ui::QgsLabelDialogBase
{
    Q_OBJECT

  public:
    QgsLabelDialog( QgsLabel *label,  QWidget * parent = 0 );
    ~QgsLabelDialog();
    int itemNoForField( QString theFieldName, QStringList theFieldList );

    /* Attributes in order used in the table */
    enum Attribute
    {
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
    void apply( void );

    /** Change font - reimplements method from base class*/
    void changeFont( );

    /** Change color - reimplements method from base class */
    void changeBufferColor( );
    void changeFontColor( );

    /** Initialise dialog to vector layer values */
    void init( void );

    void on_cboLabelField_activated( int ) {emit labelSourceSet(); }

    void on_spinFontSize_valueChanged( double d ) { mFont.setPointSizeF( d ); lblSample->setFont( mFont ); }

  signals:
    /* emitted when an item in label source list is chosen */
    void labelSourceSet();

  protected:

    /** return field index based on field's name, -1 if not found */
    int fieldIndexFromName( QString name );

  private:
    QgsLabel *mLabel;
    QColor    mFontColor;
    QColor    mBufferColor;
    QFont     mFont;
};

#endif
