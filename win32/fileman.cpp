/* 
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 * satan@xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, the file "gpl.txt"; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA, or visit http://www.gnu.org.
 */

#include "stdafx.h"


#include "fileman.h"


#define TEMP_DEFAULT_PREPEND "~xtmp"
#define DELTA_SIZE 5
#define MAX_TEMP_FILES_DIGITS 3
#define MAX_TEMP_FILES 100


TempFileManager::TempFileManager()
:directoryname(NULL),tempfiles(NULL), 
numtempfiles(0),buffersize(0)
{
}



TempFileManager::~TempFileManager()
{
    cleanup();
    if (directoryname)
        free(directoryname);
}



/**************************
public methods
**************************/

/*
windows specific implementations
*/
void
TempFileManager::attachDirectory(const char *dirname)
{
    if (!dirname)
        return;
    if (directoryname) {
        cleanup();
        free(directoryname);
    }
    directoryname = strdup(dirname);
    
    CString t_string=directoryname;
    t_string += TEMP_DEFAULT_PREPEND;
    t_string += '*';
    WIN32_FIND_DATA c_file;
    /* find first .c file in current directory */
    HANDLE t_handle;
    if (INVALID_HANDLE_VALUE != (t_handle=FindFirstFile( t_string.GetBuffer(0), &c_file)) )
    {
        CString t_newfile;
        if (c_file.cFileName) {
           t_newfile =  directoryname;
           t_newfile += c_file.cFileName;
        }
        addName(strdup(t_newfile.GetBuffer(0)));
        t_newfile.ReleaseBuffer();
        /* Find the rest of the xtm files */
        while( FindNextFile( t_handle,&c_file )  )
        {
           if (c_file.cFileName) {
               t_newfile= directoryname;
               t_newfile += '\\';
               t_newfile+= c_file.cFileName;
           }
            addName(strdup(t_newfile.GetBuffer(0)));
            t_newfile.ReleaseBuffer();
        }
    }

}



const char *
TempFileManager::getTempFileByIndex( int index )
{
    if ((index < numtempfiles) && (index >=0)) {
        return tempfiles[index];
    }
    return NULL;
}



/*
delete all files in tempfiles list
leave directory name alone.
*/
void
TempFileManager::cleanup()
{
    for (int i=0;i < numtempfiles; i++)
    {
        try
        {
            CFile::Remove(tempfiles[i]);
        }
        catch (CFileException*)
        {
//            assert(0);  commented hardts
        }
        free (tempfiles[i]);
    }
    if (tempfiles)
        free (tempfiles);
    tempfiles = NULL;
    numtempfiles = 0;
    buffersize = 0;
}



const char *
TempFileManager::newTempFileName(const char *ext)
{
    if (!directoryname || !ext)
        return NULL;

    CString buffer;
    UINT t_result = GetTempFileName(directoryname,TEMP_DEFAULT_PREPEND, TRUE, buffer.GetBuffer(MAX_PATH));
    buffer.ReleaseBuffer();
    buffer = buffer.Left(buffer.ReverseFind('.')+1);
    buffer += ext;
    if (!t_result)
        return NULL;
    addName(strdup(buffer.GetBuffer(0)));
    return tempfiles[numtempfiles-1];
}






/**************************
helper methods
**************************/
void
TempFileManager::addName( char *name )
{
    if (!tempfiles)
    {
        tempfiles = (char **) malloc(DELTA_SIZE * (sizeof (char *)));
        if (!tempfiles)
            return;
        buffersize = DELTA_SIZE;
    }
    else if (numtempfiles >= buffersize)
    {
        char **t_tempfiles = (char **) realloc(tempfiles,(buffersize+DELTA_SIZE) * (sizeof (char *)));
        if (!t_tempfiles)
            return;
        tempfiles = t_tempfiles;
        buffersize += DELTA_SIZE;
    }
    if (numtempfiles > MAX_TEMP_FILES)
    {
        cleanup();
        return;
    }
    tempfiles[numtempfiles ++] = name;
}


