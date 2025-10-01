/***********************************************
 * SYSHALT LABORATORIES
 * Software: rmtmp - temporary deleter for Linux
 * File: trash.c
 * Author: João Vítor (aka @df42Karazal)
 * SPDX-License-Identifier: MIT
***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

int mv_to_trash(const char *home, const char *fpath2) {
    char trash_path[1024];
    snprintf(trash_path, sizeof(trash_path), "%s/.local/share/Trash/files/", home);
    mkdir(trash_path, 0700);

    char *fname2 = strrchr(fpath2, '/');
    fname2 = fname2 ? fname2 + 1 : (char*) fpath2;

    char target[1024];
    snprintf(target, sizeof(target), "%s%s", trash_path, fname2);
    if (rename(fpath2, target) != 0) {
        perror("rename");
        return -1;
    }

    char info_dir[1024];
    snprintf(info_dir, sizeof(info_dir), "%s/.local/share/Trash/info/", home);
    mkdir(info_dir, 0700);

    char info_file[1024];
    snprintf(info_file, sizeof(info_file), "%s%s.trashinfo", info_dir, fname2);
    FILE *f = fopen(info_file, "w");
    if (!f) return -1;
    time_t now = time(NULL);
    char timestr[32];
    strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S", localtime(&now));
    fprintf(f, "[Trash Info]\nPath=%s\nDeletionDate=%s\n", fpath2, timestr);
    fclose(f);

    return 0;
}

void mv_from_trash(const char *home, const char *who, const char *fpath3) {
	char trash_file[1024];
	snprintf(trash_file, sizeof(trash_file), "%s/.local/share/Trash/files/%s", home, who);

	char target_file[1024];
	snprint(target_file, sizeof(target_file), "%s/%s", fpath3, who);
	
	FILE *src = fopen(trash_file, "rb");
	if (!src) {
		perror("fopen (src)");
		return -1;
	}
	
	FILE *dst = fopen(target_file, "wb");
	if (!dst) {
		perror("fopen (dst)");
		fclose(src);
		return -1;
	}

	char buf[4096];
	size_t n;
	while ((n=fread(buf, 1, sizeof(buf), src))>0) {
		fwrite(buf, 1, n, dst);
	}

	fclose(src);
	fclose(dst);
	
	remove(trash_file);
	
	char trashinfo_file[1024];
	snprintf(trashinfo_file, sizeof(trashinfo_file), "%s/.local/share/Trash/info/%s.trashinfo", home, who);
	remove(trashinfo_file);
	printf("Restored %s to %s\n", who, fpath3);

}

int main(int argc, char *argv[]) {
    char *restore_path = NULL;
    char *remove_file = NULL;
    char *restore_who = NULL;

    const char *de = getenv("XDG_CURRENT_DESKTOP");
    const char *home = getenv("HOME");

    if (!home) {
        fprintf(stderr, "HOME environment variable not set.\n");
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--restore_path=", 15) == 0) {
            restore_path = argv[i] + 15;
        } else if (strcmp(argv[i], "--remove") == 0 && i + 1 < argc) {
            remove_file = argv[i + 1];
            i++; // next arg is filename
        } else if (strcmp(argv[i], "--who") == 0 && i + 1 < argc) {
            restore_who = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--help") == 0) {
            FILE *fptr = fopen("help.txt", "r");
            if (!fptr) {
                printf("Help file didn't load correctly.\n");
                return -1;
            }
            char c;
            while ((c = fgetc(fptr)) != EOF) {
                printf("%c", c);
            }
            fclose(fptr);
            return 0;
        }
    }

    if (!remove_file && !restore_who) {
        printf("No valid arguments given. Use --help for flag list.\n");
        return 1;
    }

    if (remove_file) {
        if (mv_to_trash(home, remove_file) == 0) {
            printf("Moved %s to Trash.\n", remove_file);
        } else {
            printf("Failed to move %s to Trash.\n", remove_file);
        }
    }

    if (restore_path && restore_who) {
	mv_from_trash(restore_path, restore_who);
    }

    return 0;
}
