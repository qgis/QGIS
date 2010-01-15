

class QgsColorButton : QToolButton
{
%TypeHeaderCode
#include <qgscolorbutton.h>
%End

  public:
    QgsColorButton( QWidget *parent = 0 );
    ~QgsColorButton();

    void setColor( const QColor &color );
    QColor color() const;

  protected:
    void paintEvent( QPaintEvent *e );

};


class QgsColorButtonV2 : QPushButton
{
%TypeHeaderCode
#include <qgscolorbutton.h>
%End

  public:
    QgsColorButtonV2( QWidget* parent = 0 );
    QgsColorButtonV2( QString text, QWidget* parent = 0 );

    void setColor( const QColor &color );
    QColor color() const;
};
