class QgsDial: QDial
{
%TypeHeaderCode
#include <qgsdial.h>
%End

   QgsDial( QWidget *parent /TransferThis/ = 0 );

    void setMinimum( const QVariant &min );
    void setMaximum( const QVariant &max );
    void setSingleStep( const QVariant &step );
    void setValue( const QVariant &value );
    QVariant variantValue() const;

  signals:
    void valueChanged( const QVariant& );

  protected slots:
    void valueChanged( int );

  protected:
    virtual void paintEvent( QPaintEvent * event );
};
