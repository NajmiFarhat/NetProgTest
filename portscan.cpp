#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

using namespace std;

static bool port_is_open(string ip, int port){
    struct sockaddr_in addr_s;
    const char* addr = ip.c_str();
    short int fd=-1;
    fd_set fdset;
    struct timeval tv;
    int rc;
    int so_error;
    socklen_t len;
    struct timespec tstart={0,0}, tend={0,0};
    int seconds = 1;

    addr_s.sin_family = AF_INET;
    addr_s.sin_addr.s_addr = inet_addr(addr);
    addr_s.sin_port = htons(port);

    clock_gettime(CLOCK_MONOTONIC, &tstart);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    rc = connect(fd, (struct sockaddr *)&addr_s, sizeof(addr_s));
    if ((rc == -1) && (errno != EINPROGRESS)) {
        close(fd);
        return false;
    }
    if (rc == 0) {
        clock_gettime(CLOCK_MONOTONIC, &tend);
        close(fd);
        return true;
    }

    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    rc = select(fd + 1, NULL, &fdset, NULL, &tv);
    if (rc == 1) {
        len = sizeof(so_error);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error == 0) {
            clock_gettime(CLOCK_MONOTONIC, &tend);
            printf("socket %s:%d connected. It took %.5f seconds\n",
                addr, port, (((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec)));
            close(fd);
            return true;
        }
    }
    close(fd);
    return false;
}

// Task queue and worker control
queue<pair<string, int>> tasks;
mutex queue_mutex;
condition_variable cv;
atomic<bool> done{false};

void worker_thread() {
    while (true) {
        pair<string, int> task;
        {
            unique_lock<mutex> lock(queue_mutex);
            cv.wait(lock, [] { return !tasks.empty() || done.load(); });

            if (done.load() && tasks.empty()) break;

            task = tasks.front();
            tasks.pop();
        }

        const string& ip = task.first;
        int port = task.second;

        if (port_is_open(ip, port)) {
            cout << ip << ":" << port << " is open\n";
        }
    }
}

void scan_with_thread_pool(const string& ip_prefix, int port_start, int port_end, int num_threads) {
    // Launch worker threads
    vector<thread> workers;
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back(worker_thread);
    }

    // Populate the task queue
    {
        lock_guard<mutex> lock(queue_mutex);
        for (int i = 1; i < 255; ++i) {
            string ip = ip_prefix + to_string(i);
            for (int port = port_start; port <= port_end; ++port) {
                tasks.emplace(ip, port);
            }
        }
    }

    cv.notify_all();

    // Mark done and wait for threads
    {
        unique_lock<mutex> lock(queue_mutex);
        done = true;
    }
    cv.notify_all();

    for (auto& t : workers) {
        t.join();
    }
}

int main(int argc, char** argv) {
    const string ip_prefix = "10.10.0.";
    const int port_start = 5950;
    const int port_end = 6020;
    const int max_threads = 100; // limit concurrency

    scan_with_thread_pool(ip_prefix, port_start, port_end, max_threads);

    return 0;
}
