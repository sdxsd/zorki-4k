#!/usr/bin/env bash

function print_result () {
    if [ "$1" == "Hello World!" ]
    then
        echo "[$2 $3]: Success!"
    else
        echo "[$2 $3]: Failure!"
    fi
}

function test_normal_files_with_text () {
    testdir="testfiles/normal-files-with-text"
    filecount=5
    name="Normal Files With Text"

    mkdir -p $testdir
    rm $testdir/*.infectable
    for i in {1..5};
    do
        touch -f "$testdir/file$i.infectable"
        echo "This is a normal file, with NO viruses or SHELLCODE I swear!" > "$testdir/files$i.infectable"
    done
    ./infector $testdir shellcode &> /dev/null
    result=$(./loader $testdir)
    print_result "$result" "$filecount" "$name"
}

function test_large_random_files () {
    testdir="testfiles/large-random-files"
    filecount=3
    name="Large Random Files"

    mkdir -p $testdir
    rm $testdir/*.infectable
    for i in {1..3};
    do
        touch -f "$testdir/file$i.infectable"
        dd if=/dev/urandom of="$testdir/files$i.infectable" count=102400 &> /dev/null
    done
    ./infector $testdir shellcode &> /dev/null
    result=$(./loader $testdir)
    print_result "$result" "$filecount" "$name"
}

function test_normal_empty_files () {
    testdir="testfiles/normal-empty-files"
    filecount=10
    name="Normal Empty Files"

    mkdir -p $testdir
    rm $testdir/*.infectable
    for i in {0..10};
    do
        touch -f "$testdir/file$i.infectable"
    done
    ./infector $testdir shellcode &> /dev/null
    result=$(./loader $testdir)
    print_result "$result" "$filecount" "$name"
}

function test_all () {
    tests=(
        test_normal_empty_files
        test_large_random_files
        test_normal_files_with_text
    )
    results=()
    for test in ${tests[@]};
    do
        $test
    done
}

test_all
