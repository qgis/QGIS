/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsvectorlayerdirector.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsVectorLayerDirector : QgsGraphDirector
{
%Docstring
.. versionadded:: 3.0
 Determine making the graph from vector line layer
%End

%TypeHeaderCode
#include "qgsvectorlayerdirector.h"
%End
  public:

    enum Direction
    {
      DirectionForward,
      DirectionBackward,
      DirectionBoth,
    };

    QgsVectorLayerDirector( QgsVectorLayer *myLayer,
                            int directionFieldId,
                            const QString &directDirectionValue,
                            const QString &reverseDirectionValue,
                            const QString &bothDirectionValue,
                            const Direction defaultDirection
                          );
%Docstring
 Default constructor
 \param myLayer source vector layer
 \param directionFieldId field containing direction value
 \param directDirectionValue value for direct one-way road
 \param reverseDirectionValue value for reversed one-way road
 \param bothDirectionValue value for two-way (bidirectional) road
 \param defaultDirection default direction. Will be used if corresponding
 attribute value is not set or does not equal to the given values
%End

    virtual ~QgsVectorLayerDirector();

     virtual void makeGraph( QgsGraphBuilderInterface *builder,
                    const QVector< QgsPoint > &additionalPoints,
                    QVector< QgsPoint> &snappedPoints /Out/ ) const;
%Docstring
 MANDATORY DIRECTOR PROPERTY DECLARATION
%End

    virtual QString name() const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsvectorlayerdirector.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
