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
//多线程版
//struct CMP
//{
//	bool operator()(vector<unsigned int> a, vector<unsigned int> b) const
//
//	{
//
//		if (a.size() != b.size())
//
//			return a.size() < b.size();
//
//		else {
//
//			for (int i = 0; i < a.size() && i < b.size(); i++) {
//
//				if (a[i] != b[i]) {
//
//					return a[i] < b[i];
//
//				}
//
//			}
//
//			return a.size() < b.size();
//
//		}
//
//	}

//};

class searchEdge {
public:
	searchEdge(string testFile, string resFile);
	void init();
	void findRing();
	void storeResult();

private:
	string testFile_;
	string resFile_;
	vector<vector<unsigned int>> result;
	int cnt;
	int edgeCount;           //当前边总数
	int subdata;
	int len;
	vector<unsigned int> data;
	vector<vector<unsigned int>> my_map;
	unsigned int* pos = new unsigned int[N];
	//multiset<vector<unsigned int>, CMP> result;
	//vector<vector<vector<unsigned int>>> result_sub;
	bool loadTestData();
	void dfs(int count, int begin, int index, vector<unsigned int>& path, vector<bool>& visit);
	//bool checkRepeat(vector<unsigned int>& temp_path);
	void addEdge(unsigned int& u, unsigned int& v);
	//void circleSearch(unsigned int start, unsigned int end, int result_subindex);
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
				data[cnt] = subdata;          //将aim读入到第二列
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

	/*for (int i = 0; i < my_map.size(); i++) {
		my_map[i].reserve(20);
	}*/

	for (int j = 0; j < cnt; j++)
	{
		pos[j] = lower_bound(data.begin(), data.begin() + len, pos[j]) - data.begin();               //一个pos代表唯一一个值，每个pos的值都可以通过data[pos[i
		++j;
		pos[j] = lower_bound(data.begin(), data.begin() + len, pos[j]) - data.begin();
		my_map[pos[j - 1]].push_back(pos[j]);
	}
	delete[] pos;
	infile.close();
	return true;
}

void searchEdge::dfs(int count,int begin, int index, vector<unsigned int>& path, vector<bool>& visit)
{
	visit[index] = true;
	if (count == 7) {
		for (int i = 0; i < my_map[index].size(); i++) {
			if (begin == my_map[index][i]) {
				vector<unsigned int>temp_path;
				for (int j = 0; j < path.size(); j++) {
					temp_path.push_back(data[path[j]]);
				}
				result.push_back(temp_path);
			}
		}
	}
	else {
		for (int i = 0; i < my_map[index].size(); i++) {
			int curr = my_map[index][i];
			if (begin > curr)continue;
			if (curr != begin) {
				if (!visit[curr])
				{
					path[count] = curr;
					dfs(count + 1,begin, curr, path, visit);
				}
			}
			else{
				if (count >= 3) {
					vector<unsigned int>temp_path;
					for (int j = 0; j < count; j++) {
						temp_path.emplace_back(data[path[j]]);
					}
					result.emplace_back(temp_path);
				}
			}
		}
	}
	visit[index] = false;
}

//多线程版
//void searchEdge::dfs(int count, int index, vector<unsigned int>& path, int result_subindex)
//{
//	if (count == 7) {
//		for (int i = 0; i < my_map[index].size(); i++) {
//			//	if (visit[index] == 0)continue;
//			if (path[0] == my_map[index][i]) {
//				if (!checkRepeat(path))
//					return;
//				vector<unsigned int>temp_path;
//
//				for (int j = 0; j < path.size(); j++) {
//					temp_path.push_back(data[path[j]]);
//				}
//				result_sub[result_subindex].push_back(temp_path);
//				return;
//			}
//		}
//	}
//	else {
//		for (int i = 0; i < my_map[index].size(); i++) {
//			if (path[0] > my_map[index][i])continue;
//			if (my_map[index][i] != path[0]) {
//				path[count] = my_map[index][i];
//				dfs(++count, my_map[index][i], path, result_subindex);
//				count--;
//			}
//			else {
//				if (count >= 3) {
//					vector<unsigned int>temp_path;
//					for (int j = 0; j < count; j++) {
//						temp_path.push_back(data[path[j]]);
//					}
//					if (!checkRepeat(temp_path))
//						continue;
//					result_sub[result_subindex].push_back(temp_path);
//				}
//			}
//		}
//	}
//}

