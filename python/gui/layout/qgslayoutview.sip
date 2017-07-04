/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutview.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutView: QGraphicsView
{
%Docstring
 A graphical widget to display and interact with QgsLayouts.

 QgsLayoutView manages the layout interaction tools and mouse/key events.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutview.h"
%End
  public:

    enum Tool
    {
      ToolSelect,
      ToolAddItem,
    };

    QgsLayoutView( QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsLayoutView.
%End

    QgsLayout *currentLayout();
%Docstring
 Returns the current layout associated with the view.
.. seealso:: setCurrentLayout()
.. seealso:: layoutSet()
 :rtype: QgsLayout
%End

    void setCurrentLayout( QgsLayout *layout /KeepReference/ );
%Docstring
 Sets the current ``layout`` to edit in the view.
.. seealso:: currentLayout()
.. seealso:: layoutSet()
%End

  signals:

    void layoutSet( QgsLayout *layout );
%Docstring
 Emitted when a ``layout`` is set for the view.
.. seealso:: currentLayout()
.. seealso:: setCurrentLayout()
%End

  protected:
    virtual void mousePressEvent( QMouseEvent *event );

    virtual void mouseReleaseEvent( QMouseEvent *event );

    virtual void mouseMoveEvent( QMouseEvent *event );

    virtual void mouseDoubleClickEvent( QMouseEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutview.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
