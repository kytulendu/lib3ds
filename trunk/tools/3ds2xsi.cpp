/*
    Copyright (C) 1996-2008 by Jan Eric Kyprianidis <www.kyprianidis.com>
    All rights reserved.
    
    This program is free  software: you can redistribute it and/or modify 
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 2.1 of the License, or 
    (at your option) any later version.

    Thisprogram  is  distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
    GNU Lesser General Public License for more details.
    
    You should  have received a copy of the GNU Lesser General Public License
    along with  this program; If not, see <http://www.gnu.org/licenses/>. 
*/
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <assert.h>
#define __XSI_APPLICATION_
#define XSI_STATIC_LINK

#ifdef _MSC_VER
#pragma warning ( disable : 4996 )
#endif

#include "SemanticLayer.h"
#include <lib3ds.h>


static void
help() {
    fprintf(stderr,
            "Syntax: 3ds2xsi filename_in filename_out\n"
            "\n"
           );
    exit(1);
}


static char* input = 0;
static char* output = 0;


static void
parse_args(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
                help();
            } else {
                help();
            }
        } else {
            if (!input) {
                input = argv[i];
            } else if (!output) {
                output = argv[i];
            } else {
                help();
            }
        }
    }
    if (!input || !output) {
        help();
    }
}


int
main(int argc, char **argv) {
    Lib3dsFile *f = 0;

    parse_args(argc, argv);

    f = lib3ds_file_open(input);
    if (!f) {
        fprintf(stderr, "***ERROR***\nLoading file failed: %s\n", input);
        exit(1);
    }

	CSLScene scene;
    scene.Create(5, 0, FORMAT_TEXT);
    scene.Write(output);


    lib3ds_file_free(f);
    return 0;
}
