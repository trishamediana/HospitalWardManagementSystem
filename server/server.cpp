#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <iomanip>

#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

using namespace std;
using namespace sql;

#define SOCKET_FILENAME "/tmp/server.sock"

sql::Driver *driver;
sql::Connection *con;
sql::PreparedStatement *prep_stmt;
sql::Statement *stmt;
sql::ResultSet *res;

int server;

void signal_callback_handler(int signum)
{
  // close server
  close(server);
  // remove the socket file
  unlink(SOCKET_FILENAME);
  // signal handled
  exit(0);
}

void connect_mysql()
{
    try {
        std::cout << "connecting to mysql server...." << std::endl;
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "root", "password");
        std::cout << "connected!" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "use hospital ward" << std::endl;
        con->setSchema("hospitalward");
        std::cout << "hospital ward connected" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        std::cout << std::endl;


        return con;
    } catch (sql::SQLException &e) {
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }

    std::cout << std::endl;

    return false
}

int main(int argc, char **argv)
{
  struct sockaddr_un server_addr, client_addr;
  socklen_t clientlen = sizeof(client_addr);
  int client, buflen, nread;
  char *buf;

  puts("Hell World");

  // listen to SIGINT, SIGTERM, and SIGKILL
  signal(SIGINT, signal_callback_handler);
  signal(SIGTERM, signal_callback_handler);
  signal(SIGKILL, signal_callback_handler);

  // setup socket address structure
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sun_family = AF_UNIX;
  strcpy(server_addr.sun_path, SOCKET_FILENAME);

  // create socket
  server = socket(PF_UNIX, SOCK_STREAM, 0);
  if (!server) {
    perror("socket");
    exit(-1);
  }

  // call bind to associate the socket with our local address and
  // port
  if (bind(server, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind");
    exit(-1);
  }

  if (connect_mysql())
  {
      perror("mysql");
      exit(-1);
  }

  // convert the socket to listen for incoming connections
  if (listen(server, 0) < 0) {
    perror("listen");
    exit(-1);
  }

  puts("Listening to connection...");

  // allocate buffer
  buflen = 1024;
  buf = new char[buflen+1];

  // loop to handle all requests
  while (1) {
    unsigned int client = accept(server, (struct sockaddr *)&client_addr, &clientlen);

    // read a request
    memset(buf, 0, buflen);
    nread = recv(client, buf, buflen, 0);

    printf("\nClient says: %s\n\n", buf);

    // echo back to the client
    send(client, buf, nread, 0);

    close(client);
  }

  close(server);

  unlink(SOCKET_FILENAME);

return 0;
}
