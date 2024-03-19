#include <bitset>
#include <unistd.h>
#include <random>

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

std::string toBitAndEncrypt(std::string inputString, std::string key) {
	std::string toChars16bit = "";
	std::string bitsAfterCrypting = "";
	int i = 0;
	for (char c : inputString) { // every char in message
		std::bitset<16> bits(c); // bitset is container with size 16
		for (int u = 0; u < key.length(); u++) { // bitset in key char
			std::bitset<16> keyCharToBit(key[u]);
			std::bitset<16> indexBitsPool(i);
			int pBitsCounter = 0;
			for (int v=0;v<key.length()*8;v++) { // data to indexBitsPool based on key
				for (char o : key) {
					std::bitset<16> bitsOfKeySym(o);
					for (int ii = 0; ii < 16; ii++) {
						if (bitsOfKeySym[ii] == 1) {pBitsCounter += 1;}
					}
				}
			}
			bits = bits ^ keyCharToBit; // xor
			std::bitset<16> pBitsCounterToBits(pBitsCounter);
			indexBitsPool = indexBitsPool ^ pBitsCounterToBits;
			for (int l = 0; l < 16; l++) {
				if (keyCharToBit[l] == 1) {
					bits = bits ^ keyCharToBit; // xor
					bits = bits ^ indexBitsPool; // xor
				}
			}
		}
		bitsAfterCrypting = bitsAfterCrypting + bits.to_string();
		i++;
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

std::string toStringAndDecrypt(std::string inputString, std::string key) {
	std::string toCharFromBin = "";
	std::string bitsAfterCrypting = "";
	for (int i = 0; i < inputString.length(); i++) { // every char in encrypted message
		std::bitset<16> bits(inputString[i]); // convert char to bitset
		for (int u = 0; u < key.length(); u++) { // bitset in key char
			std::bitset<16> keyCharToBit(key[u]);
			std::bitset<16> indexBitsPool(i);
			int pBitsCounter = 0;
			for (int v=0;v<key.length()*8;v++) { // data to indexBitsPool based on key
				for (char o : key) {
					std::bitset<16> bitsOfKeySym(o);
					for (int ii = 0; ii < 16; ii++) {
						if (bitsOfKeySym[ii] == 1) {pBitsCounter += 1;}
					}
				}
			}
			bits = bits ^ keyCharToBit; // xor
			std::bitset<16> pBitsCounterToBits(pBitsCounter);
			indexBitsPool = indexBitsPool ^ pBitsCounterToBits;
			for (int l = 0; l < 16; l++) {
				if (keyCharToBit[l] == 1) {
					bits = bits ^ keyCharToBit;
					bits = bits ^ indexBitsPool;
				}
			}
		}
		bitsAfterCrypting = bitsAfterCrypting + bits.to_string();
	}
	for (int b = 0; b < bitsAfterCrypting.length(); b = b+16) {
		std::string bitsPreset = "";
		for (int j = 0; j < 16; j++) {
			bitsPreset = bitsPreset + bitsAfterCrypting[b+j];
		}
		std::bitset<16> bits(bitsPreset);
		toCharFromBin = toCharFromBin + static_cast<char>(bits.to_ulong());
	}
	return toCharFromBin;
}

void encryptMessage(char* message, std::string key) {
	std::string encryptedMessage = toBitAndEncrypt(message, readPiece(key, 12, 24));
	strcpy(message, encryptedMessage.c_str());
}