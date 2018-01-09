#!/usr/bin/perl
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

# read arguments
my $debug = 0;
die("usage: $0 [-debug] headerfile\n") unless GetOptions ("debug" => \$debug) && @ARGV == 1;
my $headerfile = $ARGV[0];

# read file
open(my $handle, "<", $headerfile) || die "Couldn't open '".$headerfile."' for reading because: ".$!;
chomp(my @INPUT_LINES = <$handle>);
close $handle;

# config
my $cfg_file = File::Spec->catfile( dirname(__FILE__), 'sipify.yaml' );
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

my $COMMENT = '';
my $COMMENT_PARAM_LIST = 0;
my $CODE_SNIPPET = 0;
my $GLOB_IFDEF_NESTING_IDX = 0;
my @GLOB_BRACKET_NESTING_IDX = (0);
my $PRIVATE_SECTION_LINE = '';
my $RETURN_TYPE = '';
my $IS_OVERRIDE = 0;
my $IF_FEATURE_CONDITION = '';
my %QFLAG_HASH;

my $LINE_COUNT = @INPUT_LINES;
my $LINE_IDX = 0;
my $LINE;
my @OUTPUT = ();


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
                                  $IS_OVERRIDE,
                                  $ACTUAL_CLASS,
                                  $#CLASSNAME)." :: ".$new_line."\n";
    return $new_line;
}

