#include <iostream>
#include <sstream>
#include <fstream>
#include<vector>
#include<algorithm>
#include <cmath>
#include <cstdlib>
#include <string.h>
#include<thread>
#include <functional> 
#include<unordered_map>
#include<queue>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

using namespace std;

const int node = 280000;
const int max_node = 200000;
const int NMIN = 0xFFFFFFFF;
unsigned int cirArr[4][5];
unsigned int result0[4][500000][3], result1[4][500000][4], result2[4][1000000][5], result3[4][2000000][6], result4[4][2000000][7];
unsigned int Gra[node][50];
unsigned int revGra[node][50];
unsigned int inDgr[node];
unsigned int outDgr[node];
string outStr[node];
string lstStr[node];
unsigned int visit[4][node];
unsigned int que[node];
unsigned int pos[node];
unsigned int firstIdFlag[node];
unsigned int idFlag[node];
unsigned int firstId[node];
unsigned int reachBegin2[4][node][2];
unsigned int idMap[4][250][250];
unsigned int idMap3[4][5000][500];
unsigned int idMap3_2[4][5000][500];
unsigned int reachBegin1[4][node];
unsigned int reachBegin3[4][node][2];
unsigned int idPoint2[4][250];
unsigned int current1[4][max_node];

class searchEdge {
public:
	searchEdge(string testFile, string resFile);
	void storeResult();
	void remSin();
	void multiSearch();
private:
	string testFile_;
	string resFile_;
	int cnt;
	int edgeCount;           //当前边总数
	int subdata;
	int firstIdCnt;
	int idCnt;
	int subIndex;
	unsigned int cicleCnt;
	int curNum;
	void init();
	void storeGra3(unsigned int begin, int threadId);
	void findRing(int threadId, unsigned int beginIndex, unsigned int endIndex);
	void dfs(unsigned int begin, unsigned int index, unsigned int* path, unsigned int* visit, int count, int threadId);
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
		firstIdFlag[++curNum] = 1;
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
		Gra[u][outDgr[u]] = ++curNum;
		revGra[curNum][inDgr[curNum]] = u;
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
	for (int i = 1; i < max_node; ++i)
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
		outStr[pos[i]] = (to_string(pos[i] - 1) + ',');
		lstStr[pos[i]] = (to_string(pos[i] - 1) + '\n');
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
		for (int i = 0; i < eLen; ++i)
		{
			v = Gra[u][i];
			if (inDgr[v] == 1)
			{
				que[h++] = v;
				inDgr[v] = 0;
			}
		}
	}
	l = 0, h = 0;
	for (int i = 0; i < firstIdCnt; ++i)
	{
		auto elem = firstId[i];
		if (0 == outDgr[elem])
		{
			que[h++] = elem;
		}
	}
	while (l < h)
	{
		v = que[l++];
		int eLen = inDgr[v];
		for (int i = 0; i < eLen; ++i)
		{
			u = revGra[v][i];
			if (outDgr[u] == 1)
			{
				que[h++] = u;
				outDgr[u] = 0;
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
			sort(revGra[elem], revGra[elem] + inDgr[elem]);
			sort(Gra[elem], Gra[elem] + outDgr[elem]);
			/*if (Gra[elem][outDgr[elem] - 1] < elem || revGra[elem][inDgr[elem] - 1] < elem)
			{
				firstId[i] = NMIN;
			}*/
		}
	}
}

