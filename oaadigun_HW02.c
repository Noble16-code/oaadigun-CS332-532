
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <limits.h>

#define INDENT_STR "\t"

/*
Name: Oladotun Adigun
BlazerId: oaadigun
Project #: oaadigun_HW02
To compile: make
*/
int opt_S = 0;            /* print attributes */
long opt_s_size = -1;     /* size filter (-s) */
char *opt_f_pattern = NULL; /* substring pattern for -f */
int opt_f_depth = -1;     /* depth limit for -f */

/* Typedef for filter function pointer */
typedef int (*filter_fn)(const char *path, const struct stat *st, int depth);

/* Forward declarations */
int filter_none(const char *path, const struct stat *st, int depth);
int filter_size(const char *path, const struct stat *st, int depth);
int filter_pattern_depth(const char *path, const struct stat *st, int depth);
int filter_combined(const char *path, const struct stat *st, int depth);

void print_permissions(mode_t mode, char *out) {
    /* rwxrwxrwx */
    out[0] = (mode & S_IRUSR) ? 'r' : '-';
    out[1] = (mode & S_IWUSR) ? 'w' : '-';
    out[2] = (mode & S_IXUSR) ? 'x' : '-';
    out[3] = (mode & S_IRGRP) ? 'r' : '-';
    out[4] = (mode & S_IWGRP) ? 'w' : '-';
    out[5] = (mode & S_IXGRP) ? 'x' : '-';
    out[6] = (mode & S_IROTH) ? 'r' : '-';
    out[7] = (mode & S_IWOTH) ? 'w' : '-';
    out[8] = (mode & S_IXOTH) ? 'x' : '-';
    out[9] = '\0';
}

void format_time(time_t t, char *buf, size_t bufsz) {
    struct tm lt;
    localtime_r(&t, &lt);
    strftime(buf, bufsz, "%Y-%m-%d %H:%M:%S", &lt);
}

/* Decide whether it should print this file according to active filters.
   Directories are always returned true, but file-level filters
   apply to regular files and symlinks. */
int should_print(const char *path, const struct stat *st, int depth) {
    /* If no filters set, always true */
    if (!opt_s_size && !opt_f_pattern) return 1;

    /* Directories: still print (structure), regardless of filters */
    if (S_ISDIR(st->st_mode)) return 1;

    /* If both -s and -f present, require both */
    if (opt_s_size > -1 && opt_f_pattern) {
        /* size and pattern+depth */
        int size_ok = (st->st_size <= opt_s_size);
        int depth_ok = (opt_f_depth < 0) ? 1 : (depth <= opt_f_depth);
        int pat_ok = strstr(path, opt_f_pattern) != NULL;
        return size_ok && pat_ok && depth_ok;
    }

    /* Only -s */
    if (opt_s_size > -1 && !opt_f_pattern) {
        if (S_ISDIR(st->st_mode)) return 1; /* directories printed */
        return st->st_size <= opt_s_size;
    }

    /* Only -f */
    if (opt_f_pattern && opt_s_size < 0) {
        int depth_ok = (opt_f_depth < 0) ? 1 : (depth <= opt_f_depth);
        return (strstr(path, opt_f_pattern) != NULL) && depth_ok;
    }

    return 1;
}

void print_entry(const char *name, const char *fullpath, const struct stat *st, int indent_level) {
    /* indent */
    for (int i = 0; i < indent_level; ++i) printf(INDENT_STR);

    /* base print name */
    if (S_ISLNK(st->st_mode)) {
        /* print link and target */
        char link_target[PATH_MAX+1];
        ssize_t r = readlink(fullpath, link_target, PATH_MAX);
        if (r < 0) {
            printf("%s -> (unreadable symlink)\n", name);
        } else {
            link_target[r] = '\0';
            if (opt_S) {
                /* attributes for link: show lstat size (link length), permissions, atime */
                char perm[10];
                print_permissions(st->st_mode, perm);
                char tbuf[64];
                format_time(st->st_atime, tbuf, sizeof(tbuf));
                printf("%s (-> %s, %lld bytes, %s, %s)\n", name, link_target, (long long)st->st_size, perm, tbuf);
            } else {
                printf("%s (%s)\n", name, link_target);
            }
        }
    } else if (S_ISDIR(st->st_mode)) {
        /* directory name on its own line */
        if (opt_S) {
            char perm[10];
            print_permissions(st->st_mode, perm);
            char tbuf[64];
            format_time(st->st_atime, tbuf, sizeof(tbuf));
            printf("%s (0 bytes, %s, %s)\n", name, perm, tbuf);
        } else {
            printf("%s\n", name);
        }
    } else {
        /* regular file or others */
        if (opt_S) {
            char perm[10];
            print_permissions(st->st_mode, perm);
            char tbuf[64];
            format_time(st->st_atime, tbuf, sizeof(tbuf));
            printf("%s (%lld bytes, %s, %s)\n", name, (long long)st->st_size, perm, tbuf);
        } else {
            printf("%s\n", name);
        }
    }
}

