/*
    Copyright (C) 2008 by Jan Eric Kyprianidis <www.kyprianidis.com>
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
#include <lib3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _MSC_VER
#pragma warning ( disable : 4996 )
#endif


/*!
\example 3ds2obj.c

Converts a <i>3DS</i> file to an <i>OBJ</i> file.

\code
Syntax: 3ds2obj [options] filename [options]\n"

Options:
  -h           This help\n"
\endcode
*/


static void
help() {
    fprintf(stderr,
            "3ds2obj\n"
            "Copyright (C) 2008 by Jan Eric Kyprianidis <www.kyprianidis.com>\n"
            "All rights reserved.\n"
            "\n"
            "Syntax: 3dsdump [options] 3ds-file obj-file\n"
            "\n"
            "Options:\n"
            "  -h           This help\n"
            "\n"
           );
    exit(1);     
}


static const char* input = 0;
static const char* output = 0;


static void parse_args(int argc, char **argv) {
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


int main(int argc, char **argv) {
    Lib3dsFile *f = 0;

    parse_args(argc, argv);

    f = lib3ds_file_open(input);
    if (!f) {
        fprintf(stderr, "***ERROR***\nLoading file failed: %s\n", input);
        exit(1);
    }

    lib3ds_file_free(f);
    return 0;
}
