/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutcombobox.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutComboBox : QComboBox
{
%Docstring
The QgsLayoutComboBox class is a combo box which displays available layouts from a QgsLayoutManager.

.. versionadded:: 3.8
%End

%TypeHeaderCode
#include "qgslayoutcombobox.h"
%End
  public:

    explicit QgsLayoutComboBox( QWidget *parent /TransferThis/ = 0, QgsLayoutManager *manager = 0 );
%Docstring
QgsLayoutComboBox creates a combo box to display a list of items in a
layout ``manager``. The layouts can optionally be filtered by type.
%End

    void setLayoutManager( QgsLayoutManager *manager );
%Docstring
Sets the layout ``manager`` containing the layouts to list in the combo box.
%End

    QgsLayoutManagerProxyModel::Filters filters() const;
%Docstring
Returns the current filters used for filtering available layouts.

.. seealso:: :py:func:`setFilters`
%End

    void setFilters( QgsLayoutManagerProxyModel::Filters filters );
%Docstring
Sets the current ``filters`` used for filtering available layouts.

.. seealso:: :py:func:`filters`
%End

    void setAllowEmptyLayout( bool allowEmpty );
%Docstring
Sets whether an optional empty layout ("not set") option is present in the combobox.

.. seealso:: :py:func:`allowEmptyLayout`
%End

    bool allowEmptyLayout() const;
%Docstring
Returns ``True`` if the combobox includes the empty layout ("not set") choice.

.. seealso:: :py:func:`setAllowEmptyLayout`
%End

    QgsMasterLayoutInterface *currentLayout() const;
%Docstring
Returns the layout currently selected in the combo box.
%End

    QgsMasterLayoutInterface *layout( int index ) const;
%Docstring
Returns the layout at the specified ``index``.
%End

  public slots:

    void setCurrentLayout( QgsMasterLayoutInterface *layout );
%Docstring
Sets the currently selected ``layout`` in the combo box.
%End

  signals:

    void layoutChanged( QgsMasterLayoutInterface *layout );
%Docstring
Emitted whenever the currently selected layout changes
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutcombobox.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
