#include <stdio.h>
#include <chaiscript/chaiscript.hpp>

static void print(const char *msg) {
    printf("%s\n", msg);
}

int main(int argc, char *argv[])
{
    chaiscript::ChaiScript chai;

    chai.add(chaiscript::var("Hello World!"), "msg");
    chai.add(chaiscript::fun(&print), "print");

    chai.eval_file("test_script.chai");

    return 0;
}