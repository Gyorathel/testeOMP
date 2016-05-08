#include <algorithm>
#include <conio.h>
#include <fstream>
#include <io.h>
#include <iostream>
#include <math.h>
#include <malloc.h>
#include <omp.h>
#include <queue>
#include <stack>
#include <stdlib.h>
#include <time.h>
#include <typeinfo.h>

#include "2DArray.h"
using namespace std;

#define listSize(p) _msize(p)/sizeof(typeid(p).name())
#define MaxThreads 16
#define LEV 3
#define MULT 4
const int procs = omp_get_num_procs();

node **Path;
bool **table[512];					//8*8*8
vector< node > stake[592];			//8*8*8+8*8+8*2
int pathNum = 0;					//all available paths
int allPath = 0;					//request output paths
int *curPath;						//sumary every thread`s path num
int step = 0;						//request step
int begin, end;						// end - begin is the parallel thread num
int row, col;						// the chess table sizes
int orx, ory;						// the first site 

#define MOD 25
int chess[MOD][MOD];
const int ctx = (MOD-1)>>1;
const int cty = (MOD-1)>>1;
const int owl = ((MOD-1)>>2)+1;


void trigrams (){							//make sure the available site for the last 5 steps
	chess[ctx][cty] = -1;//天元
	int bin = 0;
	int end = 1;
	vector<node>vr; vr.push_back( node(ctx, cty) );
	int t = 0; 
	int step = (MOD-1)>>2;
	while(t++< step) {
		end = vr.size();
		for (int k = bin; k < end; k++){
			int x0 = vr[k].x;
			int y0 = vr[k].y;
			for (int i = 0; i < 8; i++) {
				int x = x0 + direct[0][i];
				int y = y0 + direct[1][i];
				if (chess[x][y] == 0) {
					chess[x][y] = t;
					vr.push_back(node(x, y));
				}
			}
		}
		bin = end;
	}
	vr.clear();
}
bool Constraint(int x, int y, int t, int id){				// prune function
	int dx = x - orx;
	int dy = y - ory;
	int remain = step - t;
	int rm = remain<<1;
	if (abs(dx) > rm || abs(dy) > rm) return false;

	if (remain < owl){
		dx += ctx;
		dy += cty;
		if ( !chess[dx][dy] || chess[dx][dy] > remain ) return false;
	}
	return true;
}
inline bool Bound(int x, int y){							// check if out of bound
	return !(0 > x || x >= row || 0 > y || y >= col);
}
inline bool Visited(int x, int y, int id){
	return table[id][x][y];
}
inline bool solution(int t, int x, int y) {					// confirm if a successful path
	/*
	if ( t == step -1) {
		for (int i = 0; i < 8; i++) {
			int _x = x + direct[0][i];
			int _y = y + direct[1][i];
			if (_x == orx && _y == ory)
				return true;
		}
	}
	return false;*/
	int dx = abs(x - orx);
	int dy = abs(y - ory);
	//return dx*dx + dy*dy == 5;
	return ( (dx==1&&dy==2) || (dx==2&&dy==1) ); 
}
void DFS (int id)												// depth first search
{
	int *q = new int [step+3];
	memset(q, 0, _msize(q));
	int t = LEV+1;
	int cursor = MULT * ((end-begin-1+allPath)/(end-begin));
	curPath[id] = 0;

	while (stake[id].size() > LEV) 
	{
		const node &n = stake[id].back();
		if (q[t] == 8) {
			table[id][n.x][n.y] = 0;
			stake[id].pop_back(); 
			q[t--] = 0;
			continue;
		}
		int x = n.x + direct[0][q[t]];
		int y = n.y + direct[1][q[t]];
		q[t]++;
				
		if (Bound(x, y) && !Visited(x, y, id) && Constraint(x, y, t, id)) 
		{
			if (t == step - 1) //步数限定
			{	
				if (solution(t, x, y)) {											
					if (curPath[id] < cursor) {
						int site = curPath[id] + id * cursor;
						int i; 
						for (i = 0; i < stake[id].size(); i++)
							Path[site][i] = stake[id][i];
							Path[site][i] = node(x, y);	  i++;
							Path[site][i] = node(orx, ory);
					}
					curPath[id]++;
				}// end if solution
				//  else NULL;
			}	//	end if t == step-1
			else {
				stake[id].push_back( node(x, y) );
				table[id][x][y] = 1;
				t++;
			}
		}	//	end if Bound && !Visited && Constraint
	}// end while
	#pragma omp atomic
		pathNum += curPath[id];	

	delete []q;
}
bool cmp(){
	return false;
}
void sort(){}
/*
void out0()
{
	if (pathNum < allPath) {
		cout<<"--------------------"<<endl;
		cout << "have not enough Path to tour!\n"<<endl;
	}
	int n = (pathNum<allPath)?pathNum:allPath;
	cout<<"--------------------"<<endl;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j <= step; j++) {
			cout<<(char)(Path[i][j].y+'a')<<(Path[i][j].x+1)<<endl;
		}
		cout<<"-------"<<endl;
	}
	cout<<"There are "<<pathNum<<" possible "<<step<<" move tours!"<<endl;
}*/
void out0()
{
	if (pathNum < allPath) {
		printf("--------------------\n");
		printf("have not enough Path to tour!\n\n");
	}
	printf("--------------------\n");
	int m = (pathNum<allPath)?pathNum:allPath;
	int cursor = MULT * ((end-begin-1+allPath)/(end-begin));
	int p = 0;

	for (int i = begin; i < end; i++) 
	{
		int k = curPath[i]>cursor?cursor:curPath[i];
		k += i * cursor;
		for (int t = i * cursor; t < k; t++)
			if (p < m){
				p++;
				for (int j = 0; j <= step; j++) {
					printf("%c%d\n",(char)(Path[t][j].y+'a'), Path[t][j].x+1);
				}
				printf("-------\n");
			}
			else goto U;
	}
U:	printf("There are %d possible %d move tours!\n\n",pathNum, step);
}
void out()
{
	if (pathNum < allPath) {
		printf("--------------------\n");
		printf("have not enough Path to tour!\n\n");
	}
	int n = (pathNum<allPath)?pathNum:allPath;
	printf("--------------------\n");
	for (int i = 0; i < n; i++) {
		for (int j = 0; j <= step; j++) {
			printf("%c%d\n",(char)(Path[i][j].y+'a'),Path[i][j].x+1);
		}
		printf("-------\n\n");
	}
	printf("There are %d possible %d move tours!\n\n",pathNum, step);
}
void BFS()																// Breadth First Search
{	
	for (int j = 0; j < 592; j++)  stake[j].reserve(step+1);
	if (LEV+1 >= step)
		Make2DArray(Path, allPath, step+1);
	for (int i = 0; i < 512; i++) {
		Make2DArray(table[i], row, col);
		for (int j = 0; j < row; j++)
			memset(table[i][j], 0, _msize(table[i][j]));
	}
	int goon;
	begin = 0; end = 1; goon = 1;
	stake[begin].push_back( node(orx, ory) );
	table[begin][orx][ory] = 1;

	for (int t = 1; t <= LEV && t < step; t++) {
		for (int k = begin; k < end; k++) 
		{
			const node &n = stake[ k ].back();
			for (int j = 0; j < 8; j++) {
				int x = n.x + direct[0][j];
				int y = n.y + direct[1][j];
				if (Bound(x, y) && !Visited(x, y, k) && Constraint(x, y, t, 0))
				{
					if(step == t+1 && solution(t, x, y)) {
						int i;
						if (pathNum < allPath) {
							for (i = 0; i < stake[k].size(); i++)
							   Path[pathNum][i] = node(stake[k][i].x, stake[k][i].y);
							   Path[pathNum][i] = node(x, y);	i++;
							   Path[pathNum][i] = node(orx, ory);
						}
						pathNum++;
					}
					for (int i = 0; i < stake[k].size(); i++)
						stake[goon].push_back(stake[k][i]);
					stake[goon].push_back( node(x, y) );

					for (int i = 0; i < stake[k].size(); i++)
						table[goon][stake[k][i].x][stake[k][i].y] = 1;
					table[goon][x][y] = 1;

					goon++;
				}
			}//	end for j
		}//	end for k
		begin = end; end = goon; 
	}//	end for t
}
void main ()
{
	//ofstream fout("out.txt");
	//streambuf *outbuf = cout.rdbuf(fout.rdbuf());

	char s;
	cin>>col>>row;
	cin>>s>>orx;
	ory = s-'a';
	orx --;
	cin>>step;
	cin>>allPath;
	
	clock_t before, after;
	before = clock();

	if (step&1 || step > col*row) {cout<<"no available path to tour!\n\n"; return;}

	trigrams();

	BFS();
	if (pathNum != 0) {out(); return;}
	
	int r = end * MULT * ((end-begin-1+allPath)/(end-begin));
	Make2DArray(Path, r, step+1);
	curPath = new int[end];
	memset(curPath, 0, _msize(curPath));

//任务排序	
	sort();

#pragma omp parallel for num_threads(end - begin)				//create end-begin threads to parallel run
	for (int id = begin; id < end; id++){
		DFS(id);
	}
	out0();

	for (int i = 0; i < 512; i++)
		Free2DArray(table[i], row);
	Free2DArray(Path, r);
	delete[] curPath;

	after = clock();

	printf("Knight Algorithm done in %.3f secs\n\n",(float)(after - before)/ CLOCKS_PER_SEC);

	//printf("Create Threads %d\n\n",end - begin);

	//cout.rdbuf(outbuf);
	//fout.close();
}