void searchEdge::storeGra3(unsigned int begin, int threadId) {
	auto subLen = 0, subLen3 = 0, pointLen = 0;
	unsigned int add = 0;
	for (int i = 0; i < inDgr[begin]; i++) {
		auto gi = revGra[begin][i];
		current1[threadId][gi] = 1;
		if (outDgr[gi] == 0 || inDgr[gi] == 0 || gi <= begin) continue;
		for (int k = 0; k < inDgr[gi]; ++k)
		{
			unsigned int gk = revGra[gi][k];
			if (outDgr[gk] == 0 || inDgr[gk] == 0 || gk <= begin) continue;
			if (reachBegin2[threadId][gk][1] != begin)
			{
				reachBegin2[threadId][gk][0] = subLen;//数量
				reachBegin2[threadId][gk][1] = begin; //终止点
				idMap[threadId][subLen++][0] = 0; //数量
				idPoint2[threadId][pointLen++] = gk; //子节点
			}
			add = reachBegin2[threadId][gk][0];
			idMap[threadId][add][++idMap[threadId][add][0]] = gi;//中间节点
		}
	}
	sort(idPoint2[threadId], idPoint2[threadId] + pointLen);
	for (int j = 0; j < pointLen; ++j)
	{
		unsigned int gj = idPoint2[threadId][j];            //倒数第二个指向begin的
		subLen = reachBegin2[threadId][gj][0];
		for (int t = 1; t <= idMap[threadId][subLen][0]; ++t)
		{
			unsigned int gt = idMap[threadId][subLen][t];       //倒数第一个指向begin
			for (int k = 0; k < inDgr[gj]; ++k)
			{
				unsigned int gk = revGra[gj][k];
				if (gk <= begin || gk == gt || inDgr[gk] == 0 || outDgr[gk] == 0)continue;
				if (reachBegin3[threadId][gk][1] != begin)
				{
					reachBegin3[threadId][gk][0] = subLen3;
					reachBegin3[threadId][gk][1] = begin;
					idMap3_2[threadId][subLen3][0] = 0;
					idMap3[threadId][subLen3++][0] = 0;
				}
				add = reachBegin3[threadId][gk][0];
				idMap3[threadId][add][++idMap3[threadId][add][0]] = gj;
				idMap3_2[threadId][add][++idMap3_2[threadId][add][0]] = gt;
			}
		}
	}
}

void searchEdge::dfs(unsigned int begin, unsigned int index, unsigned int* path, unsigned int* visit, int count, int threadId)
{
	path[count] = index;
	++count;
	visit[index] = 1;
	int i = 0;
	int nLen = outDgr[index];
	if (count < 5)
	{
		for (; i < nLen; ++i) {
			if (Gra[index][i] < begin)continue;
			int curr = Gra[index][i];
			if (curr != begin) {
				if (visit[curr] == 1)continue;
				dfs(begin, curr, path, visit, count, threadId);
			}
			else {
				switch (count - 3) {
				case 0:
					memcpy(result0[threadId][cirArr[threadId][0]], path, count * sizeof(unsigned int));
					++cirArr[threadId][0];
					break;
				case 1:
					memcpy(result1[threadId][cirArr[threadId][1]], path, count * sizeof(unsigned int));
					++cirArr[threadId][1];
					break;
				}
			}
		}
	}
	else {
		if (current1[threadId][index] == 1) {
			memcpy(result2[threadId][cirArr[threadId][2]], path, count * sizeof(unsigned int));
			++cirArr[threadId][2];
		}
		if (reachBegin2[threadId][index][1] == begin && count == 5)
		{
			auto elem = idMap[threadId][reachBegin2[threadId][index][0]];
			for (int j = 1; j <= elem[0]; ++j)
			{
				auto next = elem[j];
				if (visit[next] == 1) continue;
				path[5] = next;
				memcpy(result3[threadId][cirArr[threadId][3]], path, 6 * sizeof(unsigned int));
				++cirArr[threadId][3];
			}
		}
		if (reachBegin3[threadId][index][1] == begin && count == 5)
		{
			auto elem = idMap3[threadId][reachBegin3[threadId][index][0]];
			for (int j = 1; j <= elem[0]; ++j)
			{
				unsigned int next1 = elem[j];
				unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][index][0]][j];
				if (visit[next1] == 1 || visit[next2] == 1) continue;
				path[5] = next1;          //gj
				path[6] = next2;          //gt
				memcpy(result4[threadId][cirArr[threadId][4]], path, 7 * sizeof(unsigned int));
				++cirArr[threadId][4];
			}
		}
	}
	visit[index] = 0;
}
void searchEdge::findRing(int threadId, unsigned int beginIndex, unsigned int endIndex)
{
	for (auto i = beginIndex; i < endIndex; ++i)
	{
		auto begin = firstId[i];             //只考虑起始点
		//if (begin == NMIN) continue;
		if (outDgr[begin] > 0 && inDgr[begin] > 0)
		{
			unsigned int path[7];

			storeGra3(begin, threadId);
			dfs(begin, begin, path, visit[threadId], 0, threadId);
		}
		for (int j = 0; j < inDgr[begin]; ++j)
		{
			current1[threadId][revGra[begin][j]] = 0;
		}
	}
}

