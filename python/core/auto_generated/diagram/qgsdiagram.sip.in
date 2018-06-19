/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/diagram/qgsdiagram.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsDiagram
{
%Docstring
Base class for all diagram types*
%End

%TypeHeaderCode
#include "qgsdiagram.h"
%End
  public:

    virtual ~QgsDiagram();

    virtual QgsDiagram *clone() const = 0 /Factory/;
%Docstring
Returns an instance that is equivalent to this one

.. versionadded:: 2.4
%End

    void clearCache();

    QgsExpression *getExpression( const QString &expression, const QgsExpressionContext &context );
%Docstring
Returns a prepared expression for the specified context.

:param expression: expression string
:param context: expression context

.. versionadded:: 2.12
%End

    virtual void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position ) = 0;
%Docstring
Draws the diagram at the given position (in pixel coordinates)
%End

    virtual QString diagramName() const = 0;
%Docstring
Gets a descriptive name for this diagram type.
%End
    virtual QSizeF diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s ) = 0;
%Docstring
Returns the size in map units the diagram will use to render.
%End
    virtual QSizeF diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) = 0;
%Docstring
Returns the size in map units the diagram will use to render. Interpolate size
%End

    virtual double legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const = 0;
%Docstring
Returns the size of the legend item for the diagram corresponding to a specified value.

:param value: value to return legend item size for
:param s: diagram settings
:param is: interpolation settings

.. versionadded:: 2.16
%End

  protected:

    QgsDiagram();
%Docstring
Constructor for QgsDiagram.
%End
    QgsDiagram( const QgsDiagram &other );

    void setPenWidth( QPen &pen, const QgsDiagramSettings &s, const QgsRenderContext &c );
%Docstring
Changes the pen width to match the current settings and rendering context

:param pen: The pen to modify
:param s: The settings that specify the pen width
:param c: The rendering specifying the proper scale units for pixel conversion
%End

    QSizeF sizePainterUnits( QSizeF size, const QgsDiagramSettings &s, const QgsRenderContext &c );
%Docstring
Calculates a size to match the current settings and rendering context

:param size: The size to convert
:param s: The settings that specify the size type
:param c: The rendering specifying the proper scale units for pixel conversion

:return: The converted size for rendering
%End

    double sizePainterUnits( double l, const QgsDiagramSettings &s, const QgsRenderContext &c );
%Docstring
Calculates a length to match the current settings and rendering context

:param l: The length to convert
:param s: Unused
:param c: The rendering specifying the proper scale units for pixel conversion

:return: The converted length for rendering
%End

    QFont scaledFont( const QgsDiagramSettings &s, const QgsRenderContext &c );
%Docstring
Calculates a size to match the current settings and rendering context

:param s: The settings that contain the font size and size type
:param c: The rendering specifying the proper scale units for pixel conversion

:return: The properly scaled font for rendering
%End

    QSizeF sizeForValue( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const;
%Docstring
Returns the scaled size of a diagram for a value, respecting the specified diagram interpolation settings.

:param value: value to calculate corresponding circular size for
:param s: diagram settings
:param is: interpolation settings

.. versionadded:: 2.16
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/diagram/qgsdiagram.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
