#!/bin/bash

# Be sure that this file has execution permissions:
# Use the nautilus explorer or chmod +x run_vad.sh


DB=/home/albert/Documents/uni/3/3B/PAV/lab/P2/db.v4
CMD=bin/vad  #write here the name and path of your program

for filewav in $DB/*/*wav; do
#    echo
    echo "**************** $filewav ****************"
    if [[ ! -f $filewav ]]; then 
	    echo "Wav file not found: $filewav" >&2
	    exit 1
    fi

    filevad=${filewav/.wav/.vad}
    filewavOut=${filewav/.wav/_silence.wav}
    $CMD -i $filewav -o $filevad  -w $filewavOut -a 4 -b 1 -c 1900 -d 6 -e 8 || exit 1

# Alternatively, uncomment to create output wave files
#    filewavOut=${filewav/.wav/.vad.wav}
     #$CMD $filewav $filevad $filewavOut || exit 1

done

scripts/vad_evaluation.pl $DB/*/*lab

exit 0