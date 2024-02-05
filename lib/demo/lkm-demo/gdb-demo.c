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
