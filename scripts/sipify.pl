#!/usr/bin/perl
use strict;
use warnings;
use File::Basename;

use constant PRIVATE => 0;
use constant PROTECTED => 1;
use constant PUBLIC => 2;

# TODO add contexts for
# "multiline function signatures"
# docustrings for QgsFeature::QgsAttributes

sub processDoxygenLine
{
    my $line = $_[0];
    # remove \a formatting
    $line =~ s/\\a (.+?)\b/``$1``/g;
    # replace :: with . (changes c++ style namespace/class directives to Python style)
    $line =~ s/::/./g;
    # replace nullptr with None (nullptr means nothing to Python devs)
    $line =~ s/\bnullptr\b/None/g;
	# replace \returns with :return:
	$line =~ s/\\return(s)?/:return:/g;

    if ( $line =~ m/[\\@](ingroup|class)/ ) {
        return ""
    }
    if ( $line =~ m/\\since .*?([\d\.]+)/i ) {
        return ".. versionadded:: $1\n";
    }
    if ( $line =~ m/[\\@]note (.*)/ ) {
        return ".. note::\n\n   $1\n";
    }
    if ( $line =~ m/[\\@]brief (.*)/ ) {
        return " $1\n";
    }
    return "$line\n";
}

my $headerfile = $ARGV[0];

open(my $handle, "<", $headerfile) || die "Couldn't open '".$headerfile."' for reading because: ".$!;
chomp(my @lines = <$handle>);
close $handle;

# contexts
my $SIP_RUN = 0;
my $HEADER_CODE = 0;
my $ACCESS = PUBLIC;
my $MULTILINE_DEFINITION = 0;

my $comment = '';
my $global_nesting_index = 0;
my $private_section_line = '';
my $classname = '';
my $return_type = '';
my $is_override = 0;
my %qflag_hash;

my $line_count = @lines;
my $line_idx = 0;
my $line;
my @output = ();

push @output, "/************************************************************************\n";
push @output, " * This file has been generated automatically from                      *\n";
push @output, " *                                                                      *\n";
push @output, sprintf " * %-*s *\n", 68, $headerfile;
push @output, " *                                                                      *\n";
push @output, " * Do not edit manually ! Edit header and run scripts/sipify.pl again   *\n";
push @output, " ************************************************************************/\n";

