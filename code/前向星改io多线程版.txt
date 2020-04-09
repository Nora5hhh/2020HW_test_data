#include <iostream>
#include <sstream>
#include <fstream>
#include<vector>
#include<algorithm>
#include <sys/time.h>
#include <cmath>
#include <cstdlib>
#include <string.h>
#include<thread>
#include<set>
#include <functional> 
#include<sys/mman.h>
#include <sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>

using namespace std;

const int N = 560000;
const int n=300000;

struct EDGE
{
	int u, v, next;            //w表示权重，后期可加上表示money，u与v表示有向顶点对
}edge[n];        //链式前向星结构体

struct CMP
{
	bool operator()(vector<int> a, vector<int> b) const
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
	int len;
	//int subdata;
	int*  data = new int[N];
	int*  pos = new int[N];
	int*  arr = new int[N];
	int*  head = new int[N]();
	vector<bool> visit;
	multiset<vector<int>, CMP> result;
	vector<vector<vector<int>>> result_sub;

	bool loadTestData();
	void dfs(int count, int index, int min, vector<int>& path, int result_subindex);
	bool checkRepeat(vector<int>& temp_path);
	void addEdge(int& u, int& v);
	void circleSearch(int start, int end, int result_subindex);
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
	FILE* fp = fopen(testFile_.c_str(), "r");
	if (fp == NULL)
	{
		printf("Open File Failure!\n");
		exit(1);
	}
	else
	{
		while (fscanf(fp, "%d,%d,%d", &id, &subdata, &money)!=EOF)
		{
			//fscanf(fp, "%d,%d,%d", &id, &subdata, &money);
			++cnt;
			data[cnt] = id;
			head[cnt] = -1;
			arr[cnt] = pos[cnt] = data[cnt];
			++cnt;
			data[cnt] = subdata;
			head[cnt] = -1;
			arr[cnt] = pos[cnt] = data[cnt];
		}
	}
	sort(data, data + cnt);                   //排序
	len = unique(data, data + cnt) - data;     //得到去重后的长度
	for (int j = 0; j < cnt; j++)
	{
		pos[j] = lower_bound(data, data + len, pos[j]) - data;               //一个pos代表唯一一个值，每个pos的值都可以通过data[pos[i
		++j;
		pos[j] = lower_bound(data, data + len, pos[j]) - data;
		addEdge(pos[j - 1], pos[j]);
	}
	fclose(fp);
	return true;
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
				head[cnt] = -1;
				arr[cnt] = pos[cnt] = data[cnt];
				++cnt;
				data[cnt] = subdata;          //将aim读入到第二列
				head[cnt] = -1;
				arr[cnt] = pos[cnt] = data[cnt];
			}
			else {
				cout << "测试文件数据格式不正确" << endl;
				return false;
			}
		}
	}
	sort(data, data + cnt);                   //排序
	len = unique(data, data + cnt) - data;     //得到去重后的长度
	for (int j = 0; j < cnt; j++)
	{
		pos[j] = lower_bound(data, data + len, pos[j]) - data;               //一个pos代表唯一一个值，每个pos的值都可以通过data[pos[i
		++j;
		pos[j] = lower_bound(data, data + len, pos[j]) - data;
		addEdge(pos[j - 1], pos[j]);
	}
	infile.close();
	return true;
}

