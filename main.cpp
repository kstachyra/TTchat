#include<iostream>
#include<thread>


#include "easylogging++.h"

#define ELPP_THREAD_SAFE
INITIALIZE_EASYLOGGINGPP

using namespace std;

void hello();

int main(int, char**)
{
	el::Configurations conf("/home/ks/ClionProjects/TTchat/easyLogging.conf");
	el::Loggers::reconfigureAllLoggers(conf);

	LOG(INFO) << "My first info log using default logger";
	thread t1(hello);

	t1.join();
	return 0;
}

void hello()
{
	LOG(INFO) << "Hello World Log :)";
}