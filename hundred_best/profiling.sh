# rename -f "s/\.sym//g" *.sym


export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH


rm -f speed.profile
rm -f callgrindSpeed.log

#PROGRAM="./nn_mainExe"
PROGRAM="./runTest"

# in case the connection to the source files is not done correctly although the debug symbols are in the shared libraries:
# possible reason: shared object is unloaded before the program terminates and valgrind will discard the debug information
# solution: avoid calling "dlclose" on these shared objects. Possibly just do --dump-before=dlclose
#valgrind  --tool=callgrind --callgrind-out-file=nn.profile --dump-before=dlclose --instr-atstart=no --fn-skip="Worker::Procedure*" --fn-skip="Kernel::DoMessage*" --collect-systime=y

#valgrind  --tool=callgrind --callgrind-out-file=speed.profile --collect-jumps=yes --simulate-cache=yes --dump-instr=yes --collect-systime=yes --collect-atstart=no --instr-atstart=no $PROGRAM # | tee callgrindNN.log

valgrind  --tool=callgrind --callgrind-out-file=speed.profile --collect-jumps=yes --simulate-cache=yes --dump-instr=yes --collect-systime=yes --collect-atstart=yes --instr-atstart=yes $PROGRAM # | tee callgrindNN.log


#valgrind  --tool=callgrind --callgrind-out-file=nn.profile --dump-before=dlclose --instr-atstart=no $PROGRAM # | tee callgrindNN.log

# --instr-atsatrt=no  --> turns off instrumentation on startup
# turn on instrumentation during execution with "callgrind_control -i on"

# for machine code
# --dump-instr=yes --collect-jumps=yes

echo callgrind_annotate --inclusive=yes --tree=both --auto=yes speed.profile 
callgrind_annotate --inclusive=yes --tree=both --auto=yes --context=15 speed.profile 
echo "to see the profile do \"kcachegrind speed.profile\""
kcachegrind speed.profile



