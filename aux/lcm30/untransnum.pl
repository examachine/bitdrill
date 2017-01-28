#!/usr/bin/perl
$ARGC = @ARGV;
if ( $ARGC < 1 ){
      # error routine
  printf ("untransnum.pl: output-table-file < input-file > output-file\n");
  exit (1);
}
    # initialization
open ( TABLEFILE, "<$ARGV[0]" );
@table = <TABLEFILE>;
@lines = <STDIN>;
%numbers = ();

    # read transform-table
foreach $trans( @table ) {
    @eles = split(" ", $trans);
    $numbers{$eles[0]} = $eles[1];
}

    # read transform-file
foreach $trans( @lines ) {
    @eles = split(" ", $trans);
    $all = @eles;
    $c = 0;
    foreach $item( @eles ) {
        if (!exists $numbers{$item}) { 
            print "$item";
        }
        print "$numbers{$item}";
        $c++;
        if ($c<$all){ print " ";}
    }
    print "\n"
}
