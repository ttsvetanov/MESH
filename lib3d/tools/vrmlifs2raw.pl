#!/usr/bin/perl

# $Id$

#
# Converts an IndexedFaceSet from a VRML file to a raw mesh
#
# Created Fri May  4 21:42:44 CEST 2001 by Diego Santa Cruz
# Copyright (C) 2000 Diego Santa Cruz All rights reserved
#

#
# CVS log:
#
# $Log: vrmlifs2raw.pl,v $
# Revision 1.2  2002/01/18 18:02:36  dsanta
# - Added support for files with multiple IndexedFaceSet nodes.
#
# Revision 1.1  2001/05/07 09:27:22  dsanta
# Initial revision. Working.
#
#

#
# Global variables
#

# The current line in the file
$line = "";
# The array of vertex coordinates
@vtcs = ();
# The array of face vertex indices, including the face terminating -1 index
@faces = ();

#
# Subroutines
#

# Gets the next non-empty line from IN and puts it in $line. All comments are
# stripped.
sub next_line {
  do {
    $line = <IN>;
    $line =~ s/\#.*$//;
  } while (defined($line) && $line =~ /^\s*$/);
}

# Reads a Coordinate node point data. The vertex data is stored in @vtcs
sub read_Coordinate {
  my $l;
  my @coords;

  while (defined ($line) && $line !~ /(^|\s)point(\s|$\)/) { # look for "point"
    next_line();
  }
  $line =~ s/^.*point//; # erase data already read
  while (defined ($line) && $line !~ /\[/) { # look for "["
    ($line =~ /^\s*$/) || die "corrupted data in Coordinate node\n";
    next_line();
  }
  $line =~ s/^.*\[//; # erase data already read
  # Read coordinates until we reach closing bracket ("]")
  while (defined ($line) && $line !~ /\]/) { # look for "]"
    if ($line !~ /^\s*$/) {
      $line =~ tr/,/ /;
      @coords = split(' ',$line);
      push @vtcs, @coords;
    }
    next_line();
  }
  # Read line with closing bracket
  $l = $line;
  $line =~ s/^.*\]//; # erase data already read
  $l =~ s/\].*$//; # strip non-coordinate data
  if ($l !~ /^\s*$/) {
    $l =~ tr/,/ /;
    @coords = split(' ',$l);
    push @vtcs, @coords;
  }
}

# Reads the coordIndex field of an IndexedFaceSet node and stores them
# in the @faces global variable. The -1 face terminating index is also stored.
# If defined $vtx_off is added to each vertex index.
sub read_coordIndex {
  my @indices;
  my $l;
  my $off;
  my $start;
  my $i;

  # Get vertex offset indices
  $off = defined($vtx_off) ? $vtx_off : 0;
  $start = $#faces+1;
  # Look for start of field
  while (defined ($line) && $line !~ /\[/) { # look for "["
    ($line =~ /^\s*$/) || die "corrupted data in coordIndex field\n";
    next_line();
  }
  $line =~ s/^.*\[//; # erase data already read
  # Read indices until we reach closing bracket ("]")
  while (defined ($line) && $line !~ /\]/) { # look for "]"
    if ($line !~ /^\s*$/) {
      $line =~ tr/,/ /;
      @indices = split(' ',$line);
      push @faces, @indices;
    }
    next_line();
  }
  # Read line with closing bracket
  $l = $line;
  $line =~ s/^.*\]//; # erase data already read
  $l =~ s/\].*$//; # strip non-index data
  if ($l !~ /^\s*$/) {
    $l =~ tr/,/ /;
    @indices = split(' ',$l);
    push @faces, @indices;
  }
  if ($faces[$#faces] ne "-1") { # last -1 is not mandatory
    push @faces, "-1";
  }
  # Add vertex index offset
  if ($off != 0) {
    for ($i = $start; $i <= $#faces; $i++) {
      if ($faces[$i] ne "-1") {
	$faces[$i] += $off;
      }
    }
  }
}

#
# Main
#

# Open the input file
if ($#ARGV == 0) {
  open(IN,$ARGV[0]) || die "Could not open input file\n";
}
else {
  print STDERR "Converts an IndexedFaceSet in a VRML file to a raw mesh\n",
    "Multiple IndexedFaceSet nodes are appended one after the other\n",
      "usage: $0 <VRML file>\n";
  exit 1;
}

# Check the VRML header
$header = <IN> || die "Empty file\n";
$header =~ /^\#VRML +V2\.0 +utf8/ || die "Input file is not VRML 2";

# Look for IndexedFaceSet fields
$n_ifs = 0;
next_line();
while (defined($line)) {
  # Look for start of next IndexedFaceSet
  while (defined($line) &&
	 $line !~ /(^|\s)IndexedFaceSet(\s|$\)/ ) { # IndexedFaceSet starts here
    next_line();
  }
  defined($line) || last; # Reached EOF
  $line =~ s/^.*IndexedFaceSet//; # erase data already read
  $n_ifs++;

  # Look for Coordinate or coordIndex in this IFS
  $n_vtcs = $#vtcs+1;
  $n_faces = $#faces+1;
  $vtx_off = $n_vtcs/3;
  while (defined($line) && ($#vtcs == $n_vtcs-1 || $#faces == $n_faces-1)) {
    if ($line =~ /(^|\s)Coordinate(\s|$\)/) {
      $line =~ s/^.*Coordinate//; # erase data already read
      if ($#vtcs == $n_vtcs-1 ) {
	read_Coordinate();
      } else {
	die "Two Coordinate nodes before a coordIndex one\n";
      }
    }
    if ($line =~ /(^|\s)coordIndex(\s|$\)/) {
      $line =~ s/^.*coordIndex//; # erase data already read
      if ($#faces == $n_faces-1) {
	read_coordIndex();
      } else {
	die "Two coordIndex nodes before a Coordinate one\n";
      }
    }
    next_line();
  }
}

# Check number of ifs, vertices and faces
if ($n_ifs == 0) {
  die "No IndexedFaceSet in input file";
}

if (($#vtcs+1)%3 != 0) {
  die "Number of vertex coordinates is not multiple of 3!\n";
}
if ($#vtcs == -1) {
  die "Vertex coordinate data missing\n";
}
if (($#faces+1)%4 != 0) {
  die "Some faces are not triangular\n";
}
if ($#faces == -1) {
  die "Face vertex index data missing\n";
}

# Print output file

# Print raw file header (we only use triangular meshes)
print STDOUT ($#vtcs+1)/3," ",($#faces+1)/4,"\n";

# Print vertex coordinates
for ($i = 0; $i <= $#vtcs; $i+=3) {
  print STDOUT join(' ',($vtcs[$i],$vtcs[$i+1],$vtcs[$i+2])),"\n";
}

# Print face vertex indices, while checking for triangular faces
for ($i = 0; $i <= $#faces; $i+=4) {
  if ($faces[$i+3] ne "-1") {
    die "Face ",$i/4," is not a triangle\n";
  }
  print STDOUT join(' ',($faces[$i],$faces[$i+1],$faces[$i+2])),"\n";
}
