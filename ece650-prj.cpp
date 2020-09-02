#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <pthread.h>
#include <mutex>
#include "graph.h"
#include <cmath>
#include <numeric>
#include <cstdio>
#include <unistd.h>
#include <cerrno>

using namespace Minisat;
using namespace std;

long Stand_dev_long(vector<long> datave, long mean){//function that calculates standard deviation of a vector of data of type 'long'
    long sd = 0;
    for (long i : datave){
        sd = sd + pow(i-mean,2);
    }
    sd = sqrt(sd/datave.size());
    return sd;
}

float Stand_dev_float(vector<float>datave, float mean){//function that calculates standard deviation of a vector of data of type 'float'
    float sd = 0;
    for (float i : datave){
        sd = sd + pow(i-mean,2);
    }
    sd = sqrt(sd/datave.size());
    return sd;
}

Graph gt;
pthread_t pthread_IO, pthread_1, pthread_2, pthread_CNF;
clockid_t start_1, start_2, start_CNF;
timespec ts_1, ts_2, ts_CNF;
vector<long> vt_1, vt_2, vt_CNF;
vector<float> vn_1, vn_2, vn_CNF;
int thread_flag, stat;
bool timeout = true;

#define handle_error(msg) \
		do { perror(msg); exit(EXIT_FAILURE);}  while(0)
#define handle_error_en(en,msg) \
		do { errno = en; perror(msg); exit(EXIT_FAILURE);} while(0)

timespec running_time(clockid_t c) {
    struct timespec ts{};
    if(clock_gettime(c,&ts) == -1)
        handle_error("clock_gettime");
    return ts;
}

void *thread_IO(void *arg) {
    bool getV = false, getE = false;
    if(gt.MaxVertices != 0) {
        while(true) {
            if(thread_flag == 3) {
                thread_flag = 0;
                break;
            }
        }
        gt.print_VC();
        gt.reset_graph();
    }
    while ((!getV) || (!getE)) {
        /*
         *   For final time and ratio data output only!
         *
         */
        /*if (cin.eof()) {//press "CTRL"+"D" at the same time
            long vtCNF_mean = accumulate(vt_CNF.begin(),vt_CNF.end(),0.0)/vt_CNF.size();
            long vtCNF_sd = Stand_dev_long(vt_CNF,vtCNF_mean);
            cout<<"CNF mean run-time is :"<<vtCNF_mean<<endl;
            cout<<"CNF run-time stand-dev is:" <<vtCNF_sd<<endl;

            long vt1_mean = accumulate(vt_1.begin(),vt_1.end(),0.0)/vt_1.size();
            long vt1_sd = Stand_dev_long(vt_1,vt1_mean);
            cout<<"VC1 mean run-time is :"<<vt1_mean<<endl;
            cout<<"VC1 run-time stand-dev is:" <<vt1_sd<<endl;

            long vt2_mean = accumulate(vt_2.begin(),vt_2.end(),0.0)/vt_2.size();
            long vt2_sd = Stand_dev_long(vt_2,vt2_mean);
            cout<<"VC2 mean run-time is :"<<vt2_mean<<endl;
            cout<<"VC2 run-time stand-dev is:" <<vt2_sd<<endl;

            vector<float> vr_1, vr_2;

            for(unsigned int i = 0; i < vn_CNF.size();i++){
                vr_1.push_back(vn_1[i]/vn_CNF[i]);
                vr_2.push_back(vn_2[i]/vn_CNF[i]);
            }

            float vr1_mean = accumulate(vr_1.begin(),vr_1.end(),0.0)/vr_1.size();
            float vr1_sd = Stand_dev_float(vr_1,vr1_mean);
            cout<<"VC1 mean appro-ratio is:"<< vr1_mean<<endl;
            cout<<"VC1 appro-ratio stand-dev is:" <<vr1_sd<<endl;

            float vr2_mean = accumulate(vr_2.begin(),vr_2.end(),0.0)/vr_2.size();
            float vr2_sd = Stand_dev_float(vr_2,vr2_mean);
            cout<<"VC2 mean appro-ratio is:"<< vr2_mean<<endl;
            cout<<"VC2 appro-ratio stand-dev is:" <<vr2_sd<<endl;

            exit(0);
        }*/
        while (!cin.eof()) {
            string line;
            getline(cin, line);
            istringstream input(line);
            char comm;
            if (!input.str().empty()) {
                input >> comm;
            } else {
                comm = 0;
            }
            if (comm == 'V') {
                int vSize;
                input >> vSize;
                if (vSize < 2) {
                    gt.reset_graph();
                    cerr << "Error: must have at least 2 vertices\n";
                }
                else {
                    gt.MaxVertices = vSize;
                    //v_flag[vSize - 1]++;
                    getV = true;
                }
            } else if (comm == 'E') {
                char sep;
                int a, b;
                input >> sep;
                while (true) {
                    input >> sep;
                    if (sep == '}') {
                        getE = true;
                        break;
                    }
                    input >> a;
                    if ((a < 0) || (a >= gt.MaxVertices)) {
                        cerr << "Error: vertex doesn't exist\n";
                        gt.reset_graph();
                        break;
                    }
                    input >> sep;
                    input >> b;
                    if ((b < 0) || (b >= gt.MaxVertices)) {
                        cerr << "Error: vertex doesn't exist\n";
                        gt.reset_graph();
                        break;
                    }
                    input >> sep;
                    gt.insert_edge(a,b);
                    input >> sep;
                    if (sep == '}') {
                        getE = true;
                        break;
                    }
                }
            }
            if (getV && getE) {
                break;
            }
        }
    }
    return NULL;
}

