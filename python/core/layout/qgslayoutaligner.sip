/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutaligner.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutAligner
{
%Docstring
 Handles aligning and distributing sets of layout items.

 QgsLayoutAligner contains methods for automatically aligning and distributing
 sets of layout items, e.g. aligning a group of items to top or left sides.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutaligner.h"
%End
  public:

    enum Alignment
    {
      Left,
      HCenter,
      Right,
      Top,
      VCenter,
      Bottom,
    };

    static void alignItems( QgsLayout *layout, const QList< QgsLayoutItem * > &items, Alignment alignment );
%Docstring
 Aligns a set of ``items`` from a ``layout`` in place.

 The ``alignment`` argument specifies the method to use when aligning the items.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutaligner.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
