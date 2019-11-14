#include <iostream>

#include <sstream>

#include <cstring>

#include <string>

#include <algorithm>

#include <map>

#include <unistd.h>

#include <fstream>

#include <sys/types.h>

#include <sys/wait.h>

using namespace std;

struct elements {
  char key;
  int value;
};

void swap(elements * xp, elements * yp) {
  elements temp = * xp;
  * xp = * yp;
  * yp = temp;
}

void selectionSort(elements arr[], int n) {
  int i, j, min_idx;

  // One by one move boundary of unsorted subarray  
  for (i = 0; i < n - 1; i++) {
    // Find the minimum element in unsorted array  
    min_idx = i;
    for (j = i + 1; j < n; j++)
      if (arr[j].value > arr[min_idx].value) {
        min_idx = j;
      }
    else if (arr[j].value == arr[min_idx].value) {
      if (arr[j].key < arr[min_idx].key) {
        min_idx = j;
      }
    }
    // Swap the found minimum element with the first element  

    swap( & arr[min_idx], & arr[i]);

  }
}

bool alreadyVisited(char x, elements r[], int n) {
  for (int i = 0; i < n; i++) {
    if (r[i].key == x) {

      return true;
    }
  }
  return false;
}

int whichElement(char x, elements r[], int n) {
  for (int i = 0; i < n; i++) {
    if (r[i].key == x) {
      return i;
    }
  }
  return -1;
}

int deleteElement(elements arrX[], int x, int & n, char arr[]) {
  // Search x in array 

  int i;
  int temp;
  temp = 0;

  while (temp <= n) {
    if (arr[temp] == arrX[x].key) {

      // reduce size of array and move all 
      // elements on space ahead 

      for (int j = temp; j < n; j++) {
        arr[j] = arr[j + 1];
      }

      n = n - 1;

    } else {
      temp++;
    }

  }
  return n;
}

int main(int argc, char const *argv[]) {
  if (argc != 3) {
    cout << "Please Enter valid parameters" << endl;
    exit(1);
  }

  pid_t pid;
  char c;
  string s = "";


  while(cin.get(c))
  {
      if(c == '\n')
      {
          s+= "<EOL>";
      }
      else
      {
          s+= c;
      }

  }
  cout<<s<<endl;
  exit(1);
  // // string file;
  // // getline(cin, file, '\0');
  // // string s = file;

  // string s = "aaaannnaaakkllaaaaap";
  int n = s.size();
  elements records[n];
  char cstr[s.size()];
  copy(s.begin(), s.end(), cstr);
  int count = 0;

  for (int i = 0; i < n; i++) {
    if (!alreadyVisited(cstr[i], records, count)) {
      records[count].key = cstr[i];
      records[count].value = 1;
      //cout<<"I am inside loop"<<records[count].key<<" : "<<records[count].value<<endl;
      count++;
    } else {
      int e = whichElement(cstr[i], records, count);
      int a = records[e].value;
      (records[e].value) = a + 1;
      //cout<<cstr[i]<<" "<<records[e].value<<endl;
    }
  }

  selectionSort(records, count);

  elements array[count];
  for (int i = 0; i < count; i++) {
    array[i].key = records[i].key;
    array[i].value = records[i].value;
    cout << array[i].key << " frequency = " << array[i].value << endl;
  }

  int k = 0;
  char arr[n];
  for (int i = 0; i < n; i++) {
    arr[i] = cstr[i];
  }

  // ofstream oFile;
  // oFile.open("output.txt");

  for (int i = 0; i < count; i++) {
    pid = fork();
    if (pid == 0) {

      cout << "Original Message:       " << cstr << endl;
      cout << "Symbol " << array[i].key << " code:    ";
      for (int j = 0; j < n; j++) {
        if (cstr[j] == array[i].key) {
          cout << "1";
        } else {
          cout << "0";
        }

      }
      cout << endl;
      _exit(0);

    }
    deleteElement(array, i, n, cstr);
    wait(0);

  }
  for (int i = 0; i < count; i++) {
    wait(0);
  }

  // string out ;

  //  oFile.close();

  // ifstream outputFile;
  // oFile.open("output.txt");
  // string line;
  // if(oFile.is_open())
  // {

  //     while (!oFile.eof())
  //     {
  //         getline(cin, out);
  //         cout << out << endl;

  //     }
  //     // while ( getline (outputFile,line) )
  //     // {
  //     //   cout << line << '\n';
  //     // }

  // }

  // oFile.close();

  return 0;
}