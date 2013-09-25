#!/usr/bin/perl
use strict;
use warnings;

if (@ARGV < 3) {
  print("Usage: $0 inhdr_file outhdr_file list of C files\n"); exit(1);
}

my $hdrfile= $ARGV[0]; shift;
my $outfile= $ARGV[0]; shift;
open(my $IN, "<", $hdrfile) || die("Can't open $hdrfile: $!");
open(my $OUT, ">", $outfile) || die("Can't open $outfile: $!");

# Copy the top of the file into the temp file
while (<$IN>) {
  last if (m{Functions exported by the library});
  print($OUT $_);
}
print($OUT $_);
print($OUT "\n");

# Open up the C files, find the function documentation & prototypes,
# and add them to the temp file.
foreach my $cfile (@ARGV) {
  open(my $CIN, "<", $cfile) || die("Can't open $cfile: $!");
  my $incomment=0;
  while (<$CIN>) {
    $incomment=1 if (m{/\*\*});
    print($OUT $_) if ($incomment);
    if ($incomment && m{\*/}) {
      # Get the next line, which is the prototype
      $_= <$CIN>;
      chomp($_);
      print($OUT "$_;\n\n");
      $incomment=0;
    }
  }
  close($CIN);
  print($OUT "\n");
}

# Copy the rest of the file into the temp file
while (<$IN>) {
  last if (m{Functions exported by the library});
  print($OUT $_);
}
close($IN);
close($OUT);
exit(0);
