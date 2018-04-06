/*
 * Copyright (c) 1991-95 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the name of Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE
 * POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

//
// Convert from ASCII/binary Inventor file to a file suitable for 
// use as a compiled in variable.
//
// Format for these files is as follows:
//
// const char *variableName = { "0xXXXX 0xXXXX 0xXXXX" };
//
// where the stuff between the quotes is a hex version of the binary file.
//

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdio.h>
#if defined(sgi)
#include <getopt.h>
#include <unistd.h>
#endif
#if defined(WIN32)
#include <mle/mlGetopt.h>
#endif
#include <string.h>
#include <assert.h>

// Include Inventor header files.
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/SoLists.h>
#include <Inventor/actions/SoWriteAction.h>

// mvo commented out 7/10/96 - needed?
//#include "InpText.h"
//#include "Image2.h"
//#include "RedrawGroup.h"

static void
print_usage(const char *progname)
{
    (void)fprintf(stderr, "Usage: %s variableName inputFile.iv\n",
		  progname);
    (void)fprintf(stderr, "variableName : The name of the variable to create.\n");
	(void)fprintf(stderr, "inputFile : The name of the Inventor file.\n");
    exit(99);
}

static void
parse_args(int argc, char **argv, char **variableName, char **inputFile)
{
    if (argc != 3)
    {
		print_usage(argv[0]);
    }
    (*variableName) = strdup(argv[1]);
	(*inputFile) = strdup(argv[2]);

    return; 
}

int main(int argc, char **argv)
{
    SoDB::init();
  
// mvo commented out 7/10/96 - needed?
//    InpText::initClass();
//    RedrawGroup::initClass();
//    Image2::initClass();

    // Parse arguments.
    char *variableName = NULL;
	char *inputFile = NULL;

    parse_args(argc, argv, &variableName, &inputFile );

    // Read stuff from file.
    SoInput in;
    SoSeparator *root;

    in.openFile(inputFile);
    root = SoDB::readAll( &in );
    if (root)
		root->ref();

    // Write stuff into a buffer.
    SoOutput out;
    out.setBinary(TRUE);
    out.setBuffer( malloc(1000), 1000, realloc );
    SoWriteAction writer(&out);
    writer.apply(root);

    if (root)
		root->unref();

    // Create the output file.
    void *buf;
    size_t size;
    out.getBuffer(buf, size);
    char *outputBuffer = (char *) buf;
    fprintf( stdout, "const char %s[] = {\n", variableName );
    fprintf(stderr,"bufferSize = %d\n", size);

    // All but last number get commas afterwards
    for ( unsigned int j = 0; j < size-1; j++ )
	{
		if (j%16 == 0)
			fprintf( stdout, "\n");
		fprintf( stdout, "0x%X, ", outputBuffer[j] );
    }

    // Last number gets no comma afterwards
    fprintf( stdout, "0x%X ", outputBuffer[size-1] );
    fprintf( stdout, "\n};\n");

    return 0;
}
