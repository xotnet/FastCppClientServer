#include <iostream>
#include "BigInt.hpp"
#include "misc.hpp"

BigInt genKey(int conn_sock) {
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
	std::string publicKeyToSend = "EncExchP1" + res.to_string();
	char resChar[publicKeyToSend.length()+1] = "";
	char friChar[500] = "";
	strcpy(resChar, publicKeyToSend.c_str());
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
    return result;
}

int main() {
	std::string address = "127.0.0.1";
	char port[10] = "1597";
	// if domain name
	if (address[0] != '0' || address[0] != '1' || address[0] != '2' || address[0] != '3' || address[0] != '4' || address[0] != '5' || address[0] != '6' || address[0] != '7' || address[0] != '8' || address[0] != '9') {
		address = resolve_net(&address[0], port);
	}
	int conn_sock = connect_net(&address[0], port);
    if (conn_sock == -1) {std::cerr << "[Err] Server is down\n"; return 0;}
	
    // BotFilterClient
    char recvBFBuf[1024] = "";
    while (true) {
        memset(recvBFBuf, '\0', sizeof(recvBFBuf));
        recv_net(conn_sock, recvBFBuf, sizeof(recvBFBuf));
        if (std::string(recvBFBuf).substr(0, 5) == "print") {
            std::cout << std::string(recvBFBuf).substr(6);
        }
		if (std::string(recvBFBuf).substr(0, 5) == "psdbf") {
			std::cout << "BotFilter passed\n\n\n";
			break;
		}
    }
    
	// gen shared key
	BigInt result = genKey(conn_sock);
	std::cout << "session secret is: " << result << "\n\n";
	// result is shared secret
	while (true) {
		char recvArrayBuf[1024] = "";
		std::string buff = "";
		
		// recv
		recv_net(conn_sock, recvArrayBuf, sizeof(recvArrayBuf));
		std::string decryptedMessage = crySynMethod(std::string(recvArrayBuf), result.to_string());
		std::cout << '\n' << decryptedMessage << '\n';
		
		//send
		std::cout << "Message to send: ";
		std::getline(std::cin, buff);
		if (buff == "logout" || buff == "exit") {exit(0);}
		buff = crySynMethod(buff, result.to_string());
		char charArrayBuff[buff.length()] = "";
		strcpy(charArrayBuff, buff.c_str());
		send_net(conn_sock, charArrayBuff, buff.length());
	}
}
