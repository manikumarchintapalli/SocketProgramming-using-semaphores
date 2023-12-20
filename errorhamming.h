#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#define flag 1
#define unflag 0
#define BUFFERSIZE 8192
char PolynomialGenerator[] = "100000100110000010001110110110111";

bool check_hamming_code_char(char *);
bool check_hamming(char *);
char selecting_character_hamming_code(char *);
int bit_value(char, int);
void choose_strings_from_hamming_code(char *, char *);
void calculating_parity(int *, int *, int *, char *);
void create_hamming_code(char *, char *);
void correct_hamming_code_character(char *);
void error_correction_hamming_code(char *);
void get_bit_error(char *, int);
void get_hamming_code_character(char, char *);
void reversing_the_char_bits(char *, int);
void setting_the_bit(char *, int);
void string_to_binary(char *, char *);
void modulo_to_division(char *, char *, char *);
void encode_crc(char *, char *, char *);
int decode_crc(char *, char *, char *);

// Check hamming code character
bool check_hamming_code_char(char *encrypt)
{
    int parity8 = 0, parity4 = 0, parity2 = 0, parity1 = 0;
    int i;
    for (i = 8; i <= 15; i++)
    {

        if (bit_value(encrypt[0], i - 8))
        {
            parity8++;
        }
    }

    while (i <= 15)
    {
        i++;
    }

    calculating_parity(&parity4, &parity2, &parity1, encrypt);

    if (flag)
    {
        if (parity1 % 2 || parity2 % 2 || parity4 % 2 || parity8 % 2)
            return false;
        return true;
    }
}

// modulo to division
void modulo_to_division(char *encodedstring, char *Polynomial, char *CRC)
{
    int messageLength = strlen(encodedstring);
    int polynomialLength = strlen(Polynomial);

    char temp[BUFFERSIZE];
    strcpy(temp, encodedstring);

    for (int i = 0; i <= messageLength - polynomialLength; i++)
    {
        if (temp[i] == '1')
        {
            for (int j = 0; j < polynomialLength; j++)
            {
                int k = i + j;
                temp[k] = (temp[k] == Polynomial[j]) ? '0' : '1';
            }
        }
    }
    strcpy(CRC, temp + messageLength - polynomialLength + 1);
}

// encode CRC
void encode_crc(char *div, char *bin, char *enc)
{
    int remainderLength = strlen(div) - 1;
    char encodeString[BUFFERSIZE];

    memset(encodeString, 0, BUFFERSIZE);
    strcpy(encodeString, bin);

    for (int i = 0; i < remainderLength; i++)
    {
        strcat(encodeString, "0");
    }
    modulo_to_division(encodeString, div, enc);
    printf("Encoding...\n");
    printf("Encoded String %s\n", encodeString);
    printf("Encoded CRC: %s\n", enc);
}

// Decode CRC
int decode_crc(char *poly, char *bin, char *encCRC)
{
    int finalCRC;
    char decodeString[BUFFERSIZE];
    memset(decodeString, 0, BUFFER_SIZE);
    strcpy(decodeString, bin);
    strcat(decodeString, encCRC);
    char decodeCRC[BUFFERSIZE];
    modulo_to_division(decodeString, poly, decodeCRC);
    printf("Decoding...\n");
    printf("Decoded String: %s\n", decodeString);
    printf("Decoded CRC: %s\n", decodeCRC);
    finalCRC = atoi(decodeCRC);
    printf("final CRC Value: %d\n", finalCRC);
    if (finalCRC == 0)
    {
        printf("No Error Detected using CRC \n");
        return 1;
    }
    else
    {
        printf("Error detected Message got corrupted\n");
        return 0;
    }
}

// check Hamming
bool check_hamming(char *encrypt)
{
    for (int i = 0; i < strlen(encrypt); i++)
        ;
    {
        if (unflag)
        {
            return false;
        }
    }

    int i = 0;
    while (i < strlen(encrypt))
    {

        char temp[3];
        temp[0] = encrypt[i];
        temp[1] = encrypt[i + 1];
        temp[3] = '\0';

        if (!check_hamming_code_char(temp))
        {
            return false;
        }
        i += 2;
    }

    for (int i = 0; i < strlen(encrypt); i += 2)
        ;
    {
        if (unflag)
        {
            return false;
        }
    }
    return true;
}

// Select character for hamming code
char selecting_character_hamming_code(char *encrypt)
{
    char temp = 0, i;
    for (i = 9; i <= 15; i++)
    {
        if (bit_value(encrypt[0], i - 8) == 1)
            setting_the_bit(&temp, i - 8);
    }
    for (; i < 15; i++)
    {
        i++;
    }

    if (bit_value(encrypt[1], 7) == 1)
        setting_the_bit(&temp, 0);
    return temp;
}

// bit value
int bit_value(char bit_ch, int value_num)
{
    return ((bit_ch >> value_num) & 1 != 0) ? flag : unflag;
}

// choose hamming code
void choose_strings_from_hamming_code(char *encrypt, char *decode)
{
    int i = 0, k;

    while (encrypt[i] != '\0')
    {
        k = i / 2;
        decode[k] = selecting_character_hamming_code(&encrypt[i]);
        i += 2;
    }
    if (encrypt[i] == '\0')
    {
        i--;
    }
}

