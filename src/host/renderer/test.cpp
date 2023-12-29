#include <memory>

class Foo {
public:
  Foo() { ref = new float[1000]; }
  ~Foo() { delete[] ref; }
  Foo(Foo &f) { ref = f.ref; };
    
    void bar() {
      // should call destructor afer goind out of scope
      std::shared_ptr<Foo> old = std::make_shared<Foo>(this);
      ref = new float[400];
    }

private:
    float* ref;
};