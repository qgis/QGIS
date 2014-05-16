# -*- coding: utf-8 -*-
"""
    pygments.lexers.functional
    ~~~~~~~~~~~~~~~~~~~~~~~~~~

    Lexers for functional languages.

    :copyright: Copyright 2006-2013 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

import re

from pygments.lexer import Lexer, RegexLexer, bygroups, include, do_insertions
from pygments.token import Text, Comment, Operator, Keyword, Name, \
     String, Number, Punctuation, Literal, Generic, Error

__all__ = ['RacketLexer', 'SchemeLexer', 'CommonLispLexer', 'HaskellLexer',
           'LiterateHaskellLexer', 'SMLLexer', 'OcamlLexer', 'ErlangLexer',
           'ErlangShellLexer', 'OpaLexer', 'CoqLexer', 'NewLispLexer',
           'ElixirLexer', 'ElixirConsoleLexer', 'KokaLexer']


class RacketLexer(RegexLexer):
    """
    Lexer for `Racket <http://racket-lang.org/>`_ source code (formerly known as
    PLT Scheme).

    *New in Pygments 1.6.*
    """

    name = 'Racket'
    aliases = ['racket', 'rkt']
    filenames = ['*.rkt', '*.rktl']
    mimetypes = ['text/x-racket', 'application/x-racket']

    # From namespace-mapped-symbols
    keywords = [
        '#%app', '#%datum', '#%expression', '#%module-begin',
        '#%plain-app', '#%plain-lambda', '#%plain-module-begin',
        '#%provide', '#%require', '#%stratified-body', '#%top',
        '#%top-interaction', '#%variable-reference', '...', 'and', 'begin',
        'begin-for-syntax', 'begin0', 'case', 'case-lambda', 'cond',
        'datum->syntax-object', 'define', 'define-for-syntax',
        'define-struct', 'define-syntax', 'define-syntax-rule',
        'define-syntaxes', 'define-values', 'define-values-for-syntax',
        'delay', 'do', 'expand-path', 'fluid-let', 'hash-table-copy',
        'hash-table-count', 'hash-table-for-each', 'hash-table-get',
        'hash-table-iterate-first', 'hash-table-iterate-key',
        'hash-table-iterate-next', 'hash-table-iterate-value',
        'hash-table-map', 'hash-table-put!', 'hash-table-remove!',
        'hash-table?', 'if', 'lambda', 'let', 'let*', 'let*-values',
        'let-struct', 'let-syntax', 'let-syntaxes', 'let-values', 'let/cc',
        'let/ec', 'letrec', 'letrec-syntax', 'letrec-syntaxes',
        'letrec-syntaxes+values', 'letrec-values', 'list-immutable',
        'make-hash-table', 'make-immutable-hash-table', 'make-namespace',
        'module', 'module-identifier=?', 'module-label-identifier=?',
        'module-template-identifier=?', 'module-transformer-identifier=?',
        'namespace-transformer-require', 'or', 'parameterize',
        'parameterize*', 'parameterize-break', 'provide',
        'provide-for-label', 'provide-for-syntax', 'quasiquote',
        'quasisyntax', 'quasisyntax/loc', 'quote', 'quote-syntax',
        'quote-syntax/prune', 'require', 'require-for-label',
        'require-for-syntax', 'require-for-template', 'set!',
        'set!-values', 'syntax', 'syntax-case', 'syntax-case*',
        'syntax-id-rules', 'syntax-object->datum', 'syntax-rules',
        'syntax/loc', 'time', 'transcript-off', 'transcript-on', 'unless',
        'unquote', 'unquote-splicing', 'unsyntax', 'unsyntax-splicing',
        'when', 'with-continuation-mark', 'with-handlers',
        'with-handlers*', 'with-syntax', 'λ'
    ]

    # From namespace-mapped-symbols
    builtins = [
        '*', '+', '-', '/', '<', '<=', '=', '>', '>=',
        'abort-current-continuation', 'abs', 'absolute-path?', 'acos',
        'add1', 'alarm-evt', 'always-evt', 'andmap', 'angle', 'append',
        'apply', 'arithmetic-shift', 'arity-at-least',
        'arity-at-least-value', 'arity-at-least?', 'asin', 'assoc', 'assq',
        'assv', 'atan', 'banner', 'bitwise-and', 'bitwise-bit-field',
        'bitwise-bit-set?', 'bitwise-ior', 'bitwise-not', 'bitwise-xor',
        'boolean?', 'bound-identifier=?', 'box', 'box-immutable', 'box?',
        'break-enabled', 'break-thread', 'build-path',
        'build-path/convention-type', 'byte-pregexp', 'byte-pregexp?',
        'byte-ready?', 'byte-regexp', 'byte-regexp?', 'byte?', 'bytes',
        'bytes->immutable-bytes', 'bytes->list', 'bytes->path',
        'bytes->path-element', 'bytes->string/latin-1',
        'bytes->string/locale', 'bytes->string/utf-8', 'bytes-append',
        'bytes-close-converter', 'bytes-convert', 'bytes-convert-end',
        'bytes-converter?', 'bytes-copy', 'bytes-copy!', 'bytes-fill!',
        'bytes-length', 'bytes-open-converter', 'bytes-ref', 'bytes-set!',
        'bytes-utf-8-index', 'bytes-utf-8-length', 'bytes-utf-8-ref',
        'bytes<?', 'bytes=?', 'bytes>?', 'bytes?', 'caaaar', 'caaadr',
        'caaar', 'caadar', 'caaddr', 'caadr', 'caar', 'cadaar', 'cadadr',
        'cadar', 'caddar', 'cadddr', 'caddr', 'cadr',
        'call-in-nested-thread', 'call-with-break-parameterization',
        'call-with-composable-continuation',
        'call-with-continuation-barrier', 'call-with-continuation-prompt',
        'call-with-current-continuation', 'call-with-escape-continuation',
        'call-with-exception-handler',
        'call-with-immediate-continuation-mark', 'call-with-input-file',
        'call-with-output-file', 'call-with-parameterization',
        'call-with-semaphore', 'call-with-semaphore/enable-break',
        'call-with-values', 'call/cc', 'call/ec', 'car', 'cdaaar',
        'cdaadr', 'cdaar', 'cdadar', 'cdaddr', 'cdadr', 'cdar', 'cddaar',
        'cddadr', 'cddar', 'cdddar', 'cddddr', 'cdddr', 'cddr', 'cdr',
        'ceiling', 'channel-get', 'channel-put', 'channel-put-evt',
        'channel-try-get', 'channel?', 'chaperone-box', 'chaperone-evt',
        'chaperone-hash', 'chaperone-of?', 'chaperone-procedure',
        'chaperone-struct', 'chaperone-struct-type', 'chaperone-vector',
        'chaperone?', 'char->integer', 'char-alphabetic?', 'char-blank?',
        'char-ci<=?', 'char-ci<?', 'char-ci=?', 'char-ci>=?', 'char-ci>?',
        'char-downcase', 'char-foldcase', 'char-general-category',
        'char-graphic?', 'char-iso-control?', 'char-lower-case?',
        'char-numeric?', 'char-punctuation?', 'char-ready?',
        'char-symbolic?', 'char-title-case?', 'char-titlecase',
        'char-upcase', 'char-upper-case?', 'char-utf-8-length',
        'char-whitespace?', 'char<=?', 'char<?', 'char=?', 'char>=?',
        'char>?', 'char?', 'check-duplicate-identifier',
        'checked-procedure-check-and-extract', 'choice-evt',
        'cleanse-path', 'close-input-port', 'close-output-port',
        'collect-garbage', 'collection-file-path', 'collection-path',
        'compile', 'compile-allow-set!-undefined',
        'compile-context-preservation-enabled',
        'compile-enforce-module-constants', 'compile-syntax',
        'compiled-expression?', 'compiled-module-expression?',
        'complete-path?', 'complex?', 'cons',
        'continuation-mark-set->context', 'continuation-mark-set->list',
        'continuation-mark-set->list*', 'continuation-mark-set-first',
        'continuation-mark-set?', 'continuation-marks',
        'continuation-prompt-available?', 'continuation-prompt-tag?',
        'continuation?', 'copy-file', 'cos',
        'current-break-parameterization', 'current-code-inspector',
        'current-command-line-arguments', 'current-compile',
        'current-continuation-marks', 'current-custodian',
        'current-directory', 'current-drive', 'current-error-port',
        'current-eval', 'current-evt-pseudo-random-generator',
        'current-gc-milliseconds', 'current-get-interaction-input-port',
        'current-inexact-milliseconds', 'current-input-port',
        'current-inspector', 'current-library-collection-paths',
        'current-load', 'current-load-extension',
        'current-load-relative-directory', 'current-load/use-compiled',
        'current-locale', 'current-memory-use', 'current-milliseconds',
        'current-module-declare-name', 'current-module-declare-source',
        'current-module-name-resolver', 'current-namespace',
        'current-output-port', 'current-parameterization',
        'current-preserved-thread-cell-values', 'current-print',
        'current-process-milliseconds', 'current-prompt-read',
        'current-pseudo-random-generator', 'current-read-interaction',
        'current-reader-guard', 'current-readtable', 'current-seconds',
        'current-security-guard', 'current-subprocess-custodian-mode',
        'current-thread', 'current-thread-group',
        'current-thread-initial-stack-size',
        'current-write-relative-directory', 'custodian-box-value',
        'custodian-box?', 'custodian-limit-memory',
        'custodian-managed-list', 'custodian-memory-accounting-available?',
        'custodian-require-memory', 'custodian-shutdown-all', 'custodian?',
        'custom-print-quotable-accessor', 'custom-print-quotable?',
        'custom-write-accessor', 'custom-write?', 'date', 'date*',
        'date*-nanosecond', 'date*-time-zone-name', 'date*?', 'date-day',
        'date-dst?', 'date-hour', 'date-minute', 'date-month',
        'date-second', 'date-time-zone-offset', 'date-week-day',
        'date-year', 'date-year-day', 'date?', 'datum-intern-literal',
        'default-continuation-prompt-tag', 'delete-directory',
        'delete-file', 'denominator', 'directory-exists?',
        'directory-list', 'display', 'displayln', 'dump-memory-stats',
        'dynamic-require', 'dynamic-require-for-syntax', 'dynamic-wind',
        'eof', 'eof-object?', 'ephemeron-value', 'ephemeron?', 'eprintf',
        'eq-hash-code', 'eq?', 'equal-hash-code',
        'equal-secondary-hash-code', 'equal?', 'equal?/recur',
        'eqv-hash-code', 'eqv?', 'error', 'error-display-handler',
        'error-escape-handler', 'error-print-context-length',
        'error-print-source-location', 'error-print-width',
        'error-value->string-handler', 'eval', 'eval-jit-enabled',
        'eval-syntax', 'even?', 'evt?', 'exact->inexact', 'exact-integer?',
        'exact-nonnegative-integer?', 'exact-positive-integer?', 'exact?',
        'executable-yield-handler', 'exit', 'exit-handler', 'exn',
        'exn-continuation-marks', 'exn-message', 'exn:break',
        'exn:break-continuation', 'exn:break?', 'exn:fail',
        'exn:fail:contract', 'exn:fail:contract:arity',
        'exn:fail:contract:arity?', 'exn:fail:contract:continuation',
        'exn:fail:contract:continuation?',
        'exn:fail:contract:divide-by-zero',
        'exn:fail:contract:divide-by-zero?',
        'exn:fail:contract:non-fixnum-result',
        'exn:fail:contract:non-fixnum-result?',
        'exn:fail:contract:variable', 'exn:fail:contract:variable-id',
        'exn:fail:contract:variable?', 'exn:fail:contract?',
        'exn:fail:filesystem', 'exn:fail:filesystem:exists',
        'exn:fail:filesystem:exists?', 'exn:fail:filesystem:version',
        'exn:fail:filesystem:version?', 'exn:fail:filesystem?',
        'exn:fail:network', 'exn:fail:network?', 'exn:fail:out-of-memory',
        'exn:fail:out-of-memory?', 'exn:fail:read',
        'exn:fail:read-srclocs', 'exn:fail:read:eof', 'exn:fail:read:eof?',
        'exn:fail:read:non-char', 'exn:fail:read:non-char?',
        'exn:fail:read?', 'exn:fail:syntax', 'exn:fail:syntax-exprs',
        'exn:fail:syntax:unbound', 'exn:fail:syntax:unbound?',
        'exn:fail:syntax?', 'exn:fail:unsupported',
        'exn:fail:unsupported?', 'exn:fail:user', 'exn:fail:user?',
        'exn:fail?', 'exn:srclocs-accessor', 'exn:srclocs?', 'exn?', 'exp',
        'expand', 'expand-once', 'expand-syntax', 'expand-syntax-once',
        'expand-syntax-to-top-form', 'expand-to-top-form',
        'expand-user-path', 'expt', 'file-exists?',
        'file-or-directory-identity', 'file-or-directory-modify-seconds',
        'file-or-directory-permissions', 'file-position', 'file-size',
        'file-stream-buffer-mode', 'file-stream-port?',
        'filesystem-root-list', 'find-executable-path',
        'find-library-collection-paths', 'find-system-path', 'fixnum?',
        'floating-point-bytes->real', 'flonum?', 'floor', 'flush-output',
        'for-each', 'force', 'format', 'fprintf', 'free-identifier=?',
        'gcd', 'generate-temporaries', 'gensym', 'get-output-bytes',
        'get-output-string', 'getenv', 'global-port-print-handler',
        'guard-evt', 'handle-evt', 'handle-evt?', 'hash', 'hash-equal?',
        'hash-eqv?', 'hash-has-key?', 'hash-placeholder?', 'hash-ref!',
        'hasheq', 'hasheqv', 'identifier-binding',
        'identifier-label-binding', 'identifier-prune-lexical-context',
        'identifier-prune-to-source-module',
        'identifier-remove-from-definition-context',
        'identifier-template-binding', 'identifier-transformer-binding',
        'identifier?', 'imag-part', 'immutable?', 'impersonate-box',
        'impersonate-hash', 'impersonate-procedure', 'impersonate-struct',
        'impersonate-vector', 'impersonator-of?',
        'impersonator-prop:application-mark',
        'impersonator-property-accessor-procedure?',
        'impersonator-property?', 'impersonator?', 'inexact->exact',
        'inexact-real?', 'inexact?', 'input-port?', 'inspector?',
        'integer->char', 'integer->integer-bytes',
        'integer-bytes->integer', 'integer-length', 'integer-sqrt',
        'integer-sqrt/remainder', 'integer?',
        'internal-definition-context-seal', 'internal-definition-context?',
        'keyword->string', 'keyword<?', 'keyword?', 'kill-thread', 'lcm',
        'length', 'liberal-define-context?', 'link-exists?', 'list',
        'list*', 'list->bytes', 'list->string', 'list->vector', 'list-ref',
        'list-tail', 'list?', 'load', 'load-extension',
        'load-on-demand-enabled', 'load-relative',
        'load-relative-extension', 'load/cd', 'load/use-compiled',
        'local-expand', 'local-expand/capture-lifts',
        'local-transformer-expand',
        'local-transformer-expand/capture-lifts', 'locale-string-encoding',
        'log', 'magnitude', 'make-arity-at-least', 'make-bytes',
        'make-channel', 'make-continuation-prompt-tag', 'make-custodian',
        'make-custodian-box', 'make-date', 'make-date*',
        'make-derived-parameter', 'make-directory', 'make-ephemeron',
        'make-exn', 'make-exn:break', 'make-exn:fail',
        'make-exn:fail:contract', 'make-exn:fail:contract:arity',
        'make-exn:fail:contract:continuation',
        'make-exn:fail:contract:divide-by-zero',
        'make-exn:fail:contract:non-fixnum-result',
        'make-exn:fail:contract:variable', 'make-exn:fail:filesystem',
        'make-exn:fail:filesystem:exists',
        'make-exn:fail:filesystem:version', 'make-exn:fail:network',
        'make-exn:fail:out-of-memory', 'make-exn:fail:read',
        'make-exn:fail:read:eof', 'make-exn:fail:read:non-char',
        'make-exn:fail:syntax', 'make-exn:fail:syntax:unbound',
        'make-exn:fail:unsupported', 'make-exn:fail:user',
        'make-file-or-directory-link', 'make-hash-placeholder',
        'make-hasheq-placeholder', 'make-hasheqv',
        'make-hasheqv-placeholder', 'make-immutable-hasheqv',
        'make-impersonator-property', 'make-input-port', 'make-inspector',
        'make-known-char-range-list', 'make-output-port', 'make-parameter',
        'make-pipe', 'make-placeholder', 'make-polar',
        'make-prefab-struct', 'make-pseudo-random-generator',
        'make-reader-graph', 'make-readtable', 'make-rectangular',
        'make-rename-transformer', 'make-resolved-module-path',
        'make-security-guard', 'make-semaphore', 'make-set!-transformer',
        'make-shared-bytes', 'make-sibling-inspector',
        'make-special-comment', 'make-srcloc', 'make-string',
        'make-struct-field-accessor', 'make-struct-field-mutator',
        'make-struct-type', 'make-struct-type-property',
        'make-syntax-delta-introducer', 'make-syntax-introducer',
        'make-thread-cell', 'make-thread-group', 'make-vector',
        'make-weak-box', 'make-weak-hasheqv', 'make-will-executor', 'map',
        'max', 'mcar', 'mcdr', 'mcons', 'member', 'memq', 'memv', 'min',
        'module->exports', 'module->imports', 'module->language-info',
        'module->namespace', 'module-compiled-exports',
        'module-compiled-imports', 'module-compiled-language-info',
        'module-compiled-name', 'module-path-index-join',
        'module-path-index-resolve', 'module-path-index-split',
        'module-path-index?', 'module-path?', 'module-predefined?',
        'module-provide-protected?', 'modulo', 'mpair?', 'nack-guard-evt',
        'namespace-attach-module', 'namespace-attach-module-declaration',
        'namespace-base-phase', 'namespace-mapped-symbols',
        'namespace-module-identifier', 'namespace-module-registry',
        'namespace-require', 'namespace-require/constant',
        'namespace-require/copy', 'namespace-require/expansion-time',
        'namespace-set-variable-value!', 'namespace-symbol->identifier',
        'namespace-syntax-introduce', 'namespace-undefine-variable!',
        'namespace-unprotect-module', 'namespace-variable-value',
        'namespace?', 'negative?', 'never-evt', 'newline',
        'normal-case-path', 'not', 'null', 'null?', 'number->string',
        'number?', 'numerator', 'object-name', 'odd?', 'open-input-bytes',
        'open-input-file', 'open-input-output-file', 'open-input-string',
        'open-output-bytes', 'open-output-file', 'open-output-string',
        'ormap', 'output-port?', 'pair?', 'parameter-procedure=?',
        'parameter?', 'parameterization?', 'path->bytes',
        'path->complete-path', 'path->directory-path', 'path->string',
        'path-add-suffix', 'path-convention-type', 'path-element->bytes',
        'path-element->string', 'path-for-some-system?',
        'path-list-string->path-list', 'path-replace-suffix',
        'path-string?', 'path?', 'peek-byte', 'peek-byte-or-special',
        'peek-bytes', 'peek-bytes!', 'peek-bytes-avail!',
        'peek-bytes-avail!*', 'peek-bytes-avail!/enable-break',
        'peek-char', 'peek-char-or-special', 'peek-string', 'peek-string!',
        'pipe-content-length', 'placeholder-get', 'placeholder-set!',
        'placeholder?', 'poll-guard-evt', 'port-closed-evt',
        'port-closed?', 'port-commit-peeked', 'port-count-lines!',
        'port-count-lines-enabled', 'port-display-handler',
        'port-file-identity', 'port-file-unlock', 'port-next-location',
        'port-print-handler', 'port-progress-evt',
        'port-provides-progress-evts?', 'port-read-handler',
        'port-try-file-lock?', 'port-write-handler', 'port-writes-atomic?',
        'port-writes-special?', 'port?', 'positive?',
        'prefab-key->struct-type', 'prefab-struct-key', 'pregexp',
        'pregexp?', 'primitive-closure?', 'primitive-result-arity',
        'primitive?', 'print', 'print-as-expression',
        'print-boolean-long-form', 'print-box', 'print-graph',
        'print-hash-table', 'print-mpair-curly-braces',
        'print-pair-curly-braces', 'print-reader-abbreviations',
        'print-struct', 'print-syntax-width', 'print-unreadable',
        'print-vector-length', 'printf', 'procedure->method',
        'procedure-arity', 'procedure-arity-includes?', 'procedure-arity?',
        'procedure-closure-contents-eq?', 'procedure-extract-target',
        'procedure-reduce-arity', 'procedure-rename',
        'procedure-struct-type?', 'procedure?', 'promise?',
        'prop:arity-string', 'prop:checked-procedure',
        'prop:custom-print-quotable', 'prop:custom-write',
        'prop:equal+hash', 'prop:evt', 'prop:exn:srclocs',
        'prop:impersonator-of', 'prop:input-port',
        'prop:liberal-define-context', 'prop:output-port',
        'prop:procedure', 'prop:rename-transformer',
        'prop:set!-transformer', 'pseudo-random-generator->vector',
        'pseudo-random-generator-vector?', 'pseudo-random-generator?',
        'putenv', 'quotient', 'quotient/remainder', 'raise',
        'raise-arity-error', 'raise-mismatch-error', 'raise-syntax-error',
        'raise-type-error', 'raise-user-error', 'random', 'random-seed',
        'rational?', 'rationalize', 'read', 'read-accept-bar-quote',
        'read-accept-box', 'read-accept-compiled', 'read-accept-dot',
        'read-accept-graph', 'read-accept-infix-dot', 'read-accept-lang',
        'read-accept-quasiquote', 'read-accept-reader', 'read-byte',
        'read-byte-or-special', 'read-bytes', 'read-bytes!',
        'read-bytes-avail!', 'read-bytes-avail!*',
        'read-bytes-avail!/enable-break', 'read-bytes-line',
        'read-case-sensitive', 'read-char', 'read-char-or-special',
        'read-curly-brace-as-paren', 'read-decimal-as-inexact',
        'read-eval-print-loop', 'read-language', 'read-line',
        'read-on-demand-source', 'read-square-bracket-as-paren',
        'read-string', 'read-string!', 'read-syntax',
        'read-syntax/recursive', 'read/recursive', 'readtable-mapping',
        'readtable?', 'real->double-flonum', 'real->floating-point-bytes',
        'real->single-flonum', 'real-part', 'real?', 'regexp',
        'regexp-match', 'regexp-match-peek', 'regexp-match-peek-immediate',
        'regexp-match-peek-positions',
        'regexp-match-peek-positions-immediate',
        'regexp-match-peek-positions-immediate/end',
        'regexp-match-peek-positions/end', 'regexp-match-positions',
        'regexp-match-positions/end', 'regexp-match/end', 'regexp-match?',
        'regexp-max-lookbehind', 'regexp-replace', 'regexp-replace*',
        'regexp?', 'relative-path?', 'remainder',
        'rename-file-or-directory', 'rename-transformer-target',
        'rename-transformer?', 'resolve-path', 'resolved-module-path-name',
        'resolved-module-path?', 'reverse', 'round', 'seconds->date',
        'security-guard?', 'semaphore-peek-evt', 'semaphore-post',
        'semaphore-try-wait?', 'semaphore-wait',
        'semaphore-wait/enable-break', 'semaphore?',
        'set!-transformer-procedure', 'set!-transformer?', 'set-box!',
        'set-mcar!', 'set-mcdr!', 'set-port-next-location!',
        'shared-bytes', 'shell-execute', 'simplify-path', 'sin',
        'single-flonum?', 'sleep', 'special-comment-value',
        'special-comment?', 'split-path', 'sqrt', 'srcloc',
        'srcloc-column', 'srcloc-line', 'srcloc-position', 'srcloc-source',
        'srcloc-span', 'srcloc?', 'string', 'string->bytes/latin-1',
        'string->bytes/locale', 'string->bytes/utf-8',
        'string->immutable-string', 'string->keyword', 'string->list',
        'string->number', 'string->path', 'string->path-element',
        'string->symbol', 'string->uninterned-symbol',
        'string->unreadable-symbol', 'string-append', 'string-ci<=?',
        'string-ci<?', 'string-ci=?', 'string-ci>=?', 'string-ci>?',
        'string-copy', 'string-copy!', 'string-downcase', 'string-fill!',
        'string-foldcase', 'string-length', 'string-locale-ci<?',
        'string-locale-ci=?', 'string-locale-ci>?',
        'string-locale-downcase', 'string-locale-upcase',
        'string-locale<?', 'string-locale=?', 'string-locale>?',
        'string-normalize-nfc', 'string-normalize-nfd',
        'string-normalize-nfkc', 'string-normalize-nfkd', 'string-ref',
        'string-set!', 'string-titlecase', 'string-upcase',
        'string-utf-8-length', 'string<=?', 'string<?', 'string=?',
        'string>=?', 'string>?', 'string?', 'struct->vector',
        'struct-accessor-procedure?', 'struct-constructor-procedure?',
        'struct-info', 'struct-mutator-procedure?',
        'struct-predicate-procedure?', 'struct-type-info',
        'struct-type-make-constructor', 'struct-type-make-predicate',
        'struct-type-property-accessor-procedure?',
        'struct-type-property?', 'struct-type?', 'struct:arity-at-least',
        'struct:date', 'struct:date*', 'struct:exn', 'struct:exn:break',
        'struct:exn:fail', 'struct:exn:fail:contract',
        'struct:exn:fail:contract:arity',
        'struct:exn:fail:contract:continuation',
        'struct:exn:fail:contract:divide-by-zero',
        'struct:exn:fail:contract:non-fixnum-result',
        'struct:exn:fail:contract:variable', 'struct:exn:fail:filesystem',
        'struct:exn:fail:filesystem:exists',
        'struct:exn:fail:filesystem:version', 'struct:exn:fail:network',
        'struct:exn:fail:out-of-memory', 'struct:exn:fail:read',
        'struct:exn:fail:read:eof', 'struct:exn:fail:read:non-char',
        'struct:exn:fail:syntax', 'struct:exn:fail:syntax:unbound',
        'struct:exn:fail:unsupported', 'struct:exn:fail:user',
        'struct:srcloc', 'struct?', 'sub1', 'subbytes', 'subprocess',
        'subprocess-group-enabled', 'subprocess-kill', 'subprocess-pid',
        'subprocess-status', 'subprocess-wait', 'subprocess?', 'substring',
        'symbol->string', 'symbol-interned?', 'symbol-unreadable?',
        'symbol?', 'sync', 'sync/enable-break', 'sync/timeout',
        'sync/timeout/enable-break', 'syntax->list', 'syntax-arm',
        'syntax-column', 'syntax-disarm', 'syntax-e', 'syntax-line',
        'syntax-local-bind-syntaxes', 'syntax-local-certifier',
        'syntax-local-context', 'syntax-local-expand-expression',
        'syntax-local-get-shadower', 'syntax-local-introduce',
        'syntax-local-lift-context', 'syntax-local-lift-expression',
        'syntax-local-lift-module-end-declaration',
        'syntax-local-lift-provide', 'syntax-local-lift-require',
        'syntax-local-lift-values-expression',
        'syntax-local-make-definition-context',
        'syntax-local-make-delta-introducer',
        'syntax-local-module-defined-identifiers',
        'syntax-local-module-exports',
        'syntax-local-module-required-identifiers', 'syntax-local-name',
        'syntax-local-phase-level',
        'syntax-local-transforming-module-provides?', 'syntax-local-value',
        'syntax-local-value/immediate', 'syntax-original?',
        'syntax-position', 'syntax-property',
        'syntax-property-symbol-keys', 'syntax-protect', 'syntax-rearm',
        'syntax-recertify', 'syntax-shift-phase-level', 'syntax-source',
        'syntax-source-module', 'syntax-span', 'syntax-taint',
        'syntax-tainted?', 'syntax-track-origin',
        'syntax-transforming-module-expression?', 'syntax-transforming?',
        'syntax?', 'system-big-endian?', 'system-idle-evt',
        'system-language+country', 'system-library-subpath',
        'system-path-convention-type', 'system-type', 'tan',
        'tcp-abandon-port', 'tcp-accept', 'tcp-accept-evt',
        'tcp-accept-ready?', 'tcp-accept/enable-break', 'tcp-addresses',
        'tcp-close', 'tcp-connect', 'tcp-connect/enable-break',
        'tcp-listen', 'tcp-listener?', 'tcp-port?', 'terminal-port?',
        'thread', 'thread-cell-ref', 'thread-cell-set!', 'thread-cell?',
        'thread-dead-evt', 'thread-dead?', 'thread-group?',
        'thread-resume', 'thread-resume-evt', 'thread-rewind-receive',
        'thread-running?', 'thread-suspend', 'thread-suspend-evt',
        'thread-wait', 'thread/suspend-to-kill', 'thread?', 'time-apply',
        'truncate', 'udp-addresses', 'udp-bind!', 'udp-bound?',
        'udp-close', 'udp-connect!', 'udp-connected?', 'udp-open-socket',
        'udp-receive!', 'udp-receive!*', 'udp-receive!-evt',
        'udp-receive!/enable-break', 'udp-receive-ready-evt', 'udp-send',
        'udp-send*', 'udp-send-evt', 'udp-send-ready-evt', 'udp-send-to',
        'udp-send-to*', 'udp-send-to-evt', 'udp-send-to/enable-break',
        'udp-send/enable-break', 'udp?', 'unbox',
        'uncaught-exception-handler', 'use-collection-link-paths',
        'use-compiled-file-paths', 'use-user-specific-search-paths',
        'values', 'variable-reference->empty-namespace',
        'variable-reference->module-base-phase',
        'variable-reference->module-declaration-inspector',
        'variable-reference->module-source',
        'variable-reference->namespace', 'variable-reference->phase',
        'variable-reference->resolved-module-path',
        'variable-reference-constant?', 'variable-reference?', 'vector',
        'vector->immutable-vector', 'vector->list',
        'vector->pseudo-random-generator',
        'vector->pseudo-random-generator!', 'vector->values',
        'vector-fill!', 'vector-immutable', 'vector-length', 'vector-ref',
        'vector-set!', 'vector-set-performance-stats!', 'vector?',
        'version', 'void', 'void?', 'weak-box-value', 'weak-box?',
        'will-execute', 'will-executor?', 'will-register',
        'will-try-execute', 'with-input-from-file', 'with-output-to-file',
        'wrap-evt', 'write', 'write-byte', 'write-bytes',
        'write-bytes-avail', 'write-bytes-avail*', 'write-bytes-avail-evt',
        'write-bytes-avail/enable-break', 'write-char', 'write-special',
        'write-special-avail*', 'write-special-evt', 'write-string', 'zero?'
    ]

    # From SchemeLexer
    valid_name = r'[a-zA-Z0-9!$%&*+,/:<=>?@^_~|-]+'

    tokens = {
        'root' : [
            (r';.*$', Comment.Single),
            (r'#\|[^|]+\|#', Comment.Multiline),

            # whitespaces - usually not relevant
            (r'\s+', Text),

            ## numbers: Keep in mind Racket reader hash prefixes,
            ## which can denote the base or the type. These don't map
            ## neatly onto pygments token types; some judgment calls
            ## here.  Note that none of these regexps attempt to
            ## exclude identifiers that start with a number, such as a
            ## variable named "100-Continue".

            # #b
            (r'#b[-+]?[01]+\.[01]+', Number.Float),
            (r'#b[01]+e[-+]?[01]+', Number.Float),
            (r'#b[-+]?[01]/[01]+', Number),
            (r'#b[-+]?[01]+', Number.Integer),
            (r'#b\S*', Error),

            # #d OR no hash prefix
            (r'(#d)?[-+]?\d+\.\d+', Number.Float),
            (r'(#d)?\d+e[-+]?\d+', Number.Float),
            (r'(#d)?[-+]?\d+/\d+', Number),
            (r'(#d)?[-+]?\d+', Number.Integer),
            (r'#d\S*', Error),

            # #e
            (r'#e[-+]?\d+\.\d+', Number.Float),
            (r'#e\d+e[-+]?\d+', Number.Float),
            (r'#e[-+]?\d+/\d+', Number),
            (r'#e[-+]?\d+', Number),
            (r'#e\S*', Error),

            # #i is always inexact-real, i.e. float
            (r'#i[-+]?\d+\.\d+', Number.Float),
            (r'#i\d+e[-+]?\d+', Number.Float),
            (r'#i[-+]?\d+/\d+', Number.Float),
            (r'#i[-+]?\d+', Number.Float),
            (r'#i\S*', Error),

            # #o
            (r'#o[-+]?[0-7]+\.[0-7]+', Number.Oct),
            (r'#o[0-7]+e[-+]?[0-7]+', Number.Oct),
            (r'#o[-+]?[0-7]+/[0-7]+', Number.Oct),
            (r'#o[-+]?[0-7]+', Number.Oct),
            (r'#o\S*', Error),

            # #x
            (r'#x[-+]?[0-9a-fA-F]+\.[0-9a-fA-F]+', Number.Hex),
            # the exponent variation (e.g. #x1e1) is N/A
            (r'#x[-+]?[0-9a-fA-F]+/[0-9a-fA-F]+', Number.Hex),
            (r'#x[-+]?[0-9a-fA-F]+', Number.Hex),
            (r'#x\S*', Error),


            # strings, symbols and characters
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'" + valid_name, String.Symbol),
            (r"#\\([()/'\"._!§$%& ?=+-]{1}|[a-zA-Z0-9]+)", String.Char),
            (r'#rx".+"', String.Regex),
            (r'#px".+"', String.Regex),

            # constants
            (r'(#t|#f)', Name.Constant),

            # keyword argument names (e.g. #:keyword)
            (r'#:\S+', Keyword.Declaration),

            # #lang
            (r'#lang \S+', Keyword.Namespace),

            # special operators
            (r"('|#|`|,@|,|\.)", Operator),

            # highlight the keywords
            ('(%s)' % '|'.join([
                re.escape(entry) + ' ' for entry in keywords]),
                Keyword
            ),

            # first variable in a quoted string like
            # '(this is syntactic sugar)
            (r"(?<='\()" + valid_name, Name.Variable),
            (r"(?<=#\()" + valid_name, Name.Variable),

            # highlight the builtins
            ("(?<=\()(%s)" % '|'.join([
                re.escape(entry) + ' ' for entry in builtins]),
                Name.Builtin
            ),

            # the remaining functions; handle both ( and [
            (r'(?<=(\(|\[|\{))' + valid_name, Name.Function),

            # find the remaining variables
            (valid_name, Name.Variable),

            # the famous parentheses!
            (r'(\(|\)|\[|\]|\{|\})', Punctuation),
        ],
    }


class SchemeLexer(RegexLexer):
    """
    A Scheme lexer, parsing a stream and outputting the tokens
    needed to highlight scheme code.
    This lexer could be most probably easily subclassed to parse
    other LISP-Dialects like Common Lisp, Emacs Lisp or AutoLisp.

    This parser is checked with pastes from the LISP pastebin
    at http://paste.lisp.org/ to cover as much syntax as possible.

    It supports the full Scheme syntax as defined in R5RS.

    *New in Pygments 0.6.*
    """
    name = 'Scheme'
    aliases = ['scheme', 'scm']
    filenames = ['*.scm', '*.ss']
    mimetypes = ['text/x-scheme', 'application/x-scheme']

    # list of known keywords and builtins taken form vim 6.4 scheme.vim
    # syntax file.
    keywords = [
        'lambda', 'define', 'if', 'else', 'cond', 'and', 'or', 'case', 'let',
        'let*', 'letrec', 'begin', 'do', 'delay', 'set!', '=>', 'quote',
        'quasiquote', 'unquote', 'unquote-splicing', 'define-syntax',
        'let-syntax', 'letrec-syntax', 'syntax-rules'
    ]
    builtins = [
        '*', '+', '-', '/', '<', '<=', '=', '>', '>=', 'abs', 'acos', 'angle',
        'append', 'apply', 'asin', 'assoc', 'assq', 'assv', 'atan',
        'boolean?', 'caaaar', 'caaadr', 'caaar', 'caadar', 'caaddr', 'caadr',
        'caar', 'cadaar', 'cadadr', 'cadar', 'caddar', 'cadddr', 'caddr',
        'cadr', 'call-with-current-continuation', 'call-with-input-file',
        'call-with-output-file', 'call-with-values', 'call/cc', 'car',
        'cdaaar', 'cdaadr', 'cdaar', 'cdadar', 'cdaddr', 'cdadr', 'cdar',
        'cddaar', 'cddadr', 'cddar', 'cdddar', 'cddddr', 'cdddr', 'cddr',
        'cdr', 'ceiling', 'char->integer', 'char-alphabetic?', 'char-ci<=?',
        'char-ci<?', 'char-ci=?', 'char-ci>=?', 'char-ci>?', 'char-downcase',
        'char-lower-case?', 'char-numeric?', 'char-ready?', 'char-upcase',
        'char-upper-case?', 'char-whitespace?', 'char<=?', 'char<?', 'char=?',
        'char>=?', 'char>?', 'char?', 'close-input-port', 'close-output-port',
        'complex?', 'cons', 'cos', 'current-input-port', 'current-output-port',
        'denominator', 'display', 'dynamic-wind', 'eof-object?', 'eq?',
        'equal?', 'eqv?', 'eval', 'even?', 'exact->inexact', 'exact?', 'exp',
        'expt', 'floor', 'for-each', 'force', 'gcd', 'imag-part',
        'inexact->exact', 'inexact?', 'input-port?', 'integer->char',
        'integer?', 'interaction-environment', 'lcm', 'length', 'list',
        'list->string', 'list->vector', 'list-ref', 'list-tail', 'list?',
        'load', 'log', 'magnitude', 'make-polar', 'make-rectangular',
        'make-string', 'make-vector', 'map', 'max', 'member', 'memq', 'memv',
        'min', 'modulo', 'negative?', 'newline', 'not', 'null-environment',
        'null?', 'number->string', 'number?', 'numerator', 'odd?',
        'open-input-file', 'open-output-file', 'output-port?', 'pair?',
        'peek-char', 'port?', 'positive?', 'procedure?', 'quotient',
        'rational?', 'rationalize', 'read', 'read-char', 'real-part', 'real?',
        'remainder', 'reverse', 'round', 'scheme-report-environment',
        'set-car!', 'set-cdr!', 'sin', 'sqrt', 'string', 'string->list',
        'string->number', 'string->symbol', 'string-append', 'string-ci<=?',
        'string-ci<?', 'string-ci=?', 'string-ci>=?', 'string-ci>?',
        'string-copy', 'string-fill!', 'string-length', 'string-ref',
        'string-set!', 'string<=?', 'string<?', 'string=?', 'string>=?',
        'string>?', 'string?', 'substring', 'symbol->string', 'symbol?',
        'tan', 'transcript-off', 'transcript-on', 'truncate', 'values',
        'vector', 'vector->list', 'vector-fill!', 'vector-length',
        'vector-ref', 'vector-set!', 'vector?', 'with-input-from-file',
        'with-output-to-file', 'write', 'write-char', 'zero?'
    ]

    # valid names for identifiers
    # well, names can only not consist fully of numbers
    # but this should be good enough for now
    valid_name = r'[a-zA-Z0-9!$%&*+,/:<=>?@^_~|-]+'

    tokens = {
        'root' : [
            # the comments - always starting with semicolon
            # and going to the end of the line
            (r';.*$', Comment.Single),

            # whitespaces - usually not relevant
            (r'\s+', Text),

            # numbers
            (r'-?\d+\.\d+', Number.Float),
            (r'-?\d+', Number.Integer),
            # support for uncommon kinds of numbers -
            # have to figure out what the characters mean
            #(r'(#e|#i|#b|#o|#d|#x)[\d.]+', Number),

            # strings, symbols and characters
            (r'"(\\\\|\\"|[^"])*"', String),
            (r"'" + valid_name, String.Symbol),
            (r"#\\([()/'\"._!§$%& ?=+-]{1}|[a-zA-Z0-9]+)", String.Char),

            # constants
            (r'(#t|#f)', Name.Constant),

            # special operators
            (r"('|#|`|,@|,|\.)", Operator),

            # highlight the keywords
            ('(%s)' % '|'.join([
                re.escape(entry) + ' ' for entry in keywords]),
                Keyword
            ),

            # first variable in a quoted string like
            # '(this is syntactic sugar)
            (r"(?<='\()" + valid_name, Name.Variable),
            (r"(?<=#\()" + valid_name, Name.Variable),

            # highlight the builtins
            ("(?<=\()(%s)" % '|'.join([
                re.escape(entry) + ' ' for entry in builtins]),
                Name.Builtin
            ),

            # the remaining functions
            (r'(?<=\()' + valid_name, Name.Function),
            # find the remaining variables
            (valid_name, Name.Variable),

            # the famous parentheses!
            (r'(\(|\))', Punctuation),
            (r'(\[|\])', Punctuation),
        ],
    }


class CommonLispLexer(RegexLexer):
    """
    A Common Lisp lexer.

    *New in Pygments 0.9.*
    """
    name = 'Common Lisp'
    aliases = ['common-lisp', 'cl']
    filenames = ['*.cl', '*.lisp', '*.el']  # use for Elisp too
    mimetypes = ['text/x-common-lisp']

    flags = re.IGNORECASE | re.MULTILINE

    ### couple of useful regexes

    # characters that are not macro-characters and can be used to begin a symbol
    nonmacro = r'\\.|[a-zA-Z0-9!$%&*+-/<=>?@\[\]^_{}~]'
    constituent = nonmacro + '|[#.:]'
    terminated = r'(?=[ "()\'\n,;`])' # whitespace or terminating macro characters

    ### symbol token, reverse-engineered from hyperspec
    # Take a deep breath...
    symbol = r'(\|[^|]+\||(?:%s)(?:%s)*)' % (nonmacro, constituent)

    def __init__(self, **options):
        from pygments.lexers._clbuiltins import BUILTIN_FUNCTIONS, \
            SPECIAL_FORMS, MACROS, LAMBDA_LIST_KEYWORDS, DECLARATIONS, \
            BUILTIN_TYPES, BUILTIN_CLASSES
        self.builtin_function = BUILTIN_FUNCTIONS
        self.special_forms = SPECIAL_FORMS
        self.macros = MACROS
        self.lambda_list_keywords = LAMBDA_LIST_KEYWORDS
        self.declarations = DECLARATIONS
        self.builtin_types = BUILTIN_TYPES
        self.builtin_classes = BUILTIN_CLASSES
        RegexLexer.__init__(self, **options)

    def get_tokens_unprocessed(self, text):
        stack = ['root']
        for index, token, value in RegexLexer.get_tokens_unprocessed(self, text, stack):
            if token is Name.Variable:
                if value in self.builtin_function:
                    yield index, Name.Builtin, value
                    continue
                if value in self.special_forms:
                    yield index, Keyword, value
                    continue
                if value in self.macros:
                    yield index, Name.Builtin, value
                    continue
                if value in self.lambda_list_keywords:
                    yield index, Keyword, value
                    continue
                if value in self.declarations:
                    yield index, Keyword, value
                    continue
                if value in self.builtin_types:
                    yield index, Keyword.Type, value
                    continue
                if value in self.builtin_classes:
                    yield index, Name.Class, value
                    continue
            yield index, token, value

    tokens = {
        'root' : [
            ('', Text, 'body'),
        ],
        'multiline-comment' : [
            (r'#\|', Comment.Multiline, '#push'), # (cf. Hyperspec 2.4.8.19)
            (r'\|#', Comment.Multiline, '#pop'),
            (r'[^|#]+', Comment.Multiline),
            (r'[|#]', Comment.Multiline),
        ],
        'commented-form' : [
            (r'\(', Comment.Preproc, '#push'),
            (r'\)', Comment.Preproc, '#pop'),
            (r'[^()]+', Comment.Preproc),
        ],
        'body' : [
            # whitespace
            (r'\s+', Text),

            # single-line comment
            (r';.*$', Comment.Single),

            # multi-line comment
            (r'#\|', Comment.Multiline, 'multiline-comment'),

            # encoding comment (?)
            (r'#\d*Y.*$', Comment.Special),

            # strings and characters
            (r'"(\\.|\\\n|[^"\\])*"', String),
            # quoting
            (r":" + symbol, String.Symbol),
            (r"'" + symbol, String.Symbol),
            (r"'", Operator),
            (r"`", Operator),

            # decimal numbers
            (r'[-+]?\d+\.?' + terminated, Number.Integer),
            (r'[-+]?\d+/\d+' + terminated, Number),
            (r'[-+]?(\d*\.\d+([defls][-+]?\d+)?|\d+(\.\d*)?[defls][-+]?\d+)' \
                + terminated, Number.Float),

            # sharpsign strings and characters
            (r"#\\." + terminated, String.Char),
            (r"#\\" + symbol, String.Char),

            # vector
            (r'#\(', Operator, 'body'),

            # bitstring
            (r'#\d*\*[01]*', Literal.Other),

            # uninterned symbol
            (r'#:' + symbol, String.Symbol),

            # read-time and load-time evaluation
            (r'#[.,]', Operator),

            # function shorthand
            (r'#\'', Name.Function),

            # binary rational
            (r'#[bB][+-]?[01]+(/[01]+)?', Number),

            # octal rational
            (r'#[oO][+-]?[0-7]+(/[0-7]+)?', Number.Oct),

            # hex rational
            (r'#[xX][+-]?[0-9a-fA-F]+(/[0-9a-fA-F]+)?', Number.Hex),

            # radix rational
            (r'#\d+[rR][+-]?[0-9a-zA-Z]+(/[0-9a-zA-Z]+)?', Number),

            # complex
            (r'(#[cC])(\()', bygroups(Number, Punctuation), 'body'),

            # array
            (r'(#\d+[aA])(\()', bygroups(Literal.Other, Punctuation), 'body'),

            # structure
            (r'(#[sS])(\()', bygroups(Literal.Other, Punctuation), 'body'),

            # path
            (r'#[pP]?"(\\.|[^"])*"', Literal.Other),

            # reference
            (r'#\d+=', Operator),
            (r'#\d+#', Operator),

            # read-time comment
            (r'#+nil' + terminated + '\s*\(', Comment.Preproc, 'commented-form'),

            # read-time conditional
            (r'#[+-]', Operator),

            # special operators that should have been parsed already
            (r'(,@|,|\.)', Operator),

            # special constants
            (r'(t|nil)' + terminated, Name.Constant),

            # functions and variables
            (r'\*' + symbol + '\*', Name.Variable.Global),
            (symbol, Name.Variable),

            # parentheses
            (r'\(', Punctuation, 'body'),
            (r'\)', Punctuation, '#pop'),
        ],
    }


class HaskellLexer(RegexLexer):
    """
    A Haskell lexer based on the lexemes defined in the Haskell 98 Report.

    *New in Pygments 0.8.*
    """
    name = 'Haskell'
    aliases = ['haskell', 'hs']
    filenames = ['*.hs']
    mimetypes = ['text/x-haskell']

    reserved = ['case','class','data','default','deriving','do','else',
                'if','in','infix[lr]?','instance',
                'let','newtype','of','then','type','where','_']
    ascii = ['NUL','SOH','[SE]TX','EOT','ENQ','ACK',
             'BEL','BS','HT','LF','VT','FF','CR','S[OI]','DLE',
             'DC[1-4]','NAK','SYN','ETB','CAN',
             'EM','SUB','ESC','[FGRU]S','SP','DEL']

    tokens = {
        'root': [
            # Whitespace:
            (r'\s+', Text),
            #(r'--\s*|.*$', Comment.Doc),
            (r'--(?![!#$%&*+./<=>?@\^|_~:\\]).*?$', Comment.Single),
            (r'{-', Comment.Multiline, 'comment'),
            # Lexemes:
            #  Identifiers
            (r'\bimport\b', Keyword.Reserved, 'import'),
            (r'\bmodule\b', Keyword.Reserved, 'module'),
            (r'\berror\b', Name.Exception),
            (r'\b(%s)(?!\')\b' % '|'.join(reserved), Keyword.Reserved),
            (r'^[_a-z][\w\']*', Name.Function),
            (r"'?[_a-z][\w']*", Name),
            (r"('')?[A-Z][\w\']*", Keyword.Type),
            #  Operators
            (r'\\(?![:!#$%&*+.\\/<=>?@^|~-]+)', Name.Function), # lambda operator
            (r'(<-|::|->|=>|=)(?![:!#$%&*+.\\/<=>?@^|~-]+)', Operator.Word), # specials
            (r':[:!#$%&*+.\\/<=>?@^|~-]*', Keyword.Type), # Constructor operators
            (r'[:!#$%&*+.\\/<=>?@^|~-]+', Operator), # Other operators
            #  Numbers
            (r'\d+[eE][+-]?\d+', Number.Float),
            (r'\d+\.\d+([eE][+-]?\d+)?', Number.Float),
            (r'0[oO][0-7]+', Number.Oct),
            (r'0[xX][\da-fA-F]+', Number.Hex),
            (r'\d+', Number.Integer),
            #  Character/String Literals
            (r"'", String.Char, 'character'),
            (r'"', String, 'string'),
            #  Special
            (r'\[\]', Keyword.Type),
            (r'\(\)', Name.Builtin),
            (r'[][(),;`{}]', Punctuation),
        ],
        'import': [
            # Import statements
            (r'\s+', Text),
            (r'"', String, 'string'),
            # after "funclist" state
            (r'\)', Punctuation, '#pop'),
            (r'qualified\b', Keyword),
            # import X as Y
            (r'([A-Z][a-zA-Z0-9_.]*)(\s+)(as)(\s+)([A-Z][a-zA-Z0-9_.]*)',
             bygroups(Name.Namespace, Text, Keyword, Text, Name), '#pop'),
            # import X hiding (functions)
            (r'([A-Z][a-zA-Z0-9_.]*)(\s+)(hiding)(\s+)(\()',
             bygroups(Name.Namespace, Text, Keyword, Text, Punctuation), 'funclist'),
            # import X (functions)
            (r'([A-Z][a-zA-Z0-9_.]*)(\s+)(\()',
             bygroups(Name.Namespace, Text, Punctuation), 'funclist'),
            # import X
            (r'[a-zA-Z0-9_.]+', Name.Namespace, '#pop'),
        ],
        'module': [
            (r'\s+', Text),
            (r'([A-Z][a-zA-Z0-9_.]*)(\s+)(\()',
             bygroups(Name.Namespace, Text, Punctuation), 'funclist'),
            (r'[A-Z][a-zA-Z0-9_.]*', Name.Namespace, '#pop'),
        ],
        'funclist': [
            (r'\s+', Text),
            (r'[A-Z][a-zA-Z0-9_]*', Keyword.Type),
            (r'(_[\w\']+|[a-z][\w\']*)', Name.Function),
            (r'--.*$', Comment.Single),
            (r'{-', Comment.Multiline, 'comment'),
            (r',', Punctuation),
            (r'[:!#$%&*+.\\/<=>?@^|~-]+', Operator),
            # (HACK, but it makes sense to push two instances, believe me)
            (r'\(', Punctuation, ('funclist', 'funclist')),
            (r'\)', Punctuation, '#pop:2'),
        ],
        'comment': [
            # Multiline Comments
            (r'[^-{}]+', Comment.Multiline),
            (r'{-', Comment.Multiline, '#push'),
            (r'-}', Comment.Multiline, '#pop'),
            (r'[-{}]', Comment.Multiline),
        ],
        'character': [
            # Allows multi-chars, incorrectly.
            (r"[^\\']", String.Char),
            (r"\\", String.Escape, 'escape'),
            ("'", String.Char, '#pop'),
        ],
        'string': [
            (r'[^\\"]+', String),
            (r"\\", String.Escape, 'escape'),
            ('"', String, '#pop'),
        ],
        'escape': [
            (r'[abfnrtv"\'&\\]', String.Escape, '#pop'),
            (r'\^[][A-Z@\^_]', String.Escape, '#pop'),
            ('|'.join(ascii), String.Escape, '#pop'),
            (r'o[0-7]+', String.Escape, '#pop'),
            (r'x[\da-fA-F]+', String.Escape, '#pop'),
            (r'\d+', String.Escape, '#pop'),
            (r'\s+\\', String.Escape, '#pop'),
        ],
    }


line_re = re.compile('.*?\n')
bird_re = re.compile(r'(>[ \t]*)(.*\n)')

class LiterateHaskellLexer(Lexer):
    """
    For Literate Haskell (Bird-style or LaTeX) source.

    Additional options accepted:

    `litstyle`
        If given, must be ``"bird"`` or ``"latex"``.  If not given, the style
        is autodetected: if the first non-whitespace character in the source
        is a backslash or percent character, LaTeX is assumed, else Bird.

    *New in Pygments 0.9.*
    """
    name = 'Literate Haskell'
    aliases = ['lhs', 'literate-haskell']
    filenames = ['*.lhs']
    mimetypes = ['text/x-literate-haskell']

    def get_tokens_unprocessed(self, text):
        hslexer = HaskellLexer(**self.options)

        style = self.options.get('litstyle')
        if style is None:
            style = (text.lstrip()[0:1] in '%\\') and 'latex' or 'bird'

        code = ''
        insertions = []
        if style == 'bird':
            # bird-style
            for match in line_re.finditer(text):
                line = match.group()
                m = bird_re.match(line)
                if m:
                    insertions.append((len(code),
                                       [(0, Comment.Special, m.group(1))]))
                    code += m.group(2)
                else:
                    insertions.append((len(code), [(0, Text, line)]))
        else:
            # latex-style
            from pygments.lexers.text import TexLexer
            lxlexer = TexLexer(**self.options)

            codelines = 0
            latex = ''
            for match in line_re.finditer(text):
                line = match.group()
                if codelines:
                    if line.lstrip().startswith('\\end{code}'):
                        codelines = 0
                        latex += line
                    else:
                        code += line
                elif line.lstrip().startswith('\\begin{code}'):
                    codelines = 1
                    latex += line
                    insertions.append((len(code),
                                       list(lxlexer.get_tokens_unprocessed(latex))))
                    latex = ''
                else:
                    latex += line
            insertions.append((len(code),
                               list(lxlexer.get_tokens_unprocessed(latex))))
        for item in do_insertions(insertions, hslexer.get_tokens_unprocessed(code)):
            yield item


class SMLLexer(RegexLexer):
    """
    For the Standard ML language.

    *New in Pygments 1.5.*
    """

    name = 'Standard ML'
    aliases = ['sml']
    filenames = ['*.sml', '*.sig', '*.fun',]
    mimetypes = ['text/x-standardml', 'application/x-standardml']

    alphanumid_reserved = [
        # Core
        'abstype', 'and', 'andalso', 'as', 'case', 'datatype', 'do', 'else',
        'end', 'exception', 'fn', 'fun', 'handle', 'if', 'in', 'infix',
        'infixr', 'let', 'local', 'nonfix', 'of', 'op', 'open', 'orelse',
        'raise', 'rec', 'then', 'type', 'val', 'with', 'withtype', 'while',
        # Modules
        'eqtype', 'functor', 'include', 'sharing', 'sig', 'signature',
        'struct', 'structure', 'where',
    ]

    symbolicid_reserved = [
        # Core
        ':', '\|', '=', '=>', '->', '#',
        # Modules
        ':>',
    ]

    nonid_reserved = [ '(', ')', '[', ']', '{', '}', ',', ';', '...', '_' ]

    alphanumid_re = r"[a-zA-Z][a-zA-Z0-9_']*"
    symbolicid_re = r"[!%&$#+\-/:<=>?@\\~`^|*]+"

    # A character constant is a sequence of the form #s, where s is a string
    # constant denoting a string of size one character. This setup just parses
    # the entire string as either a String.Double or a String.Char (depending
    # on the argument), even if the String.Char is an erronous
    # multiple-character string.
    def stringy (whatkind):
        return [
            (r'[^"\\]', whatkind),
            (r'\\[\\\"abtnvfr]', String.Escape),
            # Control-character notation is used for codes < 32,
            # where \^@ == \000
            (r'\\\^[\x40-\x5e]', String.Escape),
            # Docs say 'decimal digits'
            (r'\\[0-9]{3}', String.Escape),
            (r'\\u[0-9a-fA-F]{4}', String.Escape),
            (r'\\\s+\\', String.Interpol),
            (r'"', whatkind, '#pop'),
        ]

    # Callbacks for distinguishing tokens and reserved words
    def long_id_callback(self, match):
        if match.group(1) in self.alphanumid_reserved: token = Error
        else: token = Name.Namespace
        yield match.start(1), token, match.group(1)
        yield match.start(2), Punctuation, match.group(2)

    def end_id_callback(self, match):
        if match.group(1) in self.alphanumid_reserved: token = Error
        elif match.group(1) in self.symbolicid_reserved: token = Error
        else: token = Name
        yield match.start(1), token, match.group(1)

    def id_callback(self, match):
        str = match.group(1)
        if str in self.alphanumid_reserved: token = Keyword.Reserved
        elif str in self.symbolicid_reserved: token = Punctuation
        else: token = Name
        yield match.start(1), token, str

    tokens = {
        # Whitespace and comments are (almost) everywhere
        'whitespace': [
            (r'\s+', Text),
            (r'\(\*', Comment.Multiline, 'comment'),
        ],

        'delimiters': [
            # This lexer treats these delimiters specially:
            # Delimiters define scopes, and the scope is how the meaning of
            # the `|' is resolved - is it a case/handle expression, or function
            # definition by cases? (This is not how the Definition works, but
            # it's how MLton behaves, see http://mlton.org/SMLNJDeviations)
            (r'\(|\[|{', Punctuation, 'main'),
            (r'\)|\]|}', Punctuation, '#pop'),
            (r'\b(let|if|local)\b(?!\')', Keyword.Reserved, ('main', 'main')),
            (r'\b(struct|sig|while)\b(?!\')', Keyword.Reserved, 'main'),
            (r'\b(do|else|end|in|then)\b(?!\')', Keyword.Reserved, '#pop'),
        ],

        'core': [
            # Punctuation that doesn't overlap symbolic identifiers
            (r'(%s)' % '|'.join([re.escape(z) for z in nonid_reserved]),
             Punctuation),

            # Special constants: strings, floats, numbers in decimal and hex
            (r'#"', String.Char, 'char'),
            (r'"', String.Double, 'string'),
            (r'~?0x[0-9a-fA-F]+', Number.Hex),
            (r'0wx[0-9a-fA-F]+', Number.Hex),
            (r'0w\d+', Number.Integer),
            (r'~?\d+\.\d+[eE]~?\d+', Number.Float),
            (r'~?\d+\.\d+', Number.Float),
            (r'~?\d+[eE]~?\d+', Number.Float),
            (r'~?\d+', Number.Integer),

            # Labels
            (r'#\s*[1-9][0-9]*', Name.Label),
            (r'#\s*(%s)' % alphanumid_re, Name.Label),
            (r'#\s+(%s)' % symbolicid_re, Name.Label),
            # Some reserved words trigger a special, local lexer state change
            (r'\b(datatype|abstype)\b(?!\')', Keyword.Reserved, 'dname'),
            (r'(?=\b(exception)\b(?!\'))', Text, ('ename')),
            (r'\b(functor|include|open|signature|structure)\b(?!\')',
             Keyword.Reserved, 'sname'),
            (r'\b(type|eqtype)\b(?!\')', Keyword.Reserved, 'tname'),

            # Regular identifiers, long and otherwise
            (r'\'[0-9a-zA-Z_\']*', Name.Decorator),
            (r'(%s)(\.)' % alphanumid_re, long_id_callback, "dotted"),
            (r'(%s)' % alphanumid_re, id_callback),
            (r'(%s)' % symbolicid_re, id_callback),
        ],
        'dotted': [
            (r'(%s)(\.)' % alphanumid_re, long_id_callback),
            (r'(%s)' % alphanumid_re, end_id_callback, "#pop"),
            (r'(%s)' % symbolicid_re, end_id_callback, "#pop"),
            (r'\s+', Error),
            (r'\S+', Error),
        ],


        # Main parser (prevents errors in files that have scoping errors)
        'root': [ (r'', Text, 'main') ],

        # In this scope, I expect '|' to not be followed by a function name,
        # and I expect 'and' to be followed by a binding site
        'main': [
            include('whitespace'),

            # Special behavior of val/and/fun
            (r'\b(val|and)\b(?!\')', Keyword.Reserved, 'vname'),
            (r'\b(fun)\b(?!\')', Keyword.Reserved,
             ('#pop', 'main-fun', 'fname')),

            include('delimiters'),
            include('core'),
            (r'\S+', Error),
        ],

        # In this scope, I expect '|' and 'and' to be followed by a function
        'main-fun': [
            include('whitespace'),

            (r'\s', Text),
            (r'\(\*', Comment.Multiline, 'comment'),

            # Special behavior of val/and/fun
            (r'\b(fun|and)\b(?!\')', Keyword.Reserved, 'fname'),
            (r'\b(val)\b(?!\')', Keyword.Reserved,
             ('#pop', 'main', 'vname')),

            # Special behavior of '|' and '|'-manipulating keywords
            (r'\|', Punctuation, 'fname'),
            (r'\b(case|handle)\b(?!\')', Keyword.Reserved,
             ('#pop', 'main')),

            include('delimiters'),
            include('core'),
            (r'\S+', Error),
        ],

        # Character and string parsers
        'char': stringy(String.Char),
        'string': stringy(String.Double),

        'breakout': [
            (r'(?=\b(%s)\b(?!\'))' % '|'.join(alphanumid_reserved), Text, '#pop'),
        ],

        # Dealing with what comes after module system keywords
        'sname': [
            include('whitespace'),
            include('breakout'),

            (r'(%s)' % alphanumid_re, Name.Namespace),
            (r'', Text, '#pop'),
        ],

        # Dealing with what comes after the 'fun' (or 'and' or '|') keyword
        'fname': [
            include('whitespace'),
            (r'\'[0-9a-zA-Z_\']*', Name.Decorator),
            (r'\(', Punctuation, 'tyvarseq'),

            (r'(%s)' % alphanumid_re, Name.Function, '#pop'),
            (r'(%s)' % symbolicid_re, Name.Function, '#pop'),

            # Ignore interesting function declarations like "fun (x + y) = ..."
            (r'', Text, '#pop'),
        ],

        # Dealing with what comes after the 'val' (or 'and') keyword
        'vname': [
            include('whitespace'),
            (r'\'[0-9a-zA-Z_\']*', Name.Decorator),
            (r'\(', Punctuation, 'tyvarseq'),

            (r'(%s)(\s*)(=(?!%s))' % (alphanumid_re, symbolicid_re),
             bygroups(Name.Variable, Text, Punctuation), '#pop'),
            (r'(%s)(\s*)(=(?!%s))' % (symbolicid_re, symbolicid_re),
             bygroups(Name.Variable, Text, Punctuation), '#pop'),
            (r'(%s)' % alphanumid_re, Name.Variable, '#pop'),
            (r'(%s)' % symbolicid_re, Name.Variable, '#pop'),

            # Ignore interesting patterns like 'val (x, y)'
            (r'', Text, '#pop'),
        ],

        # Dealing with what comes after the 'type' (or 'and') keyword
        'tname': [
            include('whitespace'),
            include('breakout'),

            (r'\'[0-9a-zA-Z_\']*', Name.Decorator),
            (r'\(', Punctuation, 'tyvarseq'),
            (r'=(?!%s)' % symbolicid_re, Punctuation, ('#pop', 'typbind')),

            (r'(%s)' % alphanumid_re, Keyword.Type),
            (r'(%s)' % symbolicid_re, Keyword.Type),
            (r'\S+', Error, '#pop'),
        ],

        # A type binding includes most identifiers
        'typbind': [
            include('whitespace'),

            (r'\b(and)\b(?!\')', Keyword.Reserved, ('#pop', 'tname')),

            include('breakout'),
            include('core'),
            (r'\S+', Error, '#pop'),
        ],

        # Dealing with what comes after the 'datatype' (or 'and') keyword
        'dname': [
            include('whitespace'),
            include('breakout'),

            (r'\'[0-9a-zA-Z_\']*', Name.Decorator),
            (r'\(', Punctuation, 'tyvarseq'),
            (r'(=)(\s*)(datatype)',
             bygroups(Punctuation, Text, Keyword.Reserved), '#pop'),
            (r'=(?!%s)' % symbolicid_re, Punctuation,
             ('#pop', 'datbind', 'datcon')),

            (r'(%s)' % alphanumid_re, Keyword.Type),
            (r'(%s)' % symbolicid_re, Keyword.Type),
            (r'\S+', Error, '#pop'),
        ],

        # common case - A | B | C of int
        'datbind': [
            include('whitespace'),

            (r'\b(and)\b(?!\')', Keyword.Reserved, ('#pop', 'dname')),
            (r'\b(withtype)\b(?!\')', Keyword.Reserved, ('#pop', 'tname')),
            (r'\b(of)\b(?!\')', Keyword.Reserved),

            (r'(\|)(\s*)(%s)' % alphanumid_re,
             bygroups(Punctuation, Text, Name.Class)),
            (r'(\|)(\s+)(%s)' % symbolicid_re,
             bygroups(Punctuation, Text, Name.Class)),

            include('breakout'),
            include('core'),
            (r'\S+', Error),
        ],

        # Dealing with what comes after an exception
        'ename': [
            include('whitespace'),

            (r'(exception|and)\b(\s+)(%s)' % alphanumid_re,
             bygroups(Keyword.Reserved, Text, Name.Class)),
            (r'(exception|and)\b(\s*)(%s)' % symbolicid_re,
             bygroups(Keyword.Reserved, Text, Name.Class)),
            (r'\b(of)\b(?!\')', Keyword.Reserved),

            include('breakout'),
            include('core'),
            (r'\S+', Error),
        ],

        'datcon': [
            include('whitespace'),
            (r'(%s)' % alphanumid_re, Name.Class, '#pop'),
            (r'(%s)' % symbolicid_re, Name.Class, '#pop'),
            (r'\S+', Error, '#pop'),
        ],

        # Series of type variables
        'tyvarseq': [
            (r'\s', Text),
            (r'\(\*', Comment.Multiline, 'comment'),

            (r'\'[0-9a-zA-Z_\']*', Name.Decorator),
            (alphanumid_re, Name),
            (r',', Punctuation),
            (r'\)', Punctuation, '#pop'),
            (symbolicid_re, Name),
        ],

        'comment': [
            (r'[^(*)]', Comment.Multiline),
            (r'\(\*', Comment.Multiline, '#push'),
            (r'\*\)', Comment.Multiline, '#pop'),
            (r'[(*)]', Comment.Multiline),
        ],
    }


class OcamlLexer(RegexLexer):
    """
    For the OCaml language.

    *New in Pygments 0.7.*
    """

    name = 'OCaml'
    aliases = ['ocaml']
    filenames = ['*.ml', '*.mli', '*.mll', '*.mly']
    mimetypes = ['text/x-ocaml']

    keywords = [
      'as', 'assert', 'begin', 'class', 'constraint', 'do', 'done',
      'downto', 'else', 'end', 'exception', 'external', 'false',
      'for', 'fun', 'function', 'functor', 'if', 'in', 'include',
      'inherit', 'initializer', 'lazy', 'let', 'match', 'method',
      'module', 'mutable', 'new', 'object', 'of', 'open', 'private',
      'raise', 'rec', 'sig', 'struct', 'then', 'to', 'true', 'try',
      'type', 'value', 'val', 'virtual', 'when', 'while', 'with',
    ]
    keyopts = [
      '!=','#','&','&&','\(','\)','\*','\+',',','-',
      '-\.','->','\.','\.\.',':','::',':=',':>',';',';;','<',
      '<-','=','>','>]','>}','\?','\?\?','\[','\[<','\[>','\[\|',
      ']','_','`','{','{<','\|','\|]','}','~'
    ]

    operators = r'[!$%&*+\./:<=>?@^|~-]'
    word_operators = ['and', 'asr', 'land', 'lor', 'lsl', 'lxor', 'mod', 'or']
    prefix_syms = r'[!?~]'
    infix_syms = r'[=<>@^|&+\*/$%-]'
    primitives = ['unit', 'int', 'float', 'bool', 'string', 'char', 'list', 'array']

    tokens = {
        'escape-sequence': [
            (r'\\[\\\"\'ntbr]', String.Escape),
            (r'\\[0-9]{3}', String.Escape),
            (r'\\x[0-9a-fA-F]{2}', String.Escape),
        ],
        'root': [
            (r'\s+', Text),
            (r'false|true|\(\)|\[\]', Name.Builtin.Pseudo),
            (r'\b([A-Z][A-Za-z0-9_\']*)(?=\s*\.)',
             Name.Namespace, 'dotted'),
            (r'\b([A-Z][A-Za-z0-9_\']*)', Name.Class),
            (r'\(\*(?![)])', Comment, 'comment'),
            (r'\b(%s)\b' % '|'.join(keywords), Keyword),
            (r'(%s)' % '|'.join(keyopts[::-1]), Operator),
            (r'(%s|%s)?%s' % (infix_syms, prefix_syms, operators), Operator),
            (r'\b(%s)\b' % '|'.join(word_operators), Operator.Word),
            (r'\b(%s)\b' % '|'.join(primitives), Keyword.Type),

            (r"[^\W\d][\w']*", Name),

            (r'-?\d[\d_]*(.[\d_]*)?([eE][+\-]?\d[\d_]*)', Number.Float),
            (r'0[xX][\da-fA-F][\da-fA-F_]*', Number.Hex),
            (r'0[oO][0-7][0-7_]*', Number.Oct),
            (r'0[bB][01][01_]*', Number.Binary),
            (r'\d[\d_]*', Number.Integer),

            (r"'(?:(\\[\\\"'ntbr ])|(\\[0-9]{3})|(\\x[0-9a-fA-F]{2}))'",
             String.Char),
            (r"'.'", String.Char),
            (r"'", Keyword), # a stray quote is another syntax element

            (r'"', String.Double, 'string'),

            (r'[~?][a-z][\w\']*:', Name.Variable),
        ],
        'comment': [
            (r'[^(*)]+', Comment),
            (r'\(\*', Comment, '#push'),
            (r'\*\)', Comment, '#pop'),
            (r'[(*)]', Comment),
        ],
        'string': [
            (r'[^\\"]+', String.Double),
            include('escape-sequence'),
            (r'\\\n', String.Double),
            (r'"', String.Double, '#pop'),
        ],
        'dotted': [
            (r'\s+', Text),
            (r'\.', Punctuation),
            (r'[A-Z][A-Za-z0-9_\']*(?=\s*\.)', Name.Namespace),
            (r'[A-Z][A-Za-z0-9_\']*', Name.Class, '#pop'),
            (r'[a-z_][A-Za-z0-9_\']*', Name, '#pop'),
        ],
    }


class ErlangLexer(RegexLexer):
    """
    For the Erlang functional programming language.

    Blame Jeremy Thurgood (http://jerith.za.net/).

    *New in Pygments 0.9.*
    """

    name = 'Erlang'
    aliases = ['erlang']
    filenames = ['*.erl', '*.hrl', '*.es', '*.escript']
    mimetypes = ['text/x-erlang']

    keywords = [
        'after', 'begin', 'case', 'catch', 'cond', 'end', 'fun', 'if',
        'let', 'of', 'query', 'receive', 'try', 'when',
        ]

    builtins = [ # See erlang(3) man page
        'abs', 'append_element', 'apply', 'atom_to_list', 'binary_to_list',
        'bitstring_to_list', 'binary_to_term', 'bit_size', 'bump_reductions',
        'byte_size', 'cancel_timer', 'check_process_code', 'delete_module',
        'demonitor', 'disconnect_node', 'display', 'element', 'erase', 'exit',
        'float', 'float_to_list', 'fun_info', 'fun_to_list',
        'function_exported', 'garbage_collect', 'get', 'get_keys',
        'group_leader', 'hash', 'hd', 'integer_to_list', 'iolist_to_binary',
        'iolist_size', 'is_atom', 'is_binary', 'is_bitstring', 'is_boolean',
        'is_builtin', 'is_float', 'is_function', 'is_integer', 'is_list',
        'is_number', 'is_pid', 'is_port', 'is_process_alive', 'is_record',
        'is_reference', 'is_tuple', 'length', 'link', 'list_to_atom',
        'list_to_binary', 'list_to_bitstring', 'list_to_existing_atom',
        'list_to_float', 'list_to_integer', 'list_to_pid', 'list_to_tuple',
        'load_module', 'localtime_to_universaltime', 'make_tuple', 'md5',
        'md5_final', 'md5_update', 'memory', 'module_loaded', 'monitor',
        'monitor_node', 'node', 'nodes', 'open_port', 'phash', 'phash2',
        'pid_to_list', 'port_close', 'port_command', 'port_connect',
        'port_control', 'port_call', 'port_info', 'port_to_list',
        'process_display', 'process_flag', 'process_info', 'purge_module',
        'put', 'read_timer', 'ref_to_list', 'register', 'resume_process',
        'round', 'send', 'send_after', 'send_nosuspend', 'set_cookie',
        'setelement', 'size', 'spawn', 'spawn_link', 'spawn_monitor',
        'spawn_opt', 'split_binary', 'start_timer', 'statistics',
        'suspend_process', 'system_flag', 'system_info', 'system_monitor',
        'system_profile', 'term_to_binary', 'tl', 'trace', 'trace_delivered',
        'trace_info', 'trace_pattern', 'trunc', 'tuple_size', 'tuple_to_list',
        'universaltime_to_localtime', 'unlink', 'unregister', 'whereis'
        ]

    operators = r'(\+\+?|--?|\*|/|<|>|/=|=:=|=/=|=<|>=|==?|<-|!|\?)'
    word_operators = [
        'and', 'andalso', 'band', 'bnot', 'bor', 'bsl', 'bsr', 'bxor',
        'div', 'not', 'or', 'orelse', 'rem', 'xor'
        ]

    atom_re = r"(?:[a-z][a-zA-Z0-9_]*|'[^\n']*[^\\]')"

    variable_re = r'(?:[A-Z_][a-zA-Z0-9_]*)'

    escape_re = r'(?:\\(?:[bdefnrstv\'"\\/]|[0-7][0-7]?[0-7]?|\^[a-zA-Z]))'

    macro_re = r'(?:'+variable_re+r'|'+atom_re+r')'

    base_re = r'(?:[2-9]|[12][0-9]|3[0-6])'

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'%.*\n', Comment),
            ('(' + '|'.join(keywords) + r')\b', Keyword),
            ('(' + '|'.join(builtins) + r')\b', Name.Builtin),
            ('(' + '|'.join(word_operators) + r')\b', Operator.Word),
            (r'^-', Punctuation, 'directive'),
            (operators, Operator),
            (r'"', String, 'string'),
            (r'<<', Name.Label),
            (r'>>', Name.Label),
            ('(' + atom_re + ')(:)', bygroups(Name.Namespace, Punctuation)),
            ('(?:^|(?<=:))(' + atom_re + r')(\s*)(\()',
             bygroups(Name.Function, Text, Punctuation)),
            (r'[+-]?'+base_re+r'#[0-9a-zA-Z]+', Number.Integer),
            (r'[+-]?\d+', Number.Integer),
            (r'[+-]?\d+.\d+', Number.Float),
            (r'[]\[:_@\".{}()|;,]', Punctuation),
            (variable_re, Name.Variable),
            (atom_re, Name),
            (r'\?'+macro_re, Name.Constant),
            (r'\$(?:'+escape_re+r'|\\[ %]|[^\\])', String.Char),
            (r'#'+atom_re+r'(:?\.'+atom_re+r')?', Name.Label),
            ],
        'string': [
            (escape_re, String.Escape),
            (r'"', String, '#pop'),
            (r'~[0-9.*]*[~#+bBcdefginpPswWxX]', String.Interpol),
            (r'[^"\\~]+', String),
            (r'~', String),
            ],
        'directive': [
            (r'(define)(\s*)(\()('+macro_re+r')',
             bygroups(Name.Entity, Text, Punctuation, Name.Constant), '#pop'),
            (r'(record)(\s*)(\()('+macro_re+r')',
             bygroups(Name.Entity, Text, Punctuation, Name.Label), '#pop'),
            (atom_re, Name.Entity, '#pop'),
            ],
        }


class ErlangShellLexer(Lexer):
    """
    Shell sessions in erl (for Erlang code).

    *New in Pygments 1.1.*
    """
    name = 'Erlang erl session'
    aliases = ['erl']
    filenames = ['*.erl-sh']
    mimetypes = ['text/x-erl-shellsession']

    _prompt_re = re.compile(r'\d+>(?=\s|\Z)')

    def get_tokens_unprocessed(self, text):
        erlexer = ErlangLexer(**self.options)

        curcode = ''
        insertions = []
        for match in line_re.finditer(text):
            line = match.group()
            m = self._prompt_re.match(line)
            if m is not None:
                end = m.end()
                insertions.append((len(curcode),
                                   [(0, Generic.Prompt, line[:end])]))
                curcode += line[end:]
            else:
                if curcode:
                    for item in do_insertions(insertions,
                                    erlexer.get_tokens_unprocessed(curcode)):
                        yield item
                    curcode = ''
                    insertions = []
                if line.startswith('*'):
                    yield match.start(), Generic.Traceback, line
                else:
                    yield match.start(), Generic.Output, line
        if curcode:
            for item in do_insertions(insertions,
                                      erlexer.get_tokens_unprocessed(curcode)):
                yield item


class OpaLexer(RegexLexer):
    """
    Lexer for the Opa language (http://opalang.org).

    *New in Pygments 1.5.*
    """

    name = 'Opa'
    aliases = ['opa']
    filenames = ['*.opa']
    mimetypes = ['text/x-opa']

    # most of these aren't strictly keywords
    # but if you color only real keywords, you might just
    # as well not color anything
    keywords = [
        'and', 'as', 'begin', 'css', 'database', 'db', 'do', 'else', 'end',
        'external', 'forall', 'if', 'import', 'match', 'package', 'parser',
        'rec', 'server', 'then', 'type', 'val', 'with', 'xml_parser',
    ]

    # matches both stuff and `stuff`
    ident_re = r'(([a-zA-Z_]\w*)|(`[^`]*`))'

    op_re = r'[.=\-<>,@~%/+?*&^!]'
    punc_re = r'[()\[\],;|]' # '{' and '}' are treated elsewhere
                               # because they are also used for inserts

    tokens = {
        # copied from the caml lexer, should be adapted
        'escape-sequence': [
            (r'\\[\\\"\'ntr}]', String.Escape),
            (r'\\[0-9]{3}', String.Escape),
            (r'\\x[0-9a-fA-F]{2}', String.Escape),
        ],

        # factorizing these rules, because they are inserted many times
        'comments': [
            (r'/\*', Comment, 'nested-comment'),
            (r'//.*?$', Comment),
        ],
        'comments-and-spaces': [
            include('comments'),
            (r'\s+', Text),
        ],

        'root': [
            include('comments-and-spaces'),
            # keywords
            (r'\b(%s)\b' % '|'.join(keywords), Keyword),
            # directives
            # we could parse the actual set of directives instead of anything
            # starting with @, but this is troublesome
            # because it needs to be adjusted all the time
            # and assuming we parse only sources that compile, it is useless
            (r'@'+ident_re+r'\b', Name.Builtin.Pseudo),

            # number literals
            (r'-?.[\d]+([eE][+\-]?\d+)', Number.Float),
            (r'-?\d+.\d*([eE][+\-]?\d+)', Number.Float),
            (r'-?\d+[eE][+\-]?\d+', Number.Float),
            (r'0[xX][\da-fA-F]+', Number.Hex),
            (r'0[oO][0-7]+', Number.Oct),
            (r'0[bB][01]+', Number.Binary),
            (r'\d+', Number.Integer),
            # color literals
            (r'#[\da-fA-F]{3,6}', Number.Integer),

            # string literals
            (r'"', String.Double, 'string'),
            # char literal, should be checked because this is the regexp from
            # the caml lexer
            (r"'(?:(\\[\\\"'ntbr ])|(\\[0-9]{3})|(\\x[0-9a-fA-F]{2})|.)'",
             String.Char),

            # this is meant to deal with embedded exprs in strings
            # every time we find a '}' we pop a state so that if we were
            # inside a string, we are back in the string state
            # as a consequence, we must also push a state every time we find a
            # '{' or else we will have errors when parsing {} for instance
            (r'{', Operator, '#push'),
            (r'}', Operator, '#pop'),

            # html literals
            # this is a much more strict that the actual parser,
            # since a<b would not be parsed as html
            # but then again, the parser is way too lax, and we can't hope
            # to have something as tolerant
            (r'<(?=[a-zA-Z>])', String.Single, 'html-open-tag'),

            # db path
            # matching the '[_]' in '/a[_]' because it is a part
            # of the syntax of the db path definition
            # unfortunately, i don't know how to match the ']' in
            # /a[1], so this is somewhat inconsistent
            (r'[@?!]?(/\w+)+(\[_\])?', Name.Variable),
            # putting the same color on <- as on db path, since
            # it can be used only to mean Db.write
            (r'<-(?!'+op_re+r')', Name.Variable),

            # 'modules'
            # although modules are not distinguished by their names as in caml
            # the standard library seems to follow the convention that modules
            # only area capitalized
            (r'\b([A-Z]\w*)(?=\.)', Name.Namespace),

            # operators
            # = has a special role because this is the only
            # way to syntactic distinguish binding constructions
            # unfortunately, this colors the equal in {x=2} too
            (r'=(?!'+op_re+r')', Keyword),
            (r'(%s)+' % op_re, Operator),
            (r'(%s)+' % punc_re, Operator),

            # coercions
            (r':', Operator, 'type'),
            # type variables
            # we need this rule because we don't parse specially type
            # definitions so in "type t('a) = ...", "'a" is parsed by 'root'
            ("'"+ident_re, Keyword.Type),

            # id literal, #something, or #{expr}
            (r'#'+ident_re, String.Single),
            (r'#(?={)', String.Single),

            # identifiers
            # this avoids to color '2' in 'a2' as an integer
            (ident_re, Text),

            # default, not sure if that is needed or not
            # (r'.', Text),
        ],

        # it is quite painful to have to parse types to know where they end
        # this is the general rule for a type
        # a type is either:
        # * -> ty
        # * type-with-slash
        # * type-with-slash -> ty
        # * type-with-slash (, type-with-slash)+ -> ty
        #
        # the code is pretty funky in here, but this code would roughly
        # translate in caml to:
        # let rec type stream =
        # match stream with
        # | [< "->";  stream >] -> type stream
        # | [< "";  stream >] ->
        #   type_with_slash stream
        #   type_lhs_1 stream;
        # and type_1 stream = ...
        'type': [
            include('comments-and-spaces'),
            (r'->', Keyword.Type),
            (r'', Keyword.Type, ('#pop', 'type-lhs-1', 'type-with-slash')),
        ],

        # parses all the atomic or closed constructions in the syntax of type
        # expressions: record types, tuple types, type constructors, basic type
        # and type variables
        'type-1': [
            include('comments-and-spaces'),
            (r'\(', Keyword.Type, ('#pop', 'type-tuple')),
            (r'~?{', Keyword.Type, ('#pop', 'type-record')),
            (ident_re+r'\(', Keyword.Type, ('#pop', 'type-tuple')),
            (ident_re, Keyword.Type, '#pop'),
            ("'"+ident_re, Keyword.Type),
            # this case is not in the syntax but sometimes
            # we think we are parsing types when in fact we are parsing
            # some css, so we just pop the states until we get back into
            # the root state
            (r'', Keyword.Type, '#pop'),
        ],

        # type-with-slash is either:
        # * type-1
        # * type-1 (/ type-1)+
        'type-with-slash': [
            include('comments-and-spaces'),
            (r'', Keyword.Type, ('#pop', 'slash-type-1', 'type-1')),
        ],
        'slash-type-1': [
            include('comments-and-spaces'),
            ('/', Keyword.Type, ('#pop', 'type-1')),
            # same remark as above
            (r'', Keyword.Type, '#pop'),
        ],

        # we go in this state after having parsed a type-with-slash
        # while trying to parse a type
        # and at this point we must determine if we are parsing an arrow
        # type (in which case we must continue parsing) or not (in which
        # case we stop)
        'type-lhs-1': [
            include('comments-and-spaces'),
            (r'->', Keyword.Type, ('#pop', 'type')),
            (r'(?=,)', Keyword.Type, ('#pop', 'type-arrow')),
            (r'', Keyword.Type, '#pop'),
        ],
        'type-arrow': [
            include('comments-and-spaces'),
            # the look ahead here allows to parse f(x : int, y : float -> truc)
            # correctly
            (r',(?=[^:]*?->)', Keyword.Type, 'type-with-slash'),
            (r'->', Keyword.Type, ('#pop', 'type')),
            # same remark as above
            (r'', Keyword.Type, '#pop'),
        ],

        # no need to do precise parsing for tuples and records
        # because they are closed constructions, so we can simply
        # find the closing delimiter
        # note that this function would be not work if the source
        # contained identifiers like `{)` (although it could be patched
        # to support it)
        'type-tuple': [
            include('comments-and-spaces'),
            (r'[^\(\)/*]+', Keyword.Type),
            (r'[/*]', Keyword.Type),
            (r'\(', Keyword.Type, '#push'),
            (r'\)', Keyword.Type, '#pop'),
        ],
        'type-record': [
            include('comments-and-spaces'),
            (r'[^{}/*]+', Keyword.Type),
            (r'[/*]', Keyword.Type),
            (r'{', Keyword.Type, '#push'),
            (r'}', Keyword.Type, '#pop'),
        ],

#        'type-tuple': [
#            include('comments-and-spaces'),
#            (r'\)', Keyword.Type, '#pop'),
#            (r'', Keyword.Type, ('#pop', 'type-tuple-1', 'type-1')),
#        ],
#        'type-tuple-1': [
#            include('comments-and-spaces'),
#            (r',?\s*\)', Keyword.Type, '#pop'), # ,) is a valid end of tuple, in (1,)
#            (r',', Keyword.Type, 'type-1'),
#        ],
#        'type-record':[
#            include('comments-and-spaces'),
#            (r'}', Keyword.Type, '#pop'),
#            (r'~?(?:\w+|`[^`]*`)', Keyword.Type, 'type-record-field-expr'),
#        ],
#        'type-record-field-expr': [
#
#        ],

        'nested-comment': [
            (r'[^/*]+', Comment),
            (r'/\*', Comment, '#push'),
            (r'\*/', Comment, '#pop'),
            (r'[/*]', Comment),
        ],

        # the copy pasting between string and single-string
        # is kinda sad. Is there a way to avoid that??
        'string': [
            (r'[^\\"{]+', String.Double),
            (r'"', String.Double, '#pop'),
            (r'{', Operator, 'root'),
            include('escape-sequence'),
        ],
        'single-string': [
            (r'[^\\\'{]+', String.Double),
            (r'\'', String.Double, '#pop'),
            (r'{', Operator, 'root'),
            include('escape-sequence'),
        ],

        # all the html stuff
        # can't really reuse some existing html parser
        # because we must be able to parse embedded expressions

        # we are in this state after someone parsed the '<' that
        # started the html literal
        'html-open-tag': [
            (r'[\w\-:]+', String.Single, ('#pop', 'html-attr')),
            (r'>', String.Single, ('#pop', 'html-content')),
        ],

        # we are in this state after someone parsed the '</' that
        # started the end of the closing tag
        'html-end-tag': [
            # this is a star, because </> is allowed
            (r'[\w\-:]*>', String.Single, '#pop'),
        ],

        # we are in this state after having parsed '<ident(:ident)?'
        # we thus parse a possibly empty list of attributes
        'html-attr': [
            (r'\s+', Text),
            (r'[\w\-:]+=', String.Single, 'html-attr-value'),
            (r'/>', String.Single, '#pop'),
            (r'>', String.Single, ('#pop', 'html-content')),
        ],

        'html-attr-value': [
            (r"'", String.Single, ('#pop', 'single-string')),
            (r'"', String.Single, ('#pop', 'string')),
            (r'#'+ident_re, String.Single, '#pop'),
            (r'#(?={)', String.Single, ('#pop', 'root')),
            (r'[^"\'{`=<>]+', String.Single, '#pop'),
            (r'{', Operator, ('#pop', 'root')), # this is a tail call!
        ],

        # we should probably deal with '\' escapes here
        'html-content': [
            (r'<!--', Comment, 'html-comment'),
            (r'</', String.Single, ('#pop', 'html-end-tag')),
            (r'<', String.Single, 'html-open-tag'),
            (r'{', Operator, 'root'),
            (r'[^<{]+', String.Single),
        ],

        'html-comment': [
            (r'-->', Comment, '#pop'),
            (r'[^\-]+|-', Comment),
        ],
    }


class CoqLexer(RegexLexer):
    """
    For the `Coq <http://coq.inria.fr/>`_ theorem prover.

    *New in Pygments 1.5.*
    """

    name = 'Coq'
    aliases = ['coq']
    filenames = ['*.v']
    mimetypes = ['text/x-coq']

    keywords1 = [
        # Vernacular commands
        'Section', 'Module', 'End', 'Require', 'Import', 'Export', 'Variable',
        'Variables', 'Parameter', 'Parameters', 'Axiom', 'Hypothesis',
        'Hypotheses', 'Notation', 'Local', 'Tactic', 'Reserved', 'Scope',
        'Open', 'Close', 'Bind', 'Delimit', 'Definition', 'Let', 'Ltac',
        'Fixpoint', 'CoFixpoint', 'Morphism', 'Relation', 'Implicit',
        'Arguments', 'Set', 'Unset', 'Contextual', 'Strict', 'Prenex',
        'Implicits', 'Inductive', 'CoInductive', 'Record', 'Structure',
        'Canonical', 'Coercion', 'Theorem', 'Lemma', 'Corollary',
        'Proposition', 'Fact', 'Remark', 'Example', 'Proof', 'Goal', 'Save',
        'Qed', 'Defined', 'Hint', 'Resolve', 'Rewrite', 'View', 'Search',
        'Show', 'Print', 'Printing', 'All', 'Graph', 'Projections', 'inside',
        'outside',
    ]
    keywords2 = [
        # Gallina
        'forall', 'exists', 'exists2', 'fun', 'fix', 'cofix', 'struct',
        'match', 'end',  'in', 'return', 'let', 'if', 'is', 'then', 'else',
        'for', 'of', 'nosimpl', 'with', 'as',
    ]
    keywords3 = [
        # Sorts
        'Type', 'Prop',
    ]
    keywords4 = [
        # Tactics
        'pose', 'set', 'move', 'case', 'elim', 'apply', 'clear', 'hnf', 'intro',
        'intros', 'generalize', 'rename', 'pattern', 'after', 'destruct',
        'induction', 'using', 'refine', 'inversion', 'injection', 'rewrite',
        'congr', 'unlock', 'compute', 'ring', 'field', 'replace', 'fold',
        'unfold', 'change', 'cutrewrite', 'simpl', 'have', 'suff', 'wlog',
        'suffices', 'without', 'loss', 'nat_norm', 'assert', 'cut', 'trivial',
        'revert', 'bool_congr', 'nat_congr', 'symmetry', 'transitivity', 'auto',
        'split', 'left', 'right', 'autorewrite',
    ]
    keywords5 = [
        # Terminators
        'by', 'done', 'exact', 'reflexivity', 'tauto', 'romega', 'omega',
        'assumption', 'solve', 'contradiction', 'discriminate',
    ]
    keywords6 = [
        # Control
        'do', 'last', 'first', 'try', 'idtac', 'repeat',
    ]
      # 'as', 'assert', 'begin', 'class', 'constraint', 'do', 'done',
      # 'downto', 'else', 'end', 'exception', 'external', 'false',
      # 'for', 'fun', 'function', 'functor', 'if', 'in', 'include',
      # 'inherit', 'initializer', 'lazy', 'let', 'match', 'method',
      # 'module', 'mutable', 'new', 'object', 'of', 'open', 'private',
      # 'raise', 'rec', 'sig', 'struct', 'then', 'to', 'true', 'try',
      # 'type', 'val', 'virtual', 'when', 'while', 'with'
    keyopts = [
        '!=', '#', '&', '&&', r'\(', r'\)', r'\*', r'\+', ',', '-',
        r'-\.', '->', r'\.', r'\.\.', ':', '::', ':=', ':>', ';', ';;', '<',
        '<-', '=', '>', '>]', '>}', r'\?', r'\?\?', r'\[', r'\[<', r'\[>',
        r'\[\|', ']', '_', '`', '{', '{<', r'\|', r'\|]', '}', '~', '=>',
        r'/\\', r'\\/',
        u'Π', u'λ',
    ]
    operators = r'[!$%&*+\./:<=>?@^|~-]'
    word_operators = ['and', 'asr', 'land', 'lor', 'lsl', 'lxor', 'mod', 'or']
    prefix_syms = r'[!?~]'
    infix_syms = r'[=<>@^|&+\*/$%-]'
    primitives = ['unit', 'int', 'float', 'bool', 'string', 'char', 'list',
                  'array']

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'false|true|\(\)|\[\]', Name.Builtin.Pseudo),
            (r'\(\*', Comment, 'comment'),
            (r'\b(%s)\b' % '|'.join(keywords1), Keyword.Namespace),
            (r'\b(%s)\b' % '|'.join(keywords2), Keyword),
            (r'\b(%s)\b' % '|'.join(keywords3), Keyword.Type),
            (r'\b(%s)\b' % '|'.join(keywords4), Keyword),
            (r'\b(%s)\b' % '|'.join(keywords5), Keyword.Pseudo),
            (r'\b(%s)\b' % '|'.join(keywords6), Keyword.Reserved),
            (r'\b([A-Z][A-Za-z0-9_\']*)(?=\s*\.)',
             Name.Namespace, 'dotted'),
            (r'\b([A-Z][A-Za-z0-9_\']*)', Name.Class),
            (r'(%s)' % '|'.join(keyopts[::-1]), Operator),
            (r'(%s|%s)?%s' % (infix_syms, prefix_syms, operators), Operator),
            (r'\b(%s)\b' % '|'.join(word_operators), Operator.Word),
            (r'\b(%s)\b' % '|'.join(primitives), Keyword.Type),

            (r"[^\W\d][\w']*", Name),

            (r'\d[\d_]*', Number.Integer),
            (r'0[xX][\da-fA-F][\da-fA-F_]*', Number.Hex),
            (r'0[oO][0-7][0-7_]*', Number.Oct),
            (r'0[bB][01][01_]*', Number.Binary),
            (r'-?\d[\d_]*(.[\d_]*)?([eE][+\-]?\d[\d_]*)', Number.Float),

            (r"'(?:(\\[\\\"'ntbr ])|(\\[0-9]{3})|(\\x[0-9a-fA-F]{2}))'",
             String.Char),
            (r"'.'", String.Char),
            (r"'", Keyword), # a stray quote is another syntax element

            (r'"', String.Double, 'string'),

            (r'[~?][a-z][\w\']*:', Name.Variable),
        ],
        'comment': [
            (r'[^(*)]+', Comment),
            (r'\(\*', Comment, '#push'),
            (r'\*\)', Comment, '#pop'),
            (r'[(*)]', Comment),
        ],
        'string': [
            (r'[^"]+', String.Double),
            (r'""', String.Double),
            (r'"', String.Double, '#pop'),
        ],
        'dotted': [
            (r'\s+', Text),
            (r'\.', Punctuation),
            (r'[A-Z][A-Za-z0-9_\']*(?=\s*\.)', Name.Namespace),
            (r'[A-Z][A-Za-z0-9_\']*', Name.Class, '#pop'),
            (r'[a-z][a-z0-9_\']*', Name, '#pop'),
            (r'', Text, '#pop')
        ],
    }

    def analyse_text(text):
        if text.startswith('(*'):
            return True


class NewLispLexer(RegexLexer):
    """
    For `newLISP. <www.newlisp.org>`_ source code (version 10.3.0).

    *New in Pygments 1.5.*
    """

    name = 'NewLisp'
    aliases = ['newlisp']
    filenames = ['*.lsp', '*.nl']
    mimetypes = ['text/x-newlisp', 'application/x-newlisp']

    flags = re.IGNORECASE | re.MULTILINE | re.UNICODE

    # list of built-in functions for newLISP version 10.3
    builtins = [
        '^', '--', '-', ':', '!', '!=', '?', '@', '*', '/', '&', '%', '+', '++',
        '<', '<<', '<=', '=', '>', '>=', '>>', '|', '~', '$', '$0', '$1', '$10',
        '$11', '$12', '$13', '$14', '$15', '$2', '$3', '$4', '$5', '$6', '$7',
        '$8', '$9', '$args', '$idx', '$it', '$main-args', 'abort', 'abs',
        'acos', 'acosh', 'add', 'address', 'amb', 'and',  'and', 'append-file',
        'append', 'apply', 'args', 'array-list', 'array?', 'array', 'asin',
        'asinh', 'assoc', 'atan', 'atan2', 'atanh', 'atom?', 'base64-dec',
        'base64-enc', 'bayes-query', 'bayes-train', 'begin', 'begin', 'begin',
        'beta', 'betai', 'bind', 'binomial', 'bits', 'callback', 'case', 'case',
        'case', 'catch', 'ceil', 'change-dir', 'char', 'chop', 'Class', 'clean',
        'close', 'command-event', 'cond', 'cond', 'cond', 'cons', 'constant',
        'context?', 'context', 'copy-file', 'copy', 'cos', 'cosh', 'count',
        'cpymem', 'crc32', 'crit-chi2', 'crit-z', 'current-line', 'curry',
        'date-list', 'date-parse', 'date-value', 'date', 'debug', 'dec',
        'def-new', 'default', 'define-macro', 'define-macro', 'define',
        'delete-file', 'delete-url', 'delete', 'destroy', 'det', 'device',
        'difference', 'directory?', 'directory', 'div', 'do-until', 'do-while',
        'doargs',  'dolist',  'dostring', 'dotimes',  'dotree', 'dump', 'dup',
        'empty?', 'encrypt', 'ends-with', 'env', 'erf', 'error-event',
        'eval-string', 'eval', 'exec', 'exists', 'exit', 'exp', 'expand',
        'explode', 'extend', 'factor', 'fft', 'file-info', 'file?', 'filter',
        'find-all', 'find', 'first', 'flat', 'float?', 'float', 'floor', 'flt',
        'fn', 'for-all', 'for', 'fork', 'format', 'fv', 'gammai', 'gammaln',
        'gcd', 'get-char', 'get-float', 'get-int', 'get-long', 'get-string',
        'get-url', 'global?', 'global', 'if-not', 'if', 'ifft', 'import', 'inc',
        'index', 'inf?', 'int', 'integer?', 'integer', 'intersect', 'invert',
        'irr', 'join', 'lambda-macro', 'lambda?', 'lambda', 'last-error',
        'last', 'legal?', 'length', 'let', 'let', 'let', 'letex', 'letn',
        'letn', 'letn', 'list?', 'list', 'load', 'local', 'log', 'lookup',
        'lower-case', 'macro?', 'main-args', 'MAIN', 'make-dir', 'map', 'mat',
        'match', 'max', 'member', 'min', 'mod', 'module', 'mul', 'multiply',
        'NaN?', 'net-accept', 'net-close', 'net-connect', 'net-error',
        'net-eval', 'net-interface', 'net-ipv', 'net-listen', 'net-local',
        'net-lookup', 'net-packet', 'net-peek', 'net-peer', 'net-ping',
        'net-receive-from', 'net-receive-udp', 'net-receive', 'net-select',
        'net-send-to', 'net-send-udp', 'net-send', 'net-service',
        'net-sessions', 'new', 'nil?', 'nil', 'normal', 'not', 'now', 'nper',
        'npv', 'nth', 'null?', 'number?', 'open', 'or', 'ostype', 'pack',
        'parse-date', 'parse', 'peek', 'pipe', 'pmt', 'pop-assoc', 'pop',
        'post-url', 'pow', 'prefix', 'pretty-print', 'primitive?', 'print',
        'println', 'prob-chi2', 'prob-z', 'process', 'prompt-event',
        'protected?', 'push', 'put-url', 'pv', 'quote?', 'quote', 'rand',
        'random', 'randomize', 'read', 'read-char', 'read-expr', 'read-file',
        'read-key', 'read-line', 'read-utf8', 'read', 'reader-event',
        'real-path', 'receive', 'ref-all', 'ref', 'regex-comp', 'regex',
        'remove-dir', 'rename-file', 'replace', 'reset', 'rest', 'reverse',
        'rotate', 'round', 'save', 'search', 'seed', 'seek', 'select', 'self',
        'semaphore', 'send', 'sequence', 'series', 'set-locale', 'set-ref-all',
        'set-ref', 'set', 'setf',  'setq', 'sgn', 'share', 'signal', 'silent',
        'sin', 'sinh', 'sleep', 'slice', 'sort', 'source', 'spawn', 'sqrt',
        'starts-with', 'string?', 'string', 'sub', 'swap', 'sym', 'symbol?',
        'symbols', 'sync', 'sys-error', 'sys-info', 'tan', 'tanh', 'term',
        'throw-error', 'throw', 'time-of-day', 'time', 'timer', 'title-case',
        'trace-highlight', 'trace', 'transpose', 'Tree', 'trim', 'true?',
        'true', 'unicode', 'unify', 'unique', 'unless', 'unpack', 'until',
        'upper-case', 'utf8', 'utf8len', 'uuid', 'wait-pid', 'when', 'while',
        'write', 'write-char', 'write-file', 'write-line', 'write',
        'xfer-event', 'xml-error', 'xml-parse', 'xml-type-tags', 'zero?',
    ]

    # valid names
    valid_name = r'([a-zA-Z0-9!$%&*+.,/<=>?@^_~|-])+|(\[.*?\])+'

    tokens = {
        'root': [
            # shebang
            (r'#!(.*?)$', Comment.Preproc),
            # comments starting with semicolon
            (r';.*$', Comment.Single),
            # comments starting with #
            (r'#.*$', Comment.Single),

            # whitespace
            (r'\s+', Text),

            # strings, symbols and characters
            (r'"(\\\\|\\"|[^"])*"', String),

            # braces
            (r"{", String, "bracestring"),

            # [text] ... [/text] delimited strings
            (r'\[text\]*', String, "tagstring"),

            # 'special' operators...
            (r"('|:)", Operator),

            # highlight the builtins
            ('(%s)' % '|'.join(re.escape(entry) + '\\b' for entry in builtins),
             Keyword),

            # the remaining functions
            (r'(?<=\()' + valid_name, Name.Variable),

            # the remaining variables
            (valid_name, String.Symbol),

            # parentheses
            (r'(\(|\))', Punctuation),
        ],

        # braced strings...
        'bracestring': [
             ("{", String, "#push"),
             ("}", String, "#pop"),
             ("[^{}]+", String),
        ],

        # tagged [text]...[/text] delimited strings...
        'tagstring': [
            (r'(?s)(.*?)(\[/text\])', String, '#pop'),
        ],
    }


class ElixirLexer(RegexLexer):
    """
    For the `Elixir language <http://elixir-lang.org>`_.

    *New in Pygments 1.5.*
    """

    name = 'Elixir'
    aliases = ['elixir', 'ex', 'exs']
    filenames = ['*.ex', '*.exs']
    mimetypes = ['text/x-elixir']

    def gen_elixir_sigil_rules():
        states = {}

        states['strings'] = [
            (r'(%[A-Ba-z])?"""(?:.|\n)*?"""', String.Doc),
            (r"'''(?:.|\n)*?'''", String.Doc),
            (r'"', String.Double, 'dqs'),
            (r"'.*'", String.Single),
            (r'(?<!\w)\?(\\(x\d{1,2}|\h{1,2}(?!\h)\b|0[0-7]{0,2}(?![0-7])\b|'
             r'[^x0MC])|(\\[MC]-)+\w|[^\s\\])', String.Other)
        ]

        for lbrace, rbrace, name, in ('\\{', '\\}', 'cb'), \
                                     ('\\[', '\\]', 'sb'), \
                                     ('\\(', '\\)', 'pa'), \
                                     ('\\<', '\\>', 'lt'):

            states['strings'] += [
                (r'%[a-z]' + lbrace, String.Double, name + 'intp'),
                (r'%[A-Z]' + lbrace, String.Double, name + 'no-intp')
            ]

            states[name +'intp'] = [
                (r'' + rbrace + '[a-z]*', String.Double, "#pop"),
                include('enddoublestr')
            ]

            states[name +'no-intp'] = [
                (r'.*' + rbrace + '[a-z]*', String.Double , "#pop")
            ]

        return states

    tokens = {
        'root': [
            (r'\s+', Text),
            (r'#.*$', Comment.Single),
            (r'\b(case|cond|end|bc|lc|if|unless|try|loop|receive|fn|defmodule|'
             r'defp?|defprotocol|defimpl|defrecord|defmacrop?|defdelegate|'
             r'defexception|exit|raise|throw|unless|after|rescue|catch|else)\b(?![?!])|'
             r'(?<!\.)\b(do|\-\>)\b\s*', Keyword),
            (r'\b(import|require|use|recur|quote|unquote|super|refer)\b(?![?!])',
                Keyword.Namespace),
            (r'(?<!\.)\b(and|not|or|when|xor|in)\b', Operator.Word),
            (r'%=|\*=|\*\*=|\+=|\-=|\^=|\|\|=|'
             r'<=>|<(?!<|=)|>(?!<|=|>)|<=|>=|===|==|=~|!=|!~|(?=[ \t])\?|'
             r'(?<=[ \t])!+|&&|\|\||\^|\*|\+|\-|/|'
             r'\||\+\+|\-\-|\*\*|\/\/|\<\-|\<\>|<<|>>|=|\.', Operator),
            (r'(?<!:)(:)([a-zA-Z_]\w*([?!]|=(?![>=]))?|\<\>|===?|>=?|<=?|'
             r'<=>|&&?|%\(\)|%\[\]|%\{\}|\+\+?|\-\-?|\|\|?|\!|//|[%&`/\|]|'
             r'\*\*?|=?~|<\-)|([a-zA-Z_]\w*([?!])?)(:)(?!:)', String.Symbol),
            (r':"', String.Symbol, 'interpoling_symbol'),
            (r'\b(nil|true|false)\b(?![?!])|\b[A-Z]\w*\b', Name.Constant),
            (r'\b(__(FILE|LINE|MODULE|MAIN|FUNCTION)__)\b(?![?!])', Name.Builtin.Pseudo),
            (r'[a-zA-Z_!][\w_]*[!\?]?', Name),
            (r'[(){};,/\|:\\\[\]]', Punctuation),
            (r'@[a-zA-Z_]\w*|&\d', Name.Variable),
            (r'\b(0[xX][0-9A-Fa-f]+|\d(_?\d)*(\.(?![^\d\s])'
             r'(_?\d)*)?([eE][-+]?\d(_?\d)*)?|0[bB][01]+)\b', Number),
            (r'%r\/.*\/', String.Regex),
            include('strings'),
        ],
        'dqs': [
            (r'"', String.Double, "#pop"),
            include('enddoublestr')
        ],
        'interpoling': [
            (r'#{', String.Interpol, 'interpoling_string'),
        ],
        'interpoling_string' : [
            (r'}', String.Interpol, "#pop"),
            include('root')
        ],
        'interpoling_symbol': [
            (r'"', String.Symbol, "#pop"),
            include('interpoling'),
            (r'[^#"]+', String.Symbol),
        ],
        'enddoublestr' : [
            include('interpoling'),
            (r'[^#"]+', String.Double),
        ]
    }
    tokens.update(gen_elixir_sigil_rules())


class ElixirConsoleLexer(Lexer):
    """
    For Elixir interactive console (iex) output like:

    .. sourcecode:: iex

        iex> [head | tail] = [1,2,3]
        [1,2,3]
        iex> head
        1
        iex> tail
        [2,3]
        iex> [head | tail]
        [1,2,3]
        iex> length [head | tail]
        3

    *New in Pygments 1.5.*
    """

    name = 'Elixir iex session'
    aliases = ['iex']
    mimetypes = ['text/x-elixir-shellsession']

    _prompt_re = re.compile('(iex|\.{3})> ')

    def get_tokens_unprocessed(self, text):
        exlexer = ElixirLexer(**self.options)

        curcode = ''
        insertions = []
        for match in line_re.finditer(text):
            line = match.group()
            if line.startswith(u'** '):
                insertions.append((len(curcode),
                                   [(0, Generic.Error, line[:-1])]))
                curcode += line[-1:]
            else:
                m = self._prompt_re.match(line)
                if m is not None:
                    end = m.end()
                    insertions.append((len(curcode),
                                       [(0, Generic.Prompt, line[:end])]))
                    curcode += line[end:]
                else:
                    if curcode:
                        for item in do_insertions(insertions,
                                        exlexer.get_tokens_unprocessed(curcode)):
                            yield item
                        curcode = ''
                        insertions = []
                    yield match.start(), Generic.Output, line
        if curcode:
            for item in do_insertions(insertions,
                                      exlexer.get_tokens_unprocessed(curcode)):
                yield item


class KokaLexer(RegexLexer):
    """
    Lexer for the `Koka <http://research.microsoft.com/en-us/projects/koka/>`_
    language.

    *New in Pygments 1.6.*
    """

    name = 'Koka'
    aliases = ['koka']
    filenames = ['*.kk', '*.kki']
    mimetypes = ['text/x-koka']

    keywords = [
        'infix', 'infixr', 'infixl', 'prefix', 'postfix',
        'type', 'cotype', 'rectype', 'alias',
        'struct', 'con',
        'fun', 'function', 'val', 'var',
        'external',
        'if', 'then', 'else', 'elif', 'return', 'match',
        'private', 'public', 'private',
        'module', 'import', 'as',
        'include', 'inline',
        'rec',
        'try', 'yield', 'enum',
        'interface', 'instance',
    ]

    # keywords that are followed by a type
    typeStartKeywords = [
        'type', 'cotype', 'rectype', 'alias', 'struct', 'enum',
    ]

    # keywords valid in a type
    typekeywords = [
        'forall', 'exists', 'some', 'with',
    ]

    # builtin names and special names
    builtin = [
        'for', 'while', 'repeat',
        'foreach', 'foreach-indexed',
        'error', 'catch', 'finally',
        'cs', 'js', 'file', 'ref', 'assigned',
    ]

    # symbols that can be in an operator
    symbols = '[\$%&\*\+@!/\\\^~=\.:\-\?\|<>]+'

    # symbol boundary: an operator keyword should not be followed by any of these
    sboundary = '(?!'+symbols+')'

    # name boundary: a keyword should not be followed by any of these
    boundary = '(?![a-zA-Z0-9_\\-])'

    # main lexer
    tokens = {
        'root': [
            include('whitespace'),

            # go into type mode
            (r'::?' + sboundary, Keyword.Type, 'type'),
            (r'alias' + boundary, Keyword, 'alias-type'),
            (r'struct' + boundary, Keyword, 'struct-type'),
            (r'(%s)' % '|'.join(typeStartKeywords) + boundary, Keyword, 'type'),

            # special sequences of tokens (we use ?: for non-capturing group as
            # required by 'bygroups')
            (r'(module)(\s*)((?:interface)?)(\s*)'
             r'((?:[a-z](?:[a-zA-Z0-9_]|\-[a-zA-Z])*\.)*'
             r'[a-z](?:[a-zA-Z0-9_]|\-[a-zA-Z])*)',
             bygroups(Keyword, Text, Keyword, Text, Name.Namespace)),
            (r'(import)(\s+)((?:[a-z](?:[a-zA-Z0-9_]|\-[a-zA-Z])*\.)*[a-z]'
             r'(?:[a-zA-Z0-9_]|\-[a-zA-Z])*)(\s*)((?:as)?)'
             r'((?:[A-Z](?:[a-zA-Z0-9_]|\-[a-zA-Z])*)?)',
             bygroups(Keyword, Text, Name.Namespace, Text, Keyword,
                      Name.Namespace)),

            # keywords
            (r'(%s)' % '|'.join(typekeywords) + boundary, Keyword.Type),
            (r'(%s)' % '|'.join(keywords) + boundary, Keyword),
            (r'(%s)' % '|'.join(builtin) + boundary, Keyword.Pseudo),
            (r'::|:=|\->|[=\.:]' + sboundary, Keyword),
            (r'\-' + sboundary, Generic.Strong),

            # names
            (r'[A-Z]([a-zA-Z0-9_]|\-[a-zA-Z])*(?=\.)', Name.Namespace),
            (r'[A-Z]([a-zA-Z0-9_]|\-[a-zA-Z])*(?!\.)', Name.Class),
            (r'[a-z]([a-zA-Z0-9_]|\-[a-zA-Z])*', Name),
            (r'_([a-zA-Z0-9_]|\-[a-zA-Z])*', Name.Variable),

            # literal string
            (r'@"', String.Double, 'litstring'),

            # operators
            (symbols, Operator),
            (r'`', Operator),
            (r'[\{\}\(\)\[\];,]', Punctuation),

            # literals. No check for literal characters with len > 1
            (r'[0-9]+\.[0-9]+([eE][\-\+]?[0-9]+)?', Number.Float),
            (r'0[xX][0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+', Number.Integer),

            (r"'", String.Char, 'char'),
            (r'"', String.Double, 'string'),
        ],

        # type started by alias
        'alias-type': [
            (r'=',Keyword),
            include('type')
        ],

        # type started by struct
        'struct-type': [
            (r'(?=\((?!,*\)))',Punctuation, '#pop'),
            include('type')
        ],

        # type started by colon
        'type': [
            (r'[\(\[<]', Keyword.Type, 'type-nested'),
            include('type-content')
        ],

        # type nested in brackets: can contain parameters, comma etc.
        'type-nested': [
            (r'[\)\]>]', Keyword.Type, '#pop'),
            (r'[\(\[<]', Keyword.Type, 'type-nested'),
            (r',', Keyword.Type),
            (r'([a-z](?:[a-zA-Z0-9_]|\-[a-zA-Z])*)(\s*)(:)(?!:)',
             bygroups(Name.Variable,Text,Keyword.Type)),  # parameter name
            include('type-content')
        ],

        # shared contents of a type
        'type-content': [
            include('whitespace'),

            # keywords
            (r'(%s)' % '|'.join(typekeywords) + boundary, Keyword.Type),
            (r'(?=((%s)' % '|'.join(keywords) + boundary + '))',
             Keyword, '#pop'),  # need to match because names overlap...

            # kinds
            (r'[EPH]' + boundary, Keyword.Type),
            (r'[*!]', Keyword.Type),

            # type names
            (r'[A-Z]([a-zA-Z0-9_]|\-[a-zA-Z])*(?=\.)', Name.Namespace),
            (r'[A-Z]([a-zA-Z0-9_]|\-[a-zA-Z])*(?!\.)', Name.Class),
            (r'[a-z][0-9]*(?![a-zA-Z_\-])', Keyword.Type),   # Generic.Emph
            (r'_([a-zA-Z0-9_]|\-[a-zA-Z])*', Keyword.Type),  # Generic.Emph
            (r'[a-z]([a-zA-Z0-9_]|\-[a-zA-Z])*', Keyword.Type),

            # type keyword operators
            (r'::|\->|[\.:|]', Keyword.Type),

            #catchall
            (r'', Text, '#pop')
        ],

        # comments and literals
        'whitespace': [
            (r'\s+', Text),
            (r'/\*', Comment.Multiline, 'comment'),
            (r'//.*$', Comment.Single)
        ],
        'comment': [
            (r'[^/\*]+', Comment.Multiline),
            (r'/\*', Comment.Multiline, '#push'),
            (r'\*/', Comment.Multiline, '#pop'),
            (r'[\*/]', Comment.Multiline),
        ],
        'litstring': [
            (r'[^"]+', String.Double),
            (r'""', String.Escape),
            (r'"', String.Double, '#pop'),
        ],
        'string': [
            (r'[^\\"\n]+', String.Double),
            include('escape-sequence'),
            (r'["\n]', String.Double, '#pop'),
        ],
        'char': [
            (r'[^\\\'\n]+', String.Char),
            include('escape-sequence'),
            (r'[\'\n]', String.Char, '#pop'),
        ],
        'escape-sequence': [
            (r'\\[abfnrtv0\\\"\'\?]', String.Escape),
            (r'\\x[0-9a-fA-F]{2}', String.Escape),
            (r'\\u[0-9a-fA-F]{4}', String.Escape),
            # Yes, \U literals are 6 hex digits.
            (r'\\U[0-9a-fA-F]{6}', String.Escape)
        ]
    }

