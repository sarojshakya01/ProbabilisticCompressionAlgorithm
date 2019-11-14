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

#include <unistd.h> 

#include <pthread.h>

#include <sys/socket.h> 

#include <arpa/inet.h> 

#include <ctime>

#include <netdb.h>

#define maxsize 100

using namespace std;

int size = 0;
char const * s_IP;
int port = 0;
char *ps;
char inbuff[1000];
char remMsg[maxsize] = {0};

struct elements {
  char key;
  char code[maxsize] = {0};
  int value;
};

struct elements myarray[maxsize];

void swap(elements * xp, elements * yp) {
  elements temp = * xp;
  * xp = * yp;
  * yp = temp;
}

void selectionSort(elements arr[], int n) {
  int min_idx = 0;

  // One by one move boundary of unsorted subarray  
  for (int i = 0; i < n - 1; i++) {
    
    // Find the minimum element in unsorted array  
    min_idx = i;
    for (int j = i + 1; j < n; j++)
      if (arr[j].value > arr[min_idx].value) {
        min_idx = j;
      }
    else if (arr[j].value == arr[min_idx].value) {
      if (arr[j].key < arr[min_idx].key) {
        min_idx = j;
      }
    }

    // Swap the found minimum element with the first element  
    swap(&arr[min_idx], &arr[i]);
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

int cSD = 0;

int findIndex(char ch, const char *str) {
  int i = 0, index = -1;
  while(str[i] != '\0'){
      if (ch == str[i]){
          index = i;
          break;
      }
      i++;
  }
  return index;
}

void serverConf() {
    sockaddr_in ser_add;
     
    struct hostent* host = gethostbyname(s_IP);

    bzero((char*)&ser_add, sizeof(ser_add));

    ser_add.sin_family = AF_INET;
    ser_add.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    ser_add.sin_port = htons(port);
    cSD = socket(AF_INET, SOCK_STREAM, 0);
    if (cSD < 0) {
        perror("Socket Failed");
        exit(EXIT_FAILURE); // terminate with error
    }      
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, s_IP, &ser_add.sin_addr) <=0) { 
        perror("Invalid IP"); 
        exit(EXIT_FAILURE); // terminate with error
    } 

    if (connect(cSD, (struct sockaddr *)&ser_add, sizeof(ser_add)) < 0) { 
        perror("Connection Failed"); 
        exit(EXIT_FAILURE); // terminate with error
    }
}

int threadCount = 0;

void * childFunc(void* ch) {
    int val_read = 0;

    serverConf();
    
    if (threadCount == 0){
        send(cSD, ps, size, 0);
    }
    else {
        send(cSD, ch, 1, 0);
        memset(&inbuff, 0, sizeof(inbuff));//clear the inbuff
        
        memset(&remMsg, 0, sizeof(remMsg));//clear the remMsg

        val_read = recv(cSD, (char*)&inbuff, sizeof(inbuff), 0);
        
        int code_end = findIndex((char)0x01, inbuff);
        int j = 0;
        
        for (int i = 0; i < strlen(inbuff); i++) {
            
            if (i < code_end) {
                myarray[threadCount - 1].code[i] = inbuff[i];
            }
            else if (i > code_end){
                remMsg[j] = inbuff[i];
                j++;
            }
        }
    }
    
    threadCount++;
    
    close(cSD);
}

char * replace_EOL(const char *str) {
    int i = 0, j = 0;
    static char new_string[maxsize];
    memset(new_string, 0, sizeof(new_string)); // reset the array;
    while(str[i] != '\0'){
        if (str[i] == '\n'){
            new_string[j] = '<';
            j++;
            new_string[j] = 'E';
            j++;
            new_string[j] = 'O';
            j++;
            new_string[j] = 'L';
            j++;
            new_string[j] = '>';
        } else {
            new_string[j] = str[i];
        }
        i++;
        j++;
    }
    return new_string;
}

int main(int argc, char const *argv[]) {
  if (argc != 3) {
    cout << "Please Enter valid parameters" << endl;
    exit(1);
  }

  s_IP = argv[1];
  port = stoi(argv[2]);

  string s = "";
  char c;
  
  while(cin.get(c)) {
    s+= c;
  }
  
  size = s.size();
  
  char arr[size + 1] = {0};
  char carr[size + 1] = {0};
  
  int i = 0;
  while (s[i] != '\0') {
    arr[i] = s[i];
    carr[i] = s[i];
    i++;
  }

  ps = arr;

  elements records[size];

  int count = 0;

  for (int i = 0; i < size; i++) {

    if (!alreadyVisited(carr[i], records, count)) {
      records[count].key = carr[i];
      records[count].value = 1;

      count++;
    } else {
      int e = whichElement(carr[i], records, count);
      int a = records[e].value;
      (records[e].value) = a + 1;
    }
  }

  selectionSort(records, count);

  for (int i = 0; i < count; i++) {
    myarray[i].key = records[i].key;
    myarray[i].value = records[i].value;

    if (myarray[i].key == '\n'){
      cout << "<EOL>" << " frequency = " << myarray[i].value << '\n';
    } else {
      cout << myarray[i].key << " frequency = " << myarray[i].value << endl;
    }
  }

  pthread_t td[count];
  
  cout << "Original Message:   " << replace_EOL(ps) << '\n';
  
  for (int i = 0; i <= count; i++) {
    
    pthread_create(&td[i], NULL, &childFunc, (void*)&myarray[i-1].key);
    
    pthread_join(td[i],NULL);
    
    // if (i == 1){
    //   cout << "Remaining Message:  " << replace_EOL(remMsg) << '\n';
    // }

    if (i > 0){
      if (myarray[i-1].key == '\n') {
          cout << "Symbol " << "<EOL>" <<" code:  " << myarray[i-1].code << endl;
        } else {
          cout << "Symbol " << myarray[i-1].key <<" code:      " << myarray[i-1].code << endl;
        }
      if (i != count)
      cout << "Remaining Message:  " << replace_EOL(remMsg) << '\n';
    }
  }

  ofstream outputFile;
  outputFile.open("./output.txt");
  
  if(outputFile) {
    for (int i = 0; i < count; i++) {
      outputFile << myarray[i].code <<'\n';
    }
  }

  outputFile.close();
  exit(EXIT_SUCCESS);
  return 0;
}