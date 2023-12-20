//    Copyright (C) 2018-2023 Jakub Melka
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


#include "pdfparser.h"
#include "pdfconstants.h"
#include "pdfexception.h"

#include <QFile>
#include <QThread>
#include <QMetaEnum>

#include "pdfdbgheap.h"

#include <cctype>
#include <memory>

namespace pdf
{

PDFLexicalAnalyzer::PDFLexicalAnalyzer(const char* begin, const char* end) :
    m_begin(begin),
    m_current(begin),
    m_end(end),
    m_tokenizingPostScriptFunction(false)
{

}

PDFLexicalAnalyzer::Token PDFLexicalAnalyzer::fetch()
{
    // Skip whitespace/comments at first
    skipWhitespaceAndComments();

    // If we are at end of token, then return immediately
    if (isAtEnd())
    {
        return Token(TokenType::EndOfFile);
    }

    switch (lookChar())
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '-':
        case '.':
        {
            // Scan integer or real number. If integer overflows, then it is converted to the real number. If
            // real number overflow, then error is reported. This behaviour is according to the PDF 1.7 specification,
            // chapter 3.2.2.

            // First, treat special characters
            bool positive = fetchChar('+');
            bool negative = fetchChar('-');
            bool dot = fetchChar('.');
            bool treatAsReal = dot;
            bool atLeastOneDigit = false;

            if (isAtEnd())
            {
                error(tr("Expected a number, but end of stream reached."));
            }

            PDFInteger integer = 0;
            PDFReal real = 0.0;
            PDFReal scale = 0.1;

            // Now, we can only have digits and a single dot
            while (!isAtEnd())
            {
                if (!dot && fetchChar('.'))
                {
                    // Entering real mode
                    dot = true;
                    treatAsReal = true;
                    real = integer;
                }
                else if (std::isdigit(static_cast<unsigned char>(lookChar())))
                {
                    atLeastOneDigit = true;
                    PDFInteger digit = lookChar() - '0';
                    ++m_current;

                    if (!treatAsReal)
                    {
                        // Treat value as integer
                        integer = integer * 10 + digit;

                        // Check, if integer has not overflown, if yes, treat him as real
                        // according to the PDF 1.7 specification.
                        if (!isValidInteger(integer))
                        {
                            treatAsReal = true;
                            real = integer;
                        }
                    }
                    else
                    {
                        // Treat value as real
                        if (!dot)
                        {
                            real = real * 10.0 + digit;
                        }
                        else
                        {
                            real = real + scale * digit;
                            scale *= 0.1;
                        }
                    }
                }
                else if (isWhitespace(lookChar()) || isDelimiter(lookChar()))
                {
                    // Whitespace appeared - whitespaces/delimiters delimits tokens - break
                    break;
                }
                else
                {
                    // Another character other than dot and digit appeared - this is an error
                    error(tr("Invalid format of number. Character '%1' appeared.").arg(lookChar()));
                }
            }

            // Now, we have scanned whole token number, check for errors.
            if (positive && negative)
            {
                error(tr("Both '+' and '-' appeared in number. Invalid format of number."));
            }

            if (!atLeastOneDigit)
            {
                error(tr("Bad format of number - no digits appeared."));
            }

            // Check for real overflow
            if (treatAsReal && !std::isfinite(real))
            {
                error(tr("Real number overflow."));
            }

            if (negative)
            {
                integer = -integer;
                real = -real;
            }

            return !treatAsReal ? Token(TokenType::Integer, QVariant(static_cast<qint64>(integer))) : Token(TokenType::Real, real);
        }

        case CHAR_LEFT_BRACKET:
        {
            // String '(', sequence of literal characters enclosed in "()", see PDF 1.7 Reference,
            // chapter 3.2.3. Note: literal string can have properly balanced brackets inside.

            int parenthesisBalance = 1;
            QByteArray string;
            string.reserve(STRING_BUFFER_RESERVE);

            // Skip first character
            fetchChar();

            while (true)
            {
                // Scan string, see, what next char is.
                const char character = fetchChar();
                switch (character)
                {
                    case CHAR_LEFT_BRACKET:
                    {
                        ++parenthesisBalance;
                        string.push_back(character);
                        break;
                    }
                    case CHAR_RIGHT_BRACKET:
                    {
                        if (--parenthesisBalance == 0)
                        {
                            // We are done.
                            return Token(TokenType::String, string);
                        }
                        else
                        {
                            string.push_back(character);
                        }
                        break;
                    }

                    case CHAR_BACKSLASH:
                    {
                        // Escape sequence. Check, what it means. Possible values are in PDF 1.7 Reference,
                        // chapter 3.2.3, Table 3.2 - Escape Sequence in Literal Strings
                        const char escaped = fetchChar();
                        switch (escaped)
                        {
                            case 'n':
                            {
                                string += '\n';
                                break;
                            }
                            case 'r':
                            {
                                string += '\r';
                                break;
                            }
                            case 't':
                            {
                                string += '\t';
                                break;
                            }
                            case 'b':
                            {
                                string += '\b';
                                break;
                            }
                            case 'f':
                            {
                                string += '\f';
                                break;
                            }
                            case '\\':
                            case '(':
                            case ')':
                            {
                                string += escaped;
                                break;
                            }

                            case '\n':
                            {
                                // Nothing done here, EOL is not part of the string, because it was escaped
                                break;
                            }

                            case '\r':
                            {
                                // Skip EOL
                                fetchChar('\n');
                                break;
                            }

                            default:
                            {
                                // Undo fetch char, we do not want to miss first digit
                                --m_current;

                                // Try to scan octal value. Octal number can have 3 digits in this case.
                                // According to specification, overflow value can be truncated.
                                int octalNumber = -1;
                                if (fetchOctalNumber(3, &octalNumber))
                                {
                                    string += static_cast<char>(octalNumber);
                                }

                                // If it is not an octal number, then we silently ignore it.
                                // Documentation states that we should ignore the backslash
                                // character if it has other form than above.

                                break;
                            }
                        }

                        break;
                    }

                    default:
                    {
                        // Normal character
                        string.push_back(character);
                        break;
                    }
                }
            }

            // This code should be unreachable. Either normal string is scanned - then it is returned
            // in the while cycle above, or exception is thrown.
            Q_ASSERT(false);
            return Token(TokenType::EndOfFile);
        }

        case CHAR_SLASH:
        {
            // Name object. According to the PDF Reference 1.7, chapter 3.2.4 name object can have zero length,
            // and can contain #XX characters, where XX is hexadecimal number.

            fetchChar();

            QByteArray name;
            name.reserve(NAME_BUFFER_RESERVE);

            while (!isAtEnd())
            {
                if (fetchChar(CHAR_MARK))
                {
                    const char hexHighCharacter = fetchChar();
                    const char hexLowCharacter = fetchChar();

                    if (isHexCharacter(hexHighCharacter) && isHexCharacter(hexLowCharacter))
                    {
                        name += QByteArray::fromHex(QByteArray::fromRawData(m_current - 2, 2));
                    }
                    else
                    {
                        // Throw an error - hexadecimal number is expected.
                        error(tr("Hexadecimal number must follow character '#' in the name."));
                    }

                    continue;
                }

                // Now, we have other character, than '#', if it is a regular character,
                // then add it to the name, otherwise end scanning.
                const char character = lookChar();

                if (isRegular(character))
                {
                    name += character;
                    ++m_current;
                }
                else
                {
                    // Matched non-regular character - end of name.
                    break;
                }
            }

            return Token(TokenType::Name, std::move(name));
        }

        case CHAR_ARRAY_START:
        {
            ++m_current;
            return Token(TokenType::ArrayStart);
        }

        case CHAR_ARRAY_END:
        {
            ++m_current;
            return Token(TokenType::ArrayEnd);
        }

        case CHAR_LEFT_ANGLE:
        {
            ++m_current;

            // Check if it is dictionary start
            if (fetchChar(CHAR_LEFT_ANGLE))
            {
                return Token(TokenType::DictionaryStart);
            }
            else
            {
                // Reserve two times normal size, because in hexadecimal string, each character
                // is represented as a pair of hexadecimal numbers.
                QByteArray hexadecimalString;
                hexadecimalString.reserve(STRING_BUFFER_RESERVE * 2);

                // Scan hexadecimal string
                while (!isAtEnd())
                {
                    const char character = fetchChar();
                    if (isHexCharacter(character))
                    {
                        hexadecimalString += character;
                    }
                    else if (character == CHAR_RIGHT_ANGLE)
                    {
                        // End of string mark. According to the specification, string can contain odd number
                        // of hexadecimal digits, in this case, zero is appended to the string.
                        if (hexadecimalString.size() % 2 == 1)
                        {
                            hexadecimalString += '0';
                        }

                        QByteArray decodedString = QByteArray::fromHex(hexadecimalString);
                        return Token(TokenType::String, std::move(decodedString));
                    }
                    else if (isWhitespace(character))
                    {
                        // Do nothing, whitespace character should be ignored
                        // according to the specification.
                    }
                    else
                    {
                        // This is unexpected. Invalid character in hexadecimal string.
                        error(tr("Invalid character in hexadecimal string."));
                    }
                }

                error(tr("Unexpected end of stream reached while scanning hexadecimal string."));
            }
            break;
        }

        case CHAR_RIGHT_ANGLE:
        {
            // This must be a mark of dictionary end, because in other way, we should reach end of
            // string in the code above.
            ++m_current;

            if (fetchChar(CHAR_RIGHT_ANGLE))
            {
                return Token(TokenType::DictionaryEnd);
            }

            error(tr("Invalid character '%1'").arg(CHAR_RIGHT_ANGLE));
            break;
        }

        default:
        {
            // Now, we have skipped whitespaces. So actual character must be either regular, or it is special.
            // We have treated all special characters above. For this reason, if we match special character,
            // then we report an error.
            Q_ASSERT(!isWhitespace(lookChar()));

            if (isRegular(lookChar()))
            {
                // It should be sequence of regular characters - command, true, false, null...
                QByteArray command;
                command.reserve(COMMAND_BUFFER_RESERVE);

                while (!isAtEnd() && isRegular(lookChar()))
                {
                    command += fetchChar();
                }

                if (command == BOOL_OBJECT_TRUE_STRING)
                {
                    return Token(TokenType::Boolean, true);
                }
                else if (command == BOOL_OBJECT_FALSE_STRING)
                {
                    return Token(TokenType::Boolean, false);
                }
                else if (command == NULL_OBJECT_STRING)
                {
                    return Token(TokenType::Null);
                }
                else
                {
                    return Token(TokenType::Command, std::move(command));
                }
            }
            else if (m_tokenizingPostScriptFunction)
            {
                const char currentChar = lookChar();
                if (currentChar == CHAR_LEFT_CURLY_BRACKET || currentChar == CHAR_RIGHT_CURLY_BRACKET)
                {
                    return Token(TokenType::Command, QByteArray(1, fetchChar()));
                }

                error(tr("Unexpected character '%1' in the stream.").arg(currentChar));
            }
            else
            {
                error(tr("Unexpected character '%1' in the stream.").arg(lookChar()));
            }
            break;
        }
    }

