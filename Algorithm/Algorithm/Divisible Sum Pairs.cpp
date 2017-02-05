/*
�Է�1. N�� K�� �޴´�
�Է�2. ���� ����Ʈ A�� �޴µ�, �� ������ N��ŭ�̴�

���� i�� j�� �Է¹��� A�� �� ������ ���� K�� �������� �� ������ �������� ���̴�.
�� i�� j�� �߻��� �� �ִ� ����� ���� ���϶�.

����
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
