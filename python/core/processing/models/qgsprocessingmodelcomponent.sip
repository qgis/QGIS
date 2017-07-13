/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/models/qgsprocessingmodelcomponent.h             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsProcessingModelComponent
{
%Docstring
 Represents a component of a model algorithm.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsprocessingmodelcomponent.h"
%End
  public:

    QString description() const;
%Docstring
 Returns the friendly description text for the component.
.. seealso:: setDescription()
 :rtype: str
%End

    void setDescription( const QString &description );
%Docstring
 Sets the friendly ``description`` text for the component.
.. seealso:: description()
%End

    QPointF position() const;
%Docstring
 Returns the position of the model component within the graphical modeler.
.. seealso:: setPosition()
 :rtype: QPointF
%End

    void setPosition( const QPointF &position );
%Docstring
 Sets the ``position`` of the model component within the graphical modeler.
.. seealso:: position()
%End

  protected:

    QgsProcessingModelComponent( const QString &description = QString() );
%Docstring
Only subclasses can be created
%End

    QgsProcessingModelComponent( const QgsProcessingModelComponent &other );
%Docstring
Copies are protected to avoid slicing
%End


    void saveCommonProperties( QVariantMap &map ) const;
%Docstring
 Saves the component properties to a QVariantMap.
.. seealso:: restoreCommonProperties()
%End

    void restoreCommonProperties( const QVariantMap &map );
%Docstring
 Restores the component properties from a QVariantMap.
.. seealso:: saveCommonProperties()
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/processing/models/qgsprocessingmodelcomponent.h             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
