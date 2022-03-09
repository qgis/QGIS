#!/usr/bin/env perl
use strict;
use warnings;
use File::Basename;
use File::Spec;
use Getopt::Long;
use YAML::Tiny;
no if $] >= 5.018000, warnings => 'experimental::smartmatch';

use constant PRIVATE => 0;
use constant PROTECTED => 1;
use constant PUBLIC => 2;

use constant STRICT => 10;
use constant UNSTRICT => 11;

use constant MULTILINE_NO => 20;
use constant MULTILINE_METHOD => 21;
use constant MULTILINE_CONDITIONAL_STATEMENT => 22;

use constant CODE_SNIPPET => 30;
use constant CODE_SNIPPET_CPP => 31;

use constant PREPEND_CODE_NO => 40;
use constant PREPEND_CODE_VIRTUAL => 41;
use constant PREPEND_CODE_MAKE_PRIVATE => 42;


# read arguments
my $debug = 0;
my $sip_output = '';
my $python_output = '';
#my $SUPPORT_TEMPLATE_DOCSTRING = 0;
#die("usage: $0 [-debug] [-template-doc] headerfile\n") unless GetOptions ("debug" => \$debug, "template-doc" => \$SUPPORT_TEMPLATE_DOCSTRING) && @ARGV == 1;
die("usage: $0 [-debug] [-sip_output FILE] [-python_output FILE] headerfile\n")
  unless GetOptions ("debug" => \$debug, "sip_output=s" => \$sip_output, "python_output=s" => \$python_output) && @ARGV == 1;
my $headerfile = $ARGV[0];

# read file
open(my $handle, "<", $headerfile) || die "Couldn't open '".$headerfile."' for reading because: ".$!;
chomp(my @INPUT_LINES = <$handle>);
close $handle;

# config
my $cfg_file = File::Spec->catfile( dirname(__FILE__), '../python/sipify.yaml' );
my $yaml = YAML::Tiny->read( $cfg_file  );
my $SIP_CONFIG = $yaml->[0];

# contexts
my $SIP_RUN = 0;
my $HEADER_CODE = 0;
my @ACCESS = (PUBLIC);
my @CLASSNAME = ();
my @DECLARED_CLASSES = ();
my @EXPORTED = (0);
my $MULTILINE_DEFINITION = MULTILINE_NO;
my $ACTUAL_CLASS = '';
my $PYTHON_SIGNATURE = '';

my $INDENT = '';
my $PREV_INDENT = '';
my $COMMENT = '';
my $COMMENT_PARAM_LIST = 0;
my $COMMENT_LAST_LINE_NOTE_WARNING = 0;
my $COMMENT_CODE_SNIPPET = 0;
my $COMMENT_TEMPLATE_DOCSTRING = 0;
my @SKIPPED_PARAMS_OUT = ();
my @SKIPPED_PARAMS_REMOVE = ();
my $GLOB_IFDEF_NESTING_IDX = 0;
my @GLOB_BRACKET_NESTING_IDX = (0);
my $PRIVATE_SECTION_LINE = '';
my $LAST_ACCESS_SECTION_LINE = '';
my $RETURN_TYPE = '';
my $IS_OVERRIDE_OR_MAKE_PRIVATE = PREPEND_CODE_NO;
my $IF_FEATURE_CONDITION = '';
my $FOUND_SINCE = 0;
my %QFLAG_HASH;

my $LINE_COUNT = @INPUT_LINES;
my $LINE_IDX = 0;
my $LINE;
my @OUTPUT = ();
my @OUTPUT_PYTHON = ();
my $DOXY_INSIDE_SIP_RUN = 0;