    return Token(TokenType::EndOfFile);
}

void PDFLexicalAnalyzer::seek(PDFInteger offset)
{
    const PDFInteger limit = std::distance(m_begin, m_end);
    if (offset >= 0 && offset <= limit)
    {
        m_current = std::next(m_begin, offset);
    }
    else
    {
        error(tr("Trying to seek stream position to %1 bytes from the start, byte offset is invalid.").arg(offset));
    }
}

void PDFLexicalAnalyzer::skipWhitespaceAndComments()
{
    bool isComment = false;

    while (m_current != m_end)
    {
        if (isComment)
        {
            // Comment ends at end of line
            if (*m_current == CHAR_CARRIAGE_RETURN || *m_current == CHAR_LINE_FEED)
            {
                isComment = false;
            }

            // Commented character - step to the next character
            ++m_current;
        }
        else if (*m_current == CHAR_PERCENT)
        {
            isComment = true;
            ++m_current;
        }
        else if (isWhitespace(*m_current))
        {
            ++m_current;
        }
        else
        {
            // Not a whitespace and not in comment
            break;
        }
    }
}

void PDFLexicalAnalyzer::skipStreamStart()
{
    // According to the PDF Reference 1.7, chapter 3.2.7, after the 'stream' keyword,
    // either carriage return + line feed, or just line feed can appear. Eat them.
    fetchChar(CHAR_CARRIAGE_RETURN);
    fetchChar(CHAR_LINE_FEED);
}

