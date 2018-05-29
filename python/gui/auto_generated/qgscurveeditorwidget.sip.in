/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscurveeditorwidget.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsCurveEditorWidget : QWidget
{
%Docstring
A widget for manipulating QgsCurveTransform curves.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscurveeditorwidget.h"
%End
  public:

    QgsCurveEditorWidget( QWidget *parent /TransferThis/ = 0, const QgsCurveTransform &curve = QgsCurveTransform() );
%Docstring
Constructor for QgsCurveEditorWidget.
%End

    ~QgsCurveEditorWidget();

    QgsCurveTransform curve() const;
%Docstring
Returns a curve representing the current curve from the widget.

.. seealso:: :py:func:`setCurve`
%End

    void setCurve( const QgsCurveTransform &curve );
%Docstring
Sets the ``curve`` to show in the widget.

.. seealso:: :py:func:`curve`
%End

    void setHistogramSource( const QgsVectorLayer *layer, const QString &expression );
%Docstring
Sets a ``layer`` and ``expression`` source for values to show in a histogram
behind the curve. The histogram is generated in a background thread to keep
the widget responsive.

.. seealso:: :py:func:`minHistogramValueRange`

.. seealso:: :py:func:`maxHistogramValueRange`
%End

    double minHistogramValueRange() const;
%Docstring
Returns the minimum expected value for the range of values shown in the histogram.

.. seealso:: :py:func:`maxHistogramValueRange`

.. seealso:: :py:func:`setMinHistogramValueRange`
%End

    double maxHistogramValueRange() const;
%Docstring
Returns the maximum expected value for the range of values shown in the histogram.

.. seealso:: :py:func:`minHistogramValueRange`

.. seealso:: :py:func:`setMaxHistogramValueRange`
%End

  public slots:

    void setMinHistogramValueRange( double minValueRange );
%Docstring
Sets the minimum expected value for the range of values shown in the histogram.

.. seealso:: :py:func:`setMaxHistogramValueRange`

.. seealso:: :py:func:`minHistogramValueRange`
%End

    void setMaxHistogramValueRange( double maxValueRange );
%Docstring
Sets the maximum expected value for the range of values shown in the histogram.

.. seealso:: :py:func:`setMinHistogramValueRange`

.. seealso:: :py:func:`maxHistogramValueRange`
%End

  signals:

    void changed();
%Docstring
Emitted when the widget curve changes
%End

  protected:

    virtual void keyPressEvent( QKeyEvent *event );


};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscurveeditorwidget.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
