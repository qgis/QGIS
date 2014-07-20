// This is the SIP interface definition for QsciLexerCPP.
//
// Copyright (c) 2014 Riverbank Computing Limited <info@riverbankcomputing.com>
// 
// This file is part of QScintilla.
// 
// This file may be used under the terms of the GNU General Public
// License versions 2.0 or 3.0 as published by the Free Software
// Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
// included in the packaging of this file.  Alternatively you may (at
// your option) use any later version of the GNU General Public
// License if such license has been publicly approved by Riverbank
// Computing Limited (or its successors, if any) and the KDE Free Qt
// Foundation. In addition, as a special exception, Riverbank gives you
// certain additional rights. These rights are described in the Riverbank
// GPL Exception version 1.1, which can be found in the file
// GPL_EXCEPTION.txt in this package.
// 
// If you are unsure which license is appropriate for your use, please
// contact the sales department at sales@riverbankcomputing.com.
// 
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


class QsciLexerCPP : QsciLexer
{
%TypeHeaderCode
#include <Qsci/qscilexercpp.h>
%End

public:
    enum {
        Default,
        InactiveDefault,
        Comment,
        InactiveComment,
        CommentLine,
        InactiveCommentLine,
        CommentDoc,
        InactiveCommentDoc,
        Number,
        InactiveNumber,
        Keyword,
        InactiveKeyword,
        DoubleQuotedString,
        InactiveDoubleQuotedString,
        SingleQuotedString,
        InactiveSingleQuotedString,
        UUID,
        InactiveUUID,
        PreProcessor,
        InactivePreProcessor,
        Operator,
        InactiveOperator,
        Identifier,
        InactiveIdentifier,
        UnclosedString,
        InactiveUnclosedString,
        VerbatimString,
        InactiveVerbatimString,
        Regex,
        InactiveRegex,
        CommentLineDoc,
        InactiveCommentLineDoc,
        KeywordSet2,
        InactiveKeywordSet2,
        CommentDocKeyword,
        InactiveCommentDocKeyword,
        CommentDocKeywordError,
        InactiveCommentDocKeywordError,
        GlobalClass,
        InactiveGlobalClass,
        RawString,
        InactiveRawString,
        TripleQuotedVerbatimString,
        InactiveTripleQuotedVerbatimString,
        HashQuotedString,
        InactiveHashQuotedString,
        PreProcessorComment,
        InactivePreProcessorComment,
        PreProcessorCommentLineDoc,
        InactivePreProcessorCommentLineDoc,
    };

%If (Qsci_Qt4)
    QsciLexerCPP(QObject *parent /TransferThis/ = 0,
            bool caseInsensitiveKeywords = false);
%End
%If (!Qsci_Qt4)
    QsciLexerCPP(QObject *parent /TransferThis/ = 0, const char *name = 0,
            bool caseInsensitiveKeywords = false);
%End

    const char *language() const;
    const char *lexer() const;
    QColor defaultColor(int style) const;
    bool defaultEolFill(int style) const;
    QFont defaultFont(int style) const;
    QColor defaultPaper(int style) const;
    const char *keywords(int set) const;
    QString description(int style) const;
    const char *wordCharacters() const;

    void refreshProperties();
    bool foldAtElse() const;
    bool foldComments() const;
    bool foldCompact() const;
    bool foldPreprocessor() const;
    bool stylePreprocessor() const;
    void setDollarsAllowed(bool allowed);
    bool dollarsAllowed() const;
    void setHighlightTripleQuotedStrings(bool enable);
    bool highlightTripleQuotedStrings() const;
    void setHighlightHashQuotedStrings(bool enable);
    bool highlightHashQuotedStrings() const;

public slots:
    virtual void setFoldAtElse(bool fold);
    virtual void setFoldComments(bool fold);
    virtual void setFoldCompact(bool fold);
    virtual void setFoldPreprocessor(bool fold);
    virtual void setStylePreprocessor(bool style);

protected:
    bool readProperties(QSettings &qs, const QString &prefix);
    bool writeProperties(QSettings &qs, const QString &prefix) const;

private:
    QsciLexerCPP(const QsciLexerCPP &);
};
