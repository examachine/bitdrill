#!/usr/bin/perl
$ARGC = @ARGV;
if ( $ARGC < 1 ){
  printf ("transnum.pl: output-table-file [separator] < input-file > output-file\n");
  exit (1);
}
open ( TABLEFILE, ">$ARGV[0]" );
@lines = <STDIN>;
$count = 0;
%numbers = ();

$sep = " ";
if ( $ARGC >= 2 ){ $sep = $ARGV[1]; } 

foreach $trans( @lines ) {
    @eles = split($sep, $trans);
    $all = @eles;
    $c = 0;
    foreach $item( @eles ) {
        $item =~ s/\r//;
        $item =~ s/\n//;
        if (!exists $numbers{$item}) { 
            $numbers{$item} = $count;
            print TABLEFILE "$count $item\n";
            $count++;
        }
        if ( $item ne "" ){
            print "$numbers{$item}";
            $c++;
            if ($c<$all){ print " ";}
        }
    }
    if ( $c > 0 ){ print "\n" }
}
