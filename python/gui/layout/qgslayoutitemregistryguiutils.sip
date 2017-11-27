/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutitemregistryguiutils.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItemRegistryGuiUtils
{
%Docstring
 A group of static utilities for working with the gui based portions of
 QgsLayoutItemRegistry.

 This class is designed to allow Python item subclasses to override the
 default GUI based QgsLayoutItemAbstractMetadata methods, which
 cannot be directly overridden from Python subclasses.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemregistryguiutils.h"
%End
  public:

    static void setItemRubberBandPrototype( int type, QgsLayoutViewRubberBand *prototype /Transfer/ );
%Docstring
 Sets a ``prototype`` for the rubber bands for the layout item with specified ``type``.
 Python subclasses of QgsLayoutItem must call this method to register their prototypes,
 as the usual c++ QgsLayoutItemAbstractMetadata are not accessible via the Python bindings.
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutitemregistryguiutils.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
