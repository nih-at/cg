BEGIN { FS = "k"; } / / { gsub(".*\\\[", "", $1); size += $1; } END { printf "%d\n", size; }