QByteArray PDFLexicalAnalyzer::fetchByteArray(PDFInteger length)
{
    Q_ASSERT(length >= 0);

    if (std::distance(m_current, m_end) < length)
    {
        error(tr("Can't read %1 bytes from the input stream. Input stream end reached.").arg(length));
    }

    QByteArray result(m_current, length);
    std::advance(m_current, length);
    return result;
}

PDFInteger PDFLexicalAnalyzer::findSubstring(const char* str, PDFInteger position) const
{
    const PDFInteger length = std::distance(m_begin, m_end);
    if (position < 0 || position >= length)
    {
        return -1;
    }

    const PDFInteger substringLength = qstrlen(str);
    const PDFInteger startPos = position;
    const PDFInteger endPos = length - substringLength;
    for (PDFInteger i = startPos; i <= endPos; ++i)
    {
        Q_ASSERT(std::distance(m_begin + i + substringLength - 1, m_end) >= 0);
        if (memcmp(m_begin + i, str, substringLength) == 0)
        {
            return i;
        }
    }

    return -1;
}

QString PDFLexicalAnalyzer::getStringFromOperandType(TokenType type)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<TokenType>();
    Q_ASSERT(metaEnum.isValid());

    const char* typeName = metaEnum.valueToKey(static_cast<int>(type));
    Q_ASSERT(typeName);

    return typeName;
}

