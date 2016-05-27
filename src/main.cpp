#include "persistence.h"

int main(int argc, char **argv)
{
    Persistence p;
    p.set_in_file("dataset/data1");
    p.calculate();
    return 0;
}
