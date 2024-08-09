#include <bits/stdc++.h>
using namespace std;

class A {
public:
  //A() noexcept = default;
  A() noexcept {
    cout<<"my starting"<<endl;
  }
  
  ~A() { 
      cout<< "~A() hello" <<endl; 
    }
};


void f() {
  A a;
  //throw "spam";
}

int main() {
  /*
  try { 
    f(); 
  } catch (...) 
  { throw; }
  */
  // test kill -9 id
  f();
  while(1){}
}
