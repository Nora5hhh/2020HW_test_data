#include <iostream>
#include <sstream>
#include <fstream>

#include<vector>
#include<map>
#include<stack>
#include<set>
#include<algorithm>

#include <cmath>
#include <cstdlib>
#include <time.h>
#include<thread>
#include <functional> 
#include<bitset>
using namespace std;
struct Node {
	int id;
	int area = 0;
	//vector<int> money;
	vector<int> subNodes;
};


struct CMP
{
	bool operator()(vector<int> a, vector<int> b) const
	{
		if (a.size() == b.size()) {
			return a[0] < b[0];
		}
		else
			return a.size() < b.size();
	}
};

class searchMap {
public:
	searchMap(string testFile, string resFile);
	void init();
	//void printSCCs();					// 打印强连通分量	
	void findRing();
	void storeResult();
private:
	string testFile_;
	string resFile_;
	map<int, Node> my_map; //<id,subNodes>
	multiset<vector<int>, CMP> result;
	vector<vector<vector<int>>> result_sub;
	clock_t start, end;
	int max_id = -1;

	bool loadTestData();
	void removeSingle();
	void dfs(int count, int index, vector<int> &path, int result_subindex);
	bool checkRepeat(vector<int> &temp_path);
	void circleSearch(int start, int end, int result_subindex);
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
			Node newNode, newReNode;
			int id, aim, money;
			char ch;
			char c = sin.peek();
			if (int(c) != -1) {
				sin >> id;
				sin >> ch;
				if (my_map.count(id) > 0) {
					sin >> aim;
					my_map[id].subNodes.push_back(aim);
				}
				else {
					newNode.id = id;
					sin >> aim;
					newNode.subNodes.push_back(aim);
					my_map.insert(pair<int, Node>(id, newNode));
				}
				max_id = max_id < id ? id : max_id;
				max_id = max_id < aim ? aim : max_id;
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


void searchMap::dfs(int count, int index, vector<int>& path, int result_subindex) {
	if (count == 7) {
		for (int i = 0; i < my_map[index].subNodes.size(); i++) {
			if (my_map.count(my_map[index].subNodes[i]) == 0)continue;
			if (path[0] == my_map[index].subNodes[i]) {
				if (!checkRepeat(path))
					return;
				result_sub[result_subindex].push_back(path);
				return;
			}
		}
	}
	else {
		for (int i = 0; i < my_map[index].subNodes.size(); i++) {

			if (my_map.count(my_map[index].subNodes[i]) == 0 || path[0] > my_map[index].subNodes[i])continue;
			if (my_map[index].subNodes[i] != path[0]) {
				path[count] = my_map[index].subNodes[i];
				dfs(++count, my_map[index].subNodes[i], path, result_subindex);
				count--;
			}
			else {
				if (count >= 3) {
					vector<int>temp_path(path.begin(), path.begin() + count);
					if (!checkRepeat(temp_path))
						return;
					result_sub[result_subindex].push_back(temp_path);
					return;
				}
			}
		}
	}
}

void searchMap::circleSearch(int start, int end, int result_subindex) {
	//cout << max_id << endl;
	for (int i = start; i < end; i++) {
		vector<int> path(7, -1);
		path[0] = my_map[i].id;
		dfs(1, i, path, result_subindex);
		//cout << i << endl;
	}
}

void searchMap::findRing() {
	int thread_count = 8;
	result_sub.resize(thread_count);
	//bind生成一个新的可调用对象
	
	thread thread1(bind(&searchMap::circleSearch, this, 0, (int)max_id / 8, 0));
	thread thread2(bind(&searchMap::circleSearch, this, (int)max_id / 8,2* (int)max_id/8, 1));
	thread thread3(bind(&searchMap::circleSearch, this, 2 *(int)max_id / 8, 3 * (int)max_id / 8, 2));
	thread thread4(bind(&searchMap::circleSearch, this, 3 * (int)max_id / 8, 4 * (int)max_id / 8, 3));
	thread thread5(bind(&searchMap::circleSearch, this, 4 * (int)max_id / 8, 5 * (int)max_id / 8, 4));
	thread thread6(bind(&searchMap::circleSearch, this, 5 * (int)max_id / 8, 6 * (int)max_id / 8, 5));
	thread thread7(bind(&searchMap::circleSearch, this, 6 * (int)max_id / 8, 7 * (int)max_id / 8, 6));
	thread thread8(bind(&searchMap::circleSearch, this, 7 * (int)max_id / 8, (int)max_id , 7));

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

void searchMap::storeResult() {
	string line;
	int i;
	ofstream fout(resFile_.c_str());
	if (!fout.is_open()) {
		cout << "打开预测结果文件失败" << endl;
	}
	fout << result.size() << endl;
	for (auto iter = result.begin(); iter != result.end(); ++iter)
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
	string testFile = "D:/test_data1.txt";
	string resultFile = "D:/result.txt";
	clock_t start, end;
	start = clock();
	searchMap my_searchMap(testFile, resultFile);

	cout << "find the circle account" << endl;
	my_searchMap.findRing();

	cout << "output result" << endl;
	my_searchMap.storeResult();
	end = clock();
	cout << (double)(end - start) / CLOCKS_PER_SEC << endl;
	return 0;
}
