#include "stdio-text-protocol.h"
#include "log.h"
#include <stdio.h>
#include "pico/stdlib.h"

void stdio_text_protocol_init(void)
{
    stdio_init_all();
}


const command_t *stdio_text_protocol_handle(void)
{
    static unsigned int line_length = 0;
    static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
    static bool truncated;

    static command_t cmd;
    
    // читаем символ без ожидания; нет символа — возвращаем NULL
    int symbol = getchar_timeout_us(0);

    if (symbol == PICO_ERROR_TIMEOUT)
    {
        return NULL;
    }

    // конец строки: пустую строку пропускаем, иначе переводим строку в терминале,
    // делим строку на слова и возвращаем команду
    if (symbol == '\r' || symbol == '\n')
    {
        if(line_length == 0)
        {
            return NULL;
        }
        
        line[line_length] = '\0';
        line_length = 0;
        putchar('\n');

        LOG_DBG("got %s\n", line);

        cmd.name = NULL;
        cmd.argc = 0;
        cmd.truncated = truncated;
        for (unsigned int i = 0; i < STDIO_TEXT_PROTOCOL_MAX_ARGS; i++) 
        {
            cmd.argv[i] = NULL;
        }
        
        truncated = false;

        bool in_word = false;

        for(unsigned int i = 0; line[i] != '\0'; i++)
        {
            if (line[i] == ' ' || line[i] == '\t') 
            {
                if (in_word) 
                {
                    line[i] = '\0'; // "Отрезаем" слово, заменяя пробел на ноль
                    in_word = false;
                }
            } 
            // Если нашли начало нового слова
            else if (!in_word) 
            {
                in_word = true;
                
                if (cmd.name == NULL) 
                {
                    cmd.name = &line[i]; // Первое слово — всегда имя команды
                } 
                else 
                {
                    if (cmd.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS) {
                        cmd.argv[cmd.argc] = &line[i]; // Запоминаем адрес аргумента
                        cmd.argc++;
                    } 
                    else 
                    {
                        cmd.truncated = true; // Аргументов больше, чем массив
                    }
                }
            }
        }

        return &cmd;
    }

    // Backspace (0x08 или 0x7f): убираем последний символ и стираем его на экране "\b \b"
    if (symbol == 0x08 || symbol == 0x7F) {
        if (line_length > 0) {
            line_length--;
            truncated = false;
            putchar('\b');
            putchar(' '); 
            putchar('\b');
        }
        return NULL;
    }

    // печатный символ: кладём в буфер и повторяем в терминал;
    // буфер полон — помечаем строку как переполненную
    if(symbol >= 0x20 && symbol <= 0x7e)
    {
        if (line_length + 1 < STDIO_TEXT_PROTOCOL_LINE_SIZE)
        {
            line[line_length] = (char)symbol;
            line_length = line_length + 1;
            putchar(symbol);
        }
        else
        {
        truncated = true;
        }
    }

    return NULL;
}