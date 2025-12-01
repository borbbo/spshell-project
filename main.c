/* main.c - spshell의 핵심 엔진 (강보민 구현: 파이프, 리다이렉션, 백그라운드) */
#include "spshell.h"
#include <fcntl.h>

// 함수 선언
void run_command(char *args[]);
void execute_simple_command(char *args[], int is_bg);
void handle_pipe(char *args[], int pipe_idx);

int main()
{
    char cmd[MAX_CMD_LEN];
    char *args[MAX_ARG_LEN];
    int arg_count;

    // 시그널 처리: 쉘 자체가 Ctrl-C에 죽지 않도록 설정 
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    while (1)
    {
        // --- 프롬프트 디자인 (색상 + 경로) ---
        char cwd[1024];

        // 현재 경로(Current Working Directory) 가져오기
        if (getcwd(cwd, sizeof(cwd)) == NULL) 
        {
            strcpy(cwd, "unknown"); // 실패 시 unknown 표시
        }
        
        // ANSI Color Code 설명:
        // \033[1;32m : 밝은 초록색 (spshell 글자)
        // \033[1;34m : 밝은 파란색 (경로)
        // \033[0m    : 색상 초기화 (원래대로)
        
        // 출력 형식: [spshell:/home/bomin/spshell_project]$ 
        printf("[\033[1;32mspshell\033[0m:\033[1;34m%s\033[0m]$ ", cwd);
        
        fflush(stdout); // 화면에 즉시 출력
        // ----------------------------------------------

        if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL)
            break;

        cmd[strcspn(cmd, "\n")] = 0;
        if (strlen(cmd) == 0)
            continue;

        add_history(cmd);

        arg_count = tokenize(cmd, args);
        if (arg_count == 0)
            continue;

        if (strcmp(args[0], "exit") == 0)
        {
            printf("spshell을 종료합니다.\n");
            break;
        }

        if (strcmp(args[0], "cd") == 0)
        {
            if (arg_count < 2)
                chdir(getenv("HOME"));
            else if (chdir(args[1]) < 0)
                perror("cd failed");
            continue;
        }

        run_command(args);
    }
    return 0;
}

int tokenize(char *cmd, char *args[])
{
    int count = 0;
    // 파이프(|) 등의 기호를 확실히 구분하기 위해 공백 필수 사용
    char *token = strtok(cmd, " \t");
    while (token != NULL && count < MAX_ARG_LEN - 1)
    {
        args[count++] = token;
        token = strtok(NULL, " \t");
    }
    args[count] = NULL;
    return count;
}

// 명령어를 분석해서 파이프가 있으면 handle_pipe,
// 없으면 execute_simple_command 호출
void run_command(char *args[])
{
    int i;
    int pipe_idx = -1;

    // 파이프(|)가 있는지 탐색
    for (i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            pipe_idx = i;
            break;
        }
    }

    if (pipe_idx != -1)
    {
        // 파이프가 발견되면 처리 함수로 위임
        handle_pipe(args, pipe_idx);
    }
    else
    {
        // 파이프가 없으면 기존처럼 실행
        // 백그라운드 확인
        int is_bg = 0;
        for (i = 0; args[i] != NULL; i++)
            ; // 끝 찾기
        if (i > 0 && strcmp(args[i - 1], "&") == 0)
        {
            is_bg = 1;
            args[i - 1] = NULL; // & 제거
        }
        execute_simple_command(args, is_bg);
    }
}

