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
#include<queue>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

using namespace std;

const int node = 60000;
const int maxValue = 60000;

const unsigned int sizeGra[10] = { 9,99,999,9999,99999,999999,9999999,99999999 ,999999999,0x7fffffff };
unsigned int cirArr[4][5];
unsigned int result0[4][500000][3], result1[4][500000][4], result2[4][1000000][5], result3[4][2000000][6], result4[4][2000000][7];
unsigned int Gra[node][50];
unsigned int revGra[node][50];
unsigned int inDgr[node];
unsigned int outDgr[node];
char outStr[node][11];
char lstStr[node][11];
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

unsigned int uintToChar(unsigned int num, char* buf)
{
	unsigned int i = 0;
	for (;; ++i)
	{
		if (num <= sizeGra[i])
		{
			++i;
			break;
		}
	}
	unsigned int len = i;
	while (i)
	{
		buf[--i] = num % 10 + '0';
		num /= 10;
	}
	return len;
}

class searchEdge {
public:
	searchEdge(string testFile, string resFile);
	void storeResult();
	void remSin();
	void multiSearch();
private:
	string testFile_;
	string resFile_;

	int firstIdCnt;
	int idCnt;
	unsigned int subLen;
	unsigned int ringCnt;
	int curNum;

	void init();
	void storeGra3(unsigned int begin, int threadId);
	void findRing(int threadId, unsigned int beginIndex, unsigned int endIndex);
	void dfs(unsigned int begin, unsigned int index, unsigned int* path, unsigned int* visit, int count, int threadId);
	void dfs_circle(unsigned int begin, unsigned int* path, unsigned int* visit, int threadId);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	ringCnt = 0;
	curNum = 0;
	subLen = 0;
	firstIdCnt = 0;
	idCnt = 0;
	init();
}

void searchEdge::init() {
	int u = 0;
	int fd = open(testFile_.c_str(), O_RDONLY);
	int flen = lseek(fd, 0, SEEK_END);
	char* buf = (char*)mmap(NULL, flen, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	char* p = buf;
	while (*p >= '0' && *p <= '9') {
		char* q = p;
		//寻找第一个逗号
		while (*q != ',') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
		}
		if (curNum > 50000) {
			while (*q != '\n') {
				++q;
			}
			p = q + 1;
			curNum = 0;
			continue;
		}
		firstIdFlag[++curNum] = 1;
		idFlag[curNum] = 1;
		u = curNum;

		curNum = 0;
		p = q + 1;
		q = p;
		while (*q != ',') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
		}
		if (curNum > 50000) {
			while (*q != '\n') {
				++q;
			}
			p = q + 1;
			curNum = 0;
			continue;
		}
		Gra[u][outDgr[u]] = ++curNum;
		revGra[curNum][inDgr[curNum]] = u;
		idFlag[curNum] = 1;
		++inDgr[curNum];
		++outDgr[u];

		curNum = 0;
		//寻找换行位置
		p = q + 1;
		q = p;
		while (*q != '\n') {
			++q;
		}
		p = q + 1;
	}
	for (int i = 1; i < maxValue; ++i)
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
		subLen = uintToChar(pos[i] - 1, outStr[pos[i]] + 1);
		outStr[pos[i]][++subLen] = ',';
		outStr[pos[i]][0] = (char)subLen;
		subLen = uintToChar(pos[i] - 1, lstStr[pos[i]] + 1);
		lstStr[pos[i]][++subLen] = '\n';
		lstStr[pos[i]][0] = (char)subLen;
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

		}
	}
}

void searchEdge::storeGra3(unsigned int begin, int threadId) {

	auto subLen = 0, subLen3 = 0, pointLen = 0;

	unsigned int ind = 0;

	for (int i = 0; i < inDgr[begin]; i++) {

		auto gi = revGra[begin][i];

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

			ind = reachBegin2[threadId][gk][0];

			idMap[threadId][ind][++idMap[threadId][ind][0]] = gi;//中间节点

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

				if (gk < begin || gk == gt || inDgr[gk] == 0 || outDgr[gk] == 0)continue;

				if (reachBegin3[threadId][gk][1] != begin)

				{

					reachBegin3[threadId][gk][0] = subLen3;

					reachBegin3[threadId][gk][1] = begin;

					idMap3_2[threadId][subLen3][0] = 0;

					idMap3[threadId][subLen3++][0] = 0;

				}


				ind = reachBegin3[threadId][gk][0];

				idMap3[threadId][ind][++idMap3[threadId][ind][0]] = gj;

				idMap3_2[threadId][ind][++idMap3_2[threadId][ind][0]] = gt;

			}

		}

	}

}

