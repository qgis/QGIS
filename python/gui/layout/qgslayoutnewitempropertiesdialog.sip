/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutnewitempropertiesdialog.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutNewItemPropertiesDialog : QDialog
{
%Docstring
 A dialog for configuring properties like the size and position of new layout items.
%End

%TypeHeaderCode
#include "qgslayoutnewitempropertiesdialog.h"
%End
  public:

    QgsLayoutNewItemPropertiesDialog( QWidget *parent = 0, Qt::WindowFlags flags = 0 );


    void setInitialItemPosition( QPointF position );

    QgsLayoutPoint itemPosition() const;
%Docstring
 :rtype: QgsLayoutPoint
%End

    QgsLayoutSize itemSize() const;
%Docstring
 :rtype: QgsLayoutSize
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutnewitempropertiesdialog.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
