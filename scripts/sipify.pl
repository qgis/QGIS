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

# read arguments
my $debug = 0;
die("usage: $0 [-debug] headerfile\n") unless GetOptions ("debug" => \$debug) && @ARGV == 1;
my $headerfile = $ARGV[0];

# read file
open(my $handle, "<", $headerfile) || die "Couldn't open '".$headerfile."' for reading because: ".$!;
chomp(my @lines = <$handle>);
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
my @EXPORTED = (0);
my $MULTILINE_DEFINITION = 0;

my $comment = '';
my $global_ifdef_nesting_index = 0;
my @global_bracket_nesting_index = (0);
my $private_section_line = '';
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

# write some code in front of line to know where the output comes from
$debug == 0 or push @output, "CODE SIP_RUN MultiLine\n";
sub dbg
{
    my $msg = '';
    $debug == 0 or $msg = sprintf("%d %-4s %-1d %-1d   ", $line_idx, $_[0], $SIP_RUN, $MULTILINE_DEFINITION);
    return $msg;
}
sub dbg_info
{
  if ($debug == 1){
    push @output, $_[0]."\n";
    print $line_idx." ".@ACCESS." ".$SIP_RUN." ".$MULTILINE_DEFINITION." ".$_[0]."\n";
  }
}

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
    if ( $line =~ m/\\see (.*)/ ) {
        return ".. seealso:: $1\n";
    }
    if ( $line =~ m/[\\@]note (.*)/ ) {
        return ".. note::\n\n   $1\n";
    }
    if ( $line =~ m/[\\@]brief (.*)/ ) {
        return " $1\n";
    }
    return "$line\n";
}

