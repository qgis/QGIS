/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorswatchgrid.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsColorSwatchGrid : QWidget
{
%Docstring
A grid of color swatches, which allows for user selection. Colors are taken from an
associated QgsColorScheme.

.. seealso:: :py:class:`QgsColorGridAction`

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorswatchgrid.h"
%End
  public:

    QgsColorSwatchGrid( QgsColorScheme *scheme, const QString &context = QString(), QWidget *parent /TransferThis/ = 0 );
%Docstring
Construct a new color swatch grid.

:param scheme: QgsColorScheme for colors to show in grid
:param context: context string provided to color scheme
:param parent: parent widget
%End

    virtual QSize minimumSizeHint() const;


    virtual QSize sizeHint() const;


    QString context() const;
%Docstring
Get the current context for the grid

:return: context string which is passed to scheme for color generation

.. seealso:: :py:func:`setContext`
%End

    void setContext( const QString &context );
%Docstring
Sets the current context for the grid

:param context: string which is passed to scheme for color generation

.. seealso:: :py:func:`context`
%End

    QColor baseColor() const;
%Docstring
Get the base color for the widget

:return: base color which is passed to scheme for color generation

.. seealso:: :py:func:`setBaseColor`
%End

    void setBaseColor( const QColor &baseColor );
%Docstring
Sets the base color for the widget

:param baseColor: base color to pass to scheme for color generation

.. seealso:: :py:func:`baseColor`
%End

    QgsNamedColorList *colors();
%Docstring
Gets the list of colors shown in the grid

:return: list of colors currently shown in the grid
%End

  public slots:

    void refreshColors();
%Docstring
Reload colors from scheme and redraws the widget
%End

  signals:

    void colorChanged( const QColor &color );
%Docstring
Emitted when a color has been selected from the widget

:param color: selected color
%End

    void hovered();
%Docstring
Emitted when mouse hovers over widget
%End

  protected:

    virtual void paintEvent( QPaintEvent *event );

    virtual void mouseMoveEvent( QMouseEvent *event );

    virtual void mousePressEvent( QMouseEvent *event );

    virtual void mouseReleaseEvent( QMouseEvent *event );

    virtual void keyPressEvent( QKeyEvent *event );

    virtual void focusInEvent( QFocusEvent *event );

    virtual void focusOutEvent( QFocusEvent *event );


};



class QgsColorSwatchGridAction: QWidgetAction
{
%Docstring
A color swatch grid which can be embedded into a menu.

.. seealso:: :py:class:`QgsColorSwatchGrid`

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorswatchgrid.h"
%End
  public:

    QgsColorSwatchGridAction( QgsColorScheme *scheme, QMenu *menu = 0, const QString &context = QString(), QWidget *parent /TransferThis/ = 0 );
%Docstring
Construct a new color swatch grid action.

:param scheme: QgsColorScheme for colors to show in grid
:param menu: parent menu
:param context: context string provided to color scheme
:param parent: parent widget
%End

    void setBaseColor( const QColor &baseColor );
%Docstring
Sets the base color for the color grid

:param baseColor: base color to pass to scheme for color generation

.. seealso:: :py:func:`baseColor`
%End

    QColor baseColor() const;
%Docstring
Get the base color for the color grid

:return: base color which is passed to scheme for color generation

.. seealso:: :py:func:`setBaseColor`
%End

    QString context() const;
%Docstring
Get the current context for the color grid

:return: context string which is passed to scheme for color generation

.. seealso:: :py:func:`setContext`
%End

    void setContext( const QString &context );
%Docstring
Sets the current context for the color grid

:param context: string which is passed to scheme for color generation

.. seealso:: :py:func:`context`
%End

    void setDismissOnColorSelection( bool dismiss );
%Docstring
Sets whether the parent menu should be dismissed and closed when a color is selected
from the action's color widget.

:param dismiss: set to true (default) to immediately close the menu when a color is selected
from the widget. If set to false, the colorChanged signal will be emitted but the menu will
stay open.

.. seealso:: :py:func:`dismissOnColorSelection`

.. versionadded:: 2.14
%End

    bool dismissOnColorSelection() const;
%Docstring
Returns whether the parent menu will be dismissed after a color is selected from the
action's color widget.

.. seealso:: :py:func:`setDismissOnColorSelection`

.. versionadded:: 2.14
%End

  public slots:

    void refreshColors();
%Docstring
Reload colors from scheme and redraws the widget
%End

  signals:

    void colorChanged( const QColor &color );
%Docstring
Emitted when a color has been selected from the widget

:param color: selected color
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscolorswatchgrid.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
