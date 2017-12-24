#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "api.hpp"
#include "keyboard.hpp"
#include "utils.hpp"

// Файл, для демонстрации работы с командами и другими сообщениями

using namespace std;
using namespace TgBot;
using namespace Utils;
using byte = unsigned char;

bool modeEncryption = false;
bool modeDecryption = false;
bool waitKey = false;
bool waitNextMessage = false;
int KEY = 0;

byte update_gamma(unsigned short& Gamma)
{
    byte gammai;
    gammai = (Gamma >> 8) ^ (Gamma & 0xff);
    return gammai;
}

std::string charToHexString(byte ch)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << (int)ch;
    if ((int)ch < 16)
    {
        std::string add = "0";
        add.push_back(oss.str()[0]);
        return add;
    }
    else
        return oss.str();
}

char * convertStringToChar(const std::string& s)
{
    char * writable = new char[s.size() + 1];
    std::copy(s.begin(), s.end(), writable);
    writable[s.size()] = '\0';
    return writable;
}

std::string convertFromHex(const std::string& message)
{
    int dec;
    byte iblock;
    std::string buff;
    std::string blockOfbytes;
    for (int i = 0; i < message.size(); i = i + 2)
    {
        buff.push_back(message[i]);
        buff.push_back(message[i + 1]);
        std::istringstream(buff) >> std::hex >> dec;
        memcpy(&iblock, &dec, 1);
        blockOfbytes.push_back(iblock);
        buff.clear();
    }
    return blockOfbytes;
}

void encrypt(const std::string& message, std::string& result, int& key)
{
    srand(key);
    unsigned short Gamma;
    byte buff1;
    byte buff2;
    byte gammai;
    char *mes = convertStringToChar(message);
    for (int i = 1; i <= message.size(); ++i)
    {
        Gamma = rand();
        gammai = update_gamma(Gamma);
        memcpy(&buff1, mes + (i - 1), 1);
        buff2 = buff1^gammai;
        buff1 = buff2;
        buff1 >>= 4;
        buff2 <<= 4;
        buff2 |= buff1;
        result += charToHexString(buff2);
    }
    delete[] mes;
}

void decrypt(const std::string& message, std::string& result, int& key)
{
    srand(key);
    unsigned short Gamma;
    byte buff1;
    byte buff2;
    byte gammai;
    std::string mes = convertFromHex(message);
    char *mess = convertStringToChar(mes);
    for (int i = 1; i <= message.size() / 2; i++)
    {
        Gamma = rand();
        gammai = update_gamma(Gamma);
        memcpy(&buff1, mess + (i - 1), 1);
        buff2 = buff1;
        buff1 <<= 4;
        buff2 >>= 4;
        buff2 |= buff1;
        buff2 = buff2^gammai;
        result += buff2;
    }
    delete[] mess;
}

void encryption(Bot& bot, Message::Ptr message)
{
	std::string enc;
	encrypt(message->text, enc, KEY);
	std::cout << KEY << std::endl;
	bot.getApi().sendMessage(message->chat->id, enc);
	modeEncryption = false;
	waitNextMessage = true;
}
void decryption(Bot& bot, Message::Ptr message)
{
	std::string dec;
	decrypt(message->text, dec, KEY);
	std::cout << KEY << std::endl;
	bot.getApi().sendMessage(message->chat->id, dec);
	modeDecryption = false;
	waitNextMessage = true;
}
void saveKey(Bot& bot, Message::Ptr message)
{
	KEY = atoi(message->text.c_str());
	std::cout << KEY << std::endl;
	bot.getApi().sendMessage(message->chat->id, "Enter message:");
	waitNextMessage = true;
	waitKey = false;
}
void onCommandStart(Bot& bot, Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Hi! I'm DecryptorLeoBot!\nI can encrypt and decrypt your messages. If you want to encrypt message use command /encrypt and if you want to decrypt it use command /decrypt. To enter the key use command /key");
}

void onCommandEnd(Bot& bot, Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Bye!");
}

void onCommandEncrypt(Bot& bot, Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Write key for encryption:");
    waitNextMessage = true;
    modeEncryption = true;
    waitKey = true;
}
void onCommandDecrypt(Bot& bot, Message::Ptr message)
{
    bot.getApi().sendMessage(message->chat->id, "Enter right key for decryption:");
    waitNextMessage = true;
    modeDecryption = true;
    waitKey = true;
}

void onCommandKey(Bot& bot, Message::Ptr message)
{
	waitKey = true;
	waitNextMessage = true;
}

std::map<std::string, std::function<void(Bot&, Message::Ptr)>> getAllCommands()
{
    // Ключом является идентификатор команды
    // значением является функция-обработчик коаманды
    // Например, при получении команды /end вызовется функция onCommandEnd
    std::map<std::string, std::function<void(Bot&, Message::Ptr)>> commands =
    {
        {"start", onCommandStart},
        {"end", onCommandEnd},
        {"encrypt", onCommandEncrypt},
        {"decrypt", onCommandDecrypt},
        {"key", onCommandKey}
    };
    return commands;
}

void onAnyMessage(Bot& bot, Message::Ptr message)
{
    printf("User wrote %s\n", message->text.c_str());
    if (StringTools::startsWith(message->text, "/start"))
        return;
    if (StringTools::startsWith(message->text, "/end"))
        return;
    if (StringTools::startsWith(message->text, "/key"))
		return;
    if (StringTools::startsWith(message->text, "/encrypt"))
        return;
    if (StringTools::startsWith(message->text, "/decrypt"))
        return;
    if (waitNextMessage)
    {
        if (modeEncryption && !waitKey && waitNextMessage )
        {
			encryption(bot, message);
            return;
        }
        else if (modeDecryption && !waitKey && waitNextMessage)
        {
			decryption(bot, message);
            return;
        }    
		else if (waitKey && waitNextMessage && !StringTools::startsWith(message->text, "/key"))
		{
			saveKey(bot, message);
			return;
		}	
    }
}

