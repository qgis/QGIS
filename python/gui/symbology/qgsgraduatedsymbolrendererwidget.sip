/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsgraduatedsymbolrendererwidget.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsGraduatedSymbolRendererWidget : QgsRendererWidget
{

%TypeHeaderCode
#include "qgsgraduatedsymbolrendererwidget.h"
%End
  public:
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
 :rtype: QgsRendererWidget
%End

    QgsGraduatedSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsGraduatedSymbolRendererWidget();

    virtual QgsFeatureRenderer *renderer();

  public slots:
    void changeGraduatedSymbol();
    void graduatedColumnChanged( const QString &field );
    void classifyGraduated();
    void reapplyColorRamp();
    void reapplySizes();
    void rangesDoubleClicked( const QModelIndex &idx );
    void rangesClicked( const QModelIndex &idx );
    void changeCurrentValue( QStandardItem *item );

    void addClass();
%Docstring
Adds a class manually to the classification
%End
    void deleteClasses();
%Docstring
Removes currently selected classes
%End
    void deleteAllClasses();
%Docstring
Removes all classes from the classification
%End
    void toggleBoundariesLink( bool linked );
%Docstring
Toggle the link between classes boundaries
%End

    void labelFormatChanged();

    void showSymbolLevels();

    void rowsMoved();
    void modelDataChanged();
    void refreshRanges( bool reset = false );

  protected:
    void updateUiFromRenderer( bool updateCount = true );
    void connectUpdateHandlers();
    void disconnectUpdateHandlers();
    bool rowsOrdered();
%Docstring
 :rtype: bool
%End

    void updateGraduatedSymbolIcon();

    QList<int> selectedClasses();
%Docstring
return a list of indexes for the classes under selection
 :rtype: list of int
%End
    QgsRangeList selectedRanges();
%Docstring
 :rtype: QgsRangeList
%End

    void changeRangeSymbol( int rangeIdx );
    void changeRange( int rangeIdx );

    void changeSelectedSymbols();

    virtual QList<QgsSymbol *> selectedSymbols();

    QgsSymbol *findSymbolForRange( double lowerBound, double upperBound, const QgsRangeList &ranges ) const;
%Docstring
 :rtype: QgsSymbol
%End
    virtual void refreshSymbolView();


    virtual void keyPressEvent( QKeyEvent *event );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsgraduatedsymbolrendererwidget.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
