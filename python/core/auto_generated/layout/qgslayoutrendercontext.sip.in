/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutrendercontext.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutRenderContext : QObject
{
%Docstring
 Stores information relating to the current rendering settings for a layout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutrendercontext.h"
%End
  public:

    enum Flag
    {
      FlagDebug,
      FlagOutlineOnly,
      FlagAntialiasing,
      FlagUseAdvancedEffects,
      FlagForceVectorOutput,
      FlagHideCoverageLayer,
    };
    typedef QFlags<QgsLayoutRenderContext::Flag> Flags;


    QgsLayoutRenderContext( QgsLayout *layout /TransferThis/ );
%Docstring
Constructor for QgsLayoutRenderContext.
%End

    void setFlags( const QgsLayoutRenderContext::Flags flags );
%Docstring
Sets the combination of ``flags`` that will be used for rendering the layout.

.. seealso:: :py:func:`setFlag`

.. seealso:: :py:func:`flags`

.. seealso:: :py:func:`testFlag`
%End

    void setFlag( const QgsLayoutRenderContext::Flag flag, const bool on = true );
%Docstring
Enables or disables a particular rendering ``flag`` for the layout. Other existing
flags are not affected.

.. seealso:: :py:func:`setFlags`

.. seealso:: :py:func:`flags`

.. seealso:: :py:func:`testFlag`
%End

    QgsLayoutRenderContext::Flags flags() const;
%Docstring
Returns the current combination of flags used for rendering the layout.

.. seealso:: :py:func:`setFlags`

.. seealso:: :py:func:`setFlag`

.. seealso:: :py:func:`testFlag`
%End

    bool testFlag( const Flag flag ) const;
%Docstring
Check whether a particular rendering ``flag`` is enabled for the layout.

.. seealso:: :py:func:`setFlags`

.. seealso:: :py:func:`setFlag`

.. seealso:: :py:func:`flags`
%End

    QgsRenderContext::Flags renderContextFlags() const;
%Docstring
Returns the combination of render context flags matched to the layout context's settings.
%End

    void setDpi( double dpi );
%Docstring
Sets the ``dpi`` for outputting the layout. This also sets the
corresponding DPI for the context's measurementConverter().

.. seealso:: :py:func:`dpi`
%End

    double dpi() const;
%Docstring
Returns the ``dpi`` for outputting the layout.

.. seealso:: :py:func:`setDpi`
%End


    QgsLayoutMeasurementConverter &measurementConverter();
%Docstring
Returns the layout measurement converter to be used in the layout. This converter is used
for translating between other measurement units and the layout's native unit.
%End

    bool isPreviewRender() const;
%Docstring
Returns true if the render current being conducted is a preview render,
i.e. it is being rendered inside a QGraphicsView widget as opposed to a destination
device (such as an image).
%End

    bool gridVisible() const;
%Docstring
Returns true if the page grid should be drawn.

.. seealso:: :py:func:`setGridVisible`
%End

    void setGridVisible( bool visible );
%Docstring
Sets whether the page grid should be ``visible``.

.. seealso:: :py:func:`gridVisible`
%End

    bool boundingBoxesVisible() const;
%Docstring
Returns true if the item bounding boxes should be drawn.

.. seealso:: :py:func:`setBoundingBoxesVisible`
%End

    void setBoundingBoxesVisible( bool visible );
%Docstring
Sets whether the item bounding boxes should be ``visible``.

.. seealso:: :py:func:`boundingBoxesVisible`
%End

    void setPagesVisible( bool visible );
%Docstring
Sets whether the page items should be ``visible`` in the layout. Removing
them will prevent both display of the page boundaries in layout views and
will also prevent them from being rendered in layout exports.

.. seealso:: :py:func:`pagesVisible`
%End

    bool pagesVisible() const;
%Docstring
Returns whether the page items are be visible in the layout. This setting
effects both display of the page boundaries in layout views and
whether they will be rendered in layout exports.

.. seealso:: :py:func:`setPagesVisible`
%End

    void setCurrentExportLayer( int layer = -1 );
%Docstring
Sets the current item ``layer`` to draw while exporting. QgsLayoutItem subclasses
which support multi-layer SVG exports must check the currentExportLayer()
and customise their rendering based on the layer.

If ``layer`` is -1, all item layers will be rendered.

.. seealso:: :py:func:`currentExportLayer`
%End

    int currentExportLayer() const;
%Docstring
Returns the current item layer to draw while exporting. QgsLayoutItem subclasses
which support multi-layer SVG exports must check this
and customise their rendering based on the layer.

If ``layer`` is -1, all item layers should be rendered.

.. seealso:: :py:func:`setCurrentExportLayer`
%End

  signals:

    void flagsChanged( QgsLayoutRenderContext::Flags flags );
%Docstring
Emitted whenever the context's ``flags`` change.

.. seealso:: :py:func:`setFlags`
%End

    void dpiChanged();
%Docstring
Emitted when the context's DPI is changed.
%End

};





/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutrendercontext.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
