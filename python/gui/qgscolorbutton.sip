
/** \ingroup gui
 * \class QgsColorButton
 * A cross platform button subclass for selecting colors. Will open a color chooser dialog when clicked.
 * Offers live updates to button from color chooser dialog
 * @note inherited base class moved from QToolButton to QPushButton in QGIS 1.9
 */

class QgsColorButton: QPushButton
{
%TypeHeaderCode
#include <qgscolorbutton.h>
%End

  public:
    /**
     * Construct a new color button.
     *
     * @param parent The parent QWidget for the dialog
     * @param cdt The title to show in the color chooser dialog
     * @param cdo Options for the color chooser dialog
     * @note changed in 1.9
     */
    QgsColorButton( QWidget *parent /TransferThis/ = 0, QString cdt = "", QColorDialog::ColorDialogOptions cdo = 0 );
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
    void setColorDialogOptions( QColorDialog::ColorDialogOptions cdo );

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
    void setColorDialogTitle( QString cdt );

    /**
     * Returns the title, which the color chooser dialog shows.
     *
     * @return Title for the color chooser dialog
     */
    QString colorDialogTitle();

    /**
     * Whether the button accepts live updates from QColorDialog.
     */
    bool acceptLiveUpdates();

    /**
     * Sets whether the button accepts live updates from QColorDialog.
     * Live updates may cause changes that are not undoable on QColorDialog cancel.
     */
    void setAcceptLiveUpdates( bool accept );

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
    void changeEvent( QEvent* e );
    void showEvent( QShowEvent* e );
    static const QPixmap& transpBkgrd();

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
     * Reimplemented to accept dropped colors
     */
    void dropEvent( QDropEvent *e );
};
