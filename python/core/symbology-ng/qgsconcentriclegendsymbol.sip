/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology-ng/qgsconcentriclegendsymbol.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsConcentricLegendSymbol : QgsMarkerSymbol
{
%Docstring
 Symbol used to display the diagram size legend.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsconcentriclegendsymbol.h"
%End
  public:

    QgsConcentricLegendSymbol(
      const QgsMarkerSymbol *symbol /Transfer/,
      const QList< QPair< QString, double > > values,
      const QgsUnitTypes::RenderUnit types,
      const double maxSize,
      const QgsDiagramSettings::SizeLegendType type );
%Docstring
 Constructor
 \param symbol The symbol used at different size
 \param values The label, size pairs
 \param types The render unit types
 \param maxSize The maximum size
 \param type The type of legend
%End
    ~QgsConcentricLegendSymbol();
    virtual QgsConcentricLegendSymbol *clone() const /Factory/;

     virtual void drawPreviewIcon(
      QPainter *painter, QSize size,
      QgsRenderContext *customContext /Transfer/ = 0 );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology-ng/qgsconcentriclegendsymbol.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
