#!/bin/bash

#CodeChecker check -j4 --saargs extra_compilation_options --tidyargs extra_tidy_compile_flags -b "make clean;make" -o "~/codechecker_results"
#CodeChecker store "~/codechecker_results" --url http://127.0.0.1:8001/Default -n pathtest_test_$(date +'%Y%m%d_%H%M')

logfile="/home/peters/codechecker_logs/test_codechecker.log"
resultpath="/home/peters/codechecker_results"
tag=$(date +'%Y%m%d_%H%M')
name=test_$tag
cmd="make"
debug="debug_analyzer"
product_url="http://127.0.0.1:8001/Default"
echo $name

make clean

cmd_log="CodeChecker log -o $logfile -b \"$cmd\" --verbose $debug"
#cmd_analyze="CodeChecker analyze $logfile -j4 --saargs extra_compilation_options --tidyargs extra_tidy_compile_flags -o $resultpath -n $name --verbose $debug --clean"
cmd_analyze="CodeChecker analyze $logfile -j4 -o $resultpath -n $name --verbose $debug"
cmd_store="CodeChecker store $resultpath --url $product_url -n $name"    # --tag $tag

echo "--- codechecker log command ---"
echo $cmd_log
echo "--- codechecker analyze command ---"
echo $cmd_analyze
echo "--- codechecker store command ---"
echo $cmd_store

eval $cmd_log
eval $cmd_analyze
eval $cmd_store



