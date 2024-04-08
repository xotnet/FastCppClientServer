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
	int conn_sock = connect_net("144.24.145.117", "1597");
    if (conn_sock == -1) {std::cerr << "[Err] Server is down\n"; return 0;}
	
    // BotFilterClient
    char recvBFBuf[1024] = "";
    while (std::string(recvBFBuf) != "BotCheck passed") {
        memset(recvBFBuf, '\0', sizeof(recvBFBuf));
        recv_net(conn_sock, recvBFBuf, sizeof(recvBFBuf));
        if (std::string(recvBFBuf).substr(0, 5) == "print") {
            std::cout << std::string(recvBFBuf).substr(6);
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
		buff = crySynMethod(buff, result.to_string());
		char charArrayBuff[buff.length()] = "";
		strcpy(charArrayBuff, buff.c_str());
		send_net(conn_sock, charArrayBuff, buff.length());
	}
}
