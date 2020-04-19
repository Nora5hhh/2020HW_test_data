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
	vector<vector<unordered_map<int, vector<int>>>> idMap3;
	vector<vector<vector<int>>> result;
	vector<int> inDgr;
	vector<int> outDgr;
	int data[N];
	vector<int> pos;
	unordered_map<int, int> id2In;
	vector<bool> arrFlag;
	vector<bool> arrFlag2;
	vector<vector<int>> Gra;
	vector<string> outStr;
	vector<string> lstStr;
	int subIndex;
	int cicleCnt;
	int curNum;

	void init();
	void dfs(int begin, int index, vector<int>& path, vector<bool> &visit, int count);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	subIndex = 5;
	cicleCnt = 0;
	curNum = 0;
	cnt = 0;
	result.resize(subIndex);
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
	pos.assign(data, data + cnt);
	sort(pos.begin(), pos.end()); //排序
	pos.erase(unique(pos.begin(), pos.end()), pos.end());
	len = pos.size();
	Gra.resize(len);
	inDgr = vector<int>(len, 0);
	outDgr = vector<int>(len, 0);
	outStr = vector<string>(len);
	lstStr = vector<string>(len);

	for (int i = 0; i < len; i++) {
		id2In[pos[i]] = i;
		outStr[i] = (to_string(pos[i]) + ',');
		lstStr[i] = (to_string(pos[i]) + '\n');
	}
	for (int i = 0; i <cnt; i += 2) {
		int u = id2In[data[i]];
		int v = id2In[data[i + 1]];
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
			vector<int> nul;
			Gra[i].swap(nul);    //将Gra中不可行的点置为0
		}
		else
		{
			sort(Gra[i].begin(), Gra[i].end());
		}
	}
}

void searchEdge::storeGra3() {
	idMap = vector<unordered_map<int, vector<int>>>(len, unordered_map<int, vector<int>>());
	idMap3 = vector<vector<unordered_map<int, vector<int>>>>(len, vector<unordered_map<int, vector<int>>>());
	for (int i = 0; i < len; i++) {
		auto cGra = Gra[i];          //第二个
		for (auto &ci : cGra) {
			auto &ccGra = Gra[ci];       //第三个
			for (auto &cci : ccGra) {
				if (cci < i && cci < ci) {
					idMap[cci][i].emplace_back(ci);
				}
			}

			for (auto& cci : ccGra)          //P3
			{
				auto& c3Gra = Gra[cci];
				for (auto& c3i : c3Gra)
				{
					if (c3i < i&&c3i < cci&&c3i < ci)
					{
						idMap3[c3i][i][ci].emplace_back(cci);
					}
				}
			}

		}
	}
}

void searchEdge::dfs(int begin, int index, vector<int>& path, vector<bool> &visit, int count)
{
	path[count] = index;
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
			result[count - 3].emplace_back(vector<int>(path.begin(), path.begin() + count));
			++cicleCnt;
			break;
		}
	}
	if (count < 5)
	{
		for (; i < nLen; ++i) {
			int curr = now[i];
			if (curr != begin && !visit[curr]) {
				dfs(begin, curr, path, visit, count);
			}
		}
	}
	
	else if (count==5&&arrFlag2[now[i]] == true)
	{
		int cIndex = now[i];
		auto elem = idMap3[begin][index][cIndex];
		for (auto& next : elem)
		{
			if (visit[next] == true) continue;
			vector<int> temp(7);
			temp.assign(path.begin(), path.begin() + 5);
			temp.emplace_back(cIndex);
			temp.emplace_back(next);
			result[4].emplace_back(temp);
		}
	}
	else if (count == 6 && arrFlag[index] == true)
	{
		auto elem = idMap[begin][index];
		for (auto next : elem)
		{
			if (visit[next] == true) continue;
			vector<int> temp(7);
			temp.assign(path.begin(), path.begin() + 6);
			temp.emplace_back(next);
			result[4].emplace_back(temp);
			++cicleCnt;
		}
	}
	visit[index] = false;
}

void searchEdge::findRing()
{
	arrFlag = vector<bool>(len, false);
	arrFlag2 = vector<bool>(len, false);
	vector<int> tempNext(len);
	vector<int> tempNext2(len);
	for (int i = 0; i < len; ++i)
	{
		if (!Gra[i].empty())
		{
			for (auto &pre : idMap[i])
			{
				arrFlag[pre.first] = true;
				tempNext.emplace_back(pre.first);
			}
			for (auto& pre : idMap3[i])
			{
				for (auto& cpre : pre)
				{
					arrFlag2[cpre.first] = true;
					tempNext2.emplace_back(cpre.first);
				}
			}
			vector<int> path;
			vector<bool> visit(len, false);
			path.resize(7, -1);
			dfs(i, i, path, visit, 0);
			for (auto &elem : tempNext)
			{
				arrFlag[elem] = false;
			}
			vector<int> nul;
			tempNext.swap(nul);
			for (auto& elem : tempNext2)
			{
				arrFlag2[elem] = false;
			}
			vector<int> nul2;
			tempNext2.swap(nul2);
		}
	}
}

void searchEdge::storeResult() 
{
	int fLen = 77 * (cicleCnt + 1);
	string Str = to_string(cicleCnt) + "\n";
	int fd = open(resFile_.c_str(), O_RDWR | O_CREAT, 00777);
	lseek(fd, fLen, SEEK_SET);
	char* buf = (char*)mmap(NULL, fLen, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	char* p = buf;
	int w = write(fd, "0", 1);       //对齐
	for (int i = 0; i < subIndex; ++i)
	{
		for (auto& elem : result[i])
		{
			for (int j = 0; j < elem.size() - 1; ++j)
			{
				string str = "";
				auto lenn = elem.size();
				for (int j = 0; j < lenn - 1; ++j)
				{
					str += outStr[elem[j]];
				}
				str += lstStr[lenn - 1];
				int sLen = str.size();
				memcpy(buf, str.c_str(), sLen);
				buf += sLen;
			}
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
	//string testFile = "10969.txt";
	//string resultFile = "/home/yinjie/result.txt";

	//clock_t start, end;
	//start = clock();
	searchEdge my_searchMap(testFile, resultFile);
	my_searchMap.remSin();
	my_searchMap.storeGra3();
	//cout << "find the circle account" << endl;
	my_searchMap.findRing();
	//cout << "output result" << endl;
	my_searchMap.storeResult();
	//end = clock();
	//cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}





