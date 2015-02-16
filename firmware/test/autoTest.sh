#!/bin/bash

# This script update the current repository, compile the FW and unitary tests, 
# run all unitary tests and publish results on wiki.
# It is designed to be called by a cron task on a test server to act
# as a poor man continuous integration tool


PROG=$(basename $0)
REPO_ADDRESS=https://code.google.com/p/solextronic/
RESULT_PAGE=results.wiki
WIKI_DIR=/home/tbouttevin/perso/solextronic/solextronic.wiki  #DEBUG
SOX_ROOT=/tmp/solextronic
RESULT_BUFFER=$SOX_ROOT/buffer.txt
ALLOW_COMMIT=OK
REV_FILE=rev.dat
RESULT=OK
SCRIPT_DIR=$(pwd)



function title { echo -e "\033[0;1m$@\033[0;0m"; }
function pass { echo -e "\033[1;32m$@\033[0;0m"; }
function warn { echo -e "\033[1;33m$@\033[0;0m"; }
function fail { echo -e "\033[1;31m$@\033[0;0m"; }

function makeCheck {
    title "Compiling '$@'"
    make -s $@ || {
        fail "Error while compiling '$@'"
        exit 2
    }
    pass "Compilation succeed ! ($@)"
}

function usage {
    echo "$PROG  [options]"
    echo "e.g. $PROG -v"
    echo " <-v> verbose mode "
    echo "====================================================="
    [ "$1" ] && exit $1
}


title "STEP 1 : check if there is a new revision"
# check content of revision file
if [ ! -e "$REV_FILE" ]; then
	# no rev file, create a fake one
	echo "toto" > $REV_FILE
fi
git fetch
SERVER_REV=$(git ls-remote -q | grep HEAD | awk '{print $1}')
CURRENT_REV=$(cat $REV_FILE)

if [ $CURRENT_REV == $SERVER_REV ] 
then
    pass "Up to date"
    exit 0
fi

title "STEP 2 : clone the new revision in $SOX_ROOT"
rm -Rf $SOX_ROOT
git clone $REPO_ADDRESS $SOX_ROOT || {
    fail "Erreur pendant le git clone"
    exit 2
}

title "STEP 3 : make FW and test bench, save result and memory occupation"
cd $SOX_ROOT/firmware
echo "" >> $RESULT_BUFFER
echo "" >> $RESULT_BUFFER
echo "== Version $(cat ../version), Git $SERVER_REV, $(date) ==" >> $RESULT_BUFFER
echo "" >> $RESULT_BUFFER
echo "<blockquote><table border=1 cellpadding=5>" >> $RESULT_BUFFER
echo "<tr><th>Test</th><th>Resultat</th><th>Details</th></tr>" >> $RESULT_BUFFER
COMPILE_LOG=$(make sim 2>&1)
COMPILE_RESULT=$?
if [ $COMPILE_RESULT -ne 0 ]
then
    fail "Erreur pendant la compilation"
    RESULT=FAIL
    MESSAGE="Echec de la compilation"
    echo $COMPILE_LOG
    echo '<tr><td>Compilation</td><td><font color="red">FAIL</font></td><td>'$COMPILE_LOG"</td></tr>" >> $RESULT_BUFFER
else
    pass "Compilation OK"
    SIZE_LOG=$(make size 2>&1) 
    echo $SIZE_LOG   
    echo '<tr><td>Compilation</td><td><font color="green">OK</font></td><td></td></tr>' >> $RESULT_BUFFER
    echo "<tr><td>SIZE</td><td>$SIZE_LOG</td><td></td></tr>" >> $RESULT_BUFFER
fi

if [ $RESULT == OK ]
then
    title "STEP 4 : run all tests and parse results"
    cd $SOX_ROOT/firmware/test
    $(./simMain.exe -v -a -r 3 ../solextronic.hex 2>&1 > $WIKI_DIR/testlogs/test_$SERVER_REV.log)
    # analyze log
awk '#!/usr/bin/awk -f
BEGIN {lastTC=0; lastRes=0}
/Verdict/{
    if($3 != lastTC && lastTC != 0){
        cl="black"
        if(lastRes=="PASS") cl="green"
        if(lastRes=="FAIL") cl="red"
        print "<tr><td>" lastTC "</td><td><font color=\""cl"\">" lastRes "</font></td><td></td></tr>";
    }
    lastTC = $3
    lastRes = $4 
}' $WIKI_DIR/testlogs/test_$SERVER_REV.log >> $RESULT_BUFFER
fi

echo "</table></blockquote>" >> $RESULT_BUFFER
echo "" >> $RESULT_BUFFER
if [ $RESULT == OK ]
then 
    echo "[http://wiki.solextronic.googlecode.com/git/testlogs/test_$SERVER_REV.log Log de test]" >> $RESULT_BUFFER
    echo "" >> $RESULT_BUFFER
fi

title "STEP 5 : add result table to wiki page and commit" 
cd $WIKI_DIR
cat $RESULT_BUFFER >> $RESULT_PAGE
if [ $ALLOW_COMMIT == OK ]
then
    if [ $RESULT == OK ]
    then 
        git add testlogs/test_$SERVER_REV.log
        git commit testlogs/test_$SERVER_REV.log $RESULT_PAGE -m "TEST - test auto sur release $SERVER_REV" 
    else
        git commit $RESULT_PAGE -m "TEST - test auto sur release $SERVER_REV" 
    fi
fi

#some cleanup
rm -Rf $SOX_ROOT

