////////////////////////////////////////////////////////////////////////////////
//
//  File           : lib/demo/lkm-demo/gdb-demo.c
//  Description    : This is a simple demo for how to use LIBIHT GDB plugin to
//                   assist debugging and tracing for a user-space application.
//
//   Author        : Di Wu, Thomason Zhao
//   Last Modified : July 10, 2024

#include <stdio.h>
#include <unistd.h>

int N = 10;
void func1();
void func2();

void func1() {
    if (N == 0) {
        return;
    }
    func2();
}

void func2() {
    N--;
    func1();
}

int main() {
    int pid = getpid();
    fprintf(stderr, "Current pid : %d\n", pid);
    func1();
    return 0;
}
