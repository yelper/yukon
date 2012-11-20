// TestProg.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <cstdlib>
using namespace std;

int overloaded(int i)
{
    return i + 1;
}

int overloaded(int i, int j)
{
    return i + j;
}

int test2(int i, int j)
{
    return i + j;
}

int main(int argc, char** argv)
{
    int first = 1;
    int second = 2;

    int third = overloaded(first);
    int fourth = overloaded(third, second);
    int fifth = test2(first, second);

    cout << "first: " << first << endl;
    cout << "second: " << second << endl;
    cout << "third: " << third << endl;
    cout << "fourth: " << fourth << endl;
    cout << "fifth: " << fifth << endl;
}


