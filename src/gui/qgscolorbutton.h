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
#include <QToolButton>
#include <QPushButton>

/** \ingroup gui
 * A cross platform button subclass for selecting colors. Will open a color chooser dialog when clicked.
 */
class GUI_EXPORT QgsColorButton: public QToolButton
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
    QgsColorButton( QWidget *parent = 0, QString cdt = tr( "Select Color" ), QColorDialog::ColorDialogOptions cdo = 0 );
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

  protected:
    void paintEvent( QPaintEvent *e );

  public slots:
    void onButtonClicked();

  signals:
    /**
     * Is emitted, whenever a new color is accepted. The color is always valid.
     * In case the new color is the same, no signal is emitted, to avoid infinite loops.
     *
     * @param color New color
     * @note added in 1.9
     */
    void colorChanged( const QColor &color );

  private:
    QString mColorDialogTitle;
    QColor mColor;
    QColorDialog::ColorDialogOptions mColorDialogOptions;
};


class GUI_EXPORT QgsColorButtonV2 : public QPushButton
{
  public:
    QgsColorButtonV2( QWidget* parent = 0 );
    QgsColorButtonV2( QString text, QWidget* parent = 0 );

    void setColor( const QColor &color );
    QColor color() const { return mColor; }

  private:
    QColor mColor;
};

#endif
