/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscompoundcolorwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsCompoundColorWidget : QgsPanelWidget
{
%Docstring
A custom QGIS widget for selecting a color, including options for selecting colors via
hue wheel, color swatches, and a color sampler.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgscompoundcolorwidget.h"
%End
  public:

    enum Layout
    {
      LayoutDefault,
      LayoutVertical,
    };

    QgsCompoundColorWidget( QWidget *parent /TransferThis/ = 0, const QColor &color = QColor(), Layout layout = LayoutDefault );
%Docstring
Constructor for QgsCompoundColorWidget

:param parent: parent widget
:param color: initial color for dialog
:param layout: widget layout to use
%End

    ~QgsCompoundColorWidget();

    QColor color() const;
%Docstring
Returns the current color for the dialog

:return: dialog color
%End

    void setAllowOpacity( const bool allowOpacity );
%Docstring
Sets whether opacity modification (transparency) is permitted
for the color dialog. Defaults to true.

:param allowOpacity: set to false to disable opacity modification

.. versionadded:: 3.0
%End

    void setDiscarded( bool discarded );
%Docstring
Sets whether the widget's color has been "discarded" and the selected color should not
be stored in the recent color list.

:param discarded: set to true to avoid adding color to recent color list on widget destruction.

.. versionadded:: 3.0
%End

  signals:

    void currentColorChanged( const QColor &color );
%Docstring
Emitted when the dialog's color changes

:param color: current color
%End

  public slots:

    void setColor( const QColor &color );
%Docstring
Sets the current color for the dialog

:param color: desired color
%End

    void setPreviousColor( const QColor &color );
%Docstring
Sets the color to show in an optional "previous color" section

:param color: previous color
%End

  protected:

    virtual void mousePressEvent( QMouseEvent *e );


    virtual void mouseMoveEvent( QMouseEvent *e );


    virtual void mouseReleaseEvent( QMouseEvent *e );


    virtual void keyPressEvent( QKeyEvent *e );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscompoundcolorwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
