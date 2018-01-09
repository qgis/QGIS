/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutreportcontext.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsLayoutReportContext : QObject
{
%Docstring
 Stores information relating to the current reporting context for a layout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutreportcontext.h"
%End
  public:

    QgsLayoutReportContext( QgsLayout *layout /TransferThis/ );
%Docstring
Constructor for QgsLayoutReportContext.
%End

    void setFeature( const QgsFeature &feature );
%Docstring
Sets the current ``feature`` for evaluating the layout. This feature may
be used for altering an item's content and appearance for a report
or atlas layout.

Emits the changed() signal.

.. seealso:: :py:func:`feature`
%End

    QgsFeature feature() const;
%Docstring
Returns the current feature for evaluating the layout. This feature may
be used for altering an item's content and appearance for a report
or atlas layout.

.. seealso:: :py:func:`currentGeometry`

.. seealso:: :py:func:`setFeature`
%End

    QgsGeometry currentGeometry( const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() ) const;
%Docstring
Returns the current feature() geometry in the given ``crs``.
If no CRS is specified, the original feature geometry is returned.

Reprojection only works if a valid layer is set for layer().

.. seealso:: :py:func:`feature`

.. seealso:: :py:func:`layer`
%End

    QgsVectorLayer *layer() const;
%Docstring
Returns the vector layer associated with the layout's context.

.. seealso:: :py:func:`setLayer`
%End

    void setLayer( QgsVectorLayer *layer );
%Docstring
Sets the vector ``layer`` associated with the layout's context.

Emits the changed() signal.

.. seealso:: :py:func:`layer`
%End

    void setPredefinedScales( const QVector<qreal> &scales );
%Docstring
Sets the list of predefined ``scales`` to use with the layout. This is used
for maps which are set to the predefined atlas scaling mode.

.. seealso:: :py:func:`predefinedScales`
%End

    QVector<qreal> predefinedScales() const;
%Docstring
Returns the current list of predefined scales for use with the layout.

.. seealso:: :py:func:`setPredefinedScales`
%End

  signals:

    void layerChanged( QgsVectorLayer *layer );
%Docstring
Emitted when the context's ``layer`` is changed.
%End

    void changed();
%Docstring
Emitted certain settings in the context is changed, e.g. by setting a new feature or vector layer
for the context.
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutreportcontext.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
