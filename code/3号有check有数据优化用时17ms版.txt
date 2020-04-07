#include <iostream>
#include<vector>
#include<map>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include<algorithm>
#include<set>
using namespace std;
struct Node {
	int id;
	//vector<int> money;
	vector<int> subNodes;
};

struct CMP
{
	 bool operator()(vector<int> a, vector<int> b) {
		if (a.size() == b.size()) {
			return a[0] < b[0];
		}
		else
			return a.size() < b.size();
	}
};

const bool isEqual(vector<int> a, vector<int> b) {
	if (a.size() == b.size()) {
		for (int i = 0; i < a.size(); i++) {
			if (a[i] != b[i])return false;
		}
	}
	else
		return false;
	return true;
}


class searchMap {
public:
	searchMap(string testFile, string resFile);
	void init();
	void findRing();
	void storeResult();
private:
	string testFile_;
	string resFile_;
	map<int, Node> my_map; //<id,subNodes>
	vector<int> path;
	map<int,int> in;
	multiset<vector<int>,CMP> my_result;

	bool loadTestData();
	void dfs(int count, int index, int min);
	bool checkRepeat(vector<int>& temp_path);
	void removeSingle();
};

searchMap::searchMap(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	init();
}

void searchMap::init() {
	//参数初始化
	path.resize(7, -1);
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
		string line;
		getline(infile, line);
		if (line.size() > 0) {
			stringstream sin(line);
			Node newNode;
			int id, aim, money;
			char ch;
			char c = sin.peek();
			if (int(c) != -1) {
				sin >> id;
				sin >> ch;
				if (my_map.count(id) > 0)
				{
					sin >> aim;

					my_map[id].subNodes.push_back(aim);
					++in[id];
					sin >> ch;
					sin >> money;
					//	my_map[id].money.push_back(money);
				}
				else {
					newNode.id = id;
					sin >> aim;

					newNode.subNodes.push_back(aim);
					sin >> ch;
					sin >> money;
					//	newNode.money.push_back(money);
					my_map.insert(pair<int, Node>(id, newNode));
					in.insert(pair<int, int>(id, 1));           //统计每个id出现次数
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

void searchMap::dfs(int count, int index, int min) {
	if (count == 7) {          //刚好到7时
		for (int i = 0; i < my_map[index].subNodes.size(); i++) {
			if (my_map.count(my_map[index].subNodes[i]) == 0) return;
			if (path[0] == my_map[index].subNodes[i] && min == path[0]) {
				if (!checkRepeat(path))
					return;
				my_result.insert(path);
			}
		}
	}
	else {
		for (int i = 0; i < my_map[index].subNodes.size(); i++) {
			if (my_map.count(my_map[index].subNodes[i]) == 0) return;
			if (my_map[index].subNodes[i] != path[0]) {    //不为环，继续遍历
				path[count] = my_map[index].subNodes[i];
				int temp_min;
				temp_min = min < my_map[index].subNodes[i] ? min : my_map[index].subNodes[i];
				dfs(++count, my_map[index].subNodes[i], temp_min);
				count--;
			}
			else {
				if (count >= 3) {
					if (min == path[0]) {
						vector<int>temp_path(path.begin(), path.begin() + count);

						if (!checkRepeat(temp_path)) //排除8字形环
							continue;
						my_result.insert(temp_path);
					}
				}
			}
		}
	}
}

bool searchMap::checkRepeat(vector<int> &temp_path) {
	for (int j = 0; j < temp_path.size() - 1; j++) {
		for (int k = j + 1; k < temp_path.size(); k++) {
			if (temp_path[j] == temp_path[k]) {
				return false;
			}
		}
	}
	return true;
}


//数据预处理，去除出度为0的边
void searchMap::removeSingle()
{
	auto iter = my_map.begin();
	while (  iter != my_map.end())
	{
		if (in[iter->first] == 1)
		{
			if (in[iter->second.subNodes[0]] == 0)
			{
				in[iter->first] = 0;
				my_map.erase(iter++);       //删除该id为key的边
			}
			else
				++iter;
		}
		else
		{
			auto vec = iter->second.subNodes;
			auto i = vec.begin();
			while ( i != vec.end())
			{
				if (in[*i] == 0)
				{
					--in[iter->first];
					iter->second.subNodes.erase(i++);
				}
				else
				{
					++i;
				}
			}
			++iter;
		}
	}
}


void searchMap::findRing() {
	removeSingle();
	for (int i = 0; i < my_map.size(); i++) {
		path[0] = my_map[i].id;
		dfs(1, i, path[0]);
	}
}

void searchMap::storeResult() {
	string line;
	int i;
	ofstream fout(resFile_.c_str());
	if (!fout.is_open()) {
		cout << "打开预测结果文件失败" << endl;
	}

	fout << my_result.size() << endl;
	for (auto iter = my_result.begin(); iter != my_result.end(); ++iter)
	{
		for (int i = 0; i < (*iter).size(); ++i)
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
	clock_t start = clock();
	clock_t end1, end2, end3;
	string testFile = "D:/test_data1.txt";
	string resultFile = "D:/result.txt";
	searchMap my_searchMap(testFile, resultFile);
	end1 = clock();
	cout << "time cost of read: " << (double)(end1 - start) << endl;
	
	cout << "find the circle account" << endl;
	my_searchMap.findRing();
	end2 = clock();
	cout << "time cost of find ring: " << (double)(end2 - end1) << endl;
	cout << "output result" << endl;
	my_searchMap.storeResult();
	end3 = clock();
	cout << "time cost of store: " << (double)(end3 - end2) << endl;
	return 0;
}


