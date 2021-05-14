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

using namespace std;
const int N = 560000;
unsigned int result[5][3000000][7];    //三维数组
unsigned int cirArr[5];

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
	int len;

	vector<unordered_map<int, vector<int>>> idMap;
	unsigned int* inDgr;
	unsigned int* outDgr;
	unsigned int data[N];
	vector<bool> arrFlag;
	vector<vector<unsigned int>> Gra;
	unsigned int pos[N];
	unordered_map<unsigned int, int> id2In;

	int subIndex;
	int cicleCnt;
	int curNum;

	void init();
	void dfs(int begin, int index, unsigned int* path, vector<bool>& visit, int count);
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
	Gra.resize(len);
	inDgr = new unsigned int[len]();
	outDgr = new unsigned int[len]();

	for (int i = 0; i < len; i++) {
		id2In[pos[i]] = i;
	}
	for (int j = 0; j < cnt; j += 2)
	{
		auto u = id2In[data[j]];
		auto v = id2In[data[j + 1]];
		Gra[u].emplace_back(v);
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
		for (int aim : Gra[temp])
		{
			--inDgr[aim];
			if (0 == inDgr[aim])
				que.push(aim);
		}
	}
	for (int i = 0; i < len; ++i)
	{
		if (outDgr[i] == 0)
			que.push(i);
	}
	while (!que.empty())
	{
		int temp = que.front();
		que.pop();
		for (int aim : Gra[temp])
		{
			--outDgr[aim];
			if (0 == outDgr[aim])
				que.push(aim);
		}
	}

	for (int i = 0; i < len; ++i)
	{
		if (inDgr[i] == 0 || outDgr[i] == 0)
		{
			vector<unsigned int> nul;
			Gra[i].swap(nul);
		}
		else
		{
			sort(Gra[i].begin(), Gra[i].end());
		}
	}
}

void searchEdge::storeGra3() {
	idMap = vector<unordered_map<int, vector<int>>>(len, unordered_map<int, vector<int>>());
	for (int i = 0; i < len; i++) {
		auto cGra = Gra[i];          //第二个
		for (auto &ci : cGra) {
			auto &ccGra = Gra[ci];       //第三个
			for (auto &cci : ccGra) {
				if (cci < i && cci < ci) {
					idMap[cci][i].emplace_back(ci);
				}
			}
		}
	}
}

void searchEdge::dfs(int begin, int index, unsigned int* path, vector<bool>& visit, int count)
{
	path[count] = pos[index];
	++count;
	visit[index] = true;
	auto &now = Gra[index];
	int i = 0;
	int nLen = now.size();
	for (; i < nLen && now[i] <= begin; ++i)
	{
		int curr = now[i];
		if (count >= 3 && curr == begin)
		{
			memcpy(result[count - 3][cirArr[count - 3]], path, count * sizeof(unsigned int));
			++cicleCnt;
			cirArr[count - 3]++;
			break;
		}
	}
	if (count < 6)
	{
		for (; i < nLen; ++i) {
			int curr = now[i];
			if (curr > begin && !visit[curr]) {
				dfs(begin, curr, path, visit, count);
			}
		}
	}

	else if (arrFlag[index] == true)
	{
		auto elem = idMap[begin][index];
		for (auto next : elem)
		{
			if (visit[next] == true) continue;
			path[6] = pos[next];
			memcpy(result[4][cirArr[4]], path, 7 * sizeof(unsigned int));
			cirArr[4]++;
			++cicleCnt;
		}
	}
	visit[index] = false;
}

void searchEdge::findRing()
{
	arrFlag = vector<bool>(len, false);
	vector<unsigned int> tempNext(len);
	vector<bool> visit = vector<bool>(len, 0);
	for (int i = 0; i < len; ++i)
	{
		if (!Gra.empty())
		{
			for (auto &elem : idMap[i])
			{
				arrFlag[elem.first] = true;
				tempNext.emplace_back(elem.first);
			}
			unsigned int path[7];
			dfs(i, i, path, visit, 0);
			for (auto &elem : tempNext)
			{
				arrFlag[elem] = false;
			}
			vector<unsigned int> nul;
			tempNext.swap(nul);            //清空
		}
	}
}

void searchEdge::storeResult() {
	int fLen = 77 * (cicleCnt + 1);
	string Str = to_string(cicleCnt) + "\n";
	int fd = open(resFile_.c_str(), O_RDWR | O_CREAT, 00777);
	lseek(fd, fLen, SEEK_SET);
	char* buf = (char*)mmap(NULL, fLen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	char* p = buf;
	int w = write(fd, "0", 1);         //对齐
	memcpy(buf, Str.c_str(), Str.size());
	buf += Str.size();
	for (int i = 0; i < subIndex; ++i)       //层数
	{
		for (int k = 0; k < cirArr[i]; ++k)      //对每一层的每个环
		{
			auto curcnt = result[i][k];          //当前环
			int lenn = i + 3;                   //当前层的长度
			string str = "";
			for (int j = 0; j < lenn; ++j)
			{
				str += to_string(curcnt[j]) + ',';
			}
			int sLen = str.size();
			str[sLen - 1] = '\n';
			memcpy(buf, str.c_str(), sLen);
			buf += sLen;
		}
	}
	munmap(buf, fLen);
	ftruncate(fd, buf - p);
	close(fd);
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