sub write_output {
    my ($dbg_code, $out) = @_;
    if ($debug == 1){
        $dbg_code = sprintf("%d %-4s :: ", $LINE_IDX, $dbg_code);
    }
    else{
        $dbg_code = '';
    }
    push @OUTPUT, "%If ($IF_FEATURE_CONDITION)\n" if $IF_FEATURE_CONDITION ne '';
    push @OUTPUT, $dbg_code.$out;
    push @OUTPUT, "%End\n" if $IF_FEATURE_CONDITION ne '';
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

sub write_header_footer {
    # small hack to turn files src/core/3d/X.h to src/core/./3d/X.h
    # otherwise "sip up to date" test fails. This is because the test uses %Include entries
    # and over there we have to use ./3d/X.h entries because SIP parser does not allow a number
    # as the first letter of a relative path
    my $headerfile_x = $headerfile;
    $headerfile_x =~ s/src\/core\/3d/src\/core\/.\/3d/;
    push @OUTPUT,  "/************************************************************************\n";
    push @OUTPUT,  " * This file has been generated automatically from                      *\n";
    push @OUTPUT,  " *                                                                      *\n";
    push @OUTPUT, sprintf " * %-*s *\n", 68, $headerfile_x;
    push @OUTPUT,  " *                                                                      *\n";
    push @OUTPUT,  " * Do not edit manually ! Edit header and run scripts/sipify.pl again   *\n";
    push @OUTPUT,  " ************************************************************************/\n";
}

sub processDoxygenLine {
    my $line = $_[0];

    # detect code snippet
    if ( $line =~ m/\\code(\{\.(\w+)\})?/ ) {
        my $codelang = "";
        $codelang = " $2" if (defined $2);
        $CODE_SNIPPET = CODE_SNIPPET;
        $CODE_SNIPPET = CODE_SNIPPET_CPP if ($codelang =~ m/cpp/ );
        $codelang =~ s/py/python/;
        return "\n" if ( $CODE_SNIPPET == CODE_SNIPPET_CPP );
        return ".. code-block::$codelang\n\n";
    }
    if ( $line =~ m/\\endcode/ ) {
        $CODE_SNIPPET = 0;
        return "\n";
    }
    if ($CODE_SNIPPET != 0){
        if ( $CODE_SNIPPET == CODE_SNIPPET_CPP ){
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
    # replace \returns with :return:
    $line =~ s/\s*\\return(s)?/\n:return:/;

    # params
    if ( $line =~ m/\\param / ){
        $line =~ s/\s*\\param (\w+)\b/:param $1:/g;
        if ( $COMMENT_PARAM_LIST == 0 )
        {
            $line = "\n$line";
        }
        $COMMENT_PARAM_LIST = 1;
    }

    if ( $line =~ m/^\s*[\\@]brief/){
        $line =~ s/[\\@]brief//;
        if ( $line =~ m/^\s*$/ ){
            return "";
        }
    }

    if ( $line =~ m/[\\@](ingroup|class)/ ) {
        return "";
    }
    if ( $line =~ m/\\since .*?([\d\.]+)/i ) {
        return "\n.. versionadded:: $1\n";
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
    else
    {
        # create links in plain text too (less performant)
        if ( $line =~ m/\b(Qgs[A-Z]\w+)\b(\.?$|[^\w]{2})/) {
            if ( defined $ACTUAL_CLASS && $1 !~ $ACTUAL_CLASS ) {
                $line =~ s/\b(Qgs[A-Z]\w+)\b(\.?$|[^\w]{2})/:py:class:`$1`$2/g;
            }
        }
        $line =~ s/\b(Qgs[A-Z]\w+\.[a-z]\w+\(\))(\.|\b|$)/:py:func:`$1`/g;
    }

    if ( $line =~ m/[\\@]note (.*)/ ) {
        return "\n.. note::\n\n   $1\n";
    }
    if ( $line =~ m/[\\@]warning (.*)/ ) {
        return "\n.. warning::\n\n   $1\n";
    }

    return "$line\n";
}

sub detect_and_remove_following_body_or_initializerlist {
    # https://regex101.com/r/ZaP3tC/8
    my $python_signature = '';
    do {no warnings 'uninitialized';
        if ( $LINE =~  m/^(\s*)?((?:(?:explicit|static|const|unsigned|virtual)\s+)*)(([\w:]+(<.*?>)?\s+[*&]?)?(~?\w+|(\w+::)?operator.{1,2})\s*\(([\w=()\/ ,&*<>."-]|::)*\)( +(?:const|SIP_[\w_]+?))*)\s*((\s*[:,]\s+\w+\(.*\))*\s*\{.*\}\s*(?:SIP_[\w_]+)?;?|(?!;))(\s*\/\/.*)?$/
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
    $line =~ s/\bSIP_TRANSFER\b/\/Transfer\//g;
    $line =~ s/\bSIP_TRANSFERBACK\b/\/TransferBack\//;
    $line =~ s/\bSIP_TRANSFERTHIS\b/\/TransferThis\//;

    $line =~ s/SIP_PYNAME\(\s*(\w+)\s*\)/\/PyName=$1\//;
    $line =~ s/SIP_VIRTUALERRORHANDLER\(\s*(\w+)\s*\)/\/VirtualErrorHandler=$1\//;

    # combine multiple annotations
    # https://regex101.com/r/uvCt4M/3
    do {no warnings 'uninitialized';
        $line =~ s/\/(\w+(=\w+)?)\/\s*\/(\w+(=\w+)?)\//\/$1,$3\//;
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
    return $line;
}

# detect a comment block, return 1 if found
# if STRICT comment block shall begin at beginning of line (no code in front)
sub detect_comment_block{
    my %args = ( strict_mode => STRICT, @_ );
    # dbg_info("detect comment strict:" . $args{strict_mode} );
    $COMMENT_PARAM_LIST = 0;
    $CODE_SNIPPET = 0;
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
# https://regex101.com/r/gUBZUk/10
sub detect_non_method_member{
  return 1 if $LINE =~ m/^\s*(?:template\s*<\w+>\s+)?(?:(const|mutable|static|friend|unsigned)\s+)*\w+(::\w+)?(<([\w<> *&,()]|::)+>)?(,?\s+\*?\w+( = (-?\d+(\.\d+)?|(\w+::)*\w+(\([^()]+\))?)|\[\d+\])?)+;/;
  return 0;
}


write_header_footer();

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
    if ( $SIP_RUN == 1 && $LINE =~ m/^ *% *(VirtualErrorHandler|MappedType|Type(?:Header)?Code|Module(?:Header)?Code|Convert(?:From|To)(?:Type|SubClass)Code|MethodCode)(.*)?$/ ){
        $LINE = "%$1$2";
        $COMMENT = '';
        dbg_info("do not process SIP code");
        while ( $LINE !~ m/^ *% *End/ ){
            write_output("COD", $LINE."\n");
            $LINE = read_line();
            $LINE =~ s/^ *% *(VirtualErrorHandler|MappedType|Type(?:Header)?Code|Module(?:Header)?Code|Convert(?:From|To)(?:Type|SubClass)Code|MethodCode)(.*)?$/%$1$2/;
            $LINE =~ s/^\s*SIP_END(.*)$/%End$1/;
        }
        $LINE =~ s/^\s*% End/%End/;
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
    if ($LINE =~ m/^\s*(class|struct) \w+(?<external> *SIP_EXTERNAL)?;\s*(\/\/.*)?$/){
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
    # Skip Q_OBJECT, Q_PROPERTY, Q_ENUM, Q_GADGET etc.
    if ($LINE =~ m/^\s*Q_(OBJECT|ENUMS|ENUM|PROPERTY|GADGET|DECLARE_METATYPE|DECLARE_TYPEINFO|DECL_DEPRECATED|NOWARN_DEPRECATED_(PUSH|POP)).*?$/){
        next;
    }

    # SIP_SKIP
    if ( $LINE =~ m/SIP_SKIP/ ){
        dbg_info('SIP SKIP!');
        $COMMENT = '';
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
    # https://regex101.com/r/6FWntP/10
    if ( $LINE =~ m/^(\s*class)\s+([A-Z]+_EXPORT\s+)?(\w+)(\s*\:\s*(public|protected|private)\s+\w+(< *(\w|::)+ *>)?(::\w+(<\w+>)?)*(,\s*(public|protected|private)\s+\w+(< *(\w|::)+ *>)?(::\w+(<\w+>)?)*)*)?(?<annot>\s*\/?\/?\s*SIP_\w+)?\s*?(\/\/.*|(?!;))$/ ){
        dbg_info("class definition started");
        push @ACCESS, PUBLIC;
        push @EXPORTED, 0;
        push @GLOB_BRACKET_NESTING_IDX, 0;
        my @template_inheritance_template = ();
        my @template_inheritance_class = ();
        do {no warnings 'uninitialized';
            push @CLASSNAME, $3;
            if ($#CLASSNAME == 0){
                # might be worth to add in-class classes later on
                # in case of a tamplate based class declaration
                # based on an in-class and in the same file
                push @DECLARED_CLASSES, $CLASSNAME[$#CLASSNAME];
            }
            dbg_info("class: ".$CLASSNAME[$#CLASSNAME]);
            if ($LINE =~ m/\b[A-Z]+_EXPORT\b/ || $#CLASSNAME != 0 || $INPUT_LINES[$LINE_IDX-2] =~ m/^\s*template</){
                # class should be exported except those not at top level or template classes
                # if class is not exported, then its methods should be (checked whenever leaving out the class)
                $EXPORTED[-1]++;
            }
        };
        $LINE = "$1 $3";
        # Inheritance
        if ($4){
            my $m = $4;
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
            $LINE .= "%Docstring\n$COMMENT\n%End\n";
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
                    # top level should stasy public
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
        $PRIVATE_SECTION_LINE = $LINE;
        $COMMENT = '';
        dbg_info("going private");
        next;
    }
    elsif ( $LINE =~ m/^\s*(public( slots)?|signals):.*$/ ){
        dbg_info("going public");
        $ACCESS[$#ACCESS] = PUBLIC;
        $COMMENT = '';
    }
    elsif ( $LINE =~ m/^\s*(protected)( slots)?:.*$/ ){
        dbg_info("going protected");
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
    if ( $LINE =~ m/^\s*enum\s+\w+.*?$/ ){
        write_output("ENU1", "$LINE\n");
        if ($LINE =~ m/\{((\s*\w+)(\s*=\s*[\w\s\d<|]+.*?)?(,?))+\s*\}/){
          # one line declaration
          $LINE !~ m/=/ or exit_with_error("spify.pl does not handle enum one liners with value assignment. Use multiple lines instead.");
          next;
        }
        else
        {
            $LINE = read_line();
            $LINE =~ m/^\s*\{\s*$/ or exit_with_error('Unexpected content: enum should be followed by {');
            write_output("ENU2", "$LINE\n");
            while ($LINE_IDX < $LINE_COUNT){
                $LINE = read_line();
                if (detect_comment_block()){
                    next;
                }
                if ($LINE =~ m/\};/){
                    last;
                }
                do {no warnings 'uninitialized';
                    my $enum_decl = $LINE =~ s/(\s*\w+)(\s+SIP_\w+(?:\([^()]+\))?)?(?:\s*=\s*(?:[\w\s\d|+-]|::|<<)+.*?)?(,?).*$/$1$2$3/r;
                    $enum_decl = fix_annotations($enum_decl);
                    write_output("ENU3", "$enum_decl\n");
                };
                detect_comment_block(strict_mode => UNSTRICT);
            }
            write_output("ENU4", "$LINE\n");
            # enums don't have Docstring apparently
            $COMMENT = '';
            next;
        }
    }

    $IS_OVERRIDE = 1 if ( $LINE =~ m/\boverride\b/);

    # keyword fixes
    do {no warnings 'uninitialized';
        $LINE =~ s/^(\s*template\s*<)(?:class|typename) (\w+>)(.*)$/$1$2$3/;
        $LINE =~ s/\s*\boverride\b//;
        $LINE =~ s/\s*\bextern \b//;
        $LINE =~ s/^(\s*)?(const )?(virtual |static )?inline /$1$2$3/;
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
    if ( $SIP_RUN != 1 && $ACCESS[$#ACCESS] == PUBLIC && $LINE =~ m/^(\s*\w+[\w<> *&:,]* \*?\w+) = [\-\w\:\.]+(\([^()]+\))?\s*;/ ){
        dbg_info("remove struct member assignment");
        $LINE = "$1;";
    }

    # catch Q_DECLARE_FLAGS
    if ( $LINE =~ m/^(\s*)Q_DECLARE_FLAGS\(\s*(.*?)\s*,\s*(.*?)\s*\)\s*$/ ){
        my $ACTUAL_CLASS = $CLASSNAME[$#CLASSNAME];
        dbg_info("Declare flags: $ACTUAL_CLASS");
        $LINE = "$1typedef QFlags<${ACTUAL_CLASS}::$3> $2;\n";
        $QFLAG_HASH{"${ACTUAL_CLASS}::$2"} = "${ACTUAL_CLASS}::$3";
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
        if ( $IS_OVERRIDE == 1 ){
            # handle multiline definition to add virtual keyword on opening line
            if ( $MULTILINE_DEFINITION != MULTILINE_NO ){
                my $virtual_line = $LINE;
                my $virtual_line_idx = $LINE_IDX;
                dbg_info("handle multiline definition to add virtual keyword on opening line");
                while ( $virtual_line !~ m/^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$/){
                    $virtual_line_idx--;
                    $virtual_line = $INPUT_LINES[$virtual_line_idx];
                    $virtual_line_idx >= 0 or exit_with_error('could not reach opening definition');
                }
                if ( $virtual_line !~ m/^(\s*)virtual\b(.*)$/ ){
                    my $idx = $#OUTPUT-$LINE_IDX+$virtual_line_idx+2;
                    #print "len: $#OUTPUT line_idx: $LINE_IDX virt: $virtual_line_idx\n"idx: $idx\n$OUTPUT[$idx]\n";
                    $OUTPUT[$idx] = $virtual_line =~ s/^(\s*?)\b(.*)$/$1 virtual $2\n/r;
                }
            }
            elsif ( $LINE !~ m/^(\s*)virtual\b(.*)$/ ){
                #sip often requires the virtual keyword to be present, or it chokes on covariant return types
                #in overridden methods
                dbg_info('adding virtual keyword for overridden method');
                $LINE =~ s/^(\s*?)\b(.*)$/$1virtual $2\n/;
            }
        }

        # remove constructor definition, function bodies, member initializing list
        $PYTHON_SIGNATURE = detect_and_remove_following_body_or_initializerlist();

        # remove inline declarations
        if ( $LINE =~  m/^(\s*)?(static |const )*(([\w:]+(<.*?>)?\s+(\*|&)?)?(\w+)( (?:const*?))*)\s*(\{.*\});(\s*\/\/.*)?$/ ){
            my $newline = "$1$3;";
            $LINE = $newline;
        }

        if ( $LINE =~  m/^\s*(?:const |virtual |static |inline )*(?!explicit)([\w:]+(?:<.*?>)?)\s+(?:\*|&)?(?:\w+|operator.{1,2})\(.*$/ ){
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

    $LINE = fix_annotations($LINE);

    # fix astyle placing space after % character
    $LINE =~ s/\/\s+GetWrapper\s+\//\/GetWrapper\//;

    write_output("NOR", "$LINE\n");
    if ($PYTHON_SIGNATURE ne ''){
        write_output("PSI", "$PYTHON_SIGNATURE\n");
    }

    # multiline definition (parenthesis left open)
    if ( $MULTILINE_DEFINITION != MULTILINE_NO ){
        dbg_info("on multiline");
        # https://regex101.com/r/DN01iM/2
        if ( $LINE =~ m/^([^()]+(\((?:[^()]++|(?1))*\)))*[^()]*\)[^()]*$/){
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
      dbg_info("Mulitline detected");
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
        $IS_OVERRIDE = 0;
        next;
    }
    if ( $LINE =~ m/^\s*template\s*<.*>/ ){
        # do not comment now for templates, wait for class definition
        next;
    }
    if ( $LINE =~ m/\/\// ||
            $LINE =~ m/\s*typedef / ||
            $LINE =~ m/\s*struct / ||
            $LINE =~ m/operator\[\]\(/ ||
            $LINE =~ m/^\s*operator\b/ ||
            $LINE =~ m/operator\s?[!+-=*\/\[\]<>]{1,2}/ ||
            $LINE =~ m/^\s*%\w+(.*)?$/ ||
            $LINE =~ m/^\s*namespace\s+\w+/ ||
            $LINE =~ m/^\s*(virtual\s*)?~/ ||
            detect_non_method_member() == 1 ){
        dbg_info('skipping comment');
        $COMMENT = '';
        $RETURN_TYPE = '';
        $IS_OVERRIDE = 0;
    }
    elsif ( $COMMENT !~ m/^\s*$/ || $RETURN_TYPE ne ''){
        if ( $IS_OVERRIDE == 1 && $COMMENT =~ m/^\s*$/ ){
            # overridden method with no new docs - so don't create a Docstring and use
            # parent class Docstring
        }
        else {
            dbg_info('writing comment');
            if ( $COMMENT !~ m/^\s*$/ ){
                write_output("CM1", "%Docstring\n");
                my @comment_lines = split /\n/, $COMMENT;
                foreach my $comment_line (@comment_lines) {
                  # if ( $RETURN_TYPE ne '' && $comment_line =~ m/^\s*\.\. \w/ ){
                  #     # return type must be added before any other paragraph-level markup
                  #     write_output("CM5", ":rtype: $RETURN_TYPE\n\n");
                  #     $RETURN_TYPE = '';
                  # }
                  write_output("CM2", "$comment_line\n");
                  # if ( $RETURN_TYPE ne '' && $comment_line =~ m/:return:/ ){
                  #     # return type must be added before any other paragraph-level markup
                  #     write_output("CM5", ":rtype: $RETURN_TYPE\n\n");
                  #     $RETURN_TYPE = '';
                  # }
                }
            write_output("CM4", "%End\n");
            }
            # if ( $RETURN_TYPE ne '' ){
            #     write_output("CM3", "\n:rtype: $RETURN_TYPE\n");
            # }
        }
        $COMMENT = '';
        $RETURN_TYPE = '';
        $IS_OVERRIDE = 0;
    }
    else {
        $IS_OVERRIDE = 0;
    }
}
write_header_footer();

print join('',@OUTPUT);
