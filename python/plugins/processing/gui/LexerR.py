# -*- coding: utf-8 -*-

"""
***************************************************************************
    LexerR.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4.QtGui import QColor
from PyQt4.Qsci import QsciLexerCustom


class LexerR(QsciLexerCustom):

    QUOTES = ["'", '"']
    SEPARATORS = ['(', '=', '.', '<']

    PRIMARY = ['break', 'else', 'F', 'FALSE', 'for', 'function', 'if', 'in',
               'Inf', 'NA', 'NaN', 'next', 'NULL', 'repeat', 'require',
               'return', 'source', 'T', 'TRUE', 'while'
               ]

    # PACKAGE = ["abbreviate", "abline", "abs", "acf", "acos", "acosh", "addmargins", "aggregate", "agrep", "alarm", "alias", "alist", "all", "anova", "any", "aov", "aperm", "append", "apply", "approx", "approxfun", "apropos", "ar", "args", "arima", "array", "arrows", "asin", "asinh", "assign", "assocplot", "atan", "atanh", "attach", "attr", "attributes", "autoload", "autoloader", "ave", "axis", "backsolve", "barplot", "basename", "beta", "bindtextdomain", "binomial", "biplot", "bitmap", "bmp", "body", "box", "boxplot", "bquote", "browser", "builtins", "bxp", "by", "bzfile", "c", "call", "cancor", "capabilities", "casefold", "cat", "category", "cbind", "ccf", "ceiling", "character", "charmatch", "chartr", "chol", "choose", "chull", "citation", "class", "close", "cm", "cmdscale", "codes", "coef", "coefficients", "col", "colnames", "colors", "colorspaces", "colours", "comment", "complex", "confint", "conflicts", "contour", "contrasts", "contributors", "convolve", "cophenetic", "coplot", "cor", "cos", "cosh", "cov", "covratio", "cpgram", "crossprod", "cummax", "cummin", "cumprod", "cumsum", "curve", "cut", "cutree", "cycle", "data", "dataentry", "date", "dbeta", "dbinom", "dcauchy", "dchisq", "de", "debug", "debugger", "decompose", "delay", "deltat", "demo", "dendrapply", "density", "deparse", "deriv", "det", "detach", "determinant", "deviance", "dexp", "df", "dfbeta", "dfbetas", "dffits", "dgamma", "dgeom", "dget", "dhyper", "diag", "diff", "diffinv", "difftime", "digamma", "dim", "dimnames", "dir", "dirname", "dist", "dlnorm", "dlogis", "dmultinom", "dnbinom", "dnorm", "dotchart", "double", "dpois", "dput", "drop", "dsignrank", "", "dt", "dump", "dunif", "duplicated", "dweibull", "dwilcox", "eapply", "ecdf", "edit", "effects", "eigen", "emacs", "embed", "end", "environment", "eval", "evalq", "example", "exists", "exp", "expression", "factanal", "factor", "factorial", "family", "fft", "fifo", "file", "filter", "find", "fitted", "fivenum", "fix", "floor", "flush", "for", "force", "formals", "format", "formula", "forwardsolve", "fourfoldplot", "frame", "frequency", "ftable", "gamma", "gaussian", "gc", "gcinfo", "gctorture", "get", "getenv", "geterrmessage", "gettext", "gettextf", "getwd", "gl", "glm", "globalenv", "gray", "grep", "grey", "grid", "gsub", "gzcon", "gzfile", "hat", "hatvalues", "hcl", "hclust", "head", "heatmap", "help", "hist", "history", "hsv", "httpclient", "iconv", "iconvlist", "identical", "identify", "if", "ifelse", "image", "influence", "inherits", "integer", "integrate", "interaction", "interactive", "intersect", "invisible", "isoreg", "jitter", "jpeg", "julian", "kappa", "kernapply", "kernel", "kmeans", "knots", "kronecker", "ksmooth", "labels", "lag", "lapply", "layout", "lbeta", "lchoose", "lcm", "legend", "length", "letters", "levels", "lfactorial", "lgamma", "library", "licence", "license", "line", "lines", "list", "lm", "load", "loadhistory", "loadings", "local", "locator", "loess", "log", "logb", "logical", "loglin", "lowess", "ls", "lsfit", "machine", "mad", "mahalanobis", "makepredictcall", "manova", "mapply", "match", "matlines", "matplot", "matpoints", "matrix", "max", "mean", "median", "medpolish", "menu", "merge", "message", "methods", "mget", "min", "missing", "mode", "monthplot", "months", "mosaicplot", "mtext", "mvfft", "names", "napredict", "naprint", "naresid", "nargs", "nchar", "ncol", "nextn", "ngettext", "nlevels", "nlm", "nls", "noquote", "nrow", "numeric", "objects", "offset", "open", "optim", "optimise", "optimize", "options", "order", "ordered", "outer", "pacf", "page", "pairlist", "pairs", "palette", "par", "parse", "paste", "pbeta", "pbinom", "pbirthday", "pcauchy", "pchisq", "pdf", "pentagamma", "person", "persp", "pexp", "pf", "pgamma", "pgeom", "phyper", "pi", "pico", "pictex", "pie", "piechart", "pipe", "plclust", "plnorm", "plogis", "plot", "pmatch", "pmax", "pmin", "pnbinom", "png", "pnorm", "points", "poisson", "poly", "polygon", "polym", "polyroot", "postscript", "power", "ppoints", "ppois", "ppr", "prcomp", "predict", "preplot", "pretty", "princomp", "print", "prmatrix", "prod", "profile", "profiler", "proj", "promax", "prompt", "provide", "psigamma", "psignrank", "pt", "ptukey", "punif", "pweibull", "pwilcox", "q", "qbeta", "qbinom", "qbirthday", "qcauchy", "qchisq", "qexp", "qf", "qgamma", "qgeom", "qhyper", "qlnorm", "qlogis", "qnbinom", "qnorm", "qpois", "qqline", "qqnorm", "qqplot", "qr", "qsignrank", "qt", "qtukey", "quantile", "quarters", "quasi", "quasibinomial", "quasipoisson", "quit", "qunif", "quote", "qweibull", "qwilcox", "rainbow", "range", "rank", "raw", "rbeta", "rbind", "rbinom", "rcauchy", "rchisq", "readline", "real", "recover", "rect", "reformulate", "regexpr", "relevel", "remove", "reorder", "rep", "replace", "replicate", "replications", "reshape", "resid", "residuals", "restart", "rev", "rexp", "rf", "rgamma", "rgb", "rgeom", "rhyper", "rle", "rlnorm", "rlogis", "rm", "rmultinom", "rnbinom", "rnorm", "round", "row", "rownames", "rowsum", "rpois", "rsignrank", "rstandard", "rstudent", "rt", "rug", "runif", "runmed", "rweibull", "rwilcox", "sample", "sapply", "save", "savehistory", "scale", "scan", "screen", "screeplot", "sd", "search", "searchpaths", "seek", "segments", "seq", "sequence", "serialize", "setdiff", "setequal", "setwd", "shell", "sign", "signif", "sin", "single", "sinh", "sink", "smooth", "solve", "sort", "spectrum", "spline", "splinefun", "split", "sprintf", "sqrt", "stack", "stars", "start", "stderr", "stdin", "stdout", "stem", "step", "stepfun", "stl", "stop", "stopifnot", "str", "strftime", "strheight", "stripchart", "strptime", "strsplit", "strtrim", "structure", "strwidth", "strwrap", "sub", "subset", "substitute", "substr", "substring", "sum", "summary", "sunflowerplot", "supsmu", "svd", "sweep", "switch", "symbols", "symnum", "system", "table", "tabulate", "tail", "tan", "tanh", "tapply", "tempdir", "tempfile", "termplot", "terms", "tetragamma", "text", "time", "title", "toeplitz", "tolower", "topenv", "toupper", "trace", "traceback", "transform", "trigamma", "trunc", "truncate", "try", "ts", "tsdiag", "tsp", "typeof", "unclass", "undebug", "union", "unique", "uniroot", "unix", "unlink", "unlist", "unname", "unserialize", "unsplit", "unstack", "untrace", "unz", "update", "upgrade", "url", "var", "varimax", "vcov", "vector", "version", "vi", "vignette", "warning", "warnings", "weekdays", "weights", "which", "window", "windows", "with", "write", "wsbrowser", "xedit", "xemacs", "xfig", "xinch", "xor", "xtabs", "xyinch", "yinch", "zapsmall"]
    # OTHER = ["acme", "aids", "aircondit", "amis", "aml", "banking", "barchart", "barley", "beaver", "bigcity", "boot", "brambles", "breslow", "bs", "bwplot", "calcium", "cane", "capability", "cav", "censboot", "channing", "city", "claridge", "cloth", "cloud", "coal", "condense", "contourplot", "control", "corr", "darwin", "densityplot", "dogs", "dotplot", "ducks", "empinf", "envelope", "environmental", "ethanol", "fir", "frets", "gpar", "grav", "gravity", "grob", "hirose", "histogram", "islay", "knn", "larrows", "levelplot", "llines", "logit", "lpoints", "lsegments", "lset", "ltext", "lvqinit", "lvqtest", "manaus", "melanoma", "melanoma", "motor", "multiedit", "neuro", "nitrofen", "nodal", "ns", "nuclear", "oneway", "parallel", "paulsen", "poisons", "polar", "qq", "qqmath", "remission", "rfs", "saddle", "salinity", "shingle", "simplex", "singer", "somgrid", "splom", "stripplot", "survival", "tau", "tmd", "tsboot", "tuna", "unit", "urine", "viewport", "wireframe", "wool", "xyplot"]

    def __init__(self, parent=None):
        QsciLexerCustom.__init__(self, parent)
        self._styles = {
            0: 'Default',
            1: 'Keyword',
            2: 'Comment',
            3: 'Number',
            4: 'String',
        }

        for (k, v) in self._styles.iteritems():
            setattr(self, v, k)

    def description(self, style):
        return self._styles.get(style, '')

    def defaultColor(self, style):
        if style == self.Default:
            return QColor('#2e3436')
        elif style == self.Keyword:
            return QColor('#204a87')
        elif style == self.Comment:
            return QColor('#c00')
        elif style == self.Number:
            return QColor('#4e9a06')
        elif style == self.String:
            return QColor('#ce5c00')

        return QsciLexerCustom.defaultColor(self, style)

    def styleText(self, start, end):
        editor = self.editor()
        if editor is None:
            return

        source = ''
        if end > editor.length():
            end = editor.length()
        if end > start:
            source = bytearray(end - start)
            editor.SendScintilla(editor.SCI_GETTEXTRANGE, start, end, source)

        if not source:
            return

        # The line index will also be needed to implement folding
        index = editor.SendScintilla(editor.SCI_LINEFROMPOSITION, start)
        if index > 0:
           # The previous state may be needed for multi-line styling
            pos = editor.SendScintilla(editor.SCI_GETLINEENDPOSITION, index
                                       - 1)
            state = editor.SendScintilla(editor.SCI_GETSTYLEAT, pos)
        else:
            state = self.Default

        set_style = self.setStyling
        self.startStyling(start, 0x1f)

        # Scintilla always asks to style whole lines
        for line in source.splitlines(True):
            length = len(line)

            if line.startswith('#'):
                state = self.Comment
            else:
                state = self.Default

            set_style(length, state)
