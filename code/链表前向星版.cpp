#include <iostream>
#include<vector>
#include<map>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include<algorithm>
using namespace std;

const int N = 60000;

struct EDGE
{
	int u, v,  next;            //w表示权重，后期可加上表示money，u与v表示有向顶点对
}edge[N];        //链式前向星结构体

const bool cmp(vector<int> a, vector<int> b) {
	if (a.size() == b.size()) {
		if (a[0] == b[0])
			return a[1] < b[1];
		else
			return a[0] < b[0];
	}
	else
		return a.size() < b.size();
}

class searchEdge {
public:
	searchEdge(string testFile, string resFile);
	void init();
	void findRing();
	void storeResult();
private:
	string testFile_;
	string resFile_;
	vector<vector<int>> result;
	//vector<int> path;
	int data[N];
	int pos[N];           //下标到id的映射
	int cnt;
	int edgeCount ;           //当前边总数
	int head[N] ;                //head[i]指向i节点的第一条边
	int dataPos[N];           //id到index的映射
	int arr[N];
	int subdata;
	int len;
	

	bool loadTestData();
	void dfs(int count, int index,int min, vector<int>& path);
	bool checkRepeat(vector<int>& temp_path);
	void addEdge(int& u, int& v);
};

searchEdge::searchEdge(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	init();
}

void searchEdge::init() {
	//参数初始化
	memset(head, -1, sizeof(head));
	cnt = -1;
	edgeCount = 0;
	//path.resize(7, -1);
	loadTestData();
}

//每加入一条边(u,v)，就在原有链表结构首部插入这条边
//因此边的顺序与读入顺序相反
void searchEdge::addEdge(int &u, int& v)
{
	edge[edgeCount].u = u;
	edge[edgeCount].v = v;
	edge[edgeCount].next = head[u];
	head[u] = edgeCount++;
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

				data[++cnt] = id;               //将id读入放到data数组中
				arr[cnt] = pos[cnt] = data[cnt];
				data[++cnt] = subdata;          //将aim读入到第二列
				arr[cnt] = pos[cnt] = data[cnt];
			}
			else {
				cout << "测试文件数据格式不正确" << endl;
				return false;
			}
		}
	}
	sort(data, data + cnt);                   //排序
	len = unique(data , data + cnt ) - data;     //得到去重后的长度
	for (int j = 0; j < cnt; j++)
	{
		pos[j] = lower_bound(data , data + len , pos[j]) - data;               //一个pos代表唯一一个值，每个pos的值都可以通过data[pos[i]]得到相应的值
		pos[++j] = lower_bound(data, data + len, pos[j]) - data;
		addEdge(pos[j-1], pos[j]);
	}
	infile.close();
	return true;
}

void searchEdge::dfs(int count, int index,int min, vector<int>& path)
{
	if (count == 7)
	{
		for (int i = head[index]; ~i; i = edge[i].next)
		{
			if (head[edge[i].v] == -1) continue;
			if (edge[i].v == min)
			{
				if (!checkRepeat(path))
					return;
				result.push_back(path);
				return;
			}
		}
	}
	else
	{
		//-1按位取反是0
		for (int i = head[index]; ~i; i = edge[i].next)
		{	
			if (head[edge[i].v]==-1||min > edge[i].v) continue;
			if (edge[i].v != min)               //尚为成环
			{
				path[count] = data[edge[i].v];
				dfs(++count, edge[i].v,min, path);
				count--;
			}
			else
			{
				if (count >= 3)
				{
					vector<int> temp_path(path.begin(), path.begin() + count);
					if (!checkRepeat(temp_path))
						return;
					result.push_back(temp_path);
				}
			}
		}
	}
}

void searchEdge::dfs(int count, int index, int min, vector<int>& path)
{
if (index == min && count != 0)
	{
		if (count >= 3)
		{
			vector<int> temp_path(path.begin(), path.begin() + count);
			if (!checkRepeat(temp_path))
				return;
			result.push_back(temp_path);
		}
		return;
	}
	else
	{
		if (count == 6)
		{
			for (int i = head[index]; ~i; i = edge[i].next)
			{
				if (edge[i].v == min)
				{
					path[count] = data[index];
					dfs(count+1, edge[i].v, min, path);
				}
			}
		}
		else
		{
			for (int i = head[index]; ~i; i = edge[i].next)
			{
				if (edge[i].v >= min)
				{
					if (visit[edge[i].v] == 0 || edge[i].v == min)
					{
						visit[index] = 1;
						path[count] = data[index];
						dfs(count+1, edge[i].v, min, path);
						visit[index] = 0;
					}
				}
			}
		}
	}
}

bool searchEdge::checkRepeat(vector<int> &temp_path) {
	for (int j = 0; j < temp_path.size() - 1; j++) {
		for (int k = j + 1; k < temp_path.size(); k++) {
			if (temp_path[j] == temp_path[k]) {
				return false;
			}
		}
	}
	return true;
}

void searchEdge::findRing()
{
	for (int i = 0; i < len; ++i)
	{
		//data中第i个元素在原来数组中的位置
		//dataPos[i] = find(arr, arr + cnt, data[i]) - arr;  //建立id到下标的映射
		vector<int> path(7, -1);
		path[0] = data[i];
		dfs(1, i,i,path);
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

	cout << "find the circle account" << endl;
	my_searchMap.findRing();
	cout << "output result" << endl;
	my_searchMap.storeResult();
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}


