#!/bin/bash
# Created: 2005-10-08, Perry Rapp
# Edited:  2007-05-12, Perry Rapp
# Edited:  2015-12-22, Matt Emmerton
#
# Apply new version number to relevant lifelines source files
# Invoke this script like so:
#  sh setversions.sh 3.0.99

##
## SUBROUTINES
##

function showusage {
  echo "Usage: sh `basename $0` X.Y.Z   # to change to specified version number"
  echo "   or: sh `basename $0` restore # to undo version number just applied"
}

function checkparm {
  if [ $1 = "restore" ]
  then
    RESTORE=1
    return
  fi

  # Store argument as $VERSION and check it is valid version
  VERSION=$1
  VPATTERN="^[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$"
  if ! echo $VERSION | grep -q $VPATTERN
  then
    showusage
    exit $E_WRONG_ARGS
  fi
}

function failexit {
  echo $1
  exit 99
}

# Function to apply version to one file
# Argument#1 is filename (eg, "NEWS")
# Arguments#2-? sed patterns, applied one after another
function alterfile {
  [ ! -z "$1" ] || failexit "Missing first argument to alterfile"
  [ ! -z "$2" ] || failexit "Missing second argument to alterfile"
  FILEPATH=$1
  shift
  cp $FILEPATH $FILEPATH.bak || failexit "Error backing up file "$FILEPATH
  # Now apply each remaining argument as sed command
  until [ -z "$1" ]
  do
    # sed doesn't seem to set its return value, so we don't check
    # that it found its match, unfortunately
    sed -e "$1" $FILEPATH > $FILEPATH.tmp
    mv $FILEPATH.tmp $FILEPATH || failexit "mv failed in alterfile: $FILEPATH.tmp"
    shift
  done
}

function altermansrc {
  [ ! -z "$1" ] || failexit "Missing first argument to altermansrc"
  [ ! -z "$2" ] || failexit "Missing first argument to altermansrc"
  MANFILE=$1
  PROGNAME=$2
  SEDPAT="s/\(^\.TH $PROGNAME 1 \"\)[0-9]\{4\}/\1$YEAR/"
  SEDPAT2="s/\(^\.TH $PROGNAME 1 \"$YEAR \)[[:alpha:]]\{3\}/\1$MONTHABBR/"
  SEDPAT3="s/\(^\.TH $PROGNAME 1 \"$YEAR $MONTHABBR\" \"Lifelines \)[0-9][[:alnum:].\-]*/\1$VERSION/"
  alterfile $MANFILE "$SEDPAT" "$SEDPAT2" "$SEDPAT3"
}

# Fix versions in an MS-Windows resource (rc) file
# Argument#1: file to change
function alterwinversions {
  FILEPATH=$1
  SEDPAT1="s/\( FILEVERSION \)[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*/\1$VERSION1,$VERSION2,$VERSION3,$JULIANVERSION/"
  SEDPAT2="s/\( PRODUCTVERSION \)[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*,[0-9][0-9]*/\1$VERSION1,$VERSION2,$VERSION3,$JULIANVERSION/"
  SEDPAT3="s/\([ ]*VALUE \"FileVersion\", \"\)[0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*/\1$VERSION1, $VERSION2, $VERSION3, $JULIANVERSION/"
  SEDPAT4="s/\([ ]*VALUE \"ProductVersion\", \"\)[0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*, [0-9][0-9]*/\1$VERSION1, $VERSION2, $VERSION3, $JULIANVERSION/"
  alterfile $FILEPATH "$SEDPAT1" "$SEDPAT2" "$SEDPAT3" "$SEDPAT4"
}

# Function to restore file from last backup
# Argument#1: file to restore (from .bak version)
function restorefile {
  if [ -e $1.bak ]
  then
    cp $1.bak $1
  fi
}

# Extract and compute new version numbers
function getversion {
  YEAR=`date +%Y`
  MONTHABBR=`date +%b`
  DAY=`date +%d`
  JULIANDAY=`date +%-j`
  JULIANVERSION=$((($YEAR-1990)*1000 + $JULIANDAY))
  if [[ $VERSION =~ ([0-9]+)\.([0-9]+)\.([0-9]+) ]]
  then
    # numeric portion of version is now $BASH_REMATCH
    # eg, for 3.0.18p3-4, $BASH_REMATCH holds 3.0.18
    # and here we pick out the 3, 0, and 18
    VERSION1=${BASH_REMATCH[1]}
    VERSION2=${BASH_REMATCH[2]}
    VERSION3=${BASH_REMATCH[3]}
  else
    failexit "Version ($VERSION) could not be parsed into expected format"
  fi
 echo "Full version=<$VERSION>"
 echo "Numeric version=<$VERSION1.$VERSION2.$VERSION3>"
 echo "Windows version=<$VERSION1.$VERSION2.$VERSION3.$JULIANVERSION>"
}

