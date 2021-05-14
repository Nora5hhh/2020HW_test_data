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
#include<unordered_map>
#define ZONE_COUNT 500
using namespace std;
const int node = 2000000;

struct pair_hash
{
	template<class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2>& p) const
	{
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);
		return h1 ^ h2;
	}
};

const unsigned int sizeGra[10] = { 9,99,999,9999,99999,999999,9999999,99999999 ,999999999,0x7fffffff };
unsigned int cirArr[4][5];

//char result1[4][36000000], result2[4][50000000], result3[4][60000000], result4[4][70000000], result5[4][90000000];
//unsigned int Gra[node][50];
//unsigned int revGra[node][50];
vector<vector<unsigned int>> Gra(node);
vector<vector<unsigned int>> revGra(node);
unsigned int nodData[2 * node];

unsigned int inDgr[node];
unsigned int outDgr[node];
char outStr[node][12];
char lstStr[node][12];
unsigned int visit[4][node];
unsigned int que[node];
unsigned int pos[node];
unsigned int moneyData[node];
unsigned int reachBegin2[4][node][2];
unsigned int idMap[4][250][250];
unsigned int idMap3[4][5000][500];
unsigned int idMap3_2[4][5000][500];
unsigned int reachBegin1[4][node];
unsigned int reachBegin3[4][node][2];
unsigned int idPoint2[4][250];
unordered_map<unsigned int, unsigned int> id2Ind;
unordered_map<pair<unsigned int, unsigned int>, unsigned int, pair_hash> id2Money;
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
	char address[5][100 * 1024 * 1024];
	unsigned int len[5];
}*zonetoThread[ZONE_COUNT];

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
	unsigned int cnt;
	unsigned int nodeLen;
	unsigned int subLen;
	unsigned int ringCnt;
	int curNum;
	unsigned int processedId = 0;
	unsigned int zoneSize;
	char* resultPosition[ZONE_COUNT][5];

	void init();
	void storeGra3(unsigned int begin, int threadId);
	void findRing(int threadId);
	void dfs_circle(unsigned int begin, unsigned int * path, unsigned int * visit, int threadId, int zonId);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	ringCnt = 0;
	curNum = 0;
	subLen = 0;
	firstIdCnt = 0;
	idCnt = 0;
	cnt = 0;
	nodeLen = 0;
	init();
}

void searchEdge::init() {
	int u = 0, v = 0, w = 0;
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
		nodData[cnt] = curNum;
		++cnt;
		curNum = 0;
		p = q + 1;
		q = p;
		while (*q != ',') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
		}
		nodData[cnt] = curNum;
		++cnt;
		curNum = 0;
		//寻找换行位置
		p = q + 1;
		q = p;
		while (*q != '\n') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
		}
		moneyData[cnt / 2 - 1] = curNum;             //将money存储进来
		curNum = 0;
		p = q + 1;
	}
	for (int i = 1; i <= cnt; i += 4)
	{
		memcpy(pos + i, nodData + (i - 1), 16);
	}
	sort(pos + 1, pos + 1 + cnt);
	nodeLen = unique(pos + 1, pos + 1 + cnt) - pos - 1;
	Gra.resize(nodeLen);
	revGra.resize(nodeLen);
	for (int i = 1; i <= nodeLen; ++i)
	{
		id2Ind[pos[i]] = i;
		subLen = uintToChar(pos[i], outStr[i] + 1);
		outStr[i][++subLen] = ',';
		outStr[i][0] = (char)subLen;
		subLen = uintToChar(pos[i], lstStr[i] + 1);
		lstStr[i][++subLen] = '\n';
		lstStr[i][0] = (char)subLen;
	}
	for (int i = 0; i < cnt; ++i)             //构建正向图与反向图
	{
		u = id2Ind[nodData[i]];
		++i;
		v = id2Ind[nodData[i]];
		Gra[u].emplace_back(v);
		revGra[v].emplace_back(u);
		w = moneyData[i / 2];
		id2Money[make_pair(u, v)] = w;
		++outDgr[u];
		++inDgr[v];
	}
	zoneSize = nodeLen / ZONE_COUNT;
}

