/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitempage.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutItemPage : QgsLayoutItem
{
%Docstring
 Item representing the paper in a layout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitempage.h"
%End
  public:

    enum Orientation
    {
      Portrait,
      Landscape
    };

    enum UndoCommand
    {
      UndoPageSymbol,
    };

    explicit QgsLayoutItemPage( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutItemPage, with the specified parent ``layout``.
%End

    static QgsLayoutItemPage *create( QgsLayout *layout ) /Factory/;
%Docstring
Returns a new page item for the specified ``layout``.

The caller takes responsibility for deleting the returned object.
%End

    virtual int type() const;


    void setPageSize( const QgsLayoutSize &size );
%Docstring
Sets the ``size`` of the page.

.. seealso:: :py:func:`pageSize`
%End

    bool setPageSize( const QString &size, Orientation orientation = Portrait );
%Docstring
Sets the page size to a known page ``size``, e.g. "A4" and ``orientation``.
The known page sizes are managed by :py:class:`QgsPageSizeRegistry`. Valid page sizes
can be retrieved via :py:func:`QgsPageSizeRegistry.entries()`
The function returns true if ``size`` was a valid page size and the page
size was changed. If false is returned then ``size`` could not be matched
to a known page size.

.. seealso:: :py:func:`pageSize`
%End

    QgsLayoutSize pageSize() const;
%Docstring
Returns the size of the page.

.. seealso:: :py:func:`setPageSize`
%End

    Orientation orientation() const;
%Docstring
Returns the page orientiation.

.. note::

   There is no direct setter for page orientation - use setPageSize() instead.
%End

    static QgsLayoutItemPage::Orientation decodePageOrientation( const QString &string, bool *ok /Out/ = 0 );
%Docstring
Decodes a ``string`` representing a page orientation. If specified, ``ok``
will be set to true if string could be successfully interpreted as a
page orientation.
%End

    virtual QRectF boundingRect() const;

    virtual void attemptResize( const QgsLayoutSize &size, bool includesFrame = false );

    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = 0 ) /Factory/;


  public slots:

    virtual void redraw();


  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual void drawFrame( QgsRenderContext &context );

    virtual void drawBackground( QgsRenderContext &context );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitempage.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