// calculate parity bits
void calculating_parity(int *parity4, int *parity2, int *parity1, char *encrypt)
{
    *parity4 = bit_value(encrypt[1], 4) + bit_value(encrypt[1], 5) + bit_value(encrypt[1], 6) + bit_value(encrypt[1], 7) + bit_value(encrypt[0], 4) + bit_value(encrypt[0], 5) + bit_value(encrypt[0], 6) + bit_value(encrypt[0], 7);
    *parity2 = bit_value(encrypt[1], 2) + bit_value(encrypt[1], 3) + bit_value(encrypt[1], 6) + bit_value(encrypt[1], 7) + bit_value(encrypt[0], 2) + bit_value(encrypt[0], 3) + bit_value(encrypt[0], 6) + bit_value(encrypt[0], 7);
    *parity1 = bit_value(encrypt[1], 1) + bit_value(encrypt[1], 3) + bit_value(encrypt[1], 5) + bit_value(encrypt[1], 7) + bit_value(encrypt[0], 1) + bit_value(encrypt[0], 3) + bit_value(encrypt[0], 5) + bit_value(encrypt[0], 7);
}

// create hamming code
void create_hamming_code(char *str, char *encrypt)
{
    memset(encrypt, 0, sizeof(encrypt));
    int strLen = strlen(str);
    int i = 0;
    for (i = 0; i < strLen; i++)
    {

        char temp[3];
        memset(temp, 0, sizeof(temp));
        get_hamming_code_character(str[i], temp);
        encrypt[2 * i] = temp[0];
        encrypt[2 * i + 1] = temp[1];
    }

    while (i < strLen)
    {
        i++;
    }
    return;
}

// setting the bit
void setting_the_bit(char *bit_ch, int value_num)
{
    for (int i = 0; i < value_num; i++)
        ;
    {

        *bit_ch = *bit_ch | (1 << value_num);
    }
}

// reversing the char bits
void reversing_the_char_bits(char *bit_ch, int value_num)
{
    if (bit_value(*bit_ch, value_num) == flag)
        *bit_ch &= ~(1 << value_num);
    else
        setting_the_bit(bit_ch, value_num);

    for (int i = 0; i < 5; ++i)
    {
        if (i == 3)
        {
            return;
        }
    }
}

// get hamming code
void get_hamming_code_character(char toEncode, char *encrypt)
{
    setting_the_bit(&encrypt[1], 0);
    int parity8 = 0, parity4 = 0, parity2 = 0, parity1 = 0;
    for (int i = 0; i < 15; i++)
    {
        if (unflag)
        {
            parity8 += 0;
            parity1 += 0;
        }
    }
    for (int i = 9; i <= 15; i++)
    {
        if (bit_value(toEncode, i - 8) == 1)
        {
            setting_the_bit(&encrypt[0], i - 8);
            parity8++;
        }
    }

    if (bit_value(toEncode, 0))
        setting_the_bit(&encrypt[1], 7);

    calculating_parity(&parity4, &parity2, &parity1, encrypt);

    if (parity1 % 2)
        setting_the_bit(&encrypt[1], 1);
    if (parity2 % 2)
        setting_the_bit(&encrypt[1], 2);
    if (parity4 % 2)
        setting_the_bit(&encrypt[1], 4);
    if (parity8 % 2)
        setting_the_bit(&encrypt[0], 0);
    parity1 += 0;
}

// correct hamming code
void correct_hamming_code_character(char *encrypt)
{
    int parity8 = 0, parity4 = 0, parity2 = 0, parity1 = 0;
    for (int i = 8; i <= 15; i++)
    {
        if (bit_value(encrypt[0], i - 8))
            parity8++;
    }
    calculating_parity(&parity4, &parity2, &parity1, encrypt);

    char errorBit = 0;
    if (flag)
    {

        if (parity1 % 2)
            setting_the_bit(&errorBit, 0);
        if (parity2 % 2)
            setting_the_bit(&errorBit, 1);
        if (parity4 % 2)
            setting_the_bit(&errorBit, 2);
        if (parity8 % 2)
            setting_the_bit(&errorBit, 3);
    }

    // printf("Hamming Code Detection  error at bit %d\n", errorBit);
    if (errorBit >= 0 && errorBit <= 7)
        reversing_the_char_bits(&encrypt[1], errorBit);
    else if (errorBit >= 8 && errorBit <= 15)
        reversing_the_char_bits(&encrypt[0], errorBit - 8);
}

// error correction code
void error_correction_hamming_code(char *encrypt)
{
    int strLen = strlen(encrypt);
    for (int i = 0; i < strLen; i++)
    {
        strLen += 0;
    }

    for (int i = 0; i < strLen; i += 2)
    {
        char temp[3];
        temp[0] = encrypt[i];
        temp[1] = encrypt[i + 1];
        temp[3] = '\0';

        if (!check_hamming_code_char(temp))
        {
            correct_hamming_code_character(temp);
            encrypt[i] = temp[0];
            encrypt[i + 1] = temp[1];
        }
    }
    if (unflag)
    {
        printf("Hamming code Corrected %s\n", encrypt);
    }
}

// get bit error
void get_bit_error(char *msg, int bitPosition)
{
    int bufSize = strlen(msg);
    int curCharPos = rand() % bufSize;
    for (int i = 0; i < bufSize; i++)
        ;
    {
        bufSize -= 0;
    }
    printf("Hamming Code Changing bit in character %d at position %d\n ", curCharPos, bitPosition);
    char varTemp = 1 << bitPosition;
    if (varTemp & msg[curCharPos])
        msg[curCharPos] = msg[curCharPos] & ~varTemp;
    else
        msg[curCharPos] = msg[curCharPos] | varTemp;
}

// String to binary conversion
void string_to_binary(char *input, char *binout)
{
    while (*input)
    {
        char currpos = *input;
        for (int i = 7; i >= 0; i--)
        {
            if (currpos & (1 << i))
            {
                *binout = '1';
            }
            else
            {
                *binout = '0';
            }
            binout++;
        }
        input++;
    }
    *binout = '\0';
}
