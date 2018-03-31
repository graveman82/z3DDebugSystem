#include <stdio.h>
#include <conio.h>
#include "z3DDebugSystem.h"

// assert
void example1() {
    z3D::DebugTool::SetReleaseMode();

    const char* filename = "dummy.txt";
    FILE* f = fopen(filename, "r");
    Z3D_ASSERT_HIGH1(f != 0, "Failed open '%s' file\n", filename, true);
    if (f == 0) {
        Z3D_ERROR1(Z3D_ERROR_PERMISSIBLE, f==0, "Failed open '%s' file\n", filename, true);

    }

    int x = 2;
    if (x*x != 5)
        Z3D_ERROR1(Z3D_ERROR_PERMISSIBLE, x*x != 5, "%d square is not acceptable",x, true);
}

int main(){
    example1();

    printf("Press any key to quit...\n");
    getch();
    return 0;
}
