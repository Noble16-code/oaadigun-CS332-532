#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE 4096
#define MAX_ARGS 128

/* Trim leading and trailing whitespace in place */
static void trim_inplace(char *s) {
    if (s == NULL) return;

    /* trim leading */
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') start++;

    if (start != s) memmove(s, start, strlen(start) + 1);

    /* trim trailing */
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[len - 1] = '\0';
        len--;
    }
}

/* Remove trailing newline from ctime string */
static void ctime_no_nl(time_t t, char *outbuf, size_t outbuf_size) {
    char *s = ctime(&t); /* ctime returns a string that ends with '\n' */
    if (s == NULL) {
        strncpy(outbuf, "unknown time", outbuf_size - 1);
        outbuf[outbuf_size - 1] = '\0';
        return;
    }
    /* copy but remove trailing newline if present */
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n') n--;
    if (n >= outbuf_size) n = outbuf_size - 1;
    memcpy(outbuf, s, n);
    outbuf[n] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <commands-file>\n", argv[0]);
        return 1;
    }

    const char *infilename = argv[1];
    FILE *infile = fopen(infilename, "r");
    if (infile == NULL) {
        perror("fopen input file");
        return 1;
    }

    FILE *logfile = fopen("output.log", "a");
    if (logfile == NULL) {
        perror("fopen output.log");
        fclose(infile);
        return 1;
    }

    char line[MAX_LINE];
    unsigned long lineno = 0;

    while (fgets(line, sizeof(line), infile) != NULL) {
        lineno++;

        /* trim the line */
        trim_inplace(line);

        /* skip empty lines and comments */
        if (line[0] == '\0') continue;
        if (line[0] == '#') continue;

        /* Keep a copy of the trimmed command text for logging */
        char cmdtext[MAX_LINE];
        strncpy(cmdtext, line, sizeof(cmdtext) - 1);
        cmdtext[sizeof(cmdtext) - 1] = '\0';


        char *arglist[MAX_ARGS];
        int argcount = 0;

        char *token = strtok(line, " \t");
        while (token != NULL && argcount < (MAX_ARGS - 1)) {
            arglist[argcount++] = token;
            token = strtok(NULL, " \t");
        }
        arglist[argcount] = NULL;

        /* If no tokens found (shouldn't happen because we trimmed), skip */
        if (argcount == 0) continue;

        /* Record start time */
        time_t start_time = time(NULL);

        pid_t pid = fork();
        if (pid < 0) {
            /* fork failed */
            perror("fork");
            /* Log failure with start time and end time same as "fork_failed" text */
            char startstr[64];
            ctime_no_nl(start_time, startstr, sizeof(startstr));
            fprintf(logfile, "%s\t%s\t%s\n", cmdtext, startstr, "fork_failed");
            fflush(logfile);
            continue;
        } else if (pid == 0) {
            /* Child process: execute the command */
            execvp(arglist[0], arglist);
            /* If execvp returns, it failed. Print a message and exit. */
            fprintf(stderr, "execvp failed on line %lu: %s : %s\n", lineno, cmdtext, strerror(errno));
            _exit(127); /* conventional exit code for exec failure */
        } else {
            /* Parent process: wait for child to finish */
            int status;
            pid_t w = waitpid(pid, &status, 0);
            (void)w; 

            /* Record end time */
            time_t end_time = time(NULL);

            /* Format times and write to log:
               <command>\t<start_time>\t<end_time>\n */
            char startstr[64], endstr[64];
            ctime_no_nl(start_time, startstr, sizeof(startstr));
            ctime_no_nl(end_time, endstr, sizeof(endstr));

            fprintf(logfile, "%s\t%s\t%s\n", cmdtext, startstr, endstr);
            fflush(logfile);
        }
    }

    fclose(infile);
    fclose(logfile);

    return 0;
}
