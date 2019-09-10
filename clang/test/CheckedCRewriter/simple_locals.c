// Tests for Checked C rewriter tool.
//
// Checks very simple inference properties for local variables.
//
// RUN: checked-c-convert %s -- | FileCheck -match-full-lines %s
// RUN: checked-c-convert %s -- | %clang_cc1 -verify -fcheckedc-extension -x c -
// expected-no-diagnostics

void f1(void) {
    int b = 0;
    int *a = &b;
    *a = 1;
}
// CHECK: void f1(void) {
// CHECK-NEXT: int b = 0;
// CHECK-NEXT: _Ptr<int> a = &b;

void f2(void) {
    char b = 'a';
    char *a = &b;
    *a = 'b';
}
//CHECK: void f2(void) {
//CHECK-NEXT: char b = 'a';
//CHECK-NEXT: _Ptr<char> a = &b;

typedef struct _BarRec {
  int a;
  int b;
  int c;
  int *d;
} BarRec;

void upd(BarRec *P, int a) {
  P->a = a;
}
//CHECK: void upd(_Ptr<BarRec> P, int a) {
//CHECK-NEXT: P->a = a;
//CHECK-NEXT: }

void canthelp(int *a, int b, int c) {
  *(a + b) = c;
}
//CHECK: void canthelp(int *a, int b, int c) {
//CHECK-NEXT:  *(a + b) = c;
//CHECK-NEXT: }

void partialhelp(int *a, int b, int c) {
  int *d = a;
  *d = 0;
  *(a + b) = c;
}
//CHECK: void partialhelp(int *a, int b, int c) {
//CHECK-NEXT: int *d = a;
//CHECK-NEXT: *d = 0;
//CHECK-NEXT:  *(a + b) = c;
//CHECK-NEXT: }

void g(void) {
    int a = 0, *b = &a;
    *b = 1;
}
//CHECK: void g(void) {
//CHECK-NEXT: int a = 0;
//CHECK-NEXT: _Ptr<int> b = &a;

void gg(void) {
  int a = 0, *b = &a, **c = &b;

  *b = 1;
  **c = 2;
}
//CHECK: void gg(void) {
//CHECK-NEXT: int a = 0;
//CHECK-NEXT: _Ptr<int> b = &a;
//CHECK-NEXT: _Ptr<_Ptr<int>> c = &b;

#define ONE 1

int goo(int *, int);
//CHECK: int goo(int *, int);

struct blah {
  int a;
  int b;
  struct blah *c;
};

int bar(int, int);

int foo(int a, int b) {
  int tmp = a + ONE;
  int *tmp2 = &tmp;
  return tmp + b + *tmp2;
}
//CHECK: int foo(int a, int b) {
//CHECK-NEXT: int tmp = a + ONE;
//CHECK-NEXT: _Ptr<int> tmp2 = &tmp;
//CHECK-NEXT: return tmp + b + *tmp2;
//CHECK-NEXT: }

int bar(int a, int b) {
  return a + b;
}
//CHECK: int bar(int a, int b) {
//CHECK-NEXT: return a + b;
//CHECK-NEXT: }

int baz(int *a, int b, int c) {
  int tmp = b + c;
  int *aa = a;
  *aa = tmp;
  return tmp;
}
//CHECK: int baz(_Ptr<int> a, int b, int c) {
//CHECK-NEXT: int tmp = b + c;
//CHECK-NEXT: _Ptr<int> aa = a;
//CHECK-NEXT: *aa = tmp;
//CHECK-NEXT: return tmp;

int arrcheck(int *a, int b) {
  return a[b];
}
//CHECK: int arrcheck(int *a, int b) {
//CHECK-NEXT: return a[b];
//CHECK-NEXT: }

int badcall(int *a, int b) {
  return arrcheck(a, b);
}
//CHECK: int badcall(int *a, int b) {
//CHECK-NEXT: return arrcheck(a, b); 
//CHECK-NEXT: }

void pullit(char *base, char *out, int *index) {
  char tmp = base[*index];
  *out = tmp;
  *index = *index + 1;

  return;
}
//CHECK: void pullit(char *base, _Ptr<char> out, _Ptr<int> index) {

void driver() {
  char buf[10] = { 0 };
  int index = 0;
  char v;

  pullit(buf, &v, &index);
  pullit(buf, &v, &index);
  pullit(buf, &v, &index);
}

typedef struct _sfoo {
  int a;
  int b;
  struct _sfoo *next;
} sfoo;
//CHECK: _Ptr<struct _sfoo> next;

int sum(sfoo *p) {
  int a = 0;
  while (p) {
    a += p->a + p->b;
    p = p->next;
  }

  return a;
}
//CHECK: int sum(_Ptr<sfoo> p) {

typedef struct _A {
  int a;
  int b;
} A, *PA, **PPA;

extern void adfsa(PA f);

void dfnk(int a, int b) {
  A j;
  PA k = &j;
  PPA u = &k;
  j.a = a;
  j.b = b;

  adfsa(&j);
}
//CHECK: void dfnk(int a, int b) {
//CHECK-NEXT: A j;
//CHECK-NEXT: _Ptr<struct _A>  k = &j;
//CHECK-NEXT: _Ptr<_Ptr<struct _A>>  u = &k;

void adsfse(void) {
  int a = 0;
  int *b = &a;

  b += 4;
  *b = 0;
}
//CHECK: void adsfse(void) {
//CHECK-NEXT: int a = 0;
//CHECK-NEXT: int *b = &a;

void dknbhd(void) {
  int a = 0;
  int *b = &a;
  int **c = &b;
  int *d = *c;

  *b = 0;

  **c = 1;


  *(d + 4) = 4;
}
//CHECK: void dknbhd(void) {
//CHECK-NEXT: int a = 0;
//CHECK-NEXT: int *b = &a;
//CHECK-NEXT: int **c = &b;
//CHECK-NEXT: int *d = *c;

extern void dfefwefrw(int **);

void cvxqqef(void) {
  int a = 0;
  int *b = &a;
  int *c = &a;

  *c = 1;

  dfefwefrw(&b);
}
//CHECK: void cvxqqef(void) {
//CHECK-NEXT: int a = 0;
//CHECK-NEXT: int *b = &a;
//CHECK-NEXT: _Ptr<int> c = &a;

// Check that constraints involving arrays work.
void ptrarr(void) {
  int *vals[4] = { 0 };
  int a = 0;
  int b = 0;
  int c = 0;
  int d = 0;

  vals[0] = &a;
  vals[1] = &b;
  vals[2] = &c;
  vals[3] = &d;

  return;
}
//CHECK: void ptrarr(void) { 
//CHECK-NEXT: _Ptr<int> vals[4] =  { 0 };

