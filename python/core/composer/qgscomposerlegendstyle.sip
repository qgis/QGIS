class QgsComposerLegendStyle
{
%TypeHeaderCode
#include <qgscomposerlegendstyle.h>
%End
  public:
    enum Style
    {
      Undefined, // should not happen, only if corrupted project file
      Hidden, // special style, item is hidden includeing margins around
      Title,
      Group,
      Subgroup, // layer
      Symbol, // symbol without label
      SymbolLabel
    };
    enum Side // margin side
    {
      Top,
      Bottom,
      Left,
      Right,
    };
    QgsComposerLegendStyle();

    QFont font() const;
    QFont & rfont();
    void setFont( const QFont & font );

    double margin( Side side );
    void setMargin( Side side, double margin );

    // set all margins
    void setMargin( double margin );

    void writeXML( const QString& name, QDomElement& elem, QDomDocument & doc ) const;

    void readXML( const QDomElement& elem, const QDomDocument& doc );

    /** Get name for style, used in project file */
    static QString styleName( Style s );

    /** Get style from name, used in project file */
    static Style styleFromName( const QString& styleName );

    /** Get style label, translated, used in UI */
    static QString styleLabel( Style s );
};