while ($line_idx < $line_count){
    $line = $lines[$line_idx];
    $line_idx++;
    #print "$line\n";

    if ($line =~ m/^\s*SIP_FEATURE\( (\w+) \)(.*)$/){
        push @output, "%Feature $1$2\n";
        next;
    }
    if ($line =~ m/^\s*SIP_IF_FEATURE\( (\!?\w+) \)(.*)$/){
        push @output, "%If ($1)$2\n";
        next;
    }
    if ($line =~ m/^\s*SIP_CONVERT_TO_SUBCLASS_CODE(.*)$/){
        push @output, "%ConvertToSubClassCode$1\n";
        next;
    }

    if ($line =~ m/^\s*SIP_END(.*)$/){
        push @output, "%End$1\n";
        next;
    }

    # Skip preprocessor stuff
    if ($line =~ m/^\s*#/){

        # skip #if 0 blocks
        if ( $line =~ m/^\s*#if 0/){
          my $nesting_index = 0;
          while ($line_idx < $line_count){
              $line = $lines[$line_idx];
              $line_idx++;
              if ( $line =~ m/^\s*#if(def)?\s+/ ){
                  $nesting_index++;
              }
              elsif ( $nesting_index == 0 && $line =~ m/^\s*#(endif|else)/ ){
                  $comment = '';
                  last;
              }
              elsif ( $nesting_index != 0 && $line =~ m/^\s*#(endif)/ ){
                      $nesting_index--;
              }
          }
          next;
        }

        if ( $line =~ m/^\s*#ifdef SIP_RUN/){
            $SIP_RUN = 1;
            if ($ACCESS == PRIVATE){
                push @output, $private_section_line."\n";
            }
            next;
        }
        if ( $SIP_RUN == 1 ){
            if ( $line =~ m/^\s*#endif/ ){
                if ( $global_nesting_index == 0 ){
                    $SIP_RUN = 0;
                    next;
                }
                else {
                    $global_nesting_index--;
                }
            }
            if ( $line =~ m/^\s*#if(def)?\s+/ ){
                $global_nesting_index++;
            }

            # if there is an else at this level, code will be ignored i.e. not SIP_RUN
            if ( $line =~ m/^\s*#else/ && $global_nesting_index == 0){
                while ($line_idx < $line_count){
                    $line = $lines[$line_idx];
                    $line_idx++;
                    if ( $line =~ m/^\s*#if(def)?\s+/ ){
                        $global_nesting_index++;
                    }
                    elsif ( $line =~ m/^\s*#endif/ ){
                        if ( $global_nesting_index == 0 ){
                            $comment = '';
                            $SIP_RUN = 0;
                            last;
                        }
                        else {
                            $global_nesting_index--;
                        }
                    }
                }
                next;
            }
        }
        elsif ( $line =~ m/^\s*#ifndef SIP_RUN/){
            # code is ignored here
            while ($line_idx < $line_count){
                $line = $lines[$line_idx];
                $line_idx++;
                if ( $line =~ m/^\s*#if(def)?\s+/ ){
                    $global_nesting_index++;
                }
                elsif ( $line =~ m/^\s*#else/ && $global_nesting_index == 0 ){
                    # code here will be printed out
                    if ($ACCESS == PRIVATE){
                        push @output, $private_section_line."\n";
                    }
                    $SIP_RUN = 1;
                    last;
                }
                elsif ( $line =~ m/^\s*#endif/ ){
                    if ( $global_nesting_index == 0 ){
                        $comment = '';
                        $SIP_RUN = 0;
                        last;
                    }
                    else {
                        $global_nesting_index--;
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
        push @output, "%End\n";
    }

    # Skip forward declarations
    if ($line =~ m/^\s*class \w+;$/){
        next;
    }
    # Skip Q_OBJECT, Q_PROPERTY, Q_ENUM, Q_GADGET
    if ($line =~ m/^\s*Q_(OBJECT|ENUMS|PROPERTY|GADGET|DECLARE_METATYPE).*?$/){
        next;
    }

    # SIP_SKIP
    if ( $line =~ m/SIP_SKIP/ ){
      $comment = '';
      # if multiline definition, remove previous lines
      if ( $MULTILINE_DEFINITION == 1){
        my $opening_line = '';
        while ( $opening_line !~ m/^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$/){
            $opening_line = pop(@output);
            $#output >= 0 or die 'could not reach opening definition';
        }
        $MULTILINE_DEFINITION = 0;
      }
      # also skip method body if there is one
      if ($lines[$line_idx] =~ m/^\s*\{/){
        my $nesting_index = 0;
        while ($line_idx < $line_count){
            $line = $lines[$line_idx];
            $line_idx++;
            if ( $nesting_index == 0 ){
                if ( $line =~ m/^\s*(:|,)/ ){
                    next;
                }
                $line =~ m/^\s*\{/ or die 'Constructor definition misses {';
                if ( $line =~ m/^\s*\{.*?\}/ ){
                    last;
                }
                $nesting_index = 1;
                next;
            }
            else {
                $nesting_index += $line =~ tr/\{//;
                $nesting_index -= $line =~ tr/\}//;
                if ($nesting_index eq 0){
                    last;
                }
            }
        }
      }
      # line skipped, go to next iteration
      next;
    }

    # Private members (exclude SIP_RUN)
    if ( $line =~ m/^\s*private( slots)?:/ ){
        $ACCESS = PRIVATE;
        $private_section_line = $line;
        $comment = '';
        next;
    }
    elsif ( $line =~ m/^\s*(public)( slots)?:.*$/ ){
        $ACCESS = PUBLIC;
        $comment = '';
    }
    elsif ( $line =~ m/^\};.*$/ ) {
        $ACCESS = PUBLIC;
        $comment = '';
    }
    elsif ( $line =~ m/^\s*(protected)( slots)?:.*$/ ){
        $ACCESS = PROTECTED;
        $comment = '';
    }
    elsif ( $ACCESS == PRIVATE && $line =~ m/SIP_FORCE/){
        push @output, $private_section_line."\n";
    }
    elsif ( $ACCESS == PRIVATE && $SIP_RUN == 0 ) {
      $comment = '';
      next;
    }
    # Skip assignment operator
    if ( $line =~ m/operator=\s*\(/ ){
        push @output, "// $line";
        next;
    }

    # Detect comment block
    if ($line =~ m/^\s*\/\*/){
        do {no warnings 'uninitialized';
            $comment = processDoxygenLine( $line =~ s/^\s*\/\*(\*)?(.*?)\n?$/$2/r );
        };
        $comment =~ s/^\s*$//;
        #$comment =~ s/^(\s*\n)*(.+)/$2/;
        while ($line_idx < $line_count){
            $line = $lines[$line_idx];
            $line_idx++;
            $comment .= processDoxygenLine( $line =~ s/\s*\*?(.*?)(\/)?\n?$/$1/r );
            if ( $line =~ m/\*\/\s*(\/\/.*?)?$/ ){
                last;
            }
        }
        $comment =~ s/\n+$//;
        #push @output, $comment;
        next;
    }

    # save comments and do not print them, except in SIP_RUN
    if ( $SIP_RUN == 0 ){
        if ( $line =~ m/^\s*\/\// ){
            $line =~ s/^\s*\/\/\!*\s*(.*?)\n?$/$1/;
            $comment = processDoxygenLine( $line );
            $comment =~ s/\n+$//;
            next;
        }
    }

    if ( $line =~ m/^(\s*struct)\s+(\w+)$/ ) {
      $ACCESS = PUBLIC;
    }

    # class declaration started
    if ( $line =~ m/^(\s*class)\s*([A-Z]+_EXPORT)?\s+(\w+)(\s*\:.*)?(\s*SIP_ABSTRACT)?$/ ){
        do {no warnings 'uninitialized';
            $classname = $3;
            $line =~ m/\b[A-Z]+_EXPORT\b/ or die "Class$classname in $headerfile should be exported with appropriate [LIB]_EXPORT macro. If this should not be available in python, wrap it in a `#ifndef SIP_RUN` block.";
        };
        $line = "$1 $3";
        # Inheritance
        if ($4){
            my $m = $4;
            $m =~ s/public //g;
            $m =~ s/,?\s*private \w+(::\w+)?//;
            $m =~ s/(\s*:)?\s*$//;
            $line .= $m;
        }
        if ($5) {
            $line .= ' /Abstract/';
        }

        $line .= "\n{\n";
        if ( $comment !~ m/^\s*$/ ){
            $line .= "%Docstring\n$comment\n%End\n";
        }
        $line .= "\n%TypeHeaderCode\n#include \"" . basename($headerfile) . "\"";

        push @output, "$line\n";

        # Skip opening curly bracket, we already added that above
        my $skip = $lines[$line_idx];
        $line_idx++;
        $skip =~ m/^\s*{\s*$/ || die "Unexpected content on line $skip";

        $comment = '';
        $HEADER_CODE = 1;
        $ACCESS = PRIVATE;
        next;
    }

    # Enum declaration
    if ( $line =~ m/^\s*enum\s+\w+.*?$/ ){
        push @output, "$line\n";
        if ($line =~ m/\{((\s*\w+)(\s*=\s*[\w\s\d<|]+.*?)?(,?))+\s*\}/){
          # one line declaration
          $line !~ m/=/ or die 'spify.pl does not handle enum one liners with value assignment. Use multiple lines instead.';
          next;
        }
        else
        {
            $line = $lines[$line_idx];
            $line_idx++;
            $line =~ m/^\s*\{\s*$/ || die "Unexpected content: enum should be followed by {\nline: $line";
            push @output, "$line\n";
            while ($line_idx < $line_count){
                $line = $lines[$line_idx];
                $line_idx++;
                if ($line =~ m/\};/){
                    last;
                }
                $line =~ s/(\s*\w+)(\s*=\s*[\w\s\d<|]+.*?)?(,?).*$/$1$3/;
                push @output, "$line\n";
            }
            push @output, "$line\n";
            # enums don't have Docstring apparently
            $comment = '';
            next;
        }
    }

    # skip non-method member declaration in non-public sections
    if ( $SIP_RUN != 1 && $ACCESS != PUBLIC && $line =~ m/^\s*(?:mutable\s)?\w+[\w<> *&:,]* \*?\w+( = \w+(\([^()]+\))?)?;/){
        next;
    }

    # remove struct member assignment
    if ( $SIP_RUN != 1 && $ACCESS == PUBLIC && $line =~ m/^(\s*\w+[\w<> *&:,]* \*?\w+) = \w+(\([^()]+\))?;/ ){
        $line = "$1;";
    }

    # catch Q_DECLARE_FLAGS
    if ( $line =~ m/^(\s*)Q_DECLARE_FLAGS\(\s*(.*?)\s*,\s*(.*?)\s*\)\s*$/ ){
        $line = "$1typedef QFlags<$classname::$3> $2;\n";
        $qflag_hash{"$classname::$2"} = "$classname::$3";
    }
    # catch Q_DECLARE_OPERATORS_FOR_FLAGS
    if ( $line =~ m/^(\s*)Q_DECLARE_OPERATORS_FOR_FLAGS\(\s*(.*?)\s*\)\s*$/ ){
        my $flag = $qflag_hash{$2};
        $line = "$1QFlags<$flag> operator|($flag f1, QFlags<$flag> f2);\n";
    }

    # remove Q_INVOKABLE
    $line =~ s/^(\s*)Q_INVOKABLE /$1/;

    do {no warnings 'uninitialized';
        # remove keywords
        if ( $line =~ m/\boverride\b/){
            $is_override = 1;

            # handle multiline definition to add virtual keyword on opening line
            if ( $MULTILINE_DEFINITION == 1 ){
                my $virtual_line = $line;
                my $virtual_line_idx = $line_idx;
                while ( $virtual_line !~ m/^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$/){
                    $virtual_line_idx--;
                    $virtual_line = $lines[$virtual_line_idx];
                    $virtual_line_idx >= 0 or die 'could not reach opening definition';
                }
                if ( $virtual_line !~ m/^(\s*)virtual\b(.*)$/ ){
                    my $idx = $#output-$line_idx+$virtual_line_idx+2;
                    #print "len: $#output line_idx: $line_idx virt: $virtual_line_idx\n"idx: $idx\n$output[$idx]\n";
                    $output[$idx] = $virtual_line =~ s/^(\s*?)\b(.*)$/$1 virtual $2\n/r;
                }
            }
            elsif ( $line !~ m/^(\s*)virtual\b(.*)$/ ){
                #sip often requires the virtual keyword to be present, or it chokes on covariant return types
                #in overridden methods
                $line =~ s/^(\s*?)\b(.*)$/$1virtual $2\n/;
            }
        }
        $line =~ s/\s*\boverride\b//;
        $line =~ s/^(\s*)?(const )?(virtual |static )?inline /$1$2$3/;
        $line =~ s/\bnullptr\b/0/g;
        $line =~ s/\s*=\s*default\b//g;

        # remove constructor definition
        if ( $line =~  m/^(\s*)?(explicit )?(\w+)\([\w\=\(\)\s\,\&\*\<\>]*\)(?!;)$/ ){
            my $newline = $line =~ s/\n/;\n/r;
            my $nesting_index = 0;
            while ($line_idx < $line_count){
                $line = $lines[$line_idx];
                $line_idx++;
                if ( $nesting_index == 0 ){
                    if ( $line =~ m/^\s*(:|,)/ ){
                        next;
                    }
                    $line =~ m/^\s*\{/ or die 'Constructor definition misses {';
                    if ( $line =~ m/^\s*\{.*?\}/ ){
                        last;
                    }
                    $nesting_index = 1;
                    next;
                }
                else {
                    $nesting_index += $line =~ tr/\{//;
                    $nesting_index -= $line =~ tr/\}//;
                    if ($nesting_index eq 0){
                        last;
                    }
                }
            }
            $line = $newline;
        }

        # remove function bodies
        if ( $SIP_RUN != 1 && $line =~  m/^(\s*)?(virtual )?(static |const )*(([\w:]+(<.*?>)?\s+(\*|&)?)?(\w+|operator.{1,2})\(.*?(\(.*\))*.*\)( (?:const|SIP_[A-Z_]*?))*)\s*(\{.*\})?(?!;)(\s*\/\/.*)?$/ ){
            my $newline = "$1$2$3$4;";
            if ($line !~ m/\{.*?\}$/){
                $line = $lines[$line_idx];
                $line_idx++;
                my $nesting_index = 1;
                if ( $line =~ m/^\s*\{\s*$/ ){
                    while ($line_idx < $line_count){
                        $line = $lines[$line_idx];
                        $line_idx++;
                        if ( $line =~ m/^\s*{/ ){
                            $nesting_index++;
                        }
                        elsif ( $line =~ m/\}\s*$/ ){
                            $nesting_index--;
                            if ($nesting_index == 0){
                                last;
                            }
                        }
                    }
                }
            }
            $line = $newline;
        }

        # remove inline declarations
        if ( $line =~  m/^(\s*)?(static |const )*(([\w:]+(<.*?>)?\s+(\*|&)?)?(\w+)( (?:const*?))*)\s*(\{.*\});(\s*\/\/.*)?$/ ){
            my $newline = "$1$3;";
            $line = $newline;
        }

        if ( $line =~  m/^\s*(?:const |virtual |static |inline )*(?!explicit)([\w:]+(?:<.*?>)?)\s+(?:\*|&)?(?:\w+|operator.{1,2})\(.*$/ ){
            if ($1 !~ m/(void|SIP_PYOBJECT|operator|return|QFlag)/ ){
                $return_type = $1;
                # replace :: with . (changes c++ style namespace/class directives to Python style)
                $return_type =~ s/::/./g;

                # replace with builtin Python types
                $return_type =~ s/\bdouble\b/float/;
                $return_type =~ s/\bQString\b/str/;
                $return_type =~ s/\bQStringList\b/list of str/;
                if ( $return_type =~ m/^(?:QList|QVector)<\s*(.*?)[\s*\*]*>$/ ){
                    $return_type = "list of $1";
                }
                if ( $return_type =~ m/^QSet<\s*(.*?)[\s*\*]*>$/ ){
                    $return_type = "set of $1";
                }
            }
        }
    };

    # deleted functions
    if ( $line =~  m/^(\s*)?(const )?(virtual |static )?((\w+(<.*?>)?\s+(\*|&)?)?(\w+|operator.{1,2})\(.*?(\(.*\))*.*\)( const)?)\s*= delete;(\s*\/\/.*)?$/ ){
      $comment = '';
      next;
    }

    # remove export macro from struct definition
    $line =~ s/^(\s*struct )\w+_EXPORT (.+)$/$1$2/;

    $line =~ s/\bSIP_FACTORY\b/\/Factory\//;
    $line =~ s/\bSIP_OUT\b/\/Out\//g;
    $line =~ s/\bSIP_INOUT\b/\/In,Out\//g;
    $line =~ s/\bSIP_TRANSFER\b/\/Transfer\//g;
    $line =~ s/\bSIP_KEEPREFERENCE\b/\/KeepReference\//;
    $line =~ s/\bSIP_TRANSFERTHIS\b/\/TransferThis\//;
    $line =~ s/\bSIP_TRANSFERBACK\b/\/TransferBack\//;
    $line =~ s/\bSIP_RELEASEGIL\b/\/ReleaseGIL\//;
    $line =~ s/\bSIP_ARRAY\b/\/Array\//;
    $line =~ s/\bSIP_ARRAYSIZE\b/\/ArraySize\//;

    $line =~ s/SIP_PYNAME\(\s*(\w+)\s*\)/\/PyName=$1\//;
    $line =~ s/(\w+)(\<(?>[^<>]|(?2))*\>)?\s+SIP_PYTYPE\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/$3/g;
    $line =~ s/=\s+[^=]*?\s+SIP_PYDEFAULTVALUE\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/= $1/g;

    $line =~ s/SIP_FORCE//;

    # fix astyle placing space after % character
    $line =~ s/\s*% (MappedType|TypeCode|TypeHeaderCode|ConvertFromTypeCode|ConvertToTypeCode|MethodCode|End)/%$1/;
    $line =~ s/\/\s+GetWrapper\s+\//\/GetWrapper\//;

    push @output, "$line\n";

    # multiline definition (parenthesis left open)
    if ( $MULTILINE_DEFINITION == 1 ){
      if ( $line =~ m/^[^()]*([^()]*\([^()]*\)[^()]*)*\)[^()]*$/){
          $MULTILINE_DEFINITION = 0;
      }
      else
      {
        next;
      }
    }
    elsif ( $line =~ m/^[^()]+\([^()]*([^()]*\([^()]*\)[^()]*)*[^)]*$/ ){
      $MULTILINE_DEFINITION = 1;
      next;
    }

    # write comment
    if ( $line =~ m/^\s*$/ )
    {
        $is_override = 0;
        next;
    }
    elsif ( $line =~ m/\/\// || $line =~ m/\s*typedef / || $line =~ m/\s*struct / ){
        $comment = '';
    }
    elsif ( $comment !~ m/^\s*$/ || $return_type ne ''){
        if ( $is_override == 1 && $comment =~ m/^\s*$/ ){
            # overridden method with no new docs - so don't create a Docstring and use
            # parent class Docstring
        }
        else {
            push @output, "%Docstring\n";
            if ( $comment !~ m/^\s*$/ ){
                push @output, "$comment\n";
            }
            if ($return_type ne '' ){
                push @output, " :rtype: $return_type\n";
            }
            push @output, "%End\n";
        }
        $comment = '';
        $return_type = '';
        $is_override = 0;
    }
}
push @output,  "/************************************************************************\n";
push @output,  " * This file has been generated automatically from                      *\n";
push @output,  " *                                                                      *\n";
push @output, sprintf " * %-*s *\n", 68, $headerfile;
push @output,  " *                                                                      *\n";
push @output,  " * Do not edit manually ! Edit header and run scripts/sipify.pl again   *\n";
push @output,  " ************************************************************************/\n";


print join('',@output);
