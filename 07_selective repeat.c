#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

int main() {
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);

    int total_frames = 10;
    int window_size = 4;

    srand(time(0));

    if (fork() == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        bool received[10] = {false};
        int frame;

        while (read(pipe1[0], &frame, sizeof(frame)) > 0) {
            if (rand() % 5 == 0) {
                cout << "Receiver: Frame " << frame << " lost\n";
                int nack = -frame; 
                write(pipe2[1], &nack, sizeof(nack));
            } else {
                cout << "Receiver: Frame " << frame << " received (ACK sent)\n";
                received[frame] = true;
                write(pipe2[1], &frame, sizeof(frame));
            }
        }

        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    } 
    else {
        close(pipe1[0]);
        close(pipe2[1]);

        bool ack[10] = {false};
        int base = 0; 

        while (base < total_frames) {
            int end = min(base + window_size, total_frames);

            cout << "\nSender Window: ";
            for (int i = base; i < end; i++) {
                cout << i << " ";
            }
            cout << endl;

            int sent_count = 0;

            for (int i = base; i < end; i++) {
                if (!ack[i]) {
                    cout << "Sender: Sending frame " << i << endl;
                    write(pipe1[1], &i, sizeof(i));
                    sent_count++;
                }
            }

            for (int k = 0; k < sent_count; k++) {
                int res;
                read(pipe2[0], &res, sizeof(res));

                if (res >= 0) {
                    cout << "Sender: ACK received for " << res << endl;
                    ack[res] = true;
                } else {
                    cout << "Sender: NACK received for frame " << -res << endl;
                }
            }

            while (base < total_frames && ack[base]) {
                base++;
            }
        }

        close(pipe1[1]);
        close(pipe2[0]);
        wait(NULL);

        cout << "\nAll frames sent successfully (Selective Repeat).\n";
    }

    return 0;
}
