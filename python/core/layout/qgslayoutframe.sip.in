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

    static QgsLayoutFrame *create( QgsLayout *layout ) /Factory/;
%Docstring
Creates a new QgsLayoutFrame belonging to the specified ``layout``.
%End

    virtual int type() const;

    virtual QIcon icon() const;


    virtual QString displayName() const;


    virtual void cleanup();


    void setContentSection( const QRectF &section );
%Docstring
Sets the visible part of the multiframe's content which is visible within
this frame (relative to the total multiframe extent in layout units).

.. seealso:: :py:func:`extent`
%End

    QgsLayoutMultiFrame *multiFrame() const;
%Docstring
Returns the parent multiframe for the frame.
%End

    virtual QgsLayoutSize minimumSize() const;

    virtual QgsLayoutSize fixedSize() const;



    QRectF extent() const;
%Docstring
Returns the visible portion of the multi frame's content which
is shown in this frame, in layout units.

.. seealso:: :py:func:`setContentSection`
%End

    bool hidePageIfEmpty() const;
%Docstring
Returns whether the page should be hidden (ie, not included in layout exports) if this frame is empty

:return: true if page should be hidden if frame is empty

.. seealso:: :py:func:`setHidePageIfEmpty`
%End

    void setHidePageIfEmpty( const bool hidePageIfEmpty );
%Docstring
Sets whether the page should be hidden (ie, not included in layout exports) if this frame is empty

:param hidePageIfEmpty: set to true if page should be hidden if frame is empty

.. seealso:: :py:func:`hidePageIfEmpty`
%End

    bool hideBackgroundIfEmpty() const;
%Docstring
Returns whether the background and frame stroke should be hidden if this frame is empty

:return: true if background and stroke should be hidden if frame is empty

.. seealso:: :py:func:`setHideBackgroundIfEmpty`
%End

    void setHideBackgroundIfEmpty( const bool hideBackgroundIfEmpty );
%Docstring
Sets whether the background and frame stroke should be hidden if this frame is empty

:param hideBackgroundIfEmpty: set to true if background and stroke should be hidden if frame is empty

.. seealso:: :py:func:`hideBackgroundIfEmpty`
%End

    bool isEmpty() const;
%Docstring
Returns whether the frame is empty.

.. seealso:: :py:func:`hidePageIfEmpty`
%End

    virtual QgsExpressionContext createExpressionContext() const;


  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual void drawFrame( QgsRenderContext &context );

    virtual void drawBackground( QgsRenderContext &context );

    virtual bool writePropertiesToElement( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &itemElement, const QDomDocument &document, const QgsReadWriteContext &context );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutframe.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
