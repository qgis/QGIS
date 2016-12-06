/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreemodellegendfullwidthnode.h            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsFullWidthLabelLegendNode : QgsSimpleLegendNode
{
%Docstring
 Implementation of legend node interface for displaying arbitrary label without icon.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayertreemodellegendfullwidthnode.h"
%End
  public:

    QgsFullWidthLabelLegendNode( QgsLayerTreeLayer *nodeLayer, const QString &label );
%Docstring
 QgsFullWidthLabelLegendNode constructor
 @param nodeLayer The node layer
 @param label The label
%End

    virtual bool fullWidth() const;
%Docstring
 Is a full width symbol, Is used when the text is above the legend graphics.
 :rtype: bool
%End

};

class QgsFullWidthSymbolLegendNode : QgsSymbolLegendNode
{
%Docstring
 Implementation of legend node interface for displaying preview of vector symbols without label.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayertreemodellegendfullwidthnode.h"
%End
  public:

    QgsFullWidthSymbolLegendNode(
      QgsLayerTreeLayer *nodeLayer, const QgsLegendSymbolItem &item,
      QObject *parent /TransferThis/ = 0 );
%Docstring
 QgsFullWidthSymbolLegendNode constructor
 \param nodeLayer The node layer
 \param item The legend symbol
 \param parent The parent node
%End

    virtual bool fullWidth() const;
%Docstring
 Is a full width symbol, Is used when the text is above the legend graphics.
 :rtype: bool
%End

    virtual bool editable() const;
%Docstring
 Is an editable symbol.
 :rtype: bool
%End

  protected:

     virtual SymbolMetrics symbolMetrics(
      const QgsLegendSettings &settings, const QgsRenderContext context,
      QgsSymbol *symbol /Transfer/ ) const;
%Docstring
 Get the symbol metrics
 \param settings
 \param context
 \param symbol
 :return: width, height, widthOffset, heightOffset
 :rtype: SymbolMetrics
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreemodellegendfullwidthnode.h            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
