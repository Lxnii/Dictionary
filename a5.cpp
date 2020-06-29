// a5.cpp

/////////////////////////////////////////////////////////////////////////
//
// Statement of Originality
// ------------------------
//
// All the code and comments below are my own original work. For any non-
// original work, I have provided citations in the comments with enough
// detail so that someone can see the exact source and extent of the
// borrowed work.
//
// In addition, I have not shared this work with anyone else, and I have
// not seen solutions from other students, tutors, websites, books,
// etc.
//
/////////////////////////////////////////////////////////////////////////

//
// You are only allowed to include these files --- don't include anything else!
// If you think it's necessary to include another file, please speak to the
// instructor.
//

#include "cmpt_error.h"
#include <iostream>
#include <string>
#include <fstream>   // for reading/writing text files
#include <sstream>   // for using string streams
using namespace std;

std::string maindict = "maindict.txt";
std::string addedwords = "addedwords.txt";

struct WordNode 
{
	string word;
	size_t hashValue;
	int type; //-1: empty node, 0: maindict, 1: addedwords
	int line; // record the line in origin file
	WordNode() : type(-1) {}
};

struct AddedWordNode 
{ 	// record the words in addedwords, for add or remove operation
	string word;
	int type; //-1: empty node, 0: in addeedwords
	AddedWordNode() : type(-1) {}
};

class HashTable 
{
public:
	HashTable()	: maindictCount(0), addedwordsCount(0), totalWordsCount(0),
	htCapacity(100), arrCapacity(30), maxStrDistance(1) 
	{
		hashTable = new WordNode[htCapacity];
		addedwordsArray = new AddedWordNode[arrCapacity];
	}
	~HashTable() 
	{
		// delete hashTable and addedwordsArray
		delete[] hashTable;
		delete[] addedwordsArray;
		hashTable = NULL;
		addedwordsArray = NULL;
	}

	bool openFile(string maindict, string addedwords) 
	{
		// open two files
		maindictFile.open(maindict, ios::in);
		addedwordsFile.open(addedwords, ios::in);
		if (maindictFile.is_open() && addedwordsFile.is_open()) 
		{
			return true;
		} 
		else 
		{
			cout << "can't find maindict.txt or addedwords.txt." << endl;
			return false;
		}
	}

	void closeFile() 
	{
		maindictFile.close();
		addedwordsFile.close();
	}

	size_t BKDRHash(string str) 
	{
		size_t hash = 0;
		int size = str.size();
		int idx = 0;
		// calculate the hashvalue by BKDRHash
		for (; idx < size; ++idx) 
		{
			hash = hash * 131 + str[idx];
		}
		return hash;
	}

	bool quadraticInsert(WordNode wordNode) 
	{
		int i, idx;
		// insert word by quadratic hash
		for (i = 0; i < htCapacity; ++i) 
		{
			// get new idx by quadratic hash
			idx = (wordNode.hashValue + i * i) % htCapacity;
			// if type is -1, current node is empty
			if (hashTable[idx].type == -1) 
			{
				hashTable[idx].type = wordNode.type;
				hashTable[idx].word = wordNode.word;
				hashTable[idx].hashValue = wordNode.hashValue;
				hashTable[idx].line = wordNode.line;
				// cout << idx << " " << hashTable[idx].type << " " <<
				// hashTable[idx].word << " "
				//    << hashTable[idx].hashValue << " " << hashTable[idx].line << endl;
				return true;
			} 
			// if find same word, do noting
			else if (hashTable[idx].word == wordNode.word) 
			{
				return false;
			}
		}
		return false;
	}

	int quadraticSearch(string word, size_t hashValue) 
	{
		int i, idx;
		// search hashtable by quadratic hash, if found word,
		// return the index about word, else return -1
		for (i = 0; i < htCapacity; ++i) 
		{
			idx = (hashValue + i * i) % htCapacity;
			if (hashTable[idx].type == -1) 
			{
				return -1;
			} 
			else if (hashTable[idx].word == word) 
			{
				return idx;
			}
		}
		return -1;
	}

	void checkWord(string word) 
	{
		// calculate hashvalue about word
		size_t hashValue = BKDRHash(word);
		// search hashtable by quadratic hash
		int idx = quadraticSearch(word, hashValue);
		if (idx != -1) 
		{
			cout << "ok" << endl;
		} 
		else 
		{
			cout << "don't know \"" << word << "\"" << endl;
		}
	}

	void addWord(string word)
	{
		WordNode wordNode;
		// calculate hashvalue about new word
		wordNode.hashValue = BKDRHash(word);
		// record line about new word
		wordNode.line = addedwordsCount;
		// added word recorded in addedwordsfile, which type is 1
		wordNode.type = 1;
		wordNode.word = word;
		// insert new word
		bool isInsert = quadraticInsert(wordNode);
		// if insert successfully, recorded in addedwordsArray
		if (isInsert) 
		{
			addedwordsArray[addedwordsCount].type = 0;
			addedwordsArray[addedwordsCount].word = word;
			++addedwordsCount;
			++totalWordsCount;
			// also need check load factor about hashtable and addedwordsArray
			if (checkhtLoadFactor())
			{
				increasehtCapacity();
			}
			if (checkArrLoadFactor()) 
			{
				increaseArrCapacity();
			}
		}
		cout << "\"" << word << "\" added to the dictionary" << endl;
	}

