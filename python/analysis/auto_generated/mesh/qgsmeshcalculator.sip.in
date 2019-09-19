/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/mesh/qgsmeshcalculator.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsMeshCalculator
{
%Docstring
Performs mesh layer calculations.

Mesh calculator can do various mathematical operations
between dataset groups from a single mesh layer.
Resulting dataset group is added to the mesh layer.
Result can be filtered by extent or a vector layer mask
spatially and by selection of times.

Note: only dataset groups defined on vertices are
implemented and supported

.. versionadded:: 3.6
%End

%TypeHeaderCode
#include "qgsmeshcalculator.h"
%End
  public:

    enum Result
    {
      Success,
      Canceled,
      CreateOutputError,
      InputLayerError,
      ParserError,
      InvalidDatasets,
      EvaluateError,
      MemoryError,
    };

    QgsMeshCalculator( const QString &formulaString,
                       const QString &outputFile,
                       const QgsRectangle &outputExtent,
                       double startTime,
                       double endTime,
                       QgsMeshLayer *layer );
%Docstring
Creates calculator with bounding box (rectangular) mask

:param formulaString: formula/expression to evaluate. Consists of dataset group names, operators and numbers
:param outputFile: file to store the resulting dataset group data
:param outputExtent: spatial filter defined by rectangle
:param startTime: time filter defining the starting dataset
:param endTime: time filter defining the ending dataset
:param layer: mesh layer with dataset groups references in formulaString
%End

    QgsMeshCalculator( const QString &formulaString,
                       const QString &outputFile,
                       const QgsGeometry &outputMask,
                       double startTime,
                       double endTime,
                       QgsMeshLayer *layer );
%Docstring
Creates calculator with geometry mask

:param formulaString: formula/expression to evaluate. Consists of dataset group names, operators and numbers
:param outputFile: file to store the resulting dataset group data
:param outputMask: spatial filter defined by geometry
:param startTime: time filter defining the starting dataset
:param endTime: time filter defining the ending dataset
:param layer: mesh layer with dataset groups references in formulaString
%End

    Result processCalculation( QgsFeedback *feedback = 0 );
%Docstring
Starts the calculation, writes new dataset group to file and adds it to the mesh layer

:param feedback: The optional feedback argument for progress reporting and cancellation support

:return: QgsMeshCalculator.Success in case of success
%End

    static Result expression_valid( const QString &formulaString, QgsMeshLayer *layer );
%Docstring
Returns whether formula is valid for particular mesh layer

:param formulaString: formula/expression to evaluate. Consists of dataset group names, operators and numbers
:param layer: mesh layer with dataset groups references in formulaString

:return: QgsMeshCalculator.Success in case of success
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/mesh/qgsmeshcalculator.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
