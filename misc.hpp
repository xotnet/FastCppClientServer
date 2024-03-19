#include <bitset>
#include "net.hpp"
#include <unistd.h>
#include <random>
#include <cstring>
void timer() {
	while (true) {sleep(1);}
}

int genRandom(int from, int upto) {
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<> dist(from, upto);
	return dist(gen);
}

std::string readPiece(std::string inputString, int start, int end = -1) {
	if (end == -1) {end = inputString.length();}
	std::string result="";
	for (int i = start; i<end; i++) {
		result = result + inputString[i];
	}
	return result;
}

std::string crySynMethod(std::string inputString, std::string key) {
	std::string toChars16bit = "";
	std::string bitsAfterCrypting = "";
	int i = 0;
	int cycleCounter = (key[0]-'0')+(key[1]-'0');
	for (char c : inputString) { // every char in message
		std::bitset<16> bits(c);
		for (int g = 0; g<key.length(); g = g + cycleCounter) {
			std::bitset<16> keyBitSet(key[g]);
			std::bitset<16> indexBits(key[i]);
			bits = bits ^ keyBitSet;
			bits = bits ^ indexBits;
			if (i < key.length()-1) {i++;}
			else {i = 0;}
		}
		bitsAfterCrypting = bitsAfterCrypting + bits.to_string();
	}
	for (int b = 0; b < bitsAfterCrypting.length(); b = b+16) {
		std::string bitsPreset = "";
		for (int j = 0; j < 16; j++) {
			bitsPreset = bitsPreset + bitsAfterCrypting[b+j];
		}
		std::bitset<16> bits(bitsPreset);
		toChars16bit = toChars16bit + static_cast<char>(bits.to_ulong());
	}
	return toChars16bit;
}
#include <iostream>
void sendAndEncryptMessage(int accepted, char* message, std::string key) {
	std::string encryptedMessage = crySynMethod(std::string(message), key);
	strcpy(message, encryptedMessage.c_str());
	unsigned int sizeMessage = encryptedMessage.length();
	send_net(accepted, message, sizeMessage);
}