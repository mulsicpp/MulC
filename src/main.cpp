#include <stdio.h>

#include "Mulc.h"

static void print(const char *msg) {
    printf("%s\n", msg);
}

int main(int argc, char *argv[])
{
    Mulc::run();

    return 0;
}