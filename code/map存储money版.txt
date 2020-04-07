#include <iostream>
#include<vector>
#include<map>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <time.h>
using namespace std;

const int max_vex_num =280000;       //最大顶点数
string testFile = "D:/test_data1.txt";
string resultFile = "D:/result.txt";

struct Node {
	int id;
	int money;
	vector<int> subNodes;
};

class searchMap {
public:
	searchMap(string testFile, string resFile);
	void init();
	void process();
	void DFS(int cur, bool visited[max_vex_num], int stack[max_vex_num], int top, bool instack[max_vex_num], int count);
private:
	string testFile_;
	string resFile_;

	int count = 0;
	bool visited[max_vex_num] = { 0 };     //表示元素是否被访问
	bool instack[max_vex_num] = { 0 };    //判断元素是否在栈中
	int stack[10];           //堆栈

	map<int, Node> my_map; //<id,subNodes>
	bool loadTestData();
};

searchMap::searchMap(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	init();
}

void searchMap::init() {
	//参数初始化
	loadTestData();
}

bool searchMap::loadTestData()
{
	ifstream infile(testFile_.c_str());
	string lineTitle;
	if (!infile) {
		cout << "打开测试文件失败" << endl;
		exit(0);
	}
	while (infile) {
		vector<double> feature;
		string line;
		getline(infile, line);
		if (line.size() > 0) {
			stringstream sin(line);
			Node newNode;
			int id, aim;   //索引
			char ch;
			char c = sin.peek();
			if (int(c) != -1) {
				sin >> id;
				sin >> ch;              //可能是钱
				if (my_map.count(id) > 0)     //文件中有该点
				{
					sin >> aim;
					my_map[id].subNodes.push_back(aim);
					sin >> ch;
					sin >> my_map[id].money;

				}
				else {
					newNode.id = id;
					sin >> aim;
					newNode.subNodes.push_back(aim);
					sin >> ch;
					sin >> newNode.money;
					my_map.insert(pair<int, Node>(id, newNode));
				}
			}
			else {
				cout << "测试文件数据格式不正确" << endl;
				return false;
			}
		}
	}
	infile.close();
	return true;
}

void searchMap::DFS(int cur, bool visited[max_vex_num], int stack[max_vex_num], int top, bool instack[max_vex_num], int count)
{
	visited[cur] = true;
	stack[++top] = cur;

	if (top > 7)
		return;            //有向边个数为3到7

	instack[cur] = true;
	for (int i = 0; i < my_map[cur].subNodes.size(); ++i)
	{
		if (my_map[cur].subNodes[i] != 0)         //存在该有向边
		{
			if (!instack[i])
			{
				DFS(i, visited, stack, top, instack, count);
			}
			else
			{
				++count;
				int t = 0;
				ofstream ofresult(resultFile.c_str());
				//将数据插入到resultfile中
				for (t = top; stack[t] != i; t--)
				{
					ofresult << stack[t] << ",";
				}
				ofresult << endl;
			}
		}
	}
	top--;             //出栈
	instack[cur] = false;
}

void searchMap::process()
{
	//调用my_map中第一个节点
	if (!my_map.empty())
	{
		DFS(my_map.begin()->first, visited, stack, -1, instack, 0);
	}
}

int main()
{
	cout << "ready to work" << endl;
	//string testFile = "D:/test_data1.txt";
	//string resultFile = "D:/result.txt";
	searchMap my_searchMap(testFile, resultFile);
	my_searchMap.process();
	return 0;
}


