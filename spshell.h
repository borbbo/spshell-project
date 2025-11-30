/* spshell.h - 함수 선언 및 공통 헤더 */
#ifndef SPSHELL_H
#define SPSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

// 매크로 상수 정의
#define MAX_CMD_LEN 1024
#define MAX_ARG_LEN 100

// --- [팀장(A) 영역] 유틸리티 및 쉘 코어 함수 ---
int tokenize(char *cmd, char *args[]);

// --- [팀원 B 영역] 파일 구조/정보 관련 명령어 ---
void do_ls(int argc, char *argv[]);
void do_pwd(int argc, char *argv[]);
void do_cd(int argc, char *argv[]);    
void do_mkdir(int argc, char *argv[]);
void do_rmdir(int argc, char *argv[]);
void do_ln(int argc, char *argv[]);

// --- [팀원 C 영역] 파일 내용/조작 관련 명령어 ---
void do_cp(int argc, char *argv[]);
void do_rm(int argc, char *argv[]);
void do_mv(int argc, char *argv[]);
void do_cat(int argc, char *argv[]);
void do_grep(int argc, char *argv[]);

#endif
