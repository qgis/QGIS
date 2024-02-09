//    Copyright (C) 2018-2021 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.


#ifndef PDFPARSER_H
#define PDFPARSER_H

#include "pdfglobal.h"
#include "pdfobject.h"
#include "pdfflatmap.h"

#include <QVariant>
#include <QByteArray>

#include <set>
#include <functional>

namespace pdf
{

// Group of whitespace characters

constexpr const char CHAR_NULL              = 0x00;
constexpr const char CHAR_TAB               = 0x09;
constexpr const char CHAR_LINE_FEED         = 0x0A;
constexpr const char CHAR_FORM_FEED         = 0x0C;
constexpr const char CHAR_CARRIAGE_RETURN   = 0x0D;
constexpr const char CHAR_SPACE             = 0x20;

// According to specification, chapter 3.1, EOL marker is one of the following characters:
//  1) Either CHAR_CARRIAGE_RETURN, or CHAR_LINE_FEED,
//  2) CHAR_CARRIAGE_RETURN followed immediately by CHAR_LINE_FEED

// Group of delimiter characters

constexpr const char CHAR_LEFT_BRACKET          = '(';
constexpr const char CHAR_RIGHT_BRACKET         = ')';
constexpr const char CHAR_LEFT_ANGLE            = '<';
constexpr const char CHAR_RIGHT_ANGLE           = '>';
constexpr const char CHAR_ARRAY_START           = '[';
constexpr const char CHAR_ARRAY_END             = ']';
constexpr const char CHAR_LEFT_CURLY_BRACKET    = '{';
constexpr const char CHAR_RIGHT_CURLY_BRACKET   = '}';
constexpr const char CHAR_SLASH                 = '/';
constexpr const char CHAR_PERCENT               = '%';
constexpr const char CHAR_BACKSLASH             = '\\';
constexpr const char CHAR_MARK                  = '#';

// These constants reserves memory while reading string or name

constexpr const int STRING_BUFFER_RESERVE = 32;
constexpr const int NAME_BUFFER_RESERVE = 16;
constexpr const int COMMAND_BUFFER_RESERVE = 16;

// Special objects - bool, null object

constexpr const char* BOOL_OBJECT_TRUE_STRING = "true";
constexpr const char* BOOL_OBJECT_FALSE_STRING = "false";
constexpr const char* NULL_OBJECT_STRING = "null";

// Special commands
constexpr const char* PDF_REFERENCE_COMMAND = "R";
constexpr const char* PDF_STREAM_START_COMMAND = "stream";
constexpr const char* PDF_STREAM_END_COMMAND = "endstream";

class PDF4QTLIBCORESHARED_EXPORT PDFLexicalAnalyzer
{
    Q_GADGET
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFLexicalAnalyzer)

public:
    PDFLexicalAnalyzer(const char* begin, const char* end);

    enum class TokenType
    {
        Boolean,
        Integer,
        Real,
        String,
        Name,
        ArrayStart,
        ArrayEnd,
        DictionaryStart,
        DictionaryEnd,
        Null,
        Command,
        EndOfFile
    };

    Q_ENUM(TokenType)

    struct Token
    {
        explicit Token() : type(TokenType::EndOfFile) { }
        explicit Token(TokenType type) : type(type) { }
        explicit Token(TokenType type, QVariant data) : type(type), data(qMove(data)) { }

        Token(const Token&) = default;
        Token(Token&&) = default;

        Token& operator=(const Token&) = default;
        Token& operator=(Token&&) = default;

        bool operator==(const Token& other) const { return type == other.type && data == other.data; }

        TokenType type;
        QVariant data;
    };

    /// Fetches a new token from the input stream. If we are at end of the input
    /// stream, then EndOfFile token is returned.
    Token fetch();

    /// Seeks stream from the start. If stream cannot be seeked (position is invalid),
    /// then exception is thrown.
    void seek(PDFInteger offset);

    /// Skips whitespace and comments
    void skipWhitespaceAndComments();