// 파이프 처리 로직
void handle_pipe(char *args[], int pipe_idx)
{
    int fd[2]; // 파이프용 파일 디스크립터
    pid_t pid1, pid2;

    // 명령어를 두 개로 분리
    args[pipe_idx] = NULL;             // 파이프 기호 자리에 NULL (왼쪽 명령어 종료)
    char **cmd2 = &args[pipe_idx + 1]; // 오른쪽 명령어 시작 지점

    if (pipe(fd) < 0)
    {
        perror("pipe error");
        return;
    }

    // 첫 번째 자식 (왼쪽 명령어: ls)
    if ((pid1 = fork()) == 0)
    {
        close(STDOUT_FILENO); // 표준 출력 닫기
        dup(fd[1]);           // 파이프의 쓰기 구멍을 표준 출력으로 복사
        close(fd[0]);
        close(fd[1]);

        // 자식 프로세스는 시그널 기본 동작으로 복구 (Ctrl-C 먹히도록)
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        // 왼쪽 명령어도 우리가 만든 함수(do_ls 등)로 연결
        int argc = 0; while(args[argc] != NULL) argc++;

        if (strcmp(args[0], "ls") == 0) { do_ls(argc, args); exit(0); }
        if (strcmp(args[0], "cat") == 0) { do_cat(argc, args); exit(0); }
        
        execvp(args[0], args);
        perror("command 1 execution failed");
        exit(1);
    }

    // 두 번째 자식 (오른쪽 명령어: grep)
    if ((pid2 = fork()) == 0)
    {
        close(STDIN_FILENO); // 표준 입력 닫기
        dup(fd[0]);          // 파이프의 읽기 구멍을 표준 입력으로 복사
        close(fd[0]);
        close(fd[1]);

        //  오른쪽 명령어의 리다이렉션(>) 처리 추가
        for (int i = 0; cmd2[i] != NULL; i++) {
            if (strcmp(cmd2[i], ">") == 0) {
                int fd_out = open(cmd2[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd_out, STDOUT_FILENO); // 표준 출력을 파일로 변경
                close(fd_out);
                cmd2[i] = NULL; // 명령어에서 '>' 제거
                break;
            }
        }

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        // 오른쪽 명령어도 우리가 만든 함수(do_grep 등)로 연결
        int argc2 = 0; while(cmd2[argc2] != NULL) argc2++;

        if (strcmp(cmd2[0], "grep") == 0) { do_grep(argc2, cmd2); exit(0); }
        if (strcmp(cmd2[0], "wc") == 0) { execvp("wc", cmd2); exit(0); } // wc는 시스템꺼 사용

        execvp(cmd2[0], cmd2);
        perror("command 2 execution failed");
        exit(1);
    }

    // 부모 프로세스
    close(fd[0]); // 부모는 파이프를 쓰지 않으므로 close
    close(fd[1]);

    // 두 자식이 끝날 때까지 대기
    wait(NULL);
    wait(NULL);
}

void execute_simple_command(char *args[], int is_bg)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork error");
        return;
    }

    if (pid == 0)
    { // 자식
        for (int i = 0; args[i] != NULL; i++)
        {
            if (strcmp(args[i], ">") == 0)
            {
                int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
                args[i] = NULL;
                break;
            }
            else if (strcmp(args[i], "<") == 0)
            {
                int fd = open(args[i + 1], O_RDONLY);
                dup2(fd, STDIN_FILENO);
                close(fd);
                args[i] = NULL;
                break;
            }
        }

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        int argc = 0;
        while (args[argc] != NULL)
        {
            argc++;
        }

        // 우리가 구현한 명령어로 연결 
        if (strcmp(args[0], "ls") == 0)
        {
            do_ls(argc, args);
            exit(0);
        }
        else if (strcmp(args[0], "pwd") == 0)
        {
            do_pwd(argc, args);
            exit(0);
        }
        else if (strcmp(args[0], "mkdir") == 0)
        {
            do_mkdir(argc, args);
            exit(0);
        }
        else if (strcmp(args[0], "rmdir") == 0)
        {
            do_rmdir(argc, args);
            exit(0);
        }
        else if (strcmp(args[0], "ln") == 0)
        {
            do_ln(argc, args);
            exit(0);
        }
        else if (strcmp(args[0], "cp") == 0)
        {
            do_cp(argc, args);
            exit(0);
        }
        else if (strcmp(args[0], "rm") == 0)
        {
            do_rm(argc, args);
            exit(0);
        }
        else if (strcmp(args[0], "mv") == 0)
        {
            do_mv(argc, args);
            exit(0);
        }
        else if (strcmp(args[0], "cat") == 0)
        {
            do_cat(argc, args);
            exit(0);
        }
        else if (strcmp(args[0], "grep") == 0)
        {
            do_grep(argc, args);
            exit(0);
        }

        // history 및 help 명령어 연결
        else if (strcmp(args[0], "history") == 0) {
            print_history();
            exit(0);
        }
        else if (strcmp(args[0], "help") == 0) {
            print_help();
            exit(0);
        }

        // 우리가 만든 명령어가 아니면 시스템 명령어 실행
        execvp(args[0], args);
        printf("%s: command not found\n", args[0]);
        exit(1);
    }
    else // 부모
    {
        if (is_bg)
            printf("[Background PID: %d]\n", pid);
        else
            waitpid(pid, NULL, 0);
    }
}
