#!/usr/bin/perl
use strict;
use warnings;
use File::Basename;

# TODO add contexts for
# "multiline function signatures"
# docustrings for QgsFeature::QgsAttributes

my $headerfile = $ARGV[0];

open(my $header, "<", $headerfile) || die "Couldn't open '".$headerfile."' for reading because: ".$!;

# contexts
my $SIP_RUN = 0;
my $HEADER_CODE = 0;
my $PRIVATE_SECTION = 0;

my $comment = '';
my $nesting_index = 0;
my $private_section_line = '';


print "/******************************************************************\n";
print " * This file has been generated automatically by sipify.pl        *\n";
print " * Do not edit manually ! Edit header file and generate it again. *\n";
print " *****************************************************************/\n";


while(!eof $header) {
    my $line = readline $header;
    #print $line;

    # Skip preprocessor stuff
    if ($line =~ m/^\s*#/) {
        if ( $line =~ m/^\s*#ifdef SIP_RUN/) {

            $SIP_RUN = 1;
            if ($PRIVATE_SECTION == 1) {
                print $private_section_line;
            }
            next;
        }
        if ( $SIP_RUN == 1 ) {
            if ( $line =~ m/^\s*#endif/ ) {
                if ( $nesting_index == 0 ){
                    $SIP_RUN = 0;
                    next;
                }
                else {
                    $nesting_index--;
                }
            }
            if ( $line =~ m/^\s*#if(def)?\s+/ ) {
                $nesting_index++;
            }

            # if there is an else at this level, code will be ignored i.e. not SIP_RUN
            if ( $line =~ m/^\s*#else/ && $nesting_index == 0) {
                while(!eof $header) {
                    $line = readline $header;
                    if ( $line =~ m/^\s*#if(def)?\s+/ ) {
                        $nesting_index++;
                    }
                    elsif ( $line =~ m/^\s*#endif/ ) {
                        if ( $nesting_index == 0 ) {
                            $SIP_RUN = 0;
                            last;
                        }
                        else {
                            $nesting_index--;
                        }
                    }
                }
                next;
            }
        }
        elsif ( $line =~ m/^\s*#ifndef SIP_RUN/) {
            # code is ignored here
            while(!eof $header) {
                $line = readline $header;
                if ( $line =~ m/^\s*#if(def)?\s+/ ) {
                    $nesting_index++;
                }
                elsif ( $line =~ m/^\s*#else/ && $nesting_index == 0 ) {
                    # code here will be printed out
                    if ($PRIVATE_SECTION == 1) {
                        print $private_section_line;
                    }
                    $SIP_RUN = 1;
                    last;
                }
                elsif ( $line =~ m/^\s*#endif/ ) {
                    if ( $nesting_index == 0 ) {
                        $SIP_RUN = 0;
                        last;
                    }
                    else {
                        $nesting_index--;
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
    if ( $HEADER_CODE && $SIP_RUN == 0 ) {
        $HEADER_CODE = 0;
        print "%End\n";
    }

    # Skip forward declarations
    if ($line =~ m/^\s*class \w+;$/) {
        next;
    }
    # Skip Q_OBJECT, Q_PROPERTY, Q_ENUM, Q_GADGET
    if ($line =~ m/^\s*Q_(OBJECT|ENUMS|PROPERTY|GADGET|DECLARE_METATYPE).*?$/) {
        next;
    }

    # SIP_SKIP
    if ( $line =~ m/SIP_SKIP/ ) {
      $line =~ s/^(\s*)(\w.*)$/$1\/\/ $2\n/;
      print $line;
      next;
    }

    # Detect comment block
    if ($line =~ m/\s*\/\*\*/) {
        $comment = '';
        while(!eof $header) {
            $line = readline $header;
            $line =~ m/\s*\*?(.*?)(\/)?$/;
            $comment .= "$1\n";
            if ( $line =~ m/\*\/$/ ) {
                last;
            }
        }
        next;
    }

    # Private members (exclude SIP_RUN)
    if ( $line =~ m/^\s*private( slots)?:.*$/ ) {
        $PRIVATE_SECTION = 1;
        $private_section_line = $line;
        next;
    }
    if ( $PRIVATE_SECTION == 1 ) {
        if ( $SIP_RUN == 0) {
            if ( $line =~ m/^\s*(public|protected)( slots)?:.*$/ || $line =~ m/^\s*\};.*$/) {
                $PRIVATE_SECTION = 0;
            }
            else {
                next;
            }
        }
    }

    # save comments and do not print them, except in SIP_RUN
    if ( $SIP_RUN == 0 ) {
        if ( $line =~ m/^\s*\/\// ) {
            $line =~ s/^\s*\/\/\!*\s*(.*)$/$1/;
            $comment = $line;
            next;
        }
    }

    # class declaration started
    if ( $line =~ m/^(\s*class)\s*([A-Z]+_EXPORT)(\s+\w+)(\s*\:.*)?$/ ) {
        $line = "$1$3";
        # Inheritance
        if ($4) {
            my $m;
            $m = $4;
            $m =~ s/public //g;
            $m =~ s/\s*private \w+,?//;
            $line .= $m;
        }

        $line .= "\n{\n%Docstring\n$comment\n%End\n";
        $line .= "\n%TypeHeaderCode\n#include \"" . basename($headerfile) . "\"\n";

        print $line;

        my $skip;
        # Skip opening curly bracket, we already added that above
        $skip = readline $header;
        $skip =~ m/^\s*{\s$/ || die "Unexpected content on line $line";

        $HEADER_CODE = 1;
        next;
    }

    # Enum declaration
    if ( $line =~ m/^\s*enum\s+\w+.*?$/ ) {
        print $line;
        $line = readline $header;
        $line =~ m/^\s*\{\s*$/ || die 'Unexpected content: enum should be followed by {';
        print $line;
        while(!eof $header) {
            $line = readline $header;
            if ($line =~ m/\}/) {
                last;
            }
            $line =~ s/(\s*\w+)(\s*=\s*\w+.*?)?(,?).*$/$1$3/;
            print $line;
        }
    }

    # remove keywords
    do {no warnings 'uninitialized';
        $line =~ s/\s*override( SIP_FACTORY)?;/$1;/;
        $line =~ s/^(\s*)?(const )?(virtual |static )?inline /$1$2$3/;
        $line =~ s/\bnullptr\b/0/g;

        # remove function bodies
        if ( $line =~  m/^(\s*)?(const )?(virtual |static )?((\w+(<.*?>)?\s+(\*|&)?)?(\w+|operator.)\(.*?(\(.*\))*.*\)( const)?)\s*(\{.*\})?(?!;)$/ ) {
            my $newline = "$1$2$3$4;\n";
            if ($line !~ m/\{.*?\}$/) {
                $line = readline $header;
                if ( $line =~ m/^\s*\{\s*$/ ) {
                    while(!eof $header) {
                        $line = readline $header;
                        if ( $line =~ m/\}\s*$/ ) {
                            last;
                        }
                    }
                }
            }
            $line = $newline;
        }
    };

    # deleted functions
    if ( $line =~  m/^(\s*)?(const )?(virtual |static )?((\w+(<.*?>)?\s+(\*|&)?)?(\w+|operator.)\(.*?(\(.*\))*.*\)( const)?)\s*= delete;$/ ) {
      $line =~ s/^/\/\//;
    }

    $line =~ s/SIP_FACTORY/\/Factory\//;
    $line =~ s/SIP_OUT/\/Out\//g;
    $line =~ s/SIP_INOUT/\/In,Out\//g;
    $line =~ s/SIP_TRANSFER/\/Transfer\//g;
    $line =~ s/SIP_PYNAME\(\s*(\w+)\s*\)/\/PyName=$1\//;
    $line =~ s/SIP_KEEPREFERENCE\((\w+)\)/\/KeepReference\//;
    $line =~ s/SIP_TRANSFERTHIS\((\w+)\)/\/TransferThis\//;
    $line =~ s/SIP_TRANSFERBACK\((\w+)\)/\/TransferBack\//;

    # fix astyle placing space after % character
    $line =~ s/\s*% (MappedType|TypeHeaderCode|ConvertFromTypeCode|ConvertToTypeCode|MethodCode|End)/%$1/;
    $line =~ s/\/\s+GetWrapper\s+\//\/GetWrapper\//;

    print $line;
}
