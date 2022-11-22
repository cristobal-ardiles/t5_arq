
typedef void (*Fun)();

extern void f();
extern void g(Fun);

void h() {
  g(f);
}
