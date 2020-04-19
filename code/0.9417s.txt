#include <iostream>
#include <sstream>
#include <fstream>
#include<vector>
#include<algorithm>
#include <cmath>
#include <cstdlib>
#include <string.h>
#include<thread>
#include<set>
#include <functional> 
#include<unordered_map>
#include<queue>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include<bitset>

using namespace std;
const int N = 560000;
unsigned int cirArr[5];
unsigned int result[5][4000000][7];
unsigned int Gra[280000][50];
unsigned int inDgr[280000];
unsigned int outDgr[280000];
string outStr[280000];
string lstStr[280000];
bitset<280000> visit;

class searchEdge {
public:
	searchEdge(string testFile, string resFile);
	void findRing();
	void storeResult();
	void remSin();
	void storeGra3();

private:
	string testFile_;
	string resFile_;
	int cnt;
	int edgeCount;           //当前边总数
	int subdata;
	int len;

	vector<unordered_map<int, vector<int>>> idMap;
	unsigned int data[N];
	unsigned int pos[N];
	vector<bool> arrFlag;
	unordered_map<unsigned int, int> id2In;
	int subIndex;
	int cicleCnt;
	int curNum;

	void init();
	void dfs(int begin, int index, unsigned int* path, bitset<280000> &visit, int count);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	subIndex = 5;
	cicleCnt = 0;
	curNum = 0;
	init();
}

void searchEdge::init() {
	int fd = open(testFile_.c_str(), O_RDONLY);
	int flen = lseek(fd, 0, SEEK_END);
	char *buf = (char *)mmap(NULL, flen, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	char *p = buf;
	while (*p >= '0' && *p <= '9') {
		char  *q = p;
		//寻找第一个逗号
		while (*q != ',') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
		}
		data[cnt] = curNum;
		++cnt;
		curNum = 0;
		p = q + 1;
		q = p;
		while (*q != ',') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
		}
		data[cnt] = curNum;
		++cnt;
		curNum = 0;
		//寻找换行位置
		p = q + 1;
		q = p;
		while (*q != '\n') {
			++q;
		}
		p = q + 1;
	}
	memcpy(pos, data, cnt * sizeof(unsigned int));
	sort(pos, pos + cnt);
	len = unique(pos, pos + cnt) - pos;

	for (int i = 0; i < len; i++) {
		id2In[pos[i]] = i;
		outStr[i] = (to_string(pos[i]) + ',');
		lstStr[i] = (to_string(pos[i]) + '\n');
	}
	for (int j = 0; j < cnt; j += 2)
	{
		auto u = id2In[data[j]];
		auto v = id2In[data[j + 1]];
		Gra[u][outDgr[u]] = v;
		++inDgr[v];
		++outDgr[u];
	}
}

void searchEdge::remSin()
{
	queue<int> que;
	for (int i = 0; i < len; i++)
	{
		if (0 == inDgr[i])
			que.push(i);

	}
	while (!que.empty())
	{
		int temp = que.front();
		que.pop();
		for (int i = 0; i < outDgr[temp]; ++i)
		{
			int aim = Gra[temp][i];
			--inDgr[aim];
			if (0 == inDgr[aim])
				que.push(aim);
		}
	}

	for (int i = 0; i < len; ++i)
	{
		if (inDgr[i] == 0 || outDgr[i] == 0)
		{
		}
		else
		{
			sort(Gra[i], Gra[i] + outDgr[i]);
		}
	}
}


void searchEdge::storeGra3() {
	idMap = vector<unordered_map<int, vector<int>>>(len, unordered_map<int, vector<int>>());
	for (int i = 0; i < len; i++) {
		for (int k = 0; k < outDgr[i]; ++k)
		{
			auto gk = Gra[i][k];               //对Gra[i]指向的下一个元素值
			for (int j = 0; j < outDgr[gk]; ++j)
			{
				auto gj = Gra[gk][j];        //Gra[gk]指向的下一个元素值
				if (gj < i&&gj < gk)
				{
					idMap[gj][i].push_back(gk);
				}
			}
		}
	}
}

void searchEdge::dfs(int begin, int index, unsigned int* path, bitset<280000> &visit, int count)
{
	path[count] = index;
	++count;
	visit[index] = 1;
	int i = 0;
	int nLen = outDgr[index];
	for (; i < nLen && Gra[index][i] <= begin; ++i)
	{
		int curr = Gra[index][i];
		if (count >= 3 && curr == begin)
		{
			memcpy(result[count - 3][cirArr[count - 3]], path, count * sizeof(unsigned int));
			++cirArr[count - 3];
			++cicleCnt;
			break;
		}
	}
	if (count < 6)
	{
		for (; i < nLen; ++i) {
			int curr = Gra[index][i];
			if (curr != begin && visit[curr] == 0) {
				dfs(begin, curr, path, visit, count);
			}
		}
	}
	else if (arrFlag[index] == true)
	{
		auto elem = idMap[begin][index];
		for (auto next : elem)
		{
			if (visit[next] == 1) continue;
			path[6] = next;
			memcpy(result[4][cirArr[4]], path, 7 * sizeof(unsigned int));
			++cirArr[4];
			++cicleCnt;
		}

	}
	visit[index] = 0;
}

void searchEdge::findRing()
{
	arrFlag = vector<bool>(len, false);
	vector<unsigned int> tempNext(len);
	for (int i = 0; i < len; ++i)
	{
		if (outDgr[i] > 0 && inDgr[i] > 0)
		{
			for (auto &pre : idMap[i])
			{
				arrFlag[pre.first] = true;
				tempNext.emplace_back(pre.first);
			}
			unsigned int path[7];
			dfs(i, i, path, visit, 0);
			for (auto &elem : tempNext)
			{
				arrFlag[elem] = false;
			}
			vector<unsigned int> nul;
			tempNext.swap(nul);
		}

	}
}

void searchEdge::storeResult() {
	int fLen = 77 * (cicleCnt + 1);
	string Str = to_string(cicleCnt) + "\n";
	FILE *out = fopen(resFile_.c_str(), "wb");
	char* buf = (char*)malloc(fLen * sizeof(char));
	char* p = buf;
	memcpy(buf, Str.c_str(), Str.size());
	buf += Str.size();
	for (int i = 0; i < subIndex; ++i)
	{
		for (int k = 0; k < cirArr[i]; ++k)
		{
			auto curcnt = result[i][k];
			int lenn = i + 3;
			for (int j = 0; j < lenn - 1; ++j)
			{
				string str = outStr[curcnt[j]];
				memcpy(buf, str.c_str(), str.size());
				buf += str.size();
			}
			string str = lstStr[curcnt[lenn - 1]];
			memcpy(buf, str.c_str(), str.size());
			buf += str.size();
		}
	}
	fwrite(p, buf - p, sizeof(char), out);
	fclose(out);
}


int main()
{
	cout << "ready to work" << endl;
	string testFile = "/data/test_data.txt";
	string resultFile = "/projects/student/result.txt";
	//string testFile = "test_data1.txt";
	//string resultFile = "/home/yinjie/result.txt";

	clock_t start, end;
	start = clock();
	searchEdge my_searchMap(testFile, resultFile);
	my_searchMap.remSin();
	my_searchMap.storeGra3();
	cout << "find the circle account" << endl;
	my_searchMap.findRing();
	cout << "output result" << endl;
	my_searchMap.storeResult();
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}





