#include "net.cpp"
#include <iostream>
#include "BigInt.hpp"
#include <random>

int genRandom(int from, int upto) {
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dist(from, upto);
	return dist(gen);
}

int main() {
	int conn_sock = connect_net("127.0.0.1", "1597");
	
	// gen shared key
	BigInt generator = 2;
	BigInt mod = std::string("42571564251765425471355154626165452431571334621325420624815730163446612426143167826542156263");
	std::cout << "generating and exchanging public key...\n";
	int exponent = genRandom(11, 999999);
	int expCopy = exponent;
	BigInt res = 1;
	while (exponent > 0) {
		if (exponent % 2 == 1) {
			res = (res * generator) % mod;
		}

		generator = (generator * generator) % mod;
		exponent /= 2;
	}
	char resChar[res.to_string().length()+1] = "";
	char friChar[500] = "";
	strcpy(resChar, res.to_string().c_str());
	recv_net(conn_sock, friChar, sizeof(friChar));
	send_net(conn_sock, resChar, sizeof(resChar));
	BigInt friendsKey(0);
	std::cout << "generating session key...\n";
	BigInt result = 1;
	friendsKey = friChar;
	while (expCopy > 0) {
		if (expCopy % 2 == 1) {
			result = (result * friendsKey) % mod;
		}

		friendsKey = (friendsKey * friendsKey) % mod;
		expCopy /= 2;
	}
	std::cout << "Shared secret is: " << result << "\n\n";
	// result is shared secret
	
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