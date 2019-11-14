#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sys/wait.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

#define max_size 100

string generated_code[max_size];
sockaddr_in server_address;

int symbol_count = 0;
int server_SD = 0;
int uniq_count = 0;
int msg_length = 0;
uint16_t port = 0;

int freq_list[max_size]={0};
char buffer[1024] = {0};
char uniq_msg[max_size]={0};
char rem_msg[max_size]={0};
char *sorted_msg;

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

// function to find uniq characters in array and their freq
void find_uniq_and_freq(char *str){
    int i = 0, j = 0;

    while (str[i] != '\0'){

        if (str[i] != '\0' && indexOf(str[i],uniq_msg) == -1){
            
            uniq_msg[j] = str[i];

            int k = 0;
            while (str[k] != '\0'){
                if (str[i] == str[k]){
                    freq_list[j]++; // count the frequency
                }
                k++;
            }

            j++;
        }
        i++;
    }
}

// function to generate the compressed code of character
char * generate_code(char ch, const char *str) {
    int i = 0;
    static char code[max_size];
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

// function to get the remaining message
void rem_message(char ch){
    int rem_count = 0;
    
    char temp_msg[max_size];
    memset(temp_msg, 0, sizeof(temp_msg));
    
    for (int i = 0; i < msg_length; i++){
        
        if (rem_msg[i] != ch){

            temp_msg[rem_count] = rem_msg[i];
            
            rem_count++;
        }
    }
    
    msg_length = rem_count;
    memset(rem_msg, 0, sizeof(rem_msg));
    copy(begin(temp_msg), end(temp_msg), begin(rem_msg));
}

// function to sort the unique characters
void sort() {
    
    for (int i = 0; i < uniq_count; i++){
        for (int j = i+1; j < uniq_count; j++)
        {
            if (freq_list[j] > freq_list[i]){
                int temp_freq = freq_list[i];
                char temp_ch = sorted_msg[i];
                string temp_code = generated_code[i];
                
                freq_list[i] = freq_list[j];
                sorted_msg[i] =  sorted_msg[j];
                generated_code[i] = generated_code[j];
                
                freq_list[j] = temp_freq;
                sorted_msg[j] = temp_ch;
                generated_code[j] = temp_code;
                
            } else if (freq_list[j] == freq_list[i]){
                if (sorted_msg[j] < sorted_msg[i]){
                    int temp_freq = freq_list[i];
                    char temp_ch = sorted_msg[i];
                    string temp_code = generated_code[i];
                    
                    freq_list[i] = freq_list[j];
                    sorted_msg[i] =  sorted_msg[j];
                    generated_code[i] = generated_code[j];
                    
                    freq_list[j] = temp_freq;
                    sorted_msg[j] = temp_ch;
                    generated_code[j] = temp_code;
                }
            }
        }
    }
}

// function thread to coonect to the client
void connect_client() {
    
    int opt = 1;

    bzero((char*)&server_address, sizeof(server_address));
    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    int addrlen = sizeof(server_address);
    
    // Creating socket file descriptor
    server_SD = socket(AF_INET, SOCK_STREAM, 0);
    if (server_SD <= 0) {
        perror("Socket Failed");
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port
    int set_socket_status = setsockopt(server_SD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    if (set_socket_status < 0) {
        perror("Set Socket Failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the server_address
    int bind_status = bind(server_SD, (struct sockaddr *)&server_address, sizeof(server_address));
    
    if (bind_status < 0) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    cout << "Waiting for a client to connect..." << endl;

    int listen_status = listen(server_SD, 5);

    if (listen_status < 0) {
        perror("Listen Failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char const *argv[]) {

    if (argc != 2) {
        cout << "Invalid parameters" << endl;
        exit(EXIT_FAILURE); // terminate with error
    }

    port = stoi(argv[1]);
    int val_read = 0;
    int new_socket = 0;

    while (1){
        
        if (symbol_count == 0) connect_client();
        
        sockaddr_in new_sock_addr;
        socklen_t new_sock_addr_size = sizeof(new_sock_addr);
        
        //accept, create a new socket descriptor to handle the new connection with client
        new_socket = accept(server_SD, (struct sockaddr *)&server_address, &new_sock_addr_size);
        
        if (new_socket < 0) {
            perror("Accept Failed");
            exit(EXIT_FAILURE);
        }

        // display this message only once
        if (symbol_count == 0) cout << "Client Connected!" << endl << endl;

        int pipe_fd[2];

        if (pipe(pipe_fd) == -1) {
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
            
            close(pipe_fd[1]); /* Close unused write end */
            
            if (symbol_count == 1)
                read(pipe_fd[0], sorted_msg, sizeof(sorted_msg));

            memset(&buffer, 0, sizeof(buffer));
            val_read = recv(new_socket, (char*)&buffer, sizeof(buffer), 0);
            
            char message[max_size]={0};
            char ch;
            int i = 0, j = 0;

            if (symbol_count == 0) {
                while(buffer[i] != '\0') {
                message[i] = buffer[i];
                rem_msg[i] = buffer[i];
                i++;
               }
               msg_length = strlen(message);

                // identify unique characters and their frequency
                find_uniq_and_freq(message);
                sorted_msg = uniq_msg;
                uniq_count = strlen(uniq_msg); // total no. of uniq characters
                
            } else {
                ch = buffer[0];

                // cout<<"Received char: '"<< ch << "'" << endl;
                
                char * code = generate_code(ch, rem_msg);
                generated_code[symbol_count - 1] = code;

                rem_message(ch);

                send(new_socket, code, strlen(code), 0);

                // separator are char1
                char const *pchar1;
                char const char1 = char(1);
                pchar1 = &char1;
                send(new_socket, pchar1, 1, 0);
                send(new_socket, rem_msg, strlen(rem_msg), 0);
            }
            
        } else { //Parent process
            close(pipe_fd[0]); /* Close unused write end */

            // sort only once (after receiving message)
            if (symbol_count == 1){
                
                // Sort Unique characters
                sort();
                
                // send the sorted array to the child
                write(pipe_fd[1], sorted_msg, sizeof(sorted_msg));    
            }
            
            close(pipe_fd[0]); /* Reader will see EOF */
            wait(NULL); /* Wait for child */
            exit(EXIT_SUCCESS);

        }
        symbol_count++;
        if (symbol_count > uniq_count) break;
    }
    cout << "Process Finished!" << endl;
    close(new_socket);
    close(server_SD);
    return 0;
}
