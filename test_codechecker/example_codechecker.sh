CodeChecker check -j4 --saargs extra_compilation_options --tidyargs extra_tidy_compile_flags --skip skipfile -n check_v7xx_pocxxdll_$(date +'%Y%m%d_%H%M') -b "./all.sh -n -p4 pcsdll"
