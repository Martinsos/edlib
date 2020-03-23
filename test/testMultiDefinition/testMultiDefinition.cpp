#include "edlib.hpp"

void nullFunction();

int main(){
    nullFunction();
    printf("The linker didn't catch any multiple function definition during linking process\n");
    return 0;
}