void searchEdge::findRing(int threadId, unsigned int beginIndex, unsigned int endIndex)

{

	for (auto i = beginIndex; i < endIndex; ++i)

	{

		auto begin = firstId[i];             //只考虑起始点
		if (outDgr[begin] > 0 && inDgr[begin] > 0)

		{

			unsigned int path[7];

			storeGra3(begin, threadId);

			dfs_circle(begin, path, visit[threadId], threadId);

		}
	}

}

void searchEdge::dfs_circle(unsigned int begin, unsigned int* path, unsigned int* visit, int threadId) {
	uint32_t index3, index4, index5, index6, index7, index8, tempPos = 0, find_index = 0, first_index = 0;
	path[0] = begin;
	visit[begin] = 1;
	if (reachBegin3[threadId][begin][1] == begin)
	{
		auto elem = idMap3[threadId][reachBegin3[threadId][begin][0]];
		for (int j = 1; j <= elem[0]; ++j)
		{
			unsigned int next1 = elem[j];
			unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][begin][0]][j];
			if (visit[next1] == 1 || visit[next2] == 1) continue;
			path[1] = next1;          //gj
			path[2] = next2;          //gt
			memcpy(result0[threadId][cirArr[threadId][0]], path, 16);
			++cirArr[threadId][0];
		}
	}
	for (uint32_t j = 0; j < outDgr[begin]; j++) {
		index3 = Gra[begin][j];
		if (begin >= index3)continue;
		path[1] = index3;
		visit[index3] = 1;
		if (reachBegin3[threadId][index3][1] == begin)
		{
			auto elem = idMap3[threadId][reachBegin3[threadId][index3][0]];
			for (int j = 1; j <= elem[0]; ++j)
			{
				unsigned int next1 = elem[j];
				unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][index3][0]][j];
				if (visit[next1] == 1 || visit[next2] == 1) continue;
				path[2] = next1;          //gj
				path[3] = next2;          //gt
				memcpy(result1[threadId][cirArr[threadId][1]], path, 16);
				++cirArr[threadId][1];
			}
		}
		for (uint32_t k = 0; k < outDgr[index3]; k++) {
			index4 = Gra[index3][k];
			if (begin >= index4 || index4 == index3)continue;
			path[2] = index4;
			visit[index4] = 1;
			if (reachBegin3[threadId][index4][1] == begin)
			{
				auto elem = idMap3[threadId][reachBegin3[threadId][index4][0]];
				for (int j = 1; j <= elem[0]; ++j)
				{
					unsigned int next1 = elem[j];
					unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][index4][0]][j];
					if (visit[next1] == 1 || visit[next2] == 1) continue;
					path[3] = next1;          //gj
					path[4] = next2;          //gt
					memcpy(result2[threadId][cirArr[threadId][2]], path, 16);
					memcpy(result2[threadId][cirArr[threadId][2]] + 4, path + 4, 16);
					++cirArr[threadId][2];
				}
			}
			for (uint32_t m = 0; m < outDgr[index4]; m++) {
				index5 = Gra[index4][m];
				if (begin >= index5 || index5 == index3 || index5 == index4)continue;
				path[3] = index5;
				visit[index5] = 1;
				if (reachBegin3[threadId][index5][1] == begin)
				{
					auto elem = idMap3[threadId][reachBegin3[threadId][index5][0]];
					for (int j = 1; j <= elem[0]; ++j)
					{
						unsigned int next1 = elem[j];
						unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][index5][0]][j];
						if (visit[next1] == 1 || visit[next2] == 1) continue;
						path[4] = next1;          //gj
						path[5] = next2;          //gt
						memcpy(result3[threadId][cirArr[threadId][3]], path, 16);
						memcpy(result3[threadId][cirArr[threadId][3]] + 4, path + 4, 16);
						++cirArr[threadId][3];
					}
				}
				for (uint32_t n = 0; n < outDgr[index5]; n++) {
					index6 = Gra[index5][n];
					if (begin >= index6 || index6 == index3 || index6 == index4 || index6 == index5)continue;
					path[4] = index6;
					visit[index6] = 1;
					if (reachBegin3[threadId][index6][1] == begin)
					{
						auto elem = idMap3[threadId][reachBegin3[threadId][index6][0]];
						for (int j = 1; j <= elem[0]; ++j)
						{
							unsigned int next1 = elem[j];
							unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][index6][0]][j];
							if (visit[next1] == 1 || visit[next2] == 1) continue;
							path[5] = next1;          //gj
							path[6] = next2;          //gt
							memcpy(result4[threadId][cirArr[threadId][4]], path, 16);
							memcpy(result4[threadId][cirArr[threadId][4]] + 4, path + 4, 16);
							++cirArr[threadId][4];
						}
					}
					visit[index6] = 0;
				}
				visit[index5] = 0;
			}
			visit[index4] = 0;
		}
		visit[index3] = 0;
	}
	visit[begin] = 0;
}

