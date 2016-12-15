/** \ingroup gui
 * \class QgsFloatingWidget
 * A QWidget subclass for creating widgets which float outside of the normal Qt layout
 * system. Floating widgets use an "anchor widget" to determine how they are anchored
 * within their parent widget.
 * \note Added in version 2.16
 */

class QgsFloatingWidget : QWidget
{
%TypeHeaderCode
#include <qgsfloatingwidget.h>
%End

  public:

    //! Reference points for anchoring widget position
    enum AnchorPoint
    {
      TopLeft, //!< Top-left of widget
      TopMiddle, //!< Top center of widget
      TopRight, //!< Top-right of widget
      MiddleLeft, //!< Middle left of widget
      Middle, //!< Middle of widget
      MiddleRight, //!< Middle right of widget
      BottomLeft, //!< Bottom-left of widget
      BottomMiddle, //!< Bottom center of widget
      BottomRight, //!< Bottom-right of widget
    };

    /** Constructor for QgsFloatingWidget.
     * @param parent parent widget
     */
    QgsFloatingWidget( QWidget* parent /TransferThis/ = nullptr );

    /** Sets the widget to "anchor" the floating widget to. The floating widget will be repositioned whenever the
     * anchor widget moves or is resized so that it maintains the same relative position to the anchor widget.
     * @param widget anchor widget. Both the floating widget and the anchor widget must share some common parent.
     * @see anchorWidget()
     */
    void setAnchorWidget( QWidget* widget );

    /** Returns the widget that the floating widget is "anchored" tto. The floating widget will be repositioned whenever the
     * anchor widget moves or is resized so that it maintains the same relative position to the anchor widget.
     * @see setAnchorWidget()
     */
    QWidget* anchorWidget();

    /** Returns the floating widget's anchor point, which corresponds to the point on the widget which should remain
     * fixed in the same relative position whenever the widget's parent is resized or moved.
     * @see setAnchorPoint()
     */
    AnchorPoint anchorPoint() const;

    /** Sets the floating widget's anchor point, which corresponds to the point on the widget which should remain
     * fixed in the same relative position whenever the widget's parent is resized or moved.
     * @param point anchor point
     * @see anchorPoint()
     */
    void setAnchorPoint( AnchorPoint point );

    /** Returns the anchor widget's anchor point, which corresponds to the point on the anchor widget which
     * the floating widget should "attach" to. The floating widget should remain fixed in the same relative position
     * to this anchor widget whenever the widget's parent is resized or moved.
     * @see setAnchorWidgetPoint()
     */
    AnchorPoint anchorWidgetPoint() const;

    /** Returns the anchor widget's anchor point, which corresponds to the point on the anchor widget which
     * the floating widget should "attach" to. The floating widget should remain fixed in the same relative position
     * to this anchor widget whenever the widget's parent is resized or moved.
     * @see setAnchorWidgetPoint()
     */
    void setAnchorWidgetPoint( AnchorPoint point );

  signals:

    //! Emitted when the anchor widget changes
    void anchorWidgetChanged( QWidget* widget );

    //! Emitted when the anchor point changes
    void anchorPointChanged( AnchorPoint point );

    //! Emitted when the anchor widget point changes
    void anchorWidgetPointChanged( AnchorPoint point );

  protected:
    void showEvent( QShowEvent* e );
    virtual void paintEvent( QPaintEvent* e );

};
