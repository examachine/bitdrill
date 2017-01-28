#!/usr/bin/perl
$ARGC = @ARGV;
if ( $ARGC < 0 ){
  printf ("sortout.pl: [separator] < input-file > output-file\n");
  exit (1);
}
@lines = <STDIN>;
$count = 0;
%numbers = ();
$m=0;

$sep = " ";
if ( $ARGC > 0 ){ $sep = $ARGV[$c]; } 
foreach $trans( @lines ) {
  @eles = split($sep, $trans);
  $eles[$#eles] =~ s/\r//;
  $eles[$#eles] =~ s/\n//;
  @eles = sort { $a <=> $b } (@eles);
  foreach $cell(@eles){
    if ( $cell == 0 ){
      if (index ( $cell, "0") >= 0 ){ $lines2[$m] .= $cell." "; }
    } else { $lines2[$m] .= $cell." "; }
  }
  $m++;
}
@lines2 = sort (@lines2);
for ( $mm=0 ; $mm<$m ; $mm++ ){
  print "$lines2[$mm]\n";
}


