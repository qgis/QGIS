/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgshistogramwidget.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





typedef QPointF QwtDoublePoint;


class QgsHistogramWidget : QWidget
{
%Docstring
 Graphical histogram for displaying distributions of field values.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgshistogramwidget.h"
%End
  public:

    QgsHistogramWidget( QWidget *parent /TransferThis/ = 0, QgsVectorLayer *layer = 0, const QString &fieldOrExp = QString() );
%Docstring
 QgsHistogramWidget constructor. If layer and fieldOrExp are specified then the histogram
 will be initially populated with the corresponding values.
 \param parent parent widget
 \param layer source vector layer
 \param fieldOrExp field name or expression string
%End

    ~QgsHistogramWidget();

    QgsVectorLayer *layer();
%Docstring
 Returns the layer currently associated with the widget.
.. seealso:: setLayer
.. seealso:: sourceFieldExp
 :rtype: QgsVectorLayer
%End

    QString sourceFieldExp() const;
%Docstring
 Returns the source field name or expression used to calculate values displayed
 in the histogram.
.. seealso:: setSourceFieldExp
.. seealso:: layer
 :rtype: str
%End

    void setPen( const QPen &pen );
%Docstring
 Sets the pen to use when drawing histogram bars. If set to Qt.NoPen then the
 pen will be automatically calculated. If ranges have been set using setGraduatedRanges()
 then the pen and brush will have no effect.
 \param pen histogram pen
.. seealso:: pen
.. seealso:: setBrush
%End

    QPen pen() const;
%Docstring
 Returns the pen used when drawing histogram bars.
.. seealso:: setPen
.. seealso:: brush
 :rtype: QPen
%End

    void setBrush( const QBrush &brush );
%Docstring
 Sets the brush used for drawing histogram bars. If ranges have been set using setGraduatedRanges()
 then the pen and brush will have no effect.
 \param brush histogram brush
.. seealso:: brush
.. seealso:: setPen
%End

    QBrush brush() const;
%Docstring
 Returns the brush used when drawing histogram bars.
.. seealso:: setBrush
.. seealso:: pen
 :rtype: QBrush
%End

    void setGraduatedRanges( const QgsRangeList &ranges );
%Docstring
 Sets the graduated ranges associated with the histogram. If set, the ranges will be used to color the histogram
 bars and for showing vertical dividers at the histogram breaks.
 \param ranges graduated range list
.. seealso:: graduatedRanges
%End

    QgsRangeList graduatedRanges() const;
%Docstring
 Returns the graduated ranges associated with the histogram. If set, the ranges will be used to color the histogram
 bars and for showing vertical dividers at the histogram breaks.
 :return: graduated range list
.. seealso:: setGraduatedRanges
 :rtype: QgsRangeList
%End

    QString xAxisTitle() const;
%Docstring
 Returns the title for the histogram's x-axis.
.. seealso:: setXAxisTitle
.. seealso:: yAxisTitle
 :rtype: str
%End

    void setXAxisTitle( const QString &title );
%Docstring
 Sets the title for the histogram's x-axis.
 \param title x-axis title, or empty string to remove title
.. seealso:: xAxisTitle
.. seealso:: setYAxisTitle
%End

    QString yAxisTitle() const;
%Docstring
 Returns the title for the histogram's y-axis.
.. seealso:: setYAxisTitle
.. seealso:: xAxisTitle
 :rtype: str
%End

    void setYAxisTitle( const QString &title );
%Docstring
 Sets the title for the histogram's y-axis.
 \param title y-axis title, or empty string to remove title
.. seealso:: yAxisTitle
.. seealso:: setXAxisTitle
%End

  public slots:

    void refreshValues();
%Docstring
 Refreshes the values for the histogram by fetching them from the layer.
%End

    void refresh();
%Docstring
 Redraws the histogram. Calling this slot does not update the values
 for the histogram, use refreshValues() to do this.
%End

    void setLayer( QgsVectorLayer *layer );
%Docstring
 Sets the vector layer associated with the histogram.
 \param layer source vector layer
.. seealso:: setSourceFieldExp
%End

    void setSourceFieldExp( const QString &fieldOrExp );
%Docstring
 Sets the source field or expression to use for values in the histogram.
 \param fieldOrExp field name or expression string
.. seealso:: setLayer
%End

  protected:

    virtual void drawHistogram();
%Docstring
 Updates and redraws the histogram.
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgshistogramwidget.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
