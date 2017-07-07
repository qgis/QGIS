/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayout.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsLayout : QGraphicsScene
{
%Docstring
 Base class for layouts, which can contain items such as maps, labels, scalebars, etc.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayout.h"
%End
  public:

    enum ZValues
    {
      ZMapTool,
    };

    QgsLayout();

    QString name() const;
%Docstring
 Returns the layout's name.
.. seealso:: setName()
 :rtype: str
%End

    void setName( const QString &name );
%Docstring
 Sets the layout's name.
.. seealso:: name()
%End

    void setUnits( QgsUnitTypes::LayoutUnit units );
%Docstring
 Sets the native measurement ``units`` for the layout. These also form the default unit
 for measurements for the layout.
.. seealso:: units()
.. seealso:: convertToLayoutUnits()
%End

    QgsUnitTypes::LayoutUnit units() const;
%Docstring
 Returns the native units for the layout.
.. seealso:: setUnits()
.. seealso:: convertToLayoutUnits()
 :rtype: QgsUnitTypes.LayoutUnit
%End

    double convertToLayoutUnits( const QgsLayoutMeasurement &measurement ) const;
%Docstring
 Converts a measurement into the layout's native units.
 :return: length of measurement in layout units
.. seealso:: convertFromLayoutUnits()
.. seealso:: units()
 :rtype: float
%End

    QSizeF convertToLayoutUnits( const QgsLayoutSize &size ) const;
%Docstring
 Converts a size into the layout's native units.
 :return: size of measurement in layout units
.. seealso:: convertFromLayoutUnits()
.. seealso:: units()
 :rtype: QSizeF
%End

    QPointF convertToLayoutUnits( const QgsLayoutPoint &point ) const;
%Docstring
 Converts a ``point`` into the layout's native units.
 :return: point in layout units
.. seealso:: convertFromLayoutUnits()
.. seealso:: units()
 :rtype: QPointF
%End

    QgsLayoutMeasurement convertFromLayoutUnits( const double length, const QgsUnitTypes::LayoutUnit unit ) const;
%Docstring
 Converts a ``length`` measurement from the layout's native units to a specified target ``unit``.
 :return: length of measurement in specified units
.. seealso:: convertToLayoutUnits()
.. seealso:: units()
 :rtype: QgsLayoutMeasurement
%End

    QgsLayoutSize convertFromLayoutUnits( const QSizeF &size, const QgsUnitTypes::LayoutUnit unit ) const;
%Docstring
 Converts a ``size`` from the layout's native units to a specified target ``unit``.
 :return: size of measurement in specified units
.. seealso:: convertToLayoutUnits()
.. seealso:: units()
 :rtype: QgsLayoutSize
%End

    QgsLayoutPoint convertFromLayoutUnits( const QPointF &point, const QgsUnitTypes::LayoutUnit unit ) const;
%Docstring
 Converts a ``point`` from the layout's native units to a specified target ``unit``.
 :return: point in specified units
.. seealso:: convertToLayoutUnits()
.. seealso:: units()
 :rtype: QgsLayoutPoint
%End

    QgsLayoutContext &context();
%Docstring
 Returns a reference to the layout's context, which stores information relating to the
 current context and rendering settings for the layout.
 :rtype: QgsLayoutContext
%End


};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayout.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
