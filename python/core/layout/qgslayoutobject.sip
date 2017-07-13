/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutobject.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutObject: QObject
{
%Docstring
 A base class for objects which belong to a layout.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutobject.h"
%End
  public:

    QgsLayoutObject( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutObject, with the specified parent ``layout``.
.. note::

   While ownership of a QgsLayoutObject is not passed to the layout,
 classes which are derived from QgsLayoutObject (such as QgsLayoutItem)
 may transfer their ownership to a layout upon construction.
%End


    QgsLayout *layout();
%Docstring
 Returns the layout the object is attached to.
 :rtype: QgsLayout
%End

  protected:


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutobject.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