bool PDFLexicalAnalyzer::fetchChar(const char character)
{
    if (!isAtEnd() && lookChar() == character)
    {
        ++m_current;
        return true;
    }

    return false;
}

char PDFLexicalAnalyzer::fetchChar()
{
    if (!isAtEnd())
    {
        return *m_current++;
    }

    error(tr("Unexpected end of stream reached."));

    return 0;
}

bool PDFLexicalAnalyzer::fetchOctalNumber(int maxDigits, int* output)
{
    Q_ASSERT(output);

    *output = 0;
    int fetchedNumbers = 0;

    while (!isAtEnd() && fetchedNumbers < maxDigits)
    {
        const char c = lookChar();
        if (c >= '0' && c <= '7')
        {
            // Valid octal characters
            const int number = c - '0';
            *output = *output * 8 + number;
            ++m_current;
            ++fetchedNumbers;
        }
        else
        {
            // Non-octal character reached
            break;
        }
    }

    return fetchedNumbers >= 1;
}

constexpr bool PDFLexicalAnalyzer::isHexCharacter(const char character)
{
    return (character >= '0' && character <= '9') || (character >= 'A' && character <= 'F') || (character >= 'a' && character <= 'f');
}

void PDFLexicalAnalyzer::error(const QString& message) const
{
    std::size_t distance = std::distance(m_begin, m_current);
    throw PDFException(tr("Error near position %1. %2").arg(distance).arg(message));
}

PDFObject PDFParsingContext::getObject(const PDFObject& object)
{
    if (object.isReference())
    {
        Q_ASSERT(m_objectFetcher);
        return m_objectFetcher(this, object.getReference());
    }

    return object;
}

