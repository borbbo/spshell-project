/* commands.c - 개별 명령어 구현 (팀원 담당) */
#include "spshell.h"

// --- [팀원 이시연] ---
// [ls]: 디렉토리 목록 출력
void do_ls(int argc, char *argv[]) {
    char *path = "."; // 인자가 없으면 현재 디렉토리
    if (argc > 1) path = argv[1];

    DIR *d = opendir(path);
    if (d == NULL) {
        perror("ls error");
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        // 숨김 파일(.)로 시작하는 건 건너뛰기
        if (dir->d_name[0] != '.') {
            printf("%s  ", dir->d_name);
        }
    }
    printf("\n");
    closedir(d);
}

// [pwd]: 현재 경로 출력
void do_pwd(int argc, char *argv[]) {
    char buf[1024];
    if (getcwd(buf, 1024) != NULL) {
        printf("%s\n", buf);
    } else {
        perror("pwd error");
    }
}

// [mkdir]: 디렉토리 생성
void do_mkdir(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: mkdir directory_name\n");
        return;
    }
    // 0755 권한: rwxr-xr-x
    if (mkdir(argv[1], 0755) < 0) {
        perror("mkdir error");
    }
}

// [rmdir]: 빈 디렉토리 삭제
void do_rmdir(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: rmdir directory_name\n");
        return;
    }
    if (rmdir(argv[1]) < 0) {
        perror("rmdir error");
    }
}

// [ln]: 링크 생성 (하드링크 및 -s 옵션 심볼릭 링크 지원)
void do_ln(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: ln [-s] target link_name\n");
        return;
    }

    // 심볼릭 링크 (ln -s 원본 링크명)
    if (strcmp(argv[1], "-s") == 0) {
        if (argc < 4) {
            fprintf(stderr, "usage: ln -s target link_name\n");
            return;
        }
        if (symlink(argv[2], argv[3]) < 0) {
            perror("ln -s error");
        }
    } 
    // 하드 링크 (ln 원본 링크명)
    else {
        if (link(argv[1], argv[2]) < 0) {
            perror("ln error");
        }
    }
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