# Call alterfile with an sed command for each file
function applyversion {
  SEDPAT="s/^\(LifeLines Source Release, Version \)[[:alnum:].\-]*$/\1$VERSION/" 
  alterfile $ROOTDIR/AUTHORS "$SEDPAT"
  alterfile $ROOTDIR/ChangeLog "$SEDPAT"
  alterfile $ROOTDIR/INSTALL "$SEDPAT"
  alterfile $ROOTDIR/NEWS "$SEDPAT"
  alterfile $ROOTDIR/README "$SEDPAT"

  SEDPAT="s/^\(AM_INIT_AUTOMAKE(lifelines, \)[0-9][[:alnum:].\-]*)$/\1$VERSION)/"
  alterfile $ROOTDIR/configure.ac "$SEDPAT"

  SEDPAT="s/\(%define lifelines_version [ ]*\)[0-9][[:alnum:].\-]*$/\1$VERSION/"
  alterfile $ROOTDIR/build/rpm/lifelines.spec "$SEDPAT"
  alterwinversions $ROOTDIR/build/msvc6/btedit/btedit.rc
  alterwinversions $ROOTDIR/build/msvc6/dbverify/dbVerify.rc
  alterwinversions $ROOTDIR/build/msvc6/llexec/llexec.rc
  alterwinversions $ROOTDIR/build/msvc6/llines/llines.rc

  altermansrc $ROOTDIR/docs/man/btedit.1 btedit
  altermansrc $ROOTDIR/docs/man/dbverify.1 dbverify
  altermansrc $ROOTDIR/docs/man/llines.1 llines
  altermansrc $ROOTDIR/docs/man/llexec.1 llexec

  SEDPAT="s/\(<\!ENTITY llversion[[:space:]]*['\"']\)[0-9][[:alnum:].\-]*/\1$VERSION/i"
  alterfile $ROOTDIR/docs/manual/ll-devguide.xml "$SEDPAT"
  alterfile $ROOTDIR/docs/manual/ll-userguide.xml "$SEDPAT"
  alterfile $ROOTDIR/docs/manual/ll-userguide.sv.xml "$SEDPAT"
  alterfile $ROOTDIR/docs/manual/ll-reportmanual.xml "$SEDPAT"
  alterfile $ROOTDIR/docs/manual/ll-reportmanual.sv.xml "$SEDPAT"
}

# Restore, for user to reverse last application
function restore {
  restorefile $ROOTDIR/AUTHORS
  restorefile $ROOTDIR/ChangeLog
  restorefile $ROOTDIR/INSTALL
  restorefile $ROOTDIR/NEWS
  restorefile $ROOTDIR/README
  restorefile $ROOTDIR/configure.ac
  restorefile $ROOTDIR/build/msvc6/btedit/btedit.rc
  restorefile $ROOTDIR/build/msvc6/dbverify/dbVerify.rc
  restorefile $ROOTDIR/build/msvc6/llexec/llexec.rc
  restorefile $ROOTDIR/build/msvc6/llines/llines.rc
  restorefile $ROOTDIR/build/rpm/lifelines.spec
  restorefile $ROOTDIR/docs/man/btedit.1
  restorefile $ROOTDIR/docs/man/dbverify.1
  restorefile $ROOTDIR/docs/man/llines.1
  restorefile $ROOTDIR/docs/man/llexec.1
  restorefile $ROOTDIR/docs/manual/ll-devguide.xml
  restorefile $ROOTDIR/docs/manual/ll-reportmanual.xml
  restorefile $ROOTDIR/docs/manual/ll-reportmanual.sv.xml
  restorefile $ROOTDIR/docs/manual/ll-userguide.xml
  restorefile $ROOTDIR/docs/manual/ll-userguide.sv.xml
}

##
## MAIN PROGRAM
##

# Determine root of repository
if [ ! -f AUTHORS ]
then
  if [ ! -f ../AUTHORS ]
  then
    echo "ERROR: Must be run from either the root of the source tree or the build/ directory!"
    exit 1
  else
    ROOTDIR=..
  fi
else
  ROOTDIR=.
fi

# Check that user passed exactly one parameter
E_WRONG_ARGS=65
if [ $# -ne 1 ] || [ -z "$1" ]
then
  showusage
  exit $E_WRONG_ARGS
fi

# Function to handle parsing argument
# Parse argument (should be a version, or "restore")
# (exits if failure)
checkparm $1

# Compute new version numbers (esp for Windows)
getversion

# Invoke whichever functionality was requested
if [ -z "$RESTORE" ]
then
  applyversion
else
  restore
fi
