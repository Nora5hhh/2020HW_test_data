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

using namespace std;

const int N = 560000;
const int n = 280000;

struct EDGE
{
	int u, v, next;            //w表示权重，后期可加上表示money，u与v表示有向顶点对
}edge[N];        //链式前向星结构体

struct CMP
{
	bool operator()(vector<unsigned int> a, vector<unsigned int> b) const
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
};

class searchEdge {
public:
	searchEdge(string testFile, string resFile);
	void init();
	void findRing();
	void storeResult();
private:
	string testFile_;
	string resFile_;
	int cnt;
	int edgeCount;           //当前边总数
	int subdata;
	int len;
	//int* data = new int[N];
	vector<unsigned int> data;
	vector<vector<unsigned int>> my_map;
	unsigned int* pos = new unsigned int[N];
	multiset<vector<unsigned int>, CMP> result;
	vector<vector<vector<unsigned int>>> result_sub;
	bool loadTestData();
	void dfs(int count, int index, vector<unsigned int>& path, int result_subindex);
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

//每加入一条边(u,v)，就在原有链表结构首部插入这条边
//因此边的顺序与读入顺序相反

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
	my_map.resize(len);
	for (int i = 0; i < my_map.size(); i++) {
		my_map[i].reserve(20);
	}
	for (int j = 0; j < cnt; j++)
	{
		pos[j] = lower_bound(data.begin(), data.begin() + len, pos[j]) - data.begin();               //一个pos代表唯一一个值，
		++j;
		pos[j] = lower_bound(data.begin(), data.begin() + len, pos[j]) - data.begin();
		//addEdge(pos[j - 1], pos[j]);
		my_map[pos[j - 1]].push_back(pos[j]);
	}
	delete[] pos;
	infile.close();
	return true;
}

void searchEdge::dfs(int count, int index, vector<unsigned int>& path, int result_subindex)
{
	if (count == 7) {
		for (int i = 0; i < my_map[index].size(); i++) {
			if (path[0] == data[my_map[index][i]]) {
				if (!checkRepeat(path))
					return;
				result_sub[result_subindex].push_back(path);
				return;
			}
		}
	}
	else {
		for (int i = 0; i < my_map[index].size(); i++) {
			if (path[0] > data[my_map[index][i]])continue;
			if (data[my_map[index][i]] != path[0]) {
				path[count] = data[my_map[index][i]];
				dfs(++count, my_map[index][i], path, result_subindex);
				count--;
			}
			else {
				if (count >= 3) {
					vector<unsigned int>temp_path(path.begin(), path.begin() + count);
					if (!checkRepeat(temp_path))
						continue;
					result_sub[result_subindex].push_back(temp_path);
				}
			}
		}
	}
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
		vector<unsigned int> path(7, -1);
		path[0] = data[i];
		dfs(1, i, path, result_subindex);
		//cout << i << endl;
	}
}

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

void searchEdge::storeResult() {
	string line;
	int i;
	ofstream fout(resFile_.c_str());
	if (!fout.is_open()) {
		cout << "打开预测结果文件失败" << endl;
	}
	fout << result.size() << endl;
	for (auto iter = result.begin(); iter != result.end(); ++iter)
	{
		for (unsigned int i = 0; i < (*iter).size(); ++i)
		{
			if (i < (*iter).size() - 1)
				fout << (*iter)[i] << ",";
			else
				fout << (*iter)[i] << endl;
		}
	}
	fout.close();
	return;
}

int main()
{
	cout << "ready to work" << endl;
	clock_t start, end;
	string testFile = "/data/test_data.txt";
	string resultFile = "/projects/student/result.txt";
	searchEdge my_searchMap(testFile, resultFile);
	start = clock();
	cout << "find the circle account" << endl;
	my_searchMap.findRing();
	end = clock();
	cout << "output result" << endl;
	my_searchMap.storeResult();
	cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}