//bool searchEdge::checkRepeat(vector<unsigned int>& temp_path) {
//	for (int j = 0; j < temp_path.size() - 1; j++) {
//		for (int k = j + 1; k < temp_path.size(); k++) {
//			if (temp_path[j] == temp_path[k]) {
//				return false;
//			}
//		}
//	}
//	return true;
//}

void searchEdge::findRing()
{
	for (int i = 0; i < len; ++i)
	{
		vector<bool> visit(len, false);
		vector<unsigned int> path;
		path.resize(7,-1);
		path[0] = i;
		dfs(1,i, i, path,visit);
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


//
//void searchEdge::circleSearch(unsigned int start, unsigned int end, int result_subindex) {
//
//	vector<vector<unsigned int>> path3;
//	path3.reserve(N);
//	int index2, index3, index4, index5, index6, index7, index8;
//
//	for (int i = start; i < end; i++) {
//		//int i = 18;
//		vector<unsigned int> temp_path(7, -1);
//		temp_path[0] = i;
//		index2 = i;
//		for (int j = 0; j < my_map[index2].size(); j++) {
//			if (temp_path[0] >= my_map[index2][j])continue;
//			temp_path[1] = my_map[index2][j];
//			index3 = my_map[index2][j];
//			for (int k = 0; k < my_map[index3].size(); k++) {
//				if (temp_path[0] >= my_map[index3][k] || temp_path[1] == my_map[index3][k])continue;
//				temp_path[2] = my_map[index3][k];
//				dfs(3, temp_path[2], temp_path, result_subindex);
//				//path3.push_back(temp_path);
//			}
//		}
//		//cout << i << endl;
//	}
//

	/*for (int i = start; i < end; i++) {
		//int i = 18;
		vector<unsigned int> temp_path(7, -1);
		temp_path[0] = i;
		index2 = i;
		for (int j = 0; j < my_map[index2].size(); j++) {
			if (temp_path[0] >= my_map[index2][j])continue;
			temp_path[1] = my_map[index2][j];
			index3 = my_map[index2][j];
			for (int k = 0; k < my_map[index3].size(); k++) {
				if (temp_path[0] >= my_map[index3][k] || temp_path[1] == my_map[index3][k])continue;
				temp_path[2] = my_map[index3][k];
				index4 = my_map[index3][k];
				for (int m=0; m < my_map[index4].size(); m++) {
					if (temp_path[0] > my_map[index4][m] || temp_path[1] == my_map[index4][m]|| temp_path[2] == my_map[index4][m])continue;

					if (temp_path[0] == my_map[index4][m]) {
						vector<unsigned int>path;
						for (int xx = 0; xx < 3; xx++) {
							path.push_back(data[temp_path[xx]]);
						}
						result_sub[result_subindex].push_back(path);
					}
					else {
						temp_path[3] = my_map[index4][m];
						index5 = my_map[index4][m];
						for (int n = 0; n < my_map[index5].size(); n++) {
							if (temp_path[0] > my_map[index5][n] || temp_path[1] == my_map[index5][n] || temp_path[2] == my_map[index5][n] || temp_path[3] == my_map[index5][n])continue;

							if (temp_path[0] == my_map[index5][n]) {
								vector<unsigned int>path;
								for (int xx = 0; xx < 4; xx++) {
									path.push_back(data[temp_path[xx]]);
								}
								result_sub[result_subindex].push_back(path);
							}
							else {
								temp_path[4] = my_map[index5][n];
								index6 = my_map[index5][n];
								for (int ii = 0; ii < my_map[index6].size(); ii++) {
									if (temp_path[0] > my_map[index6][ii] || temp_path[1] == my_map[index6][ii] || temp_path[2] == my_map[index6][ii] || temp_path[3] == my_map[index6][ii] || temp_path[4] == my_map[index6][ii])continue;

									if (temp_path[0] == my_map[index6][ii]) {
										vector<unsigned int>path;
										for (int xx = 0; xx < 5; xx++) {
											path.push_back(data[temp_path[xx]]);
										}
										result_sub[result_subindex].push_back(path);
									}
									else {

										temp_path[5] = my_map[index6][ii];
										index7 = my_map[index6][ii];
										for (int jj = 0; jj < my_map[index7].size(); jj++) {
											if (temp_path[0] > my_map[index7][jj] || temp_path[1] == my_map[index7][jj] || temp_path[2] == my_map[index7][jj] || temp_path[3] == my_map[index7][jj] || temp_path[4] == my_map[index7][jj] || temp_path[5] == my_map[index7][jj])continue;
											if (temp_path[0] == my_map[index7][jj]) {
												vector<unsigned int>path;
												for (int xx = 0; xx < 6; xx++) {
													path.push_back(data[temp_path[xx]]);
												}
												result_sub[result_subindex].push_back(path);
											}
											else {

												temp_path[6] = my_map[index7][jj];
												index8 = my_map[index7][jj];
												for (int kk = 0; kk < my_map[index8].size(); kk++) {
													if (temp_path[0] > my_map[index8][kk] || temp_path[1] == my_map[index8][kk] || temp_path[2] == my_map[index8][kk] || temp_path[3] == my_map[index8][kk] || temp_path[4] == my_map[index8][kk] || temp_path[5] == my_map[index8][kk] || temp_path[6] == my_map[index8][kk])continue;
													if (temp_path[0] == my_map[index8][kk]) {
														vector<unsigned int>path;
														for (int xx = 0; xx < 7; xx++) {
															path.push_back(data[temp_path[xx]]);
														}
														result_sub[result_subindex].push_back(path);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		/cout << i << endl;
	}
}
*/

//
//void searchEdge::findRing() {
//
//	int thread_count = 1;
//	result_sub.resize(thread_count);
//	circleSearch(0, len, 0);
//
//	for (int j = 0; j < result_sub[0].size(); j++)
//		result.insert(result_sub[0][j]);*/
//
//	int thread_count = 8;
//
//	result_sub.resize(thread_count);
//
//	//bind生成一个新的可调用对象
//
//	thread thread1(bind(&searchEdge::circleSearch, this, 0, (int)(len / 32), 0));
//	thread thread2(bind(&searchEdge::circleSearch, this, (int)(len / 32), (int)(2 * len / 32), 1));
//	thread thread3(bind(&searchEdge::circleSearch, this, (int)(2 * len / 32), (int)(3.5 * len / 32), 2));
//	thread thread4(bind(&searchEdge::circleSearch, this, (int)(3.5 * len / 32), (int)(4.8 * len / 32), 3));
//	thread thread5(bind(&searchEdge::circleSearch, this, (int)(4.8 * len / 32), (int)(6.8 * len / 32), 4));
//	thread thread6(bind(&searchEdge::circleSearch, this, (int)(6.8 * len / 32), (int)(8.8 * len / 32), 5));
//	thread thread7(bind(&searchEdge::circleSearch, this, (int)(8.8 * len / 32), (int)(12.8 * len / 32), 6));
//	thread thread8(bind(&searchEdge::circleSearch, this, (int)(12.8 * len / 32), (int)len, 7));
//
//	thread1.join();
//
//	thread2.join();
//
//	thread3.join();
//
//	thread4.join();
//
//	thread5.join();
//
//	thread6.join();
//
//	thread7.join();
//
//	thread8.join();
//
//	for (int i = 0; i < thread_count; i++) {
//
//		for (int j = 0; j < result_sub[i].size(); j++)
//
//			result.insert(result_sub[i][j]);
//
//	}
//
//}
//
//void searchEdge::storeResult() {
//
//	string line;
//
//	int i;
//
//	ofstream fout(resFile_.c_str());
//
//	if (!fout.is_open()) {
//
//		cout << "打开预测结果文件失败" << endl;
//
//	}
//
//	fout << result.size() << endl;
//
//	for (auto iter = result.begin(); iter != result.end(); ++iter)
//
//	{
//
//		for (unsigned int i = 0; i < (*iter).size(); ++i)
//
//		{
//
//			if (i < (*iter).size() - 1)
//
//				fout << (*iter)[i] << ",";
//
//			else
//
//				fout << (*iter)[i] << endl;
//		}
//	}
//
//	fout.close();
//
//	return;
//
//}

int main()
{
	cout << "ready to work" << endl;
	clock_t start, end;

	//string testFile = "/data/test_data.txt";
	string testFile = "D:/test_data1.txt";
	//string resultFile = "/projects/student/result.txt";
	string resultFile = "D:/result.txt";
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



