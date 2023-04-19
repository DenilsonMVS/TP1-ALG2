#!/bin/sh

testInput()
{
    ./bin/text_compressor -c $1 -o out.z78
    echo "input: " $1

    inputSize=$(stat -c%s "$1")
    outputSize=$(stat -c%s "out.z78")
    compression=$(echo "scale=4;(1-$outputSize/$inputSize)" | bc)
    compression_percentage=$(echo "scale=2;$compression*100" | bc -l | sed '/\./ s/\.\{0,1\}0\{1,\}$//')

    echo "input size: " $inputSize
    echo "output size: " $outputSize
    echo "compression: " $compression_percentage "%"

    ./bin/text_compressor -x out.z78
    diff out.txt $1 > diff.txt

    if [ -s diff.txt ]; then
        echo "Test" $2 "Fail"
        exit 1
    else
        echo "Test" $2 "OK"
    fi

    echo
}

make clean
make release

echo
testInput "tests/vazio.txt" 1
testInput "tests/nome.txt" 2
testInput "src/compression.hpp" 3
testInput "src/argReader.cpp" 4
testInput "Makefile" 5
testInput "src/argReader.hpp" 6
testInput "runTests.sh" 7
testInput "src/main.cpp" 8
testInput "tests/lorem1.txt" 9
testInput "tests/especificacao.txt" 10
testInput "src/compression.cpp" 11
testInput "tests/lorem2.txt" 12
testInput "tests/lorem3.txt" 13
testInput "bin/text_compressor" 14
testInput "tests/lorem7.txt" 15
testInput "tests/lorem6.txt" 16
testInput "tests/lorem5.txt" 17
testInput "tests/lorem4.txt" 18
testInput "tests/os_lusiadas.txt" 19
testInput "tests/dom_casmurro.txt" 20
testInput "tests/constituicao1988.txt" 21
testInput "tests/lorem8.txt" 22
#testInput "tests/video.mp4" 23

rm out.z78
rm out.txt
rm diff.txt
