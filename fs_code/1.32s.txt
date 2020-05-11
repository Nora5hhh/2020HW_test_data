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
#include<atomic>
#include<unordered_map>
#define ZONE_COUNT 500
using namespace std;
const int node = 2000000;

const int threadCount = 4;
struct subNode {
	unsigned int sub;
	unsigned int money;
};
bool cmp(subNode& a, subNode& b) {
	return a.sub < b.sub;
}
const unsigned int sizeGra[10] = { 9,99,999,9999,99999,999999,9999999,99999999 ,999999999,0x7fffffff };
unsigned int cirArr[threadCount][5];

vector<vector<subNode>> Gra(node);
vector<vector<subNode>> revGra(node);
unsigned int nodData[2 * node];
unsigned int inDgr[node];
unsigned int outDgr[node];
char outStr[node][12];
char lstStr[node][12];
unsigned int visit[threadCount][node];
unsigned int que[node];
unsigned int pos[2 * node + 1];
unsigned int moneyData[node];
unsigned int reachBegin2[threadCount][node][2];
unsigned int idMoney[threadCount][5000][500];
subNode  idMap[threadCount][5000][500];
subNode  idMap3[threadCount][15000][500][2];
unsigned int reachBegin1[threadCount][node];
unsigned int reachBegin3[threadCount][node][2];
unsigned int idPoint2[threadCount][node];
unordered_map<unsigned int, unsigned int> id2Ind;

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
	unsigned int cnt;
	unsigned int moneyCnt;
	unsigned int nodeLen;
	unsigned int subLen;
	unsigned int ringCnt;
	int curNum;
	atomic<unsigned int> processedId;
	unsigned int zoneSize;
	char* resultPosition[ZONE_COUNT][5];
	void init();
	void storeGra3(unsigned int begin, int threadId);
	void findRing(int threadId);
	void dfs_circle(unsigned int begin, unsigned int* path, unsigned int* visit, int threadId, int zonId);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	ringCnt = 0;
	curNum = 0;
	subLen = 0;
	processedId = 0;
	cnt = 0;
	moneyCnt = 0;
	nodeLen = 0;
	init();
}

void searchEdge::init() {
	int u = 0, v = 0, w = 0;
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
		while (*q != '\r' && *q != '\n') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
		}
		moneyData[moneyCnt++] = curNum;             //将money存储进来
		curNum = 0;
		while (*q != '\n') {
			++q;
		}
		p = q + 1;
	}
	for (int i = 1; i <= cnt; i += 4)
	{
		memcpy(pos + i, nodData + (i - 1), 16);
	}
	sort(pos + 1, pos + 1 + cnt);
	nodeLen = unique(pos + 1, pos + 1 + cnt) - pos - 1;
	id2Ind.rehash(1024 * 1024);
	Gra.resize(nodeLen + 1);
	revGra.resize(nodeLen + 1);
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
	for (int i = 0, j = 0; i < cnt; ++i, ++j)             //构建正向图与反向图
	{
		u = id2Ind[nodData[i]];
		++i;
		v = id2Ind[nodData[i]];
		w = moneyData[j];

		subNode elem;
		elem.sub = v;
		elem.money = w;
		Gra[u].emplace_back(elem);
		elem.sub = u;
		revGra[v].emplace_back(elem);
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
			v = Gra[u][i].sub;
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
			u = revGra[v][i].sub;
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
			sort(revGra[elem].begin(), revGra[elem].end(), cmp);
			sort(Gra[elem].begin(), Gra[elem].end(), cmp);
		}
	}
}