void searchEdge::multiSearch()
{
	thread thread1(bind(&searchEdge::findRing, this, 0, 0, (unsigned int)(0.05 * firstIdCnt)));
	thread thread2(bind(&searchEdge::findRing, this, 1, (unsigned int)(firstIdCnt * 0.05), (unsigned int)(firstIdCnt * 0.1)));
	thread thread3(bind(&searchEdge::findRing, this, 2, (unsigned int)(firstIdCnt * 0.1), (unsigned int)(firstIdCnt * 0.25)));
	thread thread4(bind(&searchEdge::findRing, this, 3, (unsigned int)(firstIdCnt * 0.25), (unsigned int)firstIdCnt));
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
}

void searchEdge::storeResult() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 5; j++) {
			cicleCnt += cirArr[i][j];
		}
	}
	int fLen = 77 * (cicleCnt + 1);
	string Str = to_string(cicleCnt) + "\n";
	FILE* out = fopen(resFile_.c_str(), "wb");
	char* buf = (char*)malloc(fLen * sizeof(char));
	char* p = buf;
	memcpy(buf, Str.c_str(), Str.size());
	buf += Str.size();
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][0]; ++k)
		{
			auto curcnt = result0[t][k];
			for (int j = 0; j < 2; ++j)
			{
				string str = outStr[curcnt[j]];
				memcpy(buf, str.c_str(), str.size());
				buf += str.size();
			}
			string str = lstStr[curcnt[2]];
			memcpy(buf, str.c_str(), str.size());
			buf += str.size();
		}
	}
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][1]; ++k)
		{
			auto curcnt = result1[t][k];
			for (int j = 0; j < 3; ++j)
			{
				string str = outStr[curcnt[j]];
				memcpy(buf, str.c_str(), str.size());
				buf += str.size();
			}
			string str = lstStr[curcnt[3]];
			memcpy(buf, str.c_str(), str.size());
			buf += str.size();
		}
	}
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][2]; ++k)
		{
			auto curcnt = result2[t][k];
			for (int j = 0; j < 4; ++j)
			{
				string str = outStr[curcnt[j]];
				memcpy(buf, str.c_str(), str.size());
				buf += str.size();
			}
			string str = lstStr[curcnt[4]];
			memcpy(buf, str.c_str(), str.size());
			buf += str.size();
		}
	}
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][3]; ++k)
		{
			auto curcnt = result3[t][k];
			for (int j = 0; j < 5; ++j)
			{
				string str = outStr[curcnt[j]];
				memcpy(buf, str.c_str(), str.size());
				buf += str.size();
			}
			string str = lstStr[curcnt[5]];
			memcpy(buf, str.c_str(), str.size());
			buf += str.size();
		}
	}
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][4]; ++k)
		{
			auto curcnt = result4[t][k];
			for (int j = 0; j < 6; ++j)
			{
				string str = outStr[curcnt[j]];
				memcpy(buf, str.c_str(), str.size());
				buf += str.size();
			}
			string str = lstStr[curcnt[6]];
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
	clock_t start, end;
	start = clock();
	searchEdge my_searchMap(testFile, resultFile);
	my_searchMap.remSin();
	//my_searchMap.storeGra3();
	cout << "find the circle account" << endl;
	my_searchMap.multiSearch();
	cout << "output result" << endl;
	my_searchMap.storeResult();
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}











