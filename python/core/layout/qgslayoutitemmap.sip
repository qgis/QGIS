/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemmap.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutItemMap : QgsLayoutItem
{
%Docstring
 Layout graphical items for displaying a map.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemmap.h"
%End
  public:

    explicit QgsLayoutItemMap( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItemMap, with the specified parent ``layout``.
%End

  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemmap.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
