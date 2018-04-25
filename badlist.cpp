#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <iostream>

typedef char* ListElem_t;

const int Crashcan1 = 666666666, Crashcan2 = 999999999;
const int Max_list_size = 20000, Smallptr = 7;

struct Elem
{
    int canaryleft = Crashcan1;
    ListElem_t value;
    Elem* next;
    Elem* prev;
    int canaryright = Crashcan2;
};

class List_t
{
    private:
        int canaryleft;

        Elem* head;
        Elem* tail;
        int size;

        int canaryright;

        bool dump                     (const int value,    FILE* file_error_info);
        bool dump                     (const double value, FILE* file_error_info);
        bool dump                     (const void* value,  FILE* file_error_info);
    public:
        bool ListElemOK               ();
        bool ListOK                   ();
        void Dump                     ();

        List_t                        ();
        ~List_t                       ();

        Elem* HeadEl                  ();
        Elem* TailEl                  ();
        Elem* NextEl                  (Elem* elem);
        Elem* PrevEl                  (Elem* elem);
        ListElem_t ValEl              (Elem* elem);
        int Size                      ();

        void PushFront                (const ListElem_t value);
        void PushBack                 (const ListElem_t value);
        ListElem_t PopBack            ();
        ListElem_t PopFront           ();
        void Insert                   (Elem* pointer, const ListElem_t value);
        ListElem_t Erase              (Elem* pointer);

        Elem* InOrder                 ();

};

