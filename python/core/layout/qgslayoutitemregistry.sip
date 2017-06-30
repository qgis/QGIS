/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemregistry.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutItemRegistry : QObject
{
%Docstring
 Registry of available layout item types.

 QgsLayoutItemRegistry is not usually directly created, but rather accessed through
 QgsApplication.layoutItemRegistry().

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemregistry.h"
%End
  public:

    enum ItemType
    {
      LayoutItem,

      // known
      LayoutPage,

      // item
      PluginItem,
    };

    QgsLayoutItemRegistry( QObject *parent = 0 );
%Docstring
 Creates a registry and populates it with standard item types.

 QgsLayoutItemRegistry is not usually directly created, but rather accessed through
 QgsApplication.layoutItemRegistry().
%End


};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemregistry.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
