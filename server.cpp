#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <sstream>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <random>
#include <chrono>
#include <fstream>
using namespace std;


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
  boost::asio::io_service io;
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  if (argc < 3) {
    fprintf(stderr,"usage: s <port_number> <milliseconds_between_frames>\n");
    exit(1);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    error("ERROR on binding");
  listen(sockfd,5);
  clilen = sizeof(cli_addr);
  std::cout << "waiting for client...\n";
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if (newsockfd < 0) 
    error("ERROR on accept");
  std::cout << "got one client 22 \n";
  // open the data file 
  std::ifstream reddatafile ("winequality-red.csv");
  std::ifstream whitedatafile ("winequality-white.csv");

      cout << "0" << endl;

  // uniform random number generator
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0,1);


  // main loop
  long delay = atol(argv[2]);
  bool moreWhite = true;  int redCount=0;
  bool moreRed = true;    int whiteCount=0;
  cout << "1" << endl;

  while(moreWhite==true || moreRed==true) {
    std::string line;
    moreWhite = moreRed = false;
    int whichWine = distribution(generator);
    cout << "2" << endl;
    if (whichWine==0 && getline(reddatafile,line) ) {
      moreRed = true;
      redCount++;
      line = line+";R\n";
      std::cout << "Tx(" << redCount << ") -> " << line;
      int n = write(newsockfd,line.c_str(),line.size());
      if (n < 0) 
        error("ERROR writing to socket");
      usleep(delay);
    }
    if (whichWine==1 && getline(whitedatafile,line) ) {
      moreWhite = true;
      whiteCount++;
      line = line+";W\n";
      std::cout << "Tx(" << whiteCount << ") -> " << line;
      int n = write(newsockfd,line.c_str(),line.size());
      if (n < 0) 
        error("ERROR writing to socket");
      usleep(delay*1000);
    }
  }
  reddatafile.close();
  whitedatafile.close();
  close(sockfd);
  return 0; 
}

