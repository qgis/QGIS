
/** \ingroup gui
 * \class QgsColorButtonV2
 * A cross platform button subclass for selecting colors. Will open a color chooser dialog when clicked.
 * Offers live updates to button from color chooser dialog. An attached drop down menu allows for copying
 * and pasting colors, picking colors from the screen, and selecting colors from color swatch grids.
 * \note Added in version 2.5
 */

class QgsColorButtonV2: QToolButton
{
%TypeHeaderCode
#include <qgscolorbuttonv2.h>
%End

  public:

    /*! Specifies the behaviour when the button is clicked
     */
    enum Behaviour
    {
      ShowDialog, /*!< show a color picker dialog when clicked */
      SignalOnly /*!< emit colorClicked signal only, no dialog */
    };

    /**Construct a new color button.
     * @param parent The parent QWidget for the dialog
     * @param cdt The title to show in the color chooser dialog
     * @param registry a color scheme registry for color swatch grids to show in the drop down menu. If not
     * specified, the button will use the global color scheme registry
     */
    QgsColorButtonV2( QWidget *parent /TransferThis/ = 0, QString cdt = "", QgsColorSchemeRegistry* registry = 0 );

    virtual ~QgsColorButtonV2();

    virtual QSize sizeHint() const;

    /**Return the currently selected color.
     * @returns currently selected color
     * @see setColor
     */
    QColor color() const;

    /**Sets whether alpha modification (transparency) is permitted
     * for the color. Defaults to false.
     * @param allowAlpha set to true to allow alpha modification
     * @see allowAlpha
     */
    void setAllowAlpha( const bool allowAlpha );

    /**Returns whether alpha modification (transparency) is permitted
     * for the color.
     * @returns true if alpha modification is allowed
     * @see setAllowAlpha
     */
    bool allowAlpha() const;

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
    bool acceptLiveUpdates() const;

    /**Sets whether the button accepts live updates from QColorDialog. Live updates may cause changes
     * that are not undoable on QColorDialog cancel.
     * @param accept set to true to enable live updates
     * @see acceptLiveUpdates
     */
    void setAcceptLiveUpdates( const bool accept );

    /**Sets whether the drop down menu should be shown for the button. The default behaviour is to
     * show the menu.
     * @param showMenu set to false to hide the drop down menu
     * @see showMenu
     */
    void setShowMenu( const bool showMenu );

    /**Returns whether the drop down menu is shown for the button.
     * @returns true if drop down menu is shown
     * @see setShowMenu
     */
    bool showMenu() const;

    /**Sets the behaviour for when the button is clicked. The default behaviour is to show
     * a color picker dialog.
     * @param behaviour behaviour when button is clicked
     * @see behaviour
     */
    void setBehaviour( const Behaviour behaviour );

    /**Returns the behaviour for when the button is clicked.
     * @returns behaviour when button is clicked
     * @see setBehaviour
     */
    Behaviour behaviour() const;

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
    QColor defaultColor() const;

    /**Sets whether the "no color" option should be shown in the button's drop down menu. If selected,
     * the "no color" option sets the color button's color to a totally transparent color.
     * @param showNoColorOption set to true to show the no color option. This is disabled by default.
     * @see showNoColor
     * @see setNoColorString
     * @note The "no color" option is only shown if the color button is set to show an alpha channel in the color
     * dialog (see setColorDialogOptions)
     */
    void setShowNoColor( const bool showNoColorOption );

    /**Returns whether the "no color" option is shown in the button's drop down menu. If selected,
     * the "no color" option sets the color button's color to a totally transparent color.
     * @returns true if the no color option is shown.
     * @see setShowNoColor
     * @see noColorString
     * @note The "no color" option is only shown if the color button is set to show an alpha channel in the color
     * dialog (see setColorDialogOptions)
     */
    bool showNoColor() const;

    /**Sets the string to use for the "no color" option in the button's drop down menu.
     * @param noColorString string to use for the "no color" menu option
     * @see noColorString
     * @see setShowNoColor
     * @note The "no color" option is only shown if the color button is set to show an alpha channel in the color
     * dialog (see setColorDialogOptions)
     */
    void setNoColorString( const QString noColorString );

    /**Returns the string used for the "no color" option in the button's drop down menu.
     * @returns string used for the "no color" menu option
     * @see setNoColorString
     * @see showNoColor
     * @note The "no color" option is only shown if the color button is set to show an alpha channel in the color
     * dialog (see setColorDialogOptions)
     */
    QString noColorString() const;

    /**Sets the context string for the color button. The context string is passed to all color swatch
     * grids shown in the button's drop down menu, to allow them to customise their display colors
     * based on the context.
     * @param context context string for the color button's color swatch grids
     * @see context
     */
    void setContext( const QString context );

    /**Returns the context string for the color button. The context string is passed to all color swatch
     * grids shown in the button's drop down menu, to allow them to customise their display colors
     * based on the context.
     * @returns context string for the color button's color swatch grids
     * @see setContext
     */
    QString context() const;

    /**Sets the color scheme registry for the button, which controls the color swatch grids
     * that are shown in the button's drop down menu.
     * @param registry color scheme registry for the button. Set to 0 to hide all color
     * swatch grids from the button's drop down menu.
     * @see colorSchemeRegistry
     */
    void setColorSchemeRegistry( QgsColorSchemeRegistry* registry );

    /**Returns the color scheme registry for the button, which controls the color swatch grids
     * that are shown in the button's drop down menu.
     * @returns color scheme registry for the button. If returned value is 0 then all color
     * swatch grids are hidden from the button's drop down menu.
     * @see setColorSchemeRegistry
     */
    QgsColorSchemeRegistry* colorSchemeRegistry();

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
    void setButtonBackground( const QColor &color = QColor() );

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

    /**Emitted when the button is clicked, if the button's behaviour is set to SignalOnly
     * @param color button color
     * @see setBehaviour
     * @see behaviour
     */
    void colorClicked( const QColor &color );

  protected:

    void changeEvent( QEvent* e );
    void showEvent( QShowEvent* e );
    void resizeEvent( QResizeEvent *event );

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
    void dragEnterEvent( QDragEnterEvent * e );

    /**
     * Reimplemented to reset button appearance after drag leave
     */
    void dragLeaveEvent( QDragLeaveEvent *e );

    /**
     * Reimplemented to accept dropped colors
     */
    void dropEvent( QDropEvent *e );

};
