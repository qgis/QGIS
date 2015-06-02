class QgsSymbolsListWidget : QWidget
{
%TypeHeaderCode
#include <qgssymbolslistwidget.h>
%End
  public:
    QgsSymbolsListWidget( QgsSymbolV2* symbol, QgsStyleV2* style, QMenu* menu, QWidget* parent /TransferThis/ = 0, const QgsVectorLayer * layer = 0 );

  public slots:
    void setSymbolFromStyle( const QModelIndex & index );
    void setSymbolColor( const QColor& color );
    void setMarkerAngle( double angle );
    void setMarkerSize( double size );
    void setLineWidth( double width );
    void addSymbolToStyle();
    void symbolAddedToStyle( QString name, QgsSymbolV2* symbol );
    void on_mSymbolUnitWidget_changed();
    void on_mTransparencySlider_valueChanged( int value );

    void on_groupsCombo_currentIndexChanged( int index );
    void on_groupsCombo_editTextChanged( const QString &text );

    void openStyleManager();

  signals:
    void changed();

  protected:
    void populateSymbolView();
    void populateSymbols( QStringList symbols );
    void updateSymbolColor();
    void updateSymbolInfo();
};
