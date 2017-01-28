#!/usr/bin/perl
$ARGC = @ARGV;
if ( $ARGC < 0 ){
  printf ("appendnum.pl:  [separator] < input-file > output-file\n");
  exit (1);
}
@lines = <STDIN>;
$count = 0;
%numbers = ();

$sep = " ";
if ( $ARGC >= 1 ){ $sep = $ARGV[0]; } 

foreach $trans( @lines ) {
    @eles = split($sep, $trans);
    $all = @eles;
    $c = 0;
    foreach $item( @eles ) {
        $item =~ s/\r//;
        $item =~ s/\n//;
        if ( $item ne "" ){
            print "$item.$c" ;
            $c++;
            if ($c<$all){ print $sep; }
        }
    }
    if ( $c>0 ){ print "\n" }
}
