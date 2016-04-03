
/** \ingroup gui
 * \class QgsCompoundColorWidget
 * A custom QGIS widget for selecting a color, including options for selecting colors via
 * hue wheel, color swatches, and a color sampler.
 * \note Added in version 2.16
 */

class QgsCompoundColorWidget : QWidget
{
%TypeHeaderCode
#include <qgscompoundcolorwidget.h>
%End

  public:

    /** Constructor for QgsCompoundColorWidget
     * @param parent parent widget
     * @param color initial color for dialog
     */
    QgsCompoundColorWidget( QWidget *parent /TransferThis/ = nullptr, const QColor& color = QColor() );

    ~QgsCompoundColorWidget();

    /** Returns the current color for the dialog
     * @returns dialog color
     */
    QColor color() const;

    /** Sets whether alpha modification (transparency) is permitted
     * for the color dialog. Defaults to true.
     * @param allowAlpha set to false to disable alpha modification
     */
    void setAllowAlpha( const bool allowAlpha );

  signals:

    /** Emitted when the dialog's color changes
     * @param color current color
     */
    void currentColorChanged( const QColor &color );

  public slots:

    /** Sets the current color for the dialog
     * @param color desired color
     */
    void setColor( const QColor &color );

    /** Sets the color to show in an optional "previous color" section
     * @param color previous color
     */
    void setPreviousColor( const QColor& color );

  protected:

    void mousePressEvent( QMouseEvent* e );

    void mouseMoveEvent( QMouseEvent *e );

    void mouseReleaseEvent( QMouseEvent *e );

    void keyPressEvent( QKeyEvent *e );
};
