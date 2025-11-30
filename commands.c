/* commands.c - 개별 명령어 구현 (팀원 담당) */
#include "spshell.h"

// --- [팀원 이시연] ---
void do_ls(int argc, char *argv[]) {
    printf("[Debug] ls 실행 (아직 구현 안됨)\n");
    // TODO: opendir, readdir, closedir 사용
}

void do_pwd(int argc, char *argv[]) {
    char buf[1024];
    getcwd(buf, 1024);
    printf("%s\n", buf);
}

void do_mkdir(int argc, char *argv[]) {
    printf("[Debug] mkdir 실행\n");
}

void do_rmdir(int argc, char *argv[]) {
    printf("[Debug] rmdir 실행\n");
}

void do_ln(int argc, char *argv[]) {
    printf("[Debug] ln 실행\n");
}

// --- [팀원 송녕경] ---
void do_cp(int argc, char *argv[]) {
    printf("[Debug] cp 실행\n");
    // TODO: open, read, write 사용
}

void do_rm(int argc, char *argv[]) {
    printf("[Debug] rm 실행\n");
}

void do_mv(int argc, char *argv[]) {
    printf("[Debug] mv 실행\n");
}

void do_cat(int argc, char *argv[]) {
    printf("[Debug] cat 실행\n");
}

void do_grep(int argc, char *argv[]) {
    printf("[Debug] grep 실행\n");
}