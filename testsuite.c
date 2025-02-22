#include <stdlib.h>  // Required for system()
#include <stdio.h>
int main() {
    int status = system("./testing.sh");
    return status;
}
