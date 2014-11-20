
/** \ingroup gui
 * \class QgsColorSwatchGrid
 * A grid of color swatches, which allows for user selection. Colors are taken from an
 * associated QgsColorScheme.
 * @see QgsColorGridAction
 * @note introduced in QGIS 2.5
 */
class QgsColorSwatchGrid : QWidget
{
%TypeHeaderCode
#include <qgscolorswatchgrid.h>
%End

  public:

    /**Construct a new color swatch grid.
     * @param scheme QgsColorScheme for colors to show in grid
     * @param context context string provided to color scheme
     * @param parent parent widget
     */
    QgsColorSwatchGrid( QgsColorScheme* scheme, QString context = QString(), QWidget *parent = 0 );

    virtual ~QgsColorSwatchGrid();

    //Reimplemented to set fixed size on widget
    virtual QSize minimumSizeHint() const;

    //Reimplemented to set fixed size on widget
    virtual QSize sizeHint() const;

    /**Get the current context for the grid
     * @returns context string which is passed to scheme for color generation
     * @see setContext
     */
    QString context() const;

    /**Sets the current context for the grid
     * @param context string which is passed to scheme for color generation
     * @see context
     */
    void setContext( const QString context );

    /**Get the base color for the widget
     * @returns base color which is passed to scheme for color generation
     * @see setBaseColor
     */
    QColor baseColor() const;

    /**Sets the base color for the widget
     * @param baseColor base color to pass to scheme for color generation
     * @see baseColor
     */
    void setBaseColor( const QColor &baseColor );

    /**Gets the list of colors shown in the grid
     * @returns list of colors currently shown in the grid
     */
    QgsNamedColorList *colors();

  public slots:

    /**Reload colors from scheme and redraws the widget
     */
    void refreshColors();

  signals:

    /**Emitted when a color has been selected from the widget
     * @param color selected color
     */
    void colorChanged( const QColor &color );

    /**Emitted when mouse hovers over widget
     */
    void hovered();

  protected:

    //reimplemented QWidget events
    void paintEvent( QPaintEvent * event );
    void mouseMoveEvent( QMouseEvent * event );
    void mousePressEvent( QMouseEvent * event );
    void mouseReleaseEvent( QMouseEvent * event );
    void keyPressEvent( QKeyEvent* event );
    void focusInEvent( QFocusEvent* event );
    void focusOutEvent( QFocusEvent* event );

};


/** \ingroup gui
 * \class QgsColorGridAction
 * A color swatch grid which can be embedded into a menu.
 * @see QgsColorSwatchGrid
 * @note introduced in QGIS 2.5
 */

class QgsColorSwatchGridAction: QWidgetAction
{

%TypeHeaderCode
#include <qgscolorswatchgrid.h>
%End

  public:

    /**Construct a new color swatch grid action.
     * @param scheme QgsColorScheme for colors to show in grid
     * @param menu parent menu
     * @param context context string provided to color scheme
     * @param parent parent widget
     */
    QgsColorSwatchGridAction( QgsColorScheme* scheme, QMenu* menu = 0, QString context = QString(), QWidget *parent = 0 );

    virtual ~QgsColorSwatchGridAction();

    /**Sets the base color for the color grid
     * @param baseColor base color to pass to scheme for color generation
     * @see baseColor
     */
    void setBaseColor( const QColor &baseColor );

    /**Get the base color for the color grid
     * @returns base color which is passed to scheme for color generation
     * @see setBaseColor
     */
    QColor baseColor() const;

    /**Get the current context for the color grid
     * @returns context string which is passed to scheme for color generation
     * @see setContext
     */
    QString context() const;

    /**Sets the current context for the color grid
     * @param context string which is passed to scheme for color generation
     * @see context
     */
    void setContext( const QString context );

  public slots:

    /**Reload colors from scheme and redraws the widget
     */
    void refreshColors();

  signals:

    /**Emitted when a color has been selected from the widget
     * @param color selected color
     */
    void colorChanged( const QColor &color );

};
