#include <stdlib.h>
#include <stdio.h>

void fn2() {
  throw 0;
}

void fn1() {
  fn2();
}

int main() {
  fn1();
}