void searchEdge::storeGra3(unsigned int begin, int threadId) {
	auto subLen = 0, subLen3 = 0, pointLen = 0;
	unsigned int ind = 0;
	long long pre_money, pre_money2;
	for (int i = 0; i < inDgr[begin]; i++) {
		auto gi = revGra[begin][i].sub;
		if (outDgr[gi] == 0 || inDgr[gi] == 0 || gi <= begin) continue;
		pre_money = revGra[begin][i].money;
		for (int k = 0; k < inDgr[gi]; ++k)
		{
			unsigned int gk = revGra[gi][k].sub;         //gk->gi->begin
			if (outDgr[gk] == 0 || inDgr[gk] == 0 || gk <= begin) continue;
			pre_money2 = revGra[gi][k].money;
			if (pre_money > pre_money2 * 3 || pre_money * 10 < pre_money2 * 2)continue;
			if (reachBegin2[threadId][gk][1] != begin)
			{
				reachBegin2[threadId][gk][0] = subLen;//数量
				reachBegin2[threadId][gk][1] = begin; //终止点
				idMoney[threadId][subLen][0] = 0;
				idMap[threadId][subLen++][0].sub = 0; //数量
				idPoint2[threadId][pointLen++] = gk; //子节点
			}
			ind = reachBegin2[threadId][gk][0];
			idMap[threadId][ind][++idMap[threadId][ind][0].sub].sub = gi;
			idMap[threadId][ind][idMap[threadId][ind][0].sub].money = pre_money2;//gk到gi的money
			idMoney[threadId][ind][++idMoney[threadId][ind][0]] = pre_money;
		}
	}
	sort(idPoint2[threadId], idPoint2[threadId] + pointLen);
	for (int j = 0; j < pointLen; ++j)
	{
		unsigned int gj = idPoint2[threadId][j];            //倒数第二个指向begin的
		subLen = reachBegin2[threadId][gj][0];
		for (int t = 1; t <= idMap[threadId][subLen][0].sub; ++t)
		{
			unsigned int gt = idMap[threadId][subLen][t].sub;       //gj->gt->begin
			pre_money2 = idMap[threadId][subLen][t].money;        //gj到gt的money
			pre_money = idMoney[threadId][subLen][t];         //gt到begin的money
			for (int k = 0; k < inDgr[gj]; ++k)
			{
				unsigned int gk = revGra[gj][k].sub;        //gk->gj->gt->begin
				if (gk < begin || gk == gt || inDgr[gk] == 0 || outDgr[gk] == 0)continue;
				if (pre_money2 > (long long)revGra[gj][k].money * 3 || (long long)pre_money2 * 10 < 2 * revGra[gj][k].money)continue;
				if (reachBegin3[threadId][gk][1] != begin)
				{
					reachBegin3[threadId][gk][0] = subLen3;
					reachBegin3[threadId][gk][1] = begin;
					idMap3[threadId][subLen3++][0][0].sub = 0;
				}
				ind = reachBegin3[threadId][gk][0];
				idMap3[threadId][ind][++idMap3[threadId][ind][0][0].sub][0].sub = gj;
				idMap3[threadId][ind][idMap3[threadId][ind][0][0].sub][0].money = revGra[gj][k].money;  //gk->gj的money
				idMap3[threadId][ind][idMap3[threadId][ind][0][0].sub][1].sub = gt;
				idMap3[threadId][ind][idMap3[threadId][ind][0][0].sub][1].money = pre_money;       //gt->begin的money
			}
		}
	}

}

