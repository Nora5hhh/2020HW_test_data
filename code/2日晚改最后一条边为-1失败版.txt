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

const bool cmp(vector<int> a, vector<int> b) {
	if (a.size() == b.size()) {
		return a[0] < b[0];
	}
	else
		return a.size() < b.size();
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
	//multimap<int, int> my_map;
	map<int, vector<int>> my_map;    //<id,aim>
	vector<vector<int>> result;
	

	bool loadTestData();
	void DFS(int start, int index,int count, vector<int>& path);
	bool checkRepeat(vector<int>& temp_path);
};

searchMap::searchMap(string testFile, string resFile) {
	testFile_ = testFile;
	resFile_ = resFile;
	init();
}

void searchMap::init() {
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
			int id, aim;
			char ch;
			char c = sin.peek();
			if (line.size() > 0) {
				stringstream sin(line);
				int id, aim;   //索引
				char ch;
				char c = sin.peek();
				if (int(c) != -1) {
					sin >> id;
					sin >> ch;              //可能是钱
					if (my_map.count(id) > 0)     //文件中有该点
					{
						sin >> aim;
						my_map[id].push_back(aim);
					}
					else {
						sin >> aim;
						vector<int> temp;
						temp.push_back(aim);
						my_map.insert(pair<int, vector<int>>(id, temp));
					}
				}
				else {
					cout << "测试文件数据格式不正确" << endl;
					return false;
				}
			}
		}
	}
	infile.close();
	return true;
}


void searchMap::DFS(int start, int index, int count, vector<int>& path)
{
	if (count > 7) return;          //剪枝
	auto iter = ::find(my_map[index].begin(), my_map[index].end(), start);
	path.push_back(index);
	if (iter == my_map[index].end())         //没找到
	{
		if (count < 7)
		{
			for (int i = 0; i < my_map[index].size(); ++i)
			{
				DFS(start, my_map[index][i], ++count, path);
			}
		}
		else if (count > 7)
			return;

	}
	else       //找到了
	{
		if (count > 2 && count < 8)
		{
			*iter = 0;           //擦除最后一边从而避免三次重复
			result.push_back(path);
			return;
		}
		else
			return;
	}
}

void searchMap::dfs(int count, int index, vector<int>&flag_map) {
	if (count == 7) {
		for (int i = 0; i < my_map[index].subNodes.size(); i++) {
			if (my_map.count(my_map[index].subNodes[i]) == 0)continue;
			if (path[0] == my_map[index].subNodes[i]) {
				result.push_back(path);
				return;
			}
		}
	}
	else {
		for (int i = 0; i < my_map[index].subNodes.size(); i++) {

			if (my_map.count(my_map[index].subNodes[i]) == 0 || path[0] > my_map[index].subNodes[i])continue;
			if (my_map[index].subNodes[i] != path[0]) {
				if (flag_map[my_map[index].subNodes[i]] == 1) {
					continue;
				}
				flag_map[my_map[index].subNodes[i]] = 1;
				path[count] = my_map[index].subNodes[i];
				dfs(++count, my_map[index].subNodes[i], flag_map);
				flag_map[my_map[index].subNodes[i]] = 0;
				count--;
			}
			else {
				if (count >= 3) {
					vector<int>temp_path(path.begin(), path.begin() + count);
					result.push_back(temp_path);
					return;
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

void searchMap::findRing() {
	
	for (auto elem=my_map.begin();elem!=my_map.end();++elem)
	{
		int start = elem->first;
		vector<int> path;
		DFS(start, start,1,path);	
	}
	sort(result.begin(), result.end(), cmp);
}

void searchMap::storeResult() {
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
	clock_t start = clock();
	clock_t end1,end2,end3;
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


