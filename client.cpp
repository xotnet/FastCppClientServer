#include "net.cpp"
#include <iostream>
int main() {
	int conn_sock = connect_net("144.24.145.117", "1597");
	while (true) {
		char recvArrayBuf[1024] = "";
		std::string buff = "";
		
		// recv
		recv_net(conn_sock, recvArrayBuf, sizeof(recvArrayBuf));
		std::cout << recvArrayBuf << '\n';
		
		//send
		std::cout << "Message to send: ";
		std::getline(std::cin, buff);
		char charArrayBuff[buff.length()] = "";
		strcpy(charArrayBuff, buff.c_str());
		send_net(conn_sock, charArrayBuff, buff.length());
	}
}