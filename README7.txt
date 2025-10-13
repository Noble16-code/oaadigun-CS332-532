CS 332/532
Lab 7 

---------------------------------------------
Program Name: lab7.c

Purpose:
This program reads commands from a text file (one command per line) and uses the fork(), execvp(), and wait() system calls to create and manage child processes.
It records the start and end time of each command execution and writes the results into a log file named output.log.

---------------------------------------------
How the Program Works:
1. The program takes one command-line argument — the name of the input file that contains the commands to run.
   Example input file:
       uname -a
       /sbin/ifconfig
       /home/user/hw1 500

2. For each line in the file:
   - The parent records the start time.
   - It calls fork() to create a child process.
   - The child process uses execvp() to execute the command.
   - The parent process waits for the child to finish.
   - The parent records the end time and writes a line to output.log in this format:
         <command>    <start_time>    <end_time>

3. Blank lines and comment lines (starting with #) are ignored.

---------------------------------------------
How to Compile:
    gcc -Wall -O -o lab7 lab7.c.c


---------------------------------------------
How to Run:
    ./lab7 input.txt


