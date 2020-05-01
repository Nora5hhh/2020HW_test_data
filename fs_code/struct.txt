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
#include<mutex>
#define ZONE_COUNT 500
struct subNode {
	unsigned int sub;
	unsigned int money;
};
bool cmp(subNode& a, subNode& b) {
	return a.sub < b.sub;
}
using namespace std;
const int node = 280000;
const int maxValue = 280000;

const unsigned int sizeGra[10] = { 9,99,999,9999,99999,999999,9999999,99999999 ,999999999,0x7fffffff };
unsigned int cirArr[4][5];
char result[4][5][57600000];
subNode Gra[node][50];
subNode revGra[node][50];
unsigned int inDgr[node];
unsigned int outDgr[node];
char outStr[node][7];
char lstStr[node][7];
unsigned int visit[4][node];
unsigned int que[node];
unsigned int pos[node];
unsigned int firstIdFlag[node];
unsigned int idFlag[node];
unsigned int firstId[node];
unsigned int reachBegin2[4][node][2];
unsigned int idMoney[4][250][250];
subNode idMap[4][250][250];
subNode idMap3[4][5000][500];
subNode idMap3_2[4][5000][500];
unsigned int reachBegin1[4][node];
unsigned int reachBegin3[4][node][2];
unsigned int idPoint2[4][250];
mutex mtx;
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
struct zoneInfo {
	char* address[5];
	unsigned int len[5];
}zonetoThread[ZONE_COUNT];


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
	unsigned int processedId = 0;
	unsigned int zoneSize;
	char* resultPosition[4][5];
	char* resultPositionLast[4][5];
	void init();
	void storeGra3(unsigned int begin, int threadId);
	void findRing(int threadId);
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
	unsigned int money;
	while (*p >= '0' && *p <= '9') {
		char* q = p;
		//寻找第一个逗号
		while (*q != ',') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
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
		p = q + 1;
		q = p;
		money = 0;
		while (*q != '\n' && *q != '\r') {
			money = money * 10 + (*q) - '0';
			++q;
		}
		while (*q != '\n') {
			++q;
		}
		p = q + 1;
		Gra[u][outDgr[u]].sub = ++curNum;
		Gra[u][outDgr[u]].money = money;
		revGra[curNum][inDgr[curNum]].sub = u;
		revGra[curNum][inDgr[curNum]].money = money;
		idFlag[curNum] = 1;
		++inDgr[curNum];
		++outDgr[u];

		curNum = 0;

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
	zoneSize = idCnt / ZONE_COUNT;
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
			v = Gra[u][i].sub;
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
			u = revGra[v][i].sub;
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
			sort(revGra[elem], revGra[elem] + inDgr[elem], cmp);
			sort(Gra[elem], Gra[elem] + outDgr[elem], cmp);

		}
	}
}

