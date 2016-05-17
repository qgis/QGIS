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

class QMimeData;

/** \ingroup gui
 * \class QgsColorButton
 * A cross platform button subclass for selecting colors. Will open a color chooser dialog when clicked.
 * Offers live updates to button from color chooser dialog
 * @note inherited base class moved from QToolButton to QPushButton in QGIS 1.9
 */

class GUI_EXPORT QgsColorButton: public QPushButton
{
    Q_OBJECT
    Q_PROPERTY( QString colorDialogTitle READ colorDialogTitle WRITE setColorDialogTitle )
    Q_PROPERTY( bool acceptLiveUpdates READ acceptLiveUpdates WRITE setAcceptLiveUpdates )
    Q_PROPERTY( QColor color READ color WRITE setColor )
    Q_FLAGS( QColorDialog::ColorDialogOptions )
    Q_PROPERTY( QColorDialog::ColorDialogOptions colorDialogOptions READ colorDialogOptions WRITE setColorDialogOptions )

  public:
    /**
     * Construct a new color button.
     *
     * @param parent The parent QWidget for the dialog
     * @param cdt The title to show in the color chooser dialog
     * @param cdo Options for the color chooser dialog
     * @note changed in 1.9
     */
    QgsColorButton( QWidget *parent = nullptr, const QString& cdt = "", const QColorDialog::ColorDialogOptions& cdo = nullptr );
    ~QgsColorButton();

    /**
     * Specify the current color. Will emit a colorChanged signal if the color is different to the previous.
     *
     * @param color the new color
     */
    void setColor( const QColor &color );
    /**
     * Return the currently selected color.
     *
     * @return the currently selected color
     */
    QColor color() const;

    /**
     * Specify the options for the color chooser dialog (e.g. alpha).
     *
     * @param cdo Options for the color chooser dialog
     */
    void setColorDialogOptions( const QColorDialog::ColorDialogOptions& cdo );

    /**
     * Returns the options for the color chooser dialog.
     *
     * @return Options for the color chooser dialog
     */
    QColorDialog::ColorDialogOptions colorDialogOptions();

    /**
     * Set the title, which the color chooser dialog will show.
     *
     * @param cdt Title for the color chooser dialog
     */
    void setColorDialogTitle( const QString& cdt );

    /**
     * Returns the title, which the color chooser dialog shows.
     *
     * @return Title for the color chooser dialog
     */
    QString colorDialogTitle();

    /**
     * Whether the button accepts live updates from QColorDialog.
     */
    bool acceptLiveUpdates() { return mAcceptLiveUpdates; }

    /**
     * Sets whether the button accepts live updates from QColorDialog.
     * Live updates may cause changes that are not undoable on QColorDialog cancel.
     */
    void setAcceptLiveUpdates( bool accept ) { mAcceptLiveUpdates = accept; }

  public slots:
    /**
     * Sets the background pixmap for the button based upon color and transparency.
     * Call directly to update background after adding/removing QColorDialog::ShowAlphaChannel option
     * but the color has not changed, i.e. setColor() wouldn't update button and
     * you want the button to retain the set color's alpha component regardless
     * @param color Color for button background
     */
    void setButtonBackground( QColor color = QColor() );

  signals:
    /**
     * Is emitted, whenever a new color is accepted. The color is always valid.
     * In case the new color is the same, no signal is emitted, to avoid infinite loops.
     *
     * @param color New color
     */
    void colorChanged( const QColor &color );

  protected:
    void changeEvent( QEvent* e ) override;
    void showEvent( QShowEvent* e ) override;
    static const QPixmap& transpBkgrd();

    /**
     * Reimplemented to detect right mouse button clicks on the color button and allow dragging colors
     */
    void mousePressEvent( QMouseEvent* e ) override;

    /**
     * Reimplemented to allow dragging colors from button
     */
    void mouseMoveEvent( QMouseEvent *e ) override;

    /**
     * Reimplemented to allow color picking
     */
    void mouseReleaseEvent( QMouseEvent *e ) override;

    /**
     * Reimplemented to allow cancelling color pick via keypress, and sample via space bar press
     */
    void keyPressEvent( QKeyEvent *e ) override;

    /**
     * Reimplemented to accept dragged colors
     */
    void dragEnterEvent( QDragEnterEvent * e ) override;

    /**
     * Reimplemented to accept dropped colors
     */
    void dropEvent( QDropEvent *e ) override;

  private:
    QString mColorDialogTitle;
    QColor mColor;
    QColorDialog::ColorDialogOptions mColorDialogOptions;
    bool mAcceptLiveUpdates;
    QTemporaryFile mTempPNG;
    bool mColorSet; // added in QGIS 2.1

    QPoint mDragStartPosition;
    bool mPickingColor;

    /**
     * Shows the color button context menu and handles copying and pasting color values.
     */
    void showContextMenu( QMouseEvent* event );

    /**
     * Creates mime data from the current color. Sets both the mime data's color data, and the
     * mime data's text with the color's hex code.
     * @note added in 2.3
     * @see colorFromMimeData
     */
    QMimeData* createColorMimeData() const;

    /**
     * Attempts to parse mimeData as a color, either via the mime data's color data or by
     * parsing a textual representation of a color.
     * @returns true if mime data could be intrepreted as a color
     * @param mimeData mime data
     * @param resultColor QColor to store evaluated color
     * @note added in 2.3
     * @see createColorMimeData
     */
    bool colorFromMimeData( const QMimeData *mimeData, QColor &resultColor );

#ifdef Q_OS_WIN
    /**
     * Expands a shortened Windows path to its full path name.
     * @returns full path name.
     * @param path a (possibly) shortened Windows path
     * @note added in 2.3
     */
    QString fullPath( const QString &path );
#endif

    /**
     * Ends a color picking operation
     * @param eventPos global position of pixel to sample color from
     * @param sampleColor set to true to actually sample the color, false to just cancel
     * the color picking operation
     * @note added in 2.5
     */
    void stopPicking( QPointF eventPos, bool sampleColor = true );

  private slots:
    void onButtonClicked();

    /**
     * Sets color for button, if valid.
     */
    void setValidColor( const QColor& newColor );
};

#endif
