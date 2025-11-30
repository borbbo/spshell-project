CC = gcc
CFLAGS = -Wall -g
TARGET = spshell
OBJS = main.o commands.o

# 기본 타겟
all: $(TARGET)

# 링크
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 컴파일 (.c -> .o 자동 규칙)
%.o: %.c spshell.h
	$(CC) $(CFLAGS) -c $<

# 청소
clean:
	rm -f *.o $(TARGET)