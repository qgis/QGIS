
/** \ingroup gui
 * \class QgsColorRampButton
 * A cross platform button subclass for selecting color ramps. Will open color ramp dialogs when clicked.
 * Offers live updates to button from color ramp dialog. An attached drop down menu allows for access to
 * saved color ramps, as well as option to invert the current color ramp and create new ramps.
 * \note Added in version 3.0
 */

class QgsColorRampButton : QToolButton
{
%TypeHeaderCode
#include <qgscolorrampbutton.h>
%End

  public:

    /** Specifies the behaviour when the button is clicked
     */
    enum Behaviour
    {
      ShowDialog, /*!< show a color picker dialog when clicked */
      SignalOnly /*!< emit colorClicked signal only, no dialog */
    };

    /** Construct a new color ramp button.
     * @param parent The parent QWidget for the dialog
     * @param dialogTitle The title to show in the color ramp dialog
     */
    QgsColorRampButton( QWidget *parent = nullptr, const QString& dialogTitle = QString() );

    virtual ~QgsColorRampButton();

    virtual QSize sizeHint() const;

    /** Return the current color ramp.
     * @returns currently selected color
     * @see setColor
     */
    QgsColorRamp* colorRamp() const;

    /** Set the title for the color ramp dialog window.
     * @param title Title for the color ramp dialog
     * @see colorRampDialogTitle
     */
    void setColorRampDialogTitle( const QString& title );

    /** Returns the title for the color ramp dialog window.
     * @returns title for the color ramp dialog
     * @see setColorRampDialogTitle
     */
    QString colorRampDialogTitle() const;

    /** Returns whether the button accepts live updates from QgsColorRampDialog.
     * @returns true if the button will be accepted immediately when the dialog's color ramp changes
     * @see setAcceptLiveUpdates
     */
    bool acceptLiveUpdates() const;

    /** Sets whether the button accepts live updates from QgsColorRampDialog. Live updates may cause changes
     * that are not undoable on QColorRampDialog cancel.
     * @param accept set to true to enable live updates
     * @see acceptLiveUpdates
     */
    void setAcceptLiveUpdates( const bool accept );

    /** Sets whether the drop down menu should be shown for the button. The default behaviour is to
     * show the menu.
     * @param showMenu set to false to hide the drop down menu
     * @see showMenu
     */
    void setShowMenu( const bool showMenu );

    /** Returns whether the drop down menu is shown for the button.
     * @returns true if drop down menu is shown
     * @see setShowMenu
     */
    bool showMenu() const;

    /** Sets the behaviour for when the button is clicked. The default behaviour is to show
     * a color ramp dialog.
     * @param behaviour behaviour when button is clicked
     * @see behaviour
     */
    void setBehaviour( const Behaviour behaviour );

    /** Returns the behaviour for when the button is clicked.
     * @returns behaviour when button is clicked
     * @see setBehaviour
     */
    Behaviour behaviour() const;

    /** Sets the default color ramp for the button, which is shown in the button's drop down menu for the
     * "default color ramp" option.
     * @param colorramp default color ramp for the button. Set to a null pointer to disable the default color
     * ramp option.
     * @see defaultColorRamp
     */
    void setDefaultColorRamp( QgsColorRamp* colorramp );

    /** Returns the default color ramp for the button, which is shown in the button's drop down menu for the
     * "default color ramp" option.
     * @returns default color ramp for the button. Returns a null pointer if the default color ramp
     * option is disabled.
     * @see setDefaultColorRamp
     */
    QgsColorRamp* defaultColorRamp() const;

    /** Sets whether a random colors option is shown in the button's drop down menu.
     * @param showNull set to true to show a null option
     * @see showRandom()
     */
    void setShowRandomColorRamp( bool showRandom );

    /** Returns whether random colors option is shown in the button's drop down menu.
     * @see setShowRandom()
     */
    bool showRandomColorRamp() const;

