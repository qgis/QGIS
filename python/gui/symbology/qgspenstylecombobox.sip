/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgspenstylecombobox.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsPenStyleComboBox : QComboBox
{

%TypeHeaderCode
#include "qgspenstylecombobox.h"
%End
  public:
    QgsPenStyleComboBox( QWidget *parent /TransferThis/ = 0 );

    Qt::PenStyle penStyle() const;
%Docstring
 :rtype: Qt.PenStyle
%End

    void setPenStyle( Qt::PenStyle style );

  protected:
    QIcon iconForPen( Qt::PenStyle style );
%Docstring
 :rtype: QIcon
%End

};

class QgsPenJoinStyleComboBox : QComboBox
{

%TypeHeaderCode
#include "qgspenstylecombobox.h"
%End
  public:
    QgsPenJoinStyleComboBox( QWidget *parent /TransferThis/ = 0 );

    Qt::PenJoinStyle penJoinStyle() const;
%Docstring
 :rtype: Qt.PenJoinStyle
%End

    void setPenJoinStyle( Qt::PenJoinStyle style );
};

class QgsPenCapStyleComboBox : QComboBox
{

%TypeHeaderCode
#include "qgspenstylecombobox.h"
%End
  public:
    QgsPenCapStyleComboBox( QWidget *parent /TransferThis/ = 0 );

    Qt::PenCapStyle penCapStyle() const;
%Docstring
 :rtype: Qt.PenCapStyle
%End

    void setPenCapStyle( Qt::PenCapStyle style );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgspenstylecombobox.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
