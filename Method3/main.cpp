#include <iostream> 
#include <unistd.h> 
#include <fstream>
#include <sys/wait.h>
#include <cstring>
using namespace std; 

#define maxsize 100

bool elemPresent(const char *str, char ch){
    bool exist = false;
    int i = 0;
    while(str[i] != '\0'){
        if (str[i] == ch){
            exist = true;
            break;
        }
        i++;
    }
    if (exist){
        return true;
    } else {
        return false;
    }
}

void printArray(const char *str){
    int i = 0;
    while(str[i] != '\0'){
        cout << str[i];
        i++;
    }
}

int lengthArray(const char *str){
    int i = 0;
    while(str[i] != '\0'){
       i++;
    }
    return i;
}

char * generatateCode(char ch, const char *str){
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

char * getCode(char ch, const char *str){
    int i = 0;
    int count = 0;
    static char code[maxsize];
    memset(code, 0, sizeof(code)); // reset the array;
    while(str[i] != '\0'){
        if (str[i] == ch && str[i+1] == ':'){
            for (int x = i+2; x < maxsize; x++)
            {
                if (str[x] == ';')
                    break;

                code[count] = str[x];
                count++;

            }
            
        }
        i++;
    }
    return code;
}
// Driver code 
int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cout << "Invalid parameters" << std::endl;
        exit(EXIT_FAILURE); // terminate with error
    }

    // char infilename[100] = "/home/sshakya/Documents/PratapAssignment/ProcessFork/Input.txt";
    
    ifstream infile;
    
    // infile.open(infilename);
    infile.open(argv[1]);

    if (!infile) {
        cout << "Unable to open file\n";
        exit(EXIT_FAILURE); // terminate with error
    }

    char str[maxsize];
    char uniqStr[maxsize]={};
    int countList[maxsize]={0};

    int n = sizeof(str)/sizeof(str[0]);

    
    infile >> str;
    
    infile.close();

    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t id = fork();

    if (id == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // Child process
    if (id == 0) {
        close(pipefd[1]); /* Close unused write end */
        char fullMsg[maxsize];
        char uniqMsg[maxsize];
        read(pipefd[0], fullMsg, sizeof(fullMsg));
        read(pipefd[0], uniqMsg, sizeof(uniqMsg));

        ofstream outfile;
        outfile.open("/home/sshakya/Documents/PratapAssignment/ProcessFork/Output.txt");
        
        int j = 0;
        while (fullMsg[j] != '\0'){
            j++;
        }
        int length = j;

        int i = 0;
        char tempArr[maxsize];
        memset(tempArr, 0, sizeof(tempArr));
        copy(begin(fullMsg), end(fullMsg), begin(tempArr));
        
        while (uniqMsg[i] != '\0'){
            
            outfile <<uniqMsg[i] << ":" << generatateCode(uniqMsg[i], tempArr) << ';';

            char remMsg[maxsize];
            memset(remMsg, 0, sizeof(remMsg));

            int remCount = 0;
            
            for (j = 0; j < length; j++){
                if (uniqMsg[i] != tempArr[j]){
                    remMsg[remCount] = tempArr[j];
                    remCount++;
                }
            }
            length = remCount;
            memset(tempArr, 0, sizeof(tempArr));
            copy(begin(remMsg), end(remMsg), begin(tempArr));

            i++;
        }
        
        outfile.close();
        close(pipefd[0]);
        _exit(EXIT_SUCCESS);
    } else { // Parent process
        close(pipefd[0]); /* Close unused read end */

        int uniqCount = 0, charCount = 0;
        while (str[charCount] != '\0'){

            if (str[charCount] != '\0' && !elemPresent(uniqStr, str[charCount])){
                
                uniqStr[uniqCount] = str[charCount];
                int j = 0;
                while (str[j] != '\0'){
                    if ((str[charCount] == str[j])){
                        countList[uniqCount]++;
                    }
                    j++;
                }

                uniqCount++;
            }
            charCount++;
        }
        
        // Sort Unique characters
        for (int i = 0; i < uniqCount; i++){
            for (int j = i+1; j < uniqCount; j++)
            {
                if (countList[j] > countList[i]){
                    int temp = countList[i];
                    char ch = uniqStr[i];
                    countList[i] = countList[j];
                    uniqStr[i] = uniqStr[j];
                    countList[j] = temp;
                    uniqStr[j] = ch;
                } else if (countList[j] == countList[i]){
                    if (uniqStr[j] < uniqStr[i]){
                        int temp = countList[i];
                        char ch = uniqStr[i];
                        countList[i] = countList[j];
                        uniqStr[i] = uniqStr[j];
                        countList[j] = temp;
                        uniqStr[j] = ch;
                    }
                }
                
            }
        }

        cout << '\n';
        // Displaying Unique characters
        for (int i = 0; i < uniqCount; i++){
            cout << uniqStr[i] <<" frequency = "<<countList[i] << '\n'; 
        }
        
        write(pipefd[1], str, sizeof(str));
        write(pipefd[1], uniqStr, sizeof(uniqStr));
        close(pipefd[1]); /* Reader will see EOF */

        wait(NULL); /* Wait for child */

        infile.open("/home/sshakya/Documents/PratapAssignment/ProcessFork/Output.txt");

        if (!infile) {
            cout << "Unable to open file\n";
            exit(EXIT_FAILURE); // terminate with error
        }

        char code[maxsize];
        infile >> code;
        
        infile.close();

        cout <<"\nOriginal Message: " << str <<'\n';
        
        char tempArr[maxsize];
        memset(tempArr, 0, sizeof(tempArr));
        copy(begin(str), end(str), begin(tempArr));

        for (int i = 0; i < uniqCount; i++){
            for (int j = 0; j < charCount; j++){
                cout << "Symbol " << uniqStr[i] << " code: " << getCode(uniqStr[i], code) << '\n';
                char remMsg[maxsize];
                memset(remMsg, 0, sizeof(remMsg));
                int remCount = 0;
            
                for (j = 0; j < charCount; j++){
                    if (uniqStr[i] != tempArr[j]){
                        remMsg[remCount] = tempArr[j];
                        remCount++;
                    }
                }
                charCount = remCount;
                memset(tempArr, 0, sizeof(tempArr));
                copy(begin(remMsg), end(remMsg), begin(tempArr));
                cout << "Remaining Message: " << remMsg << '\n';
            }
        }
        

        exit(EXIT_SUCCESS);
    } 
  
    
    return 0; 
}