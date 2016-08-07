class QgsGraduatedSymbolRendererWidget : QgsRendererWidget
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrendererwidget.h>
%End
  public:
    static QgsRendererWidget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer ) /Factory/;

    QgsGraduatedSymbolRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );
    ~QgsGraduatedSymbolRendererWidget();

    virtual QgsFeatureRenderer* renderer();

  public slots:
    void changeGraduatedSymbol();
    void graduatedColumnChanged( const QString& field );
    void classifyGraduated();
    void reapplyColorRamp();
    void reapplySizes();
    void rangesDoubleClicked( const QModelIndex & idx );
    void rangesClicked( const QModelIndex & idx );
    void changeCurrentValue( QStandardItem * item );

    /** Adds a class manually to the classification*/
    void addClass();
    /** Removes currently selected classes */
    void deleteClasses();
    /** Removes all classes from the classification*/
    void deleteAllClasses();
    /** Toggle the link between classes boundaries */
    void toggleBoundariesLink( bool linked );

    void sizeScaleFieldChanged( const QString& fldName );
    void scaleMethodChanged( QgsSymbol::ScaleMethod scaleMethod );
    void labelFormatChanged();

    void showSymbolLevels();

    void rowsMoved();
    void modelDataChanged();

    void on_mSizeUnitWidget_changed();
    void on_methodComboBox_currentIndexChanged( int );
    void refreshRanges( bool reset = false );

  protected:
    void updateUiFromRenderer( bool updateCount = true );
    void connectUpdateHandlers();
    void disconnectUpdateHandlers();
    bool rowsOrdered();

    void updateGraduatedSymbolIcon();

    //! return a list of indexes for the classes under selection
    QList<int> selectedClasses();
    QgsRangeList selectedRanges();

    void changeRangeSymbol( int rangeIdx );
    void changeRange( int rangeIdx );

    void changeSelectedSymbols();

    QList<QgsSymbol*> selectedSymbols();
    QgsSymbol* findSymbolForRange( double lowerBound, double upperBound, const QgsRangeList& ranges ) const;
    void refreshSymbolView();

    void keyPressEvent( QKeyEvent* event );
};
