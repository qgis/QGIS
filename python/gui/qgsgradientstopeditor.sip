/** \ingroup gui
 * \class QgsGradientStopEditor
 * An interactive editor for previewing a gradient color ramp and modifying the position of color
 * stops along the gradient.
 * \note Added in version 2.16
 */

class QgsGradientStopEditor : QWidget
{
%TypeHeaderCode
#include <qgsgradientstopeditor.h>
%End

  public:

    /** Constructor for QgsGradientStopEditor.
     * @param parent parent widget
     * @param ramp optional initial gradient ramp
     */
    QgsGradientStopEditor( QWidget* parent /TransferThis/ = nullptr, QgsVectorGradientColorRampV2* ramp = nullptr );

    /** Sets the current ramp shown in the editor.
     * @param ramp color ramp
     * @see gradientRamp()
     */
    void setGradientRamp( const QgsVectorGradientColorRampV2& ramp );

    /** Returns the current ramp created by the editor.
     * @see setGradientRamp()
     */
    QgsVectorGradientColorRampV2 gradientRamp() const;

    /** Sets the currently selected stop.
     * @param index index of stop, where 0 corresponds to the first stop
     * @see selectedStop()
     */
    void selectStop( int index );

    /** Returns details about the currently selected stop.
     * @see selectStop()
     */
    QgsGradientStop selectedStop() const;

    virtual QSize sizeHint() const;
    void paintEvent( QPaintEvent* event );

  public slots:

    /** Sets the color for the current selected stop.
     * @param color new stop color
     * @see setSelectedStopOffset()
     * @see setSelectedStopDetails()
     * @see setColor1()
     * @see setColor2()
     */
    void setSelectedStopColor( const QColor& color );

    /** Sets the offset for the current selected stop. This slot has no effect if either the
     * first or last stop is selected, as they cannot be repositioned.
     * @param offset new stop offset
     * @see setSelectedStopColor()
     * @see setSelectedStopDetails()
     */
    void setSelectedStopOffset( double offset );

    /** Sets the color and offset for the current selected stop.
     * @param color new stop color
     * @param offset new stop offset
     * @see setSelectedStopColor()
     * @see setSelectedStopOffset()
     */
    void setSelectedStopDetails( const QColor& color, double offset );

    /** Deletes the current selected stop. This slot has no effect if either the
     * first or last stop is selected, as they cannot be deleted.
     */
    void deleteSelectedStop();

    /** Sets the color for the first stop.
     * @param color new stop color
     * @see setColor2()
     * @see setSelectedStopColor()
     */
    void setColor1( const QColor& color );

    /** Sets the color for the last stop.
     * @param color new stop color
     * @see setColor1()
     * @see setSelectedStopColor()
     */
    void setColor2( const QColor& color );

  signals:

    //! Emitted when the gradient ramp is changed by a user
    void changed();

    /** Emitted when the current selected stop changes.
     * @param stop details about newly selected stop
     */
    void selectedStopChanged( const QgsGradientStop& stop );

  protected:

    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void mouseDoubleClickEvent( QMouseEvent * event );
    virtual void keyPressEvent( QKeyEvent * event );

    //Reimplemented to accept dragged colors
    void dragEnterEvent( QDragEnterEvent * e );

    //Reimplemented to accept dropped colors
    void dropEvent( QDropEvent *e );
};