sub detect_and_remove_following_body_or_initializerlist {
    # https://regex101.com/r/ZaP3tC/6
    do {no warnings 'uninitialized';
        if ( $line =~  m/^(\s*)?((?:(?:explicit|static|const|unsigned|virtual)\s+)*)(([\w:]+(<.*?>)?\s+[*&]?)?(~?\w+|(\w+::)?operator.{1,2})\s*\(([\w=()\/ ,&*<>."-]|::)*\)( (?:const|SIP_[A-Z_]*?))*)\s*((\s*[:,]\s+\w+\(.*\))*\s*\{.*\};?|(?!;))(\s*\/\/.*)?$/
             || $line =~ m/SIP_SKIP\s*(?!;)\s*(\/\/.*)?$/
             || $line =~ m/^\s*class.*SIP_SKIP/ ){
            dbg_info("remove constructor definition, function bodies, member initializing list");
            my $newline = "$1$2$3;";
            remove_following_body_or_initializerlist() unless $line =~ m/{.*}(\s*SIP_\w+)*\s*(\/\/.*)?$/;
            $line = $newline;
        }
    };
}

sub remove_following_body_or_initializerlist {
    do {no warnings 'uninitialized';
        dbg_info("remove constructor definition, function bodies, member initializing list");
        $line = $lines[$line_idx];
        $line_idx++;
        while ( $line =~ m/^\s*[:,]\s+([\w<>]|::)+\(.*?\)/){
          dbg_info("  member initializing list");
          $line = $lines[$line_idx];
          $line_idx++;
        }
        if ( $line =~ m/^\s*\{/ ){
            my $nesting_index = 0;
            while ($line_idx < $line_count){
                dbg_info("  remove body");
                $nesting_index += $line =~ tr/\{//;
                $nesting_index -= $line =~ tr/\}//;
                if ($nesting_index == 0){
                    last;
                }
                $line = $lines[$line_idx];
                $line_idx++;
            }
        }
    };
}

sub fix_annotations(){
  # printed annotations
  $line =~ s/\bSIP_ABSTRACT\b/\/Abstract\//;
  $line =~ s/\bSIP_ARRAY\b/\/Array\//g;
  $line =~ s/\bSIP_ARRAYSIZE\b/\/ArraySize\//g;
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

  # combine multiple annotations
  # https://regex101.com/r/uvCt4M/3
  do {no warnings 'uninitialized';
      $line =~ s/\/(\w+(=\w+)?)\/\s*\/(\w+(=\w+)?)\//\/$1,$3\//;
      (! $3) or dbg_info("combine multiple annotations -- works only for 2");
  };

  # unprinted annotations
  $line =~ s/(\w+)(\<(?>[^<>]|(?2))*\>)?\s+SIP_PYTYPE\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/$3/g;
  $line =~ s/=\s+[^=]*?\s+SIP_PYARGDEFAULT\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/= $1/g;
  # remove argument
  if ($line =~ m/SIP_PYARGREMOVE/){
      dbg_info("remove arg");
      if ( $MULTILINE_DEFINITION == 1 ){
          my $prev_line = pop(@output) =~ s/\n$//r;
          # update multi line status
          my $parenthesis_balance = 0;
          $parenthesis_balance += $prev_line =~ tr/\(//;
          $parenthesis_balance -= $prev_line =~ tr/\)//;
          if ($parenthesis_balance == 1){
             $MULTILINE_DEFINITION = 0;
          }
          # concat with above line to bring previous commas
          $line =~ s/^\s+//;
          $line = "$prev_line $line\n";
      }
      # see https://regex101.com/r/5iNptO/4
      $line =~ s/(?<coma>, +)?(const )?(\w+)(\<(?>[^<>]|(?4))*\>)?\s+[\w&*]+\s+SIP_PYARGREMOVE( = [^()]*(\(\s*(?:[^()]++|(?6))*\s*\))?)?(?(<coma>)|,?)//g;
  }
  $line =~ s/SIP_FORCE//;
}

# detect a comment block, return 1 if found
# if STRICT comment block shall begin at beginning of line (no code in front)
sub detect_comment_block{
    my %args = ( strict_mode => STRICT, @_ );
    # dbg_info("detect comment strict:" . $args{strict_mode} );
    if ( $line =~ m/^\s*\/\*/ || $args{strict_mode} == UNSTRICT && $line =~ m/\/\*/ ){
        dbg_info("found comment block");
        do {no warnings 'uninitialized';
            $comment = processDoxygenLine( $line =~ s/^\s*\/\*(\*)?(.*?)\n?$/$2/r );
        };
        $comment =~ s/^\s*$//;
        #$comment =~ s/^(\s*\n)*(.+)/$2/;
        while ($line !~ m/\*\/\s*(\/\/.*?)?$/){
            $line = $lines[$line_idx];
            $line_idx++;
            $comment .= processDoxygenLine( $line =~ s/\s*\*?(.*?)(\/)?\n?$/$1/r );
        }
        $comment =~ s/\n+$//;
        #push @output, dbg("XXX").$comment;
        return 1;
    }
    return 0;
}


# main loop
while ($line_idx < $line_count){
    $line = $lines[$line_idx];
    $line_idx++;
    my $actual_class = '';
    $actual_class = $CLASSNAME[$#CLASSNAME] unless $#CLASSNAME < 0;
    $debug == 0 or print sprintf('L:%d DEP:%d ACC:%d BRC:%d SIP:%d MLT:%d CLSS: %d/', $line_idx, $#ACCESS, $ACCESS[$#ACCESS], $global_bracket_nesting_index[$#global_bracket_nesting_index], $SIP_RUN, $MULTILINE_DEFINITION, $#CLASSNAME).$actual_class." :: ".$line."\n";

    if ($line =~ m/^\s*SIP_FEATURE\( (\w+) \)(.*)$/){
        push @output, dbg("SF1")."%Feature $1$2\n";
        next;
    }
    if ($line =~ m/^\s*SIP_IF_FEATURE\( (\!?\w+) \)(.*)$/){
        push @output, dbg("SF2")."%If ($1)$2\n";
        next;
    }
    if ($line =~ m/^\s*SIP_CONVERT_TO_SUBCLASS_CODE(.*)$/){
        push @output, dbg("SCS")."%ConvertToSubClassCode$1\n";
        next;
    }

    if ($line =~ m/^\s*SIP_END(.*)$/){
        push @output, dbg("SEN")."%End$1\n";
        next;
    }

    # Skip preprocessor stuff
    if ($line =~ m/^\s*#/){

        # skip #if 0 blocks
        if ( $line =~ m/^\s*#if (0|defined\(Q_OS_WIN\))/){
          dbg_info("skipping #if $1 block");
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
            if ($ACCESS[$#ACCESS] == PRIVATE){
                dbg_info("writing private content");
                push @output, dbg("PRV1").$private_section_line."\n";
            }
            next;
        }
        if ( $SIP_RUN == 1 ){
            if ( $line =~ m/^\s*#endif/ ){
                if ( $global_ifdef_nesting_index == 0 ){
                    $SIP_RUN = 0;
                    next;
                }
                else {
                    $global_ifdef_nesting_index--;
                }
            }
            if ( $line =~ m/^\s*#if(def)?\s+/ ){
                $global_ifdef_nesting_index++;
            }

            # if there is an else at this level, code will be ignored i.e. not SIP_RUN
            if ( $line =~ m/^\s*#else/ && $global_ifdef_nesting_index == 0){
                while ($line_idx < $line_count){
                    $line = $lines[$line_idx];
                    $line_idx++;
                    if ( $line =~ m/^\s*#if(def)?\s+/ ){
                        $global_ifdef_nesting_index++;
                    }
                    elsif ( $line =~ m/^\s*#endif/ ){
                        if ( $global_ifdef_nesting_index == 0 ){
                            $comment = '';
                            $SIP_RUN = 0;
                            last;
                        }
                        else {
                            $global_ifdef_nesting_index--;
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
                    $global_ifdef_nesting_index++;
                }
                elsif ( $line =~ m/^\s*#else/ && $global_ifdef_nesting_index == 0 ){
                    # code here will be printed out
                    if ($ACCESS[$#ACCESS] == PRIVATE){
                        dbg_info("writing private content");
                        push @output, dbg("PRV2").$private_section_line."\n";
                    }
                    $SIP_RUN = 1;
                    last;
                }
                elsif ( $line =~ m/^\s*#endif/ ){
                    if ( $global_ifdef_nesting_index == 0 ){
                        $comment = '';
                        $SIP_RUN = 0;
                        last;
                    }
                    else {
                        $global_ifdef_nesting_index--;
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
        push @output, dbg("HCE")."%End\n";
    }

    # Skip forward declarations
    if ($line =~ m/^\s*(class|struct) \w+;\s*(\/\/.*)?$/){
        dbg_info('skipping forward declaration');
        next;
    }
    # Skip Q_OBJECT, Q_PROPERTY, Q_ENUM, Q_GADGET
    if ($line =~ m/^\s*Q_(OBJECT|ENUMS|PROPERTY|GADGET|DECLARE_METATYPE|DECLARE_TYPEINFO|DECL_DEPRECATED).*?$/){
        next;
    }

    # SIP_SKIP
    if ( $line =~ m/SIP_SKIP/ ){
        dbg_info('SIP SKIP!');
        $comment = '';
        # if multiline definition, remove previous lines
        if ( $MULTILINE_DEFINITION == 1){
            dbg_info('SIP_SKIP with MultiLine');
            my $opening_line = '';
            while ( $opening_line !~ m/^[^()]*\(([^()]*\([^()]*\)[^()]*)*[^()]*$/){
                $opening_line = pop(@output);
                $#output >= 0 or die 'could not reach opening definition';
            }
        dbg_info("removed multiline definition of SIP_SKIP method");
        $MULTILINE_DEFINITION = 0;
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

    if ( $line =~ m/^\s*struct(\s+\w+_EXPORT)?\s+\w+$/ ) {
        dbg_info("  going to struct => public");
        push @CLASSNAME, $CLASSNAME[$#CLASSNAME]; # fake new class since struct has considered similarly
        push @ACCESS, PUBLIC;
        push @EXPORTED, $EXPORTED[-1];
        push @global_bracket_nesting_index, 0;
    }

    # class declaration started
    # https://regex101.com/r/6FWntP/5
    if ( $line =~ m/^(\s*class)\s+([A-Z]+_EXPORT\s+)?(\w+)(\s*\:\s*(public|private)\s+\w+(<([\w]|::)+>)?(::\w+(<\w+>)?)*(,\s*(public|private)\s+\w+(<([\w]|::)+>)?(::\w+(<\w+>)?)*)*)?(?<annot>\s*SIP_.*)?$/ ){
        dbg_info("class definition started");
        push @ACCESS, PUBLIC;
        push @EXPORTED, 0;
        push @global_bracket_nesting_index, 0;
        my @template_inheritance_template = ();
        my @template_inheritance_class = ();
        do {no warnings 'uninitialized';
            push @CLASSNAME, $3;
            dbg_info("class: ".$CLASSNAME[$#CLASSNAME]);
            if ($line =~ m/\b[A-Z]+_EXPORT\b/ || $#CLASSNAME != 0 || $lines[$line_idx-2] =~ m/^\s*template</){
                # class should be exported except those not at top level or template classes
                # if class is not exported, then its methods should be (checked whenever leaving out the class)
                $EXPORTED[-1]++;
            }
        };
        $line = "$1 $3";
        # Inheritance
        if ($4){
            my $m = $4;
            $m =~ s/public //g;
            $m =~ s/[,:]?\s*private \w+(::\w+)?//g;
            # detect template based inheritance
            while ($m =~ /[,:]\s+((?!QList)\w+)<((\w|::)+)>/g){
                dbg_info("template class");
                push @template_inheritance_template, $1;
                push @template_inheritance_class, $2;
            }
            $m =~ s/(\b(?!QList)\w+)<((?:\w|::)+)>/$1${2}Base/g; # use the typeded as template inheritance
            $m =~ s/(\w+)<((?:\w|::)+)>//g; # remove remaining templates
            $m =~ s/([:,])\s*,/$1/g;
            $m =~ s/(\s*[:,])?\s*$//;
            $line .= $m;
        }
        if (defined $+{annot})
        {
            $line .= "$+{annot}";
            fix_annotations();
        }

        $line .= "\n{\n";
        if ( $comment !~ m/^\s*$/ ){
            $line .= "%Docstring\n$comment\n%End\n";
        }
        $line .= "\n%TypeHeaderCode\n#include \"" . basename($headerfile) . "\"";
        # for template based inheritance, add a typedef to define the base type
        # add it to the class and to the TypeHeaderCode
        # also include the template header
        # see https://www.riverbankcomputing.com/pipermail/pyqt/2015-May/035893.html
        while (@template_inheritance_template) {
            my $tpl = pop @template_inheritance_template;
            my $cls = pop @template_inheritance_class;
            my $tpl_header = lc $tpl . ".h";
            if (exists $SIP_CONFIG->{class_headerfile}->{$tpl}){
                $tpl_header = $SIP_CONFIG->{class_headerfile}->{$tpl};
            }
            $line = "\ntypedef $tpl<$cls> ${tpl}${cls}Base;\n\n$line";
            $line .= "\n#include \"" . $tpl_header . "\"";
            $line .= "\ntypedef $tpl<$cls> ${tpl}${cls}Base;";
        }
        if ( PRIVATE ~~ @ACCESS && $#ACCESS != 0){
            # do not write anything in PRIVATE context and not top level
            dbg_info("skipping class in private context");
            next;
        }
        $ACCESS[$#ACCESS] = PRIVATE; # private by default

        push @output, dbg("CLS")."$line\n";

        # Skip opening curly bracket, incrementing hereunder
        my $skip = $lines[$line_idx];
        $line_idx++;
        $skip =~ m/^\s*{\s*$/ || die "Unexpected content on line $skip";
        $global_bracket_nesting_index[$#global_bracket_nesting_index]++;

        $comment = '';
        $HEADER_CODE = 1;
        $ACCESS[$#ACCESS] = PRIVATE;
        next;
    }

    # bracket balance in class/struct tree
    if ($SIP_RUN == 0){
        my $bracket_balance = 0;
        $bracket_balance += $line =~ tr/\{//;
        $bracket_balance -= $line =~ tr/\}//;
        if ($bracket_balance != 0){
            $global_bracket_nesting_index[$#global_bracket_nesting_index] += $bracket_balance;
            if ($global_bracket_nesting_index[$#global_bracket_nesting_index] == 0){
                dbg_info(" going up in class/struct tree");
                if ($#ACCESS > 0){
                    pop(@global_bracket_nesting_index);
                    pop(@ACCESS);
                    die "Class $CLASSNAME[$#CLASSNAME] in $headerfile at line $line_idx should be exported with appropriate [LIB]_EXPORT macro. If this should not be available in python, wrap it in a `#ifndef SIP_RUN` block."
                        if $EXPORTED[-1] == 0;
                    pop @EXPORTED;
                }
                pop(@CLASSNAME);
                if ($#ACCESS == 0){
                    dbg_info("reached top level");
                    # top level should stasy public
                    dbg_info
                    $ACCESS[$#ACCESS] = PUBLIC;
                }
                $comment = '';
                $return_type = '';
                $private_section_line = '';
            }
            dbg_info("new bracket balance: @global_bracket_nesting_index");
        }
    }

    # Private members (exclude SIP_RUN)
    if ( $line =~ m/^\s*private( slots)?:/ ){
        $ACCESS[$#ACCESS] = PRIVATE;
        $private_section_line = $line;
        $comment = '';
        dbg_info("going private");
        next;
    }
    elsif ( $line =~ m/^\s*(public( slots)?|signals):.*$/ ){
        dbg_info("going public");
        $ACCESS[$#ACCESS] = PUBLIC;
        $comment = '';
    }
    elsif ( $line =~ m/^\s*(protected)( slots)?:.*$/ ){
        dbg_info("going protected");
        $ACCESS[$#ACCESS] = PROTECTED;
        $comment = '';
    }
    elsif ( $ACCESS[$#ACCESS] == PRIVATE && $line =~ m/SIP_FORCE/){
        dbg_info("private with SIP_FORCE");
        push @output, dbg("PRV3").$private_section_line."\n";
    }
    elsif ( PRIVATE ~~ @ACCESS && $SIP_RUN == 0 ) {
        $comment = '';
        next;
    }
    # Skip operators
    if ( $line =~ m/operator(=|<<|>>|->)\s*\(/ ){
        dbg_info("skip operator");
        detect_and_remove_following_body_or_initializerlist();
        next;
    }

    # save comments and do not print them, except in SIP_RUN
    if ( $SIP_RUN == 0 ){
        if ( $line =~ m/^\s*\/\// ){
            if ($line =~ m/^\s*\/\/\!\s*(.*?)\n?$/){
                $comment = processDoxygenLine( $1 );
                $comment =~ s/\n+$//;
            }
            elsif ($lines[$line_idx-1] !~ m/\*\/.*/) {
                $comment = '';
            }
            next;
        }
    }

    # Enum declaration
    if ( $line =~ m/^\s*enum\s+\w+.*?$/ ){
        push @output, dbg("ENU1")."$line\n";
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
            push @output, dbg("ENU2")."$line\n";
            while ($line_idx < $line_count){
                $line = $lines[$line_idx];
                $line_idx++;
                if (detect_comment_block()){
                    next;
                }
                if ($line =~ m/\};/){
                    last;
                }
                my $enum_val = $line =~ s/(\s*\w+)(\s*=\s*[\w\s\d<|]+.*?)?(,?).*$/$1$3/r;
                push @output, dbg("ENU3")."$enum_val\n";
                detect_comment_block(strict_mode => UNSTRICT);
            }
            push @output, dbg("ENU4")."$line\n";
            # enums don't have Docstring apparently
            $comment = '';
            next;
        }
    }

    # skip non-method member declaration in non-public sections
    # https://regex101.com/r/gUBZUk/9
    if ( $SIP_RUN != 1 &&
         $ACCESS[$#ACCESS] != PUBLIC &&
         $line =~ m/^\s*(?:template<\w+>\s+)?(?:(const|mutable|static|friend|unsigned)\s+)*\w+(::\w+)?(<([\w<> *&,()]|::)+>)?(,?\s+\*?\w+( = (-?\d+(\.\d+)?|\w+(\([^()]+\))?)|\[\d+\])?)+;/){
        dbg_info("skip non-method member declaration in non-public sections");
        next;
    }

    # remove static const value assignment
    # https://regex101.com/r/DyWkgn/4
    $line !~ m/^\s*const static \w+/ or die "const static should be written static const in $CLASSNAME[$#CLASSNAME] at line $line_idx";
    $line =~ s/^(\s*static const \w+(?:<(?:[\w()<>, ]|::)+>)? \w+) = .*([|;])\s*(\/\/.*)?$/$1;/;
    if ( defined $2 && $2 =~ m/\|/ ){
        dbg_info("multiline const static assignment");
        my $skip = '';
        while ( $skip !~ m/;\s*(\/\/.*?)?$/ ){
            $skip = $lines[$line_idx];
            $line_idx++;
        }
    }

    # remove struct member assignment
    if ( $SIP_RUN != 1 && $ACCESS[$#ACCESS] == PUBLIC && $line =~ m/^(\s*\w+[\w<> *&:,]* \*?\w+) = \w+(\([^()]+\))?;/ ){
        dbg_info("remove struct member assignment");
        $line = "$1;";
    }

    # catch Q_DECLARE_FLAGS
    if ( $line =~ m/^(\s*)Q_DECLARE_FLAGS\(\s*(.*?)\s*,\s*(.*?)\s*\)\s*$/ ){
        my $actual_class = $CLASSNAME[$#CLASSNAME];
        dbg_info("Declare flags: $actual_class");
        $line = "$1typedef QFlags<$actual_class::$3> $2;\n";
        $qflag_hash{"$actual_class::$2"} = "$actual_class::$3";
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
                dbg_info("handle multiline definition to add virtual keyword on opening line");
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

        # keyword fixes
        $line =~ s/^(\s*template<)(?:class|typename) (\w+>)(.*)$/$1$2$3/;
        $line =~ s/\s*\boverride\b//;
        $line =~ s/^(\s*)?(const )?(virtual |static )?inline /$1$2$3/;
        $line =~ s/\bnullptr\b/0/g;
        $line =~ s/\s*=\s*default\b//g;

        if( $line =~ /\w+_EXPORT/ ) {
                $EXPORTED[-1]++;
                $line =~ s/\b\w+_EXPORT\s+//g;
        }

        # remove constructor definition, function bodies, member initializing list
        $SIP_RUN == 1 or detect_and_remove_following_body_or_initializerlist();

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

    fix_annotations();

    # fix astyle placing space after % character
    $line =~ s/\s*% (MappedType|Type(Header)?Code|Module(Header)?Code|Convert(From|To)TypeCode|MethodCode|End)/%$1/;
    $line =~ s/\/\s+GetWrapper\s+\//\/GetWrapper\//;

    push @output, dbg("NOR")."$line\n";

    # multiline definition (parenthesis left open)
    if ( $MULTILINE_DEFINITION == 1 ){
        dbg_info("on multiline");
        # https://regex101.com/r/DN01iM/2
        if ( $line =~ m/^([^()]+(\((?:[^()]++|(?1))*\)))*[^()]*\)[^()]*$/){
            $MULTILINE_DEFINITION = 0;
            dbg_info("ending multiline");
            # remove potential following body
            if ( $SIP_RUN == 0 && $line !~ m/(\{.*\}|;)\s*(\/\/.*)?$/ ){
                dbg_info("remove following body of multiline def");
                my $last_line = $line;
                remove_following_body_or_initializerlist();
                # add missing semi column
                my $dummy = pop(@output);
                push @output, dbg("MLT")."$last_line;\n";
            }
        }
        else
        {
            next;
        }
    }
    elsif ( $line =~ m/^[^()]+\([^()]*([^()]*\([^()]*\)[^()]*)*[^)]*$/ ){
      dbg_info("Mulitline detected");
      $MULTILINE_DEFINITION = 1;
      next;
    }

    # write comment
    if ( $line =~ m/^\s*$/ )
    {
        $is_override = 0;
        next;
    }
    if ( $line =~ m/^\s*template<.*>/ ){
        # do not comment now for templates, wait for class definition
        next;
    }
    elsif ( $line =~ m/\/\// ||
            $line =~ m/\s*typedef / ||
            $line =~ m/\s*struct / ||
            $line =~ m/operator\[\]\(/ ||
            $line =~ m/operator==/ ||
            ($line =~ m/operator[!+-=*\/\[\]]{1,2}/ && $#ACCESS == 0) ||  # apparently global operators cannot be documented
            $line =~ m/^\s*%\w+(.*)?$/ ){
        dbg_info('skipping comment');
        $comment = '';
        $return_type = '';
    }
    elsif ( $comment !~ m/^\s*$/ || $return_type ne ''){
        if ( $is_override == 1 && $comment =~ m/^\s*$/ ){
            # overridden method with no new docs - so don't create a Docstring and use
            # parent class Docstring
        }
        else {
            dbg_info('writing comment');
            push @output, dbg("CM1")."%Docstring\n";
            if ( $comment !~ m/^\s*$/ ){
                push @output, dbg("CM2")."$comment\n";
            }
            if ($return_type ne '' ){
                push @output, dbg("CM3")." :rtype: $return_type\n";
            }
            push @output, dbg("CM4")."%End\n";
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