void searchEdge::storeGra3(unsigned int begin, int threadId) {
	auto subLen = 0, subLen3 = 0, pointLen = 0;
	unsigned int ind = 0;
	float pre_money, pre_money2;
	for (int i = 0; i < inDgr[begin]; i++) {
		auto gi = revGra[begin][i].sub;
		if (outDgr[gi] == 0 || inDgr[gi] == 0 || gi <= begin) continue;
		pre_money = revGra[begin][i].money;
		for (int k = 0; k < inDgr[gi]; ++k)
		{
			unsigned int gk = revGra[gi][k].sub;
			if (outDgr[gk] == 0 || inDgr[gk] == 0 || gk <= begin) continue;
			if (pre_money / revGra[gi][k].money > 3 || pre_money / revGra[gi][k].money < 0.2)continue;
			pre_money2 = revGra[gi][k].money;
			if (reachBegin2[threadId][gk][1] != begin)
			{
				reachBegin2[threadId][gk][0] = subLen;//数量
				reachBegin2[threadId][gk][1] = begin; //终止点
				idMap[threadId][subLen++][0].sub = 0; //数量
				idMoney[threadId][subLen][0] = 0;
				idPoint2[threadId][pointLen++] = gk; //子节点
			}
			ind = reachBegin2[threadId][gk][0];
			idMap[threadId][ind][++idMap[threadId][ind][0].sub].sub = gi;//中间节点
			idMap[threadId][ind][idMap[threadId][ind][0].sub].money = pre_money;//中间节点
			idMoney[threadId][ind][++idMoney[threadId][subLen][0]] = pre_money2;
		}
	}
	sort(idPoint2[threadId], idPoint2[threadId] + pointLen);
	for (int j = 0; j < pointLen; ++j)
	{
		unsigned int gj = idPoint2[threadId][j];            //倒数第二个指向begin的
		subLen = reachBegin2[threadId][gj][0];
		for (int t = 1; t <= idMap[threadId][subLen][0].sub; ++t)
		{
			unsigned int gt = idMap[threadId][subLen][t].sub;       //倒数第一个指向begin
			pre_money2 = idMap[threadId][subLen][t].money;
			pre_money = idMoney[threadId][subLen][t];
			for (int k = 0; k < inDgr[gj]; ++k)
			{
				unsigned int gk = revGra[gj][k].sub;
				if (gk < begin || gk == gt || inDgr[gk] == 0 || outDgr[gk] == 0)continue;
				if (pre_money / revGra[gj][k].money > 3 || pre_money / revGra[gj][k].money < 0.2)continue;
				if (reachBegin3[threadId][gk][1] != begin)
				{
					reachBegin3[threadId][gk][0] = subLen3;
					reachBegin3[threadId][gk][1] = begin;
					idMap3_2[threadId][subLen3][0].sub = 0;
					idMap3[threadId][subLen3++][0].sub = 0;
				}
				ind = reachBegin3[threadId][gk][0];
				idMap3[threadId][ind][++idMap3[threadId][ind][0].sub].sub = gj;
				idMap3[threadId][ind][idMap3[threadId][ind][0].sub].money = revGra[gj][k].money;
				idMap3_2[threadId][ind][++idMap3_2[threadId][ind][0].sub].sub = gt;
				idMap3_2[threadId][ind][idMap3_2[threadId][ind][0].sub].money = pre_money2;
			}
		}
	}
}

void searchEdge::findRing(int threadId)

{
	resultPositionLast[threadId][0] = result[threadId][0];
	resultPositionLast[threadId][1] = result[threadId][1];
	resultPositionLast[threadId][2] = result[threadId][2];
	resultPositionLast[threadId][3] = result[threadId][3];
	resultPositionLast[threadId][4] = result[threadId][4];
	while (true) {
		mtx.lock();
		unsigned int zoneId = processedId++;
		mtx.unlock();
		if (zoneId >= 500)return;
		unsigned int i, startId = zoneId * zoneSize, endId;
		if (zoneId == ZONE_COUNT - 1)
			endId = idCnt;
		else
			endId = (zoneId + 1) * zoneSize;
		for (i = 0; i < 5; i++)
			resultPosition[threadId][i] = resultPositionLast[threadId][i];

		for (auto i = startId; i < endId; ++i)

		{

			auto begin = firstId[i];             //只考虑起始点
			if (outDgr[begin] > 0 && inDgr[begin] > 0)

			{

				unsigned int path[7];

				storeGra3(begin, threadId);

				dfs_circle(begin, path, visit[threadId], threadId);

			}
		}
		for (i = 0; i < 5; i++) {
			zonetoThread[zoneId].address[i] = resultPositionLast[threadId][i];
			zonetoThread[zoneId].len[i] = resultPosition[threadId][i] - resultPositionLast[threadId][i];
			resultPositionLast[threadId][i] = resultPosition[threadId][i];
		}
	}


}

