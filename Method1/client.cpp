#include <iostream> 
#include <unistd.h> 
#include <fstream>
#include <sys/wait.h>
#include <cstring>
#include <pthread.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <ctime>
#include <netdb.h>

using namespace std; 

#define max_size 100
#define EOL "<EOL>"

string generated_code[max_size];

char const * message;
char const * server_IP;
int msg_length = 0;
int symbol_count = 0;
int client_SD = 0;
int uniq_count = 0;
uint16_t port = 0;
int freq_list[max_size]={0};
char buffer[1024] = {0};
char code[max_size] = {0};
char rem_msg[max_size] = {0};
char msg[max_size]={0};
char uniq_msg[max_size]={};

// function to find the index in array
int indexOf(char ch, const char *str) {
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

// function to get the formatted string
char * replace_EOL(const char *str) {
    int i = 0, j = 0;
    static char new_string[max_size];
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

// function to sort the characters in uniq array
void sort() {
    
    for (int i = 0; i < uniq_count; i++){
        for (int j = i+1; j < uniq_count; j++)
        {
            if (freq_list[j] > freq_list[i]){
                int temp_freq = freq_list[i];
                char temp_ch = uniq_msg[i];
                
                freq_list[i] = freq_list[j];
                uniq_msg[i] =  uniq_msg[j];
                
                freq_list[j] = temp_freq;
                uniq_msg[j] = temp_ch;
                
            } else if (freq_list[j] == freq_list[i]){
                if (uniq_msg[j] < uniq_msg[i]){
                    int temp_freq = freq_list[i];
                    char temp_ch = uniq_msg[i];
                    
                    freq_list[i] = freq_list[j];
                    uniq_msg[i] =  uniq_msg[j];

                    freq_list[j] = temp_freq;
                    uniq_msg[j] = temp_ch;
                }
            }
        }
    }
}

// function to connect the server using socket
void connect_server() {
    sockaddr_in server_address;
     
    struct hostent* host = gethostbyname(server_IP);

    client_SD = socket(AF_INET, SOCK_STREAM, 0);
    bzero((char*)&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    server_address.sin_port = htons(port);

    if (client_SD < 0) {
        perror("Socket Failed");
        exit(EXIT_FAILURE); // terminate with error
    }      
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    int validIP = inet_pton(AF_INET, server_IP, &server_address.sin_addr);
    if(validIP <=0) { 
        perror("Invalid IP"); 
        exit(EXIT_FAILURE); // terminate with error
    } 

    int status = connect(client_SD, (struct sockaddr *)&server_address, sizeof(server_address));
    if (status < 0) { 
        perror("Connection Failed"); 
        exit(EXIT_FAILURE); // terminate with error
    }
}

// child thread
void * child_thread(void* symbol) {
    int val_read = 0;

    connect_server();
    
    if (symbol_count == 0){
        send(client_SD, message, strlen(message), 0);
    }
    else {
        send(client_SD, symbol, 1, 0);
        memset(&buffer, 0, sizeof(buffer));//clear the buffer
        memset(&code, 0, sizeof(code));//clear the code
        memset(&rem_msg, 0, sizeof(rem_msg));//clear the rem_msg

        val_read = recv(client_SD, (char*)&buffer, sizeof(buffer), 0);
        
        int code_end = indexOf((char)0x01, buffer);
        int j = 0;
        
        for (int i = 0; i < strlen(buffer); i++) {
            
            if (i < code_end) {
                code[i] = buffer[i];
            }
            else if (i > code_end){
                rem_msg[j] = buffer[i];
                j++;
            }
            
        }
        
        generated_code[symbol_count - 1] =  code;
    }
    
    symbol_count++;
    
    close(client_SD);
}

int main(int argc, char const *argv[]) {
    
    if (argc != 3) {
        cout << "Invalid parameters" << endl;
        exit(EXIT_FAILURE); // terminate with error
    }

    server_IP = argv[1];
    port = stoi(argv[2]);
    
    int i = 0;
    while(i != max_size)
    {
        cin.get(msg[i]);
        if (msg[i] == '\0')
            break;
        i++;
    }
    
    message = msg;

    msg_length = strlen(msg); // no. of characters in the message

    // identify unique characters and find their frequency
    i = 0;
    
    while (msg[i] != '\0'){
        
        if (msg[i] != '\0' && indexOf(msg[i],uniq_msg) == -1){
            
            uniq_msg[uniq_count] = msg[i];
            int j = 0;
            while (msg[j] != '\0'){
                if ((msg[i] == msg[j])){
                    freq_list[uniq_count]++;
                }
                j++;
            }

            uniq_count++;
        }
        i++;
    }
    
    cout << endl;
    
    sort();

    // Displaying Unique characters
    for (int i = 0; i < uniq_count; i++){
        if (uniq_msg[i] == '\n') {
            cout << EOL <<" frequency = "<<freq_list[i] << '\n'; 
        } else {
            cout << uniq_msg[i] <<" frequency = "<<freq_list[i] << '\n'; 
        }
    }
    
    pthread_t t[uniq_count]; //declare theread

    for (int i = 0; i <= uniq_count; i++){
        
        if (i == 1) {
            cout << "Original Message: " << replace_EOL(message) << endl;
        } else if (i > 1) {
            cout << "Remaining Message: " << replace_EOL(rem_msg) << endl;
        }
        
        pthread_create(&t[i], NULL, &child_thread, (void*)&uniq_msg[i-1]);
        
        pthread_join(t[i],NULL);
        
        if (i > 0){
            if (uniq_msg[i-1] == '\n') {
                cout << "Symbol "<< EOL <<" code: " << generated_code[i-1] << endl;
            } else {
                cout << "Symbol "<<uniq_msg[i-1]<<" code: " << generated_code[i-1] << endl;
            }
        }
    }    

    // writes the code to the file
    ofstream outfile;
    outfile.open("./Output.txt");

    if (!outfile) {
        perror("Unable to open file");
        exit(EXIT_FAILURE); // terminate with error
    }

    for (int i = 0; i < uniq_count; i ++){
        outfile << generated_code[i]<<endl;    
    }
    
    outfile.close();

    exit(EXIT_SUCCESS);
    
    return 0; 
} 
