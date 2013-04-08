class QgsSymbolsListWidget : QWidget
{
%TypeHeaderCode
#include <qgssymbolslistwidget.h>
%End
  public:
    QgsSymbolsListWidget( QgsSymbolV2* symbol, QgsStyleV2* style, QMenu* menu, QWidget* parent = NULL );

  public slots:
    void setSymbolFromStyle( const QModelIndex & index );
    void setSymbolColor( const QColor& color );
    void setMarkerAngle( double angle );
    void setMarkerSize( double size );
    void setLineWidth( double width );
    void addSymbolToStyle();
    void on_mSymbolUnitComboBox_currentIndexChanged( const QString & text );
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