void PDFParsingContext::beginParsingObject(PDFObjectReference reference)
{
    if (m_activeParsedObjectSet.search(reference))
    {
        throw PDFException(tr("Cyclical reference found while parsing object %1 %2.").arg(reference.objectNumber).arg(reference.generation));
    }
    else
    {
        m_activeParsedObjectSet.insert(reference);
    }
}

void PDFParsingContext::endParsingObject(PDFObjectReference reference)
{
    Q_ASSERT(m_activeParsedObjectSet.search(reference));
    m_activeParsedObjectSet.erase(reference);
}

PDFParser::PDFParser(const QByteArray& data, PDFParsingContext* context, Features features) :
    m_context(context),
    m_features(features),
    m_lexicalAnalyzer(data.constData(), data.constData() + data.size())
{
    m_lookAhead1 = fetch();
    m_lookAhead2 = fetch();
}

PDFParser::PDFParser(const char* begin, const char* end, PDFParsingContext* context, Features features) :
    m_context(context),
    m_features(features),
    m_lexicalAnalyzer(begin, end)
{
    m_lookAhead1 = fetch();
    m_lookAhead2 = fetch();
}

PDFParser::PDFParser(std::function<PDFLexicalAnalyzer::Token ()> tokenFetcher) :
    m_tokenFetcher(qMove(tokenFetcher)),
    m_context(nullptr),
    m_features(None),
    m_lexicalAnalyzer(nullptr, nullptr)
{
    m_lookAhead1 = fetch();
    m_lookAhead2 = fetch();
}

PDFObject PDFParser::getObject()
{
    switch (m_lookAhead1.type)
    {
        case PDFLexicalAnalyzer::TokenType::Boolean:
        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            Q_ASSERT(m_lookAhead1.data.typeId() == QMetaType::Bool);
#else
            Q_ASSERT(m_lookAhead1.data.type() == QVariant::Bool);
#endif
            const bool value = m_lookAhead1.data.toBool();
            shift();
            return PDFObject::createBool(value);
        }

        case PDFLexicalAnalyzer::TokenType::Integer:
        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            Q_ASSERT(m_lookAhead1.data.typeId() == QMetaType::LongLong);
#else
            Q_ASSERT(m_lookAhead1.data.type() == QVariant::LongLong);
#endif
            const PDFInteger value = m_lookAhead1.data.toLongLong();
            shift();

            // We must check, if we are reading reference. In this case,
            // actual value is integer and next value is command "R".
            if (m_lookAhead1.type == PDFLexicalAnalyzer::TokenType::Integer &&
                m_lookAhead2.type == PDFLexicalAnalyzer::TokenType::Command &&
                m_lookAhead2.data.toByteArray() == PDF_REFERENCE_COMMAND)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                Q_ASSERT(m_lookAhead1.data.typeId() == QMetaType::LongLong);
#else
                Q_ASSERT(m_lookAhead1.data.type() == QVariant::LongLong);
#endif
                const PDFInteger generation = m_lookAhead1.data.toLongLong();
                shift();
                shift();
                return PDFObject::createReference(PDFObjectReference(value, generation));
            }
            else
            {
                // Just normal integer
                return PDFObject::createInteger(value);
            }
        }

        case PDFLexicalAnalyzer::TokenType::Real:
        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            Q_ASSERT(m_lookAhead1.data.typeId() == QMetaType::Double);
#else
            Q_ASSERT(m_lookAhead1.data.type() == QVariant::Double);
#endif
            const PDFReal value = m_lookAhead1.data.toDouble();
            shift();
            return PDFObject::createReal(value);
        }

        case PDFLexicalAnalyzer::TokenType::String:
        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            Q_ASSERT(m_lookAhead1.data.typeId() == QMetaType::QByteArray);
#else
            Q_ASSERT(m_lookAhead1.data.type() == QVariant::ByteArray);
