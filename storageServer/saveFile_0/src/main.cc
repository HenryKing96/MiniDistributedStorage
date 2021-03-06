#include "TcpServer.h"
#include "Loop.h"
#include "InetAddress.h"
#include "MD5.h"
#include "Fileproxy.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <map>
#include <mutex>
#include <vector>
#include <fstream>
#include <iostream>
#include <openssl/md5.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <arpa/inet.h>
#include <thread>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include "upLoad.h"
#include "downLoad.h"

enum StatuOperate { sw, up, down };
StatuOperate Operate = sw;

void switchOperate(const Reactor::TcpConnectionPtr& conn,
               Reactor::Buffer* buf)
{
  std::string str = buf->readAsBlock(8);
  std::cout << str <<'\n';
  if(str == "upppload")
  {
    Operate = up;
    //conn->send("up ack");
  }
  else if(str == "download")
  {
    Operate = down;
    conn->send("down ack");
  } 
}

void onConnection(const Reactor::TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("new connection [%s] from %s\n", conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
    statu = statu1;
    Operate = sw;
  }
  else
  {
    printf("connection [%s] is down\n", conn->name().c_str());
  }
}

void onMessage(const Reactor::TcpConnectionPtr& conn,
               Reactor::Buffer* buf)
{
  //upLoad(conn, buf);
  
  if(Operate == sw)
  {
    switchOperate(conn, buf);
  }  
  else if(Operate == up)
  {
    upLoad(conn, buf);
  }
  else
  {
    downLoad(conn, buf);
  }
}

int main(int argc, char* argv[])
{
  printf("main(): pid = %d\n", getpid());

  Reactor::InetAddress listenAddr(9982);
  Reactor::Loop loop;
  Reactor::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  } 
  server.start();

  loop.loop();
}
