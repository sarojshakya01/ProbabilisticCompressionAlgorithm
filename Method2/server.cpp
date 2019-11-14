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

#include <sys/socket.h>

#include <netinet/in.h>


#define maxsize 100

using namespace std;

int size = 0;
int port = 0;
int sSD = 0;
char inbuff[1000];

sockaddr_in servAdd;

struct elements {
  char key;
  char code[maxsize] = {0};
  int value;
};

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

void connClient() {
    
    int a = 1;

    bzero((char*)&servAdd, sizeof(servAdd));
    
    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    servAdd.sin_port = htons(port);

    int addrlen = sizeof(servAdd);
    
    // Creating socket file descriptor
    sSD = socket(AF_INET, SOCK_STREAM, 0);
    if (sSD <= 0) {
        cout<<"Socket Failed!\n";
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port
    if (setsockopt(sSD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &a, sizeof(a)) < 0) {
        cout<<"Set Socket Failed\n";
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the servAdd
    if (bind(sSD, (struct sockaddr *)&servAdd, sizeof(servAdd)) < 0) {
        cout<<"Bind Failed\n";
        exit(EXIT_FAILURE);
    }

    cout << "Waiting client" << '\n';

    if (listen(sSD, 5) < 0) {
        cout<<"Listen Failed\n";
        exit(EXIT_FAILURE);
    }
}

int callCount = 0;

char * getCode(char ch, const char *str) {
    int i = 0;
    static char code[maxsize];
    memset(code, 0, sizeof(code)); // reset the array;
    while(str[i] != '\0'){
        if (ch == str[i]){
            code[i] = '1';
        } else {
            code[i] = '0';
        }
        
        i++;
    }
    return code;
}

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    cout << "Please Enter valid parameters" << '\n';
    exit(1);
  }

  port = stoi(argv[1]);
  char str[maxsize]={0};
  int count = 0;
  int newSckt;
  int distictChar = 0;
  elements myarray[size];
  elements records[size];
  
  while (1) {
    if (callCount == 0) connClient();
    sockaddr_in newSAddr;
    socklen_t newSAddrSize = sizeof(newSAddr);
    
    //accept, create a new socket descriptor to handle the new connection with client
    newSckt = accept(sSD, (struct sockaddr *)&servAdd, &newSAddrSize);
    if (newSckt < 0) {
        perror("Accept Failed");
        exit(EXIT_FAILURE);
    }

    if (callCount == 0) cout << "Client Connection Success!" << '\n' << '\n';

    pid_t pid;

    pid = fork();

    //Child Process
    if (pid == 0) {

      memset(&inbuff, 0, sizeof(inbuff));
      recv(newSckt, (char*)&inbuff, sizeof(inbuff), 0);

      char ch;
      int i = 0, j = 0;

      if (callCount == 0) {
        while(inbuff[i] != '\0') {
          str[i] = inbuff[i];
          i++;
        }
        size = i;

        for (int i = 0; i < size; i++) {

          if (!alreadyVisited(str[i], records, count)) {
            records[count].key = str[i];
            records[count].value = 1;

            count++;
          } else {
            int e = whichElement(str[i], records, count);
            int a = records[e].value;
            (records[e].value) = a + 1;
          }
        }
        distictChar = count;
      } else {
        ch = inbuff[0];

        char * scode;
        scode = getCode(ch, str);
        
        int count2 = 0;
        
        char temp_str[maxsize];
        memset(temp_str, 0, sizeof(temp_str));
        
        for (int i = 0; i < size; i++){
            
            if (str[i] != ch){

                temp_str[count2] = str[i];
                
                count2++;
            }
        }
        
        size = count2;
        memset(str, 0, sizeof(str));
        copy(begin(temp_str), end(temp_str), begin(str));
        
        send(newSckt, scode, strlen(scode), 0);

        char const *pchr1;
        char const chr1 = char(1);
        pchr1 = &chr1;
        send(newSckt, pchr1, 1, 0);
        send(newSckt, str, strlen(str), 0);

      }
      
    } else { // Parent Process
      if (callCount == 1) {
        selectionSort (records, distictChar);
      }
      
      wait(NULL); /* Wait for child */
      exit(EXIT_SUCCESS);

    }
    
    callCount++;

    if (callCount > distictChar) {
      break;
    }
    
  }
  cout << "Processing Finished!" << endl;
  close(newSckt);
  close(sSD);
  return 0;

}