#endif
            QByteArray array = m_lookAhead1.data.toByteArray();
            array.shrink_to_fit();
            shift();
            return PDFObject::createString(std::move(array));
        }

        case PDFLexicalAnalyzer::TokenType::Name:
        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            Q_ASSERT(m_lookAhead1.data.typeId() == QMetaType::QByteArray);
#else
            Q_ASSERT(m_lookAhead1.data.type() == QVariant::ByteArray);
#endif
            QByteArray array = m_lookAhead1.data.toByteArray();
            array.shrink_to_fit();
            shift();
            return PDFObject::createName(std::move(array));
        }

        case PDFLexicalAnalyzer::TokenType::ArrayStart:
        {
            shift();

            // Create shared pointer to the array (if the exception is thrown, array
            // will be properly destroyed by the shared array destructor)
            std::shared_ptr<PDFObjectContent> arraySharedPointer = std::make_shared<PDFArray>();
            PDFArray* array = static_cast<PDFArray*>(arraySharedPointer.get());

            while (m_lookAhead1.type != PDFLexicalAnalyzer::TokenType::EndOfFile &&
                   m_lookAhead1.type != PDFLexicalAnalyzer::TokenType::ArrayEnd)
            {
                array->appendItem(getObject());
            }

            // Now, we have either end of file, or array end. If former appears, then
            // it is an error - error should be reported.
            if (m_lookAhead1.type == PDFLexicalAnalyzer::TokenType::EndOfFile)
            {
                error(tr("Stream ended inside array."));
            }
            else
            {
                shift();
                return PDFObject::createArray(std::move(arraySharedPointer));
            }
            return PDFObject::createNull();
        }
        case PDFLexicalAnalyzer::TokenType::DictionaryStart:
        {
            shift();

            // Start reading the dictionary. BEWARE! It can also be a stream. In this case,
            // we must load also the stream content.
            std::shared_ptr<PDFDictionary> dictionarySharedPointer = std::make_shared<PDFDictionary>();
            PDFDictionary* dictionary = dictionarySharedPointer.get();

            // Now, scan key/value pairs
            while (m_lookAhead1.type != PDFLexicalAnalyzer::TokenType::EndOfFile &&
                   m_lookAhead1.type != PDFLexicalAnalyzer::TokenType::DictionaryEnd)
            {
                // First value should be a key
                if (m_lookAhead1.type != PDFLexicalAnalyzer::TokenType::Name)
                {
                    error(tr("Dictionary key must be a name."));
                }

                QByteArray key = m_lookAhead1.data.toByteArray();
                shift();

                // Second value should be a value
                PDFObject object = getObject();

                dictionary->addEntry(PDFInplaceOrMemoryString(std::move(key)), std::move(object));
            }

            // Now, we should reach dictionary end. If it is not the case, then end of stream occured.
            if (m_lookAhead1.type != PDFLexicalAnalyzer::TokenType::DictionaryEnd)
            {
                error(tr("End of stream inside dictionary reached."));
            }

            // Is it a content stream?
            if (m_lookAhead2.type == PDFLexicalAnalyzer::TokenType::Command &&
                m_lookAhead2.data.toByteArray() == PDF_STREAM_START_COMMAND)
            {
                if (!m_features.testFlag(AllowStreams))
                {
                    error(tr("Streams are not allowed in this context."));
                }

                // Read stream content. According to the PDF Reference 1.7, chapter 3.2.7, stream
                // content can be placed in the file. If this is the case, then try to load file
                // content in the memory. But even in this case, stream content should be skipped.

                if (!dictionary->hasKey(PDF_STREAM_DICT_LENGTH))
                {
                    error(tr("Stream length is not specified."));
                }

                PDFObject lengthObject = m_context ? m_context->getObject(dictionary->get(PDF_STREAM_DICT_LENGTH)) : dictionary->get(PDF_STREAM_DICT_LENGTH);
                if (!lengthObject.isInt())
                {
                    error(tr("Bad value of stream length. It should be an integer number."));
                }
                PDFInteger length = lengthObject.getInteger();

                if (length < 0)
                {
                    error(tr("Length of the stream buffer is negative (%1). It must be a positive number.").arg(length));
                }

                // Skip the stream start, then fetch data of the stream
                m_lexicalAnalyzer.skipStreamStart();
                QByteArray buffer = m_lexicalAnalyzer.fetchByteArray(length);

                // According to the PDF Reference 1.7, chapter 3.2.7, stream content can also be specified
                // in the external file. If this is the case, then we must try to load the stream data
                // from the external file.
                if (dictionary->hasKey(PDF_STREAM_DICT_FILE_SPECIFICATION))
                {
                    PDFObject fileName = m_context ? m_context->getObject(dictionary->get(PDF_STREAM_DICT_FILE_SPECIFICATION)) : dictionary->get(PDF_STREAM_DICT_FILE_SPECIFICATION);

                    if (!fileName.isString())
                    {
                        error(tr("Stream data should be in external file, but invalid file name is specified."));
                    }

                    QFile streamDataFile(fileName.getString());
                    if (streamDataFile.open(QFile::ReadOnly))
                    {
                        buffer = streamDataFile.readAll();
                        streamDataFile.close();
                    }
                    else
                    {
                        error(tr("Can't open stream data stored in external file '%1'.").arg(QString(fileName.getString())));
                    }
                }

                // Refill lookahead tokens
                m_lookAhead1 = fetch();
                m_lookAhead2 = fetch();

                if (m_lookAhead1.type == PDFLexicalAnalyzer::TokenType::Command &&
                    m_lookAhead1.data.toByteArray() == PDF_STREAM_END_COMMAND)
                {
                    // Everything OK, just advance and return stream object
                    shift();
                    return PDFObject::createStream(std::make_shared<PDFStream>(std::move(*dictionary), std::move(buffer)));
                }
                else
                {
                    error(tr("End of stream should end in keyword 'endstream'."));
                }
            }
            else
            {
                // Just shift (eat dictionary end) and return dictionary
                shift();
                return PDFObject::createDictionary(std::move(dictionarySharedPointer));
            }
            return PDFObject::createNull();
        }

        case PDFLexicalAnalyzer::TokenType::Null:
        {
            shift();
            return PDFObject::createNull();
        }

        case PDFLexicalAnalyzer::TokenType::ArrayEnd:
        case PDFLexicalAnalyzer::TokenType::DictionaryEnd:
        case PDFLexicalAnalyzer::TokenType::Command:
        {
            error(tr("Cannot read object. Unexpected token appeared."));
            break;
        }

        case PDFLexicalAnalyzer::TokenType::EndOfFile:
        {
            error(tr("Cannot read object. End of stream reached."));
            break;
        }
    }

    // This code should be unreachable. All values should be handled in the switch above.
    Q_ASSERT(false);
    return PDFObject::createNull();
}

PDFObject PDFParser::getObject(PDFObjectReference reference)
{
    PDFParsingContext::PDFParsingContextGuard guard(m_context, reference);
    return getObject();
}

void PDFParser::error(const QString& message) const
{
    throw PDFException(message);
}

void PDFParser::seek(PDFInteger offset)
{
    m_lexicalAnalyzer.seek(offset);

    // We must read lookahead symbols, because we invalidated them
    m_lookAhead1 = fetch();
    m_lookAhead2 = fetch();
}

bool PDFParser::fetchCommand(const char* command)
{
    if (m_lookAhead1.type == PDFLexicalAnalyzer::TokenType::Command &&
        m_lookAhead1.data.toByteArray() == command)
    {
        shift();
        return true;
    }

    return false;
}

void PDFParser::shift()
{
    m_lookAhead1 = std::move(m_lookAhead2);
    m_lookAhead2 = fetch();
}

PDFLexicalAnalyzer::Token PDFParser::fetch()
{
    return m_tokenFetcher ? m_tokenFetcher() : m_lexicalAnalyzer.fetch();
}

}   // namespace pdf
