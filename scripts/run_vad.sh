#!/bin/bash

# Be sure that this file has execution permissions:
# Use the nautilus explorer or chmod +x run_vad.sh


DB=/home/albert/Documents/uni/3/3B/PAV/lab/P2/db.v4
CMD=bin/vad  #write here the name and path of your program
for n in 1 2 3 4 5 6 7 8 9; do
    for j in 1 2 3 4 5 6 7 8 9; do
        for k in 1500 1550 1600 1650 1700 1750 1800 1850 1900 1950 2000; do
            for l in 1 2 3 4 5 6 7 8 9 10; do
                for m in 1 2 3 4 5 6 7 8 9 10; do
                    for filewav in $DB/*/*wav; do
                    #    echo
                        #echo "**************** $filewav ****************"
                        if [[ ! -f $filewav ]]; then 
                            echo "Wav file not found: $filewav" >&2
                            exit 1
                        fi

                        filevad=${filewav/.wav/.vad}
                        $CMD -i $filewav -o $filevad -a $n -b $j -c $k -d $l -e $m || exit 1
                    # Alternatively, uncomment to create output wave files
                    #    filewavOut=${filewav/.wav/.vad.wav}
                    #    $CMD $filewav $filevad $filewavOut || exit 1
                    done
                    echo "alpha1 = " $n " alpha2 = " $j " zeros = " $k " fSilence = " $l " fVoice = " $m 
                    scripts/vad_evaluation.pl $DB/*/*lab
                done
            done
        done
    done
done



exit 0