	void removeWord(string word) 
	{
		size_t hashValue = BKDRHash(word);
		// search word which needed remove
		int idx = quadraticSearch(word, hashValue);
		if (idx == -1) 
		{
			cout << "can't remove \"" << word << "\": unknown word" << endl;
		} 
		else 
		{
			if (hashTable[idx].type == 0) 
			{
				cout << "can't remove \"" << word << "\": it's in the main dictionary" << endl;
			} 
			else 
			{
				// the number of total words decrease, but addedwordsCount not decrease,
				// addedwordsCount record the nunbers about addedwordArray, 
				//for performance, don't delete word in array.
				--totalWordsCount;
				hashTable[idx].type = -1; //-1 represents empty node
				addedwordsArray[hashTable[idx].line].type = -1; //-1 represents empty node
				cout << "\"" << word << "\" removed" << endl;
			}
		}
	}

	void checkfile(string file) 
	{
		// open new file
		ifstream inputfile;
		inputfile.open(file);
		if (inputfile.is_open()) 
		{
			string word;
			int line = 0;
			while (inputfile.peek() != EOF) 
			{
				inputfile >> word;
				if (inputfile.fail())
					break;
				++line;
				// calculate hashvalue and search in hashtable
				size_t hashValue = BKDRHash(word);
				int idx = quadraticSearch(word, hashValue);
				// if can't find in hashtable, print line in origin file and word.
				if (idx == -1) 
				{
					cout << line << " " << word << endl;
				}
			}
			inputfile.close();
		} 
		else 
		{
			cout << "can't find file \"" << file << "\"" << endl;
		}
	}

	int min3(int a, int b, int c) 
	{
		int tmp = (a < b) ? a : b;
		return (tmp < c) ? tmp : c;
	}

	int getStrDistance(string word1, string word2) 
	{
		// calculate distance by Dynamic programming.
		
		// if i == 0 and j == 0，edit(i, j) = 0
		// if i == 0 and j > 0，edit(i, j) = j
		// if i > 0 and j == 0，edit(i, j) = i
		// if i >= 1 and j >= 1 ，edit(i, j) == min{ edit(i-1, j) + 1, edit(i, j-1)
		// + 1, edit(i-1, j-1) + f(i, j) }
		
		int size1 = word1.size();
		int size2 = word2.size();
		if (size1 - size2 >= -maxStrDistance && size1 - size2 <= maxStrDistance) 
		{
			int dp[205][205]; // 205 is the max length about each word
			for (int i = 0; i <= size1; ++i) 
			{
				dp[i][0] = i;
			}
			for (int i = 0; i <= size2; ++i) 
			{
				dp[0][i] = i;
			}
			for (int i = 1; i <= size1; ++i) 
			{
				for (int j = 1; j <= size2; ++j) 
				{
					int count = 0;
					if (word1[i - 1] != word2[j - 1])
						++count;
					dp[i][j] = min3(dp[i - 1][j - 1] + count, dp[i - 1][j] + 1, dp[i][j - 1] + 1);
				}
			}
			return dp[size1][size2];
		} 
		else 
		{
			return 0;
		}
	}

	void suggestWord(string word) 
	{
		cout << "Suggested words:" << endl;
		int idx = 0;
		// check every word in hashtable, and calculate the editor distance with
		// input word, if the distance less than 2, suggest.
		for (; idx < htCapacity; ++idx) 
		{
			if (hashTable[idx].type != -1) 
			{
				int distance = getStrDistance(word, hashTable[idx].word);
				if (distance == 1) 
				{
					cout << "  " << hashTable[idx].word << endl;
				}
			}
		}
	}

	void updateAddedWords() 
	{
		// cout << maindictCount << " " << addedwordsCount << " " << totalWordsCount << endl;
		// opend addedwordsfile, and write words in addedwordsArray which type is 0, if type is -1, the word has been deleted.
		ofstream outfile;
		outfile.open(addedwords);
		int idx = 0;
		for (; idx < addedwordsCount; ++idx) 
		{
			if (addedwordsArray[idx].type == 0) 
			{
				outfile << addedwordsArray[idx].word << endl;
			}
		}
		outfile.close();
	}

	// judge if load factor about hashtable over 50%
	bool checkhtLoadFactor() 
	{ 
		return (totalWordsCount * 2 > htCapacity); 
	}

	// judge if load factor about addedwordsArray over 50%
	bool checkArrLoadFactor() 
	{ 
		return (addedwordsCount * 2 > arrCapacity); 
	}