bool searchEdge::loadTestData()
{
	int fd = open(testFile_.c_str(), O_RDWR | O_CREAT,0666);
 	int fd_len = lseek(fd, 0, SEEK_END);
 	char* readBuf = (char*)mmap(NULL, fd_len, PROT_READ, MAP_PRIVATE, fd, 0);      //readBuf即文件数据
 
 	char* p=readBuf;
 	while (*p >= '0' && *p <= '9') {
         	int id, subdata;
         	char *num;
         	char* q = p;
         	while (*q != ',') {
              		++q;
         	}
         	num = (char *) malloc((q - p) * sizeof(char));
         	memcpy(num, p, q - p+1);
         	id = atoi(num);
  		free(num);
  		++cnt;
  		data[cnt] = id;               //将id读入放到data数组中
  		head[cnt] = -1;
  		arr[cnt] = pos[cnt] = data[cnt];

          	//寻找第二个逗号
         	p = q + 1;
         	q = p;
         	while (*q != ',') {
              		++q;
         	}
         	num = (char *) malloc((q - p) * sizeof(char));
         	memcpy(num, p, q - p+1);
         	subdata = atoi(num);
         	free(num);
  		++cnt;
  		data[cnt] = subdata;          //将aim读入到第二列
  		head[cnt] = -1;
 	 	arr[cnt] = pos[cnt] = data[cnt];
        	 //寻找换行位置
         	p = q + 1;
         	q = p;
         	while (*q != '\n') {
              		++q;
         	}
         	p = q + 1;
 	}
 	close(fd);
 	munmap(readBuf,fd_len);
	sort(data, data + cnt);                   //排序
	len = unique(data, data + cnt) - data;     //得到去重后的长度
	for (int j = 0; j < cnt; j++)
	{
		pos[j] = lower_bound(data, data + len, pos[j]) - data;               //一个pos代表唯一一个值，每个pos的值都可以通过data[pos[i
		++j;
		pos[j] = lower_bound(data, data + len, pos[j]) - data;
		addEdge(pos[j - 1], pos[j]);
	}
	return true;
}

void searchEdge::dfs(int count, int index,int min, vector<int>& path, int result_subindex)
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
				result_sub[result_subindex].push_back(path);
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
				dfs(count+1, edge[i].v,min, path,result_subindex);
			}
			else
			{
				if (count >= 3)
				{
					vector<int> temp_path(path.begin(), path.begin() + count);
					if (!checkRepeat(temp_path))
						return;
					result_sub[result_subindex].push_back(temp_path);
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

void searchEdge::circleSearch(int start, int end, int result_subindex) {
	for (int i = start; i < end; i++) {
		//visit=vector<bool>(len,false);
		vector<int> path(7, -1);
		path[0] = data[i];
		dfs(1,i, i, path, result_subindex);
	}
}

void searchEdge::findRing() {
	int thread_count = 8;
	result_sub.resize(thread_count);
	//bind生成一个新的可调用对象
	
	thread thread1(bind(&searchEdge::circleSearch, this, 0, (int)len / 8, 0));
	thread thread2(bind(&searchEdge::circleSearch, this, (int)len / 8,2* (int)len/8, 1));
	thread thread3(bind(&searchEdge::circleSearch, this, 2 *(int)len / 8, 3 * (int)len / 8, 2));
	thread thread4(bind(&searchEdge::circleSearch, this, 3 * (int)len / 8, 4 * (int)len / 8, 3));
	thread thread5(bind(&searchEdge::circleSearch, this, 4 * (int)len / 8, 5 * (int)len / 8, 4));
	thread thread6(bind(&searchEdge::circleSearch, this, 5 * (int)len / 8, 6 * (int)len / 8, 5));
	thread thread7(bind(&searchEdge::circleSearch, this, 6 * (int)len/ 8, 7 * (int)len / 8, 6));
	thread thread8(bind(&searchEdge::circleSearch, this, 7 * (int)len / 8, (int)len , 7));

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
	delete[] arr;
	delete[] head;
	delete[] data;
	delete[] pos;

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
	struct timeval start,end;
	gettimeofday(&start,NULL);

	cout << "ready to work" << endl;
	
	clock_t end1, end2, end3;
	string testFile = "test_data5.txt";
	string resultFile = "/home/yinjie/result.txt";
	searchEdge my_searchMap(testFile, resultFile);
	
	cout << "find the circle account" << endl;
	my_searchMap.findRing();
	
	cout << "output result" << endl;
	my_searchMap.storeResult();

	gettimeofday(&end,NULL);
	long long total_time=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
	total_time/=1000;
	cout<<"total time is "<<total_time<<" ms"<<endl;
	return 0;
}


