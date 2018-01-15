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
    if ( sipCpp->type() == QgsProcessingOutputVectorLayer::typeName() )
      sipType = sipType_QgsProcessingOutputVectorLayer;
    else if ( sipCpp->type() == QgsProcessingOutputRasterLayer::typeName() )
      sipType = sipType_QgsProcessingOutputRasterLayer;
    else if ( sipCpp->type() == QgsProcessingOutputMapLayer::typeName() )
      sipType = sipType_QgsProcessingOutputMapLayer;
    else if ( sipCpp->type() == QgsProcessingOutputHtml::typeName() )
      sipType = sipType_QgsProcessingOutputHtml;
    else if ( sipCpp->type() == QgsProcessingOutputNumber::typeName() )
      sipType = sipType_QgsProcessingOutputNumber;
    else if ( sipCpp->type() == QgsProcessingOutputString::typeName() )
      sipType = sipType_QgsProcessingOutputString;
    else if ( sipCpp->type() == QgsProcessingOutputFolder::typeName() )
      sipType = sipType_QgsProcessingOutputFolder;
    else if ( sipCpp->type() == QgsProcessingOutputFile::typeName() )
      sipType = sipType_QgsProcessingOutputFile;
    else
      sipType = nullptr;
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
%End

    QString name() const;
%Docstring
Returns the name of the output. This is the internal identifier by which
algorithms access this output.

.. seealso:: :py:func:`setName`
%End

    void setName( const QString &name );
%Docstring
Sets the ``name`` of the output. This is the internal identifier by which
algorithms access this output.

.. seealso:: :py:func:`name`
%End

    QString description() const;
%Docstring
Returns the description for the output. This is the user-visible string
used to identify this output.

.. seealso:: :py:func:`setDescription`
%End

    void setDescription( const QString &description );
%Docstring
Sets the ``description`` for the output. This is the user-visible string
used to identify this output.

.. seealso:: :py:func:`description`
%End

  protected:



};

typedef QList< const QgsProcessingOutputDefinition * > QgsProcessingOutputDefinitions;

class QgsProcessingOutputMapLayer : QgsProcessingOutputDefinition
{
%Docstring
A map layer output for processing algorithms, where layers may be either vector or raster.

If the actual layer output type is known (e.g. always vector or always raster), use
QgsProcessingOutputVectorLayer or QgsProcessingOutputRasterLayer instead.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingoutputs.h"
%End
  public:

    QgsProcessingOutputMapLayer( const QString &name, const QString &description = QString() );
%Docstring
Constructor for QgsProcessingOutputMapLayer.
%End

    static QString typeName();
%Docstring
Returns the type name for the output class.
%End

    virtual QString type() const;


};

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

    QgsProcessingOutputVectorLayer( const QString &name, const QString &description = QString(), QgsProcessing::SourceType type = QgsProcessing::TypeVectorAnyGeometry );
%Docstring
Constructor for QgsProcessingOutputVectorLayer.
%End

    static QString typeName();
%Docstring
Returns the type name for the output class.
%End
    virtual QString type() const;

    QgsProcessing::SourceType dataType() const;
%Docstring
Returns the layer type for the output layer.

.. seealso:: :py:func:`setDataType`
%End

    void setDataType( QgsProcessing::SourceType type );
%Docstring
Sets the layer ``type`` for the output layer.

.. seealso:: :py:func:`dataType`
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

    static QString typeName();
%Docstring
Returns the type name for the output class.
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

    static QString typeName();
%Docstring
Returns the type name for the output class.
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

    static QString typeName();
%Docstring
Returns the type name for the output class.
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

    static QString typeName();
%Docstring
Returns the type name for the output class.
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

    static QString typeName();
%Docstring
Returns the type name for the output class.
%End
    virtual QString type() const;

};

class QgsProcessingOutputFile : QgsProcessingOutputDefinition
{
%Docstring
A file output for processing algorithms.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingoutputs.h"
%End
  public:

    QgsProcessingOutputFile( const QString &name, const QString &description = QString() );
%Docstring
Constructor for QgsProcessingOutputFile.
%End

    static QString typeName();
%Docstring
Returns the type name for the output class.
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
