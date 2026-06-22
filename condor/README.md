# condor/

Submit the full pipeline with:

    bash starting_scripts/CL2/ProduceDag.sh
    condor_submit_dag dag/<name>.dag

Edit `executables/CL2/*.sh` to change EOS paths or environment name.
Job logs go to `condor_out/CL2`.
Same for AE, just change CL2 with AE in the path. 