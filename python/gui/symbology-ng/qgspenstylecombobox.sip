class QgsPenStyleComboBox : QComboBox
{
%TypeHeaderCode
#include <qgspenstylecombobox.h>
%End

  public:
    QgsPenStyleComboBox( QWidget* parent /TransferThis/ = NULL );

    Qt::PenStyle penStyle() const;

    void setPenStyle( Qt::PenStyle style );

  protected:
    QIcon iconForPen( Qt::PenStyle style );

};

class QgsPenJoinStyleComboBox : QComboBox
{
%TypeHeaderCode
#include <qgspenstylecombobox.h>
%End

  public:
    QgsPenJoinStyleComboBox( QWidget* parent /TransferThis/ = NULL );

    Qt::PenJoinStyle penJoinStyle() const;

    void setPenJoinStyle( Qt::PenJoinStyle style );
};

class QgsPenCapStyleComboBox : QComboBox
{
%TypeHeaderCode
#include <qgspenstylecombobox.h>
%End

  public:
    QgsPenCapStyleComboBox( QWidget* parent /TransferThis/ = NULL );

    Qt::PenCapStyle penCapStyle() const;

    void setPenCapStyle( Qt::PenCapStyle style );
};
