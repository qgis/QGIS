/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewauxiliaryfielddialog.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsNewAuxiliaryFieldDialog: QDialog
{
%Docstring

 A dialog to create a new auxiliary field

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsnewauxiliaryfielddialog.h"
%End
  public:

    QgsNewAuxiliaryFieldDialog( const QgsPropertyDefinition &definition, QgsVectorLayer *layer, bool nameOnly = true, QWidget *parent = 0 );
%Docstring
 Constructor.

 \param definition The property definition to use to create the auxiliary field
 \param layer The vector layer for which the auxiliary layer has to be created
 \param nameOnly True to indicate that only the name widget is enabled
 \param parent Parent window
%End

    QgsPropertyDefinition propertyDefinition() const;
%Docstring
 Returns the underlying property definition.
 :rtype: QgsPropertyDefinition
%End

  protected:
    virtual void accept();


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewauxiliaryfielddialog.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
