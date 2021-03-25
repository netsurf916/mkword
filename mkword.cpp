#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

struct tuple
{
    char      Key;
    uint32_t  I;
    tuple    *Next;
};

static const char  startflag = '$';
static const char  endflag   = '*';
static const char *charset   = "abcdefghijklmnopqrstuvwxyz";

void init(tuple *data)
{
    for(int i = 0; i < strlen(charset); ++i)
    {
        data[i].Key  = charset[i];
        data[i].I    = 0;
        data[i].Next = NULL;
    }
}

void printchain(tuple *data)
{
    for(int i = 0; i < strlen(charset); ++i)
    {
        printf(" [%c:%d]", data[i].Key, data[i].I);
        tuple *temp = data[i].Next;
        while(temp != NULL)
        {
            printf("->(%c:%d)", temp->Key, temp->I);
            temp = temp->Next;
        }
        printf("\n");
    }
}

char tolower(char c)
{
    if(c >= 'A' && c <= 'Z') return (c - ('A' - 'a'));
    return c;
}

bool matchcharset(const char *s)
{
    if(s == NULL) return false;
    int len   = strlen(s);
    int match = 0;
    for(int i = 0; i < len; ++i)
    {
        for(int j = 0; j < strlen(charset); ++j)
        {
            if(s[i] == charset[j] || tolower(s[i]) == charset[j])
            {
                ++match;
                break;
            }
        }
    }
    return (match == len);
}

bool addhit(tuple *data, char p, char c)
{
    bool ok = false;
    if(data == NULL) return ok;
    p = tolower(p);
    c = tolower(c);
    if(p == startflag)
    {
        for(int i = 0; i < strlen(charset); ++i)
        {
            if(data[i].Key == c)
            {
                ++data[i].I;
                ok = true;
                break;
            }
        }
    } else {
        for(int i = 0; i < strlen(charset); ++i)
        {
            if(data[i].Key == p)
            {
                if(data[i].Next == NULL)
                {
                    data[i].Next = new tuple();
                    if(data[i].Next != NULL)
                    {
                        data[i].Next->Key  = c;
                        data[i].Next->I    = 1;
                        data[i].Next->Next = NULL;
                        ok = true;
                    }
                } else {
                    tuple *temp = data[i].Next;
                    bool match = false;
                    while(!match && temp != NULL)
                    {
                        if(temp->Key == c)
                        {
                            match = true;
                            ++temp->I;
                            ok = true;
                        }
                        temp = temp->Next;
                    }
                    if(!match)
                    {
                        temp = data[i].Next;
                        while(temp->Next != NULL)
                        {
                            temp = temp->Next;
                        }
                        temp->Next = new tuple();
                        if(temp->Next != NULL)
                        {
                            temp = temp->Next;
                            temp->Key  = c;
                            temp->I    = 1;
                            temp->Next = NULL;
                            ok = true;
                        }
                    }
                }
                break;
            }
        }
    }
    return ok;
}

bool addword(tuple *data, const char *s)
{
    char prev = startflag;
    if(!matchcharset(s)) return false;
    if(strlen(s) <= 0) return false;
    bool ok = true;
    for(int i = 0; ok && i < strlen(s); ++i)
    {
        ok = addhit(data, prev, s[i]);
        prev = s[i];
    }
    ok = ok && addhit(data, prev, endflag);
    return ok;
}

uint32_t getstartlettertotal(tuple *data)
{
    uint32_t t = 0;
    if(data == NULL) return t;
    for(int i = 0; i < strlen(charset); ++i)
    {
        t += data[i].I;
    }
    return t;
}

uint32_t getchaintotal(tuple *data)
{
    uint32_t t = 0;
    tuple *temp = data;
    while(temp)
    {
        t += temp->I;
        temp = temp->Next;
    }
    return t;
}

bool getword(tuple *data, char *s, int l)
{
    int      index  = 0;
    uint32_t total  = getstartlettertotal(data);
    uint32_t random = rand() % total;
    ++random; // Make the range = [1, total]
    total = 0;
    for(int i = 0; i < strlen(charset); ++i)
    {
        total += data[i].I;
        if((total >= random) && (data[i].I > 0))
        {
            s[index++] = data[i].Key;
            s[index]   = 0;
            break;
        }
    }
    bool done = false;
    while(!done && (index < l))
    {
        for(int i = 0; !done && (index < l) && (i < strlen(charset)); ++i)
        {
            // Find a key matching the previous letter
            if(data[i].Key == s[index - 1])
            {
                total  = getchaintotal(data[i].Next);
                if(total == 0)
                {
                    s[index] = 0;
                    done = true;
                    break;
                }
                random = rand() % total;
                ++random; // Make the range = [1, total]
                total  = 0;
                tuple *temp = data[i].Next;
                while(temp != NULL)
                {
                    total += temp->I;
                    if(total >= random)
                    {
                        if(temp->Key == endflag)
                        {
                            s[index++] = 0;
                            done = true;
                        } else {
                            s[index++] = temp->Key;
                            s[index]   = 0;
                        }
                        i = -1;
                        break;
                    }
                    temp = temp->Next;
                }
            }
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    char temp[64] = {0};

    tuple *data = new tuple[strlen(charset)];
    if( data == NULL ) return 1;

    init(data);
    if(argc == 2)
    {
        int i = 0;
        FILE *input = fopen(argv[1], "r");
        if(input != NULL)
        {
            while(!feof(input))
            {
                temp[i] = fgetc(input);
                if((temp[i] >= 'a' && temp[i] <= 'z') || (temp[i] >= 'A' && temp[i] <= 'Z'))
                {
                    ++i;
                    if(i >= (sizeof(temp)/sizeof(temp[0]) - 1))
                    {
                        // The buffer is saturated
                        temp[i] = 0;
                        addword(data, temp);
                        i = 0;
                    }
                } else {
                    temp[i] = 0;
                    if(i > 0) addword(data, temp);
                    i = 0;
                }
            }
            fclose(input);
        }
        printchain(data);

        while(getword(data, temp, (sizeof(temp)/sizeof(temp[0])) - 1))
        {
            if(strlen(temp) >= 4)
            {
                printf("%s ", temp);
                fflush(stdout);
                usleep(80000);
            }
        }
    } else {
        printf(" Usage: %s <word list>\n", argv[0]);
        return 0;
    }

    // Cleanup and exit
    for(int i = 0; i < strlen(charset); ++i)
    {
        tuple *temp = data[i].Next;
        while(temp != NULL)
        {
            tuple *next = temp->Next;
            delete temp;
            temp = next;
        }
    }
    delete [] data;
    return 0;
}

