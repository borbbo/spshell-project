/* * commands.c
 * * 설명: spshell에서 사용하는 내부 명령어들의 실제 구현부가 담긴 파일입니다.
 * 시스템 콜(System Call)을 직접 사용하여 리눅스 명령어와 유사하게 동작하도록 구현하였습니다.
 * * 구현된 명령어:
 * - ls (기본 및 -l 옵션 지원)
 * - pwd, mkdir, rmdir, ln
 * - cp, rm, mv, cat, grep
 * - history, help
 */

#include "spshell.h"


// --- 전역 변수 정의 ---
char history_buffer[MAX_HISTORY][MAX_CMD_LEN];
int history_count = 0;


// --- 내부 헬퍼 함수들 (ls -l 구현을 위한 도구) ---

// 파일의 모드(st_mode) 비트 정보를 "drwxr-xr-x" 형태의 문자열로 변환
void mode_to_str(mode_t mode, char *str) {
    strcpy(str, "----------"); // 초기화

    // 1. 파일 타입 확인
    if (S_ISDIR(mode)) str[0] = 'd';       // 디렉토리
    else if (S_ISCHR(mode)) str[0] = 'c';  // 문자 장치
    else if (S_ISBLK(mode)) str[0] = 'b';  // 블록 장치
    else if (S_ISFIFO(mode)) str[0] = 'p'; // 파이프
    else if (S_ISLNK(mode)) str[0] = 'l';  // 심볼릭 링크

    // 2. 소유자(User) 권한
    if (mode & S_IRUSR) str[1] = 'r';
    if (mode & S_IWUSR) str[2] = 'w';
    if (mode & S_IXUSR) str[3] = 'x';

    // 3. 그룹(Group) 권한
    if (mode & S_IRGRP) str[4] = 'r';
    if (mode & S_IWGRP) str[5] = 'w';
    if (mode & S_IXGRP) str[6] = 'x';

    // 4. 기타(Other) 권한
    if (mode & S_IROTH) str[7] = 'r';
    if (mode & S_IWOTH) str[8] = 'w';
    if (mode & S_IXOTH) str[9] = 'x';
}


 // ls -l 명령어를 위해 파일의 상세 정보를 출력
void print_file_info(char *path, char *filename) {
    struct stat info;
    char full_path[MAX_CMD_LEN];
    char mode_str[11];
    struct passwd *pw;
    struct group *gr;
    char time_buf[64];

    // 전체 경로 생성
    sprintf(full_path, "%s/%s", path, filename);

    // 파일 정보 읽기 (lstat: 심볼릭 링크 자체의 정보를 읽음)
    if (lstat(full_path, &info) < 0) {
        perror("lstat error");
        return;
    }

    // 1. 권한 문자열 변환
    mode_to_str(info.st_mode, mode_str);

    // 2. 소유자 이름 가져오기
    pw = getpwuid(info.st_uid);
    
    // 3. 그룹 이름 가져오기
    gr = getgrgid(info.st_gid);

    // 4. 시간 포맷팅 (예: Nov 30 18:00)
    struct tm *t = localtime(&info.st_mtime);
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", t);

    // 5. 출력 (권한 링크수 소유자 그룹 크기 시간 이름)
    printf("%s %lu %s %s %5ld %s %s\n",
           mode_str,
           (unsigned long)info.st_nlink,
           (pw != NULL) ? pw->pw_name : "unknown",
           (gr != NULL) ? gr->gr_name : "unknown",
           (long)info.st_size,
           time_buf,
           filename);
}


// --- History 기능 구현 ---


// 사용자가 입력한 명령어를 메모리에 저장
void add_history(char *cmd) {
    if (strlen(cmd) == 0) return; // 빈 명령어는 저장 안 함

    // 버퍼에 복사
    strncpy(history_buffer[history_count % MAX_HISTORY], cmd, MAX_CMD_LEN);
    history_count++;
}


// 'history' 명령어 입력 시 저장된 목록을 출력합니다.
void print_history(void) {
    int start = 0;
    int i;
    int count = 0;

    // 히스토리가 MAX보다 많으면 순환 버퍼 처리
    if (history_count > MAX_HISTORY) {
        start = history_count - MAX_HISTORY;
    }

    printf("--- Command History ---\n");
    for (i = start; i < history_count; i++) 
    {
        // 인덱스 계산
        int idx = i % MAX_HISTORY;
        printf("%5d  %s\n", i + 1, history_buffer[idx]);
        count++;
    }
    printf("-----------------------\n");
}


// --- Help 기능 구현 ---


