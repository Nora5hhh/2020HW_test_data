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
const uint32_t sizeGra[10] = { 9,99,999,9999,99999,999999,9999999,99999999 ,999999999,0x7fffffff };
uint32_t cirArr[threadCount][6];
vector<vector<pair<uint32_t, uint64_t>>> Gra(node);
vector<vector<pair<uint32_t, uint64_t>>> revGra(node);
unsigned int nodData[2 * node];
unsigned int inDgr[node];
unsigned int outDgr[node];
char outStr[node][12];
char lstStr[node][12];
unsigned int visit[threadCount][node];
unsigned int que[node];
unsigned int pos[2 * node + 1];
long long moneyData[node];
unsigned int reachBegin2[threadCount][node][2];
long long idMoney[threadCount][5000][500];
pair<uint32_t, uint64_t> idMap[threadCount][5000][500];
pair<uint32_t, uint64_t> idMap3[threadCount][15000][500][2];
unsigned int reachBegin1[threadCount][node];
unsigned int reachBegin3[threadCount][node][2];
unsigned int idPoint2[threadCount][500];
unordered_map<unsigned int, unsigned int> id2Ind;

uint32_t uintToChar(uint32_t num, char* buf)
{
	uint32_t i = 0;
	for (;; ++i)
	{
		if (num <= sizeGra[i])
		{
			++i;
			break;
		}
	}
	uint32_t len = i;
	while (i)
	{
		buf[--i] = num % 10 + '0';
		num /= 10;
	}
	return len;
}
struct zoneInfo {
	char address[6][100 * 1024 * 1024];
	uint32_t len[6];
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
	uint32_t cnt;
	uint32_t nodeLen;
	uint32_t subLen;
	uint32_t ringCnt;
	long long curNum;
	atomic<uint32_t> processedId;
	uint32_t zoneSize;
	char* resultPosition[ZONE_COUNT][6];
	void init();
	void storeGra3(uint32_t begin, int threadId);
	void findRing(int threadId);
	void dfs_circle(uint32_t begin, uint32_t* path, uint32_t* visit, int threadId, int zonId);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	ringCnt = 0;
	curNum = 0;
	subLen = 0;
	firstIdCnt = 0;
	idCnt = 0;
	processedId = 0;
	cnt = 0;
	nodeLen = 0;
	init();
}

void searchEdge::init() {
	uint64_t u = 0, v = 0, w = 0;
	int tempNum = 0;
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
		while (*q != '.'&&*q != '\r'&&*q != '\n') {
			curNum = curNum * 10 + (*q) - '0';
			++q;
		}
		curNum *= 100;
		if (*q == '.')++q;
		int tempCount = 0;
		while (*q != '\r'&&*q != '\n') {
			tempNum = tempNum * 10 + (*q) - '0';
			tempCount++;
			++q;
		}
		if (tempCount == 1)
			tempNum *= 10;
		curNum += tempNum;
		tempNum = 0;
		moneyData[cnt / 2 - 1] = curNum;             //将money存储进来
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
	for (int i = 0; i < cnt; ++i)             //构建正向图与反向图
	{
		u = id2Ind[nodData[i]];
		++i;
		v = id2Ind[nodData[i]];
		w = moneyData[i / 2];


		pair<uint32_t, uint64_t> elem(v, w);
		Gra[u].emplace_back(elem);
		elem.first = u;

		revGra[v].emplace_back(elem);
		++outDgr[u];
		++inDgr[v];
	}
	zoneSize = nodeLen / ZONE_COUNT;
}

