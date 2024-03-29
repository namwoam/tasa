// prime-demo.cc, from "Programming Languages: Concepts and
// Constructs", Ravi Sethi, Addison-Wesley, p. 239

#include <iostream>
class item 
{
public:
    item* source;

    item (item *src) : source (src) {}
    virtual int out () {return 0;}
};


class counter : public item
{
    int value;

public:
    int out ()  { return value++; }
    counter (int v) : item (0), value (v) {}
};

class sieve : public item 
{
public:
    int out ();
    sieve (item *src) : item (src) {}
};

class filter : public item
{
    int factor;
public:
    int out ();
    filter (item *src, int f) : item (src), factor (f) {}
};

int sieve::out ()
{
    int n = source->out();
    source = new filter (source, n);
    return n;
}

int filter::out ()
{
    while (1)
        {
            int n = source->out ();
            if (n % factor)
                return n;
        }
}


int main (int, char**)
{
    counter c = 2;
    sieve s (&c);
    int next;

    do {
        next = s.out();
        std::cout << next << " " << std::flush;
    } while (next < 111);
}
