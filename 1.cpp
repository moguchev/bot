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

void encrypt(const std::string message, std::string& result, int& key)
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

// Функция вызывается каждый раз, когда приходит сообщение, которое начинается со /photo
// В ответ пользователю придет сообщение с картинкой
void onCommandPhoto(Bot& bot, Message::Ptr message)
{
    bot.getApi().sendPhoto(message->chat->id, InputFile::fromFile("0.jpg", "image/jpeg"));
}

// Функция для демонстрации работы с командами
// Функция вызывается каждый раз, когда приходит сообщение, которое начинается со /start
void onCommandStart(Bot& bot, Message::Ptr message)
{
    /*
    std::ifstream Introduction("C:\\example-bot\Introduction.cfg");
    std::string helloy; 
    Introduction >> helloy;
    Introduction.close();
    */
    bot.getApi().sendMessage(message->chat->id, "Hi! I'm DecryptorLeoBot!\nI can encrypt and decrypt your messages. If you want to encrypt message use command /encrypt and if you want to decrypt it use command /decrypt. To enter the key use command /key , exe.: /key 12345");
}

// Функция для демонстрации работы с командами
// Функция вызывается каждый раз, когда приходит сообщение, которое начинается со /end
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

}
// регистрируем команды
std::map<std::string, std::function<void(Bot&, Message::Ptr)>> getAllCommands()
{
    // Ключом является идентификатор команды
    // значением является функция-обработчик коаманды
    // Например, при получении команды /end вызовется функция onCommandEnd
    std::map<std::string, std::function<void(Bot&, Message::Ptr)>> commands =
    {
        {"start", onCommandStart},
        {"end", onCommandEnd},
        {"keyboard", onCommandKeyboard},
        {"photo", onCommandPhoto},
        {"encrypt", onCommandEncrypt},
        {"decrypt", onCommandDecrypt},
        {"key", onCommandKey}
    };
    return commands;
}

// Функция, которая вызывается при любом сообщении
void onAnyMessage(Bot& bot, Message::Ptr message)
{
    // логгируем действия в консоль
    printf("User wrote %s\n", message->text.c_str());

    // игнорируем сообщения, которые начинаются с /start и /end
    if (StringTools::startsWith(message->text, "/start"))
    {
        return;
    }
    if (StringTools::startsWith(message->text, "/end"))
    {
        return;
    }
    if (StringTools::startsWith(message->text, "/photo"))
    {
        return;
    }
    if (StringTools::startsWith(message->text, "/key"))
    {
        if (waitKey && waitNextMessage)
        {

            KEY = atoi(message->text.c_str());
            std::cout << KEY << std::endl;
            bot.getApi().sendMessage(message->chat->id, "Enter message:");
            waitNextMessage = true;
            waitKey = false;
            return;
        } 
        else 
        {
            KEY = atoi(message->text.c_str());
            std::cout << KEY << std::endl;
            return;
        }
        return;
    }
    if (StringTools::startsWith(message->text, "/encrypt"))
    {
        return;
    }
    if (StringTools::startsWith(message->text, "/decrypt"))
    {
        return;
    }
    if (waitNextMessage)
    {
        if (modeEncryption && !waitKey)
        {
            std::string enc;
            encrypt(message->text, enc, KEY);
            std::cout << KEY << std::endl;
            bot.getApi().sendMessage(message->chat->id, enc);
            modeEncryption = false;
            waitNextMessage = false;
            return;
        }
        else if (modeDecryption && !waitKey)
        {
            std::string dec;
            decrypt(message->text, dec, KEY);
            std::cout << KEY << std::endl;
            bot.getApi().sendMessage(message->chat->id, dec);
            modeDecryption = false;
            waitNextMessage = false;
            return;
        }
       
    }
    // если в тексте сообщения есть "Привет" приветствуем собеседника
    std::string hello = Utils::fromLocale("Hi");
    std::string hello1 = Utils::fromLocale("hi");
    std::string hello2 = Utils::fromLocale("Helloy");
    if (message->text.find(hello) != std::string::npos ||
        message->text.find(hello1) != std::string::npos ||
        message->text.find(hello2) != std::string::npos)
    {
        bot.getApi().sendMessage(message->chat->id, Utils::fromLocale("Hi,") + message->from->firstName);
        return;
    }
   // отправляем сообщение, которое получили
   //bot.getApi().sendMessage(message->chat->id, "I can only encrypt and decrypt your messages. If you want to encrypt message use command /encrypt and if you want to decrypt it use command /decrypt. To enter the key use command /key , exe.: /key 12345");
}

