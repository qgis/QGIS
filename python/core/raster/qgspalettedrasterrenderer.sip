/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgspalettedrasterrenderer.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsPalettedRasterRenderer: QgsRasterRenderer
{
%Docstring
 Renderer for paletted raster images.
%End

%TypeHeaderCode
#include "qgspalettedrasterrenderer.h"
%End
  public:

    struct Class
    {
      Class( int value, const QColor &color = QColor(), const QString &label = QString() );
%Docstring
Constructor for Class
%End

      int value;
%Docstring
Value
%End

      QColor color;
%Docstring
Color to render value
%End
      QString label;
%Docstring
Label for value
%End
    };

    typedef QList< QgsPalettedRasterRenderer::Class > ClassData;

    QgsPalettedRasterRenderer( QgsRasterInterface *input, int bandNumber, const ClassData &classes );
%Docstring
 Constructor for QgsPalettedRasterRenderer.
%End


    virtual QgsPalettedRasterRenderer *clone() const /Factory/;

%Docstring
QgsPalettedRasterRenderer cannot be copied. Use clone() instead.
 :rtype: QgsPalettedRasterRenderer
%End
    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) /Factory/;
%Docstring
 :rtype: QgsRasterRenderer
%End

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = 0 ) /Factory/;


    int nColors() const;
%Docstring
Returns number of colors
 :rtype: int
%End

    ClassData classes() const;
%Docstring
 Returns a map of value to classes (colors) used by the renderer.
 :rtype: ClassData
%End

    QString label( int idx ) const;
%Docstring
 Return optional category label
.. versionadded:: 2.1
 :rtype: str
%End

    void setLabel( int idx, const QString &label );
%Docstring
 Set category label
.. versionadded:: 2.1
%End

    int band() const;
%Docstring
 Returns the raster band used for rendering the raster.
.. versionadded:: 3.0
 :rtype: int
%End

    virtual void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;


    virtual void legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems /Out/ ) const;


    virtual QList<int> usesBands() const;


    void setSourceColorRamp( QgsColorRamp *ramp /Transfer/ );
%Docstring
 Set the source color ``ramp``. Ownership is transferred to the renderer.
.. versionadded:: 3.0
.. seealso:: :py:func:`sourceColorRamp()`
%End

    QgsColorRamp *sourceColorRamp() const;
%Docstring
 Get the source color ramp
.. versionadded:: 3.0
.. seealso:: :py:func:`setSourceColorRamp()`
 :rtype: QgsColorRamp
%End

    static QgsPalettedRasterRenderer::ClassData colorTableToClassData( const QList<QgsColorRampShader::ColorRampItem> &table );
%Docstring
 Converts a raster color ``table`` to paletted renderer class data.
.. versionadded:: 3.0
 :rtype: QgsPalettedRasterRenderer.ClassData
%End

    static QgsPalettedRasterRenderer::ClassData classDataFromString( const QString &string );
%Docstring
 Converts a ``string`` containing a color table or class data to to paletted renderer class data.
.. versionadded:: 3.0
.. seealso:: :py:func:`classDataFromFile()`
.. seealso:: :py:func:`classDataToString()`
 :rtype: QgsPalettedRasterRenderer.ClassData
%End

    static QgsPalettedRasterRenderer::ClassData classDataFromFile( const QString &path );
%Docstring
 Opens a color table file and returns corresponding paletted renderer class data.
.. versionadded:: 3.0
.. seealso:: :py:func:`classDataFromString()`
 :rtype: QgsPalettedRasterRenderer.ClassData
%End

    static QString classDataToString( const QgsPalettedRasterRenderer::ClassData &classes );
%Docstring
 Converts classes to a string representation, using the .clr/gdal color table file format.
.. versionadded:: 3.0
.. seealso:: :py:func:`classDataFromString()`
 :rtype: str
%End

    static QgsPalettedRasterRenderer::ClassData classDataFromRaster( QgsRasterInterface *raster, int bandNumber, QgsColorRamp *ramp = 0,
        QgsRasterBlockFeedback *feedback = 0 );
%Docstring
 Generates class data from a ``raster``, for the specified ``bandNumber``. An optional
 color ``ramp`` can be specified to automatically assign colors from the ramp.
.. versionadded:: 3.0
 :rtype: QgsPalettedRasterRenderer.ClassData
%End

  private:
    QgsPalettedRasterRenderer( const QgsPalettedRasterRenderer & );
    const QgsPalettedRasterRenderer &operator=( const QgsPalettedRasterRenderer & );
%Docstring
 :rtype: QgsPalettedRasterRenderer
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgspalettedrasterrenderer.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
