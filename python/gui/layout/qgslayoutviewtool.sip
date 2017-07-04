/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtool.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




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
    if ( dynamic_cast<QgsMapToolZoom *>( sipCpp ) != NULL )
      sipType = sipType_QgsMapToolZoom;
    else
      sipType = NULL;
%End
  public:

    virtual ~QgsLayoutViewTool();

    virtual void layoutMoveEvent( QMouseEvent *event );
%Docstring
 Mouse move event for overriding. Default implementation does nothing.
%End

    virtual void layoutDoubleClickEvent( QMouseEvent *event );
%Docstring
 Mouse double click event for overriding. Default implementation does nothing.
%End

    virtual void layoutPressEvent( QMouseEvent *event );
%Docstring
 Mouse press event for overriding. Default implementation does nothing.
%End

    virtual void layoutReleaseEvent( QMouseEvent *event );
%Docstring
 Mouse release event for overriding. Default implementation does nothing.
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
.. seealso:: action()
%End

    QAction *action();
%Docstring
 Returns the action associated with the tool or None if no action is associated.
.. seealso:: setAction()
 :rtype: QAction
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
 :rtype: str
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

  protected:

    QgsLayoutViewTool( QgsLayoutView *view /TransferThis/, const QString &name );
%Docstring
 Constructor for QgsLayoutViewTool, taking a layout ``view`` and
 tool ``name`` as parameters.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtool.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
