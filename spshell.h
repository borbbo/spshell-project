/* spshell.h - 함수 선언 및 공통 헤더 */
#ifndef SPSHELL_H
#define SPSHELL_H

// --- 표준 라이브러리 ---
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>           
#include <errno.h>

// --- 시스템 라이브러리 ---
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>       // 파일 정보
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>            // 사용자 정보 (struct passwd)
#include <grp.h>            // 그룹 정보 (struct group)
#include <signal.h>

// --- 매크로 상수 정의 ---
#define MAX_CMD_LEN 1024
#define MAX_ARG_LEN 100
#define MAX_HISTORY 100     

// --- 전역 변수 (히스토리용) ---
extern char history_buffer[MAX_HISTORY][MAX_CMD_LEN];
extern int history_count;

// --- 함수 프로토타입 선언 ---

// [강보민 - 유틸리티 및 쉘 코어]
int tokenize(char *cmd, char *args[]);
void add_history(char *cmd);    
void print_history(void);       
void print_help(void);          

// [이시연 - 파일 구조 관련 명령어]
void do_ls(int argc, char *argv[]);
void do_pwd(int argc, char *argv[]);
void do_mkdir(int argc, char *argv[]);
void do_rmdir(int argc, char *argv[]);
void do_ln(int argc, char *argv[]);

// [송녕경 - 파일 내용 조작 명령어]
void do_cp(int argc, char *argv[]);
void do_rm(int argc, char *argv[]);
void do_mv(int argc, char *argv[]);
void do_cat(int argc, char *argv[]);
void do_grep(int argc, char *argv[]);

#endif