sub read_line {
    my $new_line = $INPUT_LINES[$LINE_IDX];
    $LINE_IDX++;
    $debug == 0 or print sprintf('LIN:%d DEPTH:%d ACC:%d BRCK:%d SIP:%d MLT:%d OVR: %d CLSS: %s/%d',
                                  $LINE_IDX,
                                  $#ACCESS,
                                  $ACCESS[$#ACCESS],
                                  $GLOB_BRACKET_NESTING_IDX[$#GLOB_BRACKET_NESTING_IDX],
                                  $SIP_RUN,
                                  $MULTILINE_DEFINITION,
                                  $IS_OVERRIDE_OR_MAKE_PRIVATE,
                                  $ACTUAL_CLASS,
                                  $#CLASSNAME)." :: ".$new_line."\n";
   $new_line = replace_macros($new_line);
   return $new_line;
}

sub write_output {
    my ($dbg_code, $out, $prepend) = @_;
    $prepend //= "no";
    if ($debug == 1){
        $dbg_code = sprintf("%d %-4s :: ", $LINE_IDX, $dbg_code);
    }
    else{
        $dbg_code = '';
    }
    if ($prepend eq "prepend")
    {
       unshift @OUTPUT, $dbg_code . $out;
    }
    else
    {
      push @OUTPUT, "%If ($IF_FEATURE_CONDITION)\n" if $IF_FEATURE_CONDITION ne '';
      push @OUTPUT, $dbg_code . $out;
      push @OUTPUT, "%End\n" if $IF_FEATURE_CONDITION ne '';
    }
    $IF_FEATURE_CONDITION = '';
}

sub dbg_info {
    my $info = $_[0];
    if ($debug == 1){
        push @OUTPUT, $info."\n";
        print $LINE_IDX." ".@ACCESS." ".$SIP_RUN." ".$MULTILINE_DEFINITION." ".$info."\n";
    }
}

sub exit_with_error {
  die "! Sipify error in $headerfile at line :: $LINE_IDX\n! $_[0]\n";
}

sub sip_header_footer {
    my @header_footer = ();
    # small hack to turn files src/core/3d/X.h to src/core/./3d/X.h
    # otherwise "sip up to date" test fails. This is because the test uses %Include entries
    # and over there we have to use ./3d/X.h entries because SIP parser does not allow a number
    # as the first letter of a relative path
    my $headerfile_x = $headerfile;
    $headerfile_x =~ s/src\/core\/3d/src\/core\/.\/3d/;
    push @header_footer,  "/************************************************************************\n";
    push @header_footer,  " * This file has been generated automatically from                      *\n";
    push @header_footer,  " *                                                                      *\n";
    push @header_footer, sprintf " * %-*s *\n", 68, $headerfile_x;
    push @header_footer,  " *                                                                      *\n";
    push @header_footer,  " * Do not edit manually ! Edit header and run scripts/sipify.pl again   *\n";
    push @header_footer,  " ************************************************************************/\n";
    return @header_footer;
}

sub python_header {
    my @header = ();
    my $headerfile_x = $headerfile;
    $headerfile_x =~ s/src\/core\/3d/src\/core\/.\/3d/;
    push @header, "# The following has been generated automatically from ";
    push @header, sprintf "%s\n", $headerfile_x;
    return @header;
}

sub create_class_links {
    my $line = $_[0];

    if ( $line =~ m/\b(Qgs[A-Z]\w+)\b(\.?$|[^\w]{2})/) {
        if ( defined $ACTUAL_CLASS && $1 !~ $ACTUAL_CLASS ) {
            $line =~ s/\b(Qgs[A-Z]\w+)\b(\.?$|[^\w]{2})/:py:class:`$1`$2/g;
        }
    }
    $line =~ s/\b(Qgs[A-Z]\w+\.[a-z]\w+\(\))(?!\w)/:py:func:`$1`/g;
    if ( defined $ACTUAL_CLASS && $ACTUAL_CLASS) {
        $line =~ s/(?<!\.)\b(?:([a-z]\w+)\(\))(?!\w)/:py:func:`~$ACTUAL_CLASS.$1`/g;
    }
    else {
        $line =~ s/(?<!\.)\b(?:([a-z]\w+)\(\))(?!\w)/:py:func:`~$1`/g;
    }

    if ( $line =~ m/\b(?<![`~])(Qgs[A-Z]\w+)\b(?!\()/) {
        if ( (!$ACTUAL_CLASS) || $1 ne $ACTUAL_CLASS ) {
            $line =~ s/\b(?<![`~])(Qgs[A-Z]\w+)\b(?!\()/:py:class:`$1`/g;
        }
    }

    return $line;
}

sub processDoxygenLine {
    my $line = $_[0];

    if ( $line =~ m/\s*#ifdef SIP_RUN/ ) {
      $DOXY_INSIDE_SIP_RUN = 1;
      return "";
    }
    elsif ( $line =~ m/\s*#ifndef SIP_RUN/ ) {
      $DOXY_INSIDE_SIP_RUN = 2;
      return "";
    }
    elsif ($DOXY_INSIDE_SIP_RUN != 0 && $line =~ m/\s*#else/ ) {
      $DOXY_INSIDE_SIP_RUN = $DOXY_INSIDE_SIP_RUN == 1 ? 2 : 1;
      return "";
    }
    elsif ($DOXY_INSIDE_SIP_RUN != 0 && $line =~ m/\s*#endif/ ) {
      $DOXY_INSIDE_SIP_RUN = 0;
      return "";
    }

    if ($DOXY_INSIDE_SIP_RUN == 2) {
      return "";
    }

    # detect code snippet
    if ( $line =~ m/\\code(\{\.?(\w+)\})?/ ) {
        my $codelang = "";
        $codelang = " $2" if (defined $2);
        $codelang =~ m/(cpp|py|unparsed)/ or exit_with_error("invalid code snippet format: $codelang");
        $COMMENT_CODE_SNIPPET = CODE_SNIPPET;
        $COMMENT_CODE_SNIPPET = CODE_SNIPPET_CPP if ($codelang =~ m/cpp/ );
        $codelang =~ s/py/python/;
        $codelang =~ s/unparsed/raw/;
        return "\n" if ( $COMMENT_CODE_SNIPPET == CODE_SNIPPET_CPP );
        return "\n.. code-block::$codelang\n\n";
    }
    if ( $line =~ m/\\endcode/ ) {
        $COMMENT_CODE_SNIPPET = 0;
        return "\n";
    }
    if ($COMMENT_CODE_SNIPPET != 0){
        if ( $COMMENT_CODE_SNIPPET == CODE_SNIPPET_CPP ){
            # cpp code is stripped out
            return "";
        } else {
            if ( $line ne ''){
                  return "    $line\n";
              } else {
                  return "\n";
              }
        }
    }

    # remove prepending spaces
    $line =~ s/^\s+//g;
    # remove \a formatting
    $line =~ s/\\a (.+?)\b/``$1``/g;
    # replace :: with . (changes c++ style namespace/class directives to Python style)
    $line =~ s/::/./g;
    # replace nullptr with None (nullptr means nothing to Python devs)
    $line =~ s/\bnullptr\b/None/g;

    if ( $line =~ m/^\\(?<SUB>sub)?section/) {
      my $sep = "-";
      $sep = "~" if defined $+{SUB};
      $line =~ s/^\\(sub)?section \w+ //;
      my $sep_line = $line =~ s/[\w ()]/$sep/gr;
      $line .= "\n".$sep_line;
    }

    # convert ### style headings
    if ( $line =~ m/^###\s+(.*)$/) {
      $line = "$1\n".('-' x length($1));
    }
    if ( $line =~ m/^##\s+(.*)$/) {
      $line = "$1\n".('=' x length($1));
    }

    if ( $line eq '*' ) {
        $line = '';
    }

    # handle multi-line parameters/returns/lists
    if ($line ne '') {
        if ( $line =~ m/^\s*[\-#]/ ){
            # start of a list item, ensure following lines are correctly indented
            $line = "$PREV_INDENT$line";
            $INDENT = $PREV_INDENT."  ";
        }
        elsif ( $line !~ m/^\s*[\\:]+(param|note|since|return|deprecated|warning|throws)/ ) {
            # if inside multi-line parameter, ensure additional lines are indented
            $line = "$INDENT$line";
        }
    }
    else
    {
        $PREV_INDENT = $INDENT;
        $INDENT = '';
    }
    # replace \returns with :return:
    if ( $line =~ m/\\return(s)?/ ){
        $line =~ s/\s*\\return(s)?\s*/\n:return: /;
        # remove any trailing spaces, will be present now for empty 'returns' tags
        $line =~ s/\s*$//g;
        $INDENT = ' 'x( index($line,':',4) + 1);
    }

    # params
    if ( $line =~ m/\\param / ){
        $line =~ s/\s*\\param\s+(\w+)\b\s*/:param $1: /g;
        # remove any trailing spaces, will be present now for empty 'param' tags
        $line =~ s/\s*$//g;
        $INDENT = ' 'x( index($line,':',2) + 2);
        if ( $line =~ m/^:param/ ){
          if ( $COMMENT_PARAM_LIST == 0 )
          {
              $line = "\n$line";
          }
          $COMMENT_PARAM_LIST = 1;
          $COMMENT_LAST_LINE_NOTE_WARNING = 0;
        }
    }

    if ( $line =~ m/^\s*[\\@]brief/){
        $line =~ s/[\\@]brief\s*//;
        if ( $FOUND_SINCE eq 1 ) {
            exit_with_error("$headerfile\:\:$LINE_IDX Since annotation must come after brief")
        }
        $FOUND_SINCE = 0;
        if ( $line =~ m/^\s*$/ ){
            return "";
        }
    }

    if ( $line =~ m/[\\@](ingroup|class)/ ) {
        $PREV_INDENT = $INDENT;
        $INDENT = '';
        return "";
    }
    if ( $line =~ m/\\since .*?([\d\.]+)/i ) {
        $PREV_INDENT = $INDENT;
        $INDENT = '';
        $FOUND_SINCE = 1;
        return "\n.. versionadded:: $1\n";
    }
    if ( $line =~ m/\\deprecated(?:\s+since\s+(?:QGIS\s+)(?<DEPR_VERSION>[0-9.]+)(,\s*)?)?(?<DEPR_MESSAGE>.*)?/i ) {
        $PREV_INDENT = $INDENT;
        $INDENT = '';
        my $depr_line = "\n.. deprecated::";
        $depr_line .= " QGIS $+{DEPR_VERSION}" if (defined $+{DEPR_VERSION});
        $depr_line .= "\n  $+{DEPR_MESSAGE}\n" if (defined $+{DEPR_MESSAGE});
        return create_class_links($depr_line);
    }

    # create links in see also
    if ( $line =~ m/\\see +(\w+(\.\w+)*)(\([^()]*\))?/ ) {
        my $seealso = $1;
        my $seeline = '';
        dbg_info("see also: $seealso");
        if (  $seealso =~ m/^Qgs[A-Z]\w+(\([^()]*\))?$/ ) {
            dbg_info("\\see py:class");
            $seeline = ":py:class:`$seealso`";
        }
        elsif (  $seealso =~ m/^(Qgs[A-Z]\w+)\.(\w+)(\([^()]*\))?$/ ) {
            dbg_info("\\see py:func with param");
            $seeline = ":py:func:`$1.$2`";
        }
        elsif (  $seealso =~ m/^[a-z]\w+(\([^()]*\))?$/ ) {
            dbg_info("\\see py:func");
            $seeline = ":py:func:`$seealso`";
        }
        if ( $line =~ m/^\s*\\see/ ){
            if ( $seeline ne ''){
                return "\n.. seealso:: $seeline\n";
            } else {
                return "\n.. seealso:: $seealso\n";
            }
        } else {
            if ( $seeline ne ''){
                $line =~ s/\\see +(\w+(\.\w+)*(\(\))?)/$seeline/;
            } else {
                $line =~s/\\see/see/;
            }
        }
    }
    elsif ( $line !~ m/\\throws.*/ ) {
        # create links in plain text too (less performant)
        # we don't do this for "throws" lines, as Sphinx does not format these correctly
        $line = create_class_links($line)
    }

    if ( $line =~ m/[\\@]note (.*)/ ) {
        $COMMENT_LAST_LINE_NOTE_WARNING = 1;
        $PREV_INDENT = $INDENT;
        $INDENT = '';
        return "\n.. note::\n\n   $1\n";
    }
    if ( $line =~ m/[\\@]warning (.*)/ ) {
        $PREV_INDENT = $INDENT;
        $INDENT = '';
        $COMMENT_LAST_LINE_NOTE_WARNING = 1;
        return "\n.. warning::\n\n   $1\n";
    }
    if ( $line =~ m/[\\@]throws (.+?)\b\s*(.*)/ ) {
        $PREV_INDENT = $INDENT;
        $INDENT = '';
        $COMMENT_LAST_LINE_NOTE_WARNING = 1;
        return "\n:raises $1: $2\n";
    }

    if ( $line !~ m/^\s*$/ ){
        if ( $COMMENT_LAST_LINE_NOTE_WARNING == 1 ){
            dbg_info("prepend spaces for multiline warning/note xx$line");
            $line = "   $line";
        }
    } else {
        $COMMENT_LAST_LINE_NOTE_WARNING = 0;
    }
    return "$line\n";
}

sub detect_and_remove_following_body_or_initializerlist {
    # https://regex101.com/r/ZaP3tC/8
    my $python_signature = '';
    do {no warnings 'uninitialized';
        if ( $LINE =~  m/^(\s*)?((?:(?:explicit|static|const|unsigned|virtual)\s+)*)(([(?:long )\w:]+(<.*?>)?\s+[*&]?)?(~?\w+|(\w+::)?operator.{1,2})\s*\(([\w=()\/ ,&*<>."-]|::)*\)( +(?:const|SIP_[\w_]+?))*)\s*((\s*[:,]\s+\w+\(.*\))*\s*\{.*\}\s*(?:SIP_[\w_]+)?;?|(?!;))(\s*\/\/.*)?$/
             || $LINE =~ m/SIP_SKIP\s*(?!;)\s*(\/\/.*)?$/
             || $LINE =~ m/^\s*class.*SIP_SKIP/ ){
            dbg_info("remove constructor definition, function bodies, member initializing list");
            my $newline = "$1$2$3;";
            $python_signature = remove_following_body_or_initializerlist() unless $LINE =~ m/{.*}(\s*SIP_\w+)*\s*(\/\/.*)?$/;
            $LINE = $newline;
        }
    };
    return $python_signature;
}

sub remove_following_body_or_initializerlist {
    my $python_signature = '';
    do {no warnings 'uninitialized';
        dbg_info("remove constructor definition, function bodies, member initializing list");
        my $line = read_line();
        # python signature
        if ($line =~ m/^\s*\[\s*(\w+\s*)?\(/){
            dbg_info("python signature detected");
            my $nesting_index = 0;
            while ($LINE_IDX < $LINE_COUNT){
                $nesting_index += $line =~ tr/\[//;
                $nesting_index -= $line =~ tr/\]//;
                if ($nesting_index == 0){
                    if ($line =~ m/^(.*);\s*(\/\/.*)?$/){
                        $line = $1; # remove semicolon (added later)
                        $python_signature .= "\n$line";
                        return $python_signature;
                    }
                    last;
                }
                $python_signature .= "\n$line";
                $line = read_line();
            }
        }
        # member initializing list
        while ( $line =~ m/^\s*[:,]\s+([\w<>]|::)+\(.*?\)/){
          dbg_info("member initializing list");
          $line = read_line();
        }
        # body
        if ( $line =~ m/^\s*\{/ ){
            my $nesting_index = 0;
            while ($LINE_IDX < $LINE_COUNT){
                dbg_info("  remove body");
                $nesting_index += $line =~ tr/\{//;
                $nesting_index -= $line =~ tr/\}//;
                if ($nesting_index == 0){
                    last;
                }
                $line = read_line();
            }
        }
    };
    return $python_signature;
}

sub fix_annotations {
    my $line = $_[0];

    # get removed params to be able to drop them out of the API doc
    if ( $line =~ m/(\w+)\s+SIP_PYARGREMOVE/ ){
      my @removed_params = $line =~ m/(\w+)\s+SIP_PYARGREMOVE/g;
      foreach ( @removed_params ) {
        push @SKIPPED_PARAMS_REMOVE, $_;
        dbg_info("caught removed param: $SKIPPED_PARAMS_REMOVE[$#SKIPPED_PARAMS_REMOVE]");
      }
    }
    if ( $line =~ m/(\w+)\s+SIP_OUT/ ){
      my @out_params = $line =~ m/(\w+)\s+SIP_OUT/g;
      foreach ( @out_params ) {
        push @SKIPPED_PARAMS_OUT, $_;
        dbg_info("caught removed param: $SKIPPED_PARAMS_OUT[$#SKIPPED_PARAMS_OUT]");
      }
    }

    # printed annotations
    $line =~ s/\/\/\s*SIP_ABSTRACT\b/\/Abstract\//;
    $line =~ s/\bSIP_ABSTRACT\b/\/Abstract\//;
    $line =~ s/\bSIP_ALLOWNONE\b/\/AllowNone\//;
    $line =~ s/\bSIP_ARRAY\b/\/Array\//g;
    $line =~ s/\bSIP_ARRAYSIZE\b/\/ArraySize\//g;
    $line =~ s/\bSIP_DEPRECATED\b/\/Deprecated\//g;
    $line =~ s/\bSIP_CONSTRAINED\b/\/Constrained\//g;
    $line =~ s/\bSIP_EXTERNAL\b/\/External\//g;
    $line =~ s/\bSIP_FACTORY\b/\/Factory\//;
    $line =~ s/\bSIP_IN\b/\/In\//g;
    $line =~ s/\bSIP_INOUT\b/\/In,Out\//g;
    $line =~ s/\bSIP_KEEPREFERENCE\b/\/KeepReference\//;
    $line =~ s/\bSIP_NODEFAULTCTORS\b/\/NoDefaultCtors\//;
    $line =~ s/\bSIP_OUT\b/\/Out\//g;
    $line =~ s/\bSIP_RELEASEGIL\b/\/ReleaseGIL\//;
    $line =~ s/\bSIP_HOLDGIL\b/\/HoldGIL\//;
    $line =~ s/\bSIP_TRANSFER\b/\/Transfer\//g;
    $line =~ s/\bSIP_TRANSFERBACK\b/\/TransferBack\//;
    $line =~ s/\bSIP_TRANSFERTHIS\b/\/TransferThis\//;
    $line =~ s/\bSIP_GETWRAPPER\b/\/GetWrapper\//;

    $line =~ s/SIP_PYNAME\(\s*(\w+)\s*\)/\/PyName=$1\//;
    $line =~ s/SIP_TYPEHINT\(\s*([\w\s,\[\]]+?)\s*\)/\/TypeHint="$1"\//g;
    $line =~ s/SIP_VIRTUALERRORHANDLER\(\s*(\w+)\s*\)/\/VirtualErrorHandler=$1\//;
    $line =~ s/SIP_THROW\(\s*(\w+)\s*\)/throw\( $1 \)/;

    # combine multiple annotations
    # https://regex101.com/r/uvCt4M/5
    do {no warnings 'uninitialized';
        $line =~ s/\/([\w,]+(=\"?\w+\"?)?)\/\s*\/([\w,]+(=\"?\w+\"?)?)\//\/$1,$3\//;
        (! $3) or dbg_info("combine multiple annotations -- works only for 2");
    };

    # unprinted annotations
    $line =~ s/(\w+)(\<(?>[^<>]|(?2))*\>)?\s+SIP_PYALTERNATIVETYPE\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/$3/g;
    $line =~ s/=\s+[^=]*?\s+SIP_PYARGDEFAULT\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/= $1/g;
    # remove argument
    if ($line =~ m/SIP_PYARGREMOVE/){
        dbg_info("remove arg");
        if ( $MULTILINE_DEFINITION != MULTILINE_NO ){
            my $prev_line = pop(@OUTPUT) =~ s/\n$//r;
            # update multi line status
            my $parenthesis_balance = 0;
            $parenthesis_balance += $prev_line =~ tr/\(//;
            $parenthesis_balance -= $prev_line =~ tr/\)//;
            if ($parenthesis_balance == 1){
                $MULTILINE_DEFINITION = MULTILINE_NO;
            }
            # concat with above line to bring previous commas
            $line =~ s/^\s+//;
            $line = "$prev_line $line\n";
        }
        # see https://regex101.com/r/5iNptO/4
        $line =~ s/(?<coma>, +)?(const )?(\w+)(\<(?>[^<>]|(?4))*\>)?\s+[\w&*]+\s+SIP_PYARGREMOVE( = [^()]*(\(\s*(?:[^()]++|(?6))*\s*\))?)?(?(<coma>)|,?)//g;
        $line =~ s/\(\s+\)/()/;
    }
    $line =~ s/SIP_FORCE//;
    $line =~ s/SIP_DOC_TEMPLATE//;
    $line =~ s/\s+;$/;/;
    return $line;
}

sub fix_constants {
    my $line = $_[0];
    $line =~ s/\bstd::numeric_limits<double>::max\(\)/DBL_MAX/g;
    $line =~ s/\bstd::numeric_limits<double>::lowest\(\)/-DBL_MAX/g;
    $line =~ s/\bstd::numeric_limits<double>::epsilon\(\)/DBL_EPSILON/g;
    $line =~ s/\bstd::numeric_limits<qlonglong>::min\(\)/LLONG_MIN/g;
    $line =~ s/\bstd::numeric_limits<qlonglong>::max\(\)/LLONG_MAX/g;
    $line =~ s/\bstd::numeric_limits<int>::max\(\)/INT_MAX/g;
    $line =~ s/\bstd::numeric_limits<int>::min\(\)/INT_MIN/g;
    return $line;
}

sub replace_macros {
    my $line = $_[0];
    $line =~ s/\bTRUE\b/``True``/g;
    $line =~ s/\bFALSE\b/``False``/g;
    $line =~ s/\bNULLPTR\b/``None``/g;
    return $line;
}

# detect a comment block, return 1 if found
# if STRICT comment block shall begin at beginning of line (no code in front)
sub detect_comment_block{
    my %args = ( strict_mode => STRICT, @_ );
    # dbg_info("detect comment strict:" . $args{strict_mode} );
    $COMMENT_PARAM_LIST = 0;
    $INDENT = '';
    $PREV_INDENT = '';
    $COMMENT_CODE_SNIPPET = 0;
    $COMMENT_LAST_LINE_NOTE_WARNING = 0;
    $FOUND_SINCE = 0;
    @SKIPPED_PARAMS_OUT = ();
    @SKIPPED_PARAMS_REMOVE = ();
    if ( $LINE =~ m/^\s*\/\*/ || $args{strict_mode} == UNSTRICT && $LINE =~ m/\/\*/ ){
        dbg_info("found comment block");
        do {no warnings 'uninitialized';
            $COMMENT = processDoxygenLine( $LINE =~ s/^\s*\/\*(\*)?(.*?)\n?$/$2/r );
        };
        $COMMENT =~ s/^\s*$//;
        while ($LINE !~ m/\*\/\s*(\/\/.*?)?$/){
            $LINE = read_line();
            $COMMENT .= processDoxygenLine( $LINE =~ s/\s*\*?(.*?)(\/)?\n?$/$1/r );
        }
        $COMMENT =~ s/\n\s+\n/\n\n/;
        $COMMENT =~ s/\n{3,}/\n\n/;
        $COMMENT =~ s/\n+$//;
        return 1;
    }
    return 0;
}

# Detect if line is a non method member declaration
# https://regex101.com/r/gUBZUk/14
sub detect_non_method_member{
  return 1 if $LINE =~ m/^\s*(?:template\s*<\w+>\s+)?(?:(const|mutable|static|friend|unsigned)\s+)*\w+(::\w+)?(<([\w<> *&,()]|::)+>)?(,?\s+\*?\w+( = (-?\d+(\.\d+)?|((QMap|QList)<[^()]+>\(\))|(\w+::)*\w+(\([^()]?\))?)|\[\d+\])?)+;/;
  return 0;
}

# write some code in front of line to know where the output comes from
$debug == 0 or push @OUTPUT, "CODE SIP_RUN MultiLine\n";

# main loop
while ($LINE_IDX < $LINE_COUNT){

    $PYTHON_SIGNATURE = '';
    $ACTUAL_CLASS = $CLASSNAME[$#CLASSNAME] unless $#CLASSNAME < 0;
    $LINE = read_line();

    if ( $LINE =~ m/^\s*(#define )?+SIP_IF_MODULE\(.*\)$/ ){
        dbg_info('skipping SIP include condition macro');
        next;
    }

    if ($LINE =~ m/^\s*SIP_FEATURE\( (\w+) \)(.*)$/){
        write_output("SF1", "%Feature $1$2\n");
        next;
    }
    if ($LINE =~ m/^\s*SIP_PROPERTY\((.*)\)$/){
        write_output("SF1", "%Property($1)\n");
        next;
    }
    if ($LINE =~ m/^\s*SIP_IF_FEATURE\( (\!?\w+) \)(.*)$/){
        write_output("SF2", "%If ($1)$2\n");
        next;
    }
    if ($LINE =~ m/^\s*SIP_CONVERT_TO_SUBCLASS_CODE(.*)$/){
        $LINE = "%ConvertToSubClassCode$1";
        # do not go next, let run the "do not process SIP code"
    }
    if ($LINE =~ m/^\s*SIP_VIRTUAL_CATCHER_CODE(.*)$/){
        $LINE = "%VirtualCatcherCode$1";
        # do not go next, let run the "do not process SIP code"
    }

    if ($LINE =~ m/^\s*SIP_END(.*)$/){
        write_output("SEN", "%End$1\n");
        next;
    }

    if ( $LINE =~ s/SIP_WHEN_FEATURE\(\s*(.*?)\s*\)// ){
        dbg_info('found SIP_WHEN_FEATURE');
        $IF_FEATURE_CONDITION = $1;
    }

    # do not process SIP code %XXXCode
    if ( $SIP_RUN == 1 && $LINE =~ m/^ *% *(VirtualErrorHandler|MappedType|Type(?:Header)?Code|Module(?:Header)?Code|Convert(?:From|To)(?:Type|SubClass)Code|MethodCode|Docstring)(.*)?$/ ){
        $LINE = "%$1$2";
        $COMMENT = '';
        dbg_info("do not process SIP code");
        while ( $LINE !~ m/^ *% *End/ ){
            write_output("COD", $LINE."\n");
            $LINE = read_line();
            $LINE =~ s/^ *% *(VirtualErrorHandler|MappedType|Type(?:Header)?Code|Module(?:Header)?Code|Convert(?:From|To)(?:Type|SubClass)Code|MethodCode|Docstring)(.*)?$/%$1$2/;
            $LINE =~ s/^\s*SIP_END(.*)$/%End$1/;
        }
        $LINE =~ s/^\s*% End/%End/;
        write_output("COD", $LINE."\n");
        next;
    }
    # do not process SIP code %Property
    if ( $SIP_RUN == 1 && $LINE =~ m/^ *% *(Property)(.*)?$/ ){
        $LINE = "%$1$2";
        $COMMENT = '';
        write_output("COD", $LINE."\n");
        next;
    }

    # Skip preprocessor stuff
    if ($LINE =~ m/^\s*#/){
        # skip #if 0 blocks
        if ( $LINE =~ m/^\s*#if (0|defined\(Q_OS_WIN\))/){
          dbg_info("skipping #if $1 block");
          my $nesting_index = 0;
          while ($LINE_IDX < $LINE_COUNT){
              $LINE = read_line();
              if ( $LINE =~ m/^\s*#if(def)?\s+/ ){
                  $nesting_index++;
              }
              elsif ( $nesting_index == 0 && $LINE =~ m/^\s*#(endif|else)/ ){
                  $COMMENT = '';
                  last;
              }
              elsif ( $nesting_index != 0 && $LINE =~ m/^\s*#(endif)/ ){
                      $nesting_index--;
              }
          }
          next;
        }

        if ( $LINE =~ m/^\s*#ifdef SIP_RUN/){
            $SIP_RUN = 1;
            if ($ACCESS[$#ACCESS] == PRIVATE){
                dbg_info("writing private content");
                write_output("PRV1", $PRIVATE_SECTION_LINE."\n") if $PRIVATE_SECTION_LINE ne '';
                $PRIVATE_SECTION_LINE = '';
            }
            next;
        }
        if ( $SIP_RUN == 1 ){
            if ( $LINE =~ m/^\s*#endif/ ){
                if ( $GLOB_IFDEF_NESTING_IDX == 0 ){
                    $SIP_RUN = 0;
                    next;
                }
                else {
                    $GLOB_IFDEF_NESTING_IDX--;
                }
            }
            if ( $LINE =~ m/^\s*#if(def)?\s+/ ){
                $GLOB_IFDEF_NESTING_IDX++;
            }

            # if there is an else at this level, code will be ignored i.e. not SIP_RUN
            if ( $LINE =~ m/^\s*#else/ && $GLOB_IFDEF_NESTING_IDX == 0){
                while ($LINE_IDX < $LINE_COUNT){
                    $LINE = read_line();
                    if ( $LINE =~ m/^\s*#if(def)?\s+/ ){
                        $GLOB_IFDEF_NESTING_IDX++;
                    }
                    elsif ( $LINE =~ m/^\s*#endif/ ){
                        if ( $GLOB_IFDEF_NESTING_IDX == 0 ){
                            $COMMENT = '';
                            $SIP_RUN = 0;
                            last;
                        }
                        else {
                            $GLOB_IFDEF_NESTING_IDX--;
                        }
                    }
                }
                next;
            }
        }
        elsif ( $LINE =~ m/^\s*#ifndef SIP_RUN/){
            # code is ignored here
            while ($LINE_IDX < $LINE_COUNT){
                $LINE = read_line();
                if ( $LINE =~ m/^\s*#if(def)?\s+/ ){
                    $GLOB_IFDEF_NESTING_IDX++;
                }
                elsif ( $LINE =~ m/^\s*#else/ && $GLOB_IFDEF_NESTING_IDX == 0 ){
                    # code here will be printed out
                    if ($ACCESS[$#ACCESS] == PRIVATE){
                        dbg_info("writing private content");
                        write_output("PRV2", $PRIVATE_SECTION_LINE."\n") if $PRIVATE_SECTION_LINE ne '';
                        $PRIVATE_SECTION_LINE = '';
                    }
                    $SIP_RUN = 1;
                    last;
                }
                elsif ( $LINE =~ m/^\s*#endif/ ){
                    if ( $GLOB_IFDEF_NESTING_IDX == 0 ){
                        $COMMENT = '';
                        $SIP_RUN = 0;
                        last;
                    }
                    else {
                        $GLOB_IFDEF_NESTING_IDX--;
                    }
                }
            }
            next;
        }
        else {
            next;
        }
    }

    # TYPE HEADER CODE
    if ( $HEADER_CODE && $SIP_RUN == 0 ){
        $HEADER_CODE = 0;
        write_output("HCE", "%End\n");
    }

    # Skip forward declarations
    if ($LINE =~ m/^\s*(enum\s+)?(class|struct) \w+(?<external> *SIP_EXTERNAL)?;\s*(\/\/.*)?$/){
        if ($+{external}){
            dbg_info('do not skip external forward declaration');
            $COMMENT = '';
        }
        else
        {
            dbg_info('skipping forward declaration');
            next;
        }
    }
    # skip friends
    if ( $LINE =~ m/^\s*friend class \w+/ ){
        next;
    }

    # insert metaobject for Q_GADGET
    if ($LINE =~ m/^\s*Q_GADGET\b.*?$/){
        if ($LINE !~ m/SIP_SKIP/){
            dbg_info('Q_GADGET');
            write_output("HCE", "  public:\n");
            write_output("HCE", "    static const QMetaObject staticMetaObject;\n\n");
        }
        next;
    }
    # insert in python output (python/module/__init__.py)
    if ($LINE =~ m/Q_(ENUM|FLAG)\(\s*(\w+)\s*\)/ ){
        if ($LINE !~ m/SIP_SKIP/){
            my $is_flag = $1 eq 'FLAG' ? 1 : 0;
            my $enum_helper = "$ACTUAL_CLASS.$2.baseClass = $ACTUAL_CLASS";
            dbg_info("Q_ENUM/Q_FLAG $enum_helper");
            if ($python_output ne ''){
                if ($enum_helper ne ''){
                    push @OUTPUT_PYTHON, "$enum_helper\n";
                    if ($is_flag == 1){
                        # SIP seems to introduce the flags in the module rather than in the class itself
                        # as a dirty hack, inject directly in module, hopefully we don't have flags with the same name....
                        push @OUTPUT_PYTHON, "$2 = $ACTUAL_CLASS  # dirty hack since SIP seems to introduce the flags in module\n";
                    }
                }
            }
        }
        next;
    }

    # Skip Q_OBJECT, Q_PROPERTY, Q_ENUM etc.
    if ($LINE =~ m/^\s*Q_(OBJECT|ENUMS|ENUM|FLAG|PROPERTY|DECLARE_METATYPE|DECLARE_TYPEINFO|NOWARN_DEPRECATED_(PUSH|POP))\b.*?$/){
        next;
    }
    if ($LINE =~ m/^\s*QHASH_FOR_CLASS_ENUM/){
        next;
    }

    # SIP_SKIP
    if ( $LINE =~ m/SIP_SKIP|SIP_PYTHON_SPECIAL_/ ){
        dbg_info('SIP SKIP!');
        # if multiline definition, remove previous lines
        if ( $MULTILINE_DEFINITION != MULTILINE_NO){
            dbg_info('SIP_SKIP with MultiLine');
            my $opening_line = '';
            while ( $opening_line !~ m/^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$/){
                $opening_line = pop(@OUTPUT);
                $#OUTPUT >= 0 or exit_with_error('could not reach opening definition');
            }
        dbg_info("removed multiline definition of SIP_SKIP method");
        $MULTILINE_DEFINITION = MULTILINE_NO;
        }
        # also skip method body if there is one
        detect_and_remove_following_body_or_initializerlist();
        # line skipped, go to next iteration

        if ($LINE =~ m/SIP_PYTHON_SPECIAL_(\w+)\(\s*(".*"|\w+)\s*\)/ ){
            my $method_or_code = $2;
            dbg_info("PYTHON SPECIAL method or code: $method_or_code");
            my $pyop = "${ACTUAL_CLASS}.__" . lc($1) . "__ = lambda self: ";
            if ( $method_or_code =~ m/^"(.*)"$/ ){
              $pyop .= $1;
            }
            else
            {
              $pyop .= "self.${method_or_code}()";
            }
            dbg_info("PYTHON SPECIAL $pyop");
            if ($python_output ne ''){
                push @OUTPUT_PYTHON, "$pyop\n";
            }
        }

        $COMMENT = '';
        next;
    }

    # Detect comment block
    if (detect_comment_block()){
        next;
    }

    if ( $LINE =~ m/^\s*struct(\s+\w+_EXPORT)?\s+\w+$/ ) {
        dbg_info("  going to struct => public");
        push @CLASSNAME, $CLASSNAME[$#CLASSNAME]; # fake new class since struct has considered similarly
        push @ACCESS, PUBLIC;
        push @EXPORTED, $EXPORTED[-1];
        push @GLOB_BRACKET_NESTING_IDX, 0;
    }

    # class declaration started
    # https://regex101.com/r/6FWntP/16
    if ( $LINE =~ m/^(\s*(class))\s+([A-Z0-9_]+_EXPORT\s+)?(Q_DECL_DEPRECATED\s+)?(?<classname>\w+)(?<domain>\s*\:\s*(public|protected|private)\s+\w+(< *(\w|::)+ *>)?(::\w+(<\w+>)?)*(,\s*(public|protected|private)\s+\w+(< *(\w|::)+ *>)?(::\w+(<\w+>)?)*)*)?(?<annot>\s*\/?\/?\s*SIP_\w+)?\s*?(\/\/.*|(?!;))$/ ){
        dbg_info("class definition started");
        push @ACCESS, PUBLIC;
        push @EXPORTED, 0;
        push @GLOB_BRACKET_NESTING_IDX, 0;
        my @template_inheritance_template = ();
        my @template_inheritance_class = ();
        do {no warnings 'uninitialized';
            push @CLASSNAME, $+{classname};
            if ($#CLASSNAME == 0){
                # might be worth to add in-class classes later on
                # in case of a tamplate based class declaration
                # based on an in-class and in the same file
                push @DECLARED_CLASSES, $CLASSNAME[$#CLASSNAME];
            }
            dbg_info("class: ".$CLASSNAME[$#CLASSNAME]);
            if ($LINE =~ m/\b[A-Z0-9_]+_EXPORT\b/ || $#CLASSNAME != 0 || $INPUT_LINES[$LINE_IDX-2] =~ m/^\s*template\s*</){
                # class should be exported except those not at top level or template classes
                # if class is not exported, then its methods should be (checked whenever leaving out the class)
                $EXPORTED[-1]++;
            }
        };
        $LINE = "$1 $+{classname}";
        # Inheritance
        if (defined $+{domain}){
            my $m = $+{domain};
            $m =~ s/public +(\w+, *)*(Ui::\w+,? *)+//g; # remove Ui::xxx public inheritance as the namespace is causing troubles
            $m =~ s/public +//g;
            $m =~ s/[,:]?\s*private +\w+(::\w+)?//g;
            # detect template based inheritance
            while ($m =~ /[,:]\s+((?!QList)\w+)< *((\w|::)+) *>/g){
                dbg_info("template class");
                push @template_inheritance_template, $1;
                push @template_inheritance_class, $2;
            }
            $m =~ s/(\b(?!QList)\w+)< *((?:\w|::)+) *>/$1${2}Base/g; # use the typeded as template inheritance
            $m =~ s/(\w+)< *((?:\w|::)+) *>//g; # remove remaining templates
            $m =~ s/([:,])\s*,/$1/g;
            $m =~ s/(\s*[:,])?\s*$//;
            $LINE .= $m;
        }
        if (defined $+{annot})
        {
            $LINE .= "$+{annot}";
            $LINE = fix_annotations($LINE);
        }

        $LINE .= "\n{\n";
        if ( $COMMENT !~ m/^\s*$/ ){
            $LINE .= "%Docstring(signature=\"appended\")\n$COMMENT\n%End\n";
        }
        $LINE .= "\n%TypeHeaderCode\n#include \"" . basename($headerfile) . "\"";
        # for template based inheritance, add a typedef to define the base type
        # add it to the class and to the TypeHeaderCode
        # also include the template header
        # see https://www.riverbankcomputing.com/pipermail/pyqt/2015-May/035893.html
        while ( @template_inheritance_template ) {
            my $tpl = pop @template_inheritance_template;
            my $cls = pop @template_inheritance_class;
            $LINE = "\ntypedef $tpl<$cls> ${tpl}${cls}Base;\n\n$LINE";
            if ( not $tpl ~~ @DECLARED_CLASSES ){
                my $tpl_header = lc $tpl . ".h";
                if ( exists $SIP_CONFIG->{class_headerfile}->{$tpl} ){
                    $tpl_header = $SIP_CONFIG->{class_headerfile}->{$tpl};
                }
                $LINE .= "\n#include \"" . $tpl_header . "\"";
            }
            $LINE .= "\ntypedef $tpl<$cls> ${tpl}${cls}Base;";
        }
        if ( PRIVATE ~~ @ACCESS && $#ACCESS != 0){
            # do not write anything in PRIVATE context and not top level
            dbg_info("skipping class in private context");
            next;
        }
        $ACCESS[$#ACCESS] = PRIVATE; # private by default

        write_output("CLS", "$LINE\n");

        # Skip opening curly bracket, incrementing hereunder
        my $skip = read_line();
        $skip =~ m/^\s*{\s*$/ or exit_with_error("expecting { after class definition");
        $GLOB_BRACKET_NESTING_IDX[$#GLOB_BRACKET_NESTING_IDX]++;

        $COMMENT = '';
        $HEADER_CODE = 1;
        $ACCESS[$#ACCESS] = PRIVATE;
        next;
    }

    # bracket balance in class/struct tree
    if ($SIP_RUN == 0){
        my $bracket_balance = 0;
        $bracket_balance += $LINE =~ tr/\{//;
        $bracket_balance -= $LINE =~ tr/\}//;
        if ($bracket_balance != 0){
            $GLOB_BRACKET_NESTING_IDX[$#GLOB_BRACKET_NESTING_IDX] += $bracket_balance;
            if ($GLOB_BRACKET_NESTING_IDX[$#GLOB_BRACKET_NESTING_IDX] == 0){
                dbg_info(" going up in class/struct tree");
                if ($#ACCESS > 0){
                    pop(@GLOB_BRACKET_NESTING_IDX);
                    pop(@ACCESS);
                    exit_with_error("Class $CLASSNAME[$#CLASSNAME] should be exported with appropriate [LIB]_EXPORT macro. If this should not be available in python, wrap it in a `#ifndef SIP_RUN` block.")
                        if $EXPORTED[-1] == 0 and not $CLASSNAME[$#CLASSNAME] ~~ $SIP_CONFIG->{no_export_macro};
                    pop @EXPORTED;
                }
                pop(@CLASSNAME);
                if ($#ACCESS == 0){
                    dbg_info("reached top level");
                    # top level should stay public
                    $ACCESS[$#ACCESS] = PUBLIC;
                }
                $COMMENT = '';
                $RETURN_TYPE = '';
                $PRIVATE_SECTION_LINE = '';
            }
            dbg_info("new bracket balance: @GLOB_BRACKET_NESTING_IDX");
        }
    }

    # Private members (exclude SIP_RUN)
    if ( $LINE =~ m/^\s*private( slots)?:/ ){
        $ACCESS[$#ACCESS] = PRIVATE;
        $LAST_ACCESS_SECTION_LINE = $LINE;
        $PRIVATE_SECTION_LINE = $LINE;
        $COMMENT = '';
        dbg_info("going private");
        next;
    }
    elsif ( $LINE =~ m/^\s*(public( slots)?|signals):.*$/ ){
        dbg_info("going public");
        $LAST_ACCESS_SECTION_LINE = $LINE;
        $ACCESS[$#ACCESS] = PUBLIC;
        $COMMENT = '';
    }
    elsif ( $LINE =~ m/^\s*(protected)( slots)?:.*$/ ){
        dbg_info("going protected");
        $LAST_ACCESS_SECTION_LINE = $LINE;
        $ACCESS[$#ACCESS] = PROTECTED;
        $COMMENT = '';
    }
    elsif ( $ACCESS[$#ACCESS] == PRIVATE && $LINE =~ m/SIP_FORCE/){
        dbg_info("private with SIP_FORCE");
        write_output("PRV3", $PRIVATE_SECTION_LINE."\n") if $PRIVATE_SECTION_LINE ne '';
        $PRIVATE_SECTION_LINE = '';
    }
    elsif ( PRIVATE ~~ @ACCESS && $SIP_RUN == 0 ) {
        $COMMENT = '';
        next;
    }
    # Skip operators
    if ( $ACCESS[$#ACCESS] != PRIVATE && $LINE =~ m/operator(=|<<|>>|->)\s*\(/ ){
        dbg_info("skip operator");
        detect_and_remove_following_body_or_initializerlist();
        next;
    }

    # save comments and do not print them, except in SIP_RUN
    if ( $SIP_RUN == 0 ){
        if ( $LINE =~ m/^\s*\/\// ){
            if ($LINE =~ m/^\s*\/\/\!\s*(.*?)\n?$/){
                $COMMENT_PARAM_LIST = 0;
                $PREV_INDENT = $INDENT;
                $INDENT = '';
                $COMMENT_LAST_LINE_NOTE_WARNING = 0;
                $COMMENT = processDoxygenLine( $1 );
                $COMMENT =~ s/\n+$//;
            }
            elsif ($INPUT_LINES[$LINE_IDX-1] !~ m/\*\/.*/) {
                $COMMENT = '';
            }
            next;
        }
    }

    # Enum declaration
    # For scoped and type based enum, the type has to be removed
    if ( $LINE =~ m/^\s*Q_DECLARE_FLAGS\s*\(\s*(?<flags_name>\w+)\s*,\s*(?<flag_name>\w+)\s*\)\s*SIP_MONKEYPATCH_FLAGS_UNNEST\s*\(\s*(?<emkb>\w+)\s*,\s*(?<emkf>\w+)\s*\)\s*$/ ){
        push @OUTPUT_PYTHON, "$+{emkb}.$+{emkf} = $ACTUAL_CLASS.$+{flags_name}\n";
        $LINE =~ s/\s*SIP_MONKEYPATCH_FLAGS_UNNEST\(.*?\)//;
    }
    if ( $LINE =~ m/^(\s*enum(\s+Q_DECL_DEPRECATED)?\s+(?<isclass>class\s+)?(?<enum_qualname>\w+))(:?\s+SIP_.*)?(\s*:\s*\w+)?(?<oneliner>.*)$/ ){
        my $enum_decl = $1;
        $enum_decl =~ s/\s*\bQ_DECL_DEPRECATED\b//;
        write_output("ENU1", "$enum_decl");
        write_output("ENU1", $+{oneliner}) if defined $+{oneliner};
        write_output("ENU1", "\n");
        my $enum_qualname = $+{enum_qualname};
        my $is_scope_based = "0";
        $is_scope_based = "1" if defined $+{isclass};
        my $monkeypatch = "0";
        $monkeypatch = "1" if defined $is_scope_based eq "1" and $LINE =~ m/SIP_MONKEYPATCH_SCOPEENUM(_UNNEST)?(:?\(\s*(?<emkb>\w+)\s*,\s*(?<emkf>\w+)\s*\))?/;
        my $enum_mk_base = "";
        $enum_mk_base = $+{emkb} if defined $+{emkb};
        if (defined $+{emkf} and $monkeypatch eq "1"){
          if ( $ACTUAL_CLASS ne "" ) {
            if ($enum_mk_base.$+{emkf} ne $ACTUAL_CLASS.$enum_qualname) {
              push @OUTPUT_PYTHON, "$enum_mk_base.$+{emkf} = $ACTUAL_CLASS.$enum_qualname\n";
            }
          } else {
            push @OUTPUT_PYTHON, "$enum_mk_base.$+{emkf} = $enum_qualname\n";
          }
        }
        if ($LINE =~ m/\{((\s*\w+)(\s*=\s*[\w\s\d<|]+.*?)?(,?))+\s*\}/){
          # one line declaration
          $LINE !~ m/=/ or exit_with_error("Sipify does not handle enum one liners with value assignment. Use multiple lines instead. Or jusr write a new parser.");
          next;
        }
        else
        {
            $LINE = read_line();
            $LINE =~ m/^\s*\{\s*$/ or exit_with_error('Unexpected content: enum should be followed by {');
            write_output("ENU2", "$LINE\n");
            push @OUTPUT_PYTHON, "# monkey patching scoped based enum\n" if $is_scope_based eq "1";
            my @enum_members_doc = ();
            while ($LINE_IDX < $LINE_COUNT){
                $LINE = read_line();
                if (detect_comment_block()){
                    next;
                }
                last if ($LINE =~ m/\};/);
                next if ($LINE =~ m/^\s*\w+\s*\|/); # multi line declaration as sum of enums

                do {no warnings 'uninitialized';
                    my $enum_decl = $LINE =~ s/^(\s*(?<em>\w+))(\s+SIP_PYNAME(?:\(\s*(?<pyname>[^() ]+)\s*\)\s*)?)?(\s+SIP_MONKEY\w+(?:\(\s*(?<compat>[^() ]+)\s*\)\s*)?)?(?:\s*=\s*(?:[\w\s\d|+-]|::|<<)+)?(,?)(:?\s*\/\/!<\s*(?<co>.*)|.*)$/$1$3$7/r;
                    my $enum_member = $+{em};
                    my $comment = $+{co};
                    my $compat_name = $+{compat} ? $+{compat} : $enum_member;
                    dbg_info("is_scope_based:$is_scope_based enum_mk_base:$enum_mk_base monkeypatch:$monkeypatch");
                    if ($is_scope_based eq "1" and $enum_member ne "") {
                        if ( $monkeypatch eq 1 and $enum_mk_base ne ""){
                          if ( $ACTUAL_CLASS ne "" ) {
                            push @OUTPUT_PYTHON, "$enum_mk_base.$compat_name = $ACTUAL_CLASS.$enum_qualname.$enum_member\n";
                            push @OUTPUT_PYTHON, "$enum_mk_base.$compat_name.is_monkey_patched = True\n";
                            push @OUTPUT_PYTHON, "$enum_mk_base.$compat_name.__doc__ = \"$comment\"\n";
                            push @enum_members_doc, "'* ``$compat_name``: ' + $ACTUAL_CLASS.$enum_qualname.$enum_member.__doc__";
                          } else {
                            push @OUTPUT_PYTHON, "$enum_mk_base.$compat_name = $enum_qualname.$enum_member\n";
                            push @OUTPUT_PYTHON, "$enum_mk_base.$compat_name.is_monkey_patched = True\n";
                            push @OUTPUT_PYTHON, "$enum_mk_base.$compat_name.__doc__ = \"$comment\"\n";
                            push @enum_members_doc, "'* ``$compat_name``: ' + $enum_qualname.$enum_member.__doc__";
                          }
                        } else {
                            if ( $monkeypatch eq 1 )
                            {
                                push @OUTPUT_PYTHON, "$ACTUAL_CLASS.$compat_name = $ACTUAL_CLASS.$enum_qualname.$enum_member\n";
                                push @OUTPUT_PYTHON, "$ACTUAL_CLASS.$compat_name.is_monkey_patched = True\n";
                            }
                            if ( $ACTUAL_CLASS ne "" ){
                                push @OUTPUT_PYTHON, "$ACTUAL_CLASS.$enum_qualname.$compat_name.__doc__ = \"$comment\"\n";
                                push @enum_members_doc, "'* ``$compat_name``: ' + $ACTUAL_CLASS.$enum_qualname.$enum_member.__doc__";
                            } else {
                                push @OUTPUT_PYTHON, "$enum_qualname.$compat_name.__doc__ = \"$comment\"\n";
                                push @enum_members_doc, "'* ``$compat_name``: ' + $enum_qualname.$enum_member.__doc__";
                            }
                        }
                    }
                    $enum_decl = fix_annotations($enum_decl);
                    write_output("ENU3", "$enum_decl\n");
                };
                detect_comment_block(strict_mode => UNSTRICT);
            }
            write_output("ENU4", "$LINE\n");
            if ($is_scope_based eq "1") {
                $COMMENT =~ s/\n/\\n/g;
                if ( $ACTUAL_CLASS ne "" ){
                    push @OUTPUT_PYTHON, "$ACTUAL_CLASS.$enum_qualname.__doc__ = '$COMMENT\\n\\n' + " . join(" + '\\n' + ", @enum_members_doc) . "\n# --\n";
                } else {
                    push @OUTPUT_PYTHON, "$enum_qualname.__doc__ = '$COMMENT\\n\\n' + " . join(" + '\\n' + ", @enum_members_doc) . "\n# --\n";
                }
            }
            # enums don't have Docstring apparently
            $COMMENT = '';
            next;
        }
    }

    if( $LINE =~ /.*\/\/\!\</ ) {
      exit_with_error("\"\\\\!<\" doxygen command must only be used for enum documentation")
    }

    $IS_OVERRIDE_OR_MAKE_PRIVATE = PREPEND_CODE_VIRTUAL if ( $LINE =~ m/\boverride\b/);
    $IS_OVERRIDE_OR_MAKE_PRIVATE = PREPEND_CODE_VIRTUAL if ( $LINE =~ m/\bFINAL\b/);
    $IS_OVERRIDE_OR_MAKE_PRIVATE = PREPEND_CODE_MAKE_PRIVATE if ( $LINE =~ m/\bSIP_MAKE_PRIVATE\b/);

    # keyword fixes
    do {no warnings 'uninitialized';
        $LINE =~ s/^(\s*template\s*<)(?:class|typename) (\w+>)(.*)$/$1$2$3/;
        $LINE =~ s/\s*\boverride\b//;
        $LINE =~ s/\s*\bSIP_MAKE_PRIVATE\b//;
        $LINE =~ s/\s*\bFINAL\b/ \${SIP_FINAL}/;
        $LINE =~ s/\s*\bextern \b//;
        $LINE =~ s/\s*\bMAYBE_UNUSED \b//;
        $LINE =~ s/\s*\bNODISCARD \b//;
        $LINE =~ s/\s*\bQ_DECL_DEPRECATED\b//;
        $LINE =~ s/^(\s*)?(const |virtual |static )*inline /$1$2/;
        $LINE =~ s/\bconstexpr\b/const/;
        $LINE =~ s/\bnullptr\b/0/g;
        $LINE =~ s/\s*=\s*default\b//g;
    };

    if( $LINE =~ /\b\w+_EXPORT\b/ ) {
            $EXPORTED[-1]++;
            $LINE =~ s/\b\w+_EXPORT\s+//g;
    }

    # skip non-method member declaration in non-public sections
    if ( $SIP_RUN != 1 &&
         $ACCESS[$#ACCESS] != PUBLIC &&
         detect_non_method_member() == 1){
        dbg_info("skip non-method member declaration in non-public sections");
        next;
    }

    # remove static const value assignment
    # https://regex101.com/r/DyWkgn/6
    do {no warnings 'uninitialized';
        $LINE !~ m/^\s*const static \w+/ or exit_with_error("const static should be written static const in $CLASSNAME[$#CLASSNAME]");
        $LINE =~ s/^(?<staticconst> *(?<static>static )?const \w+(?:<(?:[\w<>, ]|::)+>)? \w+)(?: = [^()]+?(\((?:[^()]++|(?3))*\))?[^()]*?)?(?<endingchar>[|;]) *(\/\/.*?)?$/$1;/;
        $COMMENT = '' if (defined $+{staticconst} && ! defined $+{static});
        if ( defined $+{endingchar} && $+{endingchar} =~ m/\|/ ){
            dbg_info("multiline const static assignment");
            my $skip = '';
            while ( $skip !~ m/;\s*(\/\/.*?)?$/ ){
                $skip = read_line();
            }
        }
    };

    # remove struct member assignment
    # https://regex101.com/r/tWRGkY/2
    if ( $SIP_RUN != 1 && $ACCESS[$#ACCESS] == PUBLIC && $LINE =~ m/^(\s*\w+[\w<> *&:,]* \*?\w+) = [\-\w\:\.]+(<\w+( \*)?>)?(\([^()]*\))?\s*;/ ){
        dbg_info("remove struct member assignment");
        $LINE = "$1;";
    }

    # catch Q_DECLARE_FLAGS
    if ( $LINE =~ m/^(\s*)Q_DECLARE_FLAGS\(\s*(.*?)\s*,\s*(.*?)\s*\)\s*$/ ){
        my $ACTUAL_CLASS = $#CLASSNAME >= 0 ? $CLASSNAME[$#CLASSNAME].'::' : '';
        dbg_info("Declare flags: $ACTUAL_CLASS");
        $LINE = "$1typedef QFlags<${ACTUAL_CLASS}$3> $2;\n";
        $QFLAG_HASH{"${ACTUAL_CLASS}$2"} = "${ACTUAL_CLASS}$3";
    }
    # catch Q_DECLARE_OPERATORS_FOR_FLAGS
    if ( $LINE =~ m/^(\s*)Q_DECLARE_OPERATORS_FOR_FLAGS\(\s*(.*?)\s*\)\s*$/ ){
        my $flag = $QFLAG_HASH{$2};
        $LINE = "$1QFlags<$flag> operator|($flag f1, QFlags<$flag> f2);\n";
    }

    # remove Q_INVOKABLE
    $LINE =~ s/^(\s*)Q_INVOKABLE /$1/;

    do {no warnings 'uninitialized';
        # remove keywords
        if ( $IS_OVERRIDE_OR_MAKE_PRIVATE != PREPEND_CODE_NO ){
            # handle multiline definition to add virtual keyword or making private on opening line
            if ( $MULTILINE_DEFINITION != MULTILINE_NO ){
                my $rolling_line = $LINE;
                my $rolling_line_idx = $LINE_IDX;
                dbg_info("handle multiline definition to add virtual keyword or making private on opening line");
                while ( $rolling_line !~ m/^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$/){
                    $rolling_line_idx--;
                    $rolling_line = $INPUT_LINES[$rolling_line_idx];
                    $rolling_line_idx >= 0 or exit_with_error('could not reach opening definition');
                }
                if ( $IS_OVERRIDE_OR_MAKE_PRIVATE == PREPEND_CODE_VIRTUAL && $rolling_line !~ m/^(\s*)virtual\b(.*)$/ ){
                    my $idx = $#OUTPUT-$LINE_IDX+$rolling_line_idx+2;
                    #print "len: $#OUTPUT line_idx: $LINE_IDX virt: $rolling_line_idx\n"idx: $idx\n$OUTPUT[$idx]\n";
                    $OUTPUT[$idx] = fix_annotations($rolling_line =~ s/^(\s*?)\b(.*)$/$1 virtual $2\n/r);
                }
                elsif ( $IS_OVERRIDE_OR_MAKE_PRIVATE == PREPEND_CODE_MAKE_PRIVATE ) {
                    dbg_info("prepending private access");
                    my $idx = $#OUTPUT-$LINE_IDX+$rolling_line_idx+2;
                    my $private_access = $LAST_ACCESS_SECTION_LINE =~ s/(protected|public)/private/r;
                    splice @OUTPUT, $idx, 0, $private_access . "\n";
                    $OUTPUT[$idx+1] = fix_annotations($rolling_line) . "\n";
                }
            }
            elsif ( $IS_OVERRIDE_OR_MAKE_PRIVATE == PREPEND_CODE_MAKE_PRIVATE ) {
                dbg_info("prepending private access");
                $LINE = $LAST_ACCESS_SECTION_LINE =~ s/(protected|public)/private/r . "\n" . $LINE . "\n";
            }
            elsif (  $IS_OVERRIDE_OR_MAKE_PRIVATE == PREPEND_CODE_VIRTUAL && $LINE !~ m/^(\s*)virtual\b(.*)$/ ){
                #sip often requires the virtual keyword to be present, or it chokes on covariant return types
                #in overridden methods
                dbg_info('adding virtual keyword for overridden method');
                $LINE =~ s/^(\s*?)\b(.*)$/$1virtual $2\n/;
            }
        }

        # remove constructor definition, function bodies, member initializing list
        $PYTHON_SIGNATURE = detect_and_remove_following_body_or_initializerlist();

        # remove inline declarations
        if ( $LINE =~  m/^(\s*)?(static |const )*(([(?:long )\w:]+(<.*?>)?\s+(\*|&)?)?(\w+)( (?:const*?))*)\s*(\{.*\});(\s*\/\/.*)?$/ ){
            my $newline = "$1$3;";
            $LINE = $newline;
        }

        if ( $LINE =~  m/^\s*(?:const |virtual |static |inline )*(?!explicit)([(?:long )\w:]+(?:<.*?>)?)\s+(?:\*|&)?(?:\w+|operator.{1,2})\(.*$/ ){
            if ($1 !~ m/(void|SIP_PYOBJECT|operator|return|QFlag)/ ){
                $RETURN_TYPE = $1;
                # replace :: with . (changes c++ style namespace/class directives to Python style)
                $RETURN_TYPE =~ s/::/./g;

                # replace with builtin Python types
                $RETURN_TYPE =~ s/\bdouble\b/float/;
                $RETURN_TYPE =~ s/\bQString\b/str/;
                $RETURN_TYPE =~ s/\bQStringList\b/list of str/;
                if ( $RETURN_TYPE =~ m/^(?:QList|QVector)<\s*(.*?)[\s*\*]*>$/ ){
                    $RETURN_TYPE = "list of $1";
                }
                if ( $RETURN_TYPE =~ m/^QSet<\s*(.*?)[\s*\*]*>$/ ){
                    $RETURN_TYPE = "set of $1";
                }
            }
        }
    };

    # deleted functions
    if ( $LINE =~  m/^(\s*)?(const )?(virtual |static )?((\w+(<.*?>)?\s+(\*|&)?)?(\w+|operator.{1,2})\(.*?(\(.*\))*.*\)( const)?)\s*= delete;(\s*\/\/.*)?$/ ){
      $COMMENT = '';
      next;
    }

    # remove export macro from struct definition
    $LINE =~ s/^(\s*struct )\w+_EXPORT (.+)$/$1$2/;

    # Skip comments
    $COMMENT_TEMPLATE_DOCSTRING = 0;
    if ( $LINE =~ m/^\s*typedef\s+\w+\s*<\s*\w+\s*>\s+\w+\s+.*SIP_DOC_TEMPLATE/ ) {
        # support Docstring for template based classes in SIP 4.19.7+
        $COMMENT_TEMPLATE_DOCSTRING = 1;
    }
    elsif ( $LINE =~ m/\/\// ||
            $LINE =~ m/^\s*typedef / ||
            $LINE =~ m/\s*struct / ||
            $LINE =~ m/operator\[\]\(/ ||
            $LINE =~ m/^\s*operator\b/ ||
            $LINE =~ m/operator\s?[!+-=*\/\[\]<>]{1,2}/ ||
            $LINE =~ m/^\s*%\w+(.*)?$/ ||
            $LINE =~ m/^\s*namespace\s+\w+/ ||
            $LINE =~ m/^\s*(virtual\s*)?~/ ||
            detect_non_method_member() == 1 ){
        dbg_info('skipping comment');
        dbg_info('because typedef') if ($LINE =~ m/\s*typedef.*?(?!SIP_DOC_TEMPLATE)/);
        $COMMENT = '';
        $RETURN_TYPE = '';
        $IS_OVERRIDE_OR_MAKE_PRIVATE = PREPEND_CODE_NO;
    }

    $LINE = fix_constants($LINE);
    $LINE = fix_annotations($LINE);

    # fix astyle placing space after % character
    $LINE =~ s/\/\s+GetWrapper\s+\//\/GetWrapper\//;

    # handle enum/flags QgsSettingsEntryEnumFlag
    if ( $LINE =~ m/^(\s*)const QgsSettingsEntryEnumFlag<(.*)> (.+);$/ ) {
      my $prep_line = "class QgsSettingsEntryEnumFlag_$3
{
%TypeHeaderCode
#include \"" .basename($headerfile) . "\"
#include \"qgssettingsentry.h\"
typedef QgsSettingsEntryEnumFlag<$2> QgsSettingsEntryEnumFlag_$3;
%End
  public:
    QgsSettingsEntryEnumFlag_$3( const QString &key, QgsSettings::Section section, const $2 &defaultValue, const QString &description = QString() );
    QString key( const QString &dynamicKeyPart = QString() ) const;
    $2 value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const $2 &defaultValueOverride = $2() ) const;
};";
    $LINE = "$1const QgsSettingsEntryEnumFlag_$3 $3;";
    $COMMENT = '';
    write_output("ENF", "$prep_line\n", "prepend");
    }

    write_output("NOR", "$LINE\n");

    if ($PYTHON_SIGNATURE ne '') {
      write_output("PSI", "$PYTHON_SIGNATURE\n");
    }

    # multiline definition (parenthesis left open)
    if ( $MULTILINE_DEFINITION != MULTILINE_NO ){
        dbg_info("on multiline");
        # https://regex101.com/r/DN01iM/4
        if ( $LINE =~ m/^([^()]+(\((?:[^()]++|(?1))*\)))*[^()]*\)([^()](throw\([^()]+\))?)*$/){
            dbg_info("ending multiline");
            # remove potential following body
            if ( $MULTILINE_DEFINITION != MULTILINE_CONDITIONAL_STATEMENT && $LINE !~ m/(\{.*\}|;)\s*(\/\/.*)?$/ ){
                dbg_info("remove following body of multiline def");
                my $last_line = $LINE;
                $last_line .= remove_following_body_or_initializerlist();
                # add missing semi column
                my $dummy = pop(@OUTPUT);
                write_output("MLT", "$last_line;\n");
            }
            $MULTILINE_DEFINITION = MULTILINE_NO;
        }
        else
        {
            next;
        }
    }
    elsif ( $LINE =~ m/^[^()]+\([^()]*([^()]*\([^()]*\)[^()]*)*[^)]*$/ ){
      dbg_info("Multiline detected:: $LINE");
      if ( $LINE =~ m/^\s*((else )?if|while|for) *\(/ ){
          $MULTILINE_DEFINITION = MULTILINE_CONDITIONAL_STATEMENT;
      }
      else {
          $MULTILINE_DEFINITION = MULTILINE_METHOD;
      }
      next;
    }

    # write comment
    if ( $LINE =~ m/^\s*$/ )
    {
        dbg_info("no more override / private");
        $IS_OVERRIDE_OR_MAKE_PRIVATE = PREPEND_CODE_NO;
        next;
    }
    if ( $LINE =~ m/^\s*template\s*<.*>/ ){
        # do not comment now for templates, wait for class definition
        next;
    }
    if ( $COMMENT !~ m/^\s*$/ || $RETURN_TYPE ne ''){
        if ( $IS_OVERRIDE_OR_MAKE_PRIVATE != PREPEND_CODE_VIRTUAL && $COMMENT =~ m/^\s*$/ ){
            # overridden method with no new docs - so don't create a Docstring and use
            # parent class Docstring
        }
        else {
            dbg_info('writing comment');
            if ( $COMMENT !~ m/^\s*$/ ){
                dbg_info('comment non-empty');
                my $doc_prepend = "";
                $doc_prepend = "\@DOCSTRINGSTEMPLATE\@" if $COMMENT_TEMPLATE_DOCSTRING == 1;
                write_output("CM1", "$doc_prepend%Docstring\n");
                my @comment_lines = split /\n/, $COMMENT;
                my $skipping_param = 0;
                my @out_params = ();
                my $waiting_for_return_to_end = 0;
                foreach my $comment_line (@comment_lines) {

                  if ( ( $comment_line =~ m/versionadded:/ || $comment_line =~ m/deprecated:/ ) && $#out_params >= 0 ){
                    dbg_info('out style parameters remain to flush!');
                    # member has /Out/ parameters, but no return type, so flush out out_params docs now
                    my $first_out_param = shift(@out_params);
                    write_output("CM7", "$doc_prepend:return: - $first_out_param\n");

                    foreach my $out_param (@out_params) {
                      write_output("CM7", "$doc_prepend         - $out_param\n");
                    }
                    write_output("CM7", "$doc_prepend\n");
                    @out_params = ();
                  }

                  # if ( $RETURN_TYPE ne '' && $comment_line =~ m/^\s*\.\. \w/ ){
                  #     # return type must be added before any other paragraph-level markup
                  #     write_output("CM5", ":rtype: $RETURN_TYPE\n\n");
                  #     $RETURN_TYPE = '';
                  # }
                  if ( $comment_line =~ m/^:param\s+(\w+)/) {
                    if ( $1 ~~ @SKIPPED_PARAMS_OUT || $1 ~~ @SKIPPED_PARAMS_REMOVE ) {
                      if ( $1 ~~ @SKIPPED_PARAMS_OUT ) {
                        $comment_line =~ s/^:param\s+(\w+):\s*(.*?)$/$1: $2/;
                        $comment_line =~ s/(?:optional|if specified|if given)[,]?\s*//g;
                        push @out_params, $comment_line ;
                        $skipping_param = 2;
                      }
                      else {
                        $skipping_param = 1;
                      }
                      next;
                    }
                  }
                  if ( $skipping_param > 0 ) {
                    if ( $comment_line =~ m/^(:.*|\.\..*|\s*)$/ ){
                      $skipping_param = 0;
                    }
                    elsif ( $skipping_param == 2 ) {
                      $comment_line =~ s/^\s+/ /;
                      $out_params[$#out_params] .= $comment_line;
                      # exit_with_error('Skipped param (SIP_OUT) should have their doc on a single line');
                      next;
                    }
                    else
                    {
                      next;
                    }
                  }
                  if ( $comment_line =~ m/:return:/ && $#out_params >= 0 ){
                    $waiting_for_return_to_end = 1;
                    $comment_line =~ s/:return:/:return: -/;
                    write_output("CM2", "$doc_prepend$comment_line\n");
                    foreach my $out_param (@out_params) {
                      write_output("CM7", "$doc_prepend         - $out_param\n");
                    }
                    @out_params = ();
                  }
                  else {
                    write_output("CM2", "$doc_prepend$comment_line\n");
                  }
                  if ( $waiting_for_return_to_end == 1 ) {
                    if ($comment_line =~ m/^(:.*|\.\..*|\s*)$/) {
                      $waiting_for_return_to_end = 0;
                    }
                    else {
                      # exit_with_error('Return docstring should be single line with SIP_OUT params');
                    }
                  }
                  # if ( $RETURN_TYPE ne '' && $comment_line =~ m/:return:/ ){
                  #     # return type must be added before any other paragraph-level markup
                  #     write_output("CM5", ":rtype: $RETURN_TYPE\n\n");
                  #     $RETURN_TYPE = '';
                  # }
                }
                exit_with_error("A method with output parameters must contain a return directive (method returns ${RETURN_TYPE})") if $#out_params >= 0 and $RETURN_TYPE ne '';
                write_output("CM4", "$doc_prepend%End\n");
            }
            # if ( $RETURN_TYPE ne '' ){
            #     write_output("CM3", "\n:rtype: $RETURN_TYPE\n");
            # }
        }
        $COMMENT = '';
        $RETURN_TYPE = '';
        if ($IS_OVERRIDE_OR_MAKE_PRIVATE == PREPEND_CODE_MAKE_PRIVATE){
          write_output("MKP", $LAST_ACCESS_SECTION_LINE);
        }
        $IS_OVERRIDE_OR_MAKE_PRIVATE = PREPEND_CODE_NO;
    }
    else {
        if ($IS_OVERRIDE_OR_MAKE_PRIVATE == PREPEND_CODE_MAKE_PRIVATE){
          write_output("MKP", $LAST_ACCESS_SECTION_LINE);
        }
        $IS_OVERRIDE_OR_MAKE_PRIVATE = PREPEND_CODE_NO;
    }
}

if ( $sip_output ne ''){
  open(FH, '>', $sip_output) or die $!;
  print FH join('', sip_header_footer());
  print FH join('',@OUTPUT);
  print FH join('', sip_header_footer());
  close(FH);
} else {
  print join('', sip_header_footer());
  print join('',@OUTPUT);
  print join('', sip_header_footer());
}

if ( $python_output ne '' ){
    unlink $python_output or 1;
    if ( @OUTPUT_PYTHON ){
        open(FH2, '>', $python_output) or die $!;
        print FH2 join('', python_header());
        print FH2 join('', @OUTPUT_PYTHON);
        close(FH2);
    }
}
