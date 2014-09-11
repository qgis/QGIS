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
#ifndef QGSCOLORBUTTONV2_H
#define QGSCOLORBUTTONV2_H

#include <QColorDialog>
#include <QToolButton>
#include <QTemporaryFile>
#include "qgscolorscheme.h"

class QMimeData;
class QgsColorSchemeRegistry;

/** \ingroup gui
 * \class QgsColorButtonV2
 * A cross platform button subclass for selecting colors. Will open a color chooser dialog when clicked.
 * Offers live updates to button from color chooser dialog. An attached drop down menu allows for copying
 * and pasting colors, picking colors from the screen, and selecting colors from color swatch grids.
 * \note Added in version 2.5
 */

class GUI_EXPORT QgsColorButtonV2: public QToolButton
{
    Q_OBJECT
    Q_PROPERTY( QString colorDialogTitle READ colorDialogTitle WRITE setColorDialogTitle )
    Q_PROPERTY( bool acceptLiveUpdates READ acceptLiveUpdates WRITE setAcceptLiveUpdates )
    Q_PROPERTY( QColor color READ color WRITE setColor )
    Q_FLAGS( QColorDialog::ColorDialogOptions )
    Q_PROPERTY( QColorDialog::ColorDialogOptions colorDialogOptions READ colorDialogOptions WRITE setColorDialogOptions )

  public:

    /**Construct a new color button.
     * @param parent The parent QWidget for the dialog
     * @param cdt The title to show in the color chooser dialog
     * @param cdo Options for the color chooser dialog
     * @param registry a color scheme registry for color swatch grids to show in the drop down menu. If not
     * specified, the button will use the global color scheme registry
     */
    QgsColorButtonV2( QWidget *parent = 0, QString cdt = "", QColorDialog::ColorDialogOptions cdo = 0, QgsColorSchemeRegistry* registry = 0 );

    virtual ~QgsColorButtonV2();

    virtual QSize sizeHint() const;

    /**Return the currently selected color.
     * @returns currently selected color
     * @see setColor
     */
    QColor color() const;

    /**Set options for the color chooser dialog (e.g. whether alpha channel is shown).
     * @param cdo Options for the color chooser dialog. For instance, to allow the user to choose an alpha
     * value in the dialog, call setColorDialogOptions( QColorDialog::ShowAlphaChannel )
     * @see colorDialogOptions
     */
    void setColorDialogOptions( const QColorDialog::ColorDialogOptions cdo );

    /**Returns the options for the color chooser dialog.
     * @returns Options for the color chooser dialog
     * @see setColorDialogOptions
     */
    QColorDialog::ColorDialogOptions colorDialogOptions() const;

    /**Set the title for the color chooser dialog window.
     * @param title Title for the color chooser dialog
     * @see colorDialogTitle
     */
    void setColorDialogTitle( const QString title );

    /**Returns the title for the color chooser dialog window.
     * @returns title for the color chooser dialog
     * @see setColorDialogTitle
     */
    QString colorDialogTitle() const;

    /**Returns whether the button accepts live updates from QColorDialog.
     * @returns true if the button will be accepted immediately when the dialog's color changes
     * @see setAcceptLiveUpdates
     */
    bool acceptLiveUpdates() const { return mAcceptLiveUpdates; }

    /**Sets whether the button accepts live updates from QColorDialog. Live updates may cause changes
     * that are not undoable on QColorDialog cancel.
     * @param accept set to true to enable live updates
     * @see acceptLiveUpdates
     */
    void setAcceptLiveUpdates( const bool accept ) { mAcceptLiveUpdates = accept; }

    /**Sets the default color for the button, which is shown in the button's drop down menu for the
     * "default color" option.
     * @param color default color for the button. Set to an invalid QColor to disable the default color
     * option.
     * @see defaultColor
     */
    void setDefaultColor( const QColor color );

    /**Returns the default color for the button, which is shown in the button's drop down menu for the
     * "default color" option.
     * @returns default color for the button. Returns an invalid QColor if the default color
     * option is disabled.
     * @see setDefaultColor
     */
    QColor defaultColor() const { return mDefaultColor; }

    /**Sets whether the "no color" option should be shown in the button's drop down menu. If selected,
     * the "no color" option sets the color button's color to a totally transparent color.
     * @param showNoColorOption set to true to show the no color option. This is disabled by default.
     * @see showNoColor
     * @see setNoColorString
     * @note The "no color" option is only shown if the color button is set to show an alpha channel in the color
     * dialog (see setColorDialogOptions)
     */
    void setShowNoColor( const bool showNoColorOption ) { mShowNoColorOption = showNoColorOption; }

    /**Returns whether the "no color" option is shown in the button's drop down menu. If selected,
     * the "no color" option sets the color button's color to a totally transparent color.
     * @returns true if the no color option is shown.
     * @see setShowNoColor
     * @see noColorString
     * @note The "no color" option is only shown if the color button is set to show an alpha channel in the color
     * dialog (see setColorDialogOptions)
     */
    bool showNoColor() const { return mShowNoColorOption; }

    /**Sets the string to use for the "no color" option in the button's drop down menu.
     * @param noColorString string to use for the "no color" menu option
     * @see noColorString
     * @see setShowNoColor
     * @note The "no color" option is only shown if the color button is set to show an alpha channel in the color
     * dialog (see setColorDialogOptions)
     */
    void setNoColorString( const QString noColorString ) { mNoColorString = noColorString; }

    /**Returns the string used for the "no color" option in the button's drop down menu.
     * @returns string used for the "no color" menu option
     * @see setNoColorString
     * @see showNoColor
     * @note The "no color" option is only shown if the color button is set to show an alpha channel in the color
     * dialog (see setColorDialogOptions)
     */
    QString noColorString() const { return mNoColorString; }

