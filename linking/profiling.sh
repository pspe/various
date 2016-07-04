# rename -f "s/\.sym//g" *.sym

rm -f proftest.profile
rm -f callgrindProftest.log
valgrind  --tool=callgrind --callgrind-out-file=proftest.profile  ./prog
echo callgrind_annotate --inclusive=yes --tree=both --auto=yes proftest.profile 
callgrind_annotate --inclusive=yes --tree=both --auto=yes proftest.profile 
echo "to see the profile do \"kcachegrind proftest.profile\""
kcachegrind proftest.profile



