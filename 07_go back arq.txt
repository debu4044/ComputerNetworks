#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>
using namespace std;

int main() {
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);
    
    int total_frames = 10, window_size = 4;
    
    srand(time(0));
    
    if (fork() == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        
        int frame;
        while (read(pipe1[0], &frame, sizeof(frame)) > 0) {
            if (rand() % 5 == 0) {
                cout << "Receiver: Frame " << frame << " lost\n";
                int nack = -1;
                write(pipe2[1], &nack, sizeof(nack));
            } else {
                cout << "Receiver: ACK " << frame << endl;
                write(pipe2[1], &frame, sizeof(frame));
            }
        }
        
        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);
        
        int base = 0; 
        while (base < total_frames) {
            cout << "\nSender Window: ";
            int window_end = min(base + window_size, total_frames);
            for (int i = base; i < window_end; i++) {
                cout << i << " ";
                write(pipe1[1], &i, sizeof(i));
            }
            cout << endl;
            
            int i = base;
            while (i < window_end) {
                int ack;
                if (read(pipe2[0], &ack, sizeof(ack)) <= 0) {
                    cerr << "Error: pipe closed unexpectedly\n";
                    break;
                }
                
                if (ack == -1) {
                    cout << "Sender: Error -> Retransmit from " << i << endl;
                    break;
                } else {
                    cout << "Sender: ACK received for " << ack << endl;
                    if (ack >= base) {
                        base = ack + 1;
                    }
                }
                i++;
            }
        }
        
        close(pipe1[1]);
        close(pipe2[0]);
        wait(NULL);
        cout << "\nAll frames sent (GBN).\n";
    }
    
    return 0;
}
