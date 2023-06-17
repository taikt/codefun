#include <iostream>
using namespace std;

template<typename T, int size, typename callback>
void ForEach(T(&arr)[size], callback op) {
    for (int i=0;i<size;i++) {
        op(arr[i]);
    }
}

// test capture class's member variable
class Myclass {
public:
    float price=4;
public:
    void changeMe() {
        int basePrice = 2;
        [basePrice,this]() { // de capture member variable of class, can pass this pointer to capture list
            price += basePrice;
        }();// invoke lamda
    }
    void assignFinalPrice() {
        float taxes[]{2,5,6};
        float basePrice(price);
        ForEach(taxes,[basePrice,this](float tax){
                    float taxedPrice = basePrice * tax/100;
                    price += taxedPrice;
                });
    }
};

int main() {
    []() {
        cout<<"welcome lambda\n";
    }();// invoke lambda

    auto fn = []() {
        cout<<"welcome lambda2\n";
    };
    fn();// fn is function object
    // lambda expression duoc thuc hien internally nhu 1 function object

    auto sum = [](int x, int y) {
        return x+y;
    };

    cout<<sum(1,2)<<"\n";
    cout<<"print array\n";
    int arr[]{1,4,5};
    int offset = 3;
    ForEach(arr,[offset](int &x) mutable{ // capture list la ben trong []
                x += offset; // copy bien offset ben ngoai by value
                offset++;    // khong the modify offset trong lamda
                             // => solution?: them keyword mutable

            });
    int st{};
    ForEach(arr,[st](int &x) mutable{
                st += x;
            });
    cout<<"sum1="<<st<<"\n"; // st ben trong va ngoai khac nhau, nen st khong duoc update result
                            // solution: pass capture list by reference, keyword mutable co the luoc bo
                            // check next
    ForEach(arr,[&st](int &x){ // capture list by reference
                st += x;
            });
    cout<<"sum2="<<st<<"\n";

    ForEach(arr,[&st,offset](int &x){ // capture list by reference and value combine
                                      // capture st by reference, offset by value
                x += offset;
                st += x;
            });
    cout<<"sum3="<<st<<"\n";

    ForEach(arr,[&](int &x){ // capture all variables by reference (i.e offset, st captured by reference)
                st += x;
            });
    cout<<"sum4="<<st<<"\n";

    ForEach(arr,[=](int &x){ // capture all variables by value (i.e offset, st captured by value)
                //st += x; // error compile
            });

    ForEach(arr,[=,&st](int &x){ // capture all variables by value except st by reference
                st += x;
            });
     cout<<"sum5="<<st<<"\n";

    ForEach(arr,[&,offset](int &x){ // capture all variables by reference except offset by value
                st += x;
            });
    // important note: capture list se capture local variable, cac bien static va global duoc capture tu dong

    // test capture class's member variable
    Myclass a;
    a.changeMe();
    cout<<"price="<<a.price<<"\n";
    a.assignFinalPrice();
    cout<<"taxed price="<<a.price<<"\n";
    ForEach(arr,[](int x){
                cout<<x<<" ";
            });


    cout<<"\n";
    return 0;
}
