#ifndef DRW_TEXTCODEC_H
#define DRW_TEXTCODEC_H

#include <string>

class DRW_Converter;

class DRW_TextCodec
{
  public:
    DRW_TextCodec();
    ~DRW_TextCodec();
    std::string fromUtf8( std::string s );
    std::string toUtf8( std::string s );
    int getVersion() {return version;}
    void setVersion( std::string *v, bool dxfFormat );
    void setVersion( int v, bool dxfFormat );
    void setCodePage( std::string *c, bool dxfFormat );
    void setCodePage( std::string c, bool dxfFormat ) {setCodePage( &c, dxfFormat );}
    std::string getCodePage() {return cp;}

  private:
    std::string correctCodePage( const std::string &s );

  private:
    int version;
    std::string cp;
    DRW_Converter *conv = nullptr;
};

class DRW_Converter
{
  public:
    DRW_Converter( const int *t, int l )
    {
      table = t;
      cpLength = l;
    }
    virtual ~DRW_Converter() = default;
    virtual std::string fromUtf8( std::string *s ) {return *s;}
    virtual std::string toUtf8( std::string *s );
    std::string encodeText( std::string stmp );
    std::string decodeText( int c );
    std::string encodeNum( int c );
    int decodeNum( std::string s, int *b );
    const int *table = nullptr;
    int cpLength;
};

class DRW_ConvUTF16 : public DRW_Converter
{
  public:
    DRW_ConvUTF16(): DRW_Converter( nullptr, 0 ) {}
    virtual std::string fromUtf8( std::string *s );
    virtual std::string toUtf8( std::string *s );
};

class DRW_ConvTable : public DRW_Converter
{
  public:
    DRW_ConvTable( const int *t, int l ): DRW_Converter( t, l ) {}
    virtual std::string fromUtf8( std::string *s );
    virtual std::string toUtf8( std::string *s );
};

class DRW_ConvDBCSTable : public DRW_Converter
{
  public:
    DRW_ConvDBCSTable( const int *t,  const int *lt, const int dt[][2], int l ): DRW_Converter( t, l )
    {
      leadTable = lt;
      doubleTable = dt;
    }

    virtual std::string fromUtf8( std::string *s );
    virtual std::string toUtf8( std::string *s );
  private:
    const int *leadTable = nullptr;
    const int ( *doubleTable )[2];

};

class DRW_Conv932Table : public DRW_Converter
{
  public:
    DRW_Conv932Table( const int *t,  const int *lt, const int dt[][2], int l ): DRW_Converter( t, l )
    {
      leadTable = lt;
      doubleTable = dt;
    }

    virtual std::string fromUtf8( std::string *s );
    virtual std::string toUtf8( std::string *s );
  private:
    const int *leadTable = nullptr;
    const int ( *doubleTable )[2];

};

#endif // DRW_TEXTCODEC_H