void searchEdge::dfs_circle(unsigned int begin, unsigned int* path, unsigned int* visit, int threadId) {
	uint32_t index3, index4, index5, index6, index7, index8, tempPos = 0, find_index = 0, first_index = 0, strLen = 0;
	float pre_money;
	path[0] = begin;
	visit[begin] = 1;
	if (reachBegin3[threadId][begin][1] == begin)
	{
		auto elem = idMap3[threadId][reachBegin3[threadId][begin][0]];
		for (int j = 1; j <= elem[0].sub; ++j)
		{

			if ((float)elem[j].money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money > 3 || (float)elem[j].money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money < 0.2)continue;
			unsigned int next1 = elem[j].sub;
			unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].sub;
			if (visit[next1] == 1 || visit[next2] == 1) continue;
			memcpy(resultPosition[threadId][0], outStr[path[0]] + 1, 16);
			strLen = outStr[path[0]][0];
			resultPosition[threadId][0] += strLen;
			memcpy(resultPosition[threadId][0], outStr[next1] + 1, 16);
			strLen = outStr[next1][0];
			resultPosition[threadId][0] += strLen;
			memcpy(resultPosition[threadId][0], lstStr[next2] + 1, 16);
			strLen = lstStr[next2][0];
			resultPosition[threadId][0] += strLen;
			++cirArr[threadId][0];
		}
	}
	for (uint32_t j = 0; j < outDgr[begin]; j++) {
		index3 = Gra[begin][j].sub;
		if (begin >= index3)continue;
		pre_money = Gra[begin][j].money;
		path[1] = index3;
		visit[index3] = 1;
		if (reachBegin3[threadId][index3][1] == begin)
		{
			auto elem = idMap3[threadId][reachBegin3[threadId][index3][0]];
			for (int j = 1; j <= elem[0].sub; ++j)
			{
				if (elem[j].money / pre_money > 3 || elem[j].money / pre_money < 0.2)continue;
				if (pre_money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money > 3 || pre_money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money < 0.2)continue;
				unsigned int next1 = elem[j].sub;
				unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][index3][0]][j].sub;
				if (visit[next1] == 1 || visit[next2] == 1) continue;
				for (int i = 0; i < 2; i++) {
					memcpy(resultPosition[threadId][1], outStr[path[i]] + 1, 16);
					strLen = outStr[path[i]][0];
					resultPosition[threadId][1] += strLen;
				}
				memcpy(resultPosition[threadId][1], outStr[next1] + 1, 16);
				strLen = outStr[next1][0];
				resultPosition[threadId][1] += strLen;
				memcpy(resultPosition[threadId][1], lstStr[next2] + 1, 16);
				strLen = lstStr[next2][0];
				resultPosition[threadId][1] += strLen;
				++cirArr[threadId][1];
			}
		}
		for (uint32_t k = 0; k < outDgr[index3]; k++) {
			index4 = Gra[index3][k].sub;
			if (begin >= index4 || index4 == index3)continue;
			if (Gra[index4][k].money / pre_money > 3 || Gra[index4][k].money / pre_money < 0.2)continue;
			pre_money = Gra[index3][k].money;
			path[2] = index4;
			visit[index4] = 1;
			if (reachBegin3[threadId][index4][1] == begin)
			{
				auto elem = idMap3[threadId][reachBegin3[threadId][index4][0]];
				for (int j = 1; j <= elem[0].sub; ++j)
				{
					if (elem[j].money / pre_money > 3 || elem[j].money / pre_money < 0.2)continue;
					if (pre_money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money > 3 || pre_money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money < 0.2)continue;
					unsigned int next1 = elem[j].sub;
					unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][index4][0]][j].sub;
					if (visit[next1] == 1 || visit[next2] == 1) continue;
					for (int i = 0; i < 3; i++) {
						memcpy(resultPosition[threadId][2], outStr[path[i]] + 1, 16);
						strLen = outStr[path[i]][0];
						resultPosition[threadId][2] += strLen;
					}
					memcpy(resultPosition[threadId][2], outStr[next1] + 1, 16);
					strLen = outStr[next1][0];
					resultPosition[threadId][2] += strLen;
					memcpy(resultPosition[threadId][2], lstStr[next2] + 1, 16);
					strLen = lstStr[next2][0];
					resultPosition[threadId][2] += strLen;
					++cirArr[threadId][2];
				}
			}
			for (uint32_t m = 0; m < outDgr[index4]; m++) {
				index5 = Gra[index4][m].sub;
				if (begin >= index5 || index5 == index3 || index5 == index4)continue;
				if (Gra[index5][k].money / pre_money > 3 || Gra[index5][k].money / pre_money < 0.2)continue;
				pre_money = Gra[begin][j].money;
				path[3] = index5;
				visit[index5] = 1;
				if (reachBegin3[threadId][index5][1] == begin)
				{
					auto elem = idMap3[threadId][reachBegin3[threadId][index5][0]];
					for (int j = 1; j <= elem[0].sub; ++j)
					{
						if (elem[j].money / pre_money > 3 || elem[j].money / pre_money < 0.2)continue;
						if (pre_money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money > 3 || pre_money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money < 0.2)continue;
						unsigned int next1 = elem[j].sub;
						unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][index5][0]][j].sub;
						if (visit[next1] == 1 || visit[next2] == 1) continue;
						for (int i = 0; i < 4; i++) {
							memcpy(resultPosition[threadId][3], outStr[path[i]] + 1, 16);
							strLen = outStr[path[i]][0];
							resultPosition[threadId][3] += strLen;
						}
						memcpy(resultPosition[threadId][3], outStr[next1] + 1, 16);
						strLen = outStr[next1][0];
						resultPosition[threadId][3] += strLen;
						memcpy(resultPosition[threadId][3], lstStr[next2] + 1, 16);
						strLen = lstStr[next2][0];
						resultPosition[threadId][3] += strLen;
						++cirArr[threadId][3];
					}
				}
				for (uint32_t n = 0; n < outDgr[index5]; n++) {
					index6 = Gra[index5][n].sub;
					if (begin >= index6 || index6 == index3 || index6 == index4 || index6 == index5)continue;
					path[4] = index6;
					if (Gra[index6][k].money / pre_money > 3 || Gra[index6][k].money / pre_money < 0.2)continue;
					pre_money = Gra[begin][j].money;
					visit[index6] = 1;
					if (reachBegin3[threadId][index6][1] == begin)
					{
						auto elem = idMap3[threadId][reachBegin3[threadId][index6][0]];
						for (int j = 1; j <= elem[0].sub; ++j)
						{
							if (elem[j].money / pre_money > 3 || elem[j].money / pre_money < 0.2)continue;
							if (pre_money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money > 3 || pre_money / idMap3_2[threadId][reachBegin3[threadId][begin][0]][j].money < 0.2)continue;
							unsigned int next1 = elem[j].sub;
							unsigned int next2 = idMap3_2[threadId][reachBegin3[threadId][index6][0]][j].sub;
							if (visit[next1] == 1 || visit[next2] == 1) continue;
							for (int i = 0; i < 5; i++) {
								memcpy(resultPosition[threadId][4], outStr[path[i]] + 1, 16);
								strLen = outStr[path[i]][0];
								resultPosition[threadId][4] += strLen;
							}
							memcpy(resultPosition[threadId][4], outStr[next1] + 1, 16);
							strLen = outStr[next1][0];
							resultPosition[threadId][4] += strLen;
							memcpy(resultPosition[threadId][4], lstStr[next2] + 1, 16);
							strLen = lstStr[next2][0];
							resultPosition[threadId][4] += strLen;
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
	thread thread1(bind(&searchEdge::findRing, this, 0));
	thread thread2(bind(&searchEdge::findRing, this, 1));
	thread thread3(bind(&searchEdge::findRing, this, 2));
	//thread thread3(bind(&searchEdge::findRing, this, 3));
	findRing(3);
	thread1.join();
	thread2.join();
	thread3.join();
	//thread4.join();
}

void searchEdge::storeResult() {
	//cout << "he" << endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 5; j++) {
			ringCnt += cirArr[i][j];
		}
	}
	unsigned int strLen = 0;
	char stbuf[20];
	strLen = uintToChar(ringCnt, stbuf);
	stbuf[strLen++] = '\n';

	FILE* out = fopen(resFile_.c_str(), "wb");
	fwrite(stbuf, strLen, sizeof(char), out);
	for (int i = 0; i < 5; i++) {
		for (unsigned int zone = 0; zone < ZONE_COUNT; ++zone) {
			fwrite(zonetoThread[zone].address[i], zonetoThread[zone].len[i], sizeof(char), out);
		}
	}
	fclose(out);
}

int main()
{
	cout << "ready to work" << endl;
	string testFile = "data/test_data.txt";
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
