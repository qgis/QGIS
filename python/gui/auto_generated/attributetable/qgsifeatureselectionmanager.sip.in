/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsifeatureselectionmanager.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsIFeatureSelectionManager : QObject
{
%Docstring
Is an interface class to abstract feature selection handling.

e.g. QgsVectorLayer implements this interface to manage its selections.
%End

%TypeHeaderCode
#include "qgsifeatureselectionmanager.h"
%End
  public:
    QgsIFeatureSelectionManager( QObject *parent /TransferThis/ );
    virtual int selectedFeatureCount() = 0;
%Docstring
The number of features that are selected in this layer

:return: See description
%End

    virtual void select( const QgsFeatureIds &ids ) = 0;
%Docstring
Select features

:param ids:            Feature ids to select
%End

    virtual void deselect( const QgsFeatureIds &ids ) = 0;
%Docstring
Deselect features

:param ids:            Feature ids to deselect
%End

    virtual void setSelectedFeatures( const QgsFeatureIds &ids ) = 0;
%Docstring
Change selection to the new set of features. Dismisses the current selection.
Will emit the selectionChanged( const :py:class:`QgsFeatureIds`&, const :py:class:`QgsFeatureIds`&, bool ) signal with the
clearAndSelect flag set.

:param ids:   The ids which will be the new selection
%End

    virtual const QgsFeatureIds &selectedFeatureIds() const = 0;
%Docstring
Return reference to identifiers of selected features

:return: A list of QgsFeatureId's

.. seealso:: :py:func:`selectedFeatures`
%End

  signals:

    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, const bool clearAndSelect );
%Docstring
This signal is emitted when selection was changed

:param selected:        Newly selected feature ids
:param deselected:      Ids of all features which have previously been selected but are not any more
:param clearAndSelect:  In case this is set to true, the old selection was dismissed and the new selection corresponds to selected
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/attributetable/qgsifeatureselectionmanager.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
