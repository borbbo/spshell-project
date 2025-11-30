/* commands.c - 개별 명령어 구현 (팀원 담당) */
#include "spshell.h"

// --- [팀원 이시연] ---
// [ls]: 디렉토리 목록 출력
void do_ls(int argc, char *argv[])
{
    char *path = "."; // 인자가 없으면 현재 디렉토리
    if (argc > 1)
        path = argv[1];

    DIR *d = opendir(path);
    if (d == NULL)
    {
        perror("ls error");
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        // 숨김 파일(.)로 시작하는 건 건너뛰기
        if (dir->d_name[0] != '.')
        {
            printf("%s  ", dir->d_name);
        }
    }
    printf("\n");
    closedir(d);
}

// [pwd]: 현재 경로 출력
void do_pwd(int argc, char *argv[])
{
    char buf[1024];
    if (getcwd(buf, 1024) != NULL)
    {
        printf("%s\n", buf);
    }
    else
    {
        perror("pwd error");
    }
}

// [mkdir]: 디렉토리 생성
void do_mkdir(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: mkdir directory_name\n");
        return;
    }
    // 0755 권한: rwxr-xr-x
    if (mkdir(argv[1], 0755) < 0)
    {
        perror("mkdir error");
    }
}

// [rmdir]: 빈 디렉토리 삭제
void do_rmdir(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: rmdir directory_name\n");
        return;
    }
    if (rmdir(argv[1]) < 0)
    {
        perror("rmdir error");
    }
}

// [ln]: 링크 생성 (하드링크 및 -s 옵션 심볼릭 링크 지원)
void do_ln(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: ln [-s] target link_name\n");
        return;
    }

    // 심볼릭 링크 (ln -s 원본 링크명)
    if (strcmp(argv[1], "-s") == 0)
    {
        if (argc < 4)
        {
            fprintf(stderr, "usage: ln -s target link_name\n");
            return;
        }
        if (symlink(argv[2], argv[3]) < 0)
        {
            perror("ln -s error");
        }
    }
    // 하드 링크 (ln 원본 링크명)
    else
    {
        if (link(argv[1], argv[2]) < 0)
        {
            perror("ln error");
        }
    }
}

// --- [팀원 송녕경] ---
// [cp]: 파일 복사 (read/write 시스템 콜 사용)
void do_cp(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: cp source target\n");
        return;
    }

    int rfd = open(argv[1], O_RDONLY);
    if (rfd < 0)
    {
        perror("cp source error");
        return;
    }

    // 대상 파일: 쓰기전용 | 생성 | 내용삭제, 권한 0644
    int wfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (wfd < 0)
    {
        perror("cp target error");
        close(rfd);
        return;
    }

    char buf[4096];
    ssize_t n;
    while ((n = read(rfd, buf, sizeof(buf))) > 0)
    {
        if (write(wfd, buf, n) != n)
        {
            perror("cp write error");
            break;
        }
    }

    close(rfd);
    close(wfd);
}

// [rm]: 파일 삭제
void do_rm(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: rm filename\n");
        return;
    }
    if (unlink(argv[1]) < 0)
    {
        perror("rm error");
    }
}

// [mv]: 파일 이동 (이름 변경)
void do_mv(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: mv old_name new_name\n");
        return;
    }
    if (rename(argv[1], argv[2]) < 0)
    {
        perror("mv error");
    }
}

// [cat]: 파일 내용 출력
void do_cat(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: cat filename\n");
        return;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("cat error");
        return;
    }

    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        write(STDOUT_FILENO, buf, n); // 표준 출력(화면)으로 쓰기
    }

    printf("\n"); // 보기 좋게 마지막 줄바꿈
    close(fd);
}

// [grep]: 파일에서 특정 문자열 검색
void do_grep(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: grep pattern filename\n");
        return;
    }

    // grep은 줄 단위 처리가 편해야 하므로 fopen 사용 (C 표준 라이브러리 허용 범위)
    FILE *fp = fopen(argv[2], "r");
    if (fp == NULL)
    {
        perror("grep error");
        return;
    }

    char line[MAX_CMD_LEN];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        // strstr: 문자열 안에 문자열이 있는지 찾음
        if (strstr(line, argv[1]) != NULL)
        {
            printf("%s", line);
        }
    }
    fclose(fp);
}
