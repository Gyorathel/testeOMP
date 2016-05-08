#include <stddef.h>
#include <iostream>
//OR #define NULL ((void *)0)

template < typename T >
void Make2DArray(T **(&a),int m, int n)
{
	a = new T *[m];
	for (int i = 0; i < m; i++)
		a[i] = new T [n];
}

template < typename T >
void Free2DArray (T **a, int n)
{
	for (int i = 0; i < n; i++)
		delete[]a[i];
	delete[]a;
}

typedef struct node{
	int x;
	int y;
	//int lev;
	node(){x = y = 0;}
	//node(int _x, int _y, int _lev) {x = _x; y = _y; lev = _lev;}
	node(int _x, int _y){x = _x; y = _y;}
}node;


const int direct[2][8] = {//ÉßÐÎ°ËØÔÕó
	{-2,  -1,   1,   2,  -2,  -1,   1,   2},
	{ 1,   2,   2,   1,  -1,  -2,  -2,  -1}
};
