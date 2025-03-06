#include "display/color_output.h"
#include <stdio.h>
#include <windows.h>

void print_color(int color, const char *text) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    printf("%s", text);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN |
                                      FOREGROUND_BLUE);
}
