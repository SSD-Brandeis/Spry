//write an example of enum within a class in c++
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

class A
{
    private:
        int x = 0;
        int *y = new int[12];

    public:

        A setX_not_reference_return(int x)
        {
            this->x = x;
            return *this;
        }

        A& setX_reference_return(int x)
        {
            this->x = x;
            return *this;
        }

        A setY_not_reference_return(int *y)
        {
            free(this->y);
            this->y = y;
            return *this;
        }

        A& setY_reference_return(int *y)
        {
            free(this->y);
            this->y = y;
            return *this;
        }

        int& getX_reference_return(){return x;}
        int getX_not_reference_return(){return x;}

        void print()
        {
            cout << "x: " ;
            cout << x << ", " ;
            cout << "y: ";
            for_each(y, y+12, [](int x){cout << x << ", ";});
            cout << endl;
        }
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

    A getIthA_not_reference_return(int i){return vA[i];}
    A& getIthA_reference_return(int i){return vA[i];}
    
    void print()
    {
        for_each(vA.begin(), vA.end(), [](A x){x.print();});
    }
};


int main()
{
    B &b = *(new B());

    vector<A> vA;
    int *y = new int[12];
    y[0] = 1;
    y[1] = 2;
    y[2] = 3;
    vA.push_back( A().setX_not_reference_return(1). setY_not_reference_return(y) );
    //print vA size
    cout << vA.size() << endl;
    for_each(vA.begin(), vA.end(), [](A x){x.print();});

 
    cout << endl << "#2 " << endl;
    b.set_VA(vA);
    int &x1 = vA[0].getX_reference_return();
    vA[0].print();
    x1 = 3;
    cout << x1 << endl;
    vA[0].print();
    b.set_VA(vA);


    cout << endl << "#3 " << endl;
    int x2 = vA[0].getX_not_reference_return();
    vA[0].print();
    x2 = 5;
    cout << x2 << endl;
    vA[0].print();
    b.set_VA(vA);



    cout << endl << "#4 " << endl;
    //print B
    cout << "B: " << endl;
    b.print();

    //use printf print unsigned long vA size
    // printf("%lu ", vA.size());

    return 0;
}