    /** Returns true if the current color is null.
     * @see setShowNull()
     * @see showNull()
     */
    bool isRandomColorRamp() const;

    /** Sets whether a set to null (clear) option is shown in the button's drop down menu.
     * @param showNull set to true to show a null option
     * @see showNull()
     * @see isNull()
     */
    void setShowNull( bool showNull );

    /** Returns whether the set to null (clear) option is shown in the button's drop down menu.
     * @see setShowNull()
     * @see isNull()
     */
    bool showNull() const;

    /** Returns true if the current color is null.
     * @see setShowNull()
     * @see showNull()
     */
    bool isNull() const;

    /** Sets the context string for the color ramp button. The context string is passed to all color ramp
     * preview icons shown in the button's drop down menu, to (eventually) allow them to customise their display colors
     * based on the context.
     * @param context context string for the color dialog button's color ramp preview icons
     * @see context
     */
    void setContext( const QString& context );

    /** Returns the context string for the color ramp button. The context string is passed to all color ramp
     * preview icons shown in the button's drop down menu, to (eventually) allow them to customise their display colors
     * based on the context.
     * @returns context context string for the color dialog button's color ramp preview icons
     * @see setContext
     */
    QString context() const;

    /** Sets whether the color ramp button only shows gradient type ramps
     * @param gradientonly set to true to show only gradient type ramps
     * @see showGradientOnly
     */
    void setShowGradientOnly( bool gradientonly );

    /** Returns true if the color ramp button only shows gradient type ramps
     * @see setShowGradientOnly
     */
    bool showGradientOnly() const;

  public slots:

    /** Sets the current color ramp for the button. Will emit a colorRampChanged() signal if the color ramp is different
     * to the previous color ramp.
     * @param colorramp New color ramp for the button
     * @see setRandomColorRamp, setColorRampFromName, colorRamp
     */
    void setColorRamp( QgsColorRamp* colorramp );

    /** Sets the current color ramp for the button to random colors. Will emit a colorRampChanged() signal
     * if the color ramp is different to the previous color ramp.
     * @see setColorRamp, setColorRampFromName, colorRamp
     */
    void setRandomColorRamp();

    /** Sets the current color ramp for the button using a saved color ramp name. Will emit a colorRampChanged() signal
     * if the color ramp is different to the previous color ramp.
     * @param name Name of saved color ramp
     * @see setColorRamp, setRandomColorRamp, colorRamp
     */
    void setColorRampFromName( QString name = QString() );

    /** Sets the background pixmap for the button based upon current color ramp.
     * @param colorramp Color ramp for button background. If no color ramp is specified, the button's current
     * color ramp will be used
     */
    void setButtonBackground( QgsColorRamp* colorramp = nullptr );

    /** Sets color ramp to the button's default color ramp, if set.
     * @see setDefaultColorRamp
     * @see defaultColorRamp
     * @see setToNull()
     */
    void setToDefaultColorRamp();

    /** Sets color ramp to null.
     * @see setToDefaultColorRamp()
     */
    void setToNull();

  signals:

    /** Emitted whenever a new color ramp is set for the button. The color ramp is always valid.
     * In case the new color ramp is the same, no signal is emitted to avoid infinite loops.
     */
    void colorRampChanged();

    /** Emitted when the button is clicked, if the button's behaviour is set to SignalOnly
     * @param colorramp Button's current color ramp
     * @see setBehaviour
     * @see behaviour
     * @note may not be available in Python bindings on some platforms
     */
    void colorRampClicked( const QgsColorRamp* colorramp );

  protected:

    bool event( QEvent *e );
    void changeEvent( QEvent* e );
    void showEvent( QShowEvent* e );
    void resizeEvent( QResizeEvent *event );

    /**
     * Reimplemented to detect right mouse button clicks on the color ramp button
     */
    void mousePressEvent( QMouseEvent* e );

};