    /// Skips stream start
    void skipStreamStart();

    /// Reads number of bytes from the buffer and creates a byte array from it.
    /// If end of stream appears before desired end byte, exception is thrown.
    /// \param length Length of the buffer
    QByteArray fetchByteArray(PDFInteger length);

    /// Returns, if whole stream was scanned
    inline bool isAtEnd() const { return m_current == m_end; }

    /// Returns current position in the stream
    inline PDFInteger pos() const { return std::distance(m_begin, m_current); }

    /// Finds substring in the content stream, search is started from position.
    /// If position is invalid, or substring is not found, -1 is returned.
    /// \param str String to be found
    /// \param position Position, from which the search is started
    PDFInteger findSubstring(const char* str, PDFInteger position) const;

    /// Switch parser mode for tokenizing PostScript function
    void setTokenizingPostScriptFunction() { m_tokenizingPostScriptFunction = true; }

    /// Returns true, if character is a whitespace character according to the PDF 1.7 specification
    /// \param character Character to be tested
    static constexpr bool isWhitespace(char character);

    /// Returns true, if character is a delimiter character according to the PDF 1.7 specification
    /// \param character Character to be tested
    static constexpr bool isDelimiter(char character);

    /// Returns true, if character is a regular character according to the PDF 1.7 specification
    /// \param character Character to be tested
    static constexpr bool isRegular(char character) { return !isWhitespace(character) && !isDelimiter(character); }

    /// Returns string from operand type
    /// \param type Token type
    static QString getStringFromOperandType(TokenType type);

private:
    inline char lookChar() const { Q_ASSERT(m_current != m_end); return *m_current; }

    /// If current char is equal to the argument, then move position by one character and return true.
    /// If not, then return false and current position will be unchanged.
    /// \param character Character to be fetched
    bool fetchChar(const char character);

    /// Forcefully fetches next char from the stream. If stream is at end, then exception is thrown.
    /// Current position will be advanced to the next one.
    char fetchChar();

    /// Tries to fetch octal number with minimum 1 digits and specified maximum number of digits.
    /// If octal number cannot be fetched, then false is returned, otherwise true is returned.
    /// Result number is stored in the pointer.
    /// \param maxDigits Maximum number of digits
    /// \param output Non-null pointer to the result number
    bool fetchOctalNumber(int maxDigits, int* output);

    /// Returns true, if charachter represents hexadecimal number, i.e. digit 0-9,
    /// or letter A-F, or small letter a-f.
    static constexpr bool isHexCharacter(const char character);

    /// Throws an error exception
    void error(const QString& message) const;

    const char* m_begin;
    const char* m_current;
    const char* m_end;
    bool m_tokenizingPostScriptFunction;
};

/// Parsing context. Used for example to detect cyclic reference errors.
class PDFParsingContext
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFParsingContext)

public:
    explicit PDFParsingContext(std::function<PDFObject(PDFParsingContext*, PDFObjectReference)> objectFetcher) :
        m_objectFetcher(std::move(objectFetcher))
    {

    }

    /// Guard guarding the cyclical references (by reference).
    class PDFParsingContextGuard
    {
    public:
        explicit inline PDFParsingContextGuard(PDFParsingContext* context, PDFObjectReference reference) :
            m_context(context),
            m_reference(reference)
        {
            m_context->beginParsingObject(m_reference);
        }

        inline ~PDFParsingContextGuard()
        {
            m_context->endParsingObject(m_reference);
        }

    private:
        PDFParsingContext* m_context;
        PDFObjectReference m_reference;
    };

    /// Guard guarding the cyclical references (by object).
    class PDFParsingContextObjectGuard
    {
    public:
        explicit inline PDFParsingContextObjectGuard(PDFParsingContext* context, const PDFObject* object) :
            m_context(context),
            m_object(object)
        {
            if (object->isReference())
            {
                m_context->beginParsingObject(object->getReference());
            }
        }

        inline ~PDFParsingContextObjectGuard()
        {
            if (m_object->isReference())
            {
                m_context->endParsingObject(m_object->getReference());
            }
        }

    private:
        PDFParsingContext* m_context;
        const PDFObject* m_object;
    };

    /// Returns dereferenced object, if object is a reference. If it is not a reference,
    /// then same object is returned.
    PDFObject getObject(const PDFObject& object);

