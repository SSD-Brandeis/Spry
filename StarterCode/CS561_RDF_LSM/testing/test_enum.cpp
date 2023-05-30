//write an example of enum within a class in c++
#include <iostream>
#include <bits/stdc++.h>
using namespace std;
class test
{
public:
    enum color {red, green, blue};

    color c, c1, c2;
    
    test();

    test(color c1)
    {
        c = c1;
    }

    // test set_c1(color c1)
    test& set_c1(color c1)
    {
        this->c1 = c1;
        return *this;
    }

    test& set_c2(color c2)
    {
        this->c2 = c2;
        return *this;
    }
    
    void print()
    {
        cout << c << ", " << c1 << ", " << c2 << endl;
    }
};
test::test() : c(red), c1(green), c2(blue) {}


int main()
{
    test t(test::green);
    test t2 = t.set_c1(test::blue).set_c2(test::red);
    // test &t2 = t.set_c1(test::blue).set_c2(test::red);
    t.set_c1(test::red);
    t2.print();
    t.print();
    vector<test> v(1);
    for(auto &x: v){
        x.set_c1(test::blue).set_c2(test::red);
        x.print();
    }
    return 0;
}



