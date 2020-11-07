#include <condition_variable>
#include <functional>
#include <iostream>
#include <future>
#include <vector>
#include <thread>
#include <queue>
#include <iostream>
#include <fstream>
#include <string>
#include <map>


using namespace std;
//int threadNumber = 0;

//used to ensure fileName that references file to be opened only changes after the current encrypting thread has opened its file and begun reading
condition_variable fileWait;
mutex fileMut;
bool changeFileName = true;

//defines map to be used to encrypt date
map<char, char> scrambler;

class threadPool {
public:
    using Task = function<void(string fileName)>;

    
    explicit threadPool(size_t numThreads)
    {
        start(numThreads);
    }
    ~threadPool()
    {
        stop();
    }

    void enqueue(Task task)
    {
        {
            unique_lock<mutex> lock{ mEventMutex };
            mTasks.emplace(move(task));
        }
        mEventVar.notify_one();
    }

    void setNames(queue<string> namesOfFiles)
    {
        fileNames = namesOfFiles;
    }

private:
    vector<thread> mThreads;
    string nextFile;

    condition_variable mEventVar;
    mutex mEventMutex;
    bool mStopping = false;
    
    queue<Task> mTasks;
    queue<string> fileNames;

    void start(size_t numThreads)
    {
        for (auto i = 0u; i < numThreads; i++)
        {
            mThreads.emplace_back([=] {
                while (true)
                {
                    Task task;

                    {
                        unique_lock<mutex> lock{ mEventMutex };
                        mEventVar.wait(lock, [=] {return mStopping || !mTasks.empty() ;});
                        
                        if (mStopping && mTasks.empty())
                            break;

                        task = move(mTasks.front());
                        mTasks.pop();
                        
                        {
                            unique_lock<mutex> fileLock{ fileMut };
                            fileWait.wait(fileLock, [] { return changeFileName;});
                            nextFile = fileNames.front();
                            fileNames.pop();
                        }

                        changeFileName = false;
                    }
                    //cout << nextFile << endl;
                    task(nextFile);
                }
            });
               
        }
    }

    void stop() noexcept
    {
        {
            unique_lock<mutex> lock{ mEventMutex };
            mStopping = true;
        }

        mEventVar.notify_all();

        for (auto& thread : mThreads) {
            thread.join();
        }
    }
};
    
void encrypt(string fileName) { 
    //threadNumber++;
    //cout << threadNumber << endl;
    //cout << fileName << endl;
    string encName = fileName.substr(0, fileName.size()-4) + "_Encrypted.txt";
    
    fstream testFile(fileName, ios::in);
    fstream testFileOut(encName, ios::out);

    changeFileName = true;
    fileWait.notify_all();

    if (!testFile) {
        cerr << "Error opening file" << endl;
        exit(1);
    }

    string item;

    while (true) {
        getline(testFile, item);

        for (string::size_type i = 0; i < item.size(); i++) {
            item[i] = scrambler[item[i]];
        }

        testFileOut << item << endl;

        if (testFile.eof()) break;

    }

    testFile.close();
    testFileOut.close();
    cout << encName << "done" << endl;

};

int main()
{
    //defines list of file names and initializes thread pool
    queue<string> filesToEncrypt;
    threadPool thisPool(2);
    string fileAdd;

    //defines map used to scramble data
    scrambler['a'] = '.';
    scrambler['b'] = 'P';
    scrambler['b'] = 'O';
    scrambler['d'] = '0';
    scrambler['e'] = 'Z';
    scrambler['f'] = 'k';
    scrambler['g'] = 'A';
    scrambler['h'] = 'r';
    scrambler['i'] = 'q';
    scrambler['j'] = '7';
    scrambler['k'] = '3';
    scrambler['l'] = 'k';
    scrambler['m'] = 'm';
    scrambler['n'] = 'N';
    scrambler['o'] = 'M';
    scrambler['p'] = 'O';
    scrambler['q'] = 'Q';
    scrambler['r'] = 'd';
    scrambler['s'] = 'f';
    scrambler['t'] = 'j';
    scrambler['u'] = 'H';
    scrambler['v'] = '1';
    scrambler['w'] = 'h';
    scrambler['x'] = '!';
    scrambler['y'] = 'c';
    scrambler['z'] = 'V';
    scrambler['A'] = 'X';
    scrambler['B'] = 'i';
    scrambler['C'] = '-';
    scrambler['D'] = '+';
    scrambler['E'] = '=';
    scrambler['F'] = '?';
    scrambler['G'] = '/';
    scrambler['H'] = 's';
    scrambler['I'] = 'S';
    scrambler['J'] = 'g';
    scrambler['K'] = 'G';
    scrambler['L'] = 'o';
    scrambler['M'] = '(';
    scrambler['N'] = ')';
    scrambler['O'] = '*';
    scrambler['P'] = '&';
    scrambler['Q'] = '^';
    scrambler['R'] = '%';
    scrambler['S'] = '$';
    scrambler['T'] = '#';
    scrambler['U'] = '@';
    scrambler['V'] = '!';
    scrambler['W'] = '~';
    scrambler['X'] = 'l';
    scrambler['Y'] = 'L';
    scrambler['Z'] = 'Y';
    scrambler['.'] = 'R';
    scrambler['!'] = 'F';
    scrambler['?'] = '/';
   
    
    cout << "Please enter a list of files to be encrypted, enter START when you want the process to begin" << endl;
    
    while (true) {
        if (fileAdd == "START")
            break;
       
        cin >> fileAdd;
        filesToEncrypt.push(fileAdd);
    }

    thisPool.setNames(filesToEncrypt);

    for (int i = 0; i < filesToEncrypt.size()-1; i++)
    {
        //cout << filesToEncrypt.size() << endl; 
        thisPool.enqueue(encrypt);
    }

    return 0;
}