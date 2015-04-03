
/** \ingroup gui
 * \class QgsColorDialog
 * A dialog for selecting a color
 */

class QgsColorDialog : QObject
{
%TypeHeaderCode
#include <qgscolordialog.h>
%End
  public:
    QgsColorDialog();
    ~QgsColorDialog();

    /** Return a color selection from a QColorDialog, with live updating of interim selections.
     * @param initialColor The initial color of the selection dialog.
     * @param updateObject The receiver object of the live updating.
     * @param updateSlot The receiver object's slot for live updating (e.g. SLOT( setValidColor( const QColor& ) ) ).
     * @param parent Parent widget. Usually 0 is best for native system color dialogs.
     * @param title The title of the QColorDialog.
     * @param options ColorDialogOptions passed to QColorDialog.
     * @return Selected color on accepted() or initialColor on rejected().
     */
    static QColor getLiveColor( const QColor& initialColor, QObject* updateObject, const char* updateSlot,
                                QWidget* parent = 0,
                                const QString& title = "",
                                QColorDialog::ColorDialogOptions options = 0 );
};


/** \ingroup gui
 * \class QgsColorDialogV2
 * A custom QGIS dialog for selecting a color. Has many improvements over the standard Qt color picker dialog, including
 * hue wheel supports, color swatches, and a color sampler.
 * \note Added in version 2.5
 */

class QgsColorDialogV2 : QDialog
{
%TypeHeaderCode
#include <qgscolordialog.h>
%End

  public:

    /**Create a new color picker dialog
     * @param parent parent widget
     * @param fl window flags
     * @param color initial color for dialog
     */
    QgsColorDialogV2( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags,
                      const QColor& color = QColor() );

    ~QgsColorDialogV2();

    /**Returns the current color for the dialog
     * @returns dialog color
     */
    QColor color() const;

    /**Sets the title for the color dialog
     * @param title title for dialog box
     */
    void setTitle( const QString title );

    /**Sets whether alpha modification (transparency) is permitted
     * for the color dialog. Defaults to true.
     * @param allowAlpha set to false to disable alpha modification
     */
    void setAllowAlpha( const bool allowAlpha );

    /**Return a color selection from a color dialog, with live updating of interim selections.
     * @param initialColor the initial color of the selection dialog.
     * @param updateObject the receiver object of the live updating.
     * @param updateSlot the receiver object's slot for live updating (e.g. SLOT( setValidColor( const QColor& ) ) ).
     * @param parent parent widget
     * @param title the title of the dialog.
     * @param allowAlpha set to true to allow modification of color alpha value (transparency)
     * @return Selected color on accepted() or initialColor on rejected().
     * @see getColor
     */
    static QColor getLiveColor( const QColor& initialColor, QObject* updateObject, const char* updateSlot,
                                QWidget* parent = 0,
                                const QString& title = QString(),
                                const bool allowAlpha = true );

    /**Return a color selection from a color dialog.
     * @param initialColor the initial color of the selection dialog.
     * @param parent parent widget
     * @param title the title of the dialog.
     * @param allowAlpha set to true to allow modification of color alpha value (transparency)
     * @return Selected color on accepted() or initialColor on rejected().
     * @see getLiveColor
     */
    static QColor getColor( const QColor &initialColor, QWidget *parent, const QString &title = QString(),
                            const bool allowAlpha = false );

  signals:

    /**Emitted when the dialog's color changes
     * @param color current color
     */
    void currentColorChanged( const QColor &color );

  public slots:

    /**Sets the current color for the dialog
     * @param color desired color
     */
    void setColor( const QColor &color );

  protected:

    void closeEvent( QCloseEvent* e );

    void mousePressEvent( QMouseEvent* e );

    void mouseMoveEvent( QMouseEvent *e );

    void mouseReleaseEvent( QMouseEvent *e );

    void keyPressEvent( QKeyEvent *e );
};
