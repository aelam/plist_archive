//
//  main.c
//  bplist_parser
//
//  Created by Wang Lun on 2020/9/28.
//  Copyright © 2020 MicroFocus. All rights reserved.
//

#include <stdio.h>
#include <plist/plist.h>
#include "bplist_parser.h"


char *readFile(char *fileName) {
    FILE *file = fopen(fileName, "r");
    char *code;
    size_t n = 0;
    int c;

    if (file == NULL) return NULL; //could not open file
    fseek(file, 0, SEEK_END);
    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    code = malloc(f_size);

    while ((c = fgetc(file)) != EOF) {
        code[n++] = (char)c;
    }

    code[n] = '\0';

    return code;
}


int main(int argc, const char * argv[]) {
    // insert code here...
    plist_t plist = NULL;
    const char *data = readFile("/Users/wanglun/Documents/workspace/bplist_parser/bplist_parser/processes.plist");
    plist_from_xml(data, strlen(data), &plist);
    
    parse_bplist(plist);
    
    printf("Hello, World!\n");
    return 0;
}
