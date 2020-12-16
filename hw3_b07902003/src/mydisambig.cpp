#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "File.h"
#include "Ngram.h"
#include "Vocab.h"
using namespace std;
int main(int argc, char const *argv[])
{
	ifstream mapIn;
	mapIn.open(argv[2]);
	string mapLine;
	map<string, vector<string>> zhuyinMap;
	map<string, vector<string>>::iterator iter;
	int a = 0;
	string delimiter = " ";
	string token;
	while (getline(mapIn, mapLine))
	{
		a++;
		// cout << mapLine.length() << endl;
		size_t pos = 0;
		vector<string> tmp;

		while ((pos = mapLine.find(delimiter)) != string::npos)
		{
			// cout << pos << endl;
			token = mapLine.substr(0, pos);
			tmp.push_back(token);
			mapLine.erase(0, pos + delimiter.length());
		}
		string key;
		for (int i = 0; i < tmp.size(); i++)
		{
			if (i == 0)
			{
				key = tmp[0];
			}
			else
			{
				zhuyinMap[key].push_back(tmp[i]);
			}
		}
		zhuyinMap[key].push_back(mapLine);
		// cout << "a = " << a << " mapsize = " << zhuyinMap[key].size() << endl;
	}
	mapIn.close();
	vector<string> v1;
	v1.push_back("<s>");
	vector<string> v2;
	v2.push_back("</s>");
	zhuyinMap["<s>"] = v1;
	zhuyinMap["</s>"] = v2;
	Vocab voc;
	Ngram languageModel(voc, 2);
	File languageModelFile(argv[3], "r");
	languageModel.read(languageModelFile);
	languageModelFile.close();
	File textFile(argv[1], "r");
	ofstream outfile;
	outfile.open(argv[4]);
	//decode
	char *buf;
	int j = 0;
	while (buf = textFile.getline())
	{
		int wordCount;
		const char *line[maxWordsPerLine];
		wordCount = Vocab::parseWords(buf, &(line[1]), maxWordsPerLine);
		line[0] = "<s>";
		line[wordCount + 1] = "</s>";
		wordCount += 2;
		// cout << wordCount << endl;
		vector<vector<LogP>> prob = vector<vector<LogP>>(256, vector<LogP>(1024, -100));
		vector<vector<VocabIndex>> vidx_graph = vector<vector<VocabIndex>>(256, vector<VocabIndex>(1024, 0)); // Record the (i, j) VocalIndex of Big5
		vector<vector<VocabIndex>> backtrack = vector<vector<VocabIndex>>(256, vector<VocabIndex>(1024, 0));
		vector<int> candi_num = vector<int>(256, 0);
		VocabIndex empty_context[] = {Vocab_None};
		VocabIndex bi_context[] = {Vocab_None, Vocab_None};
		VocabIndex candi = voc.getIndex(line[0]);
		if (candi == Vocab_None)
			candi = voc.getIndex(Vocab_Unknown);
		vidx_graph[0][0] = candi;
		// cout << candi << endl;
		prob[0][0] = 0;
		candi_num[0] = 1;
		backtrack[0][0] = -1;
		// cout << prob[0][0] << endl;
		for (int i = 1; i < wordCount; i++)
		{
			int j = 0;
			for (auto word : zhuyinMap[(string)line[i]])
			{
				candi = voc.getIndex(word.c_str());
				if (candi == Vocab_None)
					candi = voc.getIndex(Vocab_Unknown);
				LogP maxP = LogP_Zero;
				LogP curP;
				// cout << "candi = " << candi << endl;
				vidx_graph[i][j] = candi;
				for (int k = 0; k < candi_num[i - 1]; k++)
				{
					VocabIndex last = vidx_graph[i - 1][k];
					bi_context[0] = last;
					// cout << "bi " << bi_context[0] << " candi " << candi << languageModel.wordProb(candi, bi_context) << endl;
					curP = languageModel.wordProb(candi, bi_context);
					if (curP == LogP_Zero)
					{
						cout << i << word << k << "-inf" << endl;
					}
					curP += prob[i - 1][k];
					// cout << word << " " << curP << " maxP " << maxP << endl;
					if (curP > maxP)
					{
						maxP = curP;
						backtrack[i][j] = k;
					}
				}
				prob[i][j] = maxP;
				// cout << i << "," << j << maxP << endl;
				j++;
			}
			// cout << i << line[i] << " " << words.size() << "" << endl;
			candi_num[i] = j;
			// cout << maxP << endl;
			// else
			// {
			// 	LogP maxP = LogP_Zero;
			// 	LogP curP;
			// 	// cout << iter->first << words.size() << endl;
			// 	candi = voc.getIndex(line[i]);
			// 	if (candi == Vocab_None)
			// 		candi = voc.getIndex(Vocab_Unknown);
			// 	for (int k = 0; k < candi_num[i - 1]; k++)
			// 	{
			// 		bi_context[0] = vidx_graph[i - 1][k];
			// 		curP = languageModel.wordProb(candi, bi_context);
			// 		curP += prob[i - 1][k];
			// 		if (curP > maxP)
			// 		{
			// 			maxP = curP;
			// 			backtrack[i][j] = k;
			// 		}
			// 	}
			// 	prob[i][0] = maxP;
			// 	vidx_graph[i][0] = candi;
			// 	candi_num[i] = 1;
			// 	// cout << "else = " << maxP << endl;
			// }
		}

		// cout << "done a line" << endl;
		LogP maxp = LogP_Zero;
		int decode_max_idx = backtrack[wordCount - 1][0];
		string decode_path[256];
		decode_path[0] = "<s>";
		decode_path[wordCount - 1] = "</s>";
		// cout << "here" << endl;

		// for (int i = 0; i < candi_num[wordCount - 1]; i++)
		// {

		// 	// cout << i << " " << prob[wordCount - 1][i] << endl;
		// 	if (prob[wordCount - 1][i] > maxp)
		// 	{
		// 		maxp = prob[wordCount - 1][i];
		// 		decode_max_idx = i;
		// 	}
		// }
		// cout << "gotcha" << endl;
		for (int i = wordCount - 1; i > 0; i--)
		{
			// cout << "in for" << endl;
			// cout << line[i] << " " << decode_max_idx << " " << i << endl;
			decode_path[i - 1] = zhuyinMap[line[i - 1]][decode_max_idx];
			// if (iter != zhuyinMap.end()){
			// 	cout << "if" << endl;
			// 	decode_path[i] = iter->s[decode_max_idx];
			// }
			// else{
			// 	cout << "else" << endl;
			// 	decode_path[i] = line[i];
			// }
			// cout << decode_path[i] << endl;
			decode_max_idx = backtrack[i - 1][decode_max_idx];
		}
		for (int i = 0; i < wordCount - 1; i++)
		{
			// cout << decode_path[i] << ' ';
			outfile << decode_path[i] << ' ';
		}
		outfile << decode_path[wordCount - 1] << endl;
		// cout << decode_path[wordCount - 1] << endl;
	}
}