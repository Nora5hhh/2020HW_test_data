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
const int minValue = 0xFFFFFFFF;
const int node = 280000;
unsigned int cirArr[5];
unsigned int result[5][3000000][7];
unsigned int Gra[node][50];
unsigned int inDgr[node];
unsigned int outDgr[node];
string outStr[node];
string lstStr[node];
unsigned int visit[node];
unsigned int que[node];

unsigned int pos[2 * node];
unsigned int firstIdFlag[node];
unsigned int idFlag[2 * node];
unsigned int firstId[node];

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
	int firstIdCnt;
	int idCnt;

	vector<unordered_map<unsigned int, vector<unsigned int>>> idMap;
	unsigned int data[2 * node];
	vector<bool> arrFlag;
	int subIndex;
	int cicleCnt;
	int curNum;

	void init();
	void dfs(unsigned int begin, unsigned int index, unsigned int* path, unsigned int* visit, int count);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	subIndex = 5;
	cicleCnt = 0;
	curNum = 0;
	firstIdCnt = 0;
	idCnt = 0;
	init();
}

void searchEdge::init() {
	int u = 0;
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
		firstIdFlag[curNum] = 1;
		idFlag[curNum] = 1;
		u = curNum;
		++cnt;
		curNum = 0;
		p = q + 1;
		q = p;
		while (*q != ',') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
		}
		data[cnt] = curNum;
		Gra[u][outDgr[u]] = curNum;
		idFlag[curNum] = 1;
		++inDgr[curNum];
		++outDgr[u];
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
	for (int i = 0; i < node; ++i)
	{
		if (firstIdFlag[i] == 1)
		{
			firstId[firstIdCnt++] = i;
		}
		if (idFlag[i] == 1)
		{
			pos[idCnt++] = i;       //以此有序
		}
	}
	for (int i = 0; i < idCnt; i++) {
		outStr[pos[i]] = (to_string(pos[i]) + ',');
		lstStr[pos[i]] = (to_string(pos[i]) + '\n');
	}
}

void searchEdge::remSin()
{
	unsigned int l = 0, h = 0;
	unsigned int u = 0, v = 0;
	for (int i = 0; i < firstIdCnt; i++)
	{
		auto elem = firstId[i];      //只对起始点判断出度入度
		if (0 == inDgr[elem])
			que[h++] = elem;
	}
	while (l < h)
	{
		u = que[l++];
		int eLen = outDgr[u];            //对于入度为0的点，遍历它指向的点
		outDgr[u] = 0;
		for (int i = 0; i < eLen; ++i)
		{
			v = Gra[u][i];
			--inDgr[v];
			if (inDgr[v] == 0)
			{
				que[h++] = v;
			}
		}
	}

	for (int i = 0; i < firstIdCnt; ++i)
	{
		auto elem = firstId[i];
		if (inDgr[elem] == 0 || outDgr[elem] == 0)
		{
		}
		else
		{
			sort(Gra[elem], Gra[elem] + outDgr[elem]);
		}
	}
}

void searchEdge::storeGra3() {
	idMap = vector<unordered_map<unsigned int, vector<unsigned int>>>(node, unordered_map<unsigned int, vector<unsigned int>>());
	for (int i = 0; i < firstIdCnt; i++) {
		auto gi = firstId[i];
		if (outDgr[gi] == 0 || inDgr[gi] == 0) continue;
		for (int k = 0; k < outDgr[gi]; ++k)
		{
			unsigned int gk = Gra[gi][k];               //对Gra[i]指向的下一个元素值
			if (outDgr[gk] == 0 || inDgr[gk] == 0) continue;
			for (int j = 0; j < outDgr[gk]; ++j)
			{
				unsigned int gj = Gra[gk][j];        //Gra[gk]指向的下一个元素值
				if (gj < gi&&gj < gk)
				{
					idMap[gj][gi].push_back(gk);
				}
			}
		}
	}
}

void searchEdge::dfs(unsigned int begin, unsigned int index, unsigned int* path, unsigned int* visit, int count)
{
	path[count] = index;
	++count;
	visit[index] = 1;
	int i = 0;
	int nLen = outDgr[index];
	for (; i < nLen && Gra[index][i] <= begin; ++i)
	{
		unsigned int curr = Gra[index][i];
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
	arrFlag = vector<bool>(idCnt, false);
	vector<unsigned int> tempNext(idCnt);
	for (int i = 0; i < firstIdCnt; ++i)
	{
		auto begin = firstId[i];             //只考虑起始点
		if (outDgr[begin] > 0 && inDgr[begin] > 0)
		{
			for (auto &pre : idMap[begin])
			{
				arrFlag[pre.first] = true;
				tempNext.emplace_back(pre.first);
			}
			unsigned int path[7];
			dfs(begin, begin, path, visit, 0);
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
	//string testFile = "test_data5.txt";
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





