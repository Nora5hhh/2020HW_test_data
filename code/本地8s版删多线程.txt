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

using namespace std;

const int N = 560000;

const bool cmp(vector<unsigned int> a, vector<unsigned int> b)
{
	if (a.size() != b.size())
		return a.size() < b.size();
	else {
		for (int i = 0; i < a.size() && i < b.size(); i++) {
			if (a[i] != b[i]) {
				return a[i] < b[i];
			}
		}
		return a.size() < b.size();
	}
}

class searchEdge {
public:
	searchEdge(string testFile, string resFile);
	void init();
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
	vector<vector<unsigned int>> result;
	vector<unsigned int> inDgr;
	vector<unsigned int> outDgr;
	vector<vector<bool>> flag;
	//int* data = new int[N];
	vector<unsigned int> data;
	vector<vector<unsigned int>> Gra;
	unsigned int* pos = new unsigned int[N];
	//multiset<vector<unsigned int>, CMP> result;
	vector<vector<vector<unsigned int>>> result_sub;
	bool loadTestData();
	void dfs(int begin, int index, vector<unsigned int>& path);
	bool checkRepeat(vector<unsigned int>& temp_path);
	void addEdge(unsigned int& u, unsigned int& v);
	void circleSearch(unsigned int start, unsigned int end, int result_subindex);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	init();
}

void searchEdge::init() {
	//参数初始化
	cnt = -1;
	edgeCount = 0;
	data.resize(N);
	loadTestData();
}


bool searchEdge::loadTestData()
{
	ifstream infile(testFile_.c_str());
	string lineTitle;
	if (!infile) {
		cout << "打开测试文件失败" << endl;
		exit(0);
	}
	while (infile) {
		string line;
		getline(infile, line);
		if (line.size() > 0) {
			stringstream sin(line);
			int id;
			char ch;
			char c = sin.peek();
			if (int(c) != -1) {
				sin >> id;
				sin >> ch;
				sin >> subdata;
				++cnt;
				data[cnt] = id;               //将id读入放到data数组中
				pos[cnt] = data[cnt];
				++cnt;
				data[cnt] = subdata;          //将aim读入到第二
				pos[cnt] = data[cnt];
			}
			else {
				cout << "测试文件数据格式不正确" << endl;
				return false;
			}
		}
	}
	sort(data.begin(), data.end());                   //排序
	len = unique(data.begin(), data.end()) - data.begin();     //得到去重后的长度
	Gra.resize(len);
	inDgr.resize(len, 0);
	outDgr.resize(len, 0);
	flag.resize(len, vector<bool>(len, false));
	
	for (int j = 0; j < cnt; ++j)
	{
		pos[j] = lower_bound(data.begin(), data.begin() + len, pos[j]) - data.begin();               //一个pos代表唯一一个值，
		++j;
		pos[j] = lower_bound(data.begin(), data.begin() + len, pos[j]) - data.begin();
		
		Gra[pos[j-1 ]].push_back(pos[j]);
		
		inDgr[pos[j]]++;
		outDgr[pos[j-1]]++;
	}
	delete[] pos;
	infile.close();
	return true;
}

void searchEdge::remSin()
{
	queue<int> que;
	for (int i = 0; i < len; i++)
	{
		if (0 == inDgr[i])              //下标
			que.push(i);
		
	}
	while (!que.empty())
	{
		int temp = que.front();
		que.pop();
		for (int aim : Gra[temp])
		{
			--inDgr[aim];
			if (0== inDgr[aim])
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
			if (0==outDgr[aim])
				que.push(aim);
		}
	}
	
	for (int i = 0; i < len; ++i)
	{
		sort(Gra[i].begin(), Gra[i].end());      //将Gra有序化
	}
}


void searchEdge::storeGra3() {
	idMap = vector<unordered_map<int, vector<int>>>(len, unordered_map<int, vector<int>>());
	for (int i = 0; i < len; i++) {
		auto aim = Gra[i];           //id对应的aim
		for (auto &ga : aim) {                //对于aim中的每点
			auto &suba = Gra[ga];           //aim对应的aim
			for (auto &j : suba) {
				if (j != i && ga > j) {
					idMap[j][i].push_back(ga);          //存储了长度为3的路径
					flag[j][i] = true;
				}
			}
		}
	}
}

