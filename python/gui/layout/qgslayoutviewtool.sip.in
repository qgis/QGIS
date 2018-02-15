/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtool.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




%ModuleHeaderCode
#include <qgslayoutviewtooladditem.h>
%End

class QgsLayoutViewTool : QObject
{
%Docstring
Abstract base class for all layout view tools.
Layout view tools are user interactive tools for manipulating and adding items
to QgsLayoutView widgets.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewtool.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsLayoutViewToolAddItem *>( sipCpp ) != NULL )
      sipType = sipType_QgsLayoutViewToolAddItem;
    else
      sipType = NULL;
%End
  public:

    enum Flag
    {
      FlagSnaps,
    };
    typedef QFlags<QgsLayoutViewTool::Flag> Flags;


    ~QgsLayoutViewTool();

    QgsLayoutViewTool::Flags flags() const;
%Docstring
Returns the current combination of flags set for the tool.

.. seealso:: :py:func:`setFlags`
%End

    virtual void layoutMoveEvent( QgsLayoutViewMouseEvent *event );
%Docstring
Mouse move event for overriding. Default implementation does nothing.
%End

    virtual void layoutDoubleClickEvent( QgsLayoutViewMouseEvent *event );
%Docstring
Mouse double-click event for overriding. Default implementation does nothing.
%End

    virtual void layoutPressEvent( QgsLayoutViewMouseEvent *event );
%Docstring
Mouse press event for overriding. Default implementation does nothing.
Note that subclasses must ensure that they correctly handle cases
when a layoutPressEvent is called without a corresponding
layoutReleaseEvent (e.g. due to tool being changed mid way
through a press-release operation).
%End

    virtual void layoutReleaseEvent( QgsLayoutViewMouseEvent *event );
%Docstring
Mouse release event for overriding. Default implementation does nothing.
Note that subclasses must ensure that they correctly handle cases
when a layoutPressEvent is called without a corresponding
layoutReleaseEvent (e.g. due to tool being changed mid way
through a press-release operation).
%End

    virtual void wheelEvent( QWheelEvent *event );
%Docstring
Mouse wheel event for overriding. Default implementation does nothing.
%End

    virtual void keyPressEvent( QKeyEvent *event );
%Docstring
Key press event for overriding. Default implementation does nothing.
%End

    virtual void keyReleaseEvent( QKeyEvent *event );
%Docstring
Key release event for overriding. Default implementation does nothing.
%End

    void setAction( QAction *action );
%Docstring
Associates an ``action`` with this tool. When the setLayoutTool
method of QgsLayoutView is called the action's state will be set to on.
Usually this will cause a toolbutton to appear pressed in and
the previously used toolbutton to pop out.

.. seealso:: :py:func:`action`
%End

    QAction *action();
%Docstring
Returns the action associated with the tool or None if no action is associated.

.. seealso:: :py:func:`setAction`
%End

    void setCursor( const QCursor &cursor );
%Docstring
Sets a user defined ``cursor`` for use when the tool is active.
%End

    virtual void activate();
%Docstring
Called when tool is set as the currently active layout tool.
Overridden implementations must take care to call the base class implementation.
%End

    virtual void deactivate();
%Docstring
Called when tool is deactivated.
Overridden implementations must take care to call the base class implementation.
%End

    QString toolName() const;
%Docstring
Returns a user-visible, translated name for the tool.
%End

    QgsLayoutView *view() const;
%Docstring
Returns the view associated with the tool.

.. seealso:: :py:func:`layout`
%End

    QgsLayout *layout() const;
%Docstring
Returns the layout associated with the tool.

.. seealso:: :py:func:`view`
%End

    virtual QList< QgsLayoutItem * > ignoredSnapItems() const;
%Docstring
Returns a list of items which should be ignored while snapping events
for this tool.
%End

  signals:

    void activated();
%Docstring
Emitted when the tool is activated.
%End

    void deactivated();
%Docstring
Emitted when the tool is deactivated.
%End

    void itemFocused( QgsLayoutItem *item );
%Docstring
Emitted when an ``item`` is "focused" by the tool, i.e. it should become the active
item and should have its properties displayed in any designer windows.
%End

  protected:

    void setFlags( const QgsLayoutViewTool::Flags flags );
%Docstring
Sets the combination of ``flags`` that will be used for the tool.

.. seealso:: :py:func:`flags`
%End

    QgsLayoutViewTool( QgsLayoutView *view /TransferThis/, const QString &name );
%Docstring
Constructor for QgsLayoutViewTool, taking a layout ``view`` and
tool ``name`` as parameters.
%End

    bool isClickAndDrag( QPoint startViewPoint, QPoint endViewPoint ) const;
%Docstring
Returns true if a mouse press/release operation which started at
``startViewPoint`` and ended at ``endViewPoint`` should be considered
a "click and drag". If false is returned, the operation should be
instead treated as just a click on ``startViewPoint``.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtool.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