/* Recursive traversal.
   current_depth: depth of this path relative to starting directory (start = 0)
   indent_level: how many tabs to print before entries
*/
void traverse(const char *path, int current_depth, int indent_level) {
    struct stat st;
    if (lstat(path, &st) < 0) {
        fprintf(stderr, "lstat failed on %s: %s\n", path, strerror(errno));
        return;
    }

    /* Extract basename for printing */
    const char *name = path;
    const char *p = strrchr(path, '/');
    if (p) name = p + 1;

    /* For the root starting directory, we want to print its name without indentation */
    if (current_depth == 0) {
        /* Print starting directory name */
        print_entry(name, path, &st, 0);
    }

    if (!S_ISDIR(st.st_mode)) {
        /* If starting path is a file, we've printed it; nothing more to do */
        if (current_depth == 0) return;
    }

    /* If this is a directory, open and iterate its entries */
    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(path);
        if (!d) {
            fprintf(stderr, "opendir failed on %s: %s\n", path, strerror(errno));
            return;
        }

        struct dirent *entry;
        /* We'll collect entries so that directories are printed and traversed with their own indentation
           while files are printed at current indent+1. */

        /* First pass: print files and symlinks and other non-directory entries */
        while ((entry = readdir(d)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char childpath[PATH_MAX+1];
            snprintf(childpath, sizeof(childpath), "%s/%s", path, entry->d_name);

            struct stat childst;
            if (lstat(childpath, &childst) < 0) {
                fprintf(stderr, "lstat failed on %s: %s\n", childpath, strerror(errno));
                continue;
            }

            if (S_ISDIR(childst.st_mode)) {
                /* skip directories in this pass */
                continue;
            }

            /* Check filters for files; directories printed later
               For should_print we pass the entry name to pattern matching, this mirrors expectation. */
            if (should_print(entry->d_name, &childst, current_depth + 1)) {
                print_entry(entry->d_name, childpath, &childst, indent_level + 1);
            }
        }

        /* Second pass: handle subdirectories so their structure prints in order */
        rewinddir(d);
        while ((entry = readdir(d)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char childpath[PATH_MAX+1];
            snprintf(childpath, sizeof(childpath), "%s/%s", path, entry->d_name);

            struct stat childst;
            if (lstat(childpath, &childst) < 0) continue;

            if (S_ISDIR(childst.st_mode)) {
                /* always print directory name with current indentation +1 */
                print_entry(entry->d_name, childpath, &childst, indent_level + 1);

                /* If -f has a depth limit and we've reached it, do not descend further */
                if (opt_f_pattern && opt_f_depth >= 0 && (current_depth + 1) > opt_f_depth) {
                    /* Do not descend */
                } else {
                    traverse(childpath, current_depth + 1, indent_level + 1);
                }
            }
        }

        closedir(d);
    }
}

void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [-S] [-s size] [-f pattern depth] [startdir]\n", prog);
}

int main(int argc, char *argv[]) {
    int opt;
    /* We'll use getopt to parse -S -s and -f */

    /* Custom parsing because -f takes two arguments (pattern and depth) */
    int idx = 1;
    while (idx < argc) {
        if (strcmp(argv[idx], "-S") == 0) {
            opt_S = 1;
            idx++;
        } else if (strcmp(argv[idx], "-s") == 0) {
            if (idx + 1 >= argc) { usage(argv[0]); return 1; }
            opt_s_size = atol(argv[idx+1]);
            idx += 2;
        } else if (strcmp(argv[idx], "-f") == 0) {
            if (idx + 2 >= argc) { usage(argv[0]); return 1; }
            opt_f_pattern = argv[idx+1];
            opt_f_depth = atoi(argv[idx+2]);
            idx += 3;
        } else if (argv[idx][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[idx]);
            usage(argv[0]);
            return 1;
        } else {
            /* positional arg - start directory */
            break;
        }
    }

    const char *startdir = ".";
    if (idx < argc) startdir = argv[idx];

    /* Normalize startdir path (remove trailing slash if present, except root) */
    char real_start[PATH_MAX+1];
    if (realpath(startdir, real_start) == NULL) {
        /* realpath can fail for non-existent or permissions; fall back to given path */
        strncpy(real_start, startdir, sizeof(real_start));
        real_start[sizeof(real_start)-1] = '\0';
    }

    traverse(real_start, 0, 0);

    return 0;
}