private:
    void beginParsingObject(PDFObjectReference reference);
    void endParsingObject(PDFObjectReference reference);

    using KeySet = PDFFlatMap<PDFObjectReference, 2>;

    /// This function fetches object, if it is needed
    std::function<PDFObject(PDFParsingContext*, PDFObjectReference)> m_objectFetcher;

    /// Set containing objects currently being parsed.
    KeySet m_activeParsedObjectSet;
};

/// Class for parsing objects. Checks cyclical references. If
/// the object cannot be obtained from the stream, exception is thrown.
class PDF4QTLIBCORESHARED_EXPORT PDFParser
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFParser)

public:
    enum Feature
    {
        None            = 0x0000,
        AllowStreams    = 0x0001
    };

    Q_DECLARE_FLAGS(Features, Feature)

    explicit PDFParser(const QByteArray& data, PDFParsingContext* context, Features features);
    explicit PDFParser(const char* begin, const char* end, PDFParsingContext* context, Features features);
    explicit PDFParser(std::function<PDFLexicalAnalyzer::Token(void)> tokenFetcher);

    /// Fetches single object from the stream. Does not check
    /// cyclical references. If object cannot be fetched, then
    /// exception is thrown.
    PDFObject getObject();

    /// Fetches single object from the stream. Performs check for
    /// cyclical references. If object cannot be fetched, then
    /// exception is thrown.
    PDFObject getObject(PDFObjectReference reference);

    /// Throws an error exception
    void error(const QString& message) const;

    /// Seeks stream from the start. If stream cannot be seeked (position is invalid),
    /// then exception is thrown.
    void seek(PDFInteger offset);

    /// Returns currently scanned token
    const PDFLexicalAnalyzer::Token& lookahead() const { return m_lookAhead1; }

    /// If current token is a command with same string, then eat this command
    /// and return true. Otherwise do nothing and return false.
    /// \param command Command to be fetched
    bool fetchCommand(const char* command);

private:
    void shift();

    PDFLexicalAnalyzer::Token fetch();

    /// Functor for fetching tokens
    std::function<PDFLexicalAnalyzer::Token(void)> m_tokenFetcher;

    /// Parsing context (multiple parsers can share it)
    PDFParsingContext* m_context;

    /// Enabled features
    Features m_features;

    /// Lexical analyzer for scanning tokens
    PDFLexicalAnalyzer m_lexicalAnalyzer;

    PDFLexicalAnalyzer::Token m_lookAhead1;
    PDFLexicalAnalyzer::Token m_lookAhead2;
};

// Implementation

inline
constexpr bool PDFLexicalAnalyzer::isWhitespace(char character)
{
    switch (character)
    {
        case CHAR_NULL:
        case CHAR_TAB:
        case CHAR_LINE_FEED:
        case CHAR_FORM_FEED:
        case CHAR_CARRIAGE_RETURN:
        case CHAR_SPACE:
            return true;

        default:
            return false;
    }
}

inline
constexpr bool PDFLexicalAnalyzer::isDelimiter(char character)
{
    switch (character)
    {
        case CHAR_LEFT_BRACKET:
        case CHAR_RIGHT_BRACKET:
        case CHAR_LEFT_ANGLE:
        case CHAR_RIGHT_ANGLE:
        case CHAR_ARRAY_START:
        case CHAR_ARRAY_END:
        case CHAR_LEFT_CURLY_BRACKET:
        case CHAR_RIGHT_CURLY_BRACKET:
        case CHAR_SLASH:
        case CHAR_PERCENT:
            return true;

        default:
            return false;
    }
}

}   // namespace pdf

#endif // PDFPARSER_H
