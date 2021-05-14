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
#include<unordered_map>
#include <sys/mman.h>

using namespace std;

const int node = 280000;
const int maxValue = 200000;

const unsigned int sizeGra[10] = { 9,99,999,9999,99999,999999,9999999,99999999, 999999999,4294967295 };
unsigned int cirArr[5][4];
unsigned int result[5][4][2000000][7];
unsigned int Gra[node][50];
unsigned int revGra[node][50];
unsigned int inDgr[node];
unsigned int outDgr[node];
char outStr[node][12];
char lstStr[node][12];
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
	void stroreMulThread();
	void remSin();
	void multiSearch();
private:
	string testFile_;
	string resFile_;

	int firstIdCnt;
	int idCnt;
	unsigned int subLen;
	unsigned int ringCnt;
	int curNum, cnt, len;
	unordered_map<unsigned int, int> id2In;
	int strLen[4] = { 0 };
	char* buf[4];
	void init();
	void storeGra3(unsigned int begin, int threadId);
	void findRing(int threadId, unsigned int beginIndex, unsigned int endIndex);
	void dfs(unsigned int begin, unsigned int index, unsigned int* path, unsigned int* visit, int count, int threadId);
	void dfs_circle(unsigned int begin, unsigned int* path, unsigned int* visit, int threadId);
	void storeResult(int start, int end);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	ringCnt = 0;
	curNum = 0;
	subLen = 0;
	idCnt = 0;
	cnt = 0;
	len = 0;
	init();
}