#define assert(condition)                                                   \
        if (!condition)                                                     \
        {                                                                   \
        printf("ASSERTION %s in %s (%d)\n", #condition, __FILE__, __LINE__);\
        abort();                                                            \
        }

//--------------------------------------------------------
//! Macro to choose DEBUG mode
//!
//! @note write '1' to 'if' to use DEBUG mode
//! @note write '0' to 'if' not to use DEBUG mode
//--------------------------------------------------------
#if (0)
    #define ASSERT_LIST(); \
            if (!ListOK ())\
            {                 \
            Dump();    \
            assert(0);        \
            }
#else
    #define ASSERT_LIST();
#endif


List_t::List_t ():
    canaryleft (Crashcan1),

    head (nullptr),
    tail (nullptr),
    size (0),

    canaryright (Crashcan2)
    {
        ASSERT_LIST()
    }

List_t::~List_t ()
{
    ASSERT_LIST()

    while (head != tail)
    {
        Elem* head_c = head;
        head = head->next;

        delete (head_c);
    }
    delete (head);

    head = nullptr;
    tail = nullptr;
    size = 0;
}

bool List_t::ListElemOK ()
{
    bool isOK = 1;

    Elem* current = head;

    for (int i = 0; i < size; i++)
    {
        if (current != head)
        {
            if (current->prev->next != current)
                isOK = 1;
        }

        else
        {
            if (current->prev != nullptr)
                isOK = 0;
        }

        if (current != tail)
        {
            if (current->next->prev != current)
                isOK = 1;
        }

        else
        {
            if (current->next != nullptr)
                isOK = 0;
        }

        if (current == nullptr)
            isOK = 0;

        if (current->canaryleft != Crashcan1 || current->canaryright != Crashcan2)
            isOK = 0;

        current = current->next;
    }

    if (current != nullptr)
        isOK = 0;

    return isOK;
}

bool List_t::ListOK ()
{
    return this && size >= 0 && canaryleft == Crashcan1 && canaryright == Crashcan2 && ListElemOK ();
}

bool List_t::dump (const int value, FILE* output_file)
{
    fprintf (output_file, "%d ", value);

    return 1;
}

bool List_t::dump (const double value, FILE* output_file)
{
    if (isnan (value))
    {
        fprintf (output_file, "%f\\nnot a number ", value);

        return 0;
    }

    else
    {
        fprintf (output_file, " %f ", value);

        return 1;
    }
}

bool List_t::dump (const void* value, FILE* output_file)
{
    ListElem_t* val = (ListElem_t*) value;

    if (val == nullptr)
    {
        fprintf (output_file, " %p\\nzero pointer ", val);

        return 0;
    }

    else
    {
        fprintf (output_file, " %p ", val);

        return 1;
    }
}

void List_t::Dump ()
{
    FILE* output_file = fopen ("Dump.dot", "w");
    //assert (output_file);

    fprintf (output_file, "digraph List\n{\n\trankdir = LR\n");
    fprintf (output_file, "\t\tnode [ shape = \"box\", color = \"black\" ]\n");
    fprintf (output_file, "\t\tedge [ color = \"black\" ]\n\n");

    if (canaryleft != Crashcan1)
        fprintf (output_file, "\tCanaryLEFT [ label = \"CanaryLEFT = %d\\nBUT EXPECTED %d\","
                 "color = \"red\" ]\n", canaryleft, Crashcan1);
    else
        fprintf (output_file, "\tCanaryLEFT [ label = \"CanaryLEFT = %d\" ]\n", canaryleft);

    if (canaryright != Crashcan2)
        fprintf (output_file, "\tCanaryRIGHT [ label = \"CanaryRIGHT = %d\\nBUT EXPECTED %d\","
                 "color = \"red\" ]\n", canaryright, Crashcan2);
    else
        fprintf (output_file, "\tCanaryRIGHT [ label = \"CanaryRIGHT = %d\" ]\n", canaryright);

    if (size >= 0)
        fprintf (output_file, "\tsize [ label = \"size = %d\" ]\n\n", size);
    else
        fprintf (output_file, "\tsize [ label = \"size = %d\\n<0\", color = \"red\"]\n\n", size);

    Elem* current = head;
    //assert (current);

    int number = 1;

    while (current != nullptr)
    {
        fprintf (output_file, "\telem%d [ shape = \"record\", label = \"<pointer>%d\\n", number, number);

        if (current == head)
        {
            fprintf (output_file, "HEAD\\n");
        }

        if (current == tail)
        {
            fprintf (output_file, "TAIL\\n");
        }

        fprintf (output_file, "%p | {<prev> prev\\n %p | value\\n "
                 , current, current->prev);

        bool isnormal = dump (current->value, output_file);

        if (current->canaryleft != Crashcan1 || current->canaryright != Crashcan2)
            isnormal = 0;

        fprintf (output_file, "| <next> next\\n%p} | {canaryLEFT\\n%d | canaryRIGHT\\n%d} \", color = "
                 , current->next, current->canaryleft, current->canaryright);

        if (isnormal)
            fprintf (output_file, "\"blue\" ]\n");

        else
            fprintf (output_file, "\"red\" ]\n");

        number++;
        current = current->next;
    }

    fprintf (output_file, "\n");
    current = head;

    number = 1;

    while (current != nullptr)
    {
        if (current->prev != nullptr)
            fprintf (output_file, "\telem%d:<prev> -> elem%d:<pointer>\n", number, number - 1);

        if (current->next != nullptr)
            fprintf (output_file, "\telem%d:<next> -> elem%d:<pointer> [ color = \"green\" ]\n", number, number + 1);

        number++;
        current = current->next;
    }


    fprintf (output_file, "}");
    fclose (output_file);


    system ("dot -Tpng Dump.dot -o dump.png");
}


void List_t::PushBack (const ListElem_t value)
{
    ASSERT_LIST()

    if (size >= Max_list_size)
    {
        printf ("NOT ENOUGH MEMORY\n");
        assert (0);
    }

    Elem* elem = new Elem;

    if (size == 0)
    {
        head = elem;
        tail = elem;

        elem->next = nullptr;
        elem->prev = nullptr;
        elem->value = value;
    }

    else
    {
        tail->next = elem;

        elem->prev = tail;
        elem->next = nullptr;
        elem->value = value;

        tail = elem;
    }
    size++;

    ASSERT_LIST()
}


void List_t::PushFront (const ListElem_t value)
{
    ASSERT_LIST()

    if (size >= Max_list_size)
    {
        printf ("NOT ENOUGH MEMORY\n");
        assert (0);
    }

    Elem* elem = new Elem;

    if (size == 0)
    {
        head = elem;
        tail = elem;

        elem->next = nullptr;
        elem->prev = nullptr;
        elem-> value = value;
    }

    else
    {
        head->prev = elem;

        elem->next = head;
        elem->prev = nullptr;
        elem->value = value;

        head = elem;
    }
    size++;

    ASSERT_LIST()
}

ListElem_t List_t::PopBack ()
{
    ASSERT_LIST()

    if (size == 0)
    {
        printf ("LIST IS EMPTY\n");
        assert (0)
    }

    ListElem_t value = tail->value;
    Elem* elem = tail;

    if (size == 1)
    {
        head = nullptr;
        tail = nullptr;
    }

    else
    {
        tail->prev->next = nullptr;

        tail = tail->prev;
    }
    size--;

    delete elem;

    ASSERT_LIST()

    return value;
}

ListElem_t List_t::PopFront ()
{
    ASSERT_LIST()

    if (size == 0)
    {
        printf ("LIST IS EMPTY\n");
        assert (0);
    }

    ListElem_t value = head->value;
    Elem* elem = head;

    if (size == 1)
    {
        head = nullptr;
        tail = nullptr;
    }

    else
    {
        head->next->prev = nullptr;

        head = head->next;
    }
    size--;

    delete elem;

    ASSERT_LIST()

    return value;
}

void List_t::Insert (Elem* pointer, const ListElem_t value)
{
    ASSERT_LIST()

    if (size >= Max_list_size)
    {
        printf ("NOT ENOUGH MEMORY\n");
        assert (0);
    }

    if (pointer == tail)
        PushBack (value);

    else if (pointer->next->prev != pointer)
    {
        printf ("BAD POINTER %p\n", pointer);//value
        assert (0);
    }

    else
    {
        Elem* elem = new Elem;

        elem->next = pointer->next;
        elem->prev = pointer;
        elem->value = value;

        elem->next->prev = elem;
        elem->prev->next = elem;

        size++;
    }

    ASSERT_LIST()
}

ListElem_t List_t::Erase (Elem* pointer)
{
    ASSERT_LIST()

    if (size == 0)
    {
        printf ("LIST IS EMPTY\n");
        assert (0);
    }

    if (pointer == tail)
        return PopBack ();

    else if (pointer == head)
        return PopFront ();

    else if (pointer->next->prev == pointer && pointer->prev->next == pointer)
    {
        Elem* elem = pointer;
        ListElem_t value = elem->value;

        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;

        delete elem;

        size--;

        ASSERT_LIST()

        return value;
    }
}

Elem* List_t::HeadEl ()
{
    ASSERT_LIST()

    return head;
}

Elem* List_t::TailEl ()
{
    ASSERT_LIST()

    return tail;
}

Elem* List_t::NextEl (Elem* elem)
{
    ASSERT_LIST()

    if (elem->next->prev != elem)
    {
        printf ("BAD POINTER %p\n", elem);
        assert (0);
    }

    return elem->next;
}

Elem* List_t::PrevEl (Elem* elem)
{
    ASSERT_LIST()

    if (elem->prev->next != elem)
    {
        printf ("BAD POINTER %p\n", elem);
        assert (0);
    }

    return elem->prev;
}

ListElem_t List_t::ValEl (Elem* elem)
{
    ASSERT_LIST()

    if (elem != tail && elem != head && elem->next->prev != elem)
    {
        printf ("BAD POINTER %p\n", elem);
        assert (0);
    }

    return elem->value;
}

int List_t::Size ()
{
    ASSERT_LIST()

    return size;
}

Elem* List_t::InOrder ()
{
    ASSERT_LIST()

    Elem buffer[size] = {};

    int i = 0;

    for (Elem* current = head; current != nullptr; current = current->next)
    {
        buffer[i++] = *current;
    }

    ASSERT_LIST()

    return buffer;
}
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>


const unsigned int Tablesize = 1019, Unsint = 2147483648;


struct String{
    char* str;
    int length;
};

int sizeofFILE                (FILE *filein);
char* make_buffer             ();
int num_of_str                (const char* buffer);
void make_text                (String* text, char* buffer);
unsigned int hash_one         (const String string);
unsigned int hash_ded         (const String string);
unsigned int hash_len         (const String string);
unsigned int hash_sum         (const String string);
unsigned int hash_xor         (const String string);
unsigned int hash_gnu         (const String string);
void Make_hash_table          (String* text, const int number_of_strings, const char* filename,
                               unsigned int hash_func (const String));

int main()
{
    clock_t t = clock();
    char* buffer = make_buffer ();
    std::cout << clock() - t << " time for making buffer\n";
    t = clock();
    const int num_of_strings = num_of_str (buffer);

    String* text = (String*) calloc(num_of_strings, sizeof (*text));
    assert(text);
    make_text (text, buffer);
    std::cout << clock() - t << " time for making text\n";
    t = clock();


    Make_hash_table (text, num_of_strings, "table_1.csv", hash_one);
    Make_hash_table (text, num_of_strings, "table_2.csv", hash_ded);
    Make_hash_table (text, num_of_strings, "table_3.csv", hash_len);
    Make_hash_table (text, num_of_strings, "table_4.csv", hash_sum);
    Make_hash_table (text, num_of_strings, "table_5.csv", hash_xor);
    Make_hash_table (text, num_of_strings, "table_6.csv", hash_gnu);
    std::cout << clock() - t << " time for making hash table\n";

    free (text);
    free (buffer);
    return 0;
}

char* make_buffer ()
{
    FILE* filein = fopen ("bible.txt", "rb");
    assert(filein);

    const int file_length = sizeofFILE (filein);
    assert (file_length != -1);

    char* buffer = (char*) calloc (file_length + 1, sizeof (*buffer));
    assert(buffer);

    fread (buffer, file_length, sizeof (*buffer), filein);

    return buffer;
}

int sizeofFILE (FILE *filein){
    fseek (filein, 0, SEEK_END);
    const int file_length = ftell (filein);
    rewind (filein);
    return file_length;
}

int num_of_str (const char* buffer){
    int num_str = 1, i = 0;
    while (buffer[i] != '\0'){
        if (!isalpha(buffer[i]) && buffer[i] != '\0') num_str++;
        i++;
    }
    return num_str;
}

void make_text (String* text, char* buffer)
{
    int num_of_strings = 0;
    text[num_of_strings++].str = buffer;
    int i = 0;
    while (buffer[i] != '\0'){
        if (!isalpha(buffer[i]) && buffer[i] != '\0'){
            buffer[i] = '\0';
            text[num_of_strings - 1].length = buffer + i - text[num_of_strings - 1].str + 1;
            text[num_of_strings++].str = buffer + i + 1;
        }
        i++;
    }

    text[num_of_strings - 1].length = buffer + i - text[num_of_strings - 1].str + 1;
}

void Make_hash_table (String* text, const int number_of_strings, const char* filename,
                      unsigned int hash_func (const String))
{
    FILE* output_file = fopen (filename, "w");
    assert (output_file);

    List_t hash_table[Tablesize] = {};
    short cmpval = 0;

    for (int i = 0; i < number_of_strings; i++)
    {
        if (text[i].str[0] != '\0')
        {
            unsigned int hash_word = hash_func (text[i]) % Tablesize;

            bool isINlist = 0;


            __asm__(        ".intel_syntax noprefix\n\t"
                    "iter:   cmp rdi, 0\n\t"
                            "je fin\n\t"
                            "add rdi, 8\n\t"
                            "mov rdx, rdi\n\t"
                            "mov rdi, [rdi]\n\t"
                            "mov rsi, %1\n\t"
                            "nxt: mov al, [rdi]\n\t"
                            "cmpsb\n\t"
                            "jne mism\n\t"
                            "cmp al, 0\n\t"
                            "jne nxt\n\t"
                            "mov %0, 1\n\t"
                            "jmp fin\n\t"
                    "mism:   add rdx, 8\n\t"
                            "mov rdi, rdx\n\t"
                            "mov rdi, [rdi]\n\t"
                            "jmp iter\n\t"
                    "fin:\n"
                            ".att_syntax\n\t"
                            :"=r"(isINlist)
                            :"r"(text[i].str), "D"(hash_table[hash_word].HeadEl ())
                            :"%rax", "rdx", "rsi"
                );

            /*for (Elem* current = hash_table[hash_word].HeadEl (); current != nullptr; current = current->next)
            {
                //printf("%d\n", sizeof(*current));
                __asm__(        ".intel_syntax noprefix\n\t"
                                "nxt: mov al, [rdi]\n\t"
                                "cmpsb\n\t"
                                "jne mism\n\t"
                                "cmp al, 0\n\t"
                                "jne nxt\n\t"
                                "mov %0, 0\n\t"
                                "jmp fin\n\t"
                        "mism:   mov %0, 1\n\t"
                        "fin:\n"
                                ".att_syntax\n\t"
                                :"=r"(cmpval)
                                :"D"(current->value), "S"(text[i].str)
                                :"%rax"
                );
                //cmpval = strcmp (current->value, text[i].str);
                //if (current -> next)
                //    printf("%s\n", ((current + 16)));

                if (!cmpval)
                {
                    isINlist = 1;
                    break;
                }
            }*/

            if (!isINlist)
                hash_table[hash_word].PushBack (text[i].str);
        }
    }

    //hash_table[0].Dump();

    for (int i = 0; i < Tablesize; i++)
    {
        fprintf (output_file, "%d; %d\n", i, hash_table[i].Size ());
    }

    fclose (output_file);
}

unsigned int hash_one (const String string)
{
    return 1;
}

unsigned int hash_ded (const String string)
{
    return tolower (string.str[0]) - 'a';
}

unsigned int hash_len (const String string)
{
    return string.length;
}

unsigned int hash_sum (const String string)
{
    unsigned int hash = 0;

    for (const char* cur = string.str; *cur != '\0'; cur++)
    {
        hash += (*cur);
    }

    return hash;
}

unsigned int hash_xor (const String string)
{
    unsigned int hash = 0;

    for (const char* cur = string.str; *cur != '\0'; cur++)
    {
        hash = ( (hash >> 1) | (hash << 31)) ^ (*cur);
    }

    return hash;
}

unsigned int hash_gnu (const String string)
{
    unsigned int hash = 0;

    for (const char* cur = string.str; *cur != '\0'; cur++)
    {
         hash = ( (hash << 5) + hash) + (*cur);
    }

    return hash;
}

