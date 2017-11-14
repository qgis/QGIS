/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsextentgroupbox.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsExtentGroupBox : QgsCollapsibleGroupBox
{
%Docstring
 Collapsible group box for configuration of extent, typically for a save operation.

 Besides allowing the user to enter the extent manually, it comes with options to use
 original extent or extent defined by the current view in map canvas.

 When using the widget, make sure to call setOriginalExtent(), setCurrentExtent() and setOutputCrs() during initialization.

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsextentgroupbox.h"
%End
  public:

    enum ExtentState
    {
      OriginalExtent,
      CurrentExtent,
      UserExtent,
      ProjectLayerExtent,
      DrawOnCanvas,
    };

    explicit QgsExtentGroupBox( QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsExtentGroupBox.
%End

    void setOriginalExtent( const QgsRectangle &originalExtent, const QgsCoordinateReferenceSystem &originalCrs );
%Docstring
 Sets the original extent and coordinate reference system for the widget. This should be called as part of initialization.
.. seealso:: originalExtent()
.. seealso:: originalCrs()
%End

    QgsRectangle originalExtent() const;
%Docstring
 Returns the original extent set for the widget.
.. seealso:: setOriginalExtent()
.. seealso:: originalCrs()
 :rtype: QgsRectangle
%End

    QgsCoordinateReferenceSystem originalCrs() const;
%Docstring
 Returns the original coordinate reference system set for the widget.
.. seealso:: originalExtent()
.. seealso:: setOriginalExtent()
 :rtype: QgsCoordinateReferenceSystem
%End

    void setCurrentExtent( const QgsRectangle &currentExtent, const QgsCoordinateReferenceSystem &currentCrs );
%Docstring
 Sets the current extent to show in the widget - should be called as part of initialization (or whenever current extent changes).
 The current extent is usually set to match the current map canvas extent.
.. seealso:: currentExtent()
.. seealso:: currentCrs()
%End

    QgsRectangle currentExtent() const;
%Docstring
 Returns the current extent set for the widget. The current extent is usually set to match the
 current map canvas extent.
.. seealso:: setCurrentExtent()
.. seealso:: currentCrs()
 :rtype: QgsRectangle
%End

    QgsCoordinateReferenceSystem currentCrs() const;
%Docstring
 Returns the coordinate reference system for the current extent set for the widget. The current
 extent and CRS usually reflects the map canvas extent and CRS.
.. seealso:: setCurrentExtent()
.. seealso:: currentExtent()
 :rtype: QgsCoordinateReferenceSystem
%End

    void setOutputCrs( const QgsCoordinateReferenceSystem &outputCrs );
%Docstring
 Sets the output CRS - may need to be used for transformation from original/current extent.
 Should be called as part of initialization and whenever the the output CRS is changed.
 The current extent will be reprojected into the new output CRS.
%End

    QgsRectangle outputExtent() const;
%Docstring
 Returns the extent shown in the widget - in output CRS coordinates.
 :rtype: QgsRectangle
%End

    QgsExtentGroupBox::ExtentState extentState() const;
%Docstring
 Returns the currently selected state for the widget's extent.
 :rtype: QgsExtentGroupBox.ExtentState
%End

    void setTitleBase( const QString &title );
%Docstring
 Sets the base part of ``title`` of the group box (will be appended with extent state)
.. versionadded:: 2.12
.. seealso:: titleBase()
%End

    QString titleBase() const;
%Docstring
 Returns the base part of title of the group box (will be appended with extent state).
.. versionadded:: 2.12
.. seealso:: setTitleBase()
 :rtype: str
%End

    void setMapCanvas( QgsMapCanvas *canvas );
%Docstring
 Sets the map canvas to enable dragging of extent on a canvas.
 \param canvas the map canvas
.. versionadded:: 3.0
%End

    QSize ratio() const;
%Docstring
 Returns the current fixed aspect ratio to be used when dragging extent onto the canvas.
 If the aspect ratio isn't fixed, the width and height will be set to zero.
.. versionadded:: 3.0
 *
 :rtype: QSize
%End

  public slots:

    void setOutputExtentFromOriginal();
%Docstring
 Sets the output extent to be the same as original extent (may be transformed to output CRS).
%End

    void setOutputExtentFromCurrent();
%Docstring
 Sets the output extent to be the same as current extent (may be transformed to output CRS).
%End

    void setOutputExtentFromUser( const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the output extent to a custom extent (may be transformed to output CRS).
%End

    void setOutputExtentFromLayer( const QgsMapLayer *layer );
%Docstring
 Sets the output extent to match a ``layer``'s extent (may be transformed to output CRS).
.. versionadded:: 3.0
%End

    void setOutputExtentFromDrawOnCanvas();
%Docstring
 Sets the output extent by dragging on the canvas.
.. versionadded:: 3.0
%End

    void setRatio( QSize ratio );
%Docstring
 Sets a fixed aspect ratio to be used when dragging extent onto the canvas.
 To unset a fixed aspect ratio, set the width and height to zero.
 \param ratio aspect ratio's width and height
.. versionadded:: 3.0
 *
%End

  signals:

    void extentChanged( const QgsRectangle &r );
%Docstring
 Emitted when the widget's extent is changed.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsextentgroupbox.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