void print_help(void) {
    printf("\n");
    printf("===========================================\n");
    printf("        System Programming Shell (spshell) \n");
    printf("===========================================\n");
    printf("Usage:\n");
    printf("  command [arguments] [options]\n\n");
    printf("Built-in Commands:\n");
    printf("  ls [-l]    : List directory contents (supports details)\n");
    printf("  cd [dir]   : Change the current directory\n");
    printf("  pwd        : Print working directory\n");
    printf("  mkdir [dir]: Create a new directory\n");
    printf("  rmdir [dir]: Remove an empty directory\n");
    printf("  cp [A] [B] : Copy file A to B\n");
    printf("  mv [A] [B] : Move/Rename file A to B\n");
    printf("  rm [file]  : Remove a file\n");
    printf("  cat [file] : Print file content\n");
    printf("  grep [S][F]: Search string S in file F\n");
    printf("  history    : Show command history\n");
    printf("  help       : Show this help message\n");
    printf("  exit       : Exit the shell\n");
    printf("===========================================\n\n");
}

// --- [이시연] ---

// [ls]: 디렉토리 목록 출력 (-l 옵션 지원)
void do_ls(int argc, char *argv[]) {
    char *path = ".";
    int is_long_format = 0;

    // 옵션 파싱 (-l 옵션 확인)
    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], "-l") == 0) {
            is_long_format = 1;
        } else if (argv[i][0] != '-') {
            path = argv[i]; // 경로 지정
        }
    }

    DIR *d = opendir(path);
    if (d == NULL) 
    {
        perror("ls: cannot open directory");
        return;
    }

    printf("[Directory: %s]\n", path);
    
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) 
    {
        // 숨김 파일 제외
        if (dir->d_name[0] == '.') continue;

        if (is_long_format) {
            // 상세 정보 출력 모드
            print_file_info(path, dir->d_name);
        } else {
            // 일반 출력 모드
            printf("%s  ", dir->d_name);
        }
    }
    if (!is_long_format) printf("\n");
    
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
        fprintf(stderr, "Error: mkdir requires a directory name.\n");
        fprintf(stderr, "usage: mkdir [directory_name]\n");
        return;
    }
    // 0755 권한: rwxr-xr-x
    if (mkdir(argv[1], 0755) < 0)
    {
        perror("mkdir error");
    } else {
        printf("Directory '%s' created successfully.\n", argv[1]);
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

// --- [송녕경] ---

// [cp]: 파일 복사 (read/write 시스템 콜 사용)
void do_cp(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Error: missing file operand.\n");
        fprintf(stderr, "usage: cp [source] [destination]\n");
        return;
    }

    int rfd = open(argv[1], O_RDONLY);
    if (rfd < 0)
    {
        perror("cp: cannot open source file");
        return;
    }

    // 대상 파일: 쓰기전용 | 생성 | 내용삭제, 권한 0644
    int wfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (wfd < 0)
    {
        perror("cp: cannot create destination file");
        close(rfd);
        return;
    }

    char buf[4096];
    ssize_t n;
    while ((n = read(rfd, buf, sizeof(buf))) > 0)
    {
        if (write(wfd, buf, n) != n)
        {
            perror("cp: write error");
            break;
        }
    }

    printf("File copied: %s -> %s\n", argv[1], argv[2]);
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
        fprintf(stderr, "usage: mv [old_name] [new_name]\n");
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
        fprintf(stderr, "usage: cat [filename]\n");
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
void do_grep(int argc, char *argv[]) {
    // 인자가 부족하면 사용법 출력 (적어도 패턴은 있어야 함)
    if (argc < 2) {
        fprintf(stderr, "usage: grep pattern [filename]\n");
        return;
    }

    FILE *fp;
    
    // 파일 이름(argv[2])이 있으면 파일을 열고, 없으면 표준 입력(stdin) 사용
    if (argc >= 3) {
        fp = fopen(argv[2], "r");
        if (fp == NULL) {
            perror("grep error");
            return;
        }
    } else {
        // 인자가 2개뿐이면(grep pattern) 파이프나 리다이렉션으로 들어온 입력을 읽음
        fp = stdin; 
    }

    char line[MAX_CMD_LEN];
    int line_num = 1;

    // 파일(또는 stdin)에서 한 줄씩 읽어서 검색
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, argv[1]) != NULL) {
            // 파일 이름을 모를 때(stdin) 내용만 출력
            printf("%s", line); 
        }
        line_num++;
    }

    // stdin이 아닐 때만 파일 닫기 (stdin은 닫으면 안 됨)
    if (fp != stdin) {
        fclose(fp);
    }
}