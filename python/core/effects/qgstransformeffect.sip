/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgstransformeffect.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsTransformEffect : QgsPaintEffect
{
%Docstring
 A paint effect which applies transformations (such as move,
 scale and rotate) to a picture.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgstransformeffect.h"
%End
  public:

    static QgsPaintEffect *create( const QgsStringMap &map ) /Factory/;
%Docstring
 Creates a new QgsTransformEffect effect from a properties string map.
 \param map encoded properties string map
 :return: new QgsTransformEffect
 :rtype: QgsPaintEffect
%End

    QgsTransformEffect();
%Docstring
 Constructor for QgsTransformEffect.
%End

    virtual QString type() const;
    virtual QgsStringMap properties() const;
    virtual void readProperties( const QgsStringMap &props );
    virtual QgsTransformEffect *clone() const /Factory/;

    void setTranslateX( const double translateX );
%Docstring
 Sets the transform x translation.
 \param translateX distance to translate along the x axis
.. seealso:: translateX
.. seealso:: setTranslateY
.. seealso:: setTranslateUnit
.. seealso:: setTranslateMapUnitScale
%End

    double translateX() const;
%Docstring
 Returns the transform x translation.
 :return: X distance translated along the x axis
.. seealso:: setTranslateX
.. seealso:: translateY
.. seealso:: translateUnit
.. seealso:: translateMapUnitScale
 :rtype: float
%End

    void setTranslateY( const double translateY );
%Docstring
 Sets the transform y translation.
 \param translateY distance to translate along the y axis
.. seealso:: translateY
.. seealso:: setTranslateX
.. seealso:: setTranslateUnit
.. seealso:: setTranslateMapUnitScale
%End

    double translateY() const;
%Docstring
 Returns the transform y translation.
 :return: Y distance translated along the y axis
.. seealso:: setTranslateY
.. seealso:: translateX
.. seealso:: translateUnit
.. seealso:: translateMapUnitScale
 :rtype: float
%End

    void setTranslateUnit( const QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units used for the transform translation.
 \param unit units for translation
.. seealso:: translateUnit
.. seealso:: setTranslateX
.. seealso:: setTranslateY
.. seealso:: setTranslateMapUnitScale
%End

    QgsUnitTypes::RenderUnit translateUnit() const;
%Docstring
 Returns the units used for the transform translation.
 :return: units for translation
.. seealso:: setTranslateUnit
.. seealso:: translateX
.. seealso:: translateY
.. seealso:: translateMapUnitScale
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setTranslateMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale used for the transform translation.
 \param scale map unit scale for translation
.. seealso:: translateMapUnitScale
.. seealso:: setTranslateX
.. seealso:: setTranslateY
.. seealso:: setTranslateUnit
%End

    const QgsMapUnitScale &translateMapUnitScale() const;
%Docstring
 Returns the map unit scale used for the transform translation.
 :return: map unit scale for translation
.. seealso:: setTranslateMapUnitScale
.. seealso:: translateX
.. seealso:: translateY
.. seealso:: translateUnit
 :rtype: QgsMapUnitScale
%End

    void setScaleX( const double scaleX );
%Docstring
 Sets the x axis scaling factor.
 \param scaleX factor to scale x axis by, where 1.0 = no scaling
.. seealso:: scaleX
.. seealso:: setScaleY
%End

    double scaleX() const;
%Docstring
 Returns the x axis scaling factor.
 :return: x axis scaling factor, where 1.0 = no scaling
.. seealso:: setScaleX
.. seealso:: scaleY
 :rtype: float
%End

    void setScaleY( const double scaleY );
%Docstring
 Sets the y axis scaling factor.
 \param scaleY factor to scale y axis by, where 1.0 = no scaling
.. seealso:: scaleX
%End

    double scaleY() const;
%Docstring
 Returns the y axis scaling factor.
 :return: y axis scaling factor, where 1.0 = no scaling
.. seealso:: setScaleY
.. seealso:: scaleX
 :rtype: float
%End

    void setRotation( const double rotation );
%Docstring
 Sets the transform ``rotation``, in degrees clockwise.
.. seealso:: rotation()
%End

    double rotation() const;
%Docstring
 Returns the transform rotation, in degrees clockwise.
.. seealso:: setRotation()
 :rtype: float
%End

    void setShearX( const double shearX );
%Docstring
 Sets the x axis shearing factor.
 \param shearX x axis shearing
.. seealso:: shearX
.. seealso:: setShearY
%End

    double shearX() const;
%Docstring
 Returns the x axis shearing factor.
 :return: x axis shearing
.. seealso:: setShearX
.. seealso:: shearY
 :rtype: float
%End

    void setShearY( const double shearY );
%Docstring
 Sets the y axis shearing factor.
 \param shearY y axis shearing
.. seealso:: shearY
.. seealso:: setShearX
%End

    double shearY() const;
%Docstring
 Returns the y axis shearing factor.
 :return: y axis shearing
.. seealso:: setShearY
.. seealso:: shearX
 :rtype: float
%End

    void setReflectX( const bool reflectX );
%Docstring
 Sets whether to reflect along the x-axis
 \param reflectX true to reflect horizontally
.. seealso:: reflectX
.. seealso:: setReflectY
%End

    bool reflectX() const;
%Docstring
 Returns whether transform will be reflected along the x-axis
 :return: true if transform will reflect horizontally
.. seealso:: setReflectX
.. seealso:: reflectY
 :rtype: bool
%End

    void setReflectY( const bool reflectY );
%Docstring
 Sets whether to reflect along the y-axis
 \param reflectY true to reflect horizontally
.. seealso:: reflectY
.. seealso:: setReflectX
%End

    bool reflectY() const;
%Docstring
 Returns whether transform will be reflected along the y-axis
 :return: true if transform will reflect horizontally
.. seealso:: setReflectY
.. seealso:: reflectX
 :rtype: bool
%End

  protected:

    virtual void draw( QgsRenderContext &context );
    virtual QRectF boundingRect( const QRectF &rect, const QgsRenderContext &context ) const;

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgstransformeffect.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
