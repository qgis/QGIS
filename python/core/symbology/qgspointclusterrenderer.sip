/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgspointclusterrenderer.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsPointClusterRenderer: QgsPointDistanceRenderer
{
%Docstring
A renderer that automatically clusters points with the same geographic position.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspointclusterrenderer.h"
%End
  public:

    QgsPointClusterRenderer();

    virtual QgsPointClusterRenderer *clone() const /Factory/;

    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );

    virtual void stopRender( QgsRenderContext &context );

    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;


    static QgsFeatureRenderer *create( QDomElement &symbologyElem, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Creates a renderer from XML element
%End

    QgsMarkerSymbol *clusterSymbol();
%Docstring
Returns the symbol used for rendering clustered groups (but not ownership of the symbol).

.. seealso:: :py:func:`setClusterSymbol`
%End

    void setClusterSymbol( QgsMarkerSymbol *symbol /Transfer/ );
%Docstring
Sets the symbol for rendering clustered groups.

:param symbol: new cluster symbol. Ownership is transferred to the renderer.

.. seealso:: :py:func:`clusterSymbol`
%End

    static QgsPointClusterRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
Creates a QgsPointClusterRenderer from an existing renderer.

:return: a new renderer if the conversion was possible, otherwise None.
%End

  private:
    QgsPointClusterRenderer( const QgsPointClusterRenderer & );
    QgsPointClusterRenderer &operator=( const QgsPointClusterRenderer & );
    virtual void drawGroup( QPointF centerPoint, QgsRenderContext &context, const QgsPointDistanceRenderer::ClusteredGroup &group ) ;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgspointclusterrenderer.h                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