	void increasehtCapacity() 
	{
		// record old capacity
		int oldCapacity = htCapacity;
		// new capacity is twice than old capacity
		htCapacity *= 2;
		WordNode *oldHashTable = hashTable;
		// new hashTable
		hashTable = new WordNode[htCapacity];
		int idx = 0;
		for (; idx < oldCapacity; ++idx) 
		{
			if (oldHashTable[idx].type != -1)
				// recalculate hashvalue by quadraticInsert
				quadraticInsert(oldHashTable[idx]);
		}
		// delete old hashTable
		delete[] oldHashTable;
		oldHashTable = NULL;
	}

	void increaseArrCapacity() 
	{
		arrCapacity *= 2;
		AddedWordNode *oldArr = addedwordsArray;
		addedwordsArray = new AddedWordNode[arrCapacity];
		int idx = 0;
		for (; idx < addedwordsCount; ++idx) 
		{
			// assign directly
			addedwordsArray[idx] = oldArr[idx];
		}
		delete[] oldArr;
		oldArr = NULL;
	}

	int storeSingleFile(ifstream &file, int type, int &count) 
	{
		WordNode wordNode;
		while (file.peek() != EOF) 
		{
			file >> wordNode.word;
			if (file.fail())
				break;
			// record the type about each word
			wordNode.type = type;
			// calculate hash value
			wordNode.hashValue = BKDRHash(wordNode.word);
			// record the line in orgin file
			wordNode.line = count;
			// insert word by quadratic hash
			bool isInsert = quadraticInsert(wordNode);
			if (isInsert) 
			{
				if (type == 1) 
				{
					// record the word in addedwordsfile
					addedwordsArray[count].type = 0;
					addedwordsArray[count].word = wordNode.word;
				}
				++count;
				++totalWordsCount;
				// check load factor about hashtable object
				if (checkhtLoadFactor()) 
				{
					// increase capacity about hashtable
					increasehtCapacity();
				}
					// check load factor about addedwordsArray
				if (checkArrLoadFactor()) 
				{
					// increase capacity about array
					increaseArrCapacity();
				}
			}
		}
		return count;
	}

	bool storeWords() 
	{
		// store maindictFile, type is 0
		storeSingleFile(maindictFile, 0, maindictCount);
		// store addedwordsCount, type is 1
		storeSingleFile(addedwordsFile, 1, addedwordsCount);
		// the number of total words
		totalWordsCount = maindictCount + addedwordsCount;
		// cout << maindictCount << " " << addedwordsCount << " " << totalWordsCount << endl;
		return true;
	}

private:
	ifstream maindictFile;
	ifstream addedwordsFile;
	int maindictCount;
	int addedwordsCount;
	int totalWordsCount;
	int htCapacity;     // the capacity of the hashtable, first initialized to 100
	int arrCapacity;    // the capacity of the array which contains added words,first initialized to 30
	int maxStrDistance; // the maxmum value about string distance, first initialized to 1
	WordNode *hashTable;
	AddedWordNode *addedwordsArray;
};

class InputOperate 
{
public:
	InputOperate(HashTable &hashTable) : hashTable(hashTable) {}

	//split the input line, get command and word
	void splitStr(string input, string &command, string &word) 
	{
		int size = input.size(), pre = 0;
		// get index about first space
		int idx = input.find_first_of(' ');
		// get command
		command = input.substr(pre, idx - pre);
		pre = idx + 1;
		// get word
		word = input.substr(pre, size - pre);
	}

	bool checkInput(string command, string word) 
	{
		// check all kind of command, then operate word
		if (command == "check") 
		{
			hashTable.checkWord(word);
		} 
		else if (command == "add") 
		{
			hashTable.addWord(word);
		} 
		else if (command == "remove") 
		{
			hashTable.removeWord(word);
		} 
		else if (command == "checkfile") 
		{
			hashTable.checkfile(word);
		} 
		else if (command == "suggest") 
		{
			hashTable.suggestWord(word);
		} 
		else if (command == "quit" || command == "stop" || command == "end" || command == "done") 
		{
			// if input end command, return true
			return true;
		}
		else 
		{
			cout << "unknown command, please check your command." << endl;
		}
		return false;
	}

	void handleInput() 
	{
		string input, command, word;
		cout << "Welcome to Easy Spell Checker!" << endl << endl;
		while (getline(cin, input)) 
		{
			// split each input line, get command and word
			splitStr(input, command, word);
			// check input by command and word
			bool isQuit = checkInput(command, word);
			if (isQuit) 
			{
				break;
			}
			cout << endl;
		}
		// close file
		hashTable.closeFile();
		// after loop, update AddedWordsfile, contains words which are removed or added
		hashTable.updateAddedWords();
	}

private:
	HashTable &hashTable;
};

int main() 
{
	// the object about HashTable class
	HashTable hashTable;
	// open two files
	bool isOpen = hashTable.openFile(maindict, addedwords);
	if (isOpen) 
	{
		InputOperate inputOperate(hashTable);
		// store words
		hashTable.storeWords();
		// operate input by user
		inputOperate.handleInput();
	}
	return 0;
}
