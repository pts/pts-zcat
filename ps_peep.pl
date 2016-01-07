#!/usr/local/bin/perl -w
# ps_peep.pl -- a simplistic PostScript-subset peephole optimizer
# Reads code (.psr or .psr file) from STDIN, writes to STDOUT
use integer;
use strict;

$_=join"",<STDIN>; # read STDIN
s/% PUSHF/P/g; # keep PUSHF for further processing
s/\bexch % exit/ E/g; # keep PUSHF for further processing
s/%.*//g; # remove comments
s/\s+/ /g; # remove extra whitespace
s/ *([{}]) */ $1 /g; # put spaces around curly brackets
# $_=" $_ "; # don't need this because code is always inside curly brackets

# Dat: "1-34"=~/\b34/ #: true
# Dat: "134"=~/\b34/ #: false

s/ (-?\d+) P( [^PE}]*? )E\b/$2$1/g;
y/P//d;
s/E/exch/g;

#
# Sat Nov 10 16:50:34 CET 2001
#
# I really wanted to optimise `{42} {137} ifelse' to avoid two arrays, but I
# couldn't. Basicly because the code
#
#	true /pop load def 0 1 1000000 {pop 42 137 true load exec pop} for
#
# is 2/3 times as fast as
#
#	0 1 1000000 {pop true {42} {137} ifelse pop} for
#


my $N=0;
while (1) {
  my $LN=0;
  $LN+=s/ (-?\d+) (-?\d+) exch\b/ $2 $1/g;
  $LN+=s/ (-?\d+) (-?\d+) add\b/" ".($1+$2)/ge;
  $LN+=s/ (-?\d+) (-?\d+) sub\b/" ".($1-$2)/ge;
  $LN+=s@ (-?\d+) neg\b@" ".(-1*$1)@ge;
  $LN+=s@ (-?\d+) pop\b@ @g; # :-)) 65->100 peepholes at Sun Nov 11 20:49:07 CET 2001
  # $LN+=s@ (-?\d+) not\b@" ".(-1*$1)@ge;
  $LN+=s@ (-?\d+) (-?\d+) idiv\b@" ".($1/$2)@ge; # :-(
  $LN+=s/ (-?\d+) (-?\d+) and\b/" ".($1&$2)/ge; # :-(
  $LN+=s/ (-?\d+) (-?\d+) bitshift\b/" ".($2>=0?$1<<$2:$1>>-$2)/ge; # :-(
  $LN+=s/ (-?\d+) (-?\d+) eq\b/" ".($1==$2?"true":"false")/ge; # :-(
  $LN+=s/ (-?\d+) (-?\d+) ne\b/" ".($1!=$2?"true":"false")/ge; # :-(
  $LN+=s/ (-?\d+) (-?\d+) lt\b/" ".($1<$2?"true":"false")/ge; # :-(
  last if $LN==0;
  $N+=$LN;
}
while (1) { # this should be done in a separate step to give `1 neg' -> `-1' a chance
  my $LN=0;
  $LN+=s/ -1 emudiv\b/ bload/g; # load from vm.ps's Buf 
  # print STDERR /bload/?"bload\n":"!bload\n";
  $LN+=s/ emudiv\b/ bdef 0/g; # store to vm.ps's Buf 
  # print STDERR /bload/?"ebload\n":"!ebload\n";
  last if $LN==0;
  $N+=$LN;
}
print STDERR "$N peepholes.\n";
# die "emudiv" if /emudiv/;
# die unless /bload/;

my %H=qw(bload L bdef D xload l xdef d bitshift b ifelse i add a sub s eq e
  ne n pop p dup u lt t exch c and y currentdict C);
my $HR=join'|',keys%H;
s/\b($HR)\b/$H{$1}/go if @ARGV;

# s/ (-?\d+) (gt|ge|lt|le) / XXX /g;
# s/ neg sub / XXX /g;
# s/ neg add / XXX /g;

s/  +/ /g;
s/ *([{}]) */$1/g; # remove extra spaces around curly brackets
# y/}/\n/;

print $_, "\n";