void searchEdge::init() {
	int fd = open(testFile_.c_str(), O_RDONLY);
	int flen = lseek(fd, 0, SEEK_END);
	char* buf = (char*)mmap(NULL, flen, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	char* p = buf;
	unsigned int data[node * 2];
	unsigned int pos[node * 2 + 1];
	while (*p >= '0' && *p <= '9') {
		char* q = p;
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
	memcpy(pos + 1, data, cnt * sizeof(unsigned int));
	sort(pos + 1, pos + cnt + 1);
	len = unique(pos + 1, pos + cnt + 1) - pos - 1;

	for (int i = 1; i <= len; i++) {
		id2In[pos[i]] = i;
		subLen = uintToChar(pos[i], outStr[i] + 1);
		outStr[i][++subLen] = ',';
		outStr[i][0] = (char)subLen;
		subLen = uintToChar(pos[i], lstStr[i] + 1);
		lstStr[i][++subLen] = '\n';
		lstStr[i][0] = (char)subLen;
	}
	for (int j = 0; j < cnt; j += 2)
	{
		auto u = id2In[data[j]];
		auto v = id2In[data[j + 1]];
		Gra[u][outDgr[u]] = v;
		revGra[v][inDgr[v]] = u;
		++inDgr[v];
		++outDgr[u];
	}
}
void searchEdge::remSin()
{
	unsigned int l = 0, h = 0;
	unsigned int u = 0, v = 0;
	for (int i = 1; i <= len; i++)
	{
		if (0 == inDgr[i])
			que[h++] = i;
	}
	while (l < h)
	{
		u = que[l++];
		int eLen = outDgr[u];            //对于入度为0的点，遍历它指向的点
		for (int i = 0; i <= eLen; ++i)
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
	for (int i = 1; i <= len; ++i)
	{
		if (0 == outDgr[i])
		{
			que[h++] = i;
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
	for (int i = 1; i <= len; ++i)
	{
		if (inDgr[i] == 0 || outDgr[i] == 0)
		{
		}
		else
		{
			sort(revGra[i], revGra[i] + inDgr[i]);
			sort(Gra[i], Gra[i] + outDgr[i]);

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
				if (gk == begin) {
					result[0][threadId][cirArr[0][threadId]][0] = gk;
					result[0][threadId][cirArr[0][threadId]][1] = gj;
					result[0][threadId][cirArr[0][threadId]][2] = gt;
					++cirArr[0][threadId];
					continue;
				}

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

		unsigned int begin = i;            //只考虑起始点
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
				memcpy(result[1][threadId][cirArr[1][threadId]], path, 4 * sizeof(unsigned int));
				++cirArr[1][threadId];
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
					memcpy(result[2][threadId][cirArr[2][threadId]], path, 5 * sizeof(unsigned int));
					++cirArr[2][threadId];
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
						memcpy(result[3][threadId][cirArr[3][threadId]], path, 6 * sizeof(unsigned int));
						++cirArr[3][threadId];
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
							memcpy(result[4][threadId][cirArr[4][threadId]], path, 7 * sizeof(unsigned int));
							++cirArr[4][threadId];
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
	thread thread1(bind(&searchEdge::findRing, this, 0, 1, (unsigned int)(0.05 * (len + 1))));
	thread thread2(bind(&searchEdge::findRing, this, 1, (unsigned int)((len + 1) * 0.05), (unsigned int)((len + 1) * 0.1)));
	thread thread3(bind(&searchEdge::findRing, this, 2, (unsigned int)((len + 1) * 0.1), (unsigned int)((len + 1) * 0.25)));
	thread thread4(bind(&searchEdge::findRing, this, 3, (unsigned int)((len + 1) * 0.25), (unsigned int)(len + 1)));
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
}

void searchEdge::stroreMulThread() {
	unsigned int out_count[5] = { 0 }, max_count = 0;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			out_count[i] += cirArr[i][j];
			ringCnt += cirArr[i][j];
		}
	}
	out_count[1] += out_count[0];
	for (int i = 1; i < 5; i++) {
		max_count = max_count > out_count[i] ? max_count : out_count[i];
	}
	char stbuf[15];
	strLen[0] = uintToChar(ringCnt, stbuf + 1);
	stbuf[++strLen[0]] = '\n';
	int fLen = 77 * (max_count + 1);
	FILE* out = fopen(resFile_.c_str(), "wb");
	char* p[4];
	for (int i = 0; i < 4; i++) {
		buf[i] = (char*)malloc(4 * fLen * sizeof(char));
		p[i] = buf[i];
	}
	memcpy(buf[0], stbuf + 1, strLen[0]);
	buf[0] += strLen[0];
	thread thread1(bind(&searchEdge::storeResult, this, 0, 2));
	thread thread2(bind(&searchEdge::storeResult, this, 2, 3));
	thread thread3(bind(&searchEdge::storeResult, this, 3, 4));
	thread thread4(bind(&searchEdge::storeResult, this, 4, 5));
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	for (int i = 0; i < 4; i++) {
		fwrite(p[i], buf[i] - p[i], sizeof(char), out);
		free(p[i]);
	}
	fclose(out);
}
void searchEdge::storeResult(int start, int end) {
	int limit, index_l;
	for (int i = start; i < end; i++) {
		limit = i + 2;
		if (i == 0) {
			index_l = i;
		}
		else {
			index_l = i - 1;
		}

		for (unsigned int t = 0; t < 4; ++t)
		{
			for (unsigned int k = 0; k < cirArr[i][t]; ++k)
			{
				auto curcnt = result[i][t][k];
				for (unsigned int j = 0; j < limit; ++j)
				{
					char* str = outStr[curcnt[j]];
					strLen[index_l] = str[0];
					memcpy(buf[index_l], str + 1, strLen[index_l]);
					buf[index_l] += strLen[index_l];
				}
				char* str = lstStr[curcnt[limit]];
				strLen[index_l] = str[0];
				memcpy(buf[index_l], str + 1, strLen[index_l]);
				buf[index_l] += strLen[index_l];
			}
		}
	}

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
	cout << "find the circle account" << endl;
	my_searchMap.multiSearch();
	cout << "output result" << endl;
	my_searchMap.stroreMulThread();
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}
