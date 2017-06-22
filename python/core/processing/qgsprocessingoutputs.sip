/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingoutputs.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProcessingOutputDefinition
{
%Docstring

 Base class for the definition of processing outputs.

 Output definitions encapsulate the properties regarding the outputs from algorithms, such
 as generated layers or calculated values.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingoutputs.h"
%End

%ConvertToSubClassCode
    if ( sipCpp->type() == "outputVector" )
      sipType = sipType_QgsProcessingOutputVectorLayer;
    else if ( sipCpp->type() == "outputRaster" )
      sipType = sipType_QgsProcessingOutputRasterLayer;
    else if ( sipCpp->type() == "outputHtml" )
      sipType = sipType_QgsProcessingOutputHtml;
    else if ( sipCpp->type() == "outputNumber" )
      sipType = sipType_QgsProcessingOutputNumber;
    else if ( sipCpp->type() == "outputString" )
      sipType = sipType_QgsProcessingOutputString;
    else if ( sipCpp->type() == "outputFolder" )
      sipType = sipType_QgsProcessingOutputFolder;
%End
  public:

    QgsProcessingOutputDefinition( const QString &name, const QString &description = QString() );
%Docstring
 Constructor for QgsProcessingOutputDefinition.
%End

    virtual ~QgsProcessingOutputDefinition();

    virtual QString type() const = 0;
%Docstring
 Unique output type name.
 :rtype: str
%End

    QString name() const;
%Docstring
 Returns the name of the output. This is the internal identifier by which
 algorithms access this output.
 @see setName()
 :rtype: str
%End

    void setName( const QString &name );
%Docstring
 Sets the ``name`` of the output. This is the internal identifier by which
 algorithms access this output.
 @see name()
%End

    QString description() const;
%Docstring
 Returns the description for the output. This is the user-visible string
 used to identify this output.
 @see setDescription()
 :rtype: str
%End

    void setDescription( const QString &description );
%Docstring
 Sets the ``description`` for the output. This is the user-visible string
 used to identify this output.
 @see description()
%End

  protected:



};

typedef QList< const QgsProcessingOutputDefinition * > QgsProcessingOutputDefinitions;

class QgsProcessingOutputVectorLayer : QgsProcessingOutputDefinition
{
%Docstring
 A vector layer output for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingoutputs.h"
%End
  public:

    QgsProcessingOutputVectorLayer( const QString &name, const QString &description = QString(), QgsProcessingParameterDefinition::LayerType type = QgsProcessingParameterDefinition::TypeVectorAny );
%Docstring
 Constructor for QgsProcessingOutputVectorLayer.
%End

    virtual QString type() const;

    QgsProcessingParameterDefinition::LayerType dataType() const;
%Docstring
 Returns the layer type for the output layer.
.. seealso:: setDataType()
 :rtype: QgsProcessingParameterDefinition.LayerType
%End

    void setDataType( QgsProcessingParameterDefinition::LayerType type );
%Docstring
 Sets the layer ``type`` for the output layer.
.. seealso:: dataType()
%End

};

class QgsProcessingOutputRasterLayer : QgsProcessingOutputDefinition
{
%Docstring
 A raster layer output for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingoutputs.h"
%End
  public:

    QgsProcessingOutputRasterLayer( const QString &name, const QString &description = QString() );
%Docstring
 Constructor for QgsProcessingOutputRasterLayer.
%End

    virtual QString type() const;
};

class QgsProcessingOutputHtml : QgsProcessingOutputDefinition
{
%Docstring
 A HTML file output for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingoutputs.h"
%End
  public:

    QgsProcessingOutputHtml( const QString &name, const QString &description = QString() );
%Docstring
 Constructor for QgsProcessingOutputHtml.
%End

    virtual QString type() const;
};

class QgsProcessingOutputNumber : QgsProcessingOutputDefinition
{
%Docstring
 A numeric output for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingoutputs.h"
%End
  public:

    QgsProcessingOutputNumber( const QString &name, const QString &description = QString() );
%Docstring
 Constructor for QgsProcessingOutputNumber.
%End

    virtual QString type() const;
};

class QgsProcessingOutputString : QgsProcessingOutputDefinition
{
%Docstring
 A string output for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingoutputs.h"
%End
  public:

    QgsProcessingOutputString( const QString &name, const QString &description = QString() );
%Docstring
 Constructor for QgsProcessingOutputString.
%End

    virtual QString type() const;
};

class QgsProcessingOutputFolder : QgsProcessingOutputDefinition
{
%Docstring
 A folder output for processing algorithms.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingoutputs.h"
%End
  public:

    QgsProcessingOutputFolder( const QString &name, const QString &description = QString() );
%Docstring
 Constructor for QgsProcessingOutputFolder.
%End

    virtual QString type() const;
};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/qgsprocessingoutputs.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