void searchEdge::findRing(int threadId)
{
	while (true) {
		unsigned int zoneId = processedId++;         //改为按片分配，zoneId是独一无二的
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
			auto begin = i;
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

void searchEdge::dfs_circle(unsigned int begin, unsigned int* path, unsigned int* visit, int threadId, int zonId) {
	uint32_t index3, index4, index5, index6, index7, index8, tempPos = 0, find_index = 0, first_index = 0, strLen = 0;
	long long pre_money1, pre_money2, pre_money3, pre_money4, pre_money5;
	long long money1, money2, money3, money4, money5, money6, money7;
	path[0] = begin;
	visit[begin] = 1;
	if (reachBegin3[threadId][begin][1] == begin)
	{
		auto elem = idMap3[threadId][reachBegin3[threadId][begin][0]];
		for (int j = 1; j <= elem[0][0].sub; ++j)
		{
			unsigned int next1 = elem[j][0].sub;
			unsigned int next2 = elem[j][1].sub;
			if (visit[next1] == 1 || visit[next2] == 1) continue;
			money1 = elem[j][1].money;
			money2 = elem[j][0].money;
			pre_money1 = money2 * 10 / money1;
			if (money2 > money1 * 3 || pre_money1 < 2)continue;
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
		index3 = Gra[begin][j].sub;
		money3 = Gra[begin][j].money;
		if (begin >= index3)continue;
		path[1] = index3;
		visit[index3] = 1;
		if (reachBegin3[threadId][index3][1] == begin)
		{
			auto elem = idMap3[threadId][reachBegin3[threadId][index3][0]];
			for (int j = 1; j <= elem[0][0].sub; ++j)
			{
				unsigned int next1 = elem[j][0].sub;
				unsigned int next2 = elem[j][1].sub;
				if (visit[next1] == 1 || visit[next2] == 1) continue;
				money1 = elem[j][1].money;
				if ((long long)elem[j][0].money > money3 * 3 || (long long)elem[j][0].money * 10 < 2 * money3 || money3 > money1 * 3 || money3 * 10 < 2 * money1)continue;
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
			index4 = Gra[index3][k].sub;
			if (begin >= index4 || index4 == index3)continue;
			money4 = Gra[index3][k].money;
			if (money4 > 3 * money3 || money4 * 10 < 2 * money3)continue;
			path[2] = index4;
			visit[index4] = 1;
			if (reachBegin3[threadId][index4][1] == begin)
			{
				auto elem = idMap3[threadId][reachBegin3[threadId][index4][0]];
				for (int j = 1; j <= elem[0][0].sub; ++j)
				{
					unsigned int next1 = elem[j][0].sub;
					unsigned int next2 = elem[j][1].sub;
					if (visit[next1] == 1 || visit[next2] == 1) continue;
					money1 = elem[j][1].money;
					money2 = elem[j][0].money;
					if (money2 > 3 * money4 || money2 * 10 < 2 * money4 || money3 > money1 * 3 || money3 * 10 < 2 * money1)continue;
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
				index5 = Gra[index4][m].sub;
				if (begin >= index5 || index5 == index3 || index5 == index4)continue;
				money5 = Gra[index4][m].money;
				if (money5 > money4 * 3 || money5 * 10 < 2 * money4)continue;
				path[3] = index5;
				visit[index5] = 1;
				if (reachBegin3[threadId][index5][1] == begin)
				{
					auto elem = idMap3[threadId][reachBegin3[threadId][index5][0]];
					for (int j = 1; j <= elem[0][0].sub; ++j)
					{
						unsigned int next1 = elem[j][0].sub;
						unsigned int next2 = elem[j][1].sub;
						money1 = elem[j][1].money;
						money2 = elem[j][0].money;
						if (money2 > money5 * 3 || money2 * 10 < 2 * money5 || money3 > money1 * 3 || money3 * 10 < 2 * money1)continue;
						if (visit[next1] == 1 || visit[next2] == 1) continue;
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
					index6 = Gra[index5][n].sub;
					if (begin >= index6 || index6 == index3 || index6 == index4 || index6 == index5)continue;
					money6 = Gra[index5][n].money;
					if (money6 > money5 * 3 || money6 * 10 < 2 * money5)continue;
					path[4] = index6;
					visit[index6] = 1;
					if (reachBegin3[threadId][index6][1] == begin)
					{
						auto elem = idMap3[threadId][reachBegin3[threadId][index6][0]];
						for (int j = 1; j <= elem[0][0].sub; ++j)
						{
							unsigned int next1 = elem[j][0].sub;
							unsigned int next2 = elem[j][1].sub;
							money1 = elem[j][1].money;
							money2 = elem[j][0].money;
							if (money2 > money6 * 3 || money2 * 10 < 2 * money6 || money3 > money1 * 3 || money3 * 10 < 2 * money1)continue;
							if (visit[next1] == 1 || visit[next2] == 1) continue;
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
	findRing(3);
	thread1.join();
	thread2.join();
	thread3.join();
}

void searchEdge::storeResult() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 5; j++) {
			ringCnt += cirArr[i][j];
		}
	}
	cout << ringCnt << endl;
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
	string testFile = "/data/test_data.txt";
	string resultFile = "/projects/student/result.txt";
	clock_t start, end;

	searchEdge my_searchMap(testFile, resultFile);
	my_searchMap.remSin();

	start = clock();
	cout << "find the circle account" << endl;
	my_searchMap.multiSearch();
	cout << "output result" << endl;
	end = clock();
	my_searchMap.storeResult();

	cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}