void searchEdge::multiSearch()
{
	thread thread1(bind(&searchEdge::findRing, this, 0, 0, (unsigned int)(0.06 * idCnt)));
	thread thread2(bind(&searchEdge::findRing, this, 1, (unsigned int)(idCnt * 0.06), (unsigned int)(idCnt * 0.13)));
	thread thread3(bind(&searchEdge::findRing, this, 2, (unsigned int)(idCnt * 0.13), (unsigned int)(idCnt * 0.27)));
	//thread thread4(bind(&searchEdge::findRing, this, 3, (unsigned int)(idCnt * 0.25), (unsigned int)idCnt));
	findRing(3, (unsigned int)(idCnt * 0.27), (unsigned int)idCnt);
	thread1.join();
	thread2.join();
	thread3.join();
}

void searchEdge::storeResult() {
	//cout << "he" << endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 5; j++) {
			ringCnt += cirArr[i][j];
		}
	}
	unsigned int strLen = 0;
	int fLen = 77 * (ringCnt + 1);
	char stbuf[20];
	strLen = uintToChar(ringCnt, stbuf + 1);
	stbuf[++strLen] = '\n';
	stbuf[0] = (char)strLen;

	FILE* out = fopen(resFile_.c_str(), "wb");
	char* buf = (char*)malloc(fLen * sizeof(char));
	char* p = buf;
	memcpy(buf, stbuf + 1, 16);             //一次读入16位，cacheline对齐                   
	buf += strLen;
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][0]; ++k)
		{
			auto curcnt = result0[t][k];
			for (int j = 0; j < 3; ++j)
			{
				char* str = outStr[curcnt[j]];
				strLen = str[0];
				memcpy(buf, str + 1, 16);
				buf += strLen;
			}
			*(buf - 1) = '\n';
		}
	}
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][1]; ++k)
		{
			auto curcnt = result1[t][k];
			for (int j = 0; j < 4; ++j)
			{
				char* str = outStr[curcnt[j]];
				strLen = str[0];
				memcpy(buf, str + 1, 16);
				buf += strLen;
			}
			*(buf - 1) = '\n';
		}
	}
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][2]; ++k)
		{
			auto curcnt = result2[t][k];
			for (int j = 0; j < 5; ++j)
			{
				char* str = outStr[curcnt[j]];
				strLen = str[0];
				memcpy(buf, str + 1, 16);
				buf += strLen;
			}
			*(buf - 1) = '\n';
		}
	}
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][3]; ++k)
		{
			auto curcnt = result3[t][k];
			for (int j = 0; j < 6; ++j)
			{
				char* str = outStr[curcnt[j]];
				strLen = str[0];
				memcpy(buf, str + 1, 16);
				buf += strLen;
			}
			*(buf - 1) = '\n';
		}
	}
	for (int t = 0; t < 4; ++t)
	{
		for (int k = 0; k < cirArr[t][4]; ++k)
		{
			auto curcnt = result4[t][k];
			for (int j = 0; j < 7; ++j)
			{
				char* str = outStr[curcnt[j]];
				strLen = str[0];
				memcpy(buf, str + 1, 16);
				buf += strLen;
			}
			*(buf - 1) = '\n';
		}
	}
	fwrite(p, buf - p, sizeof(char), out);
	fclose(out);
}

int main()
{
	cout << "ready to work" << endl;
	string testFile = "data/test_data1.txt";

	string resultFile = "data/result.txt";
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
