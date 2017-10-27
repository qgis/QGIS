/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutframe.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutFrame: QgsLayoutItem
{
%Docstring
 Base class for frame items, which form a layout multiframe item.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutframe.h"
%End
  public:

    QgsLayoutFrame( QgsLayout *layout, QgsLayoutMultiFrame *multiFrame );
%Docstring
 Constructor for QgsLayoutFrame, with the specified parent ``layout``
 and belonging to a ``multiFrame``.
%End

    virtual int type() const;

    virtual QString stringType() const;


    virtual QString displayName() const;


    void setContentSection( const QRectF &section );
%Docstring
 Sets the visible part of the multiframe's content which is visible within
 this frame (relative to the total multiframe extent in layout units).
.. seealso:: extent()
%End

    QgsLayoutMultiFrame *multiFrame() const;
%Docstring
 Returns the parent multiframe for the frame.
 :rtype: QgsLayoutMultiFrame
%End


    QRectF extent() const;
%Docstring
 Returns the visible portion of the multi frame's content which
 is shown in this frame, in layout units.
.. seealso:: setContentSection()
 :rtype: QRectF
%End

    bool hidePageIfEmpty() const;
%Docstring
 Returns whether the page should be hidden (ie, not included in layout exports) if this frame is empty
 :return: true if page should be hidden if frame is empty
.. seealso:: setHidePageIfEmpty()
 :rtype: bool
%End

    void setHidePageIfEmpty( const bool hidePageIfEmpty );
%Docstring
 Sets whether the page should be hidden (ie, not included in layout exports) if this frame is empty
 \param hidePageIfEmpty set to true if page should be hidden if frame is empty
.. seealso:: hidePageIfEmpty()
%End

    bool hideBackgroundIfEmpty() const;
%Docstring
 Returns whether the background and frame stroke should be hidden if this frame is empty
 :return: true if background and stroke should be hidden if frame is empty
.. seealso:: setHideBackgroundIfEmpty()
 :rtype: bool
%End

    void setHideBackgroundIfEmpty( const bool hideBackgroundIfEmpty );
%Docstring
 Sets whether the background and frame stroke should be hidden if this frame is empty
 \param hideBackgroundIfEmpty set to true if background and stroke should be hidden if frame is empty
.. seealso:: hideBackgroundIfEmpty()
%End

    bool isEmpty() const;
%Docstring
 Returns whether the frame is empty.
.. seealso:: hidePageIfEmpty()
 :rtype: bool
%End

    virtual QgsExpressionContext createExpressionContext() const;


  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    void drawFrame( QgsRenderContext &context );
    void drawBackground( QgsRenderContext &context );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutframe.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