void searchEdge::remSin()
{
	uint32_t l = 0, h = 0;
	uint32_t u = 0, v = 0;
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
			v = Gra[u][i].first;
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
			u = revGra[v][i].first;
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

void searchEdge::storeGra3(uint32_t begin, int threadId) {
	auto subLen = 0, subLen3 = 0, pointLen = 0;
	uint32_t ind = 0;
	uint64_t pre_money, pre_money2;
	for (int i = 0; i < inDgr[begin]; i++) {
		auto gi = revGra[begin][i].first;
		if (outDgr[gi] == 0 || inDgr[gi] == 0 || gi <= begin) continue;
		pre_money = revGra[begin][i].second;
		for (int k = 0; k < inDgr[gi]; ++k)
		{
			uint32_t gk = revGra[gi][k].first;         //gk->gi->begin
			if (gk <= begin || outDgr[gk] == 0 || inDgr[gk] == 0) continue;
			pre_money2 = revGra[gi][k].second;
			if (pre_money > pre_money2 * 3 || pre_money * 5 < pre_money2)continue;
			if (reachBegin2[threadId][gk][1] != begin)
			{
				reachBegin2[threadId][gk][0] = subLen;//数量
				reachBegin2[threadId][gk][1] = begin; //终止点
				idMoney[threadId][subLen][0] = 0;
				idMap[threadId][subLen++][0].first = 0; //数量
				idPoint2[threadId][pointLen++] = gk; //子节点
			}
			ind = reachBegin2[threadId][gk][0];
			idMap[threadId][ind][++idMap[threadId][ind][0].first].first = gi;
			idMap[threadId][ind][idMap[threadId][ind][0].first].second = pre_money2;//gk到gi的money
			idMoney[threadId][ind][++idMoney[threadId][ind][0]] = pre_money;
		}
	}
	sort(idPoint2[threadId], idPoint2[threadId] + pointLen);
	for (int j = 0; j < pointLen; ++j)
	{
		uint32_t gj = idPoint2[threadId][j];            //倒数第二个指向begin的
		subLen = reachBegin2[threadId][gj][0];
		for (int t = 1; t <= idMap[threadId][subLen][0].first; ++t)
		{
			uint32_t gt = idMap[threadId][subLen][t].first;       //gj->gt->begin
			pre_money2 = idMap[threadId][subLen][t].second;        //gj到gt的money
			pre_money = idMoney[threadId][subLen][t];         //gt到begin的money
			for (int k = 0; k < inDgr[gj]; ++k)
			{
				uint32_t gk = revGra[gj][k].first;        //gk->gj->gt->begin
				if (gk < begin || gk == gt || inDgr[gk] == 0 || outDgr[gk] == 0)continue;
				if (pre_money2 > revGra[gj][k].second * 3 || pre_money2 * 5 < revGra[gj][k].second)continue;
				if (reachBegin3[threadId][gk][1] != begin)
				{
					reachBegin3[threadId][gk][0] = subLen3;
					reachBegin3[threadId][gk][1] = begin;
					idMap3[threadId][subLen3++][0][0].first = 0;
				}
				ind = reachBegin3[threadId][gk][0];
				idMap3[threadId][ind][++idMap3[threadId][ind][0][0].first][0].first = gj;
				idMap3[threadId][ind][idMap3[threadId][ind][0][0].first][0].second = revGra[gj][k].second;  //gk->gj的money
				idMap3[threadId][ind][idMap3[threadId][ind][0][0].first][1].first = gt;
				idMap3[threadId][ind][idMap3[threadId][ind][0][0].first][1].second = pre_money;       //gt->begin的money
			}
		}
	}

}

void searchEdge::findRing(int threadId)
{
	while (true) {
		uint32_t zoneId = processedId++;         //改为按片分配，zoneId是独一无二的
		if (zoneId >= ZONE_COUNT)return;
		uint32_t i, startId = zoneId * zoneSize, endId;
		if (zoneId == ZONE_COUNT - 1)
			endId = nodeLen + 1;
		else
			endId = (zoneId + 1) * zoneSize;
		zonetoThread[zoneId] = (zoneInfo*)malloc(sizeof(zoneInfo));
		for (i = 0; i < 6; i++)
			resultPosition[zoneId][i] = zonetoThread[zoneId]->address[i];
		for (auto i = startId; i < endId; ++i)
		{
			auto begin = i;
			if (outDgr[begin] > 0 && inDgr[begin] > 0)
			{
				uint32_t path[8];
				storeGra3(begin, threadId);
				dfs_circle(begin, path, visit[threadId], threadId, zoneId);
			}
		}
		for (i = 0; i < 6; i++) {
			zonetoThread[zoneId]->len[i] = resultPosition[zoneId][i] - zonetoThread[zoneId]->address[i];
		}
	}
}

void searchEdge::dfs_circle(uint32_t begin, uint32_t* path, uint32_t* visit, int threadId, int zonId) {
	uint32_t index3, index4, index5, index6, index7, index8, tempPos = 0, find_index = 0, first_index = 0, strLen = 0, next1, next2;
	uint64_t money1, money2, money3, money4, money5, money6, money7;
	path[0] = begin;
	visit[begin] = 1;
	if (reachBegin3[threadId][begin][1] == begin)
	{
		auto elem = idMap3[threadId][reachBegin3[threadId][begin][0]];
		for (int j = 1; j <= elem[0][0].first; ++j)
		{
			money1 = elem[j][1].second;
			money2 = elem[j][0].second;
			if (money2 > money1 * 3 || money2 * 5 < money1)continue;
			next1 = elem[j][0].first;
			next2 = elem[j][1].first;
			if (visit[next1] == 1 || visit[next2] == 1) continue;
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
		index3 = Gra[begin][j].first;
		money3 = Gra[begin][j].second;
		if (begin >= index3)continue;
		path[1] = index3;
		visit[index3] = 1;
		if (reachBegin3[threadId][index3][1] == begin)
		{
			auto elem = idMap3[threadId][reachBegin3[threadId][index3][0]];
			for (int j = 1; j <= elem[0][0].first; ++j)
			{
				money1 = elem[j][1].second;
				if (elem[j][0].second > money3 * 3 || elem[j][0].second * 5 < money3 || money3 > money1 * 3 || money3 * 5 < money1)continue;
				next1 = elem[j][0].first;
				next2 = elem[j][1].first;
				if (visit[next1] == 1 || visit[next2] == 1) continue;
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
			index4 = Gra[index3][k].first;
			money4 = Gra[index3][k].second;
			if (money4 > 3 * money3 || money4 * 5 < money3)continue;
			if (begin >= index4 || index4 == index3)continue;
			path[2] = index4;
			visit[index4] = 1;
			if (reachBegin3[threadId][index4][1] == begin)
			{
				auto elem = idMap3[threadId][reachBegin3[threadId][index4][0]];
				for (int j = 1; j <= elem[0][0].first; ++j)
				{
					money1 = elem[j][1].second;
					money2 = elem[j][0].second;
					if (money2 > 3 * money4 || money2 * 5 < money4 || money3 > money1 * 3 || money3 * 5 < money1)continue;
					next1 = elem[j][0].first;
					next2 = elem[j][1].first;
					if (visit[next1] == 1 || visit[next2] == 1) continue;
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
				index5 = Gra[index4][m].first;
				money5 = Gra[index4][m].second;
				if (money5 > money4 * 3 || money5 * 5 < money4)continue;
				if (begin >= index5 || index5 == index3 || index5 == index4)continue;
				path[3] = index5;
				visit[index5] = 1;
				if (reachBegin3[threadId][index5][1] == begin)
				{
					auto elem = idMap3[threadId][reachBegin3[threadId][index5][0]];
					for (int j = 1; j <= elem[0][0].first; ++j)
					{
						money1 = elem[j][1].second;
						money2 = elem[j][0].second;
						if (money2 > money5 * 3 || money3 > money1 * 3 || money2 * 5 < money5 || money3 * 5 < money1)continue;
						next1 = elem[j][0].first;
						next2 = elem[j][1].first;
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
					index6 = Gra[index5][n].first;
					money6 = Gra[index5][n].second;
					if (money6 > money5 * 3 || money6 * 5 < money5)continue;
					if (begin >= index6 || index6 == index3 || index6 == index4 || index6 == index5)continue;
					path[4] = index6;
					visit[index6] = 1;
					if (reachBegin3[threadId][index6][1] == begin) {
						auto elem = idMap3[threadId][reachBegin3[threadId][index6][0]];
						for (int j = 1; j <= elem[0][0].first; ++j)
						{
							money1 = elem[j][1].second;
							money2 = elem[j][0].second;
							if (money2 > money6 * 3 || money3 > money1 * 3 || money2 * 5 < money6 || money3 * 5 < money1)continue;
							next1 = elem[j][0].first;
							next2 = elem[j][1].first;
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
					for (uint32_t ii = 0; ii < outDgr[index6]; ii++) {
						index7 = Gra[index6][ii].first;
						money7 = Gra[index6][ii].second;
						if (reachBegin3[threadId][index7][1] != begin)continue;
						if (money7 > money6 * 3 || money7 * 5 < money6)continue;
						if (begin >= index7 || index7 == index3 || index7 == index4 || index7 == index5 || index7 == index6)continue;
						path[5] = index7;
						visit[index7] = 1;
						auto elem = idMap3[threadId][reachBegin3[threadId][index7][0]];
						for (int j = 1; j <= elem[0][0].first; ++j)
						{
							money1 = elem[j][1].second;
							money2 = elem[j][0].second;
							if (money2 > money7 * 3 || money2 * 5 < money7 || money3 > money1 * 3 || money3 * 5 < money1)continue;
							next1 = elem[j][0].first;
							next2 = elem[j][1].first;
							if (visit[next1] == 1 || visit[next2] == 1) continue;
							for (int i = 0; i < 6; i++) {
								memcpy(resultPosition[zonId][5], outStr[path[i]] + 1, 16);
								strLen = outStr[path[i]][0];
								resultPosition[zonId][5] += strLen;
							}
							memcpy(resultPosition[zonId][5], outStr[next1] + 1, 16);
							strLen = outStr[next1][0];
							resultPosition[zonId][5] += strLen;
							memcpy(resultPosition[zonId][5], lstStr[next2] + 1, 16);
							strLen = lstStr[next2][0];
							resultPosition[zonId][5] += strLen;
							++cirArr[threadId][5];
						}

						visit[index7] = 0;
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
		for (int j = 0; j < 6; j++) {
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
	for (int i = 0; i < 6; i++) {
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

	//my_searchMap.storeGra3();
	start = clock();
	cout << "find the circle account" << endl;
	my_searchMap.multiSearch();
	cout << "output result" << endl;
	end = clock();
	my_searchMap.storeResult();

	cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}