void searchEdge::remSin()
{
	unsigned int l = 0, h = 0;
	unsigned int u = 0, v = 0;
	for (int i = 1; i <= nodeLen; i++)
	{
		auto elem = i;      //只对起始点判断出度入度
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
	for (int i = 1; i <= nodeLen; ++i)
	{
		auto elem = i;
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
	for (int i = 1; i <= nodeLen; ++i)
	{
		auto elem = i;
		if (inDgr[elem] == 0 || outDgr[elem] == 0)
		{
		}
		else
		{
			sort(revGra[elem].begin(), revGra[elem].end());
			sort(Gra[elem].begin(), Gra[elem].end());
		}
	}
}

void searchEdge::storeGra3(unsigned int begin, int threadId) {
	auto subLen = 0, subLen3 = 0, pointLen = 0;
	unsigned int ind = 0;
	double subMoney = 0.0;
	for (int i = 0; i < inDgr[begin]; i++) {
		auto gi = revGra[begin][i];
		if (outDgr[gi] == 0 || inDgr[gi] == 0 || gi <= begin) continue;
		for (int k = 0; k < inDgr[gi]; ++k)
		{
			unsigned int gk = revGra[gi][k];
			if (outDgr[gk] == 0 || inDgr[gk] == 0 || gk <= begin) continue;
			subMoney = (double)id2Money[make_pair(gi, begin)] / (double)id2Money[make_pair(gk, gi)];
			if (subMoney > 3 || subMoney < 0.2) continue;
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
				subMoney = (double)id2Money[make_pair(gj, gt)] / (double)id2Money[make_pair(gk, gj)];
				if (subMoney > 3 || subMoney < 0.2) continue;
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

void searchEdge::findRing(int threadId)
{
	while (true) {
		mtx.lock();
		unsigned int zoneId = processedId++;         //改为按片分配，zoneId是独一无二的
		mtx.unlock();
		if (zoneId >= ZONE_COUNT)return;
		unsigned int i, startId = zoneId * zoneSize, endId;
		if (zoneId == ZONE_COUNT - 1)
			endId = nodeLen + 1;
		else
			endId = (zoneId + 1) * zoneSize;
		zonetoThread[zoneId] = (zoneInfo*)malloc(sizeof(zoneInfo));
		for (i = 0; i < 5; i++)
			resultPosition[zoneId][i] = zonetoThread[zoneId]->address[i];
		for (auto i = startId; i < endId; ++i)
		{
			auto begin = i;             //只考虑起始点
			if (outDgr[begin] > 0 && inDgr[begin] > 0)
			{
				unsigned int path[7];
				storeGra3(begin, threadId);
				dfs_circle(begin, path, visit[threadId], threadId, zoneId);
			}
		}
		for (i = 0; i < 5; i++) {
			zonetoThread[zoneId]->len[i] = resultPosition[zoneId][i] - zonetoThread[zoneId]->address[i];
		}
	}
}

void searchEdge::dfs_circle(unsigned int begin, unsigned int* path, unsigned int* visit, int threadId,int zonId) {
	uint32_t index3, index4, index5, index6, index7, index8, tempPos = 0, find_index = 0, first_index = 0, strLen = 0;;
	double subMoney1, subMoney2, subMoney3, subMoney4, subMoney5;
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
			subMoney1 = (double)id2Money[make_pair(begin, next1)] / (double)id2Money[make_pair(next2, begin)];
			if (subMoney1 > 3 || subMoney1 < 0.2) continue;
			memcpy(resultPosition[zonId][0], outStr[path[0]] + 1, 16);
			strLen = outStr[path[0]][0];
			resultPosition[zonId][0] += strLen;
			memcpy(resultPosition[zonId][0], outStr[next1] + 1, 16);
			strLen = outStr[next1][0];
			resultPosition[zonId][0] += strLen;
			memcpy(resultPosition[zonId][0], lstStr[next2] + 1, 16);
			strLen = lstStr[next2][0];
			resultPosition[zonId][0] += strLen;
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

				subMoney1 = (double)id2Money[make_pair(begin, index3)] / (double)id2Money[make_pair(next2, begin)];
				subMoney2 = (double)id2Money[make_pair(index3, next1)] / (double)id2Money[make_pair(begin, index3)];
				if (subMoney1 > 3 || subMoney2 > 3 || subMoney1 < 0.2 || subMoney2 < 0.2) continue;
				for (int i = 0; i < 2; i++) {
					memcpy(resultPosition[zonId][1], outStr[path[i]] + 1, 16);
					strLen = outStr[path[i]][0];
					resultPosition[zonId][1] += strLen;
				}
				memcpy(resultPosition[zonId][1], outStr[next1] + 1, 16);
				strLen = outStr[next1][0];
				resultPosition[zonId][1] += strLen;
				memcpy(resultPosition[zonId][1], lstStr[next2] + 1, 16);
				strLen = lstStr[next2][0];
				resultPosition[zonId][1] += strLen;
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
					subMoney1 = (double)id2Money[make_pair(index3, index4)] / (double)id2Money[make_pair(begin, index3)];
					subMoney2 = (double)id2Money[make_pair(index4, next1)] / (double)id2Money[make_pair(index3, index4)];
					subMoney3 = (double)id2Money[make_pair(begin, index3)] / (double)id2Money[make_pair(next2, begin)];
					if (subMoney1 > 3 || subMoney1 < 0.2 || subMoney2 > 3 || subMoney2 < 0.2 || subMoney3 > 3 || subMoney3 < 0.2) continue;
					for (int i = 0; i < 3; i++) {
						memcpy(resultPosition[zonId][2], outStr[path[i]] + 1, 16);
						strLen = outStr[path[i]][0];
						resultPosition[zonId][2] += strLen;
					}
					memcpy(resultPosition[zonId][2], outStr[next1] + 1, 16);
					strLen = outStr[next1][0];
					resultPosition[zonId][2] += strLen;
					memcpy(resultPosition[zonId][2], lstStr[next2] + 1, 16);
					strLen = lstStr[next2][0];
					resultPosition[zonId][2] += strLen;
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
						subMoney1 = (double)id2Money[make_pair(begin, index3)] / (double)id2Money[make_pair(next2, begin)];
						if (subMoney1 > 3 || subMoney1 < 0.2) continue;
						subMoney2 = (double)id2Money[make_pair(index3, index4)] / (double)id2Money[make_pair(begin, index3)];
						if (subMoney2 > 3 || subMoney2 < 0.2) continue;
						subMoney3 = (double)id2Money[make_pair(index5, next1)] / (double)id2Money[make_pair(index4, index5)];
						if (subMoney3 > 3 || subMoney3 < 0.2) continue;
						subMoney4 = (double)id2Money[make_pair(index4, index5)] / (double)id2Money[make_pair(index3, index4)];
						if (subMoney4 > 3 || subMoney4 < 0.2) continue;
						for (int i = 0; i < 4; i++) {
							memcpy(resultPosition[zonId][3], outStr[path[i]] + 1, 16);
							strLen = outStr[path[i]][0];
							resultPosition[zonId][3] += strLen;
						}
						memcpy(resultPosition[zonId][3], outStr[next1] + 1, 16);
						strLen = outStr[next1][0];
						resultPosition[zonId][3] += strLen;
						memcpy(resultPosition[zonId][3], lstStr[next2] + 1, 16);
						strLen = lstStr[next2][0];
						resultPosition[zonId][3] += strLen;
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
							subMoney1 = (double)id2Money[make_pair(begin, index3)] / (double)id2Money[make_pair(next2, begin)];
							if (subMoney1 > 3 || subMoney1 < 0.2) continue;
							subMoney2 = (double)id2Money[make_pair(index3, index4)] / (double)id2Money[make_pair(begin, index3)];
							if (subMoney2 > 3 || subMoney2 < 0.2) continue;
							subMoney3 = (double)id2Money[make_pair(index4, index5)] / (double)id2Money[make_pair(index3, index4)];
							if (subMoney3 > 3 || subMoney3 < 0.2) continue;
							subMoney4 = (double)id2Money[make_pair(index5, index6)] / (double)id2Money[make_pair(index4, index5)];
							if (subMoney4 > 3 || subMoney4 < 0.2) continue;
							subMoney5 = (double)id2Money[make_pair(index6, next1)] / (double)id2Money[make_pair(index5, index6)];
							if (subMoney5 > 3 || subMoney5 < 0.2) continue;
							for (int i = 0; i < 5; i++) {
								memcpy(resultPosition[zonId][4], outStr[path[i]] + 1, 16);
								strLen = outStr[path[i]][0];
								resultPosition[zonId][4] += strLen;
							}
							memcpy(resultPosition[zonId][4], outStr[next1] + 1, 16);
							strLen = outStr[next1][0];
							resultPosition[zonId][4] += strLen;
							memcpy(resultPosition[zonId][4], lstStr[next2] + 1, 16);
							strLen = lstStr[next2][0];
							resultPosition[zonId][4] += strLen;
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
			fwrite(zonetoThread[zone]->address[i], zonetoThread[zone]->len[i], sizeof(char), out);
		}
	}
	fclose(out);
}

int main()
{
	cout << "ready to work" << endl;
	string testFile = "test_data1.txt";
	string resultFile = "/home/yinjie/result.txt";
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
