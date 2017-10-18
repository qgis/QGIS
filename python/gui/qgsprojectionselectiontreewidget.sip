/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsprojectionselectiontreewidget.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsProjectionSelectionTreeWidget : QWidget
{
%Docstring
 A widget for selecting a coordinate reference system from a tree.

 This widget implements a tree view of projections, as seen in the
 QgsProjectionSelectionDialog dialog. In most cases it is more
 suitable to use the compact QgsProjectionSelectionWidget widget.

.. seealso:: QgsProjectionSelectionDialog.
.. seealso:: QgsProjectionSelectionWidget
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprojectionselectiontreewidget.h"
%End
  public:

    QgsProjectionSelectionTreeWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsProjectionSelectionTreeWidget.
%End

    ~QgsProjectionSelectionTreeWidget();

    QgsCoordinateReferenceSystem crs() const;
%Docstring
 Returns the CRS currently selected in the widget.
.. versionadded:: 3.0
.. seealso:: setCrs()
 :rtype: QgsCoordinateReferenceSystem
%End

    void setShowNoProjection( bool show );
%Docstring
 Sets whether a "no/invalid" projection option should be shown. If this
 option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
.. seealso:: showNoProjection()
.. versionadded:: 3.0
%End

    void setShowBoundsMap( bool show );
%Docstring
 Sets whether to show the bounnds preview map.
.. seealso:: showBoundsMap()
.. versionadded:: 3.0
%End

    bool showNoProjection() const;
%Docstring
 Returns whether the "no/invalid" projection option is shown. If this
 option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
.. versionadded:: 3.0
.. seealso:: setShowNoProjection()
 :rtype: bool
%End

    bool showBoundsMap() const;
%Docstring
 Returns whether the bounds preview map is shown.
.. versionadded:: 3.0
.. seealso:: setShowBoundsMap()
 :rtype: bool
%End

    bool hasValidSelection() const;
%Docstring
 Returns true if the current selection in the widget is a valid choice. Valid
 selections include any projection and also the "no/invalid projection" option
 (if setShowNoProjection() was called). Invalid selections are the group
 headers (such as "Geographic Coordinate Systems"
 :rtype: bool
%End

  public slots:

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the initial ``crs`` to show within the dialog.
.. versionadded:: 3.0
.. seealso:: crs()
%End

    void setPreviewRect( const QgsRectangle &rect );
%Docstring
 Sets the initial "preview" rectangle for the bounds overview map.
.. versionadded:: 3.0
.. seealso:: previewRect()
%End

    QgsRectangle previewRect() const;
%Docstring
 The initial "preview" rectangle for the bounds overview map.
.. versionadded:: 3.0
.. seealso:: previewRect()
 :rtype: QgsRectangle
%End

    void setOgcWmsCrsFilter( const QSet<QString> &crsFilter );
%Docstring
 filters this widget by the given CRSs

 Sets this widget to filter the available projections to those listed
 by the given Coordinate Reference Systems.

 \param crsFilter a list of OGC Coordinate Reference Systems to filter the
                  list of projections by.  This is useful in (e.g.) WMS situations
                  where you just want to offer what the WMS server can support.

 \warning This function's behavior is undefined if it is called after the widget is shown.
%End

    void pushProjectionToFront();
%Docstring
 Marks the current selected projection for push to front of recent projections list.
%End

  signals:

    void crsSelected();
%Docstring
 Emitted when a projection is selected in the widget.
%End

    void initialized();
%Docstring
 Notifies others that the widget is now fully initialized, including deferred selection of projection.
.. versionadded:: 2.4
%End

    void projectionDoubleClicked();
%Docstring
 Emitted when a projection is double clicked in the list.
.. versionadded:: 2.14
%End

  protected:
    virtual void showEvent( QShowEvent *event );


    virtual void resizeEvent( QResizeEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsprojectionselectiontreewidget.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
