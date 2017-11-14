/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsdetaileditemdata.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsDetailedItemData
{
%Docstring
 This class is the data only representation of a
 QgsDetailedItemWidget, designed to be used in custom views.
%End

%TypeHeaderCode
#include "qgsdetaileditemdata.h"
%End
  public:

    QgsDetailedItemData();
%Docstring
 Constructor for QgsDetailedItemData.
%End

    void setTitle( const QString &title );
    void setDetail( const QString &detail );
    void setCategory( const QString &category );
    void setIcon( const QPixmap &icon );
    void setCheckable( const bool flag );
    void setChecked( const bool flag );
    void setEnabled( bool flag );

    void setRenderAsWidget( bool flag );
%Docstring
 This is a hint to the delegate to render using
 a widget rather than manually painting every
 part of the list item.
.. note::

   the delegate may completely ignore this
 depending on the delegate implementation.
%End

    QString title() const;
%Docstring
 :rtype: str
%End
    QString detail() const;
%Docstring
 :rtype: str
%End
    QString category() const;
%Docstring
 :rtype: str
%End
    QPixmap icon() const;
%Docstring
 :rtype: QPixmap
%End
    bool isCheckable() const;
%Docstring
 :rtype: bool
%End
    bool isChecked() const;
%Docstring
 :rtype: bool
%End
    bool isEnabled() const;
%Docstring
 :rtype: bool
%End
    bool isRenderedAsWidget() const;
%Docstring
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsdetaileditemdata.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
