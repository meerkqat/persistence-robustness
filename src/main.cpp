#include "persistence.h"

int main(int argc, char **argv)
{
    Persistence p;
    p.set_in_file("dataset/data2");
    p.calculate();
    return 0;
}
