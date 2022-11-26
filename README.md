# Rainfall_Simulation

To help you compare your program's output against a validation output, there is a check.sh script, which can be used as follows:

./check.py [dimension] [validation file] [output file]

 

Below are the command lines to run the sample and measurements test case inputs to match the validation outputs:

./rainfall [P] 10 0.25 4 sample_4x4.in

./rainfall [P] 20 0.5 16 sample_16x16.in

./rainfall [P] 20 0.5 32 sample_32x32.in

./rainfall [P] 30 0.25 128 sample_128x128.in

./rainfall [P] 30 0.75 512 sample_512x512.in

./rainfall [P] 35 0.5 2048 sample_2048x2048.in

./rainfall [P] 50 0.5 4096 measurement_4096x4096.in