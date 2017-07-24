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

    explicit QgsLayoutItemPage( QgsLayout *layout /TransferThis/ );
%Docstring
 Constructor for QgsLayoutItemPage, with the specified parent ``layout``.
%End
    virtual int type() const;
    virtual QString stringType() const;

    void setPageSize( const QgsLayoutSize &size );
%Docstring
 Sets the ``size`` of the page.
.. seealso:: pageSize()
%End

    bool setPageSize( const QString &size, Orientation orientation = Portrait );
%Docstring
 Sets the page size to a known page ``size``, e.g. "A4" and ``orientation``.
 The known page sizes are managed by QgsPageSizeRegistry. Valid page sizes
 can be retrieved via QgsPageSizeRegistry.entries().
 The function returns true if ``size`` was a valid page size and the page
 size was changed. If false is returned then ``size`` could not be matched
 to a known page size.
.. seealso:: pageSize()
 :rtype: bool
%End

    QgsLayoutSize pageSize() const;
%Docstring
 Returns the size of the page.
.. seealso:: setPageSize()
 :rtype: QgsLayoutSize
%End

    Orientation orientation() const;
%Docstring
 Returns the page orientiation.
.. note::

   There is no direct setter for page orientation - use setPageSize() instead.
 :rtype: Orientation
%End

    static QgsLayoutItemPage::Orientation decodePageOrientation( const QString &string, bool *ok /Out/ = 0 );
%Docstring
 Decodes a ``string`` representing a page orientation. If specified, ``ok``
 will be set to true if string could be successfully interpreted as a
 page orientation.
 :rtype: QgsLayoutItemPage.Orientation
%End

    virtual void attemptResize( const QgsLayoutSize &size );


  public slots:

    virtual void redraw();


  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitempage.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
