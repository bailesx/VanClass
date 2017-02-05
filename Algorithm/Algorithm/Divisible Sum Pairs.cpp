/*
입력1. N과 K를 받는다
입력2. 정수 리스트 A를 받는데, 그 개수는 N만큼이다

정수 i와 j는 입력받은 A의 두 정수를 더해 K로 나누었을 때 나누어 떨어지는 값이다.
이 i와 j가 발생할 수 있는 경우의 수를 구하라.

예제
INPUT
6 3
1 3 2 6 1 2

OUTPUT
5

EXPLANATION
A[0+2]
A[0+5]
A[1+3]
A[2+4]
A[4+5]
*/
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace std;


int main(){
	int n;
	int k;
	cin >> n >> k;
	vector<int> a(n);
	for(int a_i = 0;a_i < n;a_i++){
		cin >> a[a_i];
	}

	int count = 0;
	for(int i = 0; i < n; i++)
	{
		for(int j = i + 1; j < n; j++)
		{
			if((a[i]+a[j])%k==0)
			{
				count++;
			}
		}
	}
	cout << count;
	return 0;
}
