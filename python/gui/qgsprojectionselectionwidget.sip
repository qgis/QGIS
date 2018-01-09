/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsprojectionselectionwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProjectionSelectionWidget : QWidget
{
%Docstring
 A widget for selecting a projection.

.. versionadded:: 2.7
%End

%TypeHeaderCode
#include "qgsprojectionselectionwidget.h"
%End
  public:

    enum CrsOption
    {
      LayerCrs,
      ProjectCrs,
      CurrentCrs,
      DefaultCrs,
      RecentCrs,
      CrsNotSet,
    };

    explicit QgsProjectionSelectionWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsProjectionSelectionWidget
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Returns the currently selected CRS for the widget

:return: current CRS
%End

    void setOptionVisible( const CrsOption option, const bool visible );
%Docstring
Sets whether a predefined CRS option should be shown in the widget.

:param option: CRS option to show/hide
:param visible: whether the option should be shown

.. seealso:: :py:func:`optionVisible`
%End

    bool optionVisible( CrsOption option ) const;
%Docstring
Returns whether the specified CRS option is visible in the widget.

.. versionadded:: 3.0

.. seealso:: :py:func:`setOptionVisible`
%End

    void setNotSetText( const QString &text );
%Docstring
Sets the text to show for the not set option. Note that this option is not shown
by default and must be set visible by calling setOptionVisible().

.. versionadded:: 3.0
%End

    void setMessage( const QString &text );
%Docstring
Sets a ``message`` to show in the dialog. If an empty string is
passed, the message will be a generic
'define the CRS for this layer'.

.. versionadded:: 3.0
%End

  signals:

    void crsChanged( const QgsCoordinateReferenceSystem & );
%Docstring
Emitted when the selected CRS is changed
%End

    void cleared();
%Docstring
Emitted when the not set option is selected.

.. versionadded:: 3.0
%End

  public slots:

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the current CRS for the widget

:param crs: new CRS
%End

    void setLayerCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the layer CRS for the widget. If set, this will be added as an option
to the preset CRSes shown in the widget.

:param crs: layer CRS
%End

    void selectCrs();
%Docstring
Opens the dialog for selecting a new CRS
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsprojectionselectionwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
