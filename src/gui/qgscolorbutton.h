/***************************************************************************
    qgscolorbutton.h - Color button
     --------------------------------------
    Date                 : 12-Dec-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOLORBUTTON_H
#define QGSCOLORBUTTON_H

#include <QColorDialog>
#include <QPushButton>
#include <QTemporaryFile>


/** \ingroup gui
 * \class QgsColorButton
 * A cross platform button subclass for selecting colors. Will open a color chooser dialog when clicked.
 * Offers live updates to button from color chooser dialog
 * @note inherited base class moved from QToolButton to QPushButton in QGIS 1.9
 */

class GUI_EXPORT QgsColorButton: public QPushButton
{
    Q_OBJECT

  public:
    /**
     * Construct a new color button.
     *
     * @param parent The parent QWidget for the dialog
     * @param cdt The title to show in the color chooser dialog
     * @param cdo Options for the color chooser dialog
     * @note changed in 1.9
     */
    QgsColorButton( QWidget *parent = 0, QString cdt = "", QColorDialog::ColorDialogOptions cdo = 0 );
    ~QgsColorButton();

    /**
     * Specify the current color. Will emit a colorChanged signal if the color is different to the previous.
     *
     * @param color the new color
     * @note added in 1.9
     */
    void setColor( const QColor &color );
    /**
     * Return the currently selected color.
     *
     * @return the currently selected color
     * @note added in 1.9
     */
    QColor color() const;

    /**
     * Specify the options for the color chooser dialog (e.g. alpha).
     *
     * @param cdo Options for the color chooser dialog
     * @note added in 1.9
     */
    void setColorDialogOptions( QColorDialog::ColorDialogOptions cdo );

    /**
     * Returns the options for the color chooser dialog.
     *
     * @return Options for the color chooser dialog
     * @note added in 1.9
     */
    QColorDialog::ColorDialogOptions colorDialogOptions();

    /**
     * Set the title, which the color chooser dialog will show.
     *
     * @param cdt Title for the color chooser dialog
     * @note added in 1.9
     */
    void setColorDialogTitle( QString cdt );

    /**
     * Returns the title, which the color chooser dialog shows.
     *
     * @return Title for the color chooser dialog
     * @note added in 1.9
     */
    QString colorDialogTitle();

    /**
     * Whether the button accepts live updates from QColorDialog.
     *
     * @note added in 1.9
     */
    bool acceptLiveUpdates() { return mAcceptLiveUpdates; }

    /**
     * Sets whether the button accepts live updates from QColorDialog.
     * Live updates may cause changes that are not undoable on QColorDialog cancel.
     *
     * @note added in 1.9
     */
    void setAcceptLiveUpdates( bool accept ) { mAcceptLiveUpdates = accept; }

  signals:
    /**
     * Is emitted, whenever a new color is accepted. The color is always valid.
     * In case the new color is the same, no signal is emitted, to avoid infinite loops.
     *
     * @param color New color
     * @note added in 1.9
     */
    void colorChanged( const QColor &color );

  protected:
    void changeEvent( QEvent* e );
#if 0 // causes too many cyclical updates, but may be needed on some platforms
    void paintEvent( QPaintEvent* e );
#endif
    void showEvent( QShowEvent* e );
    static const QPixmap& transpBkgrd();

  private:
    QString mColorDialogTitle;
    QColor mColor;
    QColorDialog::ColorDialogOptions mColorDialogOptions;
    bool mAcceptLiveUpdates;
    QTemporaryFile mTempPNG;

  private slots:
    void onButtonClicked();

    /**
     * Sets the background pixmap for the button based upon set color and transparency.
     *
     * @note added in 1.9
     */
    void setButtonBackground();

    /**
     * Sets color for button, if valid.
     *
     * @note added in 1.9
     */
    void setValidColor( const QColor& newColor );
};

#endif
