//write an example of enum within a class in c++
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

class A
{
    private:
        int x = 0;
        int y = 0;
    public:
        A& setX(int x){this->x = x;}
        A& setY(int y){this->y = y;}
        int getX(){return x;}
        int getY(){return y;}
};



class B
{
private:
    vector<A> vA;
public:

    B set_VA(vector<A> vA)
    {
        this->vA = vA;
        return *this;
    }

    vector<A> get_VA()
    {
        return vA;
    }

    vector<A>& get_VA_reference_return()
    {
        return vA;
    }

    void print()
    {
        for_each(vA.begin(), vA.end(), [](A a){cout << a.getX() << ", " << a.getY() << endl;});
    }
};


int main()
{
    B &b = *(new B());

    vector<A> vA;
    A &a = *(new A());
    a.setX(1).setY(2);
    vA.push_back(a);

    b.set_VA(vA);

    b.print();

    A a2 = b.get_VA()[0];
    a2.setX(3).setY(4);

    b.print();

    A &a3 = b.get_VA_reference_return()[0];
    a3.setX(5).setY(6);

    b.print();

    return 0;
}