void *thread_1(void *arg) {
    long time;
    if (gt.MaxVertices == 0) {
        return NULL;
    }
    gt.APPROX_VC_1();
    vn_1.push_back(gt.VC_1.size());
    stat = pthread_getcpuclockid(pthread_self(), &start_1);
    if (stat !=0)
        handle_error_en(stat,"pthread_getcpuclockid");
    ts_1 = running_time(start_1);
    time = ts_1.tv_sec * 1000000 + ts_1.tv_nsec / 1000;
    vt_1.push_back(time);
    thread_flag++;
    return NULL;
}

void *thread_2(void *arg) {
    long time;
    if (gt.MaxVertices == 0) {
        return NULL;
    }
    gt.APPROX_VC_2();
    vn_2.push_back(gt.VC_2.size());
    stat = pthread_getcpuclockid(pthread_self(), &start_2);
    if(stat!=0)
        handle_error_en(stat,"pthread_getcpuclockid");
    ts_2 = running_time(start_2);
    time = ts_2.tv_sec * 1000000 + ts_2.tv_nsec / 1000;
    vt_2.push_back(time);
    thread_flag++;
    return NULL;
}

void *thread_CNF(void *arg) {
    long time;
    if (gt.MaxVertices == 0) {
        timeout = false;
        return NULL;
    }
    gt.CNF_SAT_VC();
    vn_CNF.push_back(gt.VC_CNF.size());
    stat = pthread_getcpuclockid(pthread_self(), &start_CNF);
    if (stat != 0)
        handle_error_en(stat, "pthread_getcpuclockid");
    ts_CNF = running_time(start_CNF);
    time = ts_CNF.tv_sec * 1000000 + ts_CNF.tv_nsec / 1000;
    vt_CNF.push_back(time);
    timeout = false;
    thread_flag++;
    return NULL;
}

int main(int argc, char ** argv) {
    int status;
    while (true) {
        status = pthread_create(&pthread_IO, NULL, thread_IO, NULL);
        if (status!=0)
            cerr<<"Error: fail to create thread IO!"<<endl;

        sleep(1);

        status = pthread_create(&pthread_1, NULL, thread_1, NULL);
        if (status !=0)
            cerr<<"Error: fail to create thread VC1!"<<endl;

        sleep(1);

        status = pthread_create(&pthread_2, NULL, thread_2, NULL);
        if (status != 0)
            cerr <<"Error: fail to create thread VC2!"<<endl;

        sleep(1);

        status = pthread_create(&pthread_CNF, NULL, thread_CNF, NULL);
        if (status != 0)
            cerr<<"Error: fail to create thread CNF!"<<endl;

        sleep(20);
        if (timeout) {
            pthread_cancel(pthread_CNF);
            vn_CNF.push_back(-1);
            vt_CNF.push_back(-1);
            thread_flag++;
        } else {
            pthread_join(pthread_CNF, NULL);
            timeout = true;
        }
        pthread_join(pthread_IO, NULL);
        pthread_join(pthread_1, NULL);
        pthread_join(pthread_2, NULL);
    }
    return 0;
}