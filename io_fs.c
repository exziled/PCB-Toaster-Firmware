//*****************************************************************************
//
// io_fs.c - File System Processing for enet_io application.
//
// Copyright (c) 2007-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.0.12573 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "utils/lwiplib.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "httpserver_raw/httpd.h"
#include "httpserver_raw/fs.h"
#include "httpserver_raw/fsdata.h"
#include "io.h"

//*****************************************************************************
//
// Include the web file system data for this application.  This file is
// generated by the makefsfile utility, using the following command:
//
//     ../../../../tools/bin/makefsfile -i fs -o io_fsdata.h -r -h -q
//
// If any changes are made to the static content of the web pages served by the
// application, this script must be used to regenerate io_fsdata.h in order
// for those changes to be picked up by the web server.
//
//*****************************************************************************
#include "io_fsdata.h"

//*****************************************************************************
//
// Open a file and return a handle to the file, if found.  Otherwise,
// return NULL.  This function also looks for special file names used to
// provide specific status information or to control various subsystems.
// These file names are used by the JavaScript on the "IO Control Demo 1"
// example web page.
//
//*****************************************************************************
struct fs_file *
fs_open(const char *pcName)
{
    const struct fsdata_file *psTree;
    struct fs_file *psFile = NULL;

    //
    // Allocate memory for the file system structure.
    //
    psFile = mem_malloc(sizeof(struct fs_file));
    if(psFile == NULL)
    {
        return(NULL);
    }

    //
    // Process request to toggle STATUS LED
    //
    if(ustrncmp(pcName, "/cgi-bin/toggle_led", 19) == 0)
    {
        static char pcBuf[4];

        //
        // Toggle the STATUS LED
        //
        io_set_led(!io_is_led_on());

        //
        // Get the new state of the LED
        //
        io_get_ledstate(pcBuf, 4);

        psFile->data = pcBuf;
        psFile->len = strlen(pcBuf);
        psFile->index = psFile->len;
        psFile->pextension = NULL;

        //
        // Return the psFile system pointer.
        //
        return(psFile);
    }

    else if (ustrncmp(pcName, "/get_temp", 9) == 0)
    {
        static char tempBuf[6];

        io_get_temperature(tempBuf, 6);

        psFile->data = tempBuf;
        psFile->len = strlen(tempBuf);
        psFile->index = psFile->len;
        psFile->pextension = NULL;


        return psFile;
    }


    else if (ustrncmp(pcName, "/set_profile=", 9) == 0)
    {
        io_start_bake((char *)(pcName + 13), strlen(pcName) - 13);

        static char tempBuf[] = "Test";

        psFile->data = tempBuf;
        psFile->len = strlen(tempBuf);
        psFile->index = psFile->len;
        psFile->pextension = NULL;


        return psFile;
    } 

    //
    // Request for LED State?
    //
    else if(ustrncmp(pcName, "/ledstate", 9) == 0)
    {
        static char pcBuf[4];

        //
        // Get the state of the LED
        //
        io_get_ledstate(pcBuf, 4);

        psFile->data = pcBuf;
        psFile->len = strlen(pcBuf);
        psFile->index = psFile->len;
        psFile->pextension = NULL;
        return(psFile);
    }
    //
    // Request for the animation speed?
    //
    else if(ustrncmp(pcName, "/get_speed", 10) == 0)
    {
        static char pcBuf[6];

        //
        // Get the current animation speed as a string.
        //
        io_get_animation_speed_string(pcBuf, 6);

        psFile->data = pcBuf;
        psFile->len = strlen(pcBuf);
        psFile->index = psFile->len;
        psFile->pextension = NULL;
        return(psFile);
    }
    //
    // Set the animation speed?
    //
    else if(ustrncmp(pcName, "/cgi-bin/set_speed?percent=", 12) == 0)
    {
        static char pcBuf[6];

        //
        // Extract the parameter and set the actual speed requested.
        //
        io_set_animation_speed_string((char*)pcName + 27);

        //
        // Get the current speed setting as a string to send back.
        //
        io_get_animation_speed_string(pcBuf, 6);

        psFile->data = pcBuf;
        psFile->len = strlen(pcBuf);
        psFile->index = psFile->len;
        psFile->pextension = NULL;
        return(psFile);
    }
    //
    // If I can't find it there, look in the rest of the main psFile system
    //
    else
    {
        //
        // Initialize the psFile system tree pointer to the root of the linked
        // list.
        //
        psTree = FS_ROOT;

        //
        // Begin processing the linked list, looking for the requested file name.
        //
        while(NULL != psTree)
        {
            //
            // Compare the requested file "pcName" to the file name in the
            // current node.
            //
            if(ustrncmp(pcName, (char *)psTree->name, psTree->len) == 0)
            {
                //
                // Fill in the data pointer and length values from the
                // linked list node.
                //
                psFile->data = (char *)psTree->data;
                psFile->len = psTree->len;

                //
                // For now, we setup the read index to the end of the file,
                // indicating that all data has been read.
                //
                psFile->index = psTree->len;

                //
                // We are not using any file system extensions in this
                // application, so set the pointer to NULL.
                //
                psFile->pextension = NULL;

                //
                // Exit the loop and return the file system pointer.
                //
                break;
            }

            //
            // If we get here, we did not find the file at this node of the
            // linked list.  Get the next element in the list.
            //
            psTree = psTree->next;
        }
    }

    //
    // If we didn't find the file, ptTee will be NULL.  Make sure we
    // return a NULL pointer if this happens.
    //
    if(NULL == psTree)
    {
        mem_free(psFile);
        psFile = NULL;
    }

    //
    // Return the file system pointer.
    //
    return(psFile);
}

//*****************************************************************************
//
// Close an opened file designated by the handle.
//
//*****************************************************************************
void
fs_close(struct fs_file *psFile)
{
    //
    // Free the main psFile system object.
    //
    mem_free(psFile);
}

//*****************************************************************************
//
// Read the next chunk of data from the file.  Return the iCount of data
// that was read.  Return 0 if no data is currently available.  Return
// a -1 if at the end of file.
//
//*****************************************************************************
int
fs_read(struct fs_file *psFile, char *pcBuffer, int iCount)
{
    int iAvailable;

    //
    // Check to see if a command (pextension = 1).
    //
    if(psFile->pextension == (void *)1)
    {
        //
        // Nothing to do for this file type.
        //
        psFile->pextension = NULL;
        return(-1);
    }

    //
    // Check to see if more data is available.
    //
    if(psFile->len == psFile->index)
    {
        //
        // There is no remaining data.  Return a -1 for EOF indication.
        //
        return(-1);
    }

    //
    // Determine how much data we can copy.  The minimum of the 'iCount'
    // parameter or the available data in the file system buffer.
    //
    iAvailable = psFile->len - psFile->index;
    if(iAvailable > iCount)
    {
        iAvailable = iCount;
    }

    //
    // Copy the data.
    //
    memcpy(pcBuffer, psFile->data + iAvailable, iAvailable);
    psFile->index += iAvailable;

    //
    // Return the count of data that we copied.
    //
    return(iAvailable);
}

//*****************************************************************************
//
// Determine the number of bytes left to read from the file.
//
//*****************************************************************************
int
fs_bytes_left(struct fs_file *psFile)
{
    //
    // Return the number of bytes left to be read from this file.
    //
    return(psFile->len - psFile->index);
}
