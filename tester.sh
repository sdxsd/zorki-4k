#!/usr/bin/env bash
#
CLEAR="\033[0m"
RED="\033[0;31m"
GREEN="\033[0;32m"
shellcode_execution_success="Hello World!"
no_shellcode_executed_success=""

function print_result () {
    if [ "$1" == "$5" ]; then
        printf "[$2 $3]: ${GREEN} Success! ${CLEAR}\n"
    elif [ $4 -eq 139 ]; then # Segfault code.
        printf "[$2 $3]: ${RED} Failure! (Segfault) ${CLEAR}\n"
    else
        printf "[$2 $3]: ${RED} Failure! $4 ${CLEAR}\n"
    fi
}

function test_large_random_files_no_shellcode () {
    testdir="testfiles/large-random-files"
    filecount=4
    name="Large Random File No Shellcode"

    mkdir -p $testdir
    rm $testdir/*.infectable
    for ((i=1; i <= filecount; i++)); do
        touch -f "$testdir/file$i.infectable"
        dd if=/dev/urandom of="$testdir/file$i.infectable" count=10240 &> /dev/null
    done
    result=$(./loader $testdir)
    print_result "$result" "$filecount" "$name" "$?" "$no_shellcode_execution_success"

}

function test_normal_files_no_shellcode () {
    testdir="testfiles/normal-files-no-shellcode"
    filecount=8
    name="Normal Files No Shellcode"

    mkdir -p $testdir
    rm $testdir/*.infectable
    for ((i=1; i <= filecount; i++)); do
        touch -f "$testdir/file$i.infectable"
        echo "This is a normal file, with lots of shellcode and evil binary data. I swear!!" > "$testdir/file$i.infectable"
    done
    result=$(./loader $testdir)
    print_result "$result" "$filecount" "$name" "$?" "$no_shellcode_execution_success"
}

function test_normal_files_with_text () {
    testdir="testfiles/normal-files-with-text"
    filecount=5
    name="Normal Files With Text"

    mkdir -p $testdir
    rm $testdir/*.infectable
    for ((i=1; i <= filecount; i++)); do
        touch -f "$testdir/file$i.infectable"
        echo "This is a normal file, with NO viruses or SHELLCODE I swear!" > "$testdir/file$i.infectable"
    done
    ./infector $testdir shellcode &> /dev/null
    result=$(./loader $testdir)
    print_result "$result" "$filecount" "$name" "$?" "$shellcode_execution_success"
}

function test_large_random_files () {
    testdir="testfiles/large-random-files"
    filecount=3
    name="Large Random Files"

    mkdir -p $testdir
    rm $testdir/*.infectable
    for ((i=1; i <= filecount; i++)); do
        touch -f "$testdir/file$i.infectable"
        dd if=/dev/urandom of="$testdir/file$i.infectable" count=10240 &> /dev/null
    done
    ./infector $testdir shellcode &> /dev/null
    result=$(./loader $testdir)
    print_result "$result" "$filecount" "$name" "$?" "$shellcode_execution_success"
}

function test_normal_empty_files () {
    testdir="testfiles/normal-empty-files"
    filecount=10
    name="Normal Empty Files"

    mkdir -p $testdir
    rm $testdir/*.infectable
    for ((i=1; i <= filecount; i++)); do
        touch -f "$testdir/file$i.infectable"
    done
    ./infector $testdir shellcode &> /dev/null
    result=$(./loader $testdir)
    print_result "$result" "$filecount" "$name" "$?" "$shellcode_execution_success"
}

function test_all () {
    tests=(
        test_normal_empty_files
        test_large_random_files
        test_normal_files_with_text
        test_normal_files_no_shellcode
        test_large_random_files_no_shellcode
    )
    for test in ${tests[@]}; do
        $test
    done
}

test_all