    /**Sets the context string for the color button. The context string is passed to all color swatch
     * grids shown in the button's drop down menu, to allow them to customise their display colors
     * based on the context.
     * @param context context string for the color button's color swatch grids
     * @see context
     */
    void setContext( const QString context ) { mContext = context; }

    /**Returns the context string for the color button. The context string is passed to all color swatch
     * grids shown in the button's drop down menu, to allow them to customise their display colors
     * based on the context.
     * @returns context string for the color button's color swatch grids
     * @see setContext
     */
    QString context() const { return mContext; }

    /**Sets the color scheme registry for the button, which controls the color swatch grids
     * that are shown in the button's drop down menu.
     * @param registry color scheme registry for the button. Set to 0 to hide all color
     * swatch grids from the button's drop down menu.
     * @see colorSchemeRegistry
     */
    void setColorSchemeRegistry( QgsColorSchemeRegistry* registry ) { mColorSchemeRegistry = registry; }

    /**Returns the color scheme registry for the button, which controls the color swatch grids
     * that are shown in the button's drop down menu.
     * @returns color scheme registry for the button. If returned value is 0 then all color
     * swatch grids are hidden from the button's drop down menu.
     * @see setColorSchemeRegistry
     */
    QgsColorSchemeRegistry* colorSchemeRegistry() { return mColorSchemeRegistry; }

  public slots:

    /**Sets the current color for the button. Will emit a colorChanged signal if the color is different
     * to the previous color.
     * @param color new color for the button
     * @see color
     */
    void setColor( const QColor &color );

    /**Sets the background pixmap for the button based upon color and transparency.
     * Call directly to update background after adding/removing QColorDialog::ShowAlphaChannel option
     * but the color has not changed, i.e. setColor() wouldn't update button and
     * you want the button to retain the set color's alpha component regardless
     * @param color Color for button background. If no color is specified, the button's current
     * color will be used
     */
    void setButtonBackground( const QColor color = QColor() );

    /**Copies the current color to the clipboard
     * @see pasteColor
     */
    void copyColor();

    /**Pastes a color from the clipboard to the color button. If clipboard does not contain a valid
     * color or string representation of a color, then no change is applied.
     * @see copyColor
     */
    void pasteColor();

    /**Activates the color picker tool, which allows for sampling a color from anywhere on the screen
     */
    void activatePicker();

    /**Sets color to a totally transparent color.
     * @note If the color button is not set to show an alpha channel in the color
     * dialog (see setColorDialogOptions) then the color will not be changed.
     */
    void setToNoColor();

    /**Sets color to the button's default color, if set.
     * @see setDefaultColor
     * @see defaultColor
     */
    void setToDefaultColor();

  signals:

    /**Is emitted whenever a new color is set for the button. The color is always valid.
     * In case the new color is the same no signal is emitted, to avoid infinite loops.
     * @param color New color
     */
    void colorChanged( const QColor &color );

  protected:

    void changeEvent( QEvent* e );
    void showEvent( QShowEvent* e );

    /**Returns a checkboard pattern pixmap for use as a background to transparent colors
     */
    static const QPixmap& transparentBackground();

    /**
     * Reimplemented to detect right mouse button clicks on the color button and allow dragging colors
     */
    void mousePressEvent( QMouseEvent* e );

    /**
     * Reimplemented to allow dragging colors from button
     */
    void mouseMoveEvent( QMouseEvent *e );

    /**
     * Reimplemented to allow color picking
     */
    void mouseReleaseEvent( QMouseEvent *e );

    /**
     * Reimplemented to allow cancelling color pick via keypress, and sample via space bar press
     */
    void keyPressEvent( QKeyEvent *e );

    /**
     * Reimplemented to accept dragged colors
     */
    void dragEnterEvent( QDragEnterEvent * e ) ;

    /**
     * Reimplemented to reset button appearance after drag leave
     */
    void dragLeaveEvent( QDragLeaveEvent *e ) ;

    /**
     * Reimplemented to accept dropped colors
     */
    void dropEvent( QDropEvent *e );

  private:
    QString mColorDialogTitle;
    QColor mColor;

    QgsColorSchemeRegistry* mColorSchemeRegistry;

    QColor mDefaultColor;
    QString mContext;
    QColorDialog::ColorDialogOptions mColorDialogOptions;
    bool mAcceptLiveUpdates;
    bool mColorSet;

    bool mShowNoColorOption;
    QString mNoColorString;

    QPoint mDragStartPosition;
    bool mPickingColor;

    QMenu* mMenu;

    QSize mIconSize;

    /**Attempts to parse mimeData as a color, either via the mime data's color data or by
     * parsing a textual representation of a color.
     * @returns true if mime data could be intrepreted as a color
     * @param mimeData mime data
     * @param resultColor QColor to store evaluated color
     * @see createColorMimeData
     */
    bool colorFromMimeData( const QMimeData *mimeData, QColor &resultColor );

    /**Ends a color picking operation
     * @param eventPos global position of pixel to sample color from
     * @param sampleColor set to true to actually sample the color, false to just cancel
     * the color picking operation
     */
    void stopPicking( QPointF eventPos, bool sampleColor = true );

    /**Adds a color to the recent colors list
     * @param color to add to recent colors list
     */
    void addRecentColor( const QColor color );

    /**Create a color icon for display in the drop down menu
     * @param color for icon
     * @param showChecks set to true to display a checkboard pattern behind
     * transparent colors
     */
    QPixmap createMenuIcon( const QColor color , const bool showChecks = true );

  private slots:

    void showColorDialog();

    /**Sets color for button, if valid.
     */
    void setValidColor( const QColor& newColor );

    /**Creates the drop down menu entries
     */
    void prepareMenu();
};

#endif