void searchEdge::dfs(int begin, int index, vector<unsigned int>& path)
{
	path.push_back(data[index]);
	auto &now = Gra[index];          //当前节点指向的下一节点集合
	auto it = lower_bound(now.begin(), now.end(), begin);
	if (it != now.end() && *it == begin && path.size() >= 3)
	{
		vector<unsigned int>temp_path = path;
		if (checkRepeat(temp_path))
		{
			result.push_back(temp_path);
		}
	}
	if (path.size() < 6)
	{
		for (; it!=now.end(); ++it) {
			if (*it != begin) {
				dfs(begin, *it, path);
			}
		}
	}
	else if (path.size() == 6)
	{
		if (flag[begin][index] == true)
		{
			auto elem = idMap[begin][index];
			for (auto next : elem)
			{
				vector<unsigned int> temp=path;
				temp.push_back(data[next]);
				if (!checkRepeat(temp))
					continue;
				result.push_back(temp);
			}
		}
	}
	path.pop_back();
}

bool searchEdge::checkRepeat(vector<unsigned int>& temp_path) {
	for (int j = 0; j < temp_path.size() - 1; j++) {
		for (int k = j + 1; k < temp_path.size(); k++) {
			if (temp_path[j] == temp_path[k]) {
				return false;
			}
		}
	}
	return true;
}


void searchEdge::circleSearch(unsigned int start, unsigned int end, int result_subindex) {
	for (unsigned int i = start; i < end; i++) {
		if (inDgr[i] > 0 && outDgr[i] > 0)
		{
			vector<unsigned int> path;
			dfs( i, i, path);
		}
	}
}

/*
void searchEdge::findRing() {
	int thread_count = 8;
	result_sub.resize(thread_count);
	//bind生成一个新的可调用对象
	thread thread1(bind(&searchEdge::circleSearch, this, 0, (int)(len / 32), 0));
	thread thread2(bind(&searchEdge::circleSearch, this, (int)(len / 32), (int)(2 * len / 32), 1));
	thread thread3(bind(&searchEdge::circleSearch, this, (int)(2 * len / 32), (int)(3.5 * len / 32), 2));
	thread thread4(bind(&searchEdge::circleSearch, this, (int)(3.5 * len / 32), (int)(6 * len / 32), 3));
	thread thread5(bind(&searchEdge::circleSearch, this, (int)(6 * len / 32), (int)(8 * len / 32), 4));
	thread thread6(bind(&searchEdge::circleSearch, this, (int)(8 * len / 32), (int)(10 * len / 32), 5));
	thread thread7(bind(&searchEdge::circleSearch, this, (int)(10 * len / 32), (int)(14 * len / 32), 6));
	thread thread8(bind(&searchEdge::circleSearch, this, (int)(14 * len / 32), (int)len, 7));
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	thread5.join();
	thread6.join();
	thread7.join();
	thread8.join();
	for (int i = 0; i < thread_count; i++) {
		for (int j = 0; j < result_sub[i].size(); j++)
			result.insert(result_sub[i][j]);
	}
}
*/

void searchEdge::findRing()
{
	for (int i = 0; i < len; ++i)
	{
		if (inDgr[i] > 0 && outDgr[i] > 0)
		{
			vector<unsigned int> path;
			//path[0] = data[i];
			dfs(i, i, path);
		}
		
	}
	sort(result.begin(), result.end(), cmp);
}

void searchEdge::storeResult() {
	string line;
	int i;
	ofstream fout(resFile_.c_str());
	if (!fout.is_open()) {
		cout << "打开预测结果文件失败" << endl;
	}

	fout << result.size() << endl;
	for (i = 0; i < result.size(); i++) {
		for (int j = 0; j < result[i].size(); j++) {
			if (j < result[i].size() - 1)
				fout << result[i][j] << ",";
			else
				fout << result[i][j] << endl;
		}
	}
	fout.close();
	return;
}

int main()
{
	cout << "ready to work" << endl;
	string testFile = "D:/test_data1.txt";
	string resultFile = "D:/result.txt";